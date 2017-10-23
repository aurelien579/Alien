#include <alien/memory/paging.h>
#include <alien/kernel.h>
#include <alien/io.h>
#include <alien/string.h>
#include "physman.h"
#include "page_entry.h"

#define KERNEL_PD_INDEX         PD_INDEX(kinfo.vbase)

#define NO_PAGE_LEFT 1
#define PT_NOTPRESENT -154

struct _memman {
    struct table   *pagedir;
    physman_t      *physman;
};

typedef struct _memman memman_t;

typedef struct table page_table_t;
static page_table_t* current_pd;
static physman_t *physman;

/**
 * Write in the paging structures the mapping of the page to the vaddr. The page
 * table corresponding to the vaddr must exists and be mapped. Will crash the
 * kernel if not mapped !!
 */
static void
__do_kernel_mapping(ppage_t paddr, vpage_t vaddr)
{
    page_entry_t *e = &kpd.entries[PD_INDEX(vaddr)];
    page_table_t *t = (page_table_t*) (page_entry_get_base(e) + kinfo.vbase);
    t->entries[PT_INDEX(vaddr)] = page_entry_new(paddr, PAGE_ENTRY_RW);
}


/**
 * Initialize a new page table in the kernel's page directory. When called, all
 * accessible vaddrs can be used : it actually use the reserved temp page in order
 * to do the mapping.
 * First, map the newly reserved page on the temp vaddr then write to the page
 * table. Add an entry to map the reserved page on itself.
 * Then, create the necessary entry in the kernel page directory.
 */
static int
_init_kernel_page_table(ppage_t page)
{
    u32 pd_index = PD_INDEX(page + kinfo.vbase);
    vpage_t vaddr;
    ppage_t new_page;
    page_table_t *t;

    if ((new_page = physman_allocate_page(physman)) == NO_PAGE_LEFT)
        return -1; /* Not enought memory */
    
    vaddr = new_page + kinfo.vbase;
    __do_kernel_mapping(new_page, TEMP_PAGE_VADDR);

    t = (struct table*) TEMP_PAGE_VADDR;

    memset(t, 0, 4096);

    kprintf("init : %d, page : 0x%x\n", pd_index, new_page);
    kpd.entries[PD_INDEX(vaddr)] = page_entry_new(new_page, PAGE_ENTRY_RW);
    t->entries[PT_INDEX(vaddr)] = page_entry_new(new_page, PAGE_ENTRY_RW);

    return 0;
}

static int
_map_kernel_page(page_t page)
{
    vaddr_t vaddr = page + kinfo.vbase;

    /* Check if asking mapping on the temp page */
    if (PD_INDEX(vaddr) == KERNEL_PD_INDEX &&
        PT_INDEX(vaddr) == TEMP_PAGE_PT_INDEX)
    {
        return -1;
    }

    /* Handle missing page table */
    if (!page_entry_is_present(&kpd.entries[PD_INDEX(vaddr)]))
        return PT_NOTPRESENT;

    __do_kernel_mapping(page, vaddr);

    return 0;
}

static inline u8
_pt_present(page_t page)
{
    return page_entry_is_present(&kpd.entries[PD_INDEX(page + kinfo.vbase)]);
}

void*
alloc_kpage()
{
    page_t page = physman_allocate_page(physman);
    if (page == 0)
        return (void*) 0;

    if (!_pt_present(page)) {
        if (_init_kernel_page_table(page) < 0) {
            return (void*) 0;
        }
    }

    if (_map_kernel_page(page) < 0)
        kprintf("map_kernel_page error\n");

    return (void*) (page + kinfo.vbase);
}

void
init_paging()
{
    physman = physman_init(&kinfo);
    
    memset(&kpd, 0, 4096);
    
    struct table *kpt = (struct table*)
                            (physman_allocate_page(physman) + kinfo.vbase);

    memset(kpt, 0, 4096);
    kpd.entries[KERNEL_PD_INDEX] = page_entry_new(((u32)kpt) - kinfo.vbase, PAGE_ENTRY_RW);
    
    unsigned int page_used_count = physman_used_page_count(physman);
    for (u32 i = 0; i < page_used_count; i++) {
        _map_kernel_page(i * 4096);
    }

    switch_page_dir(&kpd);
}

static memman_t memman;

memman_t*
memman_init(const kernel_info_t* info)
{
    page_table_t   *pt;
    unsigned int    page_used_count;
    ppage_t         page;
    
    memman.physman = physman_init(info);
    memset(&memman.pagedir, 0, sizeof(page_table_t));
    
    if ((page = physman_allocate_page(memman.physman)) == 0) {
        kprintf("physman_allocate_page error : %d\n",
                                        physman_errno(memman.physman));
        panic("");
    }
    
    pt = (page_table_t*) (page + kinfo.vbase);
    memset(pt, 0, sizeof(page_table_t));
    page_used_count = physman_used_page_count(memman.physman);
    
    for (u32 i = 0; i < page_used_count; i++) {
        _map_kernel_page(i * 4096);
    }
    
    return &memman;
}

void
switch_page_dir(struct table* dir)
{
    extern void __switch_pagedir(u32 paddr);

    __switch_pagedir(((u32)dir) - kinfo.vbase);

    current_pd = dir;
}

static int
_user_pd_map(struct table *dir, page_t page, u32 vaddr)
{
    struct table *pt;

    if (vaddr >= kinfo.vbase) {
        return -1;
    }

    if (!page_entry_is_present(&dir->entries[PD_INDEX(vaddr)])) {
        if ((pt = (struct table*) alloc_kpage()) == 0) {
            return -1;
        }
        dir->entries[PD_INDEX(vaddr)] = page_entry_new(((u32)pt) - kinfo.vbase,
                                        PAGE_ENTRY_RW | PAGE_ENTRY_USER);
    }

    pt = (struct table*) (page_entry_get_base(&dir->entries[PD_INDEX(vaddr)]) + kinfo.vbase);
    pt->entries[PT_INDEX(vaddr)] = page_entry_new(page, PAGE_ENTRY_RW | PAGE_ENTRY_USER);

    return 1;
}

struct table*
create_user_pagedir()
{
    struct table* dir;

    if ((dir = alloc_kpage()) == 0) {
        return (struct table*) 0;
    }

    memcpy(dir, &kpd, 1024 * sizeof(page_entry_t));

    for (int i = 0; i < 1023; i++)
        _user_pd_map(dir, PAGE_SIZE * i, PAGE_SIZE * i);
    
    if (paging_user_alloc_page(dir, 0x100000) < 0)
        return (struct table*) 0;
    
    if (paging_user_alloc_page(dir, kinfo.vbase - PAGE_SIZE) < 0)
        return (struct table*) 0;    

    return dir;
}

int
paging_user_alloc_page(struct table *dir, u32 adress)
{
    page_t page;
    if ((page = physman_allocate_page(physman)) == 0 )
        return -1;
    _user_pd_map(dir, page, adress);
    
    return 0;
}
