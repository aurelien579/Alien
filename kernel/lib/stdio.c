/*******************************************************************************
 * SOURCE NAME  : stdio.c
 * VERSION      : 0.1
 * CREATED DATE : 06/02/2018
 * LAST UPDATE  : 06/02/2018
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Implements most simple io functions.
 ******************************************************************************/

#include <kernel/boot/console.h>

void putc(char c)
{
    boot_putchar(c);
}
