#include "gdt.h"
#include <kernel/io.h>

/* Defined in gdt_asm.asm */
extern void gdt_flush(u32 gp);

struct gdt_entry
{
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed));

struct gdt_ptr
{
    u16 limit;
    u32 base;
} __attribute__((packed));

static struct gdt_entry gdt[GDT_SIZE];
static struct gdt_ptr gp;

void
gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran)
{
    if (num >= GDT_SIZE) {
        puts("[ERROR] gdt_set_gate: Can't set gdt gate above size.");
        return;
    }

    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void
gdt_install()
{
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gp.base = (u32) gdt;
    gp.limit = GDT_SIZE * sizeof(struct gdt_entry) - 1;

    gdt_flush((u32) &gp);
}
