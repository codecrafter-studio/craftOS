#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "common.h"

int get_cursor_pos();
void move_cursor_to(int new_x, int new_y);
void set_color(int fore, int back, int fore_brighten);
int get_color();
void set_char_at(int x, int y, char ch);
void monitor_put(char c); // 打印字符
void monitor_clear(); // 清屏
void monitor_write(char *s); // 打印字符串
void monitor_write_hex(uint32_t hex); // 打印十六进制数
void monitor_write_dec(uint32_t dec); // 打印十进制数

#endif