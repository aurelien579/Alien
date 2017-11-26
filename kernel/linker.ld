ENTRY(loader)

SECTIONS {
    __KERNEL_VBASE__ = 0xC0000000;
    
    . = __KERNEL_VBASE__ + 0x100000;

    __KERNEL_START__ = . - __KERNEL_VBASE__;
    .text ALIGN (4K) : AT(ADDR(.text) - __KERNEL_VBASE__)
    {
       *(.multiboot)
       *(.text)
    }

    .rodata ALIGN (4K) : AT (ADDR (.rodata) - __KERNEL_VBASE__)
    {
        *(.rodata)
    }

    .data ALIGN (4K) : AT (ADDR (.data) - __KERNEL_VBASE__)
    {
        *(.data)
    }

    .bss ALIGN (4K) : AT (ADDR (.bss) - __KERNEL_VBASE__)
    {
        *(.bss)
    }

    __KERNEL_END__  = . - __KERNEL_VBASE__;
    __KERNEL_SIZE__ = __KERNEL_END__ - __KERNEL_START__;
}
