#ifndef REVOLUTION_MPDL_H
#define REVOLUTION_MPDL_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MPDLPlayerInfo {
    u32 playerNumber;
    u32 favoriteColor;
    u16 nameLength;
    u8 name[20];
    u8 macAddress[6];
    u32 progress;
} MPDLPlayerInfo;

s32 MPDLStartup(void *config);
s32 MPDLCleanup(void);
u32 MPDLStartDownload(void);
BOOL MPDLGetPlayerInfo(u32 aid, MPDLPlayerInfo *info);
u32 MPDLGetEntryBitmap(void);
u32 MPDLGetBootedBitmap(void);

#ifdef __cplusplus
}
#endif

#endif
