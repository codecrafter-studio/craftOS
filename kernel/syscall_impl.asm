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

[global read]
read:
    push ebx
    mov eax, 2
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret

[global open]
open:
    push ebx
    mov eax, 3
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    int 80h
    pop ebx
    ret

[global close]
close:
    push ebx
    mov eax, 4
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret

[global lseek]
lseek:
    push ebx
    mov eax, 5
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret

[global unlink]
unlink:
    push ebx
    mov eax, 6
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret

[global create_process]
create_process:
    push ebx
    mov eax, 7
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret

[global waitpid]
waitpid:
    push ebx
    mov eax, 8
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret

[global exit]
exit:
    push ebx
    mov eax, 9
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret

[global sbrk]
sbrk:
    push ebx
    mov eax, 10
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret