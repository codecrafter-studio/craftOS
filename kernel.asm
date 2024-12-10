[section .text]

global _start

_start: ; 此处假设gs仍指向显存
    mov ah, 0Fh
    mov al, 'K'
    mov [gs:((80 * 1 + 39) * 2)], ax ; 第1行正中央，白色K
    jmp $ ; 死循环