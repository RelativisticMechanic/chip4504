;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_clear_screen:
	pusha

	mov dx, 0
	call os_move_cursor

	mov ah, 6
	mov al, 0		
	mov bh, 0x02
	mov cx, 0
	mov dh, 24
	mov dl, 79
	int 10h

	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_print_string:
	pusha
	mov ah, 0x0E
.p_loop:
	lodsb
	cmp al, 0
	je .exit_loop
	int 0x10
	jmp .p_loop
.exit_loop:
	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SI - String
; OUT: AX - Length of string
os_string_length:
	pusha
	xor cx, cx
.s_loop:
	lodsb
	cmp al, 0
	je .done
	inc cx
	jmp .s_loop
.done:
	mov word [.len], cx
	popa
	mov ax, word [.len]
	ret
	[SECTION .data]
	.len dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SI - first string
; DI - Second string
os_string_compare:
	pusha
.c_loop:
	mov al, byte [si]
	mov bl, byte [di]
	
	cmp al, bl
	jne .not_same
	
	cmp al, 0
	je .same
	
	inc si
	inc di
	jmp .c_loop
.not_same:
	popa
	clc
	ret

.same:
	popa
	stc
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; AX - Key Out.
os_wait_for_key:
	xor ax, ax
	
	pusha
	
	mov ah, 0x10
	int 0x16
	
	mov word [.tmp_buf], ax
	
	popa
	mov ax, word [.tmp_buf]
	
	ret
	
	[SECTION .data]
	.tmp_buf dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; AX - Key out. No waiting
os_check_for_key:
	xor ax, ax
	
	pusha
	
	mov ah, 0x01
	int 0x16
	jz .no_key
	
	xor ax, ax
	int 0x16
	
	mov word [.tmp_buf], ax
	
	popa
	
	mov ax, word [.tmp_buf]
	ret
	
.no_key:
	popa
	
	xor ax, ax
	ret
	
	[SECTION .data]
	.tmp_buf dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DL, DH - X, Y.
os_move_cursor:
	pusha

	mov bh, 0
	mov ah, 2
	int 10h

	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Out: DL, DH - X, Y.
os_get_cursor_pos:
	pusha

	mov bh, 0
	mov ah, 3
	int 10h

	mov [.tmp], dx
	popa
	mov dx, [.tmp]
	ret

	[SECTION .data]
	.tmp dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_show_cursor:
	pusha
	mov ch, 6
	mov cl, 7
	mov ah, 1
	mov al, 3
	int 10h
	popa
	ret 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_hide_cursor:
	pusha
	mov ch, 32
	mov ah, 1
	mov al, 3
	int 10h
	popa
	ret
