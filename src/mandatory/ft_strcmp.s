global	ft_strcmp

section .text
ft_strcmp:
	xor	rdx, rdx

.loop:
	movzx	rcx, byte [rdi + rdx]
	movzx	r8, byte [rsi + rdx]
	cmp	rcx, 0
	je	.done
	cmp	rcx, r8
	jne	.done
	inc	rdx
	jmp	.loop

.done:
	mov	rax, rcx
	sub	rax, r8
	ret
