#include "common.h"
#include "memory.h"

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

extern uint32_t load_eflags();
extern uint32_t load_cr0();
extern void store_eflags(uint32_t);
extern void store_cr0(uint32_t);

static uint32_t memtest_sub(uint32_t start, uint32_t end)
{
    uint32_t i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    for (i = start; i <= end; i += 0x1000) {
        p = (uint32_t *) (i + 0xffc); // 每4KB检查最后4个字节
        old = *p; // 记住修改前的值
        *p = pat0; // 试写
        *p ^= 0xffffffff; // 翻转
        if (*p != pat1) { // ~pat0 = pat1，翻转之后如果不是pat1则写入失败
            *p = old; // 写回去
            break;
        }
        *p ^= 0xffffffff; // 再翻转
        if (*p != pat0) { // 两次翻转应该转回去，如果不是pat0则写入失败
            *p = old; // 写回去
            break;
        }
        *p = old; // 试写完毕，此4KB可用，恢复为修改前的值
    }
    return i; // 返回内存容量
}

static uint32_t memtest(uint32_t start, uint32_t end)
{
    char flg486 = 0;
    uint32_t eflags, cr0, i;

    eflags = load_eflags();
    eflags |= EFLAGS_AC_BIT; // AC-bit = 1
    store_eflags(eflags);
    eflags = load_eflags();
    if ((eflags & EFLAGS_AC_BIT) != 0) flg486 = 1;
    // 486的CPU会把AC位当回事，但386的则会把AC位始终置0
    // 这样就可以判断CPU是否在486以上
    // 恢复回去
    eflags &= ~EFLAGS_AC_BIT; // AC-bit = 0
    store_eflags(eflags);

    if (flg486) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; // 禁用缓存
        store_cr0(cr0);
    }

    i = memtest_sub(start, end); // 真正的内存探测函数

    if (flg486) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // 允许缓存
        store_cr0(cr0);
    }

    return i;
}

static void memman_init(memman_t *man)
{
    man->frees = 0;
}

static uint32_t memman_total(memman_t *man)
{
    uint32_t i, t = 0;
    for (i = 0; i < man->frees; i++) t += man->free[i].size; // 剩余内存总和
    return t;
}

static uint32_t memman_alloc(memman_t *man, uint32_t size)
{
    uint32_t i, a;
    for (i = 0; man->frees; i++) {
        if (man->free[i].size >= size) { // 找到了足够的内存
            a = man->free[i].addr;
            man->free[i].addr += size; // addr后移，因为原来的addr被使用了
            man->free[i].size -= size; // size也要减掉
            if (man->free[i].size == 0) { // 这一条size被分配完了
                man->frees--; // 减一条frees
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1]; // 各free前移
                }
            }
            return a; // 返回
        }
    }
    return 0; // 无可用空间
}

static int memman_free(memman_t *man, uint32_t addr, uint32_t size)
{
    int i, j;
    for (i = 0; i < man->frees; i++) {
        // 各free按addr升序排列
        if (man->free[i].addr > addr) break; // 找到位置了！
        // 现在的这个位置是第一个在addr之后的位置，有man->free[i - 1].addr < addr < man->free[i].addr
    }
    if (i > 0) {
        if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
            // 可以和前面的可用部分合并
            man->free[i - 1].size += size; // 并入
            if (i < man->frees) {
                if (addr + size == man->free[i].addr) {
                    // 可以与后面的可用部分合并
                    man->free[i - 1].size += man->free[i].size;
                    // man->free[i]删除不用
                    man->frees--; // frees减1
                    for (; i < man->frees; i++) {
                        man->free[i] = man->free[i + 1]; // 前移
                    }
                }
            }
            return 0; // free完毕
        }
    }
    // 不能与前面的合并
    if (i < man->frees) {
        if (addr + size == man->free[i].addr) {
            // 可以与后面的可用部分合并
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0; // 成功合并
        }
    }
    // 两边都合并不了
    if (man->frees < MEMMAN_FREES) {
        // free[i]之后的后移，腾出空间
        for (j = man->frees; j > i; j--) man->free[j] = man->free[j - 1];
        man->frees++;
        man->free[i].addr = addr;
        man->free[i].size = size; // 更新当前地址和大小
        return 0; // 成功合并
    }
    // 无free可用且无法合并
    return -1; // 失败
}

void *kmalloc(uint32_t size)
{
    uint32_t addr;
    memman_t *memman = (memman_t *) MEMMAN_ADDR;
    addr = memman_alloc(memman, size + 16); // 多分配16字节
    memset((void *) addr, 0, size + 16);
    char *p = (char *) addr;
    if (p) {
        *((int *) p) = size;
        p += 16;
    }
    return (void *) p;
}

void kfree(void *p)
{
    char *q = (char *) p;
    int size = 0;
    if (q) {
        q -= 16;
        size = *((int *) q);
    }
    memman_t *memman = (memman_t *) MEMMAN_ADDR;
    memman_free(memman, (uint32_t) q, size + 16);
    p = NULL;
    return;
}

void init_memory()
{
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    memman_t *memman = (memman_t *) MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x400000, memtotal - 0x400000);
}