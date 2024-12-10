#ifndef _ISR_H_
#define _ISR_H_

#include "common.h"

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, user_esp, ss;
} registers_t;

#endif