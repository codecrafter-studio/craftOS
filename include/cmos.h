#ifndef _CMOS_H_
#define _CMOS_H_

#include "common.h"

#define CMOS_INDEX 0x70
#define CMOS_DATA  0x71

#define CMOS_CUR_SEC 0x0
#define CMOS_CUR_MIN 0x2
#define CMOS_CUR_HOUR 0x4
#define CMOS_CUR_DAY 0x7
#define CMOS_CUR_MON 0x8
#define CMOS_CUR_YEAR 0x9
#define CMOS_CUR_CEN 0x32

#define bcd2hex(n) (((n >> 4) * 10) + (n & 0xf))

typedef struct {
    int year, month, day, hour, min, sec;
} current_time_t;

#define NEED_UTC_8

#endif