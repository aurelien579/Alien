#ifndef PAGING_H
#define PAGING_H

#include <types.h>

extern u32 *_kernel_end;
static int kernel_end = &_kernel_end;

static inline u32 kernel_virtual_base()
{
    extern u32 KERNEL_VIRTUAL_BASE;
    return &KERNEL_VIRTUAL_BASE;
}

void init_paging();

#endif
