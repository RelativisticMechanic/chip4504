; UI for the CHIP4504 Operating System
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; os_draw_block -- Render block of specified colour
; IN: BL/DL/DH/SI/DI = colour/start X pos/start Y pos/width/finish Y pos
[SECTION .text]
%define ITEMS_PER_PAGE 8
%define STARTING_PAGE 8
os_draw_block:
	pusha
.more:
	call os_move_cursor
	mov ah, 0x09
	mov bh, 0
	mov cx, si
	mov al, ' '
	int 10h
	inc dh
	mov ax, 0
	mov al, dh
	cmp ax, di
	jne .more
	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; os_draw_back
; AX - Up text, BX - Bottom text
os_draw_back:
	pusha
	
	call os_clear_screen
	
	push bx
	push ax
	
	mov bl, 0x70
	mov dl, 0
	mov dh, 1
	mov si, 80
	mov di, 24
	call os_draw_block
	
	mov bl, 0x0F
	mov dl, 0
	mov dh, 0
	mov si, 80
	mov di, 1
	call os_draw_block
	
	mov bl, 0x0F
	mov dl, 0
	mov dh, 24
	mov si, 80
	mov di, 25
	call os_draw_block
	
	mov dl, 0
	mov dh, 0
	call os_move_cursor
	
	pop ax
	mov si, ax
	call os_print_string
	
	mov dl, 0
	mov dh, 24
	call os_move_cursor
	
	pop ax
	mov si, ax
	call os_print_string
	
	popa
	ret 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_prepare_UI:
	
	mov ax, UI_top_string
	mov bx, UI_bottom_string
	call os_draw_back
	
	ret
	
	[SECTION .data]
	UI_top_string: db '[CHIP4504 Operating System (C) Benderx2, http://github.com/Benderx2/]', 0
	UI_bottom_string: db '[Running on x86-16, Version: ', VERSION_STRING, ' ]', 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; os_gen_file_list
; Generate a file list.
; Whenever a file is selected and ENTER is pressed, SI is returned as the file name.
os_gen_file_list:
	pusha
	
	mov byte [.current_y], 8
	mov word [.no_of_entries], 0
	mov word [.no_of_pages], 1
	mov word [.current_page], 1
	
	mov ax, file_list
	call os_list_files
	
	mov cx, 1
	mov si, file_list
.count:
	lodsb
	cmp al, 0
	je .done
	cmp al, ','
	jne .count
	inc cx
	jmp .count
.done:
	mov word [.no_of_entries], cx
	xor dx, dx
	mov ax, word [.no_of_entries]
	cmp ax, 8
	jle .one_page_only
	mov cx, ITEMS_PER_PAGE
	div cx
	cmp dx, 0
	je .no_remainder
	add ax, 1
.no_remainder:
	mov word [.no_of_pages], ax
	jmp .display_files
.one_page_only:
	mov word [.no_of_pages], 1
.display_files:
	mov bl, 0x1F
	mov dl, 24
	mov dh, 2
	mov si, 32
	mov di, 22
	call os_draw_block
	
	mov bl, 0xF0
	mov dl, 25
	mov dh, 7
	mov si, 30
	mov di, 20
	call os_draw_block
	
	mov dh, 3
	mov dl, 25
	call os_move_cursor
	
	mov si, .list_help_1
	call os_print_string
	
	mov dh, 4
	mov dl, 25
	call os_move_cursor
	
	mov si, .list_help_2
	call os_print_string
	
	mov dh, 5
	mov dl, 25
	call os_move_cursor
	
	mov si, .list_help_3
	call os_print_string
	
	call os_hide_cursor
	; get current page
	xor dx, dx
	mov ax, 8
	mov cx, word [.current_page]
	dec cx
	mul cx
	; got the starting point, now to find it where the name begins in the file list
	mov si, file_list
	mov cx, ax
.find_start_of_name:
	cmp cx, 0
	je .done_find
	lodsb
	cmp al, ','
	je .found_sep
	cmp al, 0
	je .found_sep
	jmp .find_start_of_name
.found_sep:
	dec cx
	jmp .find_start_of_name
.done_find:
	; SI = Start of file list.
	mov cx, ITEMS_PER_PAGE
.loop_begin:
	mov di, .temp_file_name
	mov dl, 30
	mov dh, byte [.current_y]
	call os_move_cursor
