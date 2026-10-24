#ifndef PRFILE2_PF_TYPES_H
#define PRFILE2_PF_TYPES_H

// Structures of the standalone PrFILE2. Based on the RevoEX vf copy (revolution/vf/vf_struct.h); this version has a
// semaphore in each PF_SFD, four contexts and four current directories per volume.

#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PF_DRIVE_COUNT 26
#define PF_CONTEXT_COUNT 4
#define PF_SFD_COUNT 5
#define PF_UFD_COUNT 5
#define PF_SDD_COUNT 3
#define PF_UDD_COUNT 3

#define PF_PATH_LEN_MAX 259
#define PF_LONG_NAME_LEN_MAX 255
#define PF_DIR_ENT_LONG_NAME_LEN 261
#define PF_ENTRY_INDEX_END 999999  // directory entry position past the last entry / not found
#define PF_INVALID_SECTOR 0xFFFFFFFF
#define PF_INVALID_CLUSTER 0xFFFFFFFF
#define PF_FILE_SIZE_MAX 0xFFFFFFFF
#define PDM_BPB_VOL_LABEL_LEN 11
#define PDM_BPB_VOL_LABEL_NO_NAME "NO NAME    "

#define PF_DRIVE_LETTER_LEN 1                                       // "A", the ':' follows
#define PF_DRIVE_SPEC_LEN 2                                         // "A:"
#define PF_LONG_NAME_OEM_BUF_LEN ((PF_LONG_NAME_LEN_MAX + 1) * 2)  // OEM long name and NUL, up to two bytes per character

// API error numbers (pf_errnum); same values as FA_ERR_* in include/lib/revolution/FA/fa_types.h
enum {
    PF_ERR_SYSTEM = -1,
    PF_ERR_EPERM = 1,
    PF_ERR_ENOENT = 2,
    PF_ERR_EIO = 5,
    PF_ERR_ENOEXEC = 8,
    PF_ERR_EBADF = 9,
    PF_ERR_ENOMEM = 12,
    PF_ERR_EACCES = 13,
    PF_ERR_EBUSY = 16,
    PF_ERR_EEXIST = 17,
    PF_ERR_EISDIR = 21,
    PF_ERR_EINVAL = 22,
    PF_ERR_ENFILE = 23,
    PF_ERR_EMFILE = 24,
    PF_ERR_EFBIG = 27,
    PF_ERR_ENOSPC = 28,
    PF_ERR_ENOLCK = 46,
    PF_ERR_ENOSYS = 88,
    PF_ERR_ENOTEMPTY = 90,
    PF_ERR_EWRTPROTECT = 201,
};

// API return values
enum {
    PF_RET_NO_ERR = 0,
    PF_RET_ERR = -1,
    PF_RET_UNMOUNT_FORCED = 1,  // forced unmount completed despite open handles or an error
    PF_RET_EOF = 1,             // pf2_feof: at end of file
};

// Internal error codes (last_error); PFAPI_convertError maps them to PF_ERR_* via pf_error_to_api_error
enum {
    PF_IERR_SYSTEM = -1,
    PF_IERR_INVALID_NAME = 1,
    PF_IERR_INVALID_PATH = 2,
    PF_IERR_NOT_FOUND = 3,
    PF_IERR_NO_FREE_DRIVE = 4,
    PF_IERR_DIR_FULL = 5,  // no free directory entry; maps to PF_ERR_ENOTEMPTY
    PF_IERR_NO_SPACE = 6,
    PF_IERR_INVALID_FORMAT = 7,
    PF_IERR_EXISTS = 8,
    PF_IERR_NOT_MOUNTED = 9,
    PF_IERR_INVALID_PARAM = 10,
    PF_IERR_WRITE_PROTECTED = 11,
    PF_IERR_NOT_SUPPORTED = 12,
    PF_IERR_BROKEN_CHAIN = 13,
    PF_IERR_INVALID_CLUSTER = 14,
    PF_IERR_INVALID_BPB = 15,
    PF_IERR_OUT_OF_RANGE = 16,
    PF_IERR_IO = 17,
    PF_IERR_IN_USE = 19,
    PF_IERR_NOT_DIR = 20,
    PF_IERR_TOO_MANY_FILES = 21,    // no free SFD/SDD
    PF_IERR_TOO_MANY_HANDLES = 22,  // no free UFD/UDD
    PF_IERR_IS_DIR = 23,
    PF_IERR_READ_ONLY = 24,
    PF_IERR_LOCKED = 25,
    PF_IERR_INTERNAL = 26,           // context registry failures, oversized tail-sector write
    PF_IERR_END_OF_FILE = 27,
    PF_IERR_INVALID_OPERATION = 28,  // removing the current/root/dot directory, cursor past the cluster chain
    PF_IERR_NOT_EMPTY = 29,
    PF_IERR_NO_MEMORY = 30,
    PF_IERR_DIFFERENT_DRIVE = 31,
    PF_IERR_DIFFERENT_DIR = 32,
    PF_IERR_BROKEN_LFN = 33,
    PF_IERR_VOLUME_MOUNTED = 36,
    PF_IERR_FILE_TOO_BIG = 37,
    PF_IERR_BAD_FILE = 38,
    PF_IERR_LOCK_FAILED = 39,
    PF_IERR_DRIVER = 0x1000,  // details in last_driver_error
};

