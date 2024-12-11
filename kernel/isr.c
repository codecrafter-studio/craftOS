#include "monitor.h"
#include "isr.h"
#include "mtask.h"

static isr_t interrupt_handlers[256];

void isr_handler(registers_t regs)
{
    asm("cli");
    monitor_write("received interrupt: ");
    monitor_write_dec(regs.int_no);
    monitor_put('\n');
    task_exit(-1); // 强制退出
}

void irq_handler(registers_t regs)
{
    if (regs.int_no >= 0x28) outb(0xA0, 0x20); // 中断号 >= 40，来自从片，发送EOI给从片
    outb(0x20, 0x20); // 发送EOI给主片

    if (interrupt_handlers[regs.int_no])
    {
        isr_t handler = interrupt_handlers[regs.int_no]; // 有自定义处理程序，调用之
        handler(&regs); // 传入寄存器
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}