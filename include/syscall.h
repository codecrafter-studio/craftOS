#ifndef _SYSCALL_H_
#define _SYSCALL_H_

typedef void *syscall_func_t;

int sys_getpid();
int sys_write(int, const void *, int);
int sys_read(int, void *, int);
int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir);

syscall_func_t syscall_table[] = {
    sys_getpid, sys_write, sys_read, sys_create_process, // 这里新增了一个函数
};

#endif