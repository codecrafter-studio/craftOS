    mov eax, 1
    mov ebx, 1
    mov ecx, string
    mov edx, strlen
    int 80h

    jmp $

string: db "Hello, World!", 0x0A, 0x00
strlen equ $ - string