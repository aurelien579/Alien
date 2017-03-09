#include <alien/io.h>
#include <alien/boot/console.h>

void kputc(char c)
{
    console_putchar(c);
}

void kputs(char* s)
{
    while (*s != '\0')
    {
        kputc(*s);
        s++;
    }
}

void kcls()
{
    console_clear();
}
