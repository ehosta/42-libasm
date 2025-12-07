section .text
	global ft_strcmp

ft_strcmp:
	xor	rax, rax

.loop:
	mov	dl, [rdi]
	mov	al, [rsi]
	test	dl, al
	jne	.done
	inc	rsi
	inc	rdi
	jmp	.loop

.done:
	
