;-------------------------------------------------------------------------------
; Source name   : task_asm.asm
; Author        : Aurélien Martin
; Description   : This library provides routines for scheduling tasks
;-------------------------------------------------------------------------------

EFLAGS_IF       equ (1 << 9)
EFLAGS_VM       equ (1 << 17)
EFLAGS_IOPL3    equ (0x3000)

SECTION .text

GLOBAL user_space_switch, tss_flush, execute_task
EXTERN printf, tasking_set_esp0
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
