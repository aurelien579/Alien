/*******************************************************************************
 * SOURCE NAME  : boot.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Contains the initial code executed after the bootloader.
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/debug.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/task.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/heap.h>
#include <kernel/kernel.h>
#include <kernel/device/device.h>
#include <kernel/device/ata.h>
#include <kernel/tests.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext4.h>
#include <kernel/drivers/vga.h>
#include <kernel/device/initram.h>

#include "multiboot.h"

static void
user_test()
{
    asm ("mov $0x01, %eax\nint $100");
    while(1);
}

/**
 * @brief The main entry point of the kernel
 */
void
kernel_main(struct mb_info *boot_info)
{
    vga_install();

    if (!parse_boot_info(boot_info)) {
        KERNEL_PANIC("[PANIC] Invalid boot info!!");
    }

    gdt_install();
    idt_install();
    paging_install(KERNEL.memlen, KERNEL_END);
    heap_install();
    //ata_install();

    /* USER MODE TEST */
    /*
    uint32_t cr3 = create_user_page_dir();
    switch_page_dir(cr3);

    uint32_t page = alloc_page(0x400000, 1);
    uint32_t stack = alloc_page(0x400000, 1);
    stack += 0xfff;

    memcpy((void *) page, &user_test, 100);
    tasking_init(cr3, page, stack, 0x23, 0x1B);

    usermode();
    */



    /* VGA TEST */
    //vga_fill_rect(10, 10, 100, 100, 1);
    //vga_put_char(0, 0, 'A');



    /* STORAGE TEST */

    /*struct device *dev = device_find("ATA-3");

    if (!dev) {
        printf("ATA-3 Not found\n");
        return;
    }

    vfs_init(dev);
    if (ext4_mount(dev, &vfs_root.node) != 1) {
        printf("Can't mount ext driver !\n");
        return;
    }

    printf("vfs_root: 0x%x\n", &vfs_root);

    struct vdir *dir = vdir_open("/toto");
    if (dir == 0) {
        printf("File not found\n");
    } else {
        struct vnode_list *cur = dir->list;
        while (cur->next) {
            printf("%s\n", cur->node.name);
            cur = cur->next;
        }
    }*/

    //test_all();
}
