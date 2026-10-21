#pragma once

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// types from the DWARF of RVLDWC 2.1.3 (options/src/cfriend/dwc_cfriend.c)
typedef enum DWCCfError {
    DWC_CF_ERROR_NONE = 0,
    DWC_CF_ERROR_NOFRIENDDATA = 1,
    DWC_CF_ERROR_CRC = 2,
    DWC_CF_ERROR_PARAM = 3,
    DWC_CF_ERROR_DATA_BROKEN = 4,
    DWC_CF_ERROR_MSGOBJ_ALREADY_SET = 5,
    DWC_CF_ERROR_MSGOBJ_PROTECTED = 6,
    DWC_CF_ERROR_MSGOBJ_FULL = 7,
    DWC_CF_ERROR_NWC24_INTERNAL = 8,
    DWC_CF_ERROR_USERDATA = 9,
    DWC_CF_ERROR_MEMORY = 10,
    DWC_CF_ERROR_GETINFO = 11,
    DWC_CF_ERROR_MAX = 12
} DWCCfError;

typedef enum DWCCfMsgType {
    DWC_CF_MSG_TYPE_REQUEST = 0,
    DWC_CF_MSG_TYPE_RESPONSE_ACCEPT = 1,
    DWC_CF_MSG_TYPE_RESPONSE_DENY = 2,
    DWC_CF_MSG_TYPE_REQUEST_REMIND = 3
} DWCCfMsgType;

// the friend-key exchange message sent through WiiConnect24
typedef struct DWCiCfMsgFormat {
    u32 header;
    u32 version;
    u32 size;
    DWCCfMsgType msgType;
    u64 appFriendKey;
} DWCiCfMsgFormat; // size 0x18

DWCCfError DWCi_CfReset(void);

#ifdef __cplusplus
}
#endif
