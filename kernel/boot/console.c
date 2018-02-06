#include "console.h"
#include <kernel/io.h>
#include <stdint.h>

#define COLOR 0x0F


/**
 * SECTION: Private variables
 */

static int x = 0;
static int y = 0;
static uint16_t *vgamem = (uint16_t *) 0xC00B8000;


/**
 * SECTION: Private functions
 */

static void vga_write(char c, uint8_t color)
{
    vgamem[y * 80 + x] = (color << 8) | c;
}

static void update_cursor()
{
    uint16_t pos = y * 80 + x;    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));

    vga_write(' ', 0x07);
}

/**
 * SECTION: Public functions
 */

void boot_clear()
{
    for (int i = 0; i < 25 * 80; i++)
        vgamem[i] = 0;
    
    x = 0;
    y = 0;
    update_cursor();
}

void boot_putchar(char c)
{    
    if (c == '\n') {
        x = 0;
        y++;
    } else {
        vga_write(c, COLOR);
        x++;
    }

    if (x >= 80) {
        x = 0;
        y++;
    }
    
    update_cursor();
}

void boot_print(const char *s)
{
    while (*s) boot_putchar(*s++);
}