// FAT directory entry attributes
enum {
    PF_ATTR_NONE = 0x00,  // search argument: no unwanted attribute
    PF_ATTR_READ_ONLY = 0x01,
    PF_ATTR_HIDDEN = 0x02,
    PF_ATTR_SYSTEM = 0x04,
    PF_ATTR_VOLUME = 0x08,
    PF_ATTR_DIR = 0x10,
    PF_ATTR_ARCHIVE = 0x20,
    PF_ATTR_NORMAL = 0x40,  // search/chmod argument: no attribute set
    PF_ATTR_LONG_NAME = 0x0F,
    PF_ATTR_ALL = 0x7F,
    PF_ATTR_STRICT = 0x80,  // search argument: every required bit and not every unwanted bit
};

// first byte of a FAT directory entry
enum {
    PF_ENTRY_END = 0x00,
    PF_ENTRY_E5_ESCAPE = 0x05,  // stored for a name starting with 0xE5
    PF_ENTRY_DELETED = 0xE5,
};

enum {
    PF_DIR_ENTRY_SIZE = 32,
    PF_DIR_ENTRY_SIZE_LOG2 = 5,
    PF_LFN_CHARS_PER_ENTRY = 13,  // UTF-16 name characters in one LFN entry
};

// byte offsets in an on-disk short name directory entry
enum {
    PF_DIRENT_ATTR = 11,
    PF_DIRENT_SMALL_LETTER_FLAG = 12,
    PF_DIRENT_CREATE_TIME_MS = 13,
    PF_DIRENT_CREATE_TIME = 14,
    PF_DIRENT_CREATE_DATE = 16,
    PF_DIRENT_ACCESS_DATE = 18,
    PF_DIRENT_CLUSTER_HI = 20,
    PF_DIRENT_MODIFY_TIME = 22,
    PF_DIRENT_MODIFY_DATE = 24,
    PF_DIRENT_CLUSTER_LO = 26,
    PF_DIRENT_FILE_SIZE = 28,
};

// on-disk LFN directory entry: the check sum and the three pieces of its 13 UTF-16 name characters
enum {
    PF_LFNENT_NAME1 = 1,
    PF_LFNENT_NAME1_SIZE = 10,
    PF_LFNENT_CHECK_SUM = 13,
    PF_LFNENT_NAME2 = 14,
    PF_LFNENT_NAME2_SIZE = 12,
    PF_LFNENT_NAME3 = 28,
    PF_LFNENT_NAME3_SIZE = 4,
    PF_LFNENT_NAME_SIZE = PF_LFNENT_NAME1_SIZE + PF_LFNENT_NAME2_SIZE + PF_LFNENT_NAME3_SIZE,
    PF_LFN_FIRST_ORDINAL = 1,  // LFN entry right before the short name entry, holding the first characters
    PF_LFN_MAX_ORDINAL = 20,
    PF_LFN_LAST_ENTRY = 0x40,  // LFN ordinal flag
    PF_LFN_PAD_CHAR = 0xFFFF,  // fills the name characters after the NUL in the last LFN entry
};

