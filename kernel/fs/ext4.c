#include <kernel/fs/ext4.h>

#include <kernel/fs/vfs.h>
#include <kernel/memory/heap.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>

#define EXT4_MAGIC 0xEF53

#define EXT4_BLOCK_SIZE 1024

struct superblock
{
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    int32_t  log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    int16_t  max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t pad;
    uint32_t lastcheck;
    uint32_t check_interval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t reserved_blocks_uid;
    uint16_t reserved_blocks_gid;
    uint32_t first_non_reserved_inode;
    uint16_t inode_size;
    uint16_t sb_block_group;
    uint32_t opt_features;
    uint32_t req_features;

    uint8_t reserved[922];
} __attribute__((packed));

struct groupinfo
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint8_t  _[14];
} __attribute__((packed));

#define GROUP_INFOS_COUNT_PER_BLOCK (EXT4_BLOCK_SIZE / sizeof(struct groupinfo))

struct inode
{
    uint16_t mode;
    uint16_t uid;
    uint32_t size_low;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t _reserved1;
    uint32_t block[15];
    uint32_t version;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t  frag;
    uint8_t  fsize;
    uint16_t _[5];
} __attribute__((packed));

struct dirent
{
    uint32_t inode;
    uint16_t size;
    uint8_t name_length_lo;
    uint8_t name_length_hi;
} __attribute__((packed));


/*
 * Incompatibily features. If one of these feature is present in sb.req_features
 */

/* Compression */
#define INCOMPAT_COMPRESSION 0x1
/* Directory entries record the file type */
#define INCOMPAT_FILETYPE 0x2
/* Filesystem needs recovery */
#define INCOMPAT_RECOVER 0x4
/* Filesystem has a separate journal device */
#define INCOMPAT_JOURNAL_DEV 0x8
/* Meta block groups */
#define INCOMPAT_META_BG 0x10
/* Files in this filesystem use extents */
#define INCOMPAT_EXTENTS 0x40
/* Enable a filesystem of 2^64 blocks */
#define INCOMPAT_64BITS 0x80
/* Multiple mount protection. Not implemented */
#define INCOMPAT_MMP 0x100
/* Flexible block groups */
#define INCOMPAT_FLEX_BG 0x200
/* Inodes can be used to store large extended attribute values */
#define INCOMPAT_EA_INODE 0x400
/* Data in directory entry. Not implemented ? */
#define INCOMPAT_DIRDATA 0x1000
/*
 * metadata_csum filesystem while the filesystem is mounted; without it, the
 * checksum definition requires all metadata blocks to be rewritten.
 */
#define INCOMPAT_CSUM_SEED 0x2000
/* Large directory >2GB or 3^3 */
#define INCOMPAT_LARGEDIR 0x4000
/* Data in inode */
#define INCOMPAT_INLINE_DATA 0x8000
/* Encrypted inodes are present on the filesystem */
#define INCOMPAT_ENCRYPT 0x10000

#define INCOMPAT_SUPPORTED INCOMPAT_FILETYPE

static void
ext4_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    printf("[EXT4] [DEBUG] ");
    vprintf(fmt, args);
    
    va_end(args);
}

static void
ext4_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    printf("[EXT4] [ERROR] ");
    vprintf(fmt, args);
    
    va_end(args);
}

static inline uint32_t
get_block_size(const struct superblock *sb)
{
    return 1024 << sb->log_block_size;
}

static inline uint32_t
inode_get_group(const struct superblock *sb, uint32_t inode_id)
{
    return (inode_id - 1) / sb->inodes_per_group;
}

static inline uint32_t
inode_get_index_in_group(const struct superblock *sb, uint32_t inode_id)
{
    return (inode_id - 1) % sb->inodes_per_group;
}

static void
read_groupinfos(struct device *dev, struct groupinfo *infos)
{
    uint32_t size = 1024;
    device_random_read(dev, 2048, &size, (uint8_t *) infos);
}

/**
 * Very important function which find an inode on the disk from its index. The
 * index is the global identifier of the inode. It is find in directory entries.
 * 
 * It first locate the correct block group,
 *      then read group infos,
 *      then locate the inode table from the group infos,
 *      then read inode table and returns the correct entry.
 * Returns the inode structure as value.
 */
