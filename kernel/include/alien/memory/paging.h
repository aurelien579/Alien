#ifndef PAGING_H
#define PAGING_H

#include <types.h>

/**
 * Should be called before any other functions in this module.
 */
void init_paging();

/**
 * Allocate a new page. Return 0 if an error occured (ie. if no more
 * physical frame is available.
 */
u32 alloc_kpage();

/**
 * Free a previously allocated frame.
 */
void free_page(u32 page);

u32 alloc_page(u32 offset, u32 user);

void switch_page_dir(u32 dir);

u32 create_user_pagedir();
u32 map(u32 frame, u32 offset, u32 user);
void unmap(u32 page);
u32 phys_addr(u32 *dir, u32 page);
u32 copy_current_pagedir();

#endif
