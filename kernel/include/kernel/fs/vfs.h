#ifndef VFS_H
#define VFS_H

#include <kernel/device/device.h>

#define VFS_NAME_MAX    256

struct vnode;

typedef struct vdir *(*vnode_open_dir_t) (struct vnode *);

struct vnode
{
    char                name[VFS_NAME_MAX];
    vnode_open_dir_t    open_dir;
    struct device      *dev;
    void               *private;
};

struct vnode_list
{
    struct vnode        node;
    struct vnode_list  *next;
};

struct vdir
{
    struct vnode       *node;
    struct vnode_list  *list;
    void               *private;
};

struct vmount
{
    char         name[VFS_NAME_MAX];
    struct vnode node;
};

extern struct vmount vfs_root;

struct vdir *vdir_open(const char *path);
struct vnode *vdir_find_node(struct vdir *dir, const char *name);
result_t vfs_init(struct device *dev);

#endif
