#include <alien/boot/paging.h>
#include <alien/kernel.h>
#include <alien/io.h>
#include <alien/string.h>

#define align(a, b) \
    if (a % b != 0) \
        a += b - a % b;

#define updiv(a, b) \
    (((a) + (b) - 1) / (b))

#define MAX_PAGE_COUNT          (512*1024*1024 / 4096)
#define KERNEL_PD_INDEX         PD_INDEX(kinfo.vbase)

#define TEMP_PAGE_PT_INDEX      1023
#define TEMP_PAGE_VADDR         ((PD_INDEX(kinfo.vbase) << 22) + (1023 << 12))
#define TEMP_PAGE_PADDR         TEMP_PAGE_VADDR - kinfo.vbase

#define NO_PAGE_LEFT 1
#define PT_NOTPRESENT -154

static struct table* current_pd;

static u8 *_bitmap;
static u32 _last_page_used;
static u32 _bitmap_size;
static u32 _total_page_count;
static u32 _kernel_page_count;
static u32 _kernel_entry_flags = PE_PRESENT | PE_RW;


static inline void
_entry_set_base(struct page_entry *entry, page_t base)
{
    entry->base = ((base) >> 12);
}

static inline page_t
_entry_get_base(const struct page_entry *entry)
{
    return (entry->base << 12);
}

static inline u32
_entry_get_vbase(const struct page_entry *entry)
{
    return _entry_get_base(entry) + kinfo.vbase;
}

static inline u8
_entry_is_present(struct page_entry *entry)
{
    return (entry->flags & PE_PRESENT) == 1;
}

static inline void
_create_entry(struct page_entry *e, u32 base, u8 flags)
{
    _entry_set_base(e, base);
    e->flags = flags;
}

static inline u8
_is_page_used(page_t page)
{
    /* Special cases */
    if (page == TEMP_PAGE_PADDR)
        return 1;

    u32 page_nbr = page / PAGE_SIZE;
    return (_bitmap[page_nbr / 8] & (1 << (page_nbr % 8))) != 0;
}

static inline void
_set_page_used(page_t page)
{
    u32 page_nbr = page / PAGE_SIZE;

    _bitmap[page_nbr / 8] |= (1 << (page_nbr % 8));

    if (page > _last_page_used) {
        _last_page_used = page;
    }
}


/**
 * Reserve a page on the bitmap. Do not map the page on any page dir ! Don't use
 * the returned page as it, you must map it first !
 */
page_t
_get_page()
{
    page_t page = 0;

    int i = 0;
    while(_bitmap[i] == 0xFF) {
        page += (8 * PAGE_SIZE);
        i++;
    }

    while (_is_page_used(page) == 1) {
        page += PAGE_SIZE;
    }

    _set_page_used(page);
    return page;
}

/**
 * Write in the paging structures the mapping of the page to the vaddr. The page
 * table corresponding to the vaddr must exists and be mapped. Will crash the
 * kernel if not mapped !!
 */
static void
__do_kernel_mapping(page_t page, u32 vaddr)
{
    struct table* t = (struct table*)
                        _entry_get_vbase(&kpd.entries[PD_INDEX(vaddr)]);
    _create_entry(&t->entries[PT_INDEX(vaddr)], page, _kernel_entry_flags);
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
_init_kernel_page_table(page_t page)
{
    u32 pd_index = PD_INDEX(page + kinfo.vbase);
    page_t new_page;
    struct table *t;

    if ((new_page = _get_page()) == NO_PAGE_LEFT)
        return -1; /* Not enought memory */

    __do_kernel_mapping(new_page, TEMP_PAGE_VADDR);

    t = (struct table*) TEMP_PAGE_VADDR;

    memset(t, 0, 4096);

    kprintf("init : %d, page : 0x%x\n", pd_index, new_page);
    _create_entry(&kpd.entries[PD_INDEX(new_page + kinfo.vbase)], new_page,
                    _kernel_entry_flags);

    _create_entry(&t->entries[PT_INDEX(new_page + kinfo.vbase)], new_page,
                    _kernel_entry_flags);

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
    if (!_entry_is_present(&kpd.entries[PD_INDEX(vaddr)]))
        return PT_NOTPRESENT;

    __do_kernel_mapping(page, vaddr);

    return 0;
}

static inline u8
_pt_present(page_t page)
{
    return _entry_is_present(&kpd.entries[PD_INDEX(page + kinfo.vbase)]);
}

void*
alloc_kpage()
{
    page_t page = _get_page();
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
    page_t page_buffer[MAX_PAGE_COUNT];
    _total_page_count = updiv(kinfo.memlen, PAGE_SIZE);
    _bitmap_size = updiv(_total_page_count, 8);

    kinfo.len += _bitmap_size * sizeof(u8); /* 'Statically' alloc space in the kernel */
    _bitmap = (u8*) kinfo.vbase + kinfo.len;

    _kernel_page_count = updiv(kinfo.len, PAGE_SIZE);

    if (_kernel_page_count > MAX_PAGE_COUNT)
        panic("too large kernel...\n");

    for (u32 i = 0; i < _kernel_page_count + 1; i++) {
        page_buffer[i] = _get_page();
    }

    memset(&kpd, 0, 4096);

    struct table *kpt = (struct table*)
                            (_kernel_page_count * PAGE_SIZE + kinfo.vbase);

    memset(kpt, 0, 4096);

    _create_entry(&kpd.entries[KERNEL_PD_INDEX],
                  _kernel_page_count * PAGE_SIZE,
                  _kernel_entry_flags);

    for (u32 i = 0; i < _kernel_page_count + 1; i++) {
        _map_kernel_page(page_buffer[i]);
    }

    switch_page_dir(&kpd);
}

void
switch_page_dir(struct table* dir)
{
    extern void __switch_page_dir(u32 paddr);

    __switch_page_dir(((u32)dir) - kinfo.vbase);

    current_pd = dir;
}

struct table*
create_user_pagedir()
{
    struct table* dir;

    if ((dir = alloc_kpage()) == 0) {
        return 0;
    }

    memcpy(dir, &kpd, 1024 * sizeof(struct page_entry));

    return dir;
}

int
user_pd_map(struct table *dir, page_t page, u32 vaddr)
{
    struct table *pt;

    if (vaddr >= kinfo.vbase) {
        return -1;
    }

    if (!_entry_is_present(&dir->entries[PD_INDEX(vaddr)])) {
        if ((pt = (struct table*) alloc_kpage()) == 0) {
            return -1;
        }
        _create_entry(&dir->entries[PD_INDEX(vaddr)],
                      ((u32)pt) - kinfo.vbase,
                      PE_PRESENT | PE_RW | PE_USER);
    }

    pt = (struct table*) _entry_get_vbase(&dir->entries[PD_INDEX(vaddr)]);
    _create_entry(&pt->entries[PT_INDEX(vaddr)], page, PE_PRESENT | PE_RW | PE_USER);

    return 1;
}

void
user_id_map(struct table* dir)
{
    for (int i = 0; i < 1023; i++)
        user_pd_map(dir, PAGE_SIZE * i, PAGE_SIZE * i);
}
