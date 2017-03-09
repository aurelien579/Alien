#include <alien/io.h>
#include <alien/kernel.h>

#include <alien/boot/paging.h>
#include <alien/boot/multiboot.h>
#include <alien/mm.h>

#include "gdt.h"
#include "idt.h"
#include "vm86.h"

void
panic(char* msg)
{
    kprintf("[PANIC] %s\n", msg);
    while (1);
}

int
parse_boot_info(struct mb_info *mbi)
{
    if (MB_CHECK_FLAG (mbi->flags, 2))
        kinfo.cmdline = (char*) (mbi->cmdline + kinfo.vbase);

    if (MB_CHECK_FLAG (mbi->flags, 6))
    {
        mbi->mmap_addr = mbi->mmap_addr + kinfo.vbase;
        struct mb_mmap_entry *mmap = (struct mb_mmap_entry *) mbi->mmap_addr;

        while ((u32) mmap < mbi->mmap_addr + mbi->mmap_length)
        {
            if (mmap->type == MB_MEM_FREE && mmap->base_low >= 0x100000)
                kinfo.memlen = (u32) mmap->base_low + mmap->len_low;
            mmap = MB_MMAP_NEXT (mmap);
        }
    }
    else
        return EMBFLAGS;

    return 0;
}

void
user_test()
{
    asm("movl $0x6482, %ecx");
    while(1);
}

extern void vm86_set_video_mode();

void
kernel_main(struct mb_info* mb_info, u32 magic, u32 vbase, u32 len)
{
    kinfo.vbase = vbase;
    kinfo.len = len;

    kcls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Invalid multiboot flag !");

    if (parse_boot_info(mb_info) < 0)
        panic("Can't get boot informations from multiboot informations");

    gdt_install();
    idt_install();

    kprintf("Available memory : %d MB\n", kinfo.memlen / (1024 * 1024));
    kprintf("kernel_end : 0x%x\n", kinfo.len);
    init_paging();

    //init_kheap();

    init_vm86();
    vm86exec(&vm86_set_video_mode, 500);

    kputs("Boot !");
}
