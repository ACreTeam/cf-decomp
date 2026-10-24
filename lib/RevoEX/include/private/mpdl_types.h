#ifndef PRIVATE_MPDL_TYPES_H
#define PRIVATE_MPDL_TYPES_H

#include <private/mp_types.h>
#include <private/wdp_types.h>
#include <revolution/mpdl.h>

typedef struct MPDLConfig {
    void *(*alloc)(u32);
    void (*free)(void *);
    s32 priority;
    u32 ggid;
    u32 tgid;
    u32 channel;
    u32 favoriteColor;
    const u16 *playerName;
    const u16 *title;
    const u16 *description;
    u32 maxChildren;
    const u8 *rom;
    const void *icon;
    const void *palette;
    u8 userInfo[32];
    WDPParentContext *context;
    WDPSession *session;
    BOOL entryFlag;
    BOOL multiBootFlag;
    u32 reserved;
    MPConfig mp;
} MPDLConfig;

#endif
