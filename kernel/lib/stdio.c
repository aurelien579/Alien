/*******************************************************************************
 * SOURCE NAME  : stdio.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements most simple io functions.
 ******************************************************************************/

#include <kernel/boot/console.h>

void
putc(char c)
{
    boot_putchar(c);
}
