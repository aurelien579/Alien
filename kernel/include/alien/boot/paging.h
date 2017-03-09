#ifndef PAGING_H
#define PAGING_H

#include <types.h>
#include <alien/kernel.h>

#define PAGE_SIZE 4096

#define PE_PRESENT  1
#define PE_RW       2
#define PE_USER     4

#define PD_INDEX(virt) (((virt) & 0xFFC00000) >> 22)
#define PT_INDEX(virt) (((virt) & 0x003FF000) >> 12)

struct page_entry
{
    u8 flags;
    u8 zero : 4;
    u32 base : 20;
} __attribute__((packed));

struct table
{
    struct page_entry entries[1024];
} __attribute__((packed));

struct table __attribute__ ((aligned (4096))) kpd;

typedef u32 page_t; /* 4096 bytes page (physical address) */

void init_paging();
void switch_page_dir(struct table* dir);
void* alloc_kpage();
int user_pd_map(struct table *dir, page_t page, u32 vaddr);
struct table *create_user_pagedir();

#endif
