#ifndef MM_H
#define MM_H

#include <types.h>

u32 init_mm(u32 mem_len);
paddr_t alloc_blocks(u32 n);
void free_blocks(u32 base, u32 n);
u32 used_memory();

#endif
