#include "mtask.h"
#include "file.h"
#include "memory.h"
#include "mtask.h"
#include "elf.h"

void ldt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint16_t ar)
{
    task_t *task = task_now();
    if (limit > 0xfffff) { // 段上限超过1MB
        ar |= 0x8000; // ar的第15位（将被当作limit_high中的G位）设为1
        limit /= 0x1000; // 段上限缩小为原来的1/4096，G位表示段上限为实际的4KB
    }
    // base部分没有其他的奇怪东西混杂，很好说
    task->ldt[num].base_low = base & 0xFFFF; // 低16位
    task->ldt[num].base_mid = (base >> 16) & 0xFF; // 中间8位
    task->ldt[num].base_high = (base >> 24) & 0xFF; // 高8位
    // limit部分混了一坨ar进来，略微复杂
    task->ldt[num].limit_low = limit & 0xFFFF; // 低16位
    task->ldt[num].limit_high = ((limit >> 16) & 0x0F) | ((ar >> 8) & 0xF0); // 现在的limit最多为0xfffff，所以最高位只剩4位作为低4位，高4位自然被ar的高12位挤占

    task->ldt[num].access_right = ar & 0xFF; // ar部分只能存低4位了
}

static void expand_user_segment(int increment)
{
    task_t *task = task_now();
    if (!task->is_user) return; // 内核都打满4GB了还需要扩容？
    gdt_entry_t *segment = &task->ldt[1];
    // 接下来把base和limit的石块拼出来
    uint32_t base = segment->base_low | (segment->base_mid << 16) | (segment->base_high << 24); // 其实可以不用拼直接用ds_base 但还是拼一下吧当练习
    uint32_t size = segment->limit_low | ((segment->limit_high & 0x0F) << 16);
    if (segment->limit_high & 0x80) size *= 0x1000;
    size++;
    // 分配新的内存
    void *new_base = (void *) kmalloc(size + increment + 5);
    if (increment > 0) return; // expand是扩容你缩水是几个意思
    memcpy(new_base, (void *) base, size); // 原来的内容全复制进去
    // 用户进程的base必然由malloc分配，故用free释放之
    kfree((void *) base);
    // 那么接下来就是把new_base设置成新的段了
    ldt_set_gate(1, (int) new_base, size + increment - 1, 0x4092 | 0x60); // 反正只有数据段允许扩容我也就设置成数据段算了
    task->ds_base = (int) new_base; // 既然ds_base变了task里的应该同步更新
}

void *sys_sbrk(int incr)
{
    task_t *task = task_now();
    if (task->is_user) { // 是应用程序
        if (task->brk_start + incr > task->brk_end) { // 如果超出已有缓冲区
            expand_user_segment(incr + 32 * 1024); // 再多扩展32KB
            task->brk_end += incr + 32 * 1024; // 由于扩展了32KB，同步将brk_end移到现在的数据段结尾
        }
        void *ret = task->brk_start; // 旧的program break
        task->brk_start += incr; // 直接添加就完事了
        return ret; // 返回之
    }
    return NULL; // 非用户不允许使用sbrk
}

void app_entry(const char *app_name, const char *cmdline, const char *work_dir)
{
    int fd = sys_open((char *) app_name, O_RDONLY);
    int size = sys_lseek(fd, -1, SEEK_END) + 1;
    sys_lseek(fd, 0, SEEK_SET);
    char *buf = (char *) kmalloc(size + 5);
    sys_read(fd, buf, size);
    int first, last;
    char *code;
    int entry = load_elf((Elf32_Ehdr *) buf, &code, &first, &last);
    if (entry == -1) task_exit(-1);
    char *ds = (char *) kmalloc(last - first + 4 * 1024 * 1024 + 1 * 1024 * 1024 - 5);
    memcpy(ds, code, last - first);
    task_now()->is_user = true;
    // 这一块就是给用户用的
    task_now()->brk_start = (void *) last - first + 4 * 1024 * 1024;
    task_now()->brk_end = (void *) last - first + 5 * 1024 * 1024 - 1;
    // 接下来把cmdline传给app，解析工作由CRT完成
    // 这样我就不用管怎么把一个char **放到栈里了（（（（
    int new_esp = last - first + 4 * 1024 * 1024 - 4;
    int prev_brk = sys_sbrk(strlen(cmdline) + 5); // 分配cmdline这么长的内存，反正也输入不了1MB长的命令
    strcpy((char *) (ds + prev_brk), cmdline); // sys_sbrk使用相对地址，要转换成以ds为基址的绝对地址需要加上ds
    *((int *) (ds + new_esp)) = (int) prev_brk; // 把prev_brk的地址写进栈里，这个位置可以被_start访问
    new_esp -= 4; // esp后移一个dword
    task_now()->ds_base = (int) ds; // 设置ds基址
    ldt_set_gate(0, (int) code, last - first - 1, 0x409a | 0x60);
    ldt_set_gate(1, (int) ds, last - first + 4 * 1024 * 1024 + 1 * 1024 * 1024 - 1, 0x4092 | 0x60);
    start_app(entry, 0 * 8 + 4, new_esp, 1 * 8 + 4, &(task_now()->tss.esp0));
    while (1);
}

int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir)
{
    int fd = sys_open((char *) app_name, O_RDONLY);
    if (fd == -1) return -1;
    sys_close(fd);
    task_t *new_task = create_kernel_task(app_entry);
    new_task->tss.esp -= 12;
    *((int *) (new_task->tss.esp + 4)) = (int) app_name;
    *((int *) (new_task->tss.esp + 8)) = (int) cmdline;
    *((int *) (new_task->tss.esp + 12)) = (int) work_dir;
    task_run(new_task);
    return task_pid(new_task);
}