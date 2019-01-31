/*******************************************************************************
 * SOURCE NAME  : paging.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provides functions for managing pages
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <kernel/memory/paging.h>
#include <kernel/kernel.h>

#define align(a, b)     \
    if (a % b != 0)     \
        a += b - a % b; \

#define updiv(a, b)     \
(((a) + (b) - 1) / (b)) \


#define LAST_PAGE (uint32_t)PAGE_SIZE * 1023 * 1024


/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

static uint8_t *bitmap;
static uint32_t bitmap_size;

static uint32_t kernel_dir[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t kernel_tab[1024] __attribute__((aligned(PAGE_SIZE)));



/*******************************************************************************
 *                          PRIVATE FUNCTIONS
 ******************************************************************************/

__attribute__((always_inline))
static inline void invlpg(uint32_t page)
{
    asm volatile ("invlpg (%0)" :: "r" (page) : "memory");
}

__attribute__((always_inline))
static inline void clear_entry(uint32_t *table, uint16_t i)
{
    table[i] = 0;
}

__attribute__((always_inline))
static inline uint8_t entry_is_present(uint32_t *table, uint16_t i)
{
    return table[i] & 1;
}

__attribute__((always_inline))
static inline uint32_t *get_pagedir()
{
    return (uint32_t *) 0xFFFFF000;
}

__attribute__((always_inline))
static inline uint32_t *get_pagetable(uint32_t i)
{
    return (uint32_t *) ((1023 << 22) + (i << 12));
}

__attribute__((always_inline))
static inline uint32_t get_pagedir_index(uint32_t page)
{
    return (page & 0xFFC00000) >> 22;
}

__attribute__((always_inline))
static inline uint32_t get_pagetab_index(uint32_t page)
{
    return (page & 0x003FF000) >> 12;
}

__attribute__((always_inline))
static inline uint32_t get_entry_frame(uint32_t entry)
{
    return entry & 0xFFFFF000;
}

static void write_entry(uint32_t *table, uint16_t i, uint32_t frame,
                        uint8_t is_user)
{
    table[i] = (frame & 0xFFFFF000);
    table[i] |= 1;
    table[i] |= 2; /* R/W */
    table[i] |= ((is_user & 0x1) << 2);
}

__attribute__((always_inline))
static uint8_t page_is_used(uint32_t page)
{
    uint32_t *dir = get_pagedir();
    uint32_t dir_idx = get_pagedir_index(page);
    uint32_t tab_idx = get_pagetab_index(page);
    
    if (entry_is_present(dir, dir_idx)) {
        uint32_t *tab = get_pagetable(dir_idx);
        if (entry_is_present(tab, tab_idx)) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

/**
 * @brief Search the first free page in the current adress space.
 * @param after Adress at wich start research
 * @return 0 if there is no more page free
 */
__attribute__((always_inline))
static inline uint32_t first_page_free(uint32_t after)
{
    for (uint32_t page = after; page <= LAST_PAGE; page += PAGE_SIZE) {
        if (!page_is_used(page)) return page;
    }

    printf("first_page_free() ERROR\n");
    return 0;
}

static uint32_t alloc_frame()
{
    uint32_t i = 0;
    uint32_t j = 0;

    while (bitmap[i] == 0xFF) {
        i++;
        if (i == bitmap_size)
            return 0;
    }

    while (bitmap[i] & (1 << j)) {
        j++;
        if (j == 8) {
            return 0;
        }
    }
    
    bitmap[i] |= (1 << j);

    return (i * 8 + j) * PAGE_SIZE;
}

static void free_frame(uint32_t frame)
{
    uint32_t i = frame / (PAGE_SIZE * 8);
    uint32_t j = (frame / PAGE_SIZE) % 8;

    bitmap[i] &= ~(1 << j);
}

static uint8_t pagetable_is_empty(uint32_t *table)
{
    for (uint32_t i = 0; i < 1024; i++) {
        if (entry_is_present(table, i)) {
            return 0;
        }
    }

    return 1;
}



/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

uint32_t phys_addr(uint32_t page)
{
    uint32_t *dir = get_pagedir();
    uint32_t dir_idx = get_pagedir_index(page);
    uint32_t tab_idx = get_pagetab_index(page);
    
    if (!entry_is_present(dir, dir_idx)) return 0;

    uint32_t *table = get_pagetable(dir_idx);

    return get_entry_frame(table[tab_idx]);
}

uint32_t map(uint32_t frame, uint32_t offset, uint32_t user)
{
    if (frame == 0) return 0;

    uint32_t page = first_page_free(offset);
    if (page == 0) return 0;

    uint32_t *dir = get_pagedir();
    uint32_t pd_idx = get_pagedir_index(page);
    uint32_t pt_idx = get_pagetab_index(page);
    
    if (!entry_is_present(dir, pd_idx)) {
        uint32_t new_frame = alloc_frame();
        if (new_frame == 0) return 0;
        write_entry(dir, pd_idx, new_frame, 0);
    }
    
    uint32_t *pagetable = get_pagetable(pd_idx);
    write_entry(pagetable, pt_idx, frame, user);

    return page;
}

void unmap(uint32_t page)
{
    uint32_t *dir = get_pagedir();
    uint32_t dir_idx = get_pagedir_index(page);
    uint32_t tab_idx = get_pagetab_index(page);
    
    uint32_t *table = get_pagetable(dir_idx);
    
    clear_entry(table, tab_idx);

    if (pagetable_is_empty(table)) {
        free_frame(get_entry_frame(dir[dir_idx]));
        clear_entry(dir, dir_idx);
    }

    invlpg(page);
}

uint32_t alloc_page(uint32_t offset, uint32_t is_user)
{
    uint32_t frame = alloc_frame();
    return map(frame, offset, is_user);
}

uint32_t alloc_kpage()
{
    return alloc_page(KERNEL_VBASE, 0);
}

void free_page(uint32_t page)
{
    free_frame(phys_addr(page));
    unmap(page);
}

void paging_install(uint32_t total_mem, uint32_t used_mem)
{
    /* Initialize the bitmap */
    uint32_t total_frame_count = updiv(total_mem, PAGE_SIZE);
    bitmap = (uint8_t *) used_mem + KERNEL_VBASE;
    bitmap_size = updiv(total_frame_count, 8);
    memset(bitmap, 0, bitmap_size);
    
    used_mem += bitmap_size;    
    uint32_t used_frame_count = updiv(used_mem, PAGE_SIZE);

    
    /* Fill the bitmap */
    for (uint32_t i = 0; i < used_frame_count; i++) {
        bitmap[i / 8] |= 1 << (i % 8);
    }


    /* Map the first page table */
    uint32_t table_addr = (uint32_t) kernel_tab;
    write_entry(kernel_dir, get_pagedir_index(KERNEL_VBASE),
                table_addr - KERNEL_VBASE, 0);
    
    
    /* Fill the first page table */
    for (uint32_t i = 0; i < used_frame_count; i++) {
        uint32_t frame = i * PAGE_SIZE;
        uint32_t page = frame + KERNEL_VBASE;
        uint32_t pt_idx = get_pagetab_index(page);
        write_entry(kernel_tab, pt_idx, frame, 0);
    }
    
    
    /* Recursively map the last page dir entry to the page dir */
    write_entry(kernel_dir, 1023, (uint32_t)kernel_dir - KERNEL_VBASE, 0);

    switch_page_dir((uint32_t)kernel_dir - KERNEL_VBASE);
}

void switch_page_dir(uint32_t dir)
{
    extern void __switch_pagedir(uint32_t paddr);
    __switch_pagedir(dir);
}
