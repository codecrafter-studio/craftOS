#include "monitor.h"
#include "gdtidt.h"

void kernel_main() // kernel.asm会跳转到这里
{
    init_gdtidt();
    monitor_clear(); // 先清屏
    monitor_write("Hello, kernel world!\n");
    // 验证write_hex和write_dec，由于没有printf，这一步十分烦人
    monitor_write_hex(0x114514);
    monitor_write(" = ");
    monitor_write_dec(0x114514);
    monitor_write("\n");
    asm("ud2");
    // 悬停
    while (1);
}