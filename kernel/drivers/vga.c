/*******************************************************************************
 * SOURCE NAME  : vga.c
 * AUTHOR       : Aurélien Martin
 * DESCRIPTION  : A basic VGA driver.
 ******************************************************************************/

#include <kernel/drivers/vga.h>
#include <kernel/io.h>
#include <string.h>

struct vga_regs
{
    uint8_t general_regs[4];
    uint8_t sequencer_regs[5];
    uint8_t crtc_regs[25];
    uint8_t graphics_regs[9];
    uint8_t attribute_regs[21];
};

struct vga_context
{
    enum vga_mode mode;
    uint8_t *mem;
    uint32_t color;
};


/*******************************************************************************
 *                          PRIVATE VARIABLES
 ******************************************************************************/

static struct vga_regs mode_13_regs = {
    { 0x63, 0x00, 0x70, 0x04 },
    { 0x03, 0x21, 0x0F, 0x00, 0x0E },
    { 0x5F, 0x4F, 0x50, 0x82, 0x24, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x31, 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0x89, 0xA3, 
      0xFF },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF },
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
      0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00 }
};

static struct vga_context vga_context;


/*******************************************************************************
 *                          PRIVATE FUNCTIONS
 ******************************************************************************/

static inline void
write_misc_reg(uint8_t value)
{
    outb(0x3C2, value);
}

static inline void
write_graphics_reg(uint8_t index, uint8_t value)
{
    outb(0x3CE, index);
    outb(0x3CF, value);
}

static inline void
write_attribute_reg(uint8_t index, uint8_t value)
{
    outb(0x3C0, index);
    outb(0x3C0, value);
}

static inline void
write_sequencer_reg(uint8_t index, uint8_t value)
{
    outb(0x3C4, index);
    outb(0x3C5, value);
}

static inline void
write_crtc_reg(uint8_t index, uint8_t value)
{
    outb(0x3D4, index);
    outb(0x3D5, value);
}

static inline uint8_t
read_misc_reg()
{
    return inb(0x3CC);
}

static inline uint8_t
read_graphics_reg(uint8_t index)
{
    outb(0x3CE, index);
    return inb(0x3CF);
}

static inline uint8_t
read_attribute_reg(uint8_t index)
{
    /* Force attribute register internal flipflop to adddress mode. The next
    write to 0x3C0 will be an address */
    inb(0x3DA);

    /* Only bits 0-4 of register 0x3C0 are the address. Bit 5 is PAS */
    outb(0x3C0, index & 0x5F);
    return inb(0x3C1);
}

static inline uint8_t
read_sequencer_reg(uint8_t index)
{
    outb(0x3C4, index);
    return inb(0x3C5);
}

static inline uint8_t
read_crtc_reg(uint8_t index)
{
    outb(0x3D4, index);
    return inb(0x3D5);
}

static inline void
unlock_crtc_reg()
{
    outb(0x3D4, 0x03);
    outb(0x3D5, inb(0x3D5) | 0x80);
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & ~0x80);
}

static inline void
disable_display()
{
    // Disable screen
    write_sequencer_reg(0x01, read_sequencer_reg(0x01) | (1 << 5));

    // Reset sequencer
    write_sequencer_reg(0, read_sequencer_reg(0) & ~(1 << 1));
}

static inline void
enable_display()
{
    // Enable screeen
    write_sequencer_reg(0x01, read_sequencer_reg(0x01) & ~(1 << 5));
    
    // Enable sequencer
    write_sequencer_reg(0, read_sequencer_reg(0) | (1 << 1));
}

