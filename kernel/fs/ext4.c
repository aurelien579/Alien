#include <kernel/fs/ext4.h>

#include <kernel/fs/vfs.h>
#include <kernel/memory/heap.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <math.h>

#define EXT4_MAGIC 0xEF53

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

static inline struct inode
find_inode(struct device *dev, const struct superblock *sb, uint32_t inode_id)
{
    uint32_t index = inode_get_index_in_group(sb, inode_id);
    uint32_t group = inode_get_group(sb, inode_id);
    uint32_t block_size = get_block_size(sb);

    struct groupinfo infos[1024/32];
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

    printf("reading: %d\n", size_to_read);

    while (size_to_read > 0) {
        int64_t sector_size_to_read = min(size_to_read, 1024);
        printf("sector %x\n", inode->block[sector]);
        
        if (device_random_read(dev, inode->block[sector] * 1024, &sector_size_to_read, out) == ERROR) {
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
    printf("ext4_open_dir\n");

    struct vdir *dir = kmalloc(sizeof(struct vdir));
    dir->node = node;

    struct superblock *sb = node->dev->fs_private;

    struct inode *inode = (struct inode *) node->private;
    uint32_t inode_size = inode->size_low;

    uint8_t *buffer = kmalloc(inode_size);
        
    inode_read(node->dev, inode, buffer, inode_size);

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

static result_t
ext4_init(struct device *dev, struct inode *root)
{
    struct superblock *sb = kmalloc(sizeof(struct superblock));
    
    uint32_t size = sizeof(struct superblock);
    device_random_read(dev, 1024, &size, sb);

    if (sb->magic != EXT4_MAGIC) {
        printf("[EXT4] Invalid magic number: 0x%x\n", sb->magic);
        return ERROR;
    }

    /* 
        REALLY REALLY REALLY basic feature detection :
        ext2 have feature 'Directory entries record the file type' which is 0x02
        Other features are not supported
    */
    if (sb->req_features != 0x02) {
        printf("[EXT4] Unsupported features: 0x%x\n", sb->req_features);
        return ERROR;
    }

    uint32_t blocksize = 1024 << sb->log_block_size;
    printf("[EXT4] blocksize=%d\n", blocksize);
    printf("[EXT4] inodes_count=%d\n", sb->inodes_count);
    printf("[EXT4] blocks_count=%d\n", sb->blocks_count);
    printf("[EXT4] blocks_per_group=%d\n", sb->blocks_per_group);
    if (updiv(sb->blocks_count, sb->blocks_per_group) != updiv(sb->inodes_count, sb->inodes_per_group)) {
        printf("[EXT4] Inconsistent block group count\n");
        return ERROR;
    }

    uint32_t groups_count = updiv(sb->blocks_count, sb->blocks_per_group);
    printf("[EXT4] groups_count=%d\n", groups_count);

    struct groupinfo groupinfos[1024/32] __attribute__((packed));
    size = sizeof(groupinfos);
    device_random_read(dev, 2048, &size, (uint8_t *) groupinfos);
    
    /*for (int i = 0; i < groups_count; i++) {
        printf("[EXT4] block_bitmap_addr=%d\n", groupinfos[i].block_bitmap_addr);
    }*/

    printf("[EXT4] first_non_reserved_inode=%d\n", sb->first_non_reserved_inode);
    struct inode root_inode = find_inode(dev, sb, 2);
    printf("[EXT4] root_inode=%d\n", root_inode.mode);

    uint8_t buffer[512];

    inode_read(dev, &root_inode, buffer, sizeof(buffer));
    
    struct dirent *ent;
    ent = buffer;

    *root = root_inode;
    dev->fs_private = sb;

    /*for (int i = 0; i < 10; i++) {
        char *name = (((uint32_t) ent) + sizeof(struct dirent));
        printf("entry { %d, %d, %d, %s }\n", ent->inode, ent->size, ent->name_length, name);
        ent = (((uint32_t) ent) + ent->size); 
    }
    
    for (uint32_t i = 0; i < 512; i++) {
        printf("%x", buffer[i]);
    }*/


    return OK;
}

result_t
ext4_mount(struct device *dev, struct vnode *node)
{
    printf("[EXT4] Mount\n");

    struct inode *root_inode = kmalloc(sizeof(struct inode));
    printf("inode: 0x%x\n", root_inode);
    result_t res;

    if ((res = ext4_init(dev, root_inode)) != OK) {
        kfree(root_inode);
        return res;
    }

    node->private = root_inode;
    node->open_dir = ext4_open_dir;

    return OK;
}
