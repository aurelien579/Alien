#ifndef GDT_H
#define GDT_H

#include <types.h>

#define GDT_SIZE 3

#define K_CODE_SEL 0x08
#define K_DATA_SEL 0x10
#define U_CODE_SEL 0x18
#define U_DATA_SEL 0x20

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
 * Install the gdt with 3 segment descriptors :
 *      0 : 0x00 - null descriptor
 *      1 : 0x08 - kernel code descriptor
 *      2 : 0x10 - kernel data descriptor
 */
void gdt_install();

#endif
