#ifndef PRIVATE_NWC24_TIME_H
#define PRIVATE_NWC24_TIME_H

#include <revolution/os/OSTime.h>
#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24iGetUniversalTimeMinute(s32* minutes);
NWC24Err NWC24iGetUniversalTime(OSTime* pTime) NO_INLINE;
NWC24Err NWC24iGetTimeDifference(OSTime* pDiff);

NWC24Err NWC24iSetRtcCounter(u32 rtcCounter, u32 flags);
NWC24Err NWC24iSynchronizeRtcCounter(BOOL forceSave);

char* NWC24iStrTimeR(OSTime seconds, char* buffer, int length);
char* NWC24iStrTime(OSTime seconds);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_TIME_H
