global	ft_write
extern	__errno_location

section	.text
ft_write:
	xor	rax, rax
	inc	rax
	syscall
	cmp	rax, 0
	jl	.error
	ret

.error:
	neg	rax
	push	rax
	call	__errno_location wrt ..plt
	pop	rdx
	mov	[rax], edx
	mov	rax, -1
	ret
