#include <kernel/paging.h>
#include <kernel/kernel.h>
#include <kernel/io.h>
#include <kernel/core/mm.h>

#define PAGE_SIZE 4096

static int
map_page(struct pd* dir, u32 frame, u32 virt, u8 flags);

/**
 * Identity map [0x0; kernel_end] -> [0xC0000000; 0xC0000000 + kernel_end]
 */
/*void
init_paging(u32 kernel_len)
{
    u32 total_frame_count = kernel_info.mem_len / PAGE_SIZE;
    if (total_frame_count * PAGE_SIZE < kernel_info.mem_len)
        total_frame_count++;

    kpd = (struct pd*) vaddr(alloc_blocks(1));

    u32 kernel_frame_count = kernel_len / PAGE_SIZE;
    if (kernel_frame_count * PAGE_SIZE < kernel_len)
        kernel_frame_count++;

    u32 total_pt_count = (total_frame_count) / 1024;
    if (total_pt_count * 1024 < total_frame_count)
        total_pt_count++;

    for (u32 i = 0; i < kernel_frame_count + total_pt_count + 1; i++) {
        map_page(kpd, i * 4096, i * 4096 + kernel_info.kernel_vbase, PE_PRESENT | PE_RW);
    }

    switch_page_dir(kpd);
}
*/

#define align(a, b) \
    if (a % b != 0) \
        a += b - a % b;

#define updiv(a, b) \
    (((a) + (b) - 1) / (b))

struct kernel_heap_info {
    u32 frame_count;
    u32 bitmap_size;
    u32 base;
    u8 *bitmap;
};

struct kernel_heap_info heap_info;

static inline int
kheap_is_free(vaddr_t addr)
{
    addr = (addr - kernel_info.kernel_vbase) >> 12;
    return heap_info.bitmap[(addr)/8] & (addr % 8);
}

static inline void
kheap_setused(vaddr_t addr)
{
    addr = (addr - kernel_info.kernel_vbase) >> 12;
    heap_info.bitmap[(addr)/8] &= (addr % 8);
}

static inline void
kheap_free(vaddr_t addr)
{
    addr = (addr - kernel_info.kernel_vbase) >> 12;
    heap_info.bitmap[(addr)/8] ^= ~(addr % 8);
}

static u32
kheap_alloc()
{
    for (u32 i = 0; i < heap_info.bitmap_size; i++) {
        if (heap_info.bitmap[i] != 0xFF) {
            for (u8 j = 0; j < 8; j++) {
                if ((heap_info.bitmap[i] & (1 << j)) == 0) {
                    heap_info.bitmap[i] |= (1 << j);
                    return (u32) heap_info.base + ((j + i * 8) * PAGE_SIZE);
                }
            }
        }
    }

    return 0;
}

static struct pd __attribute__ ((aligned (4096))) kpd;
static struct pd* current_pd;

void
init_paging()
{
    u32 base;
    u32 kernel_pt_count;
    u32 kernel_frame_count;

    /* Initialize bitmap */
    heap_info.frame_count = updiv(kernel_info.mem_len - kernel_info.kernel_end,
                                  PAGE_SIZE);
    /* Keep some space for the bitmap */
    heap_info.frame_count -= updiv(updiv(heap_info.frame_count, 8), PAGE_SIZE);
    heap_info.bitmap_size = updiv(heap_info.frame_count, 8);

    heap_info.bitmap = (u8*) (kernel_info.kernel_end + kernel_info.kernel_vbase);
    heap_info.base = (u32) heap_info.bitmap + heap_info.bitmap_size * sizeof(u8);

    align(heap_info.base, PAGE_SIZE);

    /* Initialize kernel pagedir */

    kernel_pt_count = updiv((heap_info.base - kernel_info.kernel_vbase), (PAGE_SIZE * 1024));
    kernel_frame_count = updiv((heap_info.base - kernel_info.kernel_vbase), PAGE_SIZE);
    kernel_frame_count += kernel_pt_count;

    for (u32 i = PD_INDEX(kernel_info.kernel_vbase);
         i < PD_INDEX(kernel_info.kernel_vbase) + kernel_pt_count;
         i++) {
        if ((base = kheap_alloc()) == 0) {
            panic("kheap_alloc error\n");
        }

        kpd.entries[i].base = (base - kernel_info.kernel_vbase) >> 12;
        kpd.entries[i].flags = PE_PRESENT | PE_RW;
    }

    for (u32 i = 0; i < kernel_frame_count; i++) {
        if (map_page(&kpd, i * PAGE_SIZE, i * PAGE_SIZE + kernel_info.kernel_vbase,
                 PE_PRESENT | PE_RW) < 0) {
            panic("map_page error\n");
        }
    }

    switch_page_dir(&kpd);
}

