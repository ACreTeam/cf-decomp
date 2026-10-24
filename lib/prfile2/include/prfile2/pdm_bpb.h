#ifndef PRFILE2_PDM_BPB_H
#define PRFILE2_PDM_BPB_H

#include <prfile2/pdm_api.h>
#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// boot sector (BPB) field offsets
enum {
    PDM_BPB_JUMP_BOOT = 0x00,
    PDM_BPB_OEM_NAME = 0x03,
    PDM_BPB_BYTES_PER_SECTOR = 0x0B,
    PDM_BPB_SECTORS_PER_CLUSTER = 0x0D,
    PDM_BPB_NUM_RESERVED_SECTORS = 0x0E,
    PDM_BPB_NUM_FATS = 0x10,
    PDM_BPB_NUM_ROOT_DIR_ENTRIES = 0x11,
    PDM_BPB_TOTAL_SECTORS16 = 0x13,
    PDM_BPB_MEDIA = 0x15,
    PDM_BPB_SECTORS_PER_FAT16 = 0x16,
    PDM_BPB_SECTORS_PER_TRACK = 0x18,
    PDM_BPB_NUM_HEADS = 0x1A,
    PDM_BPB_NUM_HIDDEN_SECTORS = 0x1C,
    PDM_BPB_TOTAL_SECTORS32 = 0x20,

    // FAT12/FAT16
    PDM_BPB_DRIVE = 0x24,
    PDM_BPB_RESERVED = 0x25,
    PDM_BPB_BOOT_SIG = 0x26,
    PDM_BPB_VOL_ID = 0x27,
    PDM_BPB_VOL_LABEL = 0x2B,
    PDM_BPB_FS_TYPE = 0x36,

    // FAT32
    PDM_BPB_SECTORS_PER_FAT32 = 0x24,
    PDM_BPB_EXT_FLAGS = 0x28,
    PDM_BPB_FS_VERSION = 0x2A,
    PDM_BPB_ROOT_DIR_CLUSTER = 0x2C,
    PDM_BPB_FS_INFO_SECTOR = 0x30,
    PDM_BPB_BACKUP_BOOT_SECTOR = 0x32,
    PDM_BPB_DRIVE32 = 0x40,
    PDM_BPB_RESERVED32 = 0x41,
    PDM_BPB_BOOT_SIG32 = 0x42,
    PDM_BPB_VOL_ID32 = 0x43,
    PDM_BPB_VOL_LABEL32 = 0x47,
    PDM_BPB_FS_TYPE32 = 0x52,

    PDM_BPB_SIGNATURE1 = 0x1FE,
    PDM_BPB_SIGNATURE2 = 0x1FF,
};

#define PDM_BPB_OEM_NAME_LEN 8
#define PDM_BOOT_SECTOR 0  // logical sector of the partition boot sector (BPB)
#define PDM_FSINFO_SECTOR 1  // logical sector of the FAT32 FSINFO sector
#define PDM_BPB_FS_TYPE_LEN 8

#define PDM_BPB_TOTAL_SECTORS16_LIMIT 0x10000  // larger counts go in total_sectors32

enum {
    PDM_BPB_JUMP_SHORT = 0xEB,  // JMP rel8; followed by NOP at offset 2
    PDM_BPB_JUMP_NEAR = 0xE9,
    PDM_BPB_NOP = 0x90,
    PDM_BPB_SIGNATURE1_VALUE = 0x55,
    PDM_BPB_SIGNATURE2_VALUE = 0xAA,
    PDM_BPB_EXT_BOOT_SIG = 0x29,
    PDM_BPB_MEDIA_REMOVABLE = 0xF0,
    PDM_BPB_MEDIA_FIXED = 0xF8,
    PDM_BPB_DRIVE_HARD_DISK = 0x80,
};

// BPB ext_flags
enum {
    PDM_BPB_EXT_FLAG_ACTIVE_FAT_MASK = 0x07,
    PDM_BPB_EXT_FLAG_NO_MIRRORING = 0x80,
};

#define PDM_DIR_ENTRY_SIZE 32

// cluster count limits that decide the FAT type
#define PDM_FAT12_MAX_CLUSTERS 0xFF5
#define PDM_FAT16_MAX_CLUSTERS 0xFFF5

// FSINFO sector field offsets and signatures
enum {
    PDM_FSI_LEAD_SIG = 0x000,
    PDM_FSI_STRUC_SIG = 0x1E4,
    PDM_FSI_FREE_COUNT = 0x1E8,
    PDM_FSI_NXT_FREE = 0x1EC,
    PDM_FSI_TRAIL_SIG = 0x1FC,
};

#define PDM_FSI_LEAD_SIG_VALUE 0x41615252
#define PDM_FSI_STRUC_SIG_VALUE 0x61417272
#define PDM_FSI_TRAIL_SIG_VALUE 0xAA550000
#define PDM_FSI_UNKNOWN 0xFFFFFFFF  // free_count / nxt_free not known

void pdm_bpb_CalcBPBFields(PF_BPB* p_bpb);
s32 pdm_bpb_InitVolumeWithBootSector(PF_BPB* p_bpb, u8* buf);
s32 pdm_bpb_InitVolumeWithFSINFOSector(PF_BPB* p_bpb, u8* buf, u32* p_free_count, u32* p_next_free);

#ifdef __cplusplus
}
#endif
#endif
