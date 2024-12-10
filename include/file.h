#ifndef _FILE_H_
#define _FILE_H_

#include "common.h"

typedef struct FILEINFO {
    uint8_t name[8], ext[3];
    uint8_t type, reserved[10];
    uint16_t time, date, clustno;
    uint32_t size;
}  __attribute__((packed)) fileinfo_t;

typedef struct FAT_BPB_HEADER {
    unsigned char BS_jmpBoot[3];
    unsigned char BS_OEMName[8];
    unsigned short BPB_BytsPerSec;
    unsigned char BPB_SecPerClust;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs;
    unsigned short BPB_RootEntCnt;
    unsigned short BPB_TotSec16;
    unsigned char BPB_Media;
    unsigned short BPB_FATSz16;
    unsigned short BPB_SecPerTrk;
    unsigned short BPB_NumHeads;
    unsigned int BPB_HiddSec;
    unsigned int BPB_TotSec32;
    unsigned char BS_DrvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;
    unsigned int BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FileSysType[8];
    unsigned char BS_BootCode[448];
    unsigned short BS_BootEndSig;
} __attribute__((packed)) bpb_hdr_t;

#define SECTOR_SIZE 512
#define FAT1_SECTORS 32
#define ROOT_DIR_SECTORS 32
#define FAT1_START_LBA 1
#define ROOT_DIR_START_LBA 65
#define DATA_START_LBA 97
#define SECTOR_CLUSTER_BALANCE (DATA_START_LBA - 2)
#define MAX_FILE_NUM 512

typedef enum FILE_TYPE {
    FT_USABLE,
    FT_REGULAR,
    FT_UNKNOWN
} file_type_t;

typedef enum oflags {
    O_RDONLY,
    O_WRONLY,
    O_RDWR,
    O_CREAT = 4
} oflags_t;

typedef struct FILE_STRUCT {
    void *handle;
    void *buffer;
    int pos;
    int size;
    int open_cnt;
    file_type_t type;
    oflags_t flags;
} file_t;

typedef enum seeks {
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
} seeks_t;

#endif