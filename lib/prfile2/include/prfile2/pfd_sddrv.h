#ifndef PRFILE2_PFD_SDDRV_H
#define PRFILE2_PFD_SDDRV_H

// SD card disk driver of the standalone PrFILE2 (lib/prfile2/driver/sd_drv.c).

#include <prfile2/pdm_api.h>
#include <prfile2/pdm_bpb.h>
#include <revolution/sdi.h>

#ifdef __cplusplus
extern "C" {
#endif

// PFD_SDDRV_INFO.flags
#define PFD_SDDRV_INITIALIZED 1
#define PFD_SDDRV_MOUNTED 2
#define PFD_SDDRV_CARD_INITIALIZED 4

#define PFD_SDDRV_SLOT 0  // ISD_MountCard slot of the SD card
#define PFD_SDDRV_FORMAT_PARAM_COUNT 12
#define PFD_SDDRV_RESET_RETRY_COUNT 5
#define PFD_SDDRV_MAX_TOTAL_SECTORS 0x400000  // 2 GB (SDSC)
#define PFD_SDDRV_BUF_ALIGN 32  // ISD transfers need 32-byte aligned buffers
#define PFD_SDDRV_CID_LEN 16    // 128-bit CID register
#define PFD_SDDRV_NUM_SUCCESS_UNKNOWN 0xFFFFFFFF  // transfer in progress; every return path overwrites it

// FAT layout of pfd_sddrv_full_format
#define PFD_SDDRV_NUM_RESERVED_SECTORS 1
#define PFD_SDDRV_NUM_FATS 2
#define PFD_SDDRV_ROOT_DIR_ENTRIES 512
#define PFD_SDDRV_FIXED_SYSTEM_SECTORS (PFD_SDDRV_NUM_RESERVED_SECTORS + PFD_SDDRV_ROOT_DIR_ENTRIES * PDM_DIR_ENTRY_SIZE / PDM_SECTOR_SIZE)
#define PFD_SDDRV_SECTOR_BITS (PDM_SECTOR_SIZE * 8)
#define PFD_SDDRV_FAT12_MARGIN 16  // cluster counts this close to the FAT12 limit are avoided
#define PFD_SDDRV_FAT12_MAX_SECTORS 0x7FA8  // smaller partitions get PDM_PART_TYPE_FAT12
#define PFD_SDDRV_VOL_ID_BASE_YEAR 1900

// CSD register fields
#define PFD_SDDRV_CSD_C_SIZE_LO_BITS 10   // C_SIZE is split across two words
#define PFD_SDDRV_CSD_C_SIZE_MULT_BIAS 2  // MULT = 2^(C_SIZE_MULT + 2)
#define PFD_SDDRV_CSD_READ_BL_LEN_MIN 9   // 512-byte blocks
#define PFD_SDDRV_CSD_READ_BL_LEN_MAX 11  // 2048-byte blocks

// driver error codes
enum {
    PFD_SDDRV_ERR_INVALID_PARAM = -30,
    PFD_SDDRV_ERR_UNSUPPORTED_FAT = -31,
    PFD_SDDRV_ERR_WRITE_PROTECTED = -32,
    PFD_SDDRV_ERR_NOT_INSERTED = -33,  // card absent, removed or changed
    PFD_SDDRV_ERR_TOO_LARGE = -34,
    PFD_SDDRV_ERR_NOT_MEMORY_CARD = -35,
    PFD_SDDRV_ERR_WRITE = -36,
    PFD_SDDRV_ERR_READ = -37,
    PFD_SDDRV_ERR_RESET = -38,
    PFD_SDDRV_ERR_GET_STATUS = -39,
    PFD_SDDRV_ERR_INIT_CARD = -40,
    PFD_SDDRV_ERR_MOUNT_CARD = -41,
    PFD_SDDRV_ERR_INTR_HANDLER = -42,
    PFD_SDDRV_ERR_READ_REGISTER = -43,
    PFD_SDDRV_ERR_OTHER_DISK = -44,  // already initialized for another disk
    PFD_SDDRV_ERR_NOT_MOUNTED = -45,
};

// ISD_GetDeviceStatus status bits, also the interrupt handler events
enum {
    PFD_SDDRV_STAT_INSERTED = 0x1,
    PFD_SDDRV_STAT_REMOVED = 0x2,
    PFD_SDDRV_STAT_WRITE_PROTECTED = 0x4,
    PFD_SDDRV_STAT_MEMORY_CARD = 0x10000,
};

// ISD_ReadCardRegister registers, named by the SD command that reads them
enum {
    PFD_SDDRV_REG_RCA = 3,
    PFD_SDDRV_REG_CSD = 9,
    PFD_SDDRV_REG_CID = 10,
    PFD_SDDRV_REG_OCR = 41,
};

typedef struct PFD_SDDRV_INFO {
    u32 flags;             // 0x00
    u32 bytes_per_sector;  // 0x04
    SDDev* p_dev;          // 0x08
    PDM_DISK* p_disk;      // 0x0C
    BOOL is_inserted;      // 0x10
    BOOL is_changed;       // 0x14
    s8 drv_char;           // 0x18
} PFD_SDDRV_INFO;

typedef struct PFD_SDDRV_FORMAT_PARAM {
    u32 min_sectors;         // 0x00
    u32 max_sectors;         // 0x04
    u32 boundary_unit;       // 0x08
    u32 heads;               // 0x0C
    u32 sectors_per_track;   // 0x10
    u8 sectors_per_cluster;  // 0x14
} PFD_SDDRV_FORMAT_PARAM;

typedef struct PFD_SDDRV_FORMAT_VALUE {
    u32 boundary_unit;       // 0x00
    u32 heads;               // 0x04
    u32 sectors_per_track;   // 0x08
    u8 sectors_per_cluster;  // 0x0C
} PFD_SDDRV_FORMAT_VALUE;

typedef struct PFD_SDDRV_MBR_BPB {
    u32 start_sector;        // 0x00
    u32 num_sectors;         // 0x04
    u8 sectors_per_cluster;  // 0x08
    u32 sectors_per_FAT;     // 0x0C
    u32 vol_id;              // 0x10
    u32 fat_type;            // 0x14
    u32 total_sectors;       // 0x18
} PFD_SDDRV_MBR_BPB;

extern volatile PFD_SDDRV_INFO g_pfd_sddrv_info;

s32 pfd_st_inter_callback(s32 event, void* arg);
s32 pfd_st_removal_callback(s32 event, void* arg);
s32 pfd_sddrv_init(PDM_DISK* p_disk);
s32 pfd_sddrv_mount(PDM_DISK* p_disk);
s32 pfd_sddrv_format(PDM_DISK* p_disk, const u8* param);
s32 pfd_sddrv_pread(PDM_DISK* p_disk, u8* buf, u32 block, u32 num_block, u32* p_num_success);
s32 pfd_sddrv_pwrite(PDM_DISK* p_disk, const u8* buf, u32 block, u32 num_block, u32* p_num_success);
s32 pfd_sddrv_unmount(PDM_DISK* p_disk);
s32 pfd_sddrv_finalize(PDM_DISK* p_disk);
s32 pfd_sddrv_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info);
s32 pfd_sddrv_physical_read(u32 num_sectors, u8* buf, u32 sector, u32 bps, u32* p_num_success);
s32 pfd_sddrv_physical_write(u32 num_sectors, const u8* buf, u32 sector, u32 bps, u32* p_num_success);
s32 pfd_sddrv_get_total_sectors(u32* p_total_sectors, u16* p_bytes_per_sector);
s32 pfd_sddrv_calc_mbr_bpb(PFD_SDDRV_MBR_BPB* p_param);
s32 pfd_sddrv_store_bpb_buf(PFD_SDDRV_MBR_BPB* p_param, u8* buf);
s32 pfd_sddrv_store_mbr_buf(PFD_SDDRV_MBR_BPB* p_param, u8* buf);
s32 pfd_sddrv_build_mbr_bpb(u32 total_sectors);
s32 pfd_sddrv_full_format(void);

#ifdef __cplusplus
}
#endif
#endif
