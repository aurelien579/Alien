ENTRY(entry)

SECTIONS {
    __KERNEL_VBASE__ = 0xC0000000;
    
    . = 0x100000 + __KERNEL_VBASE__;

    __KERNEL_START__ = . - __KERNEL_VBASE__;
    
    .text : AT(ADDR(.text) - __KERNEL_VBASE__)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata : AT (ADDR (.rodata) - __KERNEL_VBASE__)
    {
        *(.rodata*)
    }

    .data : AT (ADDR (.data) - __KERNEL_VBASE__)
    {
        *(.data)
    }

    .bss : AT (ADDR (.bss) - __KERNEL_VBASE__)
    {
        *(.bss)
    }
    
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment*)
    }

    __KERNEL_END__  = . - __KERNEL_VBASE__;
    __KERNEL_SIZE__ = __KERNEL_END__ - __KERNEL_START__;
}

