#include "cmos.h"

static uint8_t read_cmos(uint8_t p)
{
    uint8_t data;
    outb(CMOS_INDEX, p);
    data = inb(CMOS_DATA);
    outb(CMOS_INDEX, 0x80);
    return data;
}

#ifdef NEED_UTC_8
static bool is_leap_year(int year)
{
    if (year % 400 == 0) return true;
    return year % 4 == 0 && year % 100 != 0;
}
#endif

void get_current_time(current_time_t *ctime)
{
    ctime->year = bcd2hex(read_cmos(CMOS_CUR_CEN)) * 100 + bcd2hex(read_cmos(CMOS_CUR_YEAR));
    ctime->month = bcd2hex(read_cmos(CMOS_CUR_MON));
    ctime->day = bcd2hex(read_cmos(CMOS_CUR_DAY));
    ctime->hour = bcd2hex(read_cmos(CMOS_CUR_HOUR));
    ctime->min = bcd2hex(read_cmos(CMOS_CUR_MIN));
    ctime->sec = bcd2hex(read_cmos(CMOS_CUR_SEC));
#ifdef NEED_UTC_8
    int day_of_months[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap_year(ctime->year)) day_of_months[2]++;
    // 校正时间
    ctime->hour += 8;
    if (ctime->hour >= 24) ctime->hour -= 24, ctime->day++;
    if (ctime->day > day_of_months[ctime->month]) ctime->day = 1, ctime->month++;
    if (ctime->month > 12) ctime->month = 1, ctime->year++;
#endif
}