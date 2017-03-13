#ifndef VM86_H
#define VM86_H

#include <types.h>
#include <alien/kernel.h>

extern void vm86_bios();

void init_vm86();
void vm86exec(u32 vaddr, u32 size, struct regs r);
struct regs* vm86_get_retregs();

#endif
