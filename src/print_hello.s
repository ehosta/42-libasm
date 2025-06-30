global print_hello

section .data
hello:		db 'Hello world!', 10
helloLen:	equ $ - hello

section .text
print_hello:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel hello]
	mov rdx, helloLen
	syscall
	ret

