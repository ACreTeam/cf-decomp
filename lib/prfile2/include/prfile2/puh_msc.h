#ifndef PRFILE2_PUH_MSC_H
#define PRFILE2_PUH_MSC_H

// USB mass storage class glue of the standalone PrFILE2 (lib/prfile2/msc).

#include <prfile2/pdm_api.h>
#include <revolution/sdi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UHF_MSC_DEVICE_COUNT 8

#define UHF_MSC_CMD_BLK_INIT 0x01010005
#define UHF_MSC_CMD_BLK_FINALIZE 0x01010006
#define UHF_MSC_CMD_READ10 0x01010A00
#define UHF_MSC_CMD_WRITE10 0x01010B00

#define UHF_MSC_STATE_STARTED 1

#define UHF_MSC_MSG_ALIGN 4
#define UHF_MSC_SENSE_LEN 3  // sense key, ASC, ASCQ
#define UHF_MSC_BOUNCE_BUF_SIZE 0x200
#define UHF_MSC_MAX_TRANSFER_SIZE 0x10000

// UHF_MSC_DEVICE.flags
enum {
    UHF_MSC_DEV_FLAG_MEDIA_PRESENT = 0x4,
    UHF_MSC_DEV_FLAG_WRITE_PROTECTED = 0x1000,
};

// error codes
enum {
    UHF_MSC_ERR = -1,
    UHF_MSC_ERR_NOT_STARTED = -3,
    UHF_MSC_ERR_SYSTEM = -12,           // message or semaphore failure
    UHF_MSC_ERR_CHECK_CONDITION = -15,  // the command failed and sense data was returned
};

// sense key, additional sense code and qualifier as checked by __uhf_msc_cmd_check_sense
#define UHF_MSC_SENSE_CODE(key, asc, ascq) (((key) << 16) | ((asc) << 8) | (ascq))

// SCSI sense keys
enum {
    UHF_MSC_SENSE_KEY_NO_SENSE = 0x0,
    UHF_MSC_SENSE_KEY_RECOVERED_ERROR = 0x1,
    UHF_MSC_SENSE_KEY_NOT_READY = 0x2,
    UHF_MSC_SENSE_KEY_MEDIUM_ERROR = 0x3,
    UHF_MSC_SENSE_KEY_HARDWARE_ERROR = 0x4,
    UHF_MSC_SENSE_KEY_ILLEGAL_REQUEST = 0x5,
    UHF_MSC_SENSE_KEY_UNIT_ATTENTION = 0x6,
    UHF_MSC_SENSE_KEY_DATA_PROTECT = 0x7,
    UHF_MSC_SENSE_KEY_ABORTED_COMMAND = 0xB,
};

#define UHF_MSC_ASC_DIAGNOSTIC_FAILURE 0x40  // ASCQ is the failing component

// __uhf_msc_cmd_check_sense results
enum {
    UHF_MSC_SENSE_NO_SENSE = 0,
    UHF_MSC_SENSE_RECOVERED_WITH_RETRIES = 1,
    UHF_MSC_SENSE_RECOVERED_WITH_ECC = 2,
    UHF_MSC_SENSE_BECOMING_READY = 3,
    UHF_MSC_SENSE_INIT_REQUIRED = 4,
    UHF_MSC_SENSE_FORMAT_IN_PROGRESS = 5,
    UHF_MSC_SENSE_NOT_READY_BUSY = 6,
    UHF_MSC_SENSE_NO_REFERENCE_POSITION = 7,
    UHF_MSC_SENSE_COMM_FAILURE = 8,
    UHF_MSC_SENSE_COMM_TIMEOUT = 9,
    UHF_MSC_SENSE_COMM_OVERRUN = 10,
    UHF_MSC_SENSE_MEDIUM_NOT_PRESENT = 11,
    UHF_MSC_SENSE_INTERFACE_FAILURE = 12,
    UHF_MSC_SENSE_NOT_READY_VENDOR = 13,
    UHF_MSC_SENSE_NOT_READY_UNKNOWN = 14,
    UHF_MSC_SENSE_NO_SEEK_COMPLETE = 15,
    UHF_MSC_SENSE_WRITE_FAULT = 16,
    UHF_MSC_SENSE_ID_CRC_ERROR = 17,
    UHF_MSC_SENSE_UNRECOVERED_READ_ERROR = 18,
    UHF_MSC_SENSE_NO_ID_ADDRESS_MARK = 19,
    UHF_MSC_SENSE_NO_DATA_ADDRESS_MARK = 20,
    UHF_MSC_SENSE_RECORD_NOT_FOUND = 21,
    UHF_MSC_SENSE_UNKNOWN_FORMAT = 22,
    UHF_MSC_SENSE_FORMAT_FAILED = 23,
    UHF_MSC_SENSE_DIAGNOSTIC_FAILURE = 24,
    UHF_MSC_SENSE_PARAM_LIST_LENGTH = 25,
    UHF_MSC_SENSE_INVALID_OPCODE = 26,
    UHF_MSC_SENSE_LBA_OUT_OF_RANGE = 27,
    UHF_MSC_SENSE_INVALID_FIELD_IN_CDB = 28,
    UHF_MSC_SENSE_LUN_NOT_SUPPORTED = 29,
    UHF_MSC_SENSE_INVALID_FIELD_IN_PARAM = 30,
    UHF_MSC_SENSE_PARAM_NOT_SUPPORTED = 31,
    UHF_MSC_SENSE_INVALID_PARAM_VALUE = 32,
    UHF_MSC_SENSE_SAVING_NOT_SUPPORTED = 33,
    UHF_MSC_SENSE_MEDIUM_CHANGED = 34,
    UHF_MSC_SENSE_RESET_OCCURRED = 35,
    UHF_MSC_SENSE_COMMANDS_CLEARED = 36,
    UHF_MSC_SENSE_WRITE_PROTECTED = 37,
    UHF_MSC_SENSE_OVERLAPPED_COMMANDS = 38,
    UHF_MSC_SENSE_UNKNOWN = 39,
};

