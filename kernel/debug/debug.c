/*******************************************************************************
 * SOURCE NAME  : debug.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements debugging functions
 ******************************************************************************/

#include <kernel/cpu.h>
#include <stdio.h>

void dump_regs(const struct regs *r)
{
    printf("=== DUMP REGISTERS ===\n");
    printf("eax=0x%x, ebx=0x%x, ecx=0x%x, edx=0x%x\n",
           r->eax,
           r->ebx,
           r->ecx,
           r->edx);

    printf("esi=0x%x, edi=0x%x, esp=0x%x, ebp=0x%x\n",
           r->esi,
           r->edi,
           r->esp,
           r->ebp);
}
