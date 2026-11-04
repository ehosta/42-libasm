global	ft_read
extern	__errno_location

section	.text
ft_read:
	xor	rax, rax
	syscall
	cmp	rax, 0
	jl	.error
	ret

.error:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 8

	neg	rax
	push	rax
	call	__errno_location wrt ..plt
	pop	rdx
	mov	[rax], edx
	mov	rax, -1

	mov	rsp, rbp
	pop	rbp

	ret
