#include "common.h"
#include "stdarg.h" // 在开头添加，因为用到了va_list以及操纵va_list的这些东西
#include "unistd.h"

static void itoa(uint32_t num, char **buf_ptr_addr, int radix)
{
    uint32_t m = num % radix; // 最低位
    uint32_t i = num / radix; // 最高位
    if (i) itoa(i, buf_ptr_addr, radix); // 先把高位化为字符串
    if (m < 10) { // 处理最低位
        *((*buf_ptr_addr)++) = m + '0'; // 0~9，直接加0
    } else {
        *((*buf_ptr_addr)++) = m - 10 + 'A'; // 10~15，10~15->0~5->A~F
    }
}

int vsprintf(char *buf, const char *fmt, va_list ap)
{
    char *buf_ptr = buf; // 不动原来的buf，原来的buf可能还用得着
    const char *index_ptr = fmt; // 不动原来的fmt，但这个好像真用不着
    char index_char = *index_ptr; // fmt串中的当前字符
    int32_t arg_int; // 可能会出现的int参数
    char *arg_str; // 可能会出现的char *参数
    while (index_char) { // 没到fmt的结尾
        if (index_char != '%') { // 不是%
            *(buf_ptr++) = index_char; // 直接复制到buf
            index_char = *(++index_ptr); // 自动更新到下一个字符
            continue; // 跳过后续对于%的判断
        }
        index_char = *(++index_ptr); // 先把%跳过去
        switch (index_char) { // 对现在的index_char进行判断
            case 's':
                arg_str = va_arg(ap, char*); // 获取char *参数
                strcpy(buf_ptr, arg_str); // 直接strcpy进buf_ptr
                buf_ptr += strlen(arg_str); // buf_ptr直接跳到arg_str结尾，正好在arg_str结尾的\0处
                break;
            case 'c':
                *(buf_ptr++) = va_arg(ap, int); // 把获取到的char参数直接写进buf_ptr
                break;
            case 'x':
                arg_int = va_arg(ap, int); // 获取int参数
                itoa(arg_int, &buf_ptr, 16); // itoa早在设计时就可以修改buf_ptr，这样就直接写到buf_ptr里了，还自动跳到数末尾
                break;
            case 'd':
                arg_int = va_arg(ap, int); // 获取int参数
                if (arg_int < 0) { // 给负数前面加个符号
                    arg_int = -arg_int; // 先转负为正
                    *(buf_ptr++) = '-'; // 然后加负号
                }
                itoa(arg_int, &buf_ptr, 10); // itoa早在设计时就可以修改buf_ptr，这样就直接写到buf_ptr里了，还自动跳到数末尾
                break;
            default:
                break;
        }
        index_char = *(++index_ptr); // 再把%后面的s c x d跳过去
    }
    return strlen(buf); // 返回做完后buf的长度
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vsprintf(buf, fmt, ap);
    va_end(ap);
    return ret;
}

int vprintf(const char *fmt, va_list ap)
{
    char buf[1024] = {0}; // 理论上够了
    int ret = vsprintf(buf, fmt, ap);
    write(1, buf, ret);
    return ret;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

void puts(const char *buf)
{
    write(1, buf, strlen(buf));
    write(1, "\n", 1);
}

int putchar(char ch)
{
    printf("%c", ch);
    return ch;
}

static void readline(char *buf, int cnt) {
    char *pos = buf;
    while (read(0, pos, 1) != -1 && (pos - buf) < cnt) { // 读字符成功且没到cnt个
        switch (*pos) {
            case '\n':
            case '\r': // 回车或换行，结束
                *pos = 0;
                putchar('\n'); // read不自动回显，需要手动补一个\n
                return; // 返回
            case '\b': // 退格
                if (buf[0] != '\b') { // 如果不在第一个
                    --pos; // 指向上一个位置
                    putchar('\b'); // 手动输出一个退格
                }
                break;
            default:
                putchar(*pos); // 都不是，那就直接输出刚输入进来的东西
                pos++; // 指向下一个位置
        }
    }
}

// 转换字符串为整数
int str_to_int(const char *str) {
    int result = 0;
    while (*str) {
        if (*str < '0' || *str > '9') break; // 非数字字符
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

// 手动实现vscanf
int vscanf(const char *fmt, va_list ap) {
    const char *index_ptr = fmt;
    char index_char = *index_ptr;
    char input_buf[1024];
    int input_len = 0;

    // 读取输入
    readline(input_buf, sizeof(input_buf));

    // 遍历格式字符串
    while (index_char) {
        if (index_char != '%') {
            index_char = *(++index_ptr); // 跳过非%字符
            continue;
        }

        index_char = *(++index_ptr); // 跳过%字符

        switch (index_char) {
            case 's': // 读取字符串
                {
                    char *arg_str = va_arg(ap, char*);
                    // 复制字符串
                    strcpy(arg_str, input_buf);
                }
                break;
            case 'c': // 读取字符
                {
                    char *arg_char = va_arg(ap, char*);
                    *arg_char = input_buf[0]; // 假设读取的字符为第一个
                }
                break;
            case 'd': // 读取整数
                {
                    int *arg_int = va_arg(ap, int*);
                    *arg_int = str_to_int(input_buf); // 使用str_to_int转换
                }
                break;
            default:
                break;
        }

        index_char = *(++index_ptr); // 跳过格式符后的字符
    }

    return 1; // 成功读取一个值
}

int scanf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vscanf(fmt, ap);
    va_end(ap);
    return ret;
}