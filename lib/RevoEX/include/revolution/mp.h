#ifndef REVOLUTION_MP_H
#define REVOLUTION_MP_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MPCallback)(s32 result, const void *data);

s32 MPStartup(void *config);
s32 MPCleanup(void);
s32 MPSetUserGameInfo(const void *data, u32 size);
s32 MPSetEntryFlag(BOOL enabled);
s32 MPSendAsync(const void *data, u32 size, u16 aidBitmap, u32 port,
                u32 *result, MPCallback callback, void *arg);
s32 MPDisconnectAll(void);
u32 MPGetConnectedAIDs(void);
s32 MPUpdateBeacon(void);
s32 MPUpdateBeaconAsync(MPCallback callback, void *arg);

#ifdef __cplusplus
}
#endif

#endif
