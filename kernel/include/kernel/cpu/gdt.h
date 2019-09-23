/*******************************************************************************
 * SOURCE NAME  : gdt.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide basic routines for managing the GDT, installing it and
 *  adding entries.
 ******************************************************************************/

#ifndef CPU_GDT_H
#define CPU_GDT_H

struct tss_entry
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now..
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;
   uint32_t cs;
   uint32_t ss;
   uint32_t ds;
   uint32_t fs;
   uint32_t gs;
   uint32_t ldt;
   uint16_t trap;
   uint16_t iomap_base;
   uint8_t  bitmap[32];
} __attribute__((packed));

/**
 * Set the gdt gate @num with the attributes passed. Fails if @num is greater
 * than the gdt capacity.
 * @param num the num of the gate to set
 * @param base the base of the segment
 * @param limit the limit of the segment
 * @param access access byte
 * @param gran gran and flags byte
 * @return 1 on success, 0 on error
 */
int gdt_set_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran);

/**
 * Install the gdt with 5 segment descriptors :
 *      0 : 0x00 - null descriptor
 *      1 : 0x08 - kernel code segment descriptor
 *      2 : 0x10 - kernel data segment descriptor
 *      3 : 0x18 - user code segment descriptor
 *      4 : 0x20 - user data segment descriptor
 */
void gdt_install();

#endif
