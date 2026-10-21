global	ft_strcpy

section	.text
ft_strcpy:
	xor	rax, rax
.loop:
	cmp	byte [rdi + rax], 0
	je	.done
	inc	rax
	jmp	.loop
.done:
	ret
