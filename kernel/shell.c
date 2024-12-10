#include "shell.h" // MAX_CMD_LEN, MAX_ARG_NR
#include "stdio.h"

static char cmd_line[MAX_CMD_LEN] = {0}; // 输入命令行的内容
static char *argv[MAX_ARG_NR] = {NULL}; // argv，字面意思

static void print_prompt() // 输出提示符
{
    printf("[TUTO@localhost /] $ "); // 这一部分大家随便改，你甚至可以改成>>>
}

static void readline(char *buf, int cnt) // 输入一行或cnt个字符
{
    char *pos = buf; // 不想变buf
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

static int cmd_parse(char *cmd_str, char **argv, char token)
{
    int arg_idx = 0;
    while (arg_idx < MAX_ARG_NR) {
        argv[arg_idx] = NULL;
        arg_idx++;
    } // 开局先把上一个argv抹掉
    char *next = cmd_str; // 下一个字符
    int argc = 0; // 这就是要返回的argc了
    while (*next) { // 循环到结束为止
        if (*next != '"') {
            while (*next == token) *next++; // 多个token就只保留第一个，windows cmd就是这么处理的
            if (*next == 0) break; // 如果跳过完token之后结束了，那就直接退出
            argv[argc] = next; // 将首指针赋值过去，从这里开始就是当前参数
            while (*next && *next != token) next++; // 跳到下一个token
        } else {
            next++; // 跳过引号
            argv[argc] = next; // 这里开始就是当前参数
            while (*next && *next != '"') next++; // 跳到引号
        }
        if (*next) { // 如果这里有token字符
            *next++ = 0; // 将当前token字符设为0（结束符），next后移一个
        }
        if (argc > MAX_ARG_NR) return -1; // 参数太多，超过上限了
        argc++; // argc增一，如果最后一个字符是空格时不提前退出，argc会错误地被多加1
    }
    return argc;
}

void cmd_ver(int argc, char **argv)
{
    puts("TutorialOS Indev");
}

void cmd_execute(int argc, char **argv)
{
    if (!strcmp("ver", argv[0])) {
        cmd_ver(argc, argv);
    } else {
        printf("shell: bad command: %s\n", argv[0]);
    }
}

void shell()
{
    puts("TutorialOS Indev (tags/Indev:WIP, Jun 26 2024, 21:09) [GCC 32bit] on baremetal"); // 看着眼熟？这一部分是从 Python 3 里模仿的
    puts("Type \"ver\" for more information.\n"); // 示例，只打算支持这一个
    while (1) { // 无限循环
        print_prompt(); // 输出提示符
        memset(cmd_line, 0, MAX_CMD_LEN);
        readline(cmd_line, MAX_CMD_LEN); // 输入一行命令
        if (cmd_line[0] == 0) continue; // 啥也没有，是换行，直接跳过
        int argc = cmd_parse(cmd_line, argv, ' '); // 解析命令，按照cmd_parse的要求传入，默认分隔符为空格
        cmd_execute(argc, argv); // 执行
    }
    puts("shell: PANIC: WHILE (TRUE) LOOP ENDS! RUNNNNNNN!!!"); // 到不了，不解释
}