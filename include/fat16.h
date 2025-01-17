#ifndef FAT16_H
#define FAT16_H

int fat16_format_hd();
int lfn2sfn(const char *lfn, char *sfn);
fileinfo_t *read_dir_entries(int *dir_ents);
int fat16_create_file(fileinfo_t *finfo, char *filename);
int fat16_open_file(fileinfo_t *finfo, char *filename);
static uint16_t get_nth_fat(uint16_t n);
static void set_nth_fat(uint16_t n, uint16_t val);
static void read_nth_clust(uint16_t n, void *clust);
static void write_nth_clust(uint16_t n, const void *clust);
int fat16_read_file(fileinfo_t *finfo, void *buf);
int fat16_delete_file(char *filename);
int fat16_write_file(fileinfo_t *finfo, const void *buf, uint32_t size);

#endif