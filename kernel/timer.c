#include "timer.h"
#include "isr.h"
#include "monitor.h"

uint32_t tick = 0; // 这里做一下记录，不过也没什么用？

static void timer_callback(registers_t *regs)
{
    tick++;
    monitor_write("Tick: ");
    monitor_write_dec(tick);
    monitor_put('\n'); // 测试用，暂时打印一下ticks
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