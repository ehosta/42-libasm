global	ft_strcpy

section	.text
ft_strcpy:
	xor	rax, rax
	mov	rbx, rdi
.loop:
	lodsb
	cmp	al, 0
	je	.done
	stosb
	jmp	.loop
.done:
	xchg	rax, rbx
	ret
