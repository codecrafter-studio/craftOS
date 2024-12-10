#ifndef _MTASK_H_
#define _MTASK_H_

#include "common.h"

typedef struct TSS32 {
    uint32_t backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldtr, iomap;
} tss32_t;

typedef struct exit_retval {
    int pid, val;
} exit_retval_t;

typedef struct TASK {
    uint32_t sel;
    int32_t flags;
    exit_retval_t my_retval;
    tss32_t tss;
} task_t;

#define MAX_TASKS 1000
#define TASK_GDT0 3

typedef struct TASKCTL {
    int running, now;
    task_t *tasks[MAX_TASKS];
    task_t tasks0[MAX_TASKS];
} taskctl_t;

task_t *task_init();
task_t *task_alloc();
void task_run(task_t *task);
void task_switch();
task_t *task_now();
int task_pid(task_t *task);
void task_exit(int value);
int task_wait(int pid);

#endif