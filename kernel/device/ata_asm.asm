SECTION .text
GLOBAL ata_send_identify

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
