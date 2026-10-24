#ifndef PRIVATE_WD_H
#define PRIVATE_WD_H

#include <revolution/wd.h>

#include <private/ios/types.h>

struct WDi_Work_ {
    vs32 IOSFd;        // 0x00
    BOOL initialized;  // 0x04
    vs32 IOSHeap;      // 0x08
};

typedef struct WDMpConfig {
    u8 reserved00[0xac];
    u16 type;
    u16 interval;
    u8 options;
    u8 reservedB1;
    u16 reservedB2;
    u8 channel;
    u8 reservedB5[0x83];
    u16 reserved138;
    u8 reserved13A[0x46];
} WDMpConfig;

typedef struct WDNotification {
    u32 type;
    u32 status;
    u8 macAddress[6];
    u16 aid;
    u16 reason;
    u8 reserved12[2];
    u8 gameInfo[32];
    u8 reserved34[0x6c];
} WDNotification;

enum {
    WD_INTERNAL_ERR_OK = 0,
    WD_INTERNAL_ERR_INVALID_FD = 0x80008000,
    WD_INTERNAL_ERR_BAD_ARGUMENTS = 0x80008001,
    WD_INTERNAL_ERR_IPC_ERROR = 0x80008002,
    WD_INTERNAL_ERR_WL_ERROR = 0x80008003,
    WD_INTERNAL_ERR_4 = 0x80008004,
    WD_INTERNAL_ERR_ALLOC_FAILED = 0x80008005,
    WD_INTERNAL_ERR_ALREADY_INITIALIZED = 0x80008006,
};

struct WDi_Work_* WDi_GetWork();
#define WDi_Work ((struct WDi_Work_*)WDi_GetWork())
#define WDi_WorkSize (sizeof(struct WDi_Work_))

s32 WDi_Ioctlv(s32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vec);

s32 WDi_SetupHeap(u32 size);
s32 WDi_CleanupHeap();

s32 WD_Startup(u32 flags);
s32 WD_Cleanup();

s32 WD_SetLinkState(u32 mode);
s32 WD_GetLinkState(void);
s32 WD_SetConfig(void* buffer, u32 mode, u32 flags, u32 option);
s32 WD_ChangeBeacon(u16 tsf, void* beacon);
s32 WD_ChangeVTSF(u16 tsf);
s32 WD_DisAssoc(const u8* mac);
s32 WD_MpSendFrame(void* request, u16 size, void* result);
s32 WD_ReceiveNotification(void* notification);
s32 WD_ReceiveFrame(void* request, u16 size);

s32 WD_Scan(WDScanParam* params, u8* buffer, u32 bufferLen);
s32 WD_GetInfo(WD_Info* info);

#endif  // PRIVATE_WD_H
