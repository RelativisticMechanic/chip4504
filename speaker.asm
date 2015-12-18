;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Emit a beep of a certain frequency
; AX - Freq
os_speaker_tone:
	pusha

	mov cx, ax

	mov al, 182
	out 43h, al
	mov ax, cx
	out 42h, al
	mov al, ah
	out 42h, al

	in al, 61h
	or al, 03h
	out 61h, al

	popa
	ret
; Turns off PC speaker
os_speaker_off:
	pusha

	in al, 61h
	and al, 0FCh
	out 61h, al

	popa
	ret