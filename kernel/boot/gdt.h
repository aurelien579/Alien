#ifndef GDT_H
#define GDT_H

#include <types.h>

#define K_CODE_SEL 0x08
#define K_DATA_SEL 0x10
#define U_CODE_SEL 0x18
#define U_DATA_SEL 0x20

struct tss_entry
{
   u32 prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   u32 esp0;       // The stack pointer to load when we change to kernel mode.
   u32 ss0;        // The stack segment to load when we change to kernel mode.
   u32 esp1;       // everything below here is unusued now..
   u32 ss1;
   u32 esp2;
   u32 ss2;
   u32 cr3;
   u32 eip;
   u32 eflags;
   u32 eax;
   u32 ecx;
   u32 edx;
   u32 ebx;
   u32 esp;
   u32 ebp;
   u32 esi;
   u32 edi;
   u32 es;
   u32 cs;
   u32 ss;
   u32 ds;
   u32 fs;
   u32 gs;
   u32 ldt;
   u16 trap;
   u16 iomap_base;
   u8 bitmap[32];
} __attribute__((packed));

/**
 * Set the gdt gate @num with the attributes passed
 * @param num the num of the gate to set
 * @param base the base of the segment
 * @param limit the limit of the segment
 * @param access access byte
 * @param gran gran and flags byte
 */
void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);

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
