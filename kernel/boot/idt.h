#ifndef IDT_H
#define IDT_H

#include <types.h>

#define IDT_SIZE 256

#define IDT_EF_TASK 0x05
#define IDT_EF_INT  0x0E
#define IDT_EF_TRAP 0x0F
#define IDT_EF_P    0x80

typedef void (*irq_handler_t) (void);

void idt_install();
void enable_irq(u32 irq);
void register_irq(u32 irq, irq_handler_t handler);
void idt_set_gate(u32 num, u32 offset, u8 selector, u8 flags);

#endif
