#include <stdint.h>
#include "console.h"


#if 0
void panic(const char *msg)
{
    kprintf("[PANIC] %s\n", msg);
    while(1);
}

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

void dump_regs(struct regs r)
{
    kprintf("=== DUMP REGISTERS ===\n");
    kprintf("eax: 0x%x, ebx: 0x%x, ecx: 0x%x, edx: 0x%x\n", r.eax, r.ebx, r.ecx, r.edx);
    kprintf("esi: 0x%x, edi: 0x%x, esp: 0x%x, ebp: 0x%x\n", r.esi, r.edi, r.esp, r.ebp);
}
#endif

/**
 * @brief The main entry point of the kernel
 */
void kernel_main(void)
{
    boot_print("Hello");
}
