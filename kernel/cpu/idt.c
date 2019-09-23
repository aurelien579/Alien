/*******************************************************************************
 * SOURCE NAME  : idt.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide basic routines for managing the IDT. You must call
 *  idt_install before anything else. Then you can set your own exceptions/irqs
 *  handlers. Don't forget to enable the irq you are handling.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include <kernel/cpu/idt.h>
#include <kernel/cpu.h>
#include <kernel/io.h>
#include <kernel/debug.h>

#define MASTER_IRQ_COMMAND  0x20
#define MASTER_IRQ_DATA     0x21
#define SLAVE_IRQ_COMMAND   0xA0
#define SLAVE_IRQ_DATA      0xA1


/*******************************************************************************
 *                          PRIVATE STRUCTURES
 ******************************************************************************/

struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


/*******************************************************************************
 *                          EXTERN FUNCTIONS
 ******************************************************************************/

/* All defined in cpu.asm */
extern void idt_flush(struct idt_ptr *idt_ptr);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

extern void isr100();


/*******************************************************************************
 *                          PRIVATE GLOBAL VARIABLES
 ******************************************************************************/

static struct idt_entry     idt[IDT_MAX_ENTRIES];
static struct idt_ptr       idt_ptr;

static irq_handler_t        irq_handlers[16];
static exception_handler_t  exception_handlers[32];

static void
syscall_handler(struct isr_frame frame)
{
    if (frame.regs.eax == 0x00) {
        printf("%d\n", frame.regs.ebx);
    } else if (frame.regs.eax == 0x01) {
        //fork(frame);
        printf("FORK\n");
    } else {
        printf("unknown syscall!\n");
    }
}

static void
exception_handler(uint32_t n, uint32_t code, const struct regs *regs)
{
    if (exception_handlers[n]) {
        exception_handlers[n](n, code, regs);
    } else {
        printf("Unhandled exception: %d\n", n);
        printf("    errcode: 0x%x\n", code);
        dump_regs(regs);
        while(1);
    }
}

static void
irq_handler(uint32_t irq, const struct regs *regs)
{
    if (irq_handlers[irq]) {
        irq_handlers[irq](irq, regs);
    } else {
        irq_ack(irq);
    }
}


/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

void
idt_install()
{
    /* Create the exceptions entries */
    idt_set_entry(0, (uint32_t) isr0, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(1, (uint32_t) isr1, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(2, (uint32_t) isr2, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(3, (uint32_t) isr3, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(4, (uint32_t) isr4, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(5, (uint32_t) isr5, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(6, (uint32_t) isr6, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(7, (uint32_t) isr7, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(8, (uint32_t) isr8, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(9, (uint32_t) isr9, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(10, (uint32_t) isr10, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(11, (uint32_t) isr11, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(12, (uint32_t) isr12, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(13, (uint32_t) isr13, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(14, (uint32_t) isr14, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(15, (uint32_t) isr15, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(16, (uint32_t) isr16, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(17, (uint32_t) isr17, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(18, (uint32_t) isr18, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(19, (uint32_t) isr19, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(20, (uint32_t) isr20, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(21, (uint32_t) isr21, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(22, (uint32_t) isr22, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(23, (uint32_t) isr23, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(24, (uint32_t) isr24, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(25, (uint32_t) isr25, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(26, (uint32_t) isr26, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(27, (uint32_t) isr27, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(28, (uint32_t) isr28, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(29, (uint32_t) isr29, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(30, (uint32_t) isr30, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(31, (uint32_t) isr31, 0x08, IDT_EF_P | IDT_EF_INT);    
    
    /* Create the irqs entries */
    idt_set_entry(32, (uint32_t) isr32, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(33, (uint32_t) isr33, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(34, (uint32_t) isr34, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(35, (uint32_t) isr35, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(36, (uint32_t) isr36, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(37, (uint32_t) isr37, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(38, (uint32_t) isr38, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(39, (uint32_t) isr39, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(40, (uint32_t) isr40, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(41, (uint32_t) isr41, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(42, (uint32_t) isr42, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(43, (uint32_t) isr43, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(44, (uint32_t) isr44, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(45, (uint32_t) isr45, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(46, (uint32_t) isr46, 0x08, IDT_EF_P | IDT_EF_INT);
    idt_set_entry(47, (uint32_t) isr47, 0x08, IDT_EF_P | IDT_EF_INT);

    idt_set_entry(100, (uint32_t) isr100, 0x08, IDT_EF_P | IDT_EF_INT | IDT_EF_U);

    /* Remap irqs */
    outb(MASTER_IRQ_COMMAND, 0x11);     /* initialize master IRQ */
    outb(SLAVE_IRQ_COMMAND, 0x11);      /* initialize slave IRQ */
    outb(MASTER_IRQ_DATA, 0x20);        /* vector offset */
    outb(SLAVE_IRQ_DATA, 0x28);         /* vector offset */
    outb(MASTER_IRQ_DATA, 0x04);        /* tell there's slave IRQ at 0x0100 */
    outb(SLAVE_IRQ_DATA, 0x02);         /* tell it's cascade identity */
    outb(MASTER_IRQ_DATA, 0x01);        /* 8086 mode */
    outb(SLAVE_IRQ_DATA, 0x01);         /* 8086 mode */
    
    /* Disable all irqs */
    outb(MASTER_IRQ_DATA, 0xFF);
    outb(SLAVE_IRQ_DATA, 0xFF);
    
    /* Install the new idt */
    idt_ptr.base = (uint32_t) idt;
    idt_ptr.limit = sizeof(idt) - 1;
    idt_flush(&idt_ptr);

    /* Activate the interrupts */
    asm volatile ("sti");
}

int
idt_set_entry(uint32_t n, uint32_t offset, uint8_t selector, uint8_t flags)
{
    if (n >= IDT_MAX_ENTRIES) return 0;

    idt[n].offset_low = (offset & 0x0000FFFF);
    idt[n].offset_high = (offset & 0xFFFF0000) >> 16;

    idt[n].zero = 0;
    idt[n].selector = selector;
    idt[n].type_attr = flags;
    
    return 1;
}

void
irq_ack(uint32_t irq)
{
    if (irq >= 8)
        outb(SLAVE_IRQ_COMMAND, 0x20);
    outb(MASTER_IRQ_COMMAND, 0x20);
}

void
interrupt_handler(struct isr_frame frame)
{
    if (frame.int_no < 32) {
        exception_handler(frame.int_no, frame.errorcode, &frame.regs);
    } else if (frame.int_no == 100) {
        syscall_handler(frame);
    } else {
        irq_handler(frame.int_no - 32, &frame.regs);
    }
}

void
enable_irq(uint32_t irq)
{
    if (irq < 8) {
        uint8_t mask = inb(MASTER_IRQ_DATA);
        outb(MASTER_IRQ_DATA, mask - (1 << irq));
    } else {
        uint8_t mask = inb (SLAVE_IRQ_DATA);
        outb(SLAVE_IRQ_DATA, mask - (1 << irq));
    }
}

void
set_irq_handler(uint32_t irq, irq_handler_t handler)
{
    if (irq < 16) irq_handlers[irq] = handler;
}

void
set_exception_handler(uint32_t n, exception_handler_t handler)
{
    if (n < 32) exception_handlers[n] = handler;
}
