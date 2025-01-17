#include "monitor.h"
#include "gdtidt.h"
#include "isr.h"
#include "timer.h"
#include "memory.h"
#include "mtask.h"
#include "keyboard.h"
#include "shell.h"
#include "cmos.h"
#include "file.h"
#include "syscall.h"
#include "exec.h"

void kernel_main()
{
    monitor_clear();
    init_gdtidt();
    init_memory();
    init_timer(100);
    init_keyboard();
    asm("sti");
    task_init();

    sys_create_process("shell.cxp", "", "/");

    task_exit(0);
}