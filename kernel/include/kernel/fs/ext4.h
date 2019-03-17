#ifndef EXT4_H
#define EXT4_H
 
#include <kernel/fs/vfs.h>
#include <kernel/device/device.h>

result_t ext4_mount(struct device *dev, struct vnode *node);

#endif
