;-------------------------------------------------------------------------------
; Source name   : idt_asm.asm
; Version       : 0.1
; Created data  : 22/10/2017
; Last update   : 24/10/2017
; Author        : Aurélien Martin
; Description   : This library provides routines for scheduling tasks
;-------------------------------------------------------------------------------

EFLAGS_IF       equ (1 << 9)
EFLAGS_VM       equ (1 << 17)
EFLAGS_IOPL3    equ (0x3000)

SECTION .text

GLOBAL user_space_switch, tss_flush, vm86_jump, vm86_bios, execute_task
GLOBAL __out_of_vm86
EXTERN kprintf, tasking_set_esp0, vm86tss_set_esp, out_of_vm86
EXTERN enable_irq

tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

; void __sched(task_info_t info);
execute_task:
    add esp, 12                 ; Skip return-EIP, PID and CR3
	mov ebp, esp
	
    mov al, 0x20
    out 0x20, al
    mov eax, [ebp + 11*4]       ; Use SS as the others segment registers.
                                ; (8 for regs structures, and 3 for EIP, CS
                                ;  EFLAGS)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    push DWORD [esp + 3*4]      ; SS
    push DWORD [esp + 5*4]      ; ESP
    pushf                       ; EFLAGS
    or DWORD [esp], EFLAGS_IF   ; Set IF flag
    push DWORD [esp + 5*4]      ; CS
    push DWORD [esp + 5*4]      ; EIP
    iret

; void vm86_jump(struct regs r, u32 ip, u32 cs, u32 sp);
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
user_space_switch:
    cli
    add esp, 4					; Skip return eip
    mov ebp, esp
    
    push 0						; IRQ number to enable
    call enable_irq
    add esp, 4					; Erase enable_irq argument
	
    xor eax, eax                ; Erase all registers
    mov ebx, eax
    mov ecx, eax
    mov edx, eax
    mov esi, eax
    mov edi, eax
    
    push DWORD [ebp + 4*2]      ; SS
    push DWORD [ebp + 4]      	; ESP
    pushf                       ; EFLAGS
    or DWORD [esp], EFLAGS_IF   ; Set IF flag
    push DWORD [ebp + 4*3]     	; CS
    push DWORD [ebp]      		; EIP
    
    xor ebp, ebp
    iret

[bits 16]
vm86_bios:
    jmp $
    int 0x10
    hlt
