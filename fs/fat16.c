#include "hd.h"
#include "memory.h"
#include "file.h"
#include "cmos.h"

// 格式化文件系统
int fat16_format_hd()
{
    static unsigned char default_boot_code[] = {
        0x8c, 0xc8, 0x8e, 0xd8, 0x8e, 0xc0, 0xb8, 0x00, 0x06, 0xbb, 0x00, 0x07, 0xb9, 0x00, 0x00, 0xba,
        0x4f, 0x18, 0xcd, 0x10, 0xb6, 0x00, 0xe8, 0x02, 0x00, 0xeb, 0xfe, 0xb8, 0x6c, 0x7c, 0x89, 0xc5,
        0xb9, 0x2a, 0x00, 0xb8, 0x01, 0x13, 0xbb, 0x07, 0x00, 0xb2, 0x00, 0xcd, 0x10, 0xc3, 0x46, 0x41,
        0x54, 0x41, 0x4c, 0x3a, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6f, 0x6f, 0x74, 0x61,
        0x62, 0x6c, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6b, 0x2e, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d,
        0x20, 0x68, 0x61, 0x6c, 0x74, 0x65, 0x64, 0x2e, 0x00, 0x00
    }; // 这段代码的意思是：输出一段信息，是用nasm写完编译的
    char *fat1 = (char *) kmalloc(512);
    hd_read(FAT1_START_LBA, 1, fat1); // 读取FAT表第一个扇区
    if (fat1[0] == 0xff) { // 如果第一个字节是0xff，那就是有文件系统
        kfree(fat1);
        return 1; // 那就没有必要格式化了
    }
    kfree(fat1);
    int sectors = get_hd_sects(); // 获取硬盘扇区大小先存着
    bpb_hdr_t hdr; // 构造一个引导扇区
    hdr.BS_jmpBoot[0] = 0xeb;
    hdr.BS_jmpBoot[1] = 0x3c; // jmp到default_boot_code
    hdr.BS_jmpBoot[2] = 0x90; // nop凑够3字节
    strcpy(hdr.BS_OEMName, "TUTORIAL"); // OEM为tutorial
    hdr.BPB_BytsPerSec = 512;
    hdr.BPB_SecPerClust = 1;
    hdr.BPB_RsvdSecCnt = 1;
    hdr.BPB_NumFATs = 2; // 总共两个FAT，这是规定
    hdr.BPB_RootEntCnt = 512; // 根目录区32个扇区，一个目录项占32字节，32*512/32=512
    if (sectors < (1 << 16) - 1) {
        hdr.BPB_TotSec16 = sectors;
        hdr.BPB_TotSec32 = 0;
    } else {
        hdr.BPB_TotSec16 = 0;
        hdr.BPB_TotSec32 = sectors;
    }
    hdr.BPB_Media = 0xf8; // 硬盘统一数据
    hdr.BPB_FATSz16 = 32; // FAT16是这样的
    hdr.BPB_SecPerTrk = 63; // 硬盘统一数据
    hdr.BPB_NumHeads = 16; // 硬盘统一数据
    hdr.BPB_HiddSec = 0;
    hdr.BS_DrvNum = 0x80; // 硬盘统一数据
    hdr.BS_Reserved1 = 0;
    hdr.BS_BootSig = 0x29;
    hdr.BS_VolID = 0;
    strcpy(hdr.BS_VolLab, "FOOLISHABBY"); // 可以随便改
    strcpy(hdr.BS_FileSysType, "FAT16   "); // 尽量别改
    memset(hdr.BS_BootCode, 0, 448);
    memcpy(hdr.BS_BootCode, default_boot_code, sizeof(default_boot_code));
    hdr.BS_BootEndSig = 0xaa55;
    hd_write(0, 1, &hdr); // 引导扇区就这样了
    char initial_fat[512] = {0xff, 0xf8, 0xff, 0xff, 0}; // 硬盘统一数据
    hd_write(FAT1_START_LBA, 1, &initial_fat); // 写入FAT1
    hd_write(FAT1_START_LBA + FAT1_SECTORS, 1, &initial_fat); // 写入FAT2
    return 0;
}

