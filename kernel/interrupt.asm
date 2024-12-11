section .data
global intr_table
intr_table:
%macro ISR_ERRCODE 1
section .text
isr%1:
    push %1 ; 使处理程序知道异常号码
    jmp isr_common_stub ; 通用部分
section .data
dd isr%1
%endmacro

%macro ISR_NOERRCODE 1
section .text
isr%1:
    push byte 0 ; 异常错误码是四个字节，这里只push一个字节原因未知
    push %1 ; 使处理程序知道异常号码
    jmp isr_common_stub ; 通用部分
section .data
dd isr%1
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

section .data
%macro IRQ 1
section .text
irq%1:
    cli
    push byte 0
    push %1
    jmp irq_common_stub
section .data
dd irq%1
%endmacro

IRQ 32
IRQ 33
IRQ 34
IRQ 35
IRQ 36
IRQ 37
IRQ 38
IRQ 39
IRQ 40
IRQ 41
IRQ 42
IRQ 43
IRQ 44
IRQ 45
IRQ 46
IRQ 47

section .text
[extern irq_handler]
; 通用中断处理程序
irq_common_stub:
    pusha ; 存储所有寄存器

    mov ax, ds
    push eax ; 存储ds

    mov ax, 0x10 ; 将内核数据段赋值给各段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler ; 调用C语言处理函数

    pop eax ; 恢复各段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa ; 弹出所有寄存器

    add esp, 8 ; 弹出错误码和中断ID
    iret ; 从中断返回

section .text

[extern isr_handler] ; 将会在isr.c中被定义

; 通用中断处理程序
isr_common_stub:
    pusha ; 存储所有寄存器

    mov ax, ds
    push eax ; 存储ds

    mov ax, 0x10 ; 将内核数据段赋值给各段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler ; 调用C语言处理函数

    pop eax ; 恢复各段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa ; 弹出所有寄存器

    add esp, 8 ; 弹出错误码和中断ID
    iret ; 从中断返回

[extern syscall_manager]
[global syscall_handler]
syscall_handler:
    sti
    push ds
    push es
    pushad
    pushad

    mov ax, 0x10 ; 新增
    mov ds, ax   ; 新增
    mov es, ax   ; 新增

    call syscall_manager

    add esp, 32
    popad
    pop es
    pop ds
    iretd