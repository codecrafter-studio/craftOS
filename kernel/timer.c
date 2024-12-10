#include "timer.h"
#include "isr.h"
#include "mtask.h"

static void timer_callback(registers_t *regs)
{
    task_switch(); // 每出现一次时钟中断，切换一次任务
}

void init_timer(uint32_t freq)
{
    register_interrupt_handler(IRQ0, &timer_callback); // 将时钟中断处理程序注册给IRQ框架

    uint32_t divisor = 1193180 / freq;

    outb(0x43, 0x36); // 指令位，写入频率

    uint8_t l = (uint8_t) (divisor & 0xFF); // 低8位
    uint8_t h = (uint8_t) ((divisor >> 8) & 0xFF); // 高8位

    outb(0x40, l);
    outb(0x40, h); // 分两次发出
}