#include "stdio.h"
#include "monitor.h"

int printk(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[1024] = {0};
    int ret = vsprintf(buf, fmt, ap);
    va_end(ap);
    monitor_write(buf);
    return ret;
}