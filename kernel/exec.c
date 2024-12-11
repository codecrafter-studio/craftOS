#include "mtask.h"
#include "file.h"
#include "memory.h"
#include "mtask.h"

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

void app_entry(const char *app_name, const char *cmdline, const char *work_dir)
{
    int fd = sys_open((char *) app_name, O_RDONLY);
    int size = sys_lseek(fd, -1, SEEK_END) + 1;
    sys_lseek(fd, 0, SEEK_SET);
    char *buf = (char *) kmalloc(size + 5);
    sys_read(fd, buf, size);
    task_now()->ds_base = (int) buf; // 这里是新增的
    ldt_set_gate(0, (int) buf, size - 1, 0x409a | 0x60);
    ldt_set_gate(1, (int) buf, size - 1, 0x4092 | 0x60);
    start_app(0, 0 * 8 + 4, 0, 1 * 8 + 4, &(task_now()->tss.esp0));
    while (1);
}

int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir)
{
    task_t *new_task = create_kernel_task(app_entry);
    new_task->tss.esp -= 12;
    *((int *) (new_task->tss.esp + 4)) = (int) app_name;
    *((int *) (new_task->tss.esp + 8)) = (int) cmdline;
    *((int *) (new_task->tss.esp + 12)) = (int) work_dir;
    task_run(new_task);
    return task_pid(new_task);
}