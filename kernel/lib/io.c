#include <kernel/io.h>
#include <boot/console.h>

void putc(char c)
{
    console_putchar(c);
}

void puts(char* s)
{
    while (*s != '\0')
    {
        putc(*s);
        s++;
    }
}

void cls()
{
    console_clear();
}
