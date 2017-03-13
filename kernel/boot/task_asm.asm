global user_space_switch
global tss_flush
global vm86_jump
global vm86_bios
global __sched

EFLAGS_IF       equ (1 << 9)
EFLAGS_VM       equ (1 << 17)
EFLAGS_IOPL3    equ (0x3000)

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

extern kprintf
; void __sched(struct regs regs, u32 eip, u32 cs, u32 eflags, u32 esp, u32 ss);
__sched:
    add esp, 4                  ; Skip return-EIP

    mov al, 0x20
    out 0x20, al

    mov eax, [esp + 12*4]       ; Use ss as the others segment registers.
                                ; (8 for regs structures, and 4 for eip, cs
                                ;  eflags and esp)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    push DWORD [esp + 4*4]      ; SS
    push DWORD [esp + 4*4]      ; ESP
    pushf                       ; EFLAGS
    or DWORD [esp], EFLAGS_IF   ; Set IF flag
    push DWORD [esp + 4*4]      ; CS
    push DWORD [esp + 4*4]      ; EIP
    iret

; void vm86_jump(struct regs r, u32 ip, u32 cs, u32 sp);
extern tasking_set_esp0
extern vm86tss_set_esp
vm86_jump:
    push ebp

    mov eax, cr4
    or eax, 1
    mov cr4, eax

    push esp
    call vm86tss_set_esp

    add esp, 12                  ; Skip saved eip, pushed esp and ebp
    popa
    mov ebp, esp

    push DWORD [ebp+4]           ; GS
    push DWORD [ebp+4]           ; FS
    push DWORD [ebp+4]           ; DS
    push DWORD [ebp+4]           ; ES
    push DWORD [ebp+4]           ; SS
    push DWORD [ebp+8]           ; ESP
    pushf
    or DWORD [esp], EFLAGS_VM    ; Set VM bit
    or DWORD [esp], EFLAGS_IOPL3 ; IOPL 3
    push DWORD [ebp+4]           ; CS
    push DWORD [ebp]             ; IP
    iret

extern out_of_vm86
global __out_of_vm86
; Before entering exception handler in virtual 8086 mode, the CPU push old GS,
; FS, DS, ES, SS, ESP, EFLAGS, CS, EIP. These are the old values, before the
; exception has been catched. So in order to get return EIP from the function
; vm86_jump, to continue executing, ESP must be increased by 10 * 4
__out_of_vm86:
    add esp, 10 * 4
    mov ebp, esp
    pusha

    xor eax, eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax

    push ebp
    add esp, 4

    call out_of_vm86

    sub esp, 4
    mov esp, ebp

    pop ebp
    jmp [esp]

; void user_space_switch(u32 eip, u32 esp, u32 ss, u32 cs);
extern enable_irq
user_space_switch:
    cli
    mov ebp, esp

    push esp
    call tasking_set_esp0
    add esp, 4

    push 0
    call enable_irq
    add esp, 8                  ; Skip return-EIP and enable_irq argument (0)

    xor eax, eax                ; Erase all registers
    mov ebx, eax
    mov ecx, eax
    mov edx, eax
    mov ebp, eax
    mov esi, eax
    mov edi, eax

    push DWORD [esp + 4*2]      ; SS
    push DWORD [esp + 4*2]      ; ESP
    pushf                       ; EFLAGS
    or DWORD [esp], EFLAGS_IF   ; Set IF flag
    push DWORD [esp + 4*6]      ; CS
    push DWORD [esp + 4*4]      ; EIP
    iret

[bits 16]
vm86_bios:
    jmp $
    int 0x10
    hlt
