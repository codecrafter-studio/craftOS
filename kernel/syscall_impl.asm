[global getpid]
getpid:
    mov eax, 0
    int 80h
    ret

[global write]
write:
    push ebx
    mov eax, 1
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret