#ifndef KERNEL_H
#define KERNEL_H

static inline void outportb(unsigned short port, unsigned char data)
{
    asm volatile(
        "outb %1, %0"
        ::  "dN" (port), "a" (data)
    );
}

#endif
