#include "gdt.h"
#include "idt.h"
#include "vm86.h"
#include <alien/memory/paging.h>
#include <alien/string.h>

#define VM86_BASE_VADDR 0x10000
extern void isr13();
extern void tss_flush(u32 index);
extern void vm86_jump(struct regs r, u32 ip, u32 cs, u32 sp);

extern u32 kernel_stack;
struct tss_entry _vm86tss;

static struct regs retregs;

void
init_vm86()
{
    memset(&_vm86tss, 0, sizeof(struct tss_entry));

    _vm86tss.ss0  = 0x10;
    _vm86tss.esp0 = (u32) &kernel_stack;
    _vm86tss.iomap_base = sizeof(struct tss_entry) - (32);
    kprintf("sizeof(tss) : 0x%x\n", sizeof(struct tss_entry));
    gdt_set_gate(5, (u32) &_vm86tss, sizeof(struct tss_entry), 0xE9, 0xCF);
    tss_flush(5 * 8 | 3);
}

void
vm86tss_set_esp(u32 esp)
{
    kprintf("saved esp : 0x%x\n", esp);
    _vm86tss.esp0 = esp;
}

void out_of_vm86(struct regs r)
{
    switch_page_dir(&kpd);
    idt_set_gate(13, (u32) isr13, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);

    retregs = r;
}

struct regs*
vm86_get_retregs()
{
    return &retregs;
}

extern void __out_of_vm86(void);
void vm86exec(u32 vaddr, u32 size, struct regs r)
{
    struct table* dir = create_user_pagedir();
    paging_user_alloc_page(dir, VM86_BASE_VADDR);
    switch_page_dir(dir);
    
    u32 cs = VM86_BASE_VADDR >> 4;

    memcpy((void*) VM86_BASE_VADDR, (void*) vaddr, size);

    idt_set_gate(13, (u32) __out_of_vm86, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    u32 ip = VM86_BASE_VADDR - (cs << 4);

    vm86_jump(r, ip, cs, ip + 0xfff);
}

