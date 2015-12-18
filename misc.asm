;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; AX - Milisecs to pause
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_pause:
	pusha
	cmp ax, 0
	je .time_up

	mov cx, 0
	mov [.counter_var], cx

	mov bx, ax
	mov ax, 0
	mov al, 2
	mul bx
	mov [.orig_req_delay], ax

	mov ah, 0
	int 1Ah

	mov [.prev_tick_count], dx

.checkloop:
	mov ah,0
	int 1Ah

	cmp [.prev_tick_count], dx

	jne .up_date
	jmp .checkloop

.time_up:
	popa
	ret

.up_date:
	mov ax, [.counter_var]
	inc ax
	mov [.counter_var], ax

	cmp ax, [.orig_req_delay]
	jge .time_up

	mov [.prev_tick_count], dx

	jmp .checkloop


	.orig_req_delay		dw	0
	.counter_var		dw	0
	.prev_tick_count	dw	0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
os_int_to_string:
	pusha

	mov cx, 0
	mov bx, 10
	mov di, .t

.push:
	mov dx, 0
	div bx
	inc cx
	push dx
	test ax, ax
	jnz .push
.pop:
	pop dx
	add dl, '0'
	mov [di], dl
	inc di
	dec cx
	jnz .pop

	mov byte [di], 0

	popa
	mov ax, .t
	ret


	.t times 7 db 0