// packed FAT date (PF_DIRENT_*_DATE) and time (PF_DIRENT_*_TIME) fields
enum {
    PF_FAT_DATE_DAY_MASK = 0x1F,
    PF_FAT_DATE_MONTH_SHIFT = 5,
    PF_FAT_DATE_MONTH_MASK = 0x0F,
    PF_FAT_DATE_YEAR_SHIFT = 9,
    PF_FAT_DATE_YEAR_MASK = 0x7F,
    PF_FAT_DATE_YEAR_BASE = 1980,
    PF_FAT_TIME_SEC2_MASK = 0x1F,  // seconds / 2
    PF_FAT_TIME_MIN_SHIFT = 5,
    PF_FAT_TIME_MIN_MASK = 0x3F,
    PF_FAT_TIME_HOUR_SHIFT = 11,
    PF_FAT_TIME_HOUR_MASK = 0x1F,
};

// PF_DIR_ENT.small_letter_flag: lower-case parts of a short name without LFN entries
enum {
    PF_SMALL_LETTER_BASE = 0x08,
    PF_SMALL_LETTER_EXT = 0x10,
    PF_SMALL_LETTER_MASK = PF_SMALL_LETTER_BASE | PF_SMALL_LETTER_EXT,
};

// short (8.3) file name lengths, without the dot
enum {
    PF_SFN_BASE_LEN = 8,
    PF_SFN_EXT_LEN = 3,
    PF_SFN_LEN = PF_SFN_BASE_LEN + PF_SFN_EXT_LEN,
    PF_SFN_BUF_LEN = PF_SFN_LEN + 2,  // "BASENAME.EXT" and NUL
};

// cluster numbers with a special meaning
enum {
    PF_CLUSTER_FREE = 0,         // FAT entry of a free cluster, start_cluster of an empty file
    PF_CLUSTER_DOTDOT_ROOT = 0,  // start_cluster of a ".." entry whose parent is the root directory
    PF_CLUSTER_ROOT_DIR = 1,     // start_cluster standing for the FAT12/16 root directory region
    PF_CLUSTER_FIRST = 2,        // first data cluster
};

// PF_VOLUME.flags
enum {
    PF_VOL_FLAG_ATTACHED = 0x01,
    PF_VOL_FLAG_WRITE_PROTECTED = 0x02,
    PF_VOL_FLAG_INSERTED = 0x04,
    PF_VOL_FLAG_MOUNTED = 0x08,
    PF_VOL_FLAG_MOUNT_REQUESTED = 0x10,
    PF_VOL_FLAG_UNMOUNT_REQUESTED = 0x20,
    PF_VOL_FLAG_FORMAT_AFTER_MOUNT = 0x40,
    PF_VOL_FLAG_NO_LOGICAL_FORMAT = 0x80,
};

// PF_VOLUME.fsi_flag
enum {
    PF_FSI_FLAG_USE = 0x1,
    PF_FSI_FLAG_UPDATE = 0x2,            // rewrite the FSINFO sector when the free count changes
    PF_FSI_FLAG_FREE_COUNT_VALID = 0x4,  // num_free_clusters can be trusted
};

// stat of the SFD/SDD/UFD/UDD, context and current directory/volume slots
enum {
    PF_STAT_USED = 0x1,
    PF_STAT_VALID = 0x2,     // SFD/SDD
    PF_STAT_MODIFIED = 0x4,  // SFD: the directory entry has to be rewritten on close
};

enum {
    PF_OPEN_MODE_WRITE = 0x01,
    PF_OPEN_MODE_READ = 0x02,
    PF_OPEN_MODE_APPEND = 0x04,
    PF_OPEN_MODE_PLUS = 0x08,
    PF_OPEN_MODE_NEW = 0x10,  // fail if the file exists
};

enum {
    PF_LOCK_SHARED = 0x1,
    PF_LOCK_EXCLUSIVE = 0x2,
    PF_LOCK_NO_WAIT = 0x8,
};

enum {
    PF_SEEK_SET = 0,
    PF_SEEK_CUR = 1,
    PF_SEEK_END = 2,
};

enum {
    PF_CACHE_PAGE_SIZE = 512,  // bytes in one cache buffer
    PF_CACHE_PAGE_SIZE_LOG2 = 9,
};

// PF_SECTOR_CACHE.mode, also the pf_buffering mode bits
enum {
    PF_CACHE_MODE_WRITE_THROUGH = 0x1,
    PF_CACHE_MODE_FLUSH_ON_CLOSE = 0x2,
    PF_CACHE_MODE_SYNC_UPDATES = 0x4,  // FAT pages written through, data cache flushed after each update
};

// PF_VOLUME_SET.setting (pf_setencode)
enum {
    PF_ENCODE_MODE_OEM = 0x1,
    PF_ENCODE_MODE_UNICODE = 0x2,
    PF_ENCODE_MODE_MASK = 0x3,
};

