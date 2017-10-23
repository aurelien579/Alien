#include "physman.h"
#include <alien/string.h>

struct _physman {
    u8 *bitmap;
    u32 bitmap_size;
    u32 page_count;
    u32 used_page_count;
    errorno_t errno;
};

static physman_t physman;

physman_t*
physman_init(const kernel_info_t* info)
{
    physman.used_page_count = 0;
    physman.page_count      = updiv (info->memlen, PAGE_SIZE);
    physman.bitmap_size     = updiv (physman.page_count, 8);
    physman.bitmap          = (u8*) info->vbase + info->len;
    int kernel_page_count   = updiv (info->len + physman.bitmap_size, PAGE_SIZE);
    
    memset (physman.bitmap, 0, physman.bitmap_size * sizeof(u8));

    for (int i = 0; i < kernel_page_count; i++)
        physman_allocate_page(&physman);
    
    return &physman;
}

errno_t
physman_errno(const physman_t *m)
{
    return m->errno;
}

unsigned int
physman_used_page_count(const physman_t *m)
{
    return m->used_page_count;
}

bool
physman_is_page_used(const physman_t* m, ppage_t page)
{
    /* Special cases */
    if (page == TEMP_PAGE_PADDR)
        return true;

    u32 page_nbr = page / PAGE_SIZE;
    return (m->bitmap[page_nbr / 8] & (1 << (page_nbr % 8))) != 0;
}

static void
physman_set_page_used(physman_t* m, ppage_t page)
{
    u32 page_nbr = page / PAGE_SIZE;

    m->bitmap[page_nbr / 8] |= (1 << (page_nbr % 8));
    
    m->used_page_count++;
}


/**
 * Reserve a page on the bitmap. Do not map the page on any page dir ! Don't use
 * the returned page as it, you must map it first !
 * @return 0 if error occured
 */
ppage_t
physman_allocate_page(physman_t* m)
{
    ppage_t page = 0;
    int i = 0;
    
    if (m->used_page_count == m->page_count) {
        m->errno = physman_out_of_memory;
        return 0;
    }
    
    while(m->bitmap[i] == 0xFF) {
        page += (8 * PAGE_SIZE);
        i++;
    }

    while (physman_is_page_used(m, page) == 1) {
        page += PAGE_SIZE;
    }

    physman_set_page_used(m, page);
    return page;
}

void
physman_deallocate_page(physman_t* m, ppage_t page)
{
	
}