static inline struct inode
find_inode(struct device *dev, const struct superblock *sb, uint32_t inode_id)
{
    uint32_t index = inode_get_index_in_group(sb, inode_id);
    uint32_t group = inode_get_group(sb, inode_id);
    uint32_t block_size = get_block_size(sb);

    struct groupinfo infos[GROUP_INFOS_COUNT_PER_BLOCK];
    read_groupinfos(dev, infos);

    uint32_t table = infos[group].inode_table;

    struct inode inodes[32];
    uint32_t size = min(sb->inodes_per_group * sb->inode_size, sizeof(inodes));
    
    device_random_read(dev, table * 1024, &size, (uint8_t *) inodes);

    return inodes[index];
}

static int64_t
inode_read(struct device *dev, struct inode *inode, uint8_t *out, uint32_t size)
{
    int64_t size_to_read = min(inode->size_low, size);
    uint32_t sector = 0;

    ext4_debug("inode_read - uid %d, size_to_read %d\n", inode->uid, size_to_read);

    while (size_to_read > 0) {
        int64_t sector_size_to_read = min(size_to_read, 1024);
        
        if (device_random_read(dev, inode->block[sector] * 1024,
            &sector_size_to_read, out) == ERROR)
        {
            ext4_error("inode_read - error while reading sector 0x%x\n",
                inode->block[sector]);
            return -1;
        }

        size_to_read -= sector_size_to_read;
        sector++;
    }

    return 0;
}

static struct vdir *
ext4_open_dir(struct vnode *node)
{
    ext4_debug("ext4_open_dir\n");

    struct vdir *dir = kmalloc(sizeof(struct vdir));
    dir->node = node;

    struct superblock *sb = node->dev->fs_private;

    struct inode *inode = (struct inode *) node->private;
    uint32_t inode_size = inode->size_low;

    uint8_t *buffer = kmalloc(inode_size);
        
    if (inode_read(node->dev, inode, buffer, inode_size) < 0) {
        return 0;
    }

    dir->list = kmalloc(sizeof(struct vnode_list));
    struct vnode_list *list_cur = dir->list;
    list_cur->next = 0;

    struct dirent *ent;
    ent = buffer;

    while (((uint32_t) ent) + ent->size <= (uint32_t) buffer + inode_size) {
        if (ent->inode == 0) {
            continue;
        }

        struct vnode *cur = &list_cur->node;

        /* Read name */
        char *name = (((uint32_t) ent) + sizeof(struct dirent));
        int size = min(VFS_NAME_MAX, ent->name_length_lo);
        strncpy(cur->name, name, size);

        /* Copy common data from parent node */
        cur->open_dir = node->open_dir;
        cur->dev = node->dev;

        /* Fetch inode */
        struct inode *cur_inode = kmalloc(sizeof(struct inode));
        *cur_inode = find_inode(node->dev, sb, ent->inode);

        cur->private = cur_inode;

        struct vnode_list *next = kmalloc(sizeof(struct vnode_list));
        next->next = 0;
        list_cur->next = next;
        list_cur = next;

        ent = (((uint32_t) ent) + ent->size); 
    }

    return dir;
}

/**
 * Initialize a ext4 device. Reads the superblock and search the root inode.
 * dev: input device
 * root: output root inode
 */
static result_t
ext4_init(struct device *dev, struct inode *root)
{
    struct superblock *sb = kmalloc(sizeof(struct superblock));

    /* Read the superblock */    
    uint32_t size = sizeof(struct superblock);
    device_random_read(dev, 1024, &size, sb);

    /* Check magic number */
    if (sb->magic != EXT4_MAGIC) {
        ext4_error("Invalid magic number: 0x%x\n", sb->magic);
        return ERROR;
    }

    /* Check required features */
    if (sb->req_features != INCOMPAT_SUPPORTED) {
        ext4_error("Unsupported features: 0x%x\n", sb->req_features);
        return ERROR;
    }

    /* Parse the superblock */
    uint32_t blocksize = 1024 << sb->log_block_size;
    if (updiv(sb->blocks_count, sb->blocks_per_group) != updiv(sb->inodes_count, sb->inodes_per_group)) {
        ext4_error("Inconsistent block group count\n");
        return ERROR;
    }
  
    /* Read root inode */
    *root = find_inode(dev, sb, 2);

    /* Register the superblock in device structure */
    dev->fs_private = sb;

    return OK;
}

result_t
ext4_mount(struct device *dev, struct vnode *node)
{
    ext4_debug("Mounting ext4 filesystem...\n");

    struct inode *root_inode = kmalloc(sizeof(struct inode));
    
    result_t res;
    if ((res = ext4_init(dev, root_inode)) != OK) {
        kfree(root_inode);
        return res;
    }

    node->private = root_inode;
    node->open_dir = ext4_open_dir;

    return OK;
}
