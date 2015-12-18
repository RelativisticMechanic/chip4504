[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_floppy_reset:
	pusha
	xor ax, ax
	mov dl, byte [disk_id]
	stc
	int 13h
	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; AX : Linear Block Address
os_lba_to_chs:
	pusha
	xor dx, dx
	div word [disk_sec_per_track]
	inc dl
	mov byte [absolute_sector], dl
	xor dx, dx
	div word [disk_sides]
	mov byte [abosolute_head], dl
	mov byte [absolute_track], al
	popa
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Cluster -> LBA
os_cluster_to_lba:
	sub ax, 0x0002
	xor cx, cx
	; 1 sector per cluster
	mov cl, byte [sectors_per_cluster]
	mul cx
	add ax, word [data_sector]
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; AX : Linear Block Address of Starting sector
; ES:BX : Buffer to read
; CX : No of sectors
os_read_sectors:
	mov word [.start_sector], ax
	.start:
		mov di, 0x0005
		mov word [.sector], ax
	.read_loop:
		push ax
		push bx
		push cx
		call os_lba_to_chs
		mov ah, 0x02
		mov al, 0x01
		mov ch, byte [absolute_track]
		mov cl, byte [absolute_sector]
		mov dh, byte [abosolute_head]
		mov dl, byte [disk_id]
		int 0x13
		mov byte [.err_code], ah
		jnc .sector_read
		xor ax, ax
		int 0x13
		dec di
		pop cx
		pop bx
		pop ax
		jnz .read_loop
		mov si, disk_read_err
		call os_print_string
		xor ax, ax
		mov al, byte [.err_code]
		call os_int_to_string
		mov si, ax
		call os_print_string
		mov ah, 0x0E
		mov al, '|'
		int 0x10
		mov ax, word [.sector]
		call os_int_to_string
		mov si, ax
		call os_print_string
		mov ah, 0x0E
		mov al, '|'
		int 0x10
		mov ax, word [.start_sector]
		call os_int_to_string
		mov si, ax
		call os_print_string
		call os_halt
	.sector_read:
		pop cx
		pop bx
		pop ax
		add bx, BYTES_PER_SECTOR
		inc ax
		loop .start
		ret
		
		[SECTION .data]
		.err_code: db 0
		.sector: dw 0
		.start_sector: dw 0
		[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Get list of files on disk
; AX : Pointer to store files
; Special thanks for MikeOS for helping with this.
os_list_files:
	pusha
	
	mov word [.filelist_ptr], ax
	xor eax, eax
	call os_floppy_reset
	
	mov ax, 19
	mov cx, 14
	mov bx, disk_buffer
	call os_read_sectors
	
	; Alright we've read the sectors, now we need to copy file names to our destination
	mov si, disk_buffer
	mov di, word [.filelist_ptr]
	.entre_begin:
		; Windows marker?
		mov al, byte [si + 11] 
		cmp al, 0x0F
		je .skip_entre
		; Directory?
		test al, 0x18
		jnz .skip_entre
		
		mov al, [si]
		; 0xE5 = Deleted
		cmp al, 0xE5
		je .skip_entre
		
		cmp al, 0
		je .done
		
		mov cx, 1
		mov dx, si
	.test_entre:
		inc si
		mov al, [si]
		; ASCII range ' ' to '~' i.e. printable characters
		cmp al, ' '
		jl .next_entre
		cmp al, '~'
		ja .next_entre
		
		inc cx
		cmp cx, 11
		je .file_name_complete
		
		jmp .test_entre
	.file_name_complete:
		mov si, dx
		mov cx, 0
	.entre_name_loop:
		lodsb
		; Ignore spaces in filename (used for padding)
		cmp al, ' '
		je .ignore_space
		stosb
		inc cx
		cmp cx, 8
		je .add_dot
		cmp cx, 11
		je .entre_done
		jmp .entre_name_loop
	.ignore_space:
		inc cx
		cmp cx, 8
		je .add_dot
		jmp .entre_name_loop
	.add_dot:
		mov byte [di], '.'
		inc di
		jmp .entre_name_loop
	.entre_done:
		mov byte [di], ','
		inc di
	.next_entre:
		mov si, dx
	.skip_entre:
		add si, 32
		jmp .entre_begin
	.done:
		dec di
		mov byte [di], 0
		popa
		ret
		
	[SECTION .data]
	.filelist_ptr db 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; os_find_file
; SI - File Name
; Return AX: Cluster, BX : File Size, 0 if not found. Carry set if not found. Cleared if found.
os_find_file:
	pusha
	
	mov ax, si
	call int_filename_convert
	jc .file_convert_err
	mov word [.filename_ptr], ax
	xor eax, eax
	call os_floppy_reset
	
	mov ax, 19
	mov cx, 14
	mov bx, disk_buffer
	call os_read_sectors
	
	mov si, word [.filename_ptr]
	mov di, disk_buffer

	.loop_begin:
		mov al, byte [di]
		cmp al, 0
		je .exit_err
		cmp al, 0xE5
		je .skip
		mov al, byte [di + 11]
		cmp al, 0x0F
		je .skip
		test al, 0x18
		jnz .skip
		
		push di
		push si
		mov cx, 11
		rep cmpsb
		je .file_found
		pop si
		pop di

	.skip:
		add di, 32
		jmp .loop_begin
	.file_found:
		pop si
		pop di
		mov ax, [di + 0x001A]
		mov bx, [di + 0x001C]
		mov word [.cluster], ax
		mov word [.size], bx
		popa
		clc
		mov ax, word [.cluster]
		mov bx, word [.size]
		ret
	.file_convert_err:
		mov si, file_convert_err_msg
		call os_print_string
	.exit_err:
		popa
		xor ax, ax
		xor bx, bx
		stc
		ret
	[SECTION .data]
	.filename_ptr dw 0
	.cluster dw 0
	.size: dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; os_load_file
; AX:BX - Location to load file
; SI - File Name
os_load_file:
	pusha
	push es
	push ax
	push bx
	call os_find_file
	cmp ax, 0
	je .not_found
	.load_FAT:
		; Store cluster.
		mov word [.cluster], ax
		; CX = size of FAT
		xor ax, ax
		mov cx, word [sectors_per_FAT] 
		; NOTE: We only need to load one FAT. The other is just a copy.
		mov ax, word [reserved_sectors]
		mov bx, disk_buffer
		call os_read_sectors
		pop bx
		pop ax
		push bx
		mov es, ax
	.load_image:
		mov ax, word [.cluster]
		call os_cluster_to_lba
		pop bx
		xor cx, cx
		mov cl, byte [sectors_per_cluster]
		call os_read_sectors
		push bx
		mov ax, word [.cluster]
		mov cx, ax
		mov dx, ax
		shr dx, 0x0001
		add cx, dx
		mov bx, disk_buffer
		add bx, cx
		mov dx, word [bx]
		test ax, 0x0001
		jnz .odd_cluster
	.even_cluster:
		and dx, 0000111111111111b
		jmp .done_cluster
	.odd_cluster:
		shr dx, 0x0004
	.done_cluster:
		mov word [.cluster], dx
		cmp dx, 0x0FF0
		jb .load_image
	.return:
		pop bx
		pop es
		popa
		clc
		ret
	.not_found:
		pop bx
		pop ax
		pop es
		popa
		stc 
		ret
	[SECTION .data]
	.cluster: dw 0
	[SECTION .text]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; From MikeOS
; AX: File name to convert
; OUT: AX, converted file name
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int_filename_convert:
	pusha

	mov si, ax
	call os_string_length
	cmp ax, 14
	jg .failure	

	cmp ax, 0
	je .failure

	mov dx, ax
	mov di, .dest_string
	mov cx, 0
.copy_loop:
	lodsb
	cmp al, '.'
	je .extension_found
	stosb
	inc cx
	cmp cx, dx
	jg .failure
	jmp .copy_loop

.extension_found:
	cmp cx, 0
	je .failure

	cmp cx, 8
	je .do_extension

.add_spaces:
	mov byte [di], ' '
	inc di
	inc cx
	cmp cx, 8
	jl .add_spaces
.do_extension:
	lodsb
	cmp al, 0
	je .failure
	stosb
	lodsb
	cmp al, 0
	je .failure
	stosb
	lodsb
	cmp al, 0
	je .failure
	stosb

	mov byte [di], 0

	popa
	mov ax, .dest_string
	clc
	ret
.failure:
	popa
	stc	
	ret
	
	[SECTION .data]
	.dest_string	times 13 db 0
	[SECTION .text]
	
[SECTION .data]
disk_id: db 0
disk_sides: dw 2
disk_sec_per_track: dw 18
abosolute_head: db 0x0
absolute_track: db 0x0
absolute_sector: db 0x0
number_of_FATs: db 2
sectors_per_FAT: dw 9
reserved_sectors: dw 1
sectors_per_cluster: db 1
no_of_root_entries: dw 224
bytes_per_sector: dw 512
data_sector: db 19
; I dunno mate
shitty_buffer: times 512 db 0
disk_buffer: times 8192 db 0