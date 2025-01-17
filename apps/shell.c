#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_CMD_LEN 100
#define MAX_ARG_NR 30

static char cmd_line[MAX_CMD_LEN] = {0}; // 输入命令行的内容
static char cmd_line_back[MAX_CMD_LEN] = {0}; // 这一行是新加的
static char *argv[MAX_ARG_NR] = {NULL}; // argv，字面意思

static void print_prompt() // 输出提示符
{
    printf("csh$ "); // 这一部分大家随便改，你甚至可以改成>>>
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

int try_to_run_external(char *name, int *exist)
{
    int ret = create_process(name, cmd_line_back, "/"); // 尝试执行应用程序
    *exist = false; // 文件不存在
    if (ret == -1) { // 哇真的不存在
        char new_name[MAX_CMD_LEN] = {0}; // 由于还没有实现malloc，所以只能这么搞，反正文件最长就是MAX_CMD_LEN这么长
        strcpy(new_name, name); // 复制文件名
        int len = strlen(name); // 文件名结束位置
        new_name[len] = '.'; // 给后
        new_name[len + 1] = 'c'; // 缀加
        new_name[len + 2] = 'x'; // 上个
        new_name[len + 3] = 'p'; // .cxp
        new_name[len + 4] = '\0'; // 结束符
        ret = create_process(new_name, cmd_line_back, "/"); // 第二次尝试执行应用程序
        if (ret == -1) return -1; // 文件还是不存在，那只能不存在了
    }
    *exist = true; // 错怪你了，文件存在
    ret = waitpid(ret); // 等待直到这个pid的进程返回并拿到结果
    return ret; // 把返回值返回回去
}

void cmd_ver(int argc, char **argv)
{
    puts("craftOS 1.0");
}

void cmd_execute(int argc, char **argv)
{
    if (!strcmp("ver", argv[0])) {
        cmd_ver(argc, argv);
    } else {
        int exist;
        int ret = try_to_run_external(argv[0], &exist);
        if (!exist) {
            printf("csh: `%s` is not recognized as an internal or external command or executable file.\n", argv[0]);
        } else if (ret) {
            printf("csh: app `%s` exited abnormally, retval: %d (0x%x).\n", argv[0], ret, ret);
        }
    }
}

void shell()
{
    puts("craftOS 1.0 (v1.0, Jan 16 2025, 21:40:00) [GCC 14.2.0] on barametal"); // 看着眼熟？这一部分是从 Python 3 里模仿的
    puts("Type \"ver\" for more information.\n"); // 示例，只打算支持这一个
    while (1) { // 无限循环
        print_prompt(); // 输出提示符
        memset(cmd_line, 0, MAX_CMD_LEN);
        readline(cmd_line, MAX_CMD_LEN); // 输入一行命令
        if (cmd_line[0] == 0) continue; // 啥也没有，是换行，直接跳过
        strcpy(cmd_line_back, cmd_line);
        int argc = cmd_parse(cmd_line, argv, ' '); // 解析命令，按照cmd_parse的要求传入，默认分隔符为空格
        cmd_execute(argc, argv); // 执行
    }
    puts("csh: PANIC: WHILE (TRUE) LOOP ENDS! RUNNNNNNN!!!"); // 到不了，不解释
}

int main()
{
    shell();
    return 0;
}