
EXTERN __KERNEL_VBASE__
MB_MAGIC        equ  0x1BADB002
MB_MODALIGN     equ  1<<0
MB_MEMINFO      equ  1<<1
MB_FLAGS        equ  MB_MODALIGN | MB_MEMINFO
MB_CHECKSUM     equ -(MB_MAGIC + MB_FLAGS)

; This is the virtual base address of kernel space. It must be used to
; convert virtual addresses into physical addresses until paging is
; enabled. Note that this is not the virtual address where the kernel
; image itself is loaded -- just the amount that must be subtracted from
; a virtual address to get a physical address.

KERNEL_PAGE_NUMBER  equ (0xC0000000 >> 22) ; Page directory index of
                                                    ; kernel's 4MB PTE.
KERNEL_STACKSIZE    equ 0x8000


SECTION .multiboot
align 0x1000
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM


SECTION .data
align 0x1000
boot_page_directory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit
    ; physical address space.

    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately
    ; after paging is enabled because it can't fetch the next instruction!
    ; It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0         ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.


SECTION .text
GLOBAL loader, kernel_stack
EXTERN kernel_main, kprintf

; setting up entry point for linker
loader:
    mov ecx, (boot_page_directory - 0xC0000000)
    mov cr3, ecx                    ; Load Page Directory Base Register.

    mov ecx, cr4
    or ecx, 0x00000010              ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000              ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx

    ; Start fetching instructions in kernel space.
    ; Since eip at this point holds the physical address of this command
    ; (approximately 0x00100000) we need to do a long jump to the correct
    ; virtual address of StartInHigherHalf which is approximately 0xC0100000.
    lea ecx, [higher_half_loader]
    jmp ecx                         ; NOTE: Must be absolute jump!

higher_half_loader:
    ; Unmap the identity-mapped first 4MB of physical address space.
    ; It should not be needed anymore.
    mov dword [boot_page_directory], 0
    invlpg [0]

    ; NOTE: From now on, paging should be enabled. The first 4MB of physical
    ; address space is mapped starting at KERNEL_VIRTUAL_BASE. Everything is
    ; linked to this address, so no more position-independent code or funny
    ; business with virtual-to-physical address translation should be necessary.
    ; We now have a higher-half kernel.
    mov esp, kernel_stack                       ; Set up the stack
    
    cmp eax, 0x2BADB002
    jne .multiboot_error
    
    ; Pass Multiboot info structure
    ; WARNING: This is a physical address and may not be in the first 4MB!
    add ebx, __KERNEL_VBASE__
    push ebx

    call  kernel_main       ; call kernel proper
    
.multiboot_error:
    push .error_msg
    call kprintf
    
    jmp $
    
.error_msg: db 'Multiboot magic error', 0
    
SECTION .bss
align 0x20
kernel_stack_start:
    resb KERNEL_STACKSIZE
kernel_stack:
