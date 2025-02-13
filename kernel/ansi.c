#include "monitor.h"
// 新增部分从此开始
#include "fifo.h"
#include "stdio.h"

extern uint32_t load_eflags();
extern void store_eflags(uint32_t);

extern fifo_t decoded_key;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
// 新增部分到此结束
static int param_arr[255] = {0};
static int param_idx = 0;
static int save_x = 0, save_y = 0; // 这一行也是新增的

// 解析ansi控制序列，返回值是开始的esc后一共有多少个字符，不为合法ANSI转义序列返回-1
int parse_ansi(const char *ansi)
{
    param_idx = 0;
    memset(param_arr, 0, sizeof(param_arr)); // 清空参数数组并将当前参数放在第一个
    char *esc_start = (char *) ansi;
    ansi++; // 我们假定ansi的第一个字符总是esc
    if (*ansi == 'c') {
        // \033c，完全重置终端
        // 在这里只能是指把显示器恢复到最初的状态，也就是清屏并恢复黑底白字
        set_color(7, 0, true);
        monitor_clear();
        return 1;
    }
    if (*ansi != '[') {
        return -1; // 除了\033c外只支持CSI
    }
    ansi++; // 跳过中括号
    while (*ansi >= '0' && *ansi <= '9') {
        // 如果是数字，那么统一当做参数处理
        param_arr[param_idx] = param_arr[param_idx] * 10 + *ansi - '0'; // 当前参数结尾增加一位
        ansi++;
        if (*ansi == ';') {
            // 是分号表示当前参数结束
            param_idx++;
            ansi++; // 跳过分号继续进行参数处理
        }
    }
    // 参数解析部分结束，最后一个字符是CSI对应命令
    char cmd = *ansi;
    int cursor_pos = get_cursor_pos();
    int cursor_x = cursor_pos >> 8;
    int cursor_y = cursor_pos & 0xff;
    int color = get_color();
    int fore = color & 0x7;
    int fore_brighten = (color & 0xf) >> 3;
    int back = color >> 4;
    switch (cmd) {
        case 'A': {
            // 上移，y减小
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_y = max(cursor_y - arg, 1);
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'B': {
            // 下移，y增大
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_y = min(cursor_y + arg, 25);
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'C': {
            // 左移，x减小
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_x = max(cursor_x - arg, 1);
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'D': {
            // 右移，x增大
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_x = min(cursor_x + arg, 80);
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'E': {
            // 移到本行下面第n行开头
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_x = 1; // 第1列
            cursor_y = min(cursor_y + arg, 25); // 下面第n行
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'F': {
            // 移到本行上面第n行开头
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_x = 1; // 第1列
            cursor_y = max(cursor_y - arg, 1); // 上面第n行
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'G': {
            // 移到第n列
            int arg = param_arr[0] == 0 ? 1 : param_arr[0];
            cursor_x = min(max(arg, 1), 80);
            move_cursor_to(cursor_x, cursor_y);
            break;
        }
        case 'H':
        case 'f': {
            // 移到坐标 (n, m) 处
            int n = param_arr[0] == 0 ? 1 : param_arr[0];
            int m = param_arr[1] == 0 ? 1 : param_arr[1];
            n = min(max(n, 1), 80);
            m = min(max(m, 1), 25);
            move_cursor_to(n, m);
            break;
        }
        case 'J': {
            int n = param_arr[0];
            switch (n) {
                case 0: {
                    // 清除光标位置到屏幕末尾
                    // 清除光标所在行以后的位置
                    for (int x = cursor_x; x <= 80; x++) {
                        set_char_at(x, cursor_y, ' ');
                    }
                    // 清理光标后所有行
                    for (int y = cursor_y + 1; y <= 25; y++) {
                        for (int x = 0; x <= 80; x++) {
                            set_char_at(x, y, ' ');
                        }
                    }
                    break;
                }
                case 1: {
                    // 清除光标位置到屏幕开头
                    // 清除光标所在行以前的位置
                    for (int x = 0; x <= cursor_x; x++) {
                        set_char_at(x, cursor_y, ' ');
                    }
                    // 清理光标前所有行
                    for (int y = 0; y < cursor_y; y++) {
                        for (int x = 0; x <= 80; x++) {
                            set_char_at(x, y, ' ');
                        }
                    }
                    break;
                }
                case 2:
                case 3: {
                    // 清空全屏
                    monitor_clear();
                    break;
                }
            }
        }
        case 'K': {
            int n = param_arr[0];
            switch (n) {
                case 0: {
                    // 清除光标位置到行末尾
                    // 清除光标所在行以后的位置
                    for (int x = cursor_x; x <= 80; x++) {
                        set_char_at(x, cursor_y, ' ');
                    }
                    break;
                }
                case 1: {
                    // 清除光标位置到行开头
                    // 清除光标所在行以前的位置
                    for (int x = 0; x <= cursor_x; x++) {
                        set_char_at(x, cursor_y, ' ');
                    }
                    break;
                }
                case 2:
                case 3: {
                    // 清空整行
                    for (int x = 0; x <= 80; x++) {
                        set_char_at(x, cursor_y, ' ');
                    }
                    break;
                }
            }
        }
        case 's':
            save_x = cursor_x;
            save_y = cursor_y;
            break;
        case 'u':
            move_cursor_to(save_x, save_y);
            break;
        case 'n': {
            if (param_arr[0] != 6) {
                // 只支持\x1b[6n
                return -1;
            }
            int eflags = load_eflags();
            asm("cli");
            // strlen(\e[80;25R) = 8
            char s[10] = {0};
            int len = sprintf(s, "\x1b[%d;%dR", cursor_x, cursor_y);
            for (int i = 0; i < len; i++) fifo_put(&decoded_key, s[i]);
            store_eflags(eflags);
        }
        case 'm': {
            for (int i = 0; i <= param_idx; i++) {
                switch (param_arr[i]) {
                    case 0:
                        fore = 7; back = 0; fore_brighten = true;
                        break;
                    case 7: {
                        int t = fore; fore = back; back = t;
                        break;
                    }
                    case 30: case 31:
                    case 32: case 33:
                    case 34: case 35:
                    case 36: case 37:
                        fore = param_arr[i]; fore_brighten = false;
                        break;
                    case 39:
                        fore = 7; fore_brighten = true;
                        break;
                    case 40: case 41:
                    case 42: case 43:
                    case 44: case 45:
                    case 46: case 47:
                        back = param_arr[i];
                        break;
                    case 49:
                        back = 0;
                        break;
                    case 90: case 91:
                    case 92: case 93:
                    case 94: case 95:
                    case 96: case 97:
                        fore = param_arr[i]; fore_brighten = true;
                        break;
                }
                set_color(fore, back, fore_brighten);
            }
        }
    }
    return ansi - esc_start; // 返回esc以后的全部字符数
}