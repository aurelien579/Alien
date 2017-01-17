#ifndef IO_H
#define IO_H

#define IO_BACKSPACE 0x08
#define IO_TABULATOR 0x09
#define IO_BLANK 0x20

void putc(char c);
void puts(char* s);
void cls();

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#endif
