#ifndef IDT_H
#define IDT_H

#include <types.h>

#define IDT_SIZE 256

typedef void (*irq_handler_t) (void);

void idt_install();
void enable_irq(u32 irq);
void register_irq(u32 irq, irq_handler_t handler);

#endif