u32
alloc_page()
{
    u32 frame;

    if ((frame = kheap_alloc()) == 0) {
        return 0;
    }
    if (map_page(&kpd, frame - kernel_info.kernel_vbase, frame, PE_PRESENT | PE_RW)) {
        return 0;
    }
    return frame;
}

u32
create_user_pd(struct pd* dir, u32 size)
{
    u32 table_base;
    u32 frame_count;
    u32 pt_count;
    u32 first_frame;
    u32 frame;

    if ((dir = (struct pd*) alloc_page()) == 0) {
        panic("alloc_page error in create_user_pd\n");
    }

    frame_count = updiv(size, PAGE_SIZE);
    pt_count = updiv(frame_count, 1024);

    for (u32 i = 0; i < pt_count; i++) {
        if ((table_base = alloc_page()) < 0) {
            panic("alloc_page error in create_user_pd\n");
        }

        dir->entries[i].base = (table_base - kernel_info.kernel_vbase) >> 12;
        dir->entries[i].flags = PE_PRESENT | PE_RW | PE_USER;
    }

    frame = alloc_page();
    first_frame = frame;
    for (u32 i = 0; i < frame_count; i++) {
        map_page(dir, frame - kernel_info.kernel_vbase, i * 4096,
                 PE_PRESENT | PE_RW | PE_USER);
        frame = alloc_page();
    }

    return first_frame;
}

void
switch_page_dir(struct pd* dir)
{
    extern void __switch_page_dir(paddr_t addr);

    __switch_page_dir(paddr((vaddr_t) dir));

    current_pd = dir;
}

/**
 * return -1 if page table not present
 */
static int
map_page(struct pd* dir, u32 frame, u32 virt, u8 flags)
{
    struct pt* table;

    if ((dir->entries[PD_INDEX(virt)].flags & PE_PRESENT) == 0)
        return -1;

    table = (struct pt*) ((dir->entries[PD_INDEX(virt)].base << 12)
            + kernel_info.kernel_vbase);

    table->entries[PT_INDEX(virt)].flags = flags;
    table->entries[PT_INDEX(virt)].base = (frame & 0xFFFFF000) >> 12;

    return 0;
}

/**
 * Allocate a page in the kernel space (ie. above KERNEL_OFFSET)
 * @return virtual adress of the page or zero if not found
 */
/*vaddr_t
kalloc_page()
{
    struct pt *table;
    u32 virt;

    for (u32 i = PD_INDEX(KERNEL_OFFSET); i < 1024; i++) {
        if (kpd->entries[i].base != 0) {
            table = (struct pt*) vaddr(kpd->entries[i].base << 12);
            for (u32 j = 0; j < 1024; j++) {
                if ((table->entries[j].flags & PE_PRESENT) == 0) {
                    virt = (i << 22) + (j << 12);
                    map_page(kpd, virt - KERNEL_OFFSET, virt, PE_PRESENT | PE_RW);
                    return virt;
                }
            }
        }
    }

    return 0;
}*/
