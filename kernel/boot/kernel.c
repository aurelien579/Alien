#include <kernel/io.h>
#include <kernel/paging.h>
#include <boot/multiboot.h>
#include "gdt.h"
#include "idt.h"

void kernel_main(u32 addr, u32 magic)
{
    cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        puts("[ERROR] Not booted using multiboot !\n");
        while (1);
    }

    struct mb_info* mbi = (struct mb_info*) (addr + kernel_virtual_base());

    printf("Multiboot informations :\n\n");
    if (MB_CHECK_FLAG (mbi->flags, 0))
    {
        printf("mem_lower: 0x%x, mem_upper: 0x%x\n",
               mbi->mem_lower, mbi->mem_upper);
    }

    if (MB_CHECK_FLAG (mbi->flags, 2))
    {
        printf("cmdline: %s\n",
               (char*) (mbi->cmdline + kernel_virtual_base()));
    }

    if (MB_CHECK_FLAG (mbi->flags, 6))
    {
        mbi->mmap_addr += kernel_virtual_base();
        struct mb_mmap_entry *mmap = (struct mb_mmap_entry*) mbi->mmap_addr;

        printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",
                mbi->mmap_addr, mbi->mmap_length);

        while (mmap < mbi->mmap_addr + mbi->mmap_length)
        {
            printf("size = 0x%x, base_addr = 0x%x, length = 0x%x, type = 0x%x\n",
                   mmap->size, mmap->base_low, mmap->len_low, mmap->type);
            mmap = (struct mb_mmap_entry*)
                   (((u32)mmap) + mmap->size + sizeof (mmap->size));
        }
    }

    gdt_install();
    idt_install();
    init_paging();

    puts("Boot !");
}
