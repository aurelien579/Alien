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
#include <kernel/memory/paging.h>
#include <kernel/memory/heap.h>
#include <kernel/kernel.h>
#include <kernel/device/device.h>
#include <kernel/device/ata.h>
#include <kernel/tests.h>
#include <kernel/fs/vfs.h>

#include "multiboot.h"

/**
 * @brief The main entry point of the kernel
 */
void kernel_main(struct mb_info *boot_info)
{
    if (!parse_boot_info(boot_info)) {
        KERNEL_PANIC("[PANIC] Invalid boot info!!");
    }

    gdt_install();
    idt_install();
    paging_install(KERNEL.memlen, KERNEL_END);

    heap_install();

    /*void *ptr = kmalloc(PAGE_SIZE);
    printf("ptr: 0x%x\n", ptr);

    uint32_t page = alloc_kpage();
    printf("page: 0x%x\n", page);

    *((uint64_t *) ptr) = 0x6482;

    ptr = kmalloc(PAGE_SIZE * 10);
    printf("ptr: 0x%x\n", ptr);
    *((uint64_t *) ptr) = 0x6482;
    memset(ptr, 0x6482, 10 * PAGE_SIZE);


    printf("success!\n");*/

    ata_install();
    
    struct device *dev = device_find("ATA-3");

    vfs_init(dev);
    ext4_mount(dev, &vfs_root.node);

    struct vdir *dir = vdir_open("/toto");
    if (dir == 0) {
        printf("File not found\n");
    } else {
        struct vnode_list *cur = dir->list;
        while (cur->next) {
            printf("%s\n", cur->node.name);
            cur = cur->next;
        }
    }

    //test_all();
}
