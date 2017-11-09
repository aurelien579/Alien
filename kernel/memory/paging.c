#include <alien/memory/paging.h>
#include <alien/kernel.h>
#include <alien/io.h>
#include <alien/string.h>

#define PAGE_SIZE	0x1000

#define PAGETABLE_VADDR(i)		((1023 << 22) + (i << 12))
#define PAGE_ENTRY_BASE(e) 		(e & 0xFFFFF000)
#define PAGEDIR_INDEX(base) 	((base & 0xFFC00000) >> 22)
#define PAGETABLE_INDEX(base) 	((base & 0x003FF000) >> 12)

static u8  bitmap_size = 0;
static u8 *bitmap;
static u32 kernel_pagedir[1024] __attribute__ ((aligned (PAGE_SIZE)));
static u32 *current_pagedir = (u32 *) 0xFFFFF000;

static inline void
invlpg(u32 page)
{
	asm volatile ("invlpg (%0)" :: "r" (page) : "memory");
}

static void
write_page_entry(u32 *structure, u32 i, u32 frame, u8 user)
{
	structure[i] = (frame & 0xFFFFF000);
	structure[i] |= 1;
	structure[i] |= 2; /* R/W */
	structure[i] |= ((user & 0x1) << 2);
}

static void
clear_page_entry(u32 *structure, u32 i)
{
	structure[i] = 0;
}

static u8
page_entry_is_present(u32 *structure, u32 i)
{
	return structure[i] & 1;
}

static u32
first_page_free(u32 *structure, u32 base)
{
	u32 i = 0, j = 0;
	
	i = PAGEDIR_INDEX(base);
	j = PAGETABLE_INDEX(base);
	
	for (; i < 1024; i++) {
		if (page_entry_is_present(structure, i)) {
			u32 *page_table = (u32 *) PAGETABLE_VADDR(i);
			
			if (i > PAGEDIR_INDEX(base)) {
				j = 0;
			}
			
			for (; j < 1024; j++) {
				if (!page_entry_is_present(page_table, j)) {
					return (i << 22) + (j << 12);
				}
			}
		} else {
			return (i << 22) + (j << 12);
		}
	}
	
	kprintf("first_page_free() ERROR\n");
	return 0;
}

static u32
alloc_frame()
{
	u32 i = 0, j = 0;
	
	while (bitmap[i] == 0xFF) {
		i++;
	}
	
	if (i == bitmap_size) {
		return 0;
	}
	
	while (bitmap[i] & (1 << j)) {
		j++;
	}
	
	if (j == 8)
		return 0;
	
	bitmap[i] |= (1 << j);
	
	return (i * 8 + j) * PAGE_SIZE;
}

static void
free_frame(u32 frame)
{
	u32 i = frame / (PAGE_SIZE * 8);
	u32 j = (frame / PAGE_SIZE) % 8;
	
	bitmap[i] &= ~(1 << j);
}

static u8
pagetable_is_empty(u32 *table)
{
	for (u32 i = 0; i < 1024; i++) {
		if (page_entry_is_present(table, i)) {
			return 0;
		}
	}
	
	return 1;
}

u32
phys_addr(u32 *dir, u32 page)
{
	if (!page_entry_is_present(dir, PAGEDIR_INDEX(page))) {
		return 0;
	}
	
	u32 *table = (u32 *) PAGETABLE_VADDR(PAGEDIR_INDEX(page));
	
	u32 frame = PAGE_ENTRY_BASE(table[PAGETABLE_INDEX(page)]);
	
	return frame;
}

u32
map(u32 frame, u32 offset, u32 user)
{
	if (frame == 0)
		return 0;
	
	u32 page = first_page_free(current_pagedir, offset);
	if (page == 0)
		return 0;
	
	
	u32 pd_idx = PAGEDIR_INDEX(page);
	u32 pt_idx = PAGETABLE_INDEX(page);
	
	if (!page_entry_is_present(current_pagedir, pd_idx)) {
		u32 new_frame = alloc_frame();
		if (new_frame == 0)
			return 0;
		write_page_entry(current_pagedir, pd_idx, new_frame, user);
	}

	u32 *pagetable = (u32 *) PAGETABLE_VADDR(pd_idx);
	
	write_page_entry(pagetable, pt_idx, frame, user);	

	return page;
}

