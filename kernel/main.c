#include "monitor.h"
#include "gdtidt.h"
#include "isr.h"
#include "timer.h"
#include "memory.h"
#include "mtask.h"

task_t *create_kernel_task(void *entry)
{
    task_t *new_task;
    new_task = task_alloc();
    new_task->tss.esp = (uint32_t) kmalloc(64 * 1024) + 64 * 1024 - 4;
    new_task->tss.eip = (int) entry;
    new_task->tss.es = new_task->tss.ss = new_task->tss.ds = new_task->tss.fs = new_task->tss.gs = 2 * 8;
    new_task->tss.cs = 1 * 8;
    return new_task;
}

void task_b_main()
{
    monitor_write("Waiting for task_a to dead...\n");
    int retval = task_wait(0); // kernel_main
    monitor_write("R.I.P. task_a, retval: ");
    monitor_write_hex(retval);
    monitor_write("\nWaiting for 10 seconds (roughly)...\n");
    for (int i = 0; i < 10000000; i++) for (int j = 0; j < 20; j++);
    task_exit(114514);
}

void task_c_main()
{
    monitor_write("Waiting for task_b to dead...\n");
    int retval = task_wait(1); // task_b
    monitor_write("R.I.P. task_b, retval: ");
    monitor_write_dec(retval);
    monitor_write("\nThey're all dead, I must live!!!");
    while (1);
}

void kernel_main() // kernel.asm会跳转到这里
{
    monitor_clear();
    init_gdtidt();
    init_memory();
    init_timer(100);
    init_keyboard();
    asm("sti");

    task_t *task_a = task_init(); // task_a: pid 0
    task_t *task_b = create_kernel_task(task_b_main); // task_b: pid 1
    task_t *task_c = create_kernel_task(task_c_main); // task_c: pid 2
    task_run(task_b);
    task_run(task_c);

    monitor_write("Waiting for 10 seconds (roughly)...\n");
    for (int i = 0; i < 10000000; i++) for (int j = 0; j < 20; j++);

    // 悬停
    task_exit(0xDEADBEEF); // 再见……
}