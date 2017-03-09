#ifndef IO_H
#define IO_H

#define IO_BACKSPACE 0x08
#define IO_TABULATOR 0x09
#define IO_BLANK 0x20

void kputc(char c);
void kputs(char* s);
void kcls();

int kprintf(const char *format, ...);
int ksprintf(char *out, const char *format, ...);

#endif
