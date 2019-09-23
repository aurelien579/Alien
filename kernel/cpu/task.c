/*******************************************************************************
 * SOURCE NAME  : task.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Simple task management. Just allow switching to usermode.
 ******************************************************************************/

#include <kernel/cpu/task.h>
#include <kernel/cpu/gdt.h>
#include <kernel/cpu/idt.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/heap.h>
#include <string.h>


extern void user_space_switch(uint32_t eip, uint32_t esp, uint32_t ss, uint32_t cs);
extern void execute_task(struct task_info info);
extern uint32_t kernel_stack;
extern void tss_flush(uint32_t index);


/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

struct tss_entry kernel_tss;
struct task_header task_list;
struct task_header *current_task;


/*******************************************************************************
 *                          PRIVATE FUNCTIONS
 ******************************************************************************/

static inline void
save_current_task(struct isr_frame frame)
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


/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

void
tasking_init(uint32_t cr3, uint32_t eip, uint32_t esp, uint32_t ss, uint32_t cs)
{
    memset(&kernel_tss, 0, sizeof(struct tss_entry));
    kernel_tss.ss0 = 0x10;
    kernel_tss.cs = 0x08;

    kernel_tss.esp0 = (uint32_t) &kernel_stack;
    gdt_set_entry(5, (uint32_t) &kernel_tss, sizeof(struct tss_entry) - 1, 0xE9, 0xCF);
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
    user_space_switch(current_task->info.eip, current_task->info.esp,
                      current_task->info.ss, current_task->info.cs);
}

void
tasking_set_esp0(uint32_t esp0)
{
    kernel_tss.esp0 = esp0;
}

void
sched(struct isr_frame frame)
{
    save_current_task(frame);
    do_sched();
}

void
fork(struct isr_frame frame)
{	
    /*struct task_header *new_header = (struct task_header *) kmalloc(sizeof(struct task_header));
    //assert(new_header);
    
    save_current_task(frame);
    
    memcpy(new_header, current_task, sizeof(struct task_header));
    new_header->info.pid++;
        
    new_header->info.cr3 = copy_current_pagedir();
    
    current_task->next = new_header;
    current_task->next->next = current_task;
    
    new_header->info.regs.eax = 0;
    current_task->info.regs.eax = 1;
    
    do_sched();*/
}