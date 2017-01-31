#ifndef KERNEL_H
#define KERNEL_H

#include <types.h>

struct boot_info
{
    char* cmdline;          /* Command line passed to the kernel at boot */
    u32 mem_len;            /* Total length of the available memory */
    paddr_t kernel_end;     /* End of kernel.elf file */
    vaddr_t kernel_vbase;   /* Virtual base of the higher half kernel */
};

struct boot_info kernel_info;

static inline void outb(u16 port, u8 data)
{
    asm volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
