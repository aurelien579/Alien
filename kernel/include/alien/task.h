#ifndef TASK_H
#define TASK_H

#include <types.h>
#include <alien/kernel.h>

typedef struct task_info {
    u32         pid;
    u32         cr3;
    struct regs regs;
    u32         eflags;
    u32         eip;
    u32         cs, ss, esp;
} task_info_t;

typedef struct task_header {
    struct task_info    info;
    struct task_header *next;
} task_header_t;

void tasking_init(u32 cr3, u32 eip, u32 esp, u32 ss, u32 cs);
void usermode();
void fork();
void sched(interrupt_frame_t frame);

#endif
