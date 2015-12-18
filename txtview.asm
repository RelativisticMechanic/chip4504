;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Small text viewer
; Max text file size: 64K
[SECTION .text]
view_text_file:
	; BX contains size of file
	mov ax, 0x3000
	mov fs, ax
	
	xor cx, cx
	mov word [.skiplines], 0
	
	pusha
	
	mov ax, .text_top_string
	mov bx, .text_bottom_string
	call os_draw_back
	
	popa
	
.txt_start:
	pusha
	
	mov bl, 11110000b
	mov dh, 2
	mov dl, 0
	mov si, 80
	mov di, 23
	call os_draw_block
	
	mov dl, 0
	mov dh, 2
	call os_move_cursor
	
	popa
	
	mov si, 0
	mov ah, 0x0E

.redraw:
	
	cmp cx, 0
	je .loop_text 
	dec cx
	
.loop_skip:
	
	mov al, byte [fs:si]
	inc si
	cmp al, 10
	jne .loop_skip
	
	jmp .redraw
	
.loop_text:
	
	mov al, byte [fs:si]
	inc si
	
	cmp al, 10
	jne .skip_return
	call os_get_cursor_pos
	mov dl, 0
	call os_move_cursor
	
.skip_return:
	int 0x10
	cmp si, bx
	je .finished
	
	call os_get_cursor_pos
	cmp dh, 23
	je .get_input
	
	jmp .loop_text
	
.get_input:
	
	call os_wait_for_key
	
	cmp ah, 72
	je .go_up
	cmp ah, 80
	je .go_down
	cmp al, 'q'
	je c4504_ui
	
	jmp .get_input
	
.go_up:
	
	cmp word [.skiplines], 0
	jle .txt_start
	dec word [.skiplines]
	mov cx, word [.skiplines]
	jmp .txt_start
	
.go_down:
	
	inc word [.skiplines]
	mov cx, word [.skiplines]
	jmp .txt_start
	
.finished:
	call os_wait_for_key
	cmp ah, 72
	je .go_up
	cmp al, 'q'
	je c4504_ui
	jmp .finished
	
	[SECTION .data]
	.skiplines: dw 0
	.text_top_string: db 'CHIP4504 Text Viewer', 0
	.text_bottom_string: db 'Press Q to quit. Up/Down keys to navigate', 0
	[SECTION .text]