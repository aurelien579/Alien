#ifndef GDT_H
#define GDT_H

void gdt_set_gate(int num, unsigned int base, unsigned int limit,
                  unsigned char access, unsigned char gran);

void gdt_install();

#endif
