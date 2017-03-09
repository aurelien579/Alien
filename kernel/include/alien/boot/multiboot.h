#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <types.h>

#define EMBFLAGS -11

#define MB_CHECK_FLAG(flags, bit)   (flags & (1 << bit))

#define MULTIBOOT_HEADER_MAGIC                  0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002
#define MULTIBOOT_UNSUPPORTED                   0x0000fffc
#define MULTIBOOT_MOD_ALIGN                     0x00001000
#define MULTIBOOT_INFO_ALIGN                    0x00000004
#define MULTIBOOT_PAGE_ALIGN                    0x00000001
#define MULTIBOOT_MEMORY_INFO                   0x00000002
#define MULTIBOOT_VIDEO_MODE                    0x00000004
#define MULTIBOOT_AOUT_KLUDGE                   0x00010000
#define MULTIBOOT_INFO_MEMORY                   0x00000001
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
#define MULTIBOOT_INFO_MODS                     0x00000008
#define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200
#define MULTIBOOT_INFO_APM_TABLE                0x00000400
#define MULTIBOOT_INFO_VIDEO_INFO               0x00000800

struct mb_elf_section_header_table
{
    u32 num;
    u32 size;
    u32 addr;
    u32 shndx;
};

struct mb_aout_symbol_table
{
    u32 tabsize;
    u32 strsize;
    u32 addr;
    u32 reserved;
};

struct mb_info
{
    /* Multiboot info version number */
    u32 flags;

    /* Available memory from BIOS */
    u32 mem_lower;
    u32 mem_upper;

    /* "root" partition */
    u32 boot_device;

    /* Kernel command line */
    u32 cmdline;

    /* Boot-Module list */
    u32 mods_count;
    u32 mods_addr;

    union
    {
        struct mb_aout_symbol_table aout_sym;
        struct mb_elf_section_header_table elf_sec;
    } u;

    /* Memory Mapping buffer */
    u32 mmap_length;
    u32 mmap_addr;

    /* Drive Info buffer */
    u32 drives_length;
    u32 drives_addr;

    /* ROM configuration table */
    u32 config_table;

    /* Boot Loader Name */
    u32 boot_loader_name;

    /* APM table */
    u32 apm_table;

    /* Video */
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
};

#define MB_MMAP_NEXT(mmap) \
    (struct mb_mmap_entry*) (((u32)mmap) + mmap->size + sizeof (mmap->size))


struct mb_mmap_entry
{
    u32 size;
    u32 base_low, base_high;
    u32 len_low, len_high;
    #define MB_MEM_FREE 1
    #define MB_MEM_RESERVED  2
    u32 type;
} __attribute__((packed));

struct mb_mod_list
{
    /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
    u32 mod_start;
    u32 mod_end;

    /* Module command line */
    u32 cmdline;

    /* padding to take it to 16 bytes (must be zero) */
    u32 pad;
};


#endif