.name_loop:
	mov al, byte [si]
	inc si
	cmp cx, 0
	je .listing_complete
	cmp al, 0
	je .listing_complete
	cmp al, ','
	je .name_complete
	mov byte [di], al
	inc di
	jmp .name_loop
.name_complete:
	mov byte [di], 0
	push si
	mov si, .temp_file_name
	call os_print_string
	pop si
	add byte [.current_y], 1
	dec cx
	jmp .loop_begin
.listing_complete:
	mov byte [di], 0
	mov si, .temp_file_name
	call os_print_string
.begin_poll:
	mov byte [.current_y], 8
.polling:
	mov dl, 25
	mov dh, 20
	call os_move_cursor
	mov si, .first_page_string
	call os_print_string
	mov ax, word [.current_page]
	call os_int_to_string
	mov si, ax
	call os_print_string
	mov si, .second_page_string
	call os_print_string
	mov ax, word [.no_of_pages]
	call os_int_to_string
	mov si, ax 
	call os_print_string
	
	mov dl, 26
	mov dh, byte [.current_y]
	call os_move_cursor
	
	mov cx, 1
	mov bh, 0
	mov bl, 0xF4
	
	mov ah, 0x09
	mov al, '$'
	int 0x10
	
	call os_wait_for_key
	
	cmp al, 'w'
	je .go_up
	
	cmp al, 's'
	je .go_down
	
	cmp al, 'd'
	je .go_next
	
	cmp al, 'a'
	je .go_previous
	
	cmp al, 13
	je .go_enter
	
	jmp .polling
.go_up:
	mov al, byte [.current_y]
	sub al, STARTING_PAGE
	cmp al, 0
	je .polling
	call .remove_cur
	dec byte [.current_y]
	jmp .polling
.go_down:
	xor bx, bx
	mov bl, byte [.current_y]
	sub bl, STARTING_PAGE
	; End of Page?
	cmp bl, 7
	je .polling
	; Is this the last entry?
	mov ax, word [.current_page]
	sub ax, 1
	mov cx, ITEMS_PER_PAGE
	mul cx
	add ax, bx
	inc ax
	cmp ax, word [.no_of_entries]
	je .polling
	call .remove_cur
	inc byte [.current_y]
	jmp .polling
.go_previous:
	mov ax, word [.current_page]
	cmp ax, 1
	je .polling
	dec word [.current_page]
	mov byte [.current_y], 8
	jmp .display_files
.go_next:
	mov ax, word [.current_page]
	mov bx, word [.no_of_pages]
	cmp ax, bx
	je .polling
	inc word [.current_page]
	mov byte [.current_y], 8
	jmp .display_files
.go_enter:
	; Get shit mate
	xor dx, dx
	xor bx, bx
	mov ax, word [.current_page]
	dec ax
	mov bl, byte [.current_y]
	sub bl, STARTING_PAGE
	mov cx, 8
	mul cx
	add ax, bx
	mov dx, ax
	; Got the index! Now to find the file
	.find_filename:
		mov si, file_list
	.find_fileloop:
		cmp dx, 0
		je .found_file_name
		lodsb
		cmp al, 0
		je .wtf_err
		cmp al, ','
		je .found_sep2
		jmp .find_fileloop
	.wtf_err:
		call os_clear_screen
		mov ah, 0x0E
		mov al, 'E'
		int 0x10
		cli
		hlt
		jmp $
	.found_sep2:
		dec dx
		jmp .find_fileloop
	.found_file_name:
		mov di, .temp_file_name
		; Fetch until 0 or ','
	.store_loop:
		lodsb
		cmp al, 0
		je .done_fetch
		cmp al, ','
		je .done_fetch
		stosb
		jmp .store_loop
	.done_fetch:
		mov al, 0
		stosb
		popa
		mov si, .temp_file_name
		ret

.remove_cur:
	pusha
	; remove cursor at last position
	mov dl, 26
	mov dh, byte [.current_y]
	call os_move_cursor
	mov ah, 0x0E
	mov al, ' '
	int 0x10
	popa
	ret

	[SECTION .data]
	.current_page: dw 1
	.no_of_pages: dw 1
	.no_of_entries: dw 0
	.temp_file_name: times 14 db 0
	.current_y: db 8
	.first_page_string: db 'Page ', 0
	.second_page_string: db ' of ', 0
	.list_help_1: db 'ENTER - Select File', 0
	.list_help_2: db 'W/S - Up/down', 0
	.list_help_3: db 'A/D - Next/Previous', 0
	[SECTION .text]