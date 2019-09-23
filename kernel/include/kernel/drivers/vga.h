/*******************************************************************************
 * SOURCE NAME  : vga.h
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : Exports public functions of the VGA driver
 ******************************************************************************/

#ifndef DRIVER_VGA_H
#define DRIVER_VGA_H

#include <stdint.h>

struct bitmap_font
{
	uint8_t     Width;		///< max. character width
	uint8_t     Height;		///< character height
	uint16_t    Chars;		///< number of characters in font
	const uint8_t   *Widths;	///< width of each character
	const uint16_t  *Index;	///< encoding to character index
	const uint8_t   *Bitmap;	///< bitmap of all characters
};

extern const struct bitmap_font font;

enum vga_mode
{
	VGA_MODE_BOOT,
    VGA_MODE_13
};

void vga_install();

void vga_set_mode(enum vga_mode mode);

void vga_clear_screen();
void vga_put_char(uint32_t x, uint32_t y, char c);
void vga_move_cursor(uint32_t x, uint32_t y);
void vga_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void vga_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                   uint32_t color);

#endif
