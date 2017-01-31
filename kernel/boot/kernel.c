#include <kernel/io.h>
#include <kernel/paging.h>
#include <boot/multiboot.h>
#include <kernel/core/mm.h>
#include <kernel/kernel.h>
#include "gdt.h"
#include "idt.h"

void panic(char* msg)
{
    printf("[PANIC] %s\n", msg);
    while (1);
}

errno_t get_boot_info(struct boot_info *info, struct mb_info *mbi)
{
    if (MB_CHECK_FLAG (mbi->flags, 2))
        info->cmdline = (char*) vaddr(mbi->cmdline);
    else
        info->cmdline = (char*) 0;

    if (MB_CHECK_FLAG (mbi->flags, 6))
    {
        mbi->mmap_addr = vaddr(mbi->mmap_addr);
        struct mb_mmap_entry *mmap = (struct mb_mmap_entry *) mbi->mmap_addr;

        while ((u32) mmap < mbi->mmap_addr + mbi->mmap_length)
        {
            if (mmap->type == MB_MEM_FREE && mmap->base_low >= 0x100000)
            {
                info->kernel_end = (paddr_t) init_mm(0x100000 + mmap->len_low);
                info->mem_len = (u32) mmap->base_low + mmap->len_low;
            }
            mmap = MB_MMAP_NEXT (mmap);
        }
    }
    else
        return ERR_MBFLAGS;

    return ERR_NO;
}

void kernel_main(paddr_t addr, u32 magic)
{
    extern u32 KERNEL_VIRTUAL_BASE;
    kernel_info.kernel_vbase = (vaddr_t) &KERNEL_VIRTUAL_BASE;

    cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Invalid multiboot flag !");

    if (get_boot_info(&kernel_info, (struct mb_info*) vaddr(addr)) != ERR_NO)
        panic("Can't get boot informations from multiboot informations");

    gdt_install();
    idt_install();

    printf("Available memory : %d MB\n", kernel_info.mem_len / (1024*1024));
    init_paging(kernel_info.kernel_end);

    puts("Boot !");
}