typedef struct UHF_MSC_DEVICE {
    PDM_DISK* p_disk;  // 0x00
    u8 _04[0x10];      // 0x04
    u32 num_blocks;    // 0x14
    u32 block_size;    // 0x18
    u8 _1C[0x8];       // 0x1C
    u32 flags;         // 0x24
    u8 _28[0x8];       // 0x28
} UHF_MSC_DEVICE;

typedef struct UHF_MSC_REQ {
    UHF_MSC_DEVICE* p_dev;  // 0x00
    u8* p_sense;            // 0x04
    u32 _08;                // 0x08
    u8* p_buf;              // 0x0C
    u32 _10;                // 0x10
    u32 lba;                // 0x14
    u16 _18;                // 0x18
    u16 num_blocks;         // 0x1A
    u32 _1C;                // 0x1C
    u32 _20;                // 0x20
    PDM_DISK* p_disk;       // 0x24
} UHF_MSC_REQ;

typedef struct UHF_MSC_MSG {
    u32 cmd;          // 0x00
    s32 sem_id;       // 0x04
    s32* p_result;    // 0x08
    UHF_MSC_REQ req;  // 0x0C
    u8 _34[0x8];      // 0x34
} UHF_MSC_MSG;

extern UHF_MSC_DEVICE uhg_msc_blk_device_tbl[UHF_MSC_DEVICE_COUNT];
extern void* uhg_msc_mem_pool;

s32 usbh_msc_read10(UHF_MSC_DEVICE* p_dev, u32 lba, u16 num_blocks, u8* buf, u8* sense);
s32 usbh_msc_write10(UHF_MSC_DEVICE* p_dev, u32 lba, u16 num_blocks, u8* buf, u8* sense);
s32 _uhf_msc_api_send_message(UHF_MSC_REQ* p_req, u32 cmd);
s32 uhf_msc_get_message_id(void);

s32 __uhf_msc_cmd_check_sense(u8* sense);

s32 usbh_msc_blk_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext);
s32 uhf_msc_blk_init(PDM_DISK* p_disk);
s32 uhf_msc_blk_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info);
s32 uhf_msc_blk_mount(PDM_DISK* p_disk);
s32 uhf_msc_blk_pread(PDM_DISK* p_disk, u8* buf, u32 block, u32 num_block, u32* p_num_success);
s32 uhf_msc_blk_pwrite(PDM_DISK* p_disk, const u8* buf, u32 block, u32 num_block, u32* p_num_success);
s32 uhf_msc_blk_format(PDM_DISK* p_disk, const u8* param);
s32 uhf_msc_blk_finalize(PDM_DISK* p_disk);
s32 uhf_msc_blk_unmount(PDM_DISK* p_disk);
BOOL uhf_msc_is_media_insert(PDM_DISK* p_disk);
s32 _uhf_msc_blk_send_message(UHF_MSC_REQ* p_req, u32 cmd);

#ifdef __cplusplus
}
#endif
#endif
