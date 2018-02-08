/*******************************************************************************
 * SOURCE NAME  : multiboot.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements functions for parsing multiboot informations
 ******************************************************************************/

#include "multiboot.h"

#include <kernel/kernel.h>
#include <stdint.h>

#define HEADER_MAGIC          0x1BADB002
#define BOOTLOADER_MAGIC      0x2BADB002
#define UNSUPPORTED           0x0000fffc
#define MOD_ALIGN             0x00001000
#define INFO_ALIGN            0x00000004
#define PAGE_ALIGN            0x00000001
#define MEMORY_INFO           0x00000002
#define VIDEO_MODE            0x00000004
#define AOUT_KLUDGE           0x00010000
#define INFO_MEMORY           0x00000001
#define INFO_BOOTDEV          0x00000002
#define INFO_CMDLINE          0x00000004
#define INFO_MODS             0x00000008
#define INFO_AOUT_SYMS        0x00000010
#define INFO_ELF_SHDR         0X00000020
#define INFO_MEM_MAP          0x00000040
#define INFO_DRIVE_INFO       0x00000080
#define INFO_CONFIG_TABLE     0x00000100
#define INFO_BOOT_LOADER_NAME 0x00000200
#define INFO_APM_TABLE        0x00000400
#define INFO_VIDEO_INFO       0x00000800

struct mb_elf_section_header_table
{
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
};

struct mb_aout_symbol_table
{
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
};

struct mb_info
{
    /* Multiboot info version number */
    uint32_t flags;

    /* Available memory from BIOS */
    uint32_t mem_lower;
    uint32_t mem_upper;

    /* "root" partition */
    uint32_t boot_device;

    /* Kernel command line */
    uint32_t cmdline;

    /* Boot-Module list */
    uint32_t mods_count;
    uint32_t mods_addr;

    union
    {
        struct mb_aout_symbol_table aout_sym;
        struct mb_elf_section_header_table elf_sec;
    };

    /* Memory Mapping buffer */
    uint32_t mmap_length;
    uint32_t mmap_addr;

    /* Drive Info buffer */
    uint32_t drives_length;
    uint32_t drives_addr;

    /* ROM configuration table */
    uint32_t config_table;

    /* Boot Loader Name */
    uint32_t boot_loader_name;

    /* APM table */
    uint32_t apm_table;

    /* Video */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
};

#define MB_MMAP_NEXT(mmap) \
    (struct mb_mmap_entry*) (((uint32_t)mmap) + mmap->size + sizeof (mmap->size))


struct mb_mmap_entry
{
    uint32_t size;
    uint32_t base_low, base_high;
    uint32_t len_low, len_high;
    #define MB_MEM_FREE 1
    #define MB_MEM_RESERVED  2
    uint32_t type;
} __attribute__((packed));

struct mb_mod_list
{
    /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
    uint32_t mod_start;
    uint32_t mod_end;

    /* Module command line */
    uint32_t cmdline;

    /* padding to take it to 16 bytes (must be zero) */
    uint32_t pad;
};

static void parse_memmap(struct mb_info *mbi)
{
    mbi->mmap_addr += KERNEL_VBASE;
    struct mb_mmap_entry *mmap = (struct mb_mmap_entry *) mbi->mmap_addr;

    while ((uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length) {
        if (mmap->type == MB_MEM_FREE && mmap->base_low >= 0x100000)
            KERNEL.memlen = (uint32_t) mmap->base_low + mmap->len_low;
        mmap = MB_MMAP_NEXT (mmap);
    }
}

uint8_t parse_boot_info(struct mb_info *mbi)
{
#if 0
    if (mbi->flags && INFO_CMDLINE)
        parse_cmdline((const char *) (mbi->cmdline + KERNEL_VBASE));
#endif

    if (!(mbi->flags && INFO_MEM_MAP)) return 0;

    parse_memmap(mbi);    

    return 1;
}
