#include <kernel/paging.h>
#include <kernel/kernel.h>
#include <kernel/io.h>
#include <kernel/core/mm.h>

#define PAGE_SIZE 4096
#define KERNEL_OFFSET 0xC0000000

static void
map_page(struct pd* dir, u32 page, u32 virt, u8 flags);

/**
 * Identity map [0x0; kernel_end] -> [0xC0000000; 0xC0000000 + kernel_end]
 */
void
init_paging(u32 kernel_len)
{
    /* Pages required to map the entire memory */
    u32 total_frame_count = kernel_info.mem_len / PAGE_SIZE;
    if (total_frame_count * PAGE_SIZE < kernel_info.mem_len)
        total_frame_count++;

    kpd = (struct pd*) vaddr(alloc_blocks(1));

    /* Calculate the required pages and page tables for mapping the kernel */
    u32 kernel_frame_count = kernel_len / PAGE_SIZE;
    if (kernel_frame_count * PAGE_SIZE < kernel_len)
        kernel_frame_count++;

    u32 total_pt_count = (total_frame_count) / 1024;
    if (total_pt_count * 1024 < total_frame_count)
        total_pt_count++;

    for (u32 i = 0; i < kernel_frame_count + total_pt_count + 1; i++) {
        map_page(kpd, i * 4096, i * 4096 + KERNEL_OFFSET, PE_PRESENT | PE_RW);
    }

    switch_page_dir(kpd);
}

void
switch_page_dir(struct pd* dir)
{
    extern void __switch_page_dir(paddr_t addr);

    __switch_page_dir(paddr((vaddr_t) dir));

    current_pd = dir;
}

static void
map_page(struct pd* dir, u32 page, u32 virt, u8 flags)
{
    struct pt* table;

    if (dir->entries[PD_INDEX(virt)].base == 0) {
        /* Create a new page table at the coresponding page dir entry */
        table = (struct pt*) vaddr(alloc_blocks(1));

        dir->entries[PD_INDEX(virt)].flags = flags;
        dir->entries[PD_INDEX(virt)].base =
                    (((u32) paddr((vaddr_t) table)) & 0xFFFFF000) >> 12;
    } else {
        /* Select the page dir entry */
        table = (struct pt*) vaddr((dir->entries[PD_INDEX(virt)].base << 12));
    }

    table->entries[PT_INDEX(virt)].flags = flags;
    table->entries[PT_INDEX(virt)].base = (page & 0xFFFFF000) >> 12;
}

/**
 * Allocate a page in the kernel space (ie. above KERNEL_OFFSET)
 * @return virtual adress of the page or zero if not found
 */
vaddr_t
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
}