// PF_VOLUME_SET.config, PF_VOL_CFG.volume_config
enum {
    PF_CONFIG_NO_CHAR_CHECK = 0x10000,  // accept any character in file names
    PF_CONFIG_CHAR_CHECK = 0x20000,     // PF_VOL_CFG only: clear PF_CONFIG_NO_CHAR_CHECK
};

// PF_VOL_CFG.volume_config of a single volume
enum {
    PF_VOL_CFG_SET_FAT_BUFF_SIZE = 0x1,
    PF_VOL_CFG_SET_DATA_BUFF_SIZE = 0x2,
};

// PF_VOLUME.file_config, PF_VOL_CFG.file_config
enum {
    PF_FILE_CONFIG_UPDATE_IF_MODIFIED = 0x1,  // fclose rewrites the directory entry only after a change
    PF_FILE_CONFIG_UPDATE_ALWAYS = 0x2,       // PF_VOL_CFG only: clear PF_FILE_CONFIG_UPDATE_IF_MODIFIED
};

// unmount mode (PFVOL_unmount, PFDRV_unmount)
enum {
    PF_UNMOUNT_NORMAL = 0,
    PF_UNMOUNT_FORCE = 0x1,  // unmount despite open handles or a failed cache flush
};

// second argument of PF_CHARCODE.is_oem_mb_char / is_unicode_mb_char
enum {
    PF_MB_LEAD_BYTE = 1,
    PF_MB_TRAIL_BYTE = 2,
};

typedef struct PF_VOLUME PF_VOLUME;
typedef struct PF_SFD PF_SFD;
struct PDM_PARTITION;

typedef enum PF_FAT_TYPE {
    FAT_12 = 0,
    FAT_16 = 1,
    FAT_32 = 2,
    FAT_ERR = -1,
} PF_FAT_TYPE;

typedef struct PF_STR {
    const s8* p_head;   // 0x00
    const s8* p_tail;   // 0x04
    const s8* p_local;  // 0x08
    u32 code_mode;      // 0x0C
} PF_STR;

typedef struct PF_CURSOR {
    u32 position;           // 0x00
    u32 sector;             // 0x04
    u32 file_sector_index;  // 0x08
    u16 offset_in_sector;   // 0x0C
} PF_CURSOR;

typedef struct PF_BPB {
    u16 bytes_per_sector;          // 0x00
    u16 num_reserved_sectors;      // 0x02
    u16 num_root_dir_entries;      // 0x04
    u8 sectors_per_cluster;        // 0x06
    u8 num_FATs;                   // 0x07
    u32 total_sectors;             // 0x08
    u32 sectors_per_FAT;           // 0x0C
    u32 root_dir_cluster;          // 0x10
    u16 fs_info_sector;            // 0x14
    u16 backup_boot_sector;        // 0x16
    u16 ext_flags;                 // 0x18
    u8 media;                      // 0x1A
    PF_FAT_TYPE fat_type;          // 0x1C
    u8 log2_bytes_per_sector;      // 0x20
    u8 log2_sectors_per_cluster;   // 0x21
    u8 num_active_FATs;            // 0x22
    u16 num_root_dir_sectors;      // 0x24
    u32 active_FAT_sector;         // 0x28
    u32 first_root_dir_sector;     // 0x2C
    u32 first_data_sector;         // 0x30
    u32 num_clusters;              // 0x34
} PF_BPB;

typedef struct PF_LAST_CLUSTER {
    u32 num_last_cluster;  // 0x00
    u32 max_chain_index;   // 0x04
} PF_LAST_CLUSTER;

typedef struct PF_FAT_LAST_ACCESS {
    u32 chain_index;  // 0x00
    u32 cluster;      // 0x04
} PF_FAT_LAST_ACCESS;

typedef struct PF_DIR_ENT {
    u16 long_name[PF_DIR_ENT_LONG_NAME_LEN];  // 0x000
    u8 num_entry_LFNs;                        // 0x20A
    u8 ordinal;                               // 0x20B
    u8 check_sum;                             // 0x20C
    u8 align_pad[1];                          // 0x20D
    s8 short_name[PF_SFN_BUF_LEN];            // 0x20E
    u8 small_letter_flag;                     // 0x21B
    u8 attr;                                  // 0x21C
    u8 create_time_ms;                        // 0x21D
    u16 create_time;                          // 0x21E
    u16 create_date;                          // 0x220
    u16 access_date;                          // 0x222
    u16 modify_time;                          // 0x224
    u16 modify_date;                          // 0x226
    u32 file_size;                            // 0x228
    PF_VOLUME* p_vol;                         // 0x22C
    u32 path_len;                             // 0x230
    u32 start_cluster;                        // 0x234
    u32 entry_sector;                         // 0x238
    u16 entry_offset;                         // 0x23C
} PF_DIR_ENT;

