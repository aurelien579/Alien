#include <alien/vfs.h>
#include <alien/string.h>
#include <alien/io.h>

#define TAR_NORMAL_FILE '0'
#define TAR_HARDLINK	'1'
#define TAR_SYMLINK		'2'
#define TAR_CHAR_SPECIAL '3'
#define TAR_BLOCK_SPECIAL '4'
#define TAR_DIRECTORY '5'


#define TAR_FILE_ADRESS(h) ((char *) (((unsigned int)h) + 512))

typedef struct
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
} tar_header_t; 

static tar_header_t *headers[32];
static unsigned int file_count = 0;

static i8
initrd_find(const vfs_node_t *dir, const char *path, vfs_node_t *node);
static i64
initrd_read(const vfs_node_t *node, u32 offset, u32 len, u8 *dest);


static i64
tar_read_size(const char *in)
{
    i64 size = 0;
    int j;
    int count = 1;
 
    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);
 
    return size;
}

static int
parse(unsigned int address)
{
    unsigned int i;
 
    for (i = 0; ; i++) { 
        tar_header_t *header = (tar_header_t *) address;
 
        if (header->filename[0] == '\0')
            break;
 
        int size = tar_read_size(header->size);
 
        headers[i] = header;
 
        address += ((size / 512) + 1) * 512;
 
        if (size % 512)
            address += 512; 
    }
 
    return i;
}

static int
find_header_index(const char *path)
{
	for (unsigned int i = 0; i < file_count; i++) {
		if (!strcmp(path, headers[i]->filename)) {
			return i;
		}
	}
	
	return -1;
}

static tar_header_t*
find_header(const char *path)
{
	int i = find_header_index(path);
	
	if (i >= 0) {
		return headers[i];
	} else {	
		return (tar_header_t *) 0;
	}
}

static i64
initrd_read(const vfs_node_t *node, u32 offset, u32 len, u8 *dest)
{
	tar_header_t *header = find_header(node->path);
	
	if (!header) {
		return 0;
	}
	
	const char* ptr = TAR_FILE_ADRESS(header);
	unsigned int i = 0;
	
	while (i < len && i + offset < node->size) {
		dest[i] = ptr[i + offset];
		i++;
	}
	
	return i;
}

static void
init_node(vfs_node_t *node, const tar_header_t *header)
{
	node->read = initrd_read;
	node->write = 0;
	node->find = initrd_find;
	node->size = tar_read_size(header->size);
	strcpy(node->path, header->filename);
}

static i8
initrd_find(const vfs_node_t *dir, const char *path, vfs_node_t *node)
{
	int i = find_header_index(dir->path);
	
	if (i < 0) {
		return -1;
	}
	
	if (headers[i]->typeflag != TAR_DIRECTORY) {
		return -1;
	}
	
	i++;
	
	
	
	for (; i < (int) file_count; i++) {
		if (headers[i]->typeflag == TAR_DIRECTORY) {
			return -1;
		} else {
			int pos = strlen(headers[i]->filename) - 1;
			while (pos >= 0 && headers[i]->filename[pos] != '/')
				pos--;
							
			if (!strcmp(path, &headers[i]->filename[pos])) {
				init_node(node, headers[i]);
				return 0;
			}
		}
	}
	
	return -1;
}

void
init_initrd(unsigned int addr, vfs_node_t *root)
{
	file_count = parse(addr);
	
	init_node(root, headers[0]);
}
