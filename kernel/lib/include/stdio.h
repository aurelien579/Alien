/*******************************************************************************
 * SOURCE NAME  : stdio.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide standard io functions for the kernel. These functions
 *  actually use the boot console.
 ******************************************************************************/

#ifndef LIB_STDIO_H
#define LIB_STDIO_H

void putc(char str);
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#endif
