#include "idt.h"
#include "gdt.h"

#include <alien/kernel.h>
#include <alien/io.h>
#include <alien/task.h>

#define MASTER_IRQ_COMMAND  0x20
#define MASTER_IRQ_DATA     0x21
#define SLAVE_IRQ_COMMAND   0xA0
#define SLAVE_IRQ_DATA      0xA1

/* All defined in idt_asm.asm */
extern void idt_flush(u32 gp);

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

struct idt_entry
{
    u16 offset_low;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_high;
} __attribute__((packed));

struct idt_ptr
{
    u16 limit;
    u32 base;
} __attribute__((packed));

static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr ip;

struct irq_desc
{
    struct irq_desc* next;
    irq_handler_t handler;
};

static struct irq_desc irq_descriptors[16];

void
idt_set_gate(u32 num, u32 offset, u8 selector, u8 flags)
{
    if (num >= IDT_SIZE)
    {
        kputs("[ERROR] idt_set_gate: Can't set idt gate above size.");
        return;
    }

    idt[num].offset_low = (offset & 0x0000FFFF);
    idt[num].offset_high = (offset & 0xFFFF0000) >> 16;

    idt[num].zero = 0;
    idt[num].selector = selector;
    idt[num].type_attr = flags;
}

void
idt_install()
{
    asm("cli");
    idt_set_gate(0, (u32) isr0, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(1, (u32) isr1, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(2, (u32) isr2, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(3, (u32) isr3, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(4, (u32) isr4, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(5, (u32) isr5, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(6, (u32) isr6, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(7, (u32) isr7, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(8, (u32) isr8, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(9, (u32) isr9, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(10, (u32) isr10, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(11, (u32) isr11, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(12, (u32) isr12, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(13, (u32) isr13, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(14, (u32) isr14, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(15, (u32) isr15, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(16, (u32) isr16, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(17, (u32) isr17, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(18, (u32) isr18, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(19, (u32) isr19, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(20, (u32) isr20, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(21, (u32) isr21, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(22, (u32) isr22, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(23, (u32) isr23, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(24, (u32) isr24, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(25, (u32) isr25, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(26, (u32) isr26, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(27, (u32) isr27, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(28, (u32) isr28, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(29, (u32) isr29, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(30, (u32) isr30, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(31, (u32) isr31, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);    
    
    idt_set_gate(32, (u32) isr32, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(33, (u32) isr33, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(34, (u32) isr34, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(35, (u32) isr35, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(36, (u32) isr36, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(37, (u32) isr37, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(38, (u32) isr38, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(39, (u32) isr39, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(40, (u32) isr40, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(41, (u32) isr41, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(42, (u32) isr42, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(43, (u32) isr43, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(44, (u32) isr44, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(45, (u32) isr45, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(46, (u32) isr46, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);
    idt_set_gate(47, (u32) isr47, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);

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
    
    idt_set_gate(100, (u32) isr100, K_CODE_SEL, IDT_EF_P | IDT_EF_INT | IDT_EF_U);

    ip.base = (u32) idt;
    ip.limit = sizeof (struct idt_entry) * IDT_SIZE - 1;
    idt_flush((u32) &ip);

    asm("sti");
}

static void
syscall_handler(u32 ds, interrupt_frame_t frame)
{
    if (frame.regs.eax == 0x00) {
		kprintf("%d\n", frame.regs.ebx);
	} else if (frame.regs.eax == 0x01) {
		fork(frame);
	} else {
		kprintf("unknown syscall!\n");
	}
}

void
interrupt_handler(u32 ds, interrupt_frame_t frame)
{
	if (frame.int_no < 32) {
		kprintf("int_no: %d\n",frame.int_no);
		kprintf("    errcode: 0x%x\n", frame.errorcode);
		dump_regs(frame.regs);
		while(1);
	} else if (frame.int_no == 100) {		
		syscall_handler(ds, frame);
	} else {
		frame.int_no -= 32;
		if (frame.int_no >= 8)
			outb(SLAVE_IRQ_COMMAND, 0x20);
		outb(MASTER_IRQ_COMMAND, 0x20);

		if (frame.int_no == 0) {
			sched(frame);
		}

		struct irq_desc* desc = &irq_descriptors[frame.int_no];
		while (desc->next != 0) {
			desc->handler();
			desc = desc->next;
		}
	}
}

void
enable_irq(u32 irq)
{
    if (irq < 8) {
        u8 mask = inb (MASTER_IRQ_DATA);
        outb (MASTER_IRQ_DATA, mask - (1 << irq));
    } else {
        u8 mask = inb (SLAVE_IRQ_DATA);
        outb (SLAVE_IRQ_DATA, mask - (1 << irq));
    }
}


