#ifndef TASK_H
#define TASK_H

#include <types.h>
#include <alien/kernel.h>

struct task_info {
    u32         pid;
    u32         cr3;
    struct regs regs;
    u32         eflags;
    u32         eip;
    u32         cs, ss, esp;
};

struct task_header {
    struct task_info    info;
    struct task_header *next;
};

void tasking_init(u32 cr3, u32 eip, u32 esp, u32 ss, u32 cs);
void usermode();

#endif
