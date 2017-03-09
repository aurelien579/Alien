global user_space_switch
global vm86_set_video_mode
global tss_flush
global vm86_jump

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

extern idt_set_gate
vm86_jump:
    mov eax, cr4
    or eax, 1
    mov cr4, eax

;idt_set_gate(13, (u32) isr13, K_CODE_SEL, IDT_EF_P | IDT_EF_INT);

    mov eax, [esp+4]            ; IP
    mov ebx, [esp+8]            ; CS
    mov ecx, eax                ; SP
    add ecx, 0xFFF

    push 0x0                    ; SS
    push ecx                    ; SP
    pushf
    or dword [esp], 0x20000     ; Set VM bit
    or dword [esp], 0x3000      ; IOPL 3
    push ebx                    ; CS
    push eax                    ; IP
    iret

extern out_of_vm86
global __out_of_vm86
__out_of_vm86:
    xor eax, eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax

    call out_of_vm86

user_space_switch:
    mov eax, cr4
    or eax, 1
    mov cr4, eax

    push 0    ; ss
    push 0xfff     ; esp
    pushf
    or dword [esp], 0x20000     ; Set VM bit
    or dword [esp], 0x3000      ; IOPL 3
    push 0x1000       ; cs
    push 0x0
    iret

vm86_set_video_mode:
    xor ax, ax
    mov ax, 0x1000
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov es, ax

    mov ah, 0x0
    mov al, 0x13
    ;jmp $
    ;int 0x10
    hlt
    jmp $
