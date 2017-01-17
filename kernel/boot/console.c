#include <boot/console.h>
#include <kernel/io.h>
#include <kernel/string.h>
#include <kernel/kernel.h>
#include <types.h>

static u16 *console_buffer = (u16*) 0xC00B8000;
static u8 console_color = 0x0F;
static u16 console_x = 0;
static u16 console_y = 0;

static void console_scroll()
{
    u8 blank, temp;

    blank = (u16) IO_BLANK | (u16) console_color << 8;
    if (console_y >= 25)
    {
        temp = console_y - 25 + 1;
        memcpy (console_buffer, console_buffer + temp * 80, (25 - temp) * 80 * 2);
        memsetw (console_buffer + (25 - temp) * 80, blank, 80);
        console_y = 25 - 1;
    }
}

static void console_update_cursor()
{
    u16 position = console_y * 80 + console_x;

    *(console_buffer + position) = (0x07 << 8) | ' ';

    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8) (position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8) ((position>>8) & 0xFF));
}

void console_clear()
{
    for (int i = 0; i < 25; i++)
        memset (console_buffer, 0, 80 * 25);

    console_x = 0;
    console_y = 0;
    console_update_cursor();
}

void console_set_color(const unsigned short color)
{
    console_color = color;
}

void console_putchar(const unsigned char c)
{
    u16 *where;

    if (c == IO_BACKSPACE)
    {
        if (console_x != 0)
            console_x--;
    }
    else if (c == IO_TABULATOR)
    {
        console_x = (console_x + 8) & ~(8 - 1);
    }
    else if (c == '\r' || c == '\n')
    {
        console_x = 0;
        if (c == '\n')
            console_y++;
    }
    else
    {
        where = console_buffer + (console_y * 80 + console_x);
        *where = (u16) c | (u16) console_color << 8;
        console_x++;
    }

    if (console_x >= 80)
    {
        console_x = 0;
        console_y++;
    }

    console_scroll();
    console_update_cursor();
}
