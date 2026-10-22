#ifndef VCT_VAD_H
#define VCT_VAD_H

#include <revolution/types.h>

typedef enum {
    VCT_VAD_NONE = 0,
    VCT_VAD_DETECT,
    VCT_VAD_ACTIVE,
    VCT_VAD_LOST,
} VCTVADStatus;

typedef struct VCTVADInfo {
    BOOL activity;
    int scale;
    int releaseCount;
    int releaseTime;
} VCTVADInfo;

VCTVADStatus VCTi_GetVADStatus(u32 scale, u32 E1, u8* outScale);
void vct_init_vad(void);

#endif
