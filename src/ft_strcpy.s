section .text
	global ft_strcpy

ft_strcpy:
	mov	rax, rdi

.loop:
	mov	dl, [rsi]
	mov	[rdi], dl
	test	dl, dl
	jz	.done
	inc	rsi
	inc	rdi
	jmp	.loop

.done:
	ret