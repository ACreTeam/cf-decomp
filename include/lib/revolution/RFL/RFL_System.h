#ifndef RVL_SDK_RFL_SYSTEM_H
#define RVL_SDK_RFL_SYSTEM_H

#include <revolution/RFL/RFL_Types.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 RFLGetWorkSize(BOOL deluxeTexture); // 802B8E00
RFLErrcode RFLInitRes(void *workBuffer, const void *resource, u32 size, u32 flags); // 802B91A0
const char *RFLGetResourceFileName(void); // 802B9818
BOOL RFLIsAvailableOfficialData(u16 index); // 802C406C
RFLErrcode RFLGetCharInfo(RFLCharInfo *info, RFLDataSource source, RFLMiddleDB *middleDB, u16 index); // 802C5A0C

#ifdef __cplusplus
}
#endif
#endif
