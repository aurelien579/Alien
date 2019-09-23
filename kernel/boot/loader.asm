;-------------------------------------------------------------------------------
; SOURCE NAME   : loader.asm
; VERSION       : 0.1
; CREATED DATE  : 06/02/2018
; LAST UPDATE   : 06/02/2018
; AUTHOR        : Aurélien Martin
; DESCRIPTION   : This is the kernel's loader. It maps the kernel at
;  __KERNEL_VBASE__ using a basic page table.
;-------------------------------------------------------------------------------

EXTERN __KERNEL_VBASE__, __KERNEL_END__

MB_MAGIC            equ 0x1BADB002
MB_MODALIGN         equ 1 << 0
MB_MEMINFO          equ 1 << 1
MB_USE_GFX	        equ 1 << 2
MB_FLAGS            equ MB_MODALIGN | MB_MEMINFO | MB_USE_GFX
MB_CHECKSUM         equ -(MB_MAGIC + MB_FLAGS)

MB_MODE_TYPE        equ 0
MB_WIDTH            equ 0
MB_HEIGHT           equ 0
MB_DEPTH            equ 0

KERNEL_PAGE_NUMBER  equ (0xC0000000 >> 22) ; Page directory index of kernel's 4MB PTE.
KERNEL_STACKSIZE    equ 0x8000



SECTION .multiboot
align 0x1000
    ; Required fields
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

    ; Unused fields
    dd 0 ; header_addr
	dd 0 ; load_addr
	dd 0 ; load_end_addr
	dd 0 ; bss_end_addr
	dd 0 ; entry_addr

	; Graphics fields
	dd MB_MODE_TYPE       ; 0 = linear graphics
	dd MB_WIDTH
	dd MB_HEIGHT
	dd MB_DEPTH           ; Set me to 32 or else.



SECTION .text
GLOBAL entry
EXTERN kernel_main

entry:
    mov ecx, (boot_page_directory - 0xC0000000)
    mov dword [ecx], 0x00000083                             ; Identity map the kernel
    mov dword [ecx + KERNEL_PAGE_NUMBER * 4], 0x00000083    ; Also map it at the actual offset

    mov ecx, (boot_page_directory - 0xC0000000)

    mov cr3, ecx                    ; Load Page Directory Base Register.

    mov ecx, cr4
    or ecx, 0x00000010              ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000              ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx

    lea ecx, [higher_half_loader]
    jmp ecx                         ; NOTE: Must be absolute jump!

higher_half_loader:
    mov dword [boot_page_directory], 0
    invlpg [0]

    mov esp, kernel_stack

    add ebx, __KERNEL_VBASE__
    push ebx
    call  kernel_main       ; call kernel proper

    jmp $



SECTION .bss
GLOBAL kernel_stack

align 4096
boot_page_directory:
    resb 4096

kernel_stack_start:
    resb KERNEL_STACKSIZE
kernel_stack:
