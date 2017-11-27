#ifndef ALIEN_VFS_H
#define ALIEN_VFS_H

#include <types.h>

#define PATH_MAX 128

typedef struct vfs_node vfs_node_t;

typedef i64 (*vfs_node_read_t) (const vfs_node_t *node, u32 offset,
                                u32 len, u8 *dest);

typedef i64 (*vfs_node_write_t) (const vfs_node_t *node, u32 offset,
								 u32 len, const u8 *src);

typedef i8 (*vfs_node_find_t) (const vfs_node_t *node, const char *path,
								vfs_node_t *out);

struct vfs_node {
	char path[PATH_MAX];
	vfs_node_read_t read;
	vfs_node_write_t write;
	vfs_node_find_t find;
	u64 size;
};

i64 vfs_read (const vfs_node_t *node, u32 offset, u32 len, u8 *dest);

i64 vfs_write (const vfs_node_t *node, u32 offset, u32 len,
				const u8 *src);
				
i8 vfs_find (const vfs_node_t *node, const char *path, vfs_node_t *dir);

void vfs_init(const vfs_node_t *root);

#endif
