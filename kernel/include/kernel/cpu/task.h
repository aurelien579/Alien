#ifndef CPU_TASK_H
#define CPU_TASK_H

#include <stdint.h>
#include <kernel/cpu/idt.h>

struct task_info {
    uint32_t pid;
    uint32_t cr3;
    struct regs regs;
    uint32_t eflags;
    uint32_t eip;
    uint32_t cs, ss, esp;
};

struct task_header {
    struct task_info    info;
    struct task_header *next;
};

void tasking_init(uint32_t cr3, uint32_t eip, uint32_t esp, uint32_t ss, uint32_t cs);
void usermode();
void fork();
void sched(struct isr_frame frame);

#endif