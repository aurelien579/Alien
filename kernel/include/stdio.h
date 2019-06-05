/*******************************************************************************
 * SOURCE NAME  : stdio.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Provide standard io functions for the kernel. These functions
 *  actually use the boot console.
 ******************************************************************************/

#ifndef LIB_STDIO_H
#define LIB_STDIO_H

#include <stdarg.h>

void putc(char str);

int printf(const char *format, ...);
int vprintf(const char *format, va_list args);

int sprintf(char *out, const char *format, ...);

#endif
