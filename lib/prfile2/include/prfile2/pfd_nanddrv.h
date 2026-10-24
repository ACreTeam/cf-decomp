#ifndef PRFILE2_PFD_NANDDRV_H
#define PRFILE2_PFD_NANDDRV_H

// NAND file backed disk driver of the standalone PrFILE2 (lib/prfile2/driver/nand_drv.c).

#include <prfile2/pdm_api.h>
#include <revolution/nand.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FA_NANDDISK_COUNT 23
#define FA_NANDDISK_PATH_LEN_MAX 0x4C

// FA_NANDDISK_INFO.flags
#define FA_NANDDISK_MOUNTED 2
#define FA_NANDDISK_FORMAT 4

// geometry and volume ID of a freshly built NAND disk image
#define FA_NANDDISK_HEADS 255
#define FA_NANDDISK_SECTORS_PER_TRACK 63
#define FA_NANDDISK_SECTORS_PER_CLUSTER 1
#define FA_NANDDISK_NUM_RESERVED_SECTORS 1
#define FA_NANDDISK_ROOT_DIR_ENTRIES 128
#define FA_NANDDISK_NUM_FATS 2
#define FA_NANDDISK_VOL_ID 0x1234

// longest NAND file and directory name accepted in a disk image path
#define FA_NANDDRV_FILE_NAME_MAX 12
#define FA_NANDDRV_DIR_NAME_MAX 11
#define FA_NANDDRV_NAME_BUF_LEN 16
#define FA_NANDDRV_IO_BUF_SIZE 512  // 32-byte aligned bounce buffer for NANDRead/NANDWrite

// driver error codes
enum {
    FA_NANDDRV_ERR_INVALID_PARAM = -20,
    FA_NANDDRV_ERR_SYSTEM = -21,     // NAND init, semaphore creation, disk already mounted
    FA_NANDDRV_ERR_NOT_FOUND = -22,  // no matching or free disk entry; also I/O past the end of the file
};

typedef struct FA_NANDDISK_INFO {
    PDM_DISK* p_disk;   // 0x00
    char path[0x50];    // 0x04
    u32 size;           // 0x54
    NANDFileInfo file;  // 0x58
    u32 flags;          // 0xE4
    u32 _E8;            // 0xE8
    s32 last_error;     // 0xEC
} FA_NANDDISK_INFO;

s32 fa_nanddrv_RelateNANDFileToDsk(PDM_DISK* p_disk, FA_NANDDISK_INFO* p_info, BOOL format);
s32 fa_nanddrv_CreateNANDFileForDsk(FA_NANDDISK_INFO* p_info);
s32 fa_nanddrv_ParseCreateNANDFile(FA_NANDDISK_INFO* p_info);
s32 fa_nanddrv_VerifyBPB(u8* buf, u32* p_num_clusters);
s32 fa_nanddrv_BuildUpFSInfoSector(u8* buf);
s32 fa_nanddrv_BuildUpBootSector(FA_NANDDISK_INFO* p_info, u8* buf, s32* p_fat_type, u32 num_clusters);
s32 fa_nanddrv_init(PDM_DISK* p_disk);
s32 fa_nanddrv_mount(PDM_DISK* p_disk);
s32 fa_nanddrv_format(PDM_DISK* p_disk, const u8* param);
s32 fa_nanddrv_pread(PDM_DISK* p_disk, u8* p_buf, u32 block, u32 num_blocks, u32* p_num_success);
s32 fa_nanddrv_pwrite(PDM_DISK* p_disk, const u8* p_buf, u32 block, u32 num_blocks, u32* p_num_success);
s32 fa_nanddrv_unmount(PDM_DISK* p_disk);
s32 fa_nanddrv_finalize(PDM_DISK* p_disk);
s32 fa_nanddrv_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info);
s32 fa_nanddrv_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext);
s32 fa_nanddrv_NotifyNANDFile(const char* path, u32 size);
s32 fa_nanddrv_physical_read(u32 num_blocks, u8* buf, u32 block, u32 bps, u32* p_num_success, PDM_DISK* p_disk);
s32 fa_nanddrv_physical_write(u32 num_blocks, const u8* buf, u32 block, u32 bps, u32* p_num_success, PDM_DISK* p_disk);

#ifdef __cplusplus
}
#endif
#endif
