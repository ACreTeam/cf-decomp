#ifndef PRFILE2_PDM_API_H
#define PRFILE2_PDM_API_H

// Disk manager of the standalone PrFILE2 (lib/prfile2/dskmng). The disk and partition records are the FADisk and
// FAPartition of include/lib/revolution/FA/fa_types.h.

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PDM_DRIVE_COUNT 26

#define PDM_SECTOR_SIZE 512  // also the unit of the drivers' physical blocks
#define PDM_SECTOR_SHIFT 9

enum {
    PDM_FAT_12 = 0,
    PDM_FAT_16 = 1,
    PDM_FAT_32 = 2,
    PDM_FAT_ERR = -1,
};

// disk manager error codes
enum {
    PDM_ERR_INVALID_MASTER_BOOT = 2,
    PDM_ERR_INVALID_BOOT_SECTOR = 3,
    PDM_ERR_INVALID_BPB = 4,
    PDM_ERR_NOT_EXIST_PARTITION = 7,
    PDM_ERR_NOT_EXIST_FREE_DISK_STRUCT = 8,
    PDM_ERR_NOT_EXIST_FREE_PARTITION_STRUCT = 10,
    PDM_ERR_ALREADY_ATTACHED = 11,
    PDM_ERR_ALREADY_MOUNTED = 12,
    PDM_ERR_NOT_ATTACHED = 13,
    PDM_ERR_NOT_MOUNTED = 14,
    PDM_ERR_NOT_ACCESSIBLE = 15,  // partition not mounted or its media changed
    PDM_ERR_DISK_ATTACHED = 16,
    PDM_ERR_DISK_MOUNTED = 17,
    PDM_ERR_OUT_OF_RANGE = 18,
    PDM_ERR_DRIVER_ERROR = 21,  // driver code in pdm_part_get_driver_error_code
};

// PDM_DISK.status
enum {
    PDM_DISK_STAT_OPENED = 0x1,
    PDM_DISK_STAT_WRITE_PROTECTED = 0x2,
    PDM_DISK_STAT_INSERTED = 0x4,
};

// PDM_PARTITION.status
enum {
    PDM_PART_STAT_OPENED = 0x1,
    PDM_PART_STAT_MEDIA_CHANGED = 0x2,
    PDM_PART_STAT_ATTACHED = 0x4,
    PDM_PART_STAT_MOUNTED = 0x8,
};

// PDM_DISK_INFO.media_attr
enum {
    PDM_MEDIA_ATTR_WRITE_PROTECTED = 0x1,
    PDM_MEDIA_ATTR_FORMAT_AFTER_MOUNT = 0x2,
    PDM_MEDIA_ATTR_REMOVABLE = 0x4,
    PDM_MEDIA_ATTR_NO_LOGICAL_FORMAT = 0x8,
};

enum {
    PDM_MEDIA_EJECTED = 0,
    PDM_MEDIA_INSERTED = 1,
};

enum {
    PDM_UNMOUNT_NORMAL = 0,
    PDM_UNMOUNT_FORCE = 1,
};

// little-endian field access; the aligned path indexes element (off / 2 + off % 2)
#define PDM_LE16(buf, off)                                                                \
    (((u32)&(buf)[off] & 1) ? (u16)((buf)[off] | ((buf)[(off) + 1] << 8))                 \
                            : (u16)(((((u16*)(buf))[(off) / 2 + (off) % 2] >> 8) & 0xFF) | \
                                    ((((u16*)(buf))[(off) / 2 + (off) % 2] << 8) & 0xFF00)))
#define PDM_LE32(p) ((*(u32*)(p) >> 24) | ((*(u32*)(p) >> 8) & 0xFF00) | ((*(u32*)(p) << 8) & 0xFF0000) | (*(u32*)(p) << 24))

typedef struct PDM_DISK PDM_DISK;
typedef struct PDM_PARTITION PDM_PARTITION;