static void
vga_write_regs(const struct vga_regs *regs)
{
    // TODO: try to disable output before changing registers
    disable_display();

    write_misc_reg(regs->general_regs[0]);

    for (uint8_t i = 0; i < sizeof(regs->attribute_regs); i++) {
        write_attribute_reg(i, regs->attribute_regs[i]);
    }

    for (uint8_t i = 0; i < sizeof(regs->sequencer_regs); i++) {
        write_sequencer_reg(i, regs->sequencer_regs[i]);
    }

    for (uint8_t i = 0; i < sizeof(regs->graphics_regs); i++) {
        write_graphics_reg(i, regs->graphics_regs[i]);
    }
    
    unlock_crtc_reg();
    for (uint8_t i = 0; i < sizeof(regs->crtc_regs); i++) {
        write_crtc_reg(i, regs->crtc_regs[i]);
    }

    enable_display();
}

static inline void
mode_13_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    vga_context.mem[x + y * 320] = color;
}

static inline void
mode_13_clear_screen()
{
    memset(vga_context.mem, 0, 320 * 200);
}

static void
mode_13_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    uint8_t *ptr = vga_context.mem;

    // Place pointer on top left coordinates of the rect
    ptr += (x + y * 320);

    // Draw line by line. Each line is a memset call
    for (uint32_t row = 0; row < h; row++) {
        memset(&ptr[row * 320], color, w);
    }
}

static void
mode_13_put_char(uint32_t base_x, uint32_t base_y, char c)
{
    // bitmap start at dollar sign (31 is ascii table)
    // each character is 8 x 16 pixels
    uint16_t index = (c - 31) * 16;

    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 16; y++) {
            if (font.Bitmap[index + y] & (1 << x)) {
                // 63 is white (64 colors)
                mode_13_put_pixel(base_x + x, base_y + y, 63);
            }
        }
    }
}

static void
mode_text_move_cursor(uint32_t x, uint32_t y)
{
    uint16_t pos = y * 80 + x;
    write_crtc_reg(0x0F, (uint8_t) (pos & 0xFF));
    write_crtc_reg(0x0E, (uint8_t) ((pos >> 8) & 0xFF));
}

static void
mode_text_put_char(uint32_t x, uint32_t y, char c)
{
    ((uint16_t*) vga_context.mem)[y * 80 + x] = (vga_context.color << 8) | c;
}

static inline void
mode_text_clear_screen()
{
    memset(vga_context.mem, 0, 80 * 25 * 2);
}


/*******************************************************************************
 *                          PUBLIC FUNCTIONS
 ******************************************************************************/

void
vga_install()
{
    vga_context.mem = (uint8_t *) 0xC00B8000;
    vga_context.color = 0x0F;
    vga_context.mode = VGA_MODE_BOOT;

    // Color
    //outb(0x3C8, 0x05);
    //outb(0x3C9, 0xFF);
    //outb(0x3C9, 0x20);
    //outb(0x3C9, 0x20);
}

void
vga_set_mode(enum vga_mode mode)
{
    switch (mode) {
        case VGA_MODE_13:
            vga_write_regs(&mode_13_regs);
            outb(0x3C0, 0x20);
            vga_context.mem = (uint8_t *) 0xC00A0000;
            break;
    }

    vga_context.mode = mode;

    vga_clear_screen();
}

void
vga_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    switch (vga_context.mode) {
        case VGA_MODE_13:
            mode_13_put_pixel(x, y, color);
            break;
    }
}


void
vga_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    switch (vga_context.mode) {
        case VGA_MODE_13:
            mode_13_fill_rect(x, y, w, h, color);
            break;
    }
}

void
vga_clear_screen()
{
    disable_display();

    switch (vga_context.mode) {
        case VGA_MODE_13:
            mode_13_clear_screen();
            break;
    }

    enable_display();
}

void
vga_put_char(uint32_t x, uint32_t y, char c)
{
    switch (vga_context.mode) {
        case VGA_MODE_13:
            mode_13_put_char(x, y, c);
            break;
        case VGA_MODE_BOOT:
            mode_text_put_char(x, y, c);
            break;
    }
}

void
vga_move_cursor(uint32_t x, uint32_t y)
{
    switch (vga_context.mode) {
        case VGA_MODE_BOOT:
            mode_text_move_cursor(x, y);
            break;
    }
}