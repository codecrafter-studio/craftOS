#include "monitor.h"
#include "isr.h"

void isr_handler(registers_t regs)
{
    asm("cli");
    monitor_write("received interrupt: ");
    monitor_write_dec(regs.int_no);
    monitor_put('\n');
    while (1);
}