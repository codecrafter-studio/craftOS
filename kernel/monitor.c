#include "monitor.h"
#include "ansi.h"

static uint16_t cursor_x = 0, cursor_y = 0; // 光标位置
static uint16_t *video_memory = (uint16_t *) 0xB8000; // 一个字符占两个字节（字符本体+字符属性，即颜色等），因此用uint16_t
static uint8_t attributeByte = (0 << 4) | (15 & 0x0F);

static void move_cursor() // 根据当前光标位置（cursor_x，cursor_y）移动光标
{
    uint16_t cursorLocation = cursor_y * 80 + cursor_x; // 当前光标位置
    outb(0x3D4, 14); // 光标高8位
    outb(0x3D5, cursorLocation >> 8); // 写入
    outb(0x3D4, 15); // 光标低8位
    outb(0x3D5, cursorLocation); // 写入，由于value声明的是uint8_t，因此会自动截断
}

int get_cursor_pos() { return (cursor_x + 1) << 8 | (cursor_y + 1); }

void move_cursor_to(int new_x, int new_y)
{
    cursor_x = new_x - 1;
    cursor_y = new_y - 1;
    move_cursor();
}

void set_color(int fore, int back, int fore_brighten)
{
    fore %= 10; back %= 10;
    int ansicode2vgacode[] = {0, 4, 2, 6, 1, 5, 3, 7};
    fore = ansicode2vgacode[fore];
    back = ansicode2vgacode[back];
    fore |= fore_brighten << 3;
    attributeByte = (back << 4) | (fore & 0x0F);
}

int get_color() { return attributeByte; }

void set_char_at(int x, int y, char ch)
{
    x--, y--;
    uint16_t *location = video_memory + (y * 80 + x);
    *location = ch | (attributeByte << 8);
}

// 文本控制台共80列，25行（纵列竖行），因此当y坐标不低于25时就要滚屏了
static void scroll() // 滚屏
{
    // uint8_t attributeByte = (0 << 4) | (15 & 0x0F); // 黑底白字
    uint16_t blank = 0x20 | (attributeByte << 8); // 0x20 -> 空格这个字，attributeByte << 8 -> 属性位

    if (cursor_y >= 25) // 控制台共25行，超过即滚屏
    {
        int i;
        for (i = 0 * 80; i < 24 * 80; i++) video_memory[i] = video_memory[i + 80]; // 前24行用下一行覆盖
        for (i = 24 * 80; i < 25 * 80; i++) video_memory[i] = blank; // 第25行用空格覆盖
        cursor_y = 24; // 光标设置回24行
    }
}

void monitor_put(char c) // 打印字符
{
    uint8_t backColor = 0, foreColor = 15; // 背景：黑，前景：白
    // uint8_t attributeByte = (backColor << 4) | (foreColor & 0x0f); // 黑底白字
    uint16_t attribute = attributeByte << 8; // 高8位为字符属性位
    uint16_t *location; // 写入位置

    // 接下来对字符种类做各种各样的判断
    if (c == 0x08 && cursor_x) // 退格，且光标不在某行开始处
    {
        cursor_x--; // 直接把光标向后移一格
        video_memory[cursor_y * 80 + cursor_x] = 0x20 | (attributeByte << 8); // 空格
    }
    else if (c == 0x09) // 制表符
    {
        cursor_x = (cursor_x + 8) & ~(8 - 1); // 把光标后移至8的倍数为止
        // 这一段代码实际上的意思是：先把cursor_x + 8，然后把这一个数值变为小于它的最大的8的倍数（位运算的魅力，具体的可以在纸上推推）
    }
    else if (c == '\r') // CR
    {
        cursor_x = 0; // 光标回首
    }
    else if (c == '\n') // LF
    {
        cursor_x = 0; // 光标回首
        cursor_y++; // 下一行
    }
    else if (c >= ' ' && c <= '~') // 可打印字符
    {
        location = video_memory + (cursor_y * 80 + cursor_x); // 当前光标处就是写入字符位置
        *location = c | attribute; // 低8位：字符本体，高8位：属性，黑底白字
        cursor_x++; // 光标后移
    }

    if (cursor_x >= 80) // 总共80列，到行尾必须换行
    {
        cursor_x = 0;
        cursor_y++;
    }

    scroll(); // 滚屏，如果需要的话
    move_cursor(); // 移动光标
}

void monitor_write(char *s)
{
    for (; *s; s++) {
        if (*s == 0x1b) {
            // 是esc，则解析ansi转义序列
            int offset = parse_ansi(s);
            // offset不为-1表示有意义的ANSI转义序列
            if (offset != -1) {
                s += offset; // 跳过后续这些字符不二次输出
                continue;
            }
        }
        monitor_put(*s); // 遍历字符串直到结尾，输出每一个字符
    }
}

void monitor_clear()
{
    uint8_t attributeByte = (0 << 4) | (15 & 0x0F); // 黑底白字
    uint16_t blank = 0x20 | (attributeByte << 8); // 0x20 -> 空格这个字，attributeByte << 8 -> 属性位

    for (int i = 0; i < 80 * 25; i++) video_memory[i] = blank; // 全部打印为空格

    cursor_x = 0;
    cursor_y = 0;
    move_cursor(); // 光标置于左上角
}

void monitor_write_dec(uint32_t dec)
{
    int upper = dec / 10, rest = dec % 10;
    if (upper) monitor_write_dec(upper);
    monitor_put(rest + '0');
}

void monitor_write_hex(uint32_t hex)
{
    char buf[20]; // 32位最多0xffffffff，20个都多了
    char *p = buf; // 用于写入的指针
    char ch; // 当前十六进制字符
    int i, flag = 0; // i -> 循环变量，flag -> 前导0是否结束

    *p++ = '0';
    *p++ = 'x'; // 先存一个0x

    if (hex == 0) *p++ = '0'; // 如果是0，直接0x0结束
    else {
        for (i = 28; i >= 0; i -= 4) { // 每次4位，0xF = 0b1111
            ch = (hex >> i) & 0xF; // 0~9, A~F
            // 28的原因是多留一点后路（
            if (flag || ch > 0) { // 跳过前导0
                flag = 1; // 没有前导0就把flag设为1，这样后面再有0也不会忽略
                ch += '0'; // 0~9 => '0'~'9'
                if (ch > '9') {
                    ch += 7; // 'A' - '9' = 7
                }
                *p++ = ch; // 写入
            }
        }
    }
    *p = '\0'; // 结束符

    monitor_write(buf);
}