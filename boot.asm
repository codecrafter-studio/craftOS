    org 07c00h ; 告诉编译器程序将装载至0x7c00处

    mov ax, cs
    mov ds, ax
    mov es, ax ; 将ds es设置为cs的值（因为此时字符串存在代码段内）
    call DispStr ; 显示字符函数
    jmp $ ; 死循环

DispStr:
    mov ax, BootMessage
    mov bp, ax ; es前面设置过了，所以此处的bp就是串地址
    mov cx, 16 ; 字符串长度
    mov ax, 01301h ; 显示模式
    mov bx, 000ch ; 显示属性
    mov dl, 0 ; 显示坐标（这里只设置列因为行固定是0）
    int 10h ; 显示
    ret

BootMessage: db "Hello, OS world!"
times 510 - ($ - $$) db 0
db 0x55, 0xaa ; 确保最后两个字节是0x55AA