;-------------------------------------------------------------------------------
; SOURCE NAME   : memory.asm
; AUTHOR        : Aurélien Martin
; DESCRIPTION   : This file contains asm routines used in memory module
;-------------------------------------------------------------------------------

SECTION .text
GLOBAL __switch_pagedir

__switch_pagedir:
    mov dword eax, [esp+4]
    mov cr3, eax
    ret
