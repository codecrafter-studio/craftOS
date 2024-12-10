#include "monitor.h"
#include "gdtidt.h"
#include "memory.h"
#include "timer.h"

void kernel_main() // kernel.asm会跳转到这里
{
    monitor_clear(); // 先清屏
    init_gdtidt();
    init_timer(50);
    init_memory();
    monitor_write("Hello, kernel world!\n");
    // 验证write_hex和write_dec，由于没有printf，这一步十分烦人
    monitor_write_hex(0x114514);
    monitor_write(" = ");
    monitor_write_dec(0x114514);
    monitor_write("\n");
    //asm("sti");

    // 悬停
    while (1);
}