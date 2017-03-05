#include <kernel/paging.h>
#include <kernel/kernel.h>
#include <kernel/io.h>
#include <kernel/core/mm.h>

#define PAGE_SIZE 4096

/**
 * Identity map [0x0; kernel_end] -> [0xC0000000; 0xC0000000 + kernel_end]
 */
void
init_paging(u32 kernel_len)
{
    kpd = (struct pd*) vaddr(alloc_blocks(1));

    /* Calculate the required pages and page tables for mapping the kernel */
    u32 page_count = kernel_len / PAGE_SIZE;
    if (page_count * PAGE_SIZE < kernel_len)
        page_count++;

    u32 pt_count = page_count / 1024;
    if (pt_count * 1024 < page_count)
        pt_count++;

    /* Each page table required a new page */
    page_count += pt_count;
    if (pt_count * 1024 < page_count)
        pt_count++;

    printf("page_count: %d\n", page_count);

    for (u32 i = 0; i < page_count; i++) {
        map_page(kpd, i * 4096, i * 4096 + 0xC0000000, PE_PRESENT | PE_RW);
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

void
map_page(struct pd* dir, u32 page, u32 virt, u8 flags)
{
    struct pt* table;

    if (dir->entries[PD_INDEX(virt)].base == 0) {
        /* Create a new page table at the coresponding page dir entry */
        table = (struct pt*) vaddr(alloc_blocks(1));

        if (page == 300 * 4096) {
            printf("table : 0x%x\n", table);
        }
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
