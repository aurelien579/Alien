/*******************************************************************************
 * SOURCE NAME  : idt.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide basic routines for managing the IDT. You must call
 *  idt_install before anything else. Then you can set your own exceptions/irqs
 *  handlers. Don't forget to enable the irq you are handling.
 ******************************************************************************/

#ifndef CPU_IDT_H
#define CPU_IDT_H

#include <stdint.h>
#include <kernel/cpu.h>

#define IDT_MAX_ENTRIES 128

#define IDT_EF_TASK 0x05
#define IDT_EF_INT  0x0E
#define IDT_EF_TRAP 0x0F
#define IDT_EF_P    0x80
#define IDT_EF_U    0x60

struct isr_frame
{
    uint32_t    ds;
    struct regs regs;
    uint32_t    int_no;
    uint32_t    errorcode;
    uint32_t    eip;
    uint32_t    cs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    ss;
} __attribute__((packed));

typedef void (*irq_handler_t) (uint32_t irq, const struct regs *regs);
typedef void (*exception_handler_t) (uint32_t n, uint32_t code,
                                     const struct regs *regs);

void idt_install();
int idt_set_entry(uint32_t n, uint32_t offset, uint8_t selector, uint8_t flags);

void enable_irq(uint32_t irq);
void irq_ack(uint32_t irq);

void set_exception_handler(uint32_t n, exception_handler_t handler);
void set_irq_handler(uint32_t irq, irq_handler_t handler);

#endif
