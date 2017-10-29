#include <alien/task.h>
#include <alien/string.h>
#include <alien/memory/paging.h>
#include "gdt.h"

extern void user_space_switch(u32 eip, u32 esp, u32 ss, u32 cs);

extern void __sched(struct regs regs, u32 eip, u32 cs, u32 eflags, u32 esp, u32 ss);
extern u32 kernel_stack;
extern void tss_flush(u32 index);

struct tss_entry kernel_tss;
struct task_header task_list;
struct task_header *current_task;

void
tasking_init(u32 cr3, u32 eip, u32 esp, u32 ss, u32 cs)
{
    memset(&kernel_tss, 0, sizeof(struct tss_entry));
    kernel_tss.ss0 = 0x10;
    kernel_tss.cs = 0x08;

    kernel_tss.esp0 = (u32) &kernel_stack;
    gdt_set_gate(5, (u32) &kernel_tss, sizeof(struct tss_entry) - 1, 0xE9, 0xCF);
    tss_flush(5 * 8 | 3);

    current_task = &task_list;
    current_task->next = current_task;
    
    memset(&current_task->info.regs, 0, sizeof(struct regs));    
    current_task->info.esp = esp;
    current_task->info.ss = ss;
    current_task->info.cs = cs;
    current_task->info.eip = eip;
    current_task->info.cr3 = cr3;
}

void
usermode()
{
	switch_page_dir(current_task->info.cr3);
	user_space_switch(current_task->info.eip,
						current_task->info.esp,
						current_task->info.ss,
						current_task->info.cs);
}

void
tasking_set_esp0(u32 esp0)
{
    kernel_tss.esp0 = esp0;
}

void
sched(struct regs regs, u32 eip, u32 cs, u32 eflags, u32 esp, u32 ss)
{
    current_task->info.regs = regs;
    current_task->info.esp = esp;
    current_task->info.ss = ss;
    current_task->info.cs = cs;
    current_task->info.eip = eip,
    current_task->info.eflags = eflags;

    current_task = current_task->next;

    __sched(current_task->info.regs,
            current_task->info.eip,
            current_task->info.cs,
            current_task->info.eflags,
            current_task->info.esp,
            current_task->info.ss);
}
