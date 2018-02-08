/*******************************************************************************
 * SOURCE NAME  : multiboot.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements functions for parsing multiboot informations
 ******************************************************************************/

#ifndef BOOT_MULTIBOOT_H
#define BOOT_MULTIBOOT_H

#include <stdint.h>

struct mb_info;


/**
 * @brief Parse the boot informations and save relevant values to the KERNEL
 *  structure. Return 0 on error. An error occured if one of the required flags
 *  are missings.
 *  
 * Required flags: INFO_MEM_MAP
 * @param mbi the virtual adress of the boot info structure
 * @return 0 on error, 1 on success
 */
uint8_t parse_boot_info(struct mb_info *mbi);

#endif