void
unmap(u32 page)
{
	u32 *pagetable = (u32 *) PAGETABLE_VADDR(PAGEDIR_INDEX(page));
	clear_page_entry(pagetable, PAGETABLE_INDEX(page));
	
	if (pagetable_is_empty(pagetable)) {
		free_frame(PAGE_ENTRY_BASE(current_pagedir[PAGEDIR_INDEX(page)]));
		clear_page_entry(current_pagedir, PAGEDIR_INDEX(page));
	}
	
	invlpg(page);
}

u32
alloc_kpage()
{
	u32 frame = alloc_frame();
	
	return map(frame, kinfo.vbase, 0);
}

void
free_page(u32 page)
{
	u32 *pagetable = (u32 *) PAGETABLE_VADDR(PAGEDIR_INDEX(page));
	
	u32 frame = PAGE_ENTRY_BASE(pagetable[PAGETABLE_INDEX(page)]);
	free_frame(frame);
	
	unmap(page);
}

u32
alloc_page(u32 offset, u32 user)
{
	u32 frame = alloc_frame();
	
	return map(frame, offset, user);
}

void
init_paging()
{
	u32 total_frame_count = updiv(kinfo.memlen, PAGE_SIZE);
	bitmap_size = updiv(total_frame_count, 8);	
	u32 used_frame_count = updiv(kinfo.len + bitmap_size, PAGE_SIZE);
	bitmap = (u8 *) kinfo.len + kinfo.vbase;
	
	u32 pagetable_addr = ((u32) bitmap) + bitmap_size;
	align(pagetable_addr, PAGE_SIZE);
	memset((u32 *) pagetable_addr, 0, PAGE_SIZE);
	
	used_frame_count++;
	
	write_page_entry(kernel_pagedir, PAGEDIR_INDEX(kinfo.vbase),
					 pagetable_addr - kinfo.vbase, 0);

	for (u32 i = 0; i < used_frame_count; i++) {		
		bitmap[i / 8] |= 1 << (i % 8);
	}
	
	for (u32 i = 0; i < used_frame_count - 1; i++) {
		u32 page = kinfo.vbase + i * PAGE_SIZE;
		u32 frame = i * PAGE_SIZE;
		u32 pt_idx = PAGETABLE_INDEX(page);
		write_page_entry((u32 *) pagetable_addr, pt_idx, frame, 0);			
	}
		
	write_page_entry(kernel_pagedir, 1023,
					 (u32)kernel_pagedir - kinfo.vbase, 0);
	
	switch_page_dir(((u32)kernel_pagedir) - kinfo.vbase);
}

void
switch_page_dir(u32 dir)
{
	extern void __switch_pagedir(u32 paddr);
    __switch_pagedir(dir);
}

u32
create_user_pagedir()
{
	u32 pagedir_frame = alloc_frame();	
	u32 *pagedir = (u32 *) map(pagedir_frame, kinfo.vbase, 0);
	
	memset(pagedir, 0, 4096);
	memcpy(&pagedir[PAGEDIR_INDEX(kinfo.vbase)],
			&current_pagedir[PAGEDIR_INDEX(kinfo.vbase)],
			(1024 - PAGEDIR_INDEX(kinfo.vbase)) * 4);
	
	write_page_entry(pagedir, 1023, pagedir_frame, 0);
	
	unmap((u32) pagedir);
		
	return pagedir_frame;
}

u32
copy_current_pagedir()
{
	u32 new_pagedir_phys = create_user_pagedir();
	u32 old_pagedir_phys = phys_addr(current_pagedir, (u32) current_pagedir);

	switch_page_dir(new_pagedir_phys);
	u32 *old_pagedir = (u32 *) map(old_pagedir_phys, PAGE_SIZE, 0);
	
	for (u32 i = 0; i < PAGEDIR_INDEX(kinfo.vbase); i++) {
		if (page_entry_is_present(old_pagedir, i)) {
			u32 *pagetable = (u32 *) map(PAGE_ENTRY_BASE(old_pagedir[i]), PAGE_SIZE, 0);
			
			for (u32 j = 0; j < 1024; j++) {
				if (page_entry_is_present(pagetable, j)) {
					u32 base = PAGE_ENTRY_BASE(pagetable[j]);
					
					if (base != (u32)old_pagedir && base != (u32)pagetable) {						
						u32 new_page = alloc_page((i << 22) + (j << 12), 1);
						u32 old_page = map(base, PAGE_SIZE, 0);
						
						memcpy((u32 *) new_page, (u32 *) old_page, PAGE_SIZE);
						
						unmap(old_page);
					}
				}
			}
			unmap((u32) pagetable);
		}
	}
	
	unmap((u32) old_pagedir);
	
	return new_pagedir_phys;
}
