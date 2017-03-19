#ifndef PAGING_H
#define PAGING_H

#include <types.h>
#include <alien/kernel.h>

#define align(a, b) \
    if (a % b != 0) \
        a += b - a % b;

#define updiv(a, b) \
    (((a) + (b) - 1) / (b))

#define TEMP_PAGE_PT_INDEX      1023
#define TEMP_PAGE_VADDR         ((PD_INDEX(kinfo.vbase) << 22) + (1023 << 12))
#define TEMP_PAGE_PADDR         TEMP_PAGE_VADDR - kinfo.vbase

#define PAGE_SIZE 4096

#define PE_PRESENT  1
#define PE_RW       2
#define PE_USER     4

#define PD_INDEX(virt) (((virt) & 0xFFC00000) >> 22)
#define PT_INDEX(virt) (((virt) & 0x003FF000) >> 12)

struct _page_entry
{
    u8 flags;
    u8 zero : 4;
    u32 base : 20;
} __attribute__((packed));

typedef struct _page_entry page_entry_t;

struct table
{
    page_entry_t entries[1024];
} __attribute__((packed));

struct table __attribute__ ((aligned (4096))) kpd;
typedef u32 vpage_t;
typedef u32 ppage_t;
typedef u32 page_t; /* 4096 bytes page (physical address) */


void            init_paging             ();
void            switch_page_dir         (struct table* dir);
void            *alloc_kpage            ();
struct table    *create_user_pagedir    ();
int             paging_user_alloc_page  (struct table *dir, u32 adress);

#endif
