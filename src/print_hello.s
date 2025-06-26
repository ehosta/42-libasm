global print_hello

SECTION .data
hello:     db 'Hello world!', 10
helloLen:  equ $ - hello

SECTION .text
print_hello:
    mov rax, 1
    mov rdi, 1
    mov rsi, hello
    mov rdx, helloLen
    syscall
    ret