#ifndef _STRING_H_
#define _STRING_H_

#include "stdint.h"

void *memset(void *dst_, uint8_t value, uint32_t size);
void *memcpy(void *dst_, const void *src_, uint32_t size);
int memcmp(const void *a_, const void *b_, uint32_t size);
char *strcpy(char *dst_, const char *src_);
uint32_t strlen(const char *str);
int8_t strcmp(const char *a, const char *b);
char *strchr(const char *str, const uint8_t ch);
int startswith(const char *str, const char *prefix);
int endswith(const char *str, const char *suffix);

#endif