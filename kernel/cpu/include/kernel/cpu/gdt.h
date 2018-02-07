/*******************************************************************************
 * SOURCE NAME  : gdt.h
 * VERSION      : 0.1
 * CREATED DATE : 07/02/2018
 * LAST UPDATE  : 07/02/2018
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide basic routines for managing the GDT, installing it and
 *  adding entries.
 ******************************************************************************/

#ifndef CPU_GDT_H
#define CPU_GDT_H

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
