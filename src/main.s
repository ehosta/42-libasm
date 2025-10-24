global _start

section .data
	test_str db "Hello, World!", 0
	dest_str times 50 db 0
	newline db 10, 0

_start:
	mov	rax, 60
	mov	rdi, 0
	syscall