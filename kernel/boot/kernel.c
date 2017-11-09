#include <alien/io.h>
#include <alien/string.h>
#include <alien/kernel.h>
#include <alien/task.h>

#include <alien/memory/paging.h>
#include <alien/memory/kmalloc.h>
#include <alien/boot/multiboot.h>
#include <alien/mm.h>
#include <alien/vfs.h>
#include <alien/initrd.h>

#include <assert.h>

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

void
dump_regs(struct regs r)
{
    kprintf("=== DUMP REGISTERS ===\n");
    kprintf("eax: 0x%x, ebx: 0x%x, ecx: 0x%x, edx: 0x%x\n", r.eax, r.ebx, r.ecx, r.edx);
    kprintf("esi: 0x%x, edi: 0x%x, esp: 0x%x, ebp: 0x%x\n", r.esi, r.edi, r.esp, r.ebp);
}

void
user_test()
{
	u32 ret;
	
    asm("mov $0x01, %%eax\n"
		"int $0x64\n"
		"mov %0, %%eax"
		: "=a"(ret) :);
	
	if (ret) {
		while(1) {
			asm("mov $0x0, %eax\n"
				"mov $0x44, %ebx\n"
				"int $0x64");
		}
	} else {
		while(1) {
			asm("mov $0x0, %eax\n"
				"mov $0x22, %ebx\n"
				"int $0x64");	
		}
	}
	
	while(1);
}

void
kernel_main(struct mb_info* mb_info, u32 magic, u32 vbase, u32 len, u32 kernel_start)
{
    kinfo.vbase = vbase;
    kinfo.len = len;
    kinfo.start = kernel_start;
    
    kcls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("Invalid multiboot flag !");
	}
	
    if (parse_boot_info(mb_info) < 0) {
        panic("Can't get boot informations from multiboot informations");
	}
	
	if (mb_info->mods_count < 1) {
		panic("No module loaded!");
	}
	
    gdt_install();
    idt_install();

    kprintf("Available memory : %d MB\n", kinfo.memlen / (1024 * 1024));
    kprintf("kernel_end : 0x%x\n", kinfo.len);
    	
	struct mb_mod_list *mod_list = (struct mb_mod_list *)
									(mb_info->mods_addr + kinfo.vbase);
	
	kprintf("modaddr : 0x%x\n", mod_list->mod_start);
	
	if (mod_list->mod_end > kinfo.len) {
		kinfo.len = mod_list->mod_end;
	}
	
    init_paging();
	kmalloc_init();
	

	
	//vfs_node_t root;
	
	/*init_initrd(mod_list->mod_start + kinfo.vbase, &root);
	vfs_init(&root);
	
	vfs_find(0, "/test", &root);
	
	u8 buffer[64];
	
	vfs_read(&root, 0, 64, buffer);
	
	kprintf("%s\n", buffer);
	*/
	
    u32 cr3 = create_user_pagedir();
	switch_page_dir(cr3);
	
	u32 page = alloc_page(0x400000, 1);
	u32 stack = alloc_page(0x800000, 1);
	stack += 0xFFF;
	
    memcpy((void*) page, &user_test, 100);
	
    tasking_init(cr3, page, stack, 0x23, 0x1B);

    usermode();

    kputs("Boot !");
}
