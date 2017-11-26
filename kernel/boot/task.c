#include <alien/task.h>
#include <assert.h>
#include <alien/string.h>
#include <alien/memory/paging.h>
#include <alien/memory/kmalloc.h>
#include "gdt.h"

extern void user_space_switch(u32 eip, u32 esp, u32 ss, u32 cs);

extern void execute_task(task_info_t info);
extern u32 kernel_stack;
extern void tss_flush(u32 index);

struct tss_entry kernel_tss;
task_header_t task_list;
task_header_t *current_task;

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
    current_task->info.pid = 0;
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

static inline void
save_current_task(interrupt_frame_t frame)
{
	current_task->info.regs = frame.regs;
    current_task->info.esp = frame.esp;
    current_task->info.ss = frame.ss;
    current_task->info.cs = frame.cs;
    current_task->info.eip = frame.eip,
    current_task->info.eflags = frame.eflags;
}

static void
do_sched()
{
	current_task = current_task->next;

    execute_task(current_task->info);
}

void
sched(interrupt_frame_t frame)
{
    save_current_task(frame);
    do_sched();
}

void
fork(interrupt_frame_t frame)
{	
	task_header_t *new_header = (task_header_t *) kmalloc(sizeof(task_header_t));
	assert(new_header);
	
	save_current_task(frame);
	
	memcpy(new_header, current_task, sizeof(task_header_t));
	new_header->info.pid++;
		
	new_header->info.cr3 = copy_current_pagedir();
	
	current_task->next = new_header;
	current_task->next->next = current_task;
	
	new_header->info.regs.eax = 0;
	current_task->info.regs.eax = 1;
	
	do_sched();
}