// 把原文件名改编为FAT16所要求的8.3格式
int lfn2sfn(const char *lfn, char *sfn)
{
    int len = strlen(lfn), last_dot = -1;
    for (int i = len - 1; i >= 0; i--) { // 从尾到头遍历，寻找最后一个.的位置
        if (lfn[i] == '.') { // 找到了
            last_dot = i; // 最后一个.赋值一下
            break; // 跳出循环
        }
    }
    if (last_dot == -1) last_dot = len; // 没有扩展名，那就在最后虚空加个.
    if (lfn[0] == '.') return -1; // 首字符是.，不支持
    int len_name = last_dot, len_ext = len - 1 - last_dot; // 计算文件名与扩展名各自有多长
    if (len_name > 8) return -1; // 文件名长于8个字符，不支持
    if (len_ext > 3) return -1; // 扩展名长于3个字符，不支持
    // 事实上FAT对此有解决方案，称为长文件名（LFN），但实现较为复杂，暂时先不讨论
    char *name = (char *) kmalloc(10); // 多分配点内存
    char *ext = NULL; // ext不一定有
    if (len_ext > 0) ext = (char *) kmalloc(5); // 有扩展名，分配内存
    memcpy(name, lfn, len_name); // 把name从lfn中拷出来
    if (ext) memcpy(ext, lfn + last_dot + 1, len_ext); // 把ext从lfn中拷出来
    if (name[0] == 0xe5) name[0] = 0x05; // 如果第一个字节恰好是0xe5（已删除），将其更换为0x05
    for (int i = 0; i < len_name; i++) { // 处理文件名
        if (name[i] == '.') return -1; // 文件名中含有.，不支持
        if ((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= '0' && name[i] <= '9')) sfn[i] = name[i]; // 数字或字母留为原样
        else sfn[i] = '_'; // 其余字符变为下划线
        if (sfn[i] >= 'a' && sfn[i] <= 'z') sfn[i] -= 0x20; // 小写变大写
    }
    for (int i = len_name; i < 8; i++) sfn[i] = ' '; // 用空格填充剩余部分
    for (int i = 0; i < len_ext; i++) { // 处理扩展名
        if ((ext[i] >= 'a' && ext[i] <= 'z') || (ext[i] >= 'A' && name[i] <= 'Z') || (ext[i] >= '0' && ext[i] <= '9')) sfn[i + 8] = ext[i]; // 数字或字母留为原样
        else sfn[i + 8] = '_'; // 其余字符变为下划线
        if (sfn[i + 8] >= 'a' && sfn[i + 8] <= 'z') sfn[i + 8] -= 0x20; // 小写变大写
    }
    if (len_ext > 0) {
        for (int i = len_ext; i < 3; i++) sfn[i + 8] = ' '; // 用空格填充剩余部分
    } else {
        for (int i = 0; i < 3; i++) sfn[i + 8] = ' '; // 用空格填充剩余部分
    }
    sfn[11] = 0; // 文件名的结尾加一个\0
    return 0; // 正常退出
}

// 读取根目录目录项
fileinfo_t *read_dir_entries(int *dir_ents)
{
    fileinfo_t *root_dir = (fileinfo_t *) kmalloc(ROOT_DIR_SECTORS * SECTOR_SIZE);
    hd_read(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 将根目录的所有扇区全部读入
    int i;
    for (i = 0; i < MAX_FILE_NUM; i++) {
        if (root_dir[i].name[0] == 0) break; // 如果名字的第一个字节是0，那就说明这里没有文件
    }
    *dir_ents = i; // 将目录项个数写到指针里
    return root_dir; // 返回根目录
}

// 创建文件
int fat16_create_file(fileinfo_t *finfo, char *filename)
{
    if (filename[0] == 0xe5) filename[0] = 0x05; // 如上，若第一个字节为 0xe5，需要更换为 0x05
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将文件名转换为8.3文件名
    if (ret) return -1; // 文件名不符合8.3规范，返回
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries); // 读取所有根目录项
    int free_slot = entries; // 默认的空闲位置是最后一个
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, sfn, 8) && !memcmp(root_dir[i].ext, sfn + 8, 3)) { // 文件名和扩展名都一样
            kfree(root_dir); // 已经有了就不用创建了
            return -1;
        }
        if (root_dir[i].name[0] == 0xe5) { // 已经删除（文件名第一个字节是0xe5）
            free_slot = i; // 那就把这里当成空闲位置
            break;
        }
    }
    if (free_slot == MAX_FILE_NUM) { // 如果空闲位置已经到达根目录末尾
        kfree(root_dir); // 没地方创建也就不用创建了
        return -1;
    }
    // 开始填入fileinfo_t对应的项
    memcpy(root_dir[free_slot].name, sfn, 8); // sfn为name与ext的合体，前8个字节是name
    memcpy(root_dir[free_slot].ext, sfn + 8, 3); // 后3个字节是ext
    root_dir[free_slot].type = 0x20; // 类型为0x20（正常文件）
    root_dir[free_slot].clustno = 0; // 没有内容，所以没有簇号（同样放在下一节讲）
    root_dir[free_slot].size = 0; // 没有内容，所以大小为0
    memset(root_dir[free_slot].reserved, 0, 10); // 将预留部分全部设为0
    current_time_t ctime;
    get_current_time(&ctime); // 获取当前时间
    // 按照前文所说依次填入date和time
    root_dir[free_slot].date = ((ctime.year - 1980) << 9) | (ctime.month << 5) | ctime.day;
    root_dir[free_slot].time = (ctime.hour << 11) | (ctime.min << 5) | ctime.sec;
    if (finfo) *finfo = root_dir[free_slot]; // 创建完了不能不管，传给finfo留着
    hd_write(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 将新的根目录区写回硬盘
    kfree(root_dir); // 成功完成
    return 0;
}

// 打开文件
int fat16_open_file(fileinfo_t *finfo, char *filename)
{
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将原文件名转换为8.3
    if (ret) return -1; // 转换失败，不用打开了
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries); // 读取所有目录项
    int file_index = entries; // filename对应文件的索引
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, sfn, 8) && !memcmp(root_dir[i].ext, sfn + 8, 3)) {
            file_index = i; // 找到了
            break;
        }
    }
    if (file_index < entries) { // 如果找到了……
        *finfo = root_dir[file_index]; // 那么把对应的文件存到finfo里
        kfree(root_dir);
        return 0;
    }
    else {
        finfo = NULL; // 这一句实际上是没有用的
        kfree(root_dir);
        return -1;
    }
}