typedef struct PF_FAT_HINT {
    u32 chain_index;   // 0x00
    u32 cluster;       // 0x04
    u32 file_version;  // 0x08
} PF_FAT_HINT;

typedef struct PF_CLUSTER_LINK {
    u32* buffer;          // 0x00
    u16 interval;         // 0x04
    u16 interval_offset;  // 0x06
    u32 position;         // 0x08
    u32 max_count;        // 0x0C
    u32 save_index;       // 0x10
} PF_CLUSTER_LINK;

typedef struct PF_FFD {
    u32 file_version;                        // 0x00
    u32 start_cluster;                       // 0x04
    u32* p_start_cluster;                    // 0x08
    PF_LAST_CLUSTER last_cluster;            // 0x0C
    PF_FAT_LAST_ACCESS last_access_cluster;  // 0x14
    PF_CLUSTER_LINK cluster_link;            // 0x1C
    PF_FAT_HINT* p_hint;                     // 0x30
    PF_VOLUME* p_vol;                        // 0x34
} PF_FFD;

typedef struct PF_SDD {
    u32 stat;              // 0x00
    u16 num_handlers;      // 0x04
    PF_FFD ffd;            // 0x08
    PF_DIR_ENT dir_entry;  // 0x40
} PF_SDD;

typedef struct PF_DIR_CURSOR {
    u32 physical_entry_index;  // 0x00
    u32 logical_entry_index;   // 0x04
    u32 logical_seek_index;    // 0x08
} PF_DIR_CURSOR;

typedef struct PF_DIR {
    u32 stat;              // 0x00
    PF_SDD* p_sdd;         // 0x04
    PF_FAT_HINT hint;      // 0x08
    PF_DIR_CURSOR cursor;  // 0x14
} PF_DIR;

typedef struct PF_CACHE_PAGE {
    u16 stat;                      // 0x00
    u16 option;                    // 0x02
    u8* buffer;                    // 0x04
    u8* p_buf;                     // 0x08
    u8* p_mod_sbuf;                // 0x0C
    u8* p_mod_ebuf;                // 0x10
    u32 size;                      // 0x14
    u32 sector;                    // 0x18
    void* signature;               // 0x1C
    struct PF_CACHE_PAGE* p_next;  // 0x20
    struct PF_CACHE_PAGE* p_prev;  // 0x24
} PF_CACHE_PAGE;

typedef struct PF_SECTOR_CACHE {
    u32 mode;                           // 0x00
    u16 num_fat_pages;                  // 0x04
    u16 num_data_pages;                 // 0x06
    PF_CACHE_PAGE* pages;               // 0x08
    PF_CACHE_PAGE* p_current_fat;       // 0x0C
    PF_CACHE_PAGE* p_current_data;      // 0x10
    u8 (*buffers)[PF_CACHE_PAGE_SIZE];  // 0x14
    u32 fat_buff_size;                  // 0x18
    u32 data_buff_size;                 // 0x1C
    void* signature;                    // 0x20
} PF_SECTOR_CACHE;

typedef struct PF_CUR_DIR {
    u32 stat;              // 0x00
    s32 context_id;        // 0x04
    PF_DIR_ENT directory;  // 0x08
} PF_CUR_DIR;

typedef struct PF_DIR_TAIL {
    u32 tracker_size;     // 0x00
    u32 tracker_buff[1];  // 0x04
    u32* tracker_bits;    // 0x08
} PF_DIR_TAIL;

typedef struct PF_CLUSTER_LINK_VOL {
    u16 flag;      // 0x00
    u16 interval;  // 0x02
    u32* buffer;   // 0x04
    u32 link_max;  // 0x08
} PF_CLUSTER_LINK_VOL;

typedef struct PF_FILE {
    u32 stat;          // 0x00
    s32 open_mode;     // 0x04
    PF_SFD* p_sfd;     // 0x08
    PF_FAT_HINT hint;  // 0x0C
    s32 last_error;    // 0x18
    PF_CURSOR cursor;  // 0x1C
    u16 lock_count;    // 0x2C
} PF_FILE;

