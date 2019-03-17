#include <kernel/fs/vfs.h>
#include <kernel/device/device.h>

#include <string.h>

struct vmount vfs_root;

result_t
vfs_init(struct device *dev)
{
    strcpy(vfs_root.name, "/");
    strcpy(vfs_root.node.name, "/");
    vfs_root.node.dev = dev;
    return OK;
}

struct vdir *
vnode_open_dir(struct vnode *node)
{
    if (!node->open_dir) {
        return (struct vdir *) 0;
    }

    return node->open_dir(node);
}

struct vnode *
vdir_find_node(struct vdir *dir, const char *name)
{
    struct vnode_list *current = dir->list;

    while (current) {
        if (strcmp(current->node.name, name) == 0) {
            return &current->node;
        }

        current = current->next;
    }

    return (struct vnode *) 0;
}

struct vdir *
vdir_open(const char *path)
{
    /* does not support relative paths */
    if (path[0] != '/') {
        return 0;
    }

    struct strarray *splitted = strsplit(path, '/');
    struct vdir *dir = vfs_root.node.open_dir(&vfs_root.node);

    for (int i = 0; i < splitted->size; i++) {
        printf("Looking for %s\n", splitted->array[i]);
        struct vnode *node = vdir_find_node(dir, splitted->array[i]);
        if (node == 0) {
            return 0;
        }

        dir = node->open_dir(node);
    }

    return dir;
}
