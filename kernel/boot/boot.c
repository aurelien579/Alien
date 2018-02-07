/*******************************************************************************
 * SOURCE NAME  : boot.c
 * VERSION      : 0.1
 * CREATED DATE : 06/02/2018
 * LAST UPDATE  : 06/02/2018
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Contains the initial code executed after the bootloader.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include <kernel/debug.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>

#if 0
int parse_boot_info(struct mb_info *mbi)
{
    if (MB_CHECK_FLAG (mbi->flags, 2))
        kinfo.cmdline = (char*) (mbi->cmdline + kinfo.vbase);

    if (MB_CHECK_FLAG (mbi->flags, 6)) {
        mbi->mmap_addr = mbi->mmap_addr + kinfo.vbase;
        struct mb_mmap_entry *mmap = (struct mb_mmap_entry *) mbi->mmap_addr;

        while ((u32) mmap < mbi->mmap_addr + mbi->mmap_length)
        {
            if (mmap->type == MB_MEM_FREE && mmap->base_low >= 0x100000)
                kinfo.memlen = (u32) mmap->base_low + mmap->len_low;
            mmap = MB_MMAP_NEXT (mmap);
        }
    } else
        return EMBFLAGS;

    return 0;
}
#endif

/**
 * @brief The main entry point of the kernel
 */
void kernel_main(void)
{
    gdt_install();
    idt_install();
    printf("Hello\n");
}
