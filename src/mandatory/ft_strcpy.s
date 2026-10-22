global	ft_strcpy

section	.text
ft_strcpy:
	xor	rax, rax
	mov	rdx, rdi
.loop:
	lodsb
	stosb
	cmp	al, 0
	je	.done
	jmp	.loop
.done:
	xchg	rax, rdx
	ret