typedef struct PF_INFO {
    u32 file_size;        // 0x00
    u32 io_pointer;       // 0x04
    u32 empty_size;       // 0x08
    u32 allocated_size;   // 0x0C
    u32 lock_mode;        // 0x10
    PF_FILE* lock_owner;  // 0x14
    u32 lock_count;       // 0x18
    u32 lock_tcount;      // 0x1C
} PF_INFO;

typedef struct PF_CLUSTER_LINK_RES {
    u32* buffer;     // 0x00
    u32 max_count;   // 0x04
    u16 interval;    // 0x08
} PF_CLUSTER_LINK_RES;

typedef struct PF_LOCK {
    u16 mode;        // 0x00
    u16 count;       // 0x02
    u32 wcount;      // 0x04
    PF_FILE* owner;  // 0x08
    s32 resource;    // 0x0C
} PF_LOCK;

struct PF_SFD {
    u32 stat;              // 0x000
    PF_FFD ffd;            // 0x004
    PF_DIR_ENT dir_entry;  // 0x03C
    PF_LOCK lock;          // 0x27C
    OSSemaphore lock_sem;  // 0x28C
    u16 num_handlers;      // 0x298
};

struct PF_VOLUME {
    PF_BPB bpb;                                // 0x0000
    u32 num_free_clusters;                     // 0x0038
    u32 last_free_cluster;                     // 0x003C
    PF_SFD sfds[PF_SFD_COUNT];                 // 0x0040
    PF_FILE ufds[PF_UFD_COUNT];                // 0x0D4C
    PF_SDD sdds[PF_SDD_COUNT];                 // 0x0E3C
    PF_DIR udds[PF_UDD_COUNT];                 // 0x15BC
    s32 num_opened_files;                      // 0x161C
    s32 num_opened_directories;                // 0x1620
    PF_SECTOR_CACHE cache;                     // 0x1624
    s8 label[PDM_BPB_VOL_LABEL_LEN + 1];       // 0x1648
    PF_CUR_DIR current_dir[PF_CONTEXT_COUNT];  // 0x1654
    PF_DIR_TAIL tail_entry;                    // 0x1F74
    s32 last_error;                            // 0x1F80
    s32 last_driver_error;                     // 0x1F84
    u32 file_config;                           // 0x1F88
    u16 flags;                                 // 0x1F8C
    s8 drv_char;                               // 0x1F8E
    u16 fsi_flag;                              // 0x1F90
    PF_CLUSTER_LINK_VOL cluster_link;          // 0x1F94
    struct PDM_PARTITION* p_part;              // 0x1FA0
    s32 (*p_callback)(s32 driver_error);       // 0x1FA4
    const u8* format_param;                    // 0x1FA8
};

typedef struct PF_CUR_VOLUME {
    u32 stat;          // 0x00
    s32 context_id;    // 0x04
    PF_VOLUME* p_vol;  // 0x08
} PF_CUR_VOLUME;

typedef struct PF_CONTEXT {
    u32 stat;        // 0x00
    s32 context_id;  // 0x04
} PF_CONTEXT;

typedef struct PF_CHARCODE {
    s32 (*oem2unicode)(const s8*, u16*);          // 0x00
    s32 (*unicode2oem)(const u16*, s8*);          // 0x04
    s32 (*oem_char_width)(const s8*);             // 0x08
    BOOL (*is_oem_mb_char)(const s8, BOOL);       // 0x0C
    s32 (*unicode_char_width)(const u16*);        // 0x10
    BOOL (*is_unicode_mb_char)(const u16, BOOL);  // 0x14
} PF_CHARCODE;

typedef struct PF_VOLUME_SET {
    PF_CUR_VOLUME current_vol[PF_CONTEXT_COUNT];  // 0x00000
    s32 num_attached_drives;                      // 0x00030
    s32 num_mounted_volumes;                      // 0x00034
    u32 config;                                   // 0x00038
    void* param;                                  // 0x0003C
    s32 last_error;                               // 0x00040
    s32 last_driver_error;                        // 0x00044
    PF_CHARCODE codeset;                          // 0x00048
    u32 setting;                                  // 0x00060
    s32 context_id;                               // 0x00064
    PF_CONTEXT context[PF_CONTEXT_COUNT];         // 0x00068
    PF_VOLUME volumes[PF_DRIVE_COUNT];            // 0x00088
} PF_VOLUME_SET;

