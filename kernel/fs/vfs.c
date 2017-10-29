#include <alien/vfs.h>

static vfs_node_t vfs_root;

i64
vfs_read(const vfs_node_t *node, u32 offset, u32 len, u8 *dest)
{
	if (node) {
		if (node->read) {
			return node->read(node, offset, len, dest);
		}
	}
	
	return -1;
}

i64
vfs_write(const vfs_node_t *node, u32 offset, u32 len, const u8 *src)
{
	if (node) {
		if (node->write) {
			return node->write(node, offset, len, src);
		}
	}
	
	return -1;
}

i8
vfs_find(const vfs_node_t *node, const char *path, vfs_node_t *out)
{
	if (!node) {
		node = &vfs_root;
	}
	
	if (path) {
		if (node->find) {
				return node->find(node, path, out);
		}
	}
	
	return -1;
}

void
vfs_init(const vfs_node_t *root)
{
	vfs_root = *root;
}
