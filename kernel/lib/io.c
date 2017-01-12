#include <kernel/io.h>

extern void console_clear();
extern void console_putchar(char *s);

void puts(char* s)
{
    while (*s != '\0')
    {
        s++;
        console_putchar(s);
    }
}

void cls()
{
    console_clear();
}