extern PF_VOLUME_SET pf_vol_set;

typedef struct PF_CACHE_SETTING {
    PF_CACHE_PAGE* pages;               // 0x00
    u8 (*buffers)[PF_CACHE_PAGE_SIZE];  // 0x04
    u16 num_fat_pages;                  // 0x08
    u16 num_data_pages;                 // 0x0A
    u32 num_fat_buf_size;               // 0x0C
    u32 num_data_buf_size;              // 0x10
} PF_CACHE_SETTING;

typedef struct PF_DRV_TBL {
    struct PDM_PARTITION* p_part;  // 0x00
    PF_CACHE_SETTING* cache;       // 0x04
    s8 drive;                      // 0x08
    u8 stat;                       // 0x09
} PF_DRV_TBL;

enum {
    PF_DRV_STAT_ATTACHED = 0x01,
    PF_DRV_STAT_MOUNTED = 0x02,
    PF_DRV_STAT_INSERTED = 0x10,
};

typedef struct PF_DEV_INF {
    u32 cls;  // 0x00
    u32 ecl;  // 0x04
    u32 bps;  // 0x08
    u32 spc;  // 0x0C
} PF_DEV_INF;

typedef struct PF_VOLTAB {
    s8 v_name[PDM_BPB_VOL_LABEL_LEN + 1];  // 0x00
    u8 v_attr;                             // 0x0C
    u32 v_date;                            // 0x10
    u32 v_time;                            // 0x14
} PF_VOLTAB;

typedef struct PF_VOL_CFG {
    u32 volume_config;   // 0x00
    u32 file_config;     // 0x04
    u32 fat_buff_size;   // 0x08
    u32 data_buff_size;  // 0x0C
} PF_VOL_CFG;

typedef struct PF_DTA {
    PF_FILE* p_file;                            // 0x000
    PF_DIR* p_dir;                              // 0x004
    PF_VOLUME* p_vol;                           // 0x008
    u32 parent_start_cluster;                   // 0x00C
    u32 parent_pos;                             // 0x010
    u32 status;                                 // 0x014
    u8 num_entry_LFNs;                          // 0x018
    u8 ordinal;                                 // 0x019
    u8 check_sum;                               // 0x01A
    u8 attr;                                    // 0x01B
    s8 reg_exp[520];                            // 0x01C
    u16 Time;                                   // 0x224
    u16 Date;                                   // 0x226
    u32 FileSize;                               // 0x228
    u8 Attribute;                               // 0x22C
    s8 FileName[PF_SFN_BUF_LEN];                // 0x22D
    s8 LongName[PF_DIR_ENT_LONG_NAME_LEN * 2];  // 0x23A
    u16 reg_exp_w[260];                         // 0x444
    u16 FileName_w[PF_SFN_BUF_LEN];             // 0x64C
    u16 LongName_w[260];                        // 0x666
} PF_DTA;

typedef struct PF_STAT {
    u32 file_size;       // 0x00
    u16 access_date;     // 0x04
    u16 modify_time;     // 0x06
    u16 modify_date;     // 0x08
    u16 create_time;     // 0x0A
    u16 create_date;     // 0x0C
    u16 create_time_ms;  // 0x0E
    u8 attr;             // 0x10
} PF_STAT;

typedef struct PF_DIRENT {
    s8 long_name[PF_LONG_NAME_OEM_BUF_LEN];  // 0x000
    s8 short_name[PF_SFN_BUF_LEN];           // 0x200
} PF_DIRENT;

typedef struct PF_ENT_ITER {
    u32 index;                   // 0x00
    PF_VOLUME* p_vol;            // 0x04
    PF_FFD ffd;                  // 0x08
    u32 file_sector_index;       // 0x40
    u32 sector;                  // 0x44
    u16 offset;                  // 0x48
    u16 offset_mask;             // 0x4A
    u8 buf[PF_DIR_ENTRY_SIZE];   // 0x4C
    u8 log2_entries_per_sector;  // 0x6C
} PF_ENT_ITER;

typedef struct PF_FILE_NAME_ITER {
    const s8* buf;     // 0x00
    u32 dot_inserted;  // 0x04
    u32 is_long_name;  // 0x08
    u16 index;         // 0x0C
} PF_FILE_NAME_ITER;

#ifdef __cplusplus
}
#endif
#endif
