#include <kernel/io.h>
#include <kernel/string.h>
#include <kernel/kernel.h>

static const unsigned short *console_buffer = (unsigned short*) 0xC00B8000;
static unsigned char console_color = 0x0A;
static unsigned short console_x = 0;
static unsigned short console_y = 0;

/*static void scroll()
{
    unsigned char blank, temp;
    blank = IO_BLANK | (attrib << 8);
    if (console_y >= 25)
    {
        temp = console_y - 25 + 1;
        memcpy (textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);
        memsetw (textmemptr + (25 - temp) * 80, blank, 80);
        console_y = 25 -1;
    }
}*/

static void console_update_cursor()
{
    unsigned int temp;

    /* index = [(y * width) + x] */
    temp = console_y * 80 + console_x;

   /* This sends a command to indicies 14 and 15 in the
    *  CRT Control Register of the VGA controller. These
    *  are the high and low bytes of the index that show
    *  where the hardware cursor is to be 'blinking'. To
    *  learn more, you should look up some VGA specific
    *  programming documents. A great start to graphics:
    *  http://www.brackeen.com/home/vga */
    outportb(0x3D4, 14);
    outportb(0x3D5, temp >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, temp);
}

void console_clear()
{
    unsigned char blank;
    blank = IO_BLANK | (console_color << 8);
    for (int i = 0; i < 25; i++)
        memset (console_buffer + 1 * 80, blank, 80);

    /* update virtual cursor then move hardware cursor */
    console_x = 0;
    console_y = 0;
    console_update_cursor();
}

void console_putchar(const unsigned char c)
{
    unsigned short *where;
    unsigned short att = console_color << 8;

    if (c == IO_BACKSPACE)
    {
        if(console_x != 0)
            console_x--;
    }
    else if (c == IO_TABULATOR)
    {
        console_x = (console_x + 8) & ~(8 - 1);
    }
    else if (c == '\r' || c == '\n') /* Carriage return and newline */
    {
        console_x = 0;
        if (c == '\n')
            console_y++;
    }
    else if (c >= ' ')
    {
        where = console_buffer + (console_y * 80 + console_x);
        *where = c | att; /* Charater AND attributes: color */
        console_x++;
    }

    if (console_x >= 80)
    {
        console_x = 0;
        console_y++;
    }

    //scroll();
    console_update_cursor();
}
