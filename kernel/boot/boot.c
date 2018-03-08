/*******************************************************************************
 * SOURCE NAME  : boot.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Contains the initial code executed after the bootloader.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include <kernel/debug.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/memory/paging.h>
#include <kernel/kernel.h>
#include <kernel/device/ata.h>

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
    
    ata_install();
    
    struct device *dev = device_find("ATA-0");
    uint32_t size = 1;
    uint8_t out[4096];
    
    device_random_read(dev, 0, &size, out);
    
    printf("Hello\n");
}