typedef struct PDM_DISK_INFO {
    u32 total_sectors;      // 0x00
    u16 cylinders;          // 0x04
    u8 heads;               // 0x06
    u8 sectors_per_track;   // 0x07
    u16 bytes_per_sector;   // 0x08
    u32 media_attr;         // 0x0C
    void* format_param;     // 0x10
} PDM_DISK_INFO;

typedef struct PDM_FUNCTBL {
    s32 (*init)(PDM_DISK*);                                   // 0x00
    s32 (*finalize)(PDM_DISK*);                               // 0x04
    s32 (*mount)(PDM_DISK*);                                  // 0x08
    s32 (*unmount)(PDM_DISK*);                                // 0x0C
    s32 (*format)(PDM_DISK*, const u8*);                      // 0x10
    s32 (*physical_read)(PDM_DISK*, u8*, u32, u32, u32*);     // 0x14
    s32 (*physical_write)(PDM_DISK*, const u8*, u32, u32, u32*);  // 0x18
    s32 (*get_disk_info)(PDM_DISK*, PDM_DISK_INFO*);          // 0x1C
} PDM_FUNCTBL;

typedef struct PDM_DISK_TBL {
    PDM_FUNCTBL* p_func;  // 0x00
    u32 ui_ext;           // 0x04
} PDM_DISK_TBL;

typedef struct PDM_INIT_DISK {
    s32 (*p_func)(PDM_DISK_TBL*, u32);  // 0x00
    u32 ui_ext;                         // 0x04
} PDM_INIT_DISK;

typedef struct PDM_PART_FUNC {
    void (*insert)(PDM_PARTITION*);  // 0x00
    void (*eject)(PDM_PARTITION*);   // 0x04
} PDM_PART_FUNC;

struct PDM_PARTITION {
    u32 status;              // 0x00
    PDM_DISK* p_disk;        // 0x04
    u32 part_id;             // 0x08
    u32 start_sector;        // 0x0C
    u32 total_sector;        // 0x10
    s32 driver_last_error;   // 0x14
    s32 part_sem_id;         // 0x18
    void* p_vol;             // 0x1C
    PDM_PART_FUNC func;      // 0x20
};

struct PDM_DISK {
    u32 status;               // 0x00
    PDM_DISK_TBL disk_tbl;    // 0x04
    s16 attach_part_cnt;      // 0x0C
    s16 mount_part_cnt;       // 0x0E
    s32 disk_sem_id;          // 0x10
    s32 drv_sem_id;           // 0x14
    PDM_DISK_INFO disk_info;  // 0x18
    PDM_PARTITION* p_cur_part;  // 0x2C
};

typedef struct PDM_DISK_SET {
    u16 num_partition;                         // 0x000
    u16 num_allocated_disk;                    // 0x002
    PDM_DISK disk[PDM_DRIVE_COUNT];            // 0x004
    PDM_PARTITION partition[PDM_DRIVE_COUNT];  // 0x4E4
} PDM_DISK_SET;

extern PDM_DISK_SET pdm_disk_set;

s32 pdm_init_diskmanager(u32 config, void* param);
s32 pdm_open_disk(PDM_INIT_DISK* p_init_disk_tbl, PDM_DISK** pp_disk);
s32 pdm_close_disk(PDM_DISK* p_disk);
s32 pdm_open_partition(PDM_DISK* p_disk, s32 part_id, PDM_PARTITION** pp_part);
s32 pdm_close_partition(PDM_PARTITION* p_part);

s32 pdm_disk_init_disk_manager(u32 config, void* param);
s32 pdm_disk_add_disk(PDM_INIT_DISK* p_init_disk_tbl, PDM_DISK** pp_disk);
s32 pdm_disk_del_disk(PDM_DISK* p_disk);
s32 pdm_part_get_partition(PDM_DISK* p_disk, s32 part_id, PDM_PARTITION** pp_part);
s32 pdm_part_release_partition(PDM_PARTITION* p_part);
BOOL pdm_part_is_attached_partition(PDM_PARTITION* p_part);

#ifdef __cplusplus
}
#endif
#endif
