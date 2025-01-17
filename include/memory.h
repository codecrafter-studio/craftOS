#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

void init_memory();
void *kmalloc(uint32_t size);
void kfree(void *p);
void *krealloc(void *buffer, int size);

#define MEMMAN_FREES 4090

typedef struct FREEINFO {
    uint32_t addr, size;
} freeinfo_t;

typedef struct MEMMAN {
    int frees;
    freeinfo_t free[MEMMAN_FREES];
} memman_t;

#define MEMMAN_ADDR 0x003c0000

#endif