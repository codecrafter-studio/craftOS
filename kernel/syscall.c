#include "common.h"
#include "syscall.h"
#include "mtask.h"

void syscall_manager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
    int ds_base = task_now()->ds_base;
    typedef int (*syscall_t)(int, int, int, int, int);
    //(&eax + 1)[7] = ((syscall_t) syscall_table[eax])(ebx, ecx, edx, edi, esi);
    syscall_t syscall_fn = (syscall_t) syscall_table[eax];
    int ret = syscall_fn(ebx, ecx + ds_base, edx, edi, esi);
    int *save_reg = &eax + 1;
    save_reg[7] = ret;
}

int sys_getpid()
{
    return task_pid(task_now());
}
