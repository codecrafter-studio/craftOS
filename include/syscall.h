#ifndef _SYSCALL_H_
#define _SYSCALL_H_

int sys_getpid();
int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir);

// file.h
int sys_open(char *filename, uint32_t flags);
int sys_write(int fd, const void *msg, int len);
int sys_read(int fd, void *buf, int count);
int sys_close(int fd);
int sys_lseek(int fd, int offset, uint8_t whence);
int sys_unlink(const char *filename);

// exec.c
void *sys_sbrk(int incr);

#endif