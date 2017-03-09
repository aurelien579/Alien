#include "gdt.h"
#include "idt.h"
#include "vm86.h"
#include <alien/boot/paging.h>

#define VM86_BASE_VADDR 0x10000

extern void tss_flush(u32 index);
extern void vm86_jump(u32 ip, u32 cs);

extern u32 kernel_stack;
struct tss_entry _vm86tss;

void init_vm86()
{
    memset(&_vm86tss, 0, sizeof(struct tss_entry));

    _vm86tss.ss0  = 0x10;
    _vm86tss.esp0 = &kernel_stack;
    _vm86tss.iomap_base = &_vm86tss.bitmap[33];
    gdt_set_gate(5, &_vm86tss, sizeof(struct tss_entry), 0xE9, 0xCF);
    tss_flush(5 * 8 | 3);
}

void out_of_vm86(void)
{
    switch_page_dir(&kpd);
    kprintf("out of vm86 !\n");
    while(1);
}

extern void __out_of_vm86(void);
void vm86exec(u32 vaddr, u32 size)
{
    struct table* dir = create_user_pagedir();
    page_t page = _get_page();
    u32 cs = VM86_BASE_VADDR >> 4;

    user_id_map(dir);
    user_pd_map(dir, page, VM86_BASE_VADDR);

    switch_page_dir(dir);

    memcpy(VM86_BASE_VADDR, vaddr, size);

    idt_set_gate(13, (u32) __out_of_vm86, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    vm86_jump(VM86_BASE_VADDR - (cs << 4), cs);
}
