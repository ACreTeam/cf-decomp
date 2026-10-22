#ifndef RVL_SDK_HBM_TYPES_H
#define RVL_SDK_HBM_TYPES_H

#include <revolution/mtx.h>

typedef struct KPADStatus KPADStatus;
typedef struct MEMAllocator MEMAllocator;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
typedef bool (*HBMSoundCallback)(signed long evt, signed long num);
#else
typedef int (*HBMSoundCallback)(signed long evt, signed long num);
#endif

typedef struct HBMDataInfo {
    /* 0x00 */ void* layoutBuf;
    /* 0x04 */ void* spkSeBuf;
    /* 0x08 */ void* msgBuf;
    /* 0x0C */ void* configBuf;
    /* 0x10 */ void* mem;
    /* 0x14 */ HBMSoundCallback sound_callback;
    /* 0x18 */ int backFlag;
    /* 0x1C */ int region;
    /* 0x20 */ int cursor;
    /* 0x24 */ int messageFlag;
#if defined(VERSION_R4QK01)
    /* 0x28 */ unsigned long configBufSize;
#endif
    // The remaining fields are four bytes later in the Korean SDK.
    /* 0x28 */ unsigned long memSize;
    /* 0x2C */ float frameDelta;
    /* 0x30 */ Vec2 adjust;
    /* 0x38 */ MEMAllocator* pAllocator;
} HBMDataInfo; // size 0x3C, 0x40 in Korea

typedef struct HBMKPadData {
    /* 0x00 */ KPADStatus* kpad;
    /* 0x04 */ Vec2 pos;
    /* 0x0C */ unsigned long use_devtype;
} HBMKPadData; // size = 0x10

typedef struct HBMControllerData {
    /* 0x00 */ HBMKPadData wiiCon[4];
} HBMControllerData; // size = 0x40

#ifdef __cplusplus
}
#endif

#endif // RVL_SDK_HBM_TYPES_H
