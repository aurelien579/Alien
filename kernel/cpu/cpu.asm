;-------------------------------------------------------------------------------
; SOURCE NAME   : cpu.asm
; AUTHOR        : Aurélien Martin
; DESCRIPTION   : This library manage gdt, interrupts and IRQs handling
;-------------------------------------------------------------------------------

SECTION .text
GLOBAL gdt_flush, idt_flush
EXTERN interrupt_handler


;-------------------------------------------------------------------------------
;                                    GDT
;-------------------------------------------------------------------------------

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.ret
.ret:
    ret



;-------------------------------------------------------------------------------
;                                    IDT
;-------------------------------------------------------------------------------

idt_flush:
    push eax
    mov eax, [esp+8]
    lidt [eax]
    pop eax
    ret

%macro ISR_ERRCODE 1
GLOBAL isr%1
isr%1:
    cli
    push byte %1
    jmp isr_common
%endmacro

%macro ISR_NOERRCODE 1
GLOBAL isr%1
isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47


ISR_NOERRCODE 100

;-------------------------------------------------------------------------------
; isr_common : The common part in all ISRs
;
; Updated       : 24/10/2017
; In            : On the stack there must be :
;                       --------------
;                       |     SS     | (If stack change occured)
;                       |     ESP    | (If stack change occured)
;                       |   EFLAGS   |
;                       |     CS     |
;                       |     EIP    | <-- ESP must be there before IRET
;                       | Error Code |
;                       | ISR number |
;                       
; Returns       : Nothing
; Modifies      : Nothing
; Description   : After specifics macros, all default ISRs JMP to this routine.
;                 After the IRET, the CPU is in the same state as before the
;                 interrupt occured.

isr_common:
    pusha

    push ds        ; save DS

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call interrupt_handler

    pop ebx         ; restore DS
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa

    add esp, 8		; Skipp error code and ISR number
    sti
    iret
