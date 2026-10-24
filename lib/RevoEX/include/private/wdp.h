#ifndef PRIVATE_WDP_H
#define PRIVATE_WDP_H

#include <private/wdp_types.h>

extern const u32 wdpSegmentFlags[4];
extern const u8 wdpHeaderRegion[8];

u32 WDPi_GetBeaconCheckSum(const u8 *data);
int WDP_CompareMemory(const void *a, const void *b, u32 size);
u32 WDP_CopyString(u16 *dst, u32 size, const u16 *src);
BOOL WDPi_IsValidSegmentRegion(int type, u32 address, u32 size, BOOL extended,
                               BOOL *relocate);
u32 WDPi_InitSegmentMapper(WDPSegmentMapper *mapper, const WDPSegmentInfo *header,
                           u32 packetSize);
BOOL WDPi_GetSegmentRegion(const WDPSegmentMapper *mapper, u32 block,
                           WDPSegmentRegion *region);
BOOL WDP_MergeBitset(u32 *bits, u32 count, u32 index);
BOOL WDPi_SwitchActiveSession(WDPParentContext *context);
void WDPi_UpdateStatus(WDPParentContext *context, u32 aid, s32 status);
void WDP_InitParentContext(WDPParentContext *context, WDPCallback callback,
                           const u16 *name, u8 color, u32 packetSize);
s32 WDP_CallBeaconSendHook(WDPParentContext *context, u8 *buffer);
void WDP_CallParentConnectedHook(WDPParentContext *context, u32 aid);
void WDP_CallParentDisconnectedHook(WDPParentContext *context, u32 aid);
void WDP_CallParentSendHook(WDPParentContext *context, WDPPacket *packet);
void WDP_CallParentRecvHook(WDPParentContext *context, u32 aid,
                            const WDPPacket *packet);
void WDP_RegisterSession(WDPParentContext *context, WDPSession *session,
                         const void *icon, const void *palette, u32 maxChildren,
                         const u16 *title, const u16 *description, u32 ggid,
                         u32 index, const void *auth, const void *userInfo,
                         const u8 *rom, WDPReadSegment readSegment,
                         void *userData);
BOOL WDP_AllowChildStatus(WDPParentContext *context, u32 aid, s32 status);

static inline u32 MI_GetLE32(void* p) {
    const u8* value = (u8*)p;
    return ((value[0]) | (value[1] << 8) | (value[2] << 16) | (value[3]) << 24);
}

static inline void MI_SetLE16(void* p, u16 v) {
    u8* dst = (u8*)p;

    dst[0] = (u8)v;
    dst[1] = (u8)(v >> 8);
}

#endif
