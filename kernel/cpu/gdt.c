/*******************************************************************************
 * SOURCE NAME  : gdt.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide basic routines for managing the GDT, installing it and
 *  adding entries.
 ******************************************************************************/

#include <stdint.h>

#define MAX_ENTRIES 6


/*******************************************************************************
 *                          PRIVATE STRUCTURES
 ******************************************************************************/

struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

/* Defined in cpu.asm */
extern void gdt_flush(struct gdt_ptr *gdt_ptr);

static struct gdt_entry gdt[MAX_ENTRIES];
static struct gdt_ptr   gdt_ptr;


/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

int gdt_set_entry(uint32_t n, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran)
{
    if (n >= MAX_ENTRIES) return 0;

    gdt[n].base_low = (base & 0xFFFF);
    gdt[n].base_middle = (base >> 16) & 0xFF;
    gdt[n].base_high = (base >> 24) & 0xFF;

    gdt[n].limit_low = (limit & 0xFFFF);
    gdt[n].granularity = ((limit >> 16) & 0x0F);

    gdt[n].granularity |= (gran & 0xF0);
    gdt[n].access = access;

    return 1;
}

void gdt_install()
{
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel segments */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* User segments */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_ptr.base = (uint32_t) gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;

    gdt_flush(&gdt_ptr);
}
