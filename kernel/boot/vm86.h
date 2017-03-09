#ifndef VM86_H
#define VM86_H

#include <types.h>

void init_vm86();
void vm86exec(u32 vaddr, u32 size);

#endif
