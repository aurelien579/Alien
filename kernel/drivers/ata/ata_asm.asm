;-------------------------------------------------------------------------------
; Source name   : ata_asm.asm
; Version       : 0.1
; Created data  : 24/10/2017
; Last update   : 26/10/2017
; Author        : Aurélien Martin
; Description   : This module provide routines for ata
;-------------------------------------------------------------------------------


SECTION .text
GLOBAL ata_read, iowait, ata_send_identify
EXTERN kprintf


;-------------------------------------------------------------------------------
; iowait : Wait 100 ns
;
; Updated       : 26/10/2017
; In            : Nothing
; Returns       : Nothing
; Modifies      : Nothing
; Description   : Wait 100 ns by executing an io operation

iowait:
    out 0x80, al
    ret


;-------------------------------------------------------------------------------
; ata_send_identify : Wait 100 ns
; 
; C Declaration : int ata_identify(struct ata_device *dev, u16 *buf);
; Updated       : 26/10/2017
; In            : - dev : An known device (undefined behavior if unknown)
;                 - buf : A buffer of 256 words
; Returns       : -1 if an error occured
; Modifies      : Nothing
; Description   : Send the identify command to the device and write the data to
;                 the buffer

ata_send_identify:
    push ebp
    mov ebp, esp
    
    push esi
    push edi
    
    mov esi, [ebp + 8]  ; Ata device ptr
    mov dx,  [esi + 1]  ; Base port
    mov edi, [ebp + 12] ; Buffer
    
    ; Select the drive
    add dx, 6           ; Drive select port
    mov al, 0xA0        ; Master
    or al, [esi + 5]    ; Or slavebit
    out dx, al
    
    ; Clear 2-5 ports
    mov al, 0
    dec dx              ; 5 port
    out dx, al
    dec dx
    out dx, al
    dec dx
    out dx, al
    dec dx
    out dx, al
    
    ; Send IDENTIFY command (0XA1 or 0xEC)
    mov al, 0xA1        ; IDENTIFY command for PI devices
    mov cl, [esi]       ; Read device type
    bt ecx, 0           ; Test pi device
    jc .pi_device    
    
    mov al, 0xEC        ; IDENTIFY command for non PI devices    
.pi_device:

    add dx, 5
    out dx, al    

.wait:
    in al, dx
    bt eax, 7           ; BSY bit
    jc .wait
    
    bt eax, 0           ; Check for error
    jc .error
    
    bt eax, 3
    jc .data_rdy
.wait_data:
    in al, dx
    
    bt eax, 0           ; Check for error
    jc .error
    
    bt eax, 3
    jc .data_rdy
    
.data_rdy:
    sub dl, 7           ; Data port
    mov ecx, 256
    rep insw            ; Read 256 words to EDI
    
    mov eax, 0
    jmp .end

.error:
    mov eax, -1
    
.end:    
    pop edi
    pop esi
    pop ebp
    
    ret

.error_msg: db 'ERROR in ata_send_identify', 20, 0

    
; void ata_read(u32 base_port, u32 lba, u32 sec_count);
sata_read:
    mov ebp, 0
    mov dx, 0x1F0
    mov bl, 1
    mov edi, [esp+4]
    
    mov ecx, ebp    ; save a working copy
    mov al, bl      ; set al= sector count (0 means 256 sectors)
    or dl, 2        ; dx = sectorcount port -- usually port 1f2
    out dx, al
    
    mov al, cl      ; ecx currently holds LBA
    inc edx         ; port 1f3 -- LBAlow
    out dx, al
    
    mov al, ch
    inc edx         ; port 1f4 -- LBAmid
    out dx, al
    
    bswap ecx
    
    mov al, ch      ; bits 16 to 23 of LBA
    inc edx         ; port 1f5 -- LBAhigh
    out dx, al
    
    mov al, cl      ; bits 24 to 28 of LBA
    or al, 0xe0
    ;or al, byte [esi + dd_sbits]   ; master/slave flag | 0xe0
    inc edx         ; port 1f6 -- drive select
    out dx, al

    inc edx         ; port 1f7 -- command/status
    mov al, 0x20    ; send "read" command to drive
    out dx, al

; ignore the error bit for the first 4 status reads -- ie. implement 400ns delay on ERR only
; wait for BSY clear and DRQ set
    push edx
    mov dx, 0x3F6
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    pop edx
    
    jmp $
    
    mov ecx, 4
.lp1:
    in al, dx           ; grab a status byte
    test al, 0x80       ; BSY flag set?
    jne short .retry
    test al, 8          ; DRQ set?
    jne short .data_rdy
.retry:
    dec ecx
    jg short .lp1
; need to wait some more -- loop until BSY clears or ERR sets (error exit if ERR sets)

.pior_l:
    in al, dx		; grab a status byte
    test al, 0x80		; BSY flag set?
    jne short .pior_l	; (all other flags are meaningless if BSY is set)
    test al, 0x21		; ERR or DF set?
    jne short .fail
.data_rdy:
; if BSY and ERR are clear then DRQ must be set -- go and read the data
    sub dl, 7		; read from data port (ie. 0x1f0)
    mov cx, 256
    rep insw		; gulp one 512b sector into edi
    or dl, 7		; "point" dx back at the status register
    in al, dx		; delay 400ns to allow drive to set new values of BSY and DRQ
    in al, dx
    in al, dx
    in al, dx

; After each DRQ data block it is mandatory to either:
; receive and ack the IRQ -- or poll the status port all over again

    inc ebp			; increment the current absolute LBA
    dec ebx			; decrement the "sectors to read" count
    test bl, bl		; check if the low byte just turned 0 (more sectors to read?)
    jne short .pior_l

    sub dx, 7		; "point" dx back at the base IO port, so it's unchanged
    ;sub ebp, [esi + dd_stLBA]	; convert absolute lba back to relative
; "test" sets the zero flag for a "success" return -- also clears the carry flag
    test al, 0x21		; test the last status ERR bits
    je short .done
.fail:
    push .error_msg
    call kprintf
    add esp, 4
    stc
.done:
    ret

.error_msg: db 'ata_read failed', 10, 0

; void atapi_read(u32 base_port, u32 lba, u32 sec_count)
atapi_read:

