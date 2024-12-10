#ifndef _HD_H_
#define _HD_H_

void hd_read(int lba, int sec_cnt, void *buffer);
void hd_write(int lba, int sec_cnt, void *buffer);
int get_hd_sects();

#endif