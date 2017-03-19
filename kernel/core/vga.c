#include <types.h>
#include "../boot/vm86.h"

struct vbe_info_block {
    char    vbe_signature[4];   /* VBE Signature ('VESA') */
    u16     vbe_version;        /* VBE Version */
    u32     oem_string_ptr;     /* vbe_far_ptr to oem string */
    u32     capabilities;       /* Capabilities of graphics controller */
    u32     video_mode_ptr;     /* vbe_far_ptr to video mode list */
    u16     total_memory;       /* Number of 64KB memory blocks */
    u8      reserved[236];
};

struct vbe_info_block block;

void vga_activate()
{
    struct regs r;
    struct regs *ret;

    r.eax = 0x4F00;

    vm86exec((u32) &vm86_bios, 500, r);

    kprintf("after vm86 !\n");

    ret = vm86_get_retregs();
    kprintf("eax : 0x%x\n", ret->eax);
    if ((ret->eax & 0xFF) != 0x4F) {
        kprintf("VESA error !\n");
    }
}
