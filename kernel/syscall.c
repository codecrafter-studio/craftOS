#include "common.h"
#include "syscall.h"
#include "fifo.h" // 加在开头

extern fifo_t decoded_key; // 加在开头

void syscall_manager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) // 这里的参数顺序是pushad的倒序，不可更改
{
    typedef int (*syscall_t)(int, int, int, int, int); // 这里面只有五个寄存器勉强可以算正常用，所以只有五个参数
    //(&eax + 1)[7] = ((syscall_t) syscall_table[eax])(ebx, ecx, edx, edi, esi); // 把下面的代码压缩成上面一行是这样的
    syscall_t syscall_fn = (syscall_t) syscall_table[eax]; // 从syscall_table中拿到第 eax 个函数
    int ret = syscall_fn(ebx, ecx, edx, edi, esi); // 调用并获取返回值
    // 感谢编译器，即使给多了参数，被调用的函数也会把它们忽略掉
    int *save_reg = &eax + 1; // 进入用于返回值的pushad
    save_reg[7] = ret; // 第7个寄存器为eax，函数返回时默认将eax作为返回值
}

int sys_getpid()
{
    return task_pid(task_now());
}

int sys_write(int fd, const void *msg, int len)
{
    if (fd == 1) {
        char *s = (char *) msg;
        for (int i = 0; i < len; i++) monitor_put(s[i]);
        return 0;
    }
    return -1;
}

int sys_read(int fd, void *buf, int count)
{
    int ret = -1;
    if (fd == 0) { // 如果是标准输入
        char *buffer = (char *) buf; // 先转成char *
        uint32_t bytes_read = 0; // 读了多少个
        while (bytes_read < count) { // 没达到count个
            while (fifo_status(&decoded_key) == 0); // 只要没有新的键我就不读进来
            *buffer = fifo_get(&decoded_key); // 获取新的键
            bytes_read++;
            buffer++; // buffer指向下一个
        }
        ret = (bytes_read == 0 ? -1 : (int) bytes_read); // 如果啥也没读着就-1，否则就正常返回就行了
        return ret;
    }
    return -1; // 还没做
}