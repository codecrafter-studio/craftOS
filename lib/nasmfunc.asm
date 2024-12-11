[global gdt_flush]

gdt_flush:
    mov eax, [esp + 4] ; 根据C编译器约定，C语言传入的第一个参数位于内存esp + 4处，第二个位于esp + 8处，以此类推，第n个位于esp + n * 4处
    lgdt [eax] ; 加载gdt并重新设置
; 接下来重新设置各段
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax ; 所有数据段均使用2号数据段
    jmp 0x08:.flush ; 利用farjmp重置代码段为1号代码段并刷新流水线
.flush:
    ret ; 完成

[global idt_flush]
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

[global load_eflags]

load_eflags:
    pushfd ; eflags寄存器只能用pushfd/popfd操作，将eflags入栈/将栈中内容弹入eflags
    pop eax ; eax = eflags;
    ret ; return eax;

[global store_eflags]

store_eflags:
    mov eax, [esp + 4] ; 获取参数
    push eax
    popfd ; eflags = eax;
    ret

[global load_cr0]

load_cr0:
    mov eax, cr0 ; cr0只能和eax之间mov
    ret ; return cr0;

[global store_cr0]

store_cr0:
    mov eax, [esp + 4] ; 获取参数
    mov cr0, eax ; 赋值cr0
    ret

[global load_tr]
load_tr:
    ltr [esp + 4]
    ret

[global farjmp]
farjmp:
    jmp far [esp + 4]
    ret

[global start_app]
start_app: ; void start_app(int new_eip, int new_cs, int new_esp, int new_ss, int *esp0)
    pushad
    mov eax, [esp + 36] ; new_eip
    mov ecx, [esp + 40] ; new_cs
    mov edx, [esp + 44] ; new_esp
    mov ebx, [esp + 48] ; new_ss
    mov ebp, [esp + 52] ; esp0
    mov [ebp], esp ; *esp0 = esp
    mov [ebp + 4], ss ; *ss0 = ss
; 用新的ss重设各段，实际上并不太合理而应使用ds
    mov es, bx
    mov ds, bx
    mov fs, bx
    mov gs, bx
; 选择子或上3表示要进入r3的段
    or ecx, 3 ; new_cs.RPL=3
    or ebx, 3 ; new_ss.RPL=3
    push ebx ; new_ss
    push edx ; new_esp
    push ecx ; new_cs
    push eax ; new_eip
    retf ; 剩下的弹出的活交给 CPU 来完成