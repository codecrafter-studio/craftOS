#include "common.h"
#include "gdtidt.h"

extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);
extern void syscall_handler(); // 这里是新增的

gdt_entry_t gdt_entries[4096];
gdt_ptr_t gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint16_t ar)
{
    if (limit > 0xfffff) { // 段上限超过1MB
        ar |= 0x8000; // ar的第15位（将被当作limit_high中的G位）设为1
        limit /= 0x1000; // 段上限缩小为原来的1/4096，G位表示段上限为实际的4KB
    }
    // base部分没有其他的奇怪东西混杂，很好说
    gdt_entries[num].base_low = base & 0xFFFF; // 低16位
    gdt_entries[num].base_mid = (base >> 16) & 0xFF; // 中间8位
    gdt_entries[num].base_high = (base >> 24) & 0xFF; // 高8位
    // limit部分混了一坨ar进来，略微复杂
    gdt_entries[num].limit_low = limit & 0xFFFF; // 低16位
    gdt_entries[num].limit_high = ((limit >> 16) & 0x0F) | ((ar >> 8) & 0xF0); // 现在的limit最多为0xfffff，所以最高位只剩4位作为低4位，高4位自然被ar的高12位挤占

    gdt_entries[num].access_right = ar & 0xFF; // ar部分只能存低4位了
}

static void init_gdt()
{
    gdt_ptr.limit = sizeof(gdt_entry_t) * 4096 - 1; // GDT总共4096个描述符，但我们总共只用到3个
    gdt_ptr.base = (uint32_t) &gdt_entries; // 基地址

    gdt_set_gate(0, 0, 0,          0); // 占位用NULL段
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x409A); // 32位代码段
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x4092); // 32位数据段

    gdt_flush((uint32_t) &gdt_ptr); // 刷新gdt
}

extern void *intr_table[48];

static void idt_set_gate(uint8_t num, uint32_t offset, uint16_t sel, uint8_t flags)
{
    idt_entries[num].offset_low = offset & 0xFFFF;
    idt_entries[num].selector = sel;
    idt_entries[num].dw_count = 0;
    idt_entries[num].access_right = flags;
    idt_entries[num].offset_high = (offset >> 16) & 0xFFFF;
}

static void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    // 初始化PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    for (int i = 0; i < 32 + 16; i++) {
        idt_set_gate(i, (uint32_t) intr_table[i], 0x08, 0x8E);
    }

    idt_set_gate(0x80, (uint32_t) syscall_handler, 0x08, 0x8E | 0x60); // 这里是新增的

    idt_flush((uint32_t) &idt_ptr);
}

void init_gdtidt()
{
    init_gdt();
    init_idt();
}