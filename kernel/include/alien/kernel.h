#ifndef KERNEL_H
#define KERNEL_H

#include <types.h>
#include <alien/io.h>

#define align(a, b) \
    if (a % b != 0) \
        a += b - a % b;

#define updiv(a, b) \
    (((a) + (b) - 1) / (b))

typedef struct kernel_info
{
    char* cmdline;      /* Command line passed to the kernel at boot */
    u32 memlen;         /* Total length of the available memory */
    u32 len;            /* End of kernel.elf file */
    u32 vbase;          /* Virtual base of the higher half kernel */
    u32 start;
} kernel_info_t;

struct regs {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
} __attribute__((packed));

struct kernel_info kinfo;

void panic(char* msg);
void dump_regs(struct regs r);

static inline void
outb(u16 port, u8 data)
{
    asm volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

static inline u8
inb(u16 port)
{
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
