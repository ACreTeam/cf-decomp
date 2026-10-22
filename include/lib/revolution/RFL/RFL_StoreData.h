#ifndef RVL_SDK_RFL_STORE_DATA_H
#define RVL_SDK_RFL_STORE_DATA_H

#include <revolution/RFL/RFL_Types.h>

#ifdef __cplusplus
extern "C" {
#endif

// RFL_StoreData.c: the buffers used to exchange Mii store data between consoles. The names are inferred from what the
// functions do (the original has no symbols for them).

// A send buffer holds up to `num` store data records (0x4C bytes each) behind an 8-byte header.
u32 RFLiGetExchangeBufferSize(u16 num);                                             // 802C6010
void RFLiInitExchangeBuffer(void *buf, u16 num);                                    // 802C601C
RFLErrcode RFLiSetExchangeData(void *buf, int, int, u16 index, u16 num);            // 802C60B4

// The receive side collects the buffers of `memberNum` members.
u32 RFLiGetReceiveBufferSize(u16 memberNum, u16 num);                               // 802C61EC
void RFLiInitReceiveConverter(u32 *converter, void *buf, u16 myNo, u16 memberNum, u16 num); // 802C6200
RFLErrcode RFLiReceiveData(u32 *converter, const void *data, u16 member);           // 802C63E8
void RFLiClearReceivedMember(u32 *converter, u16 member);                           // 802C64E4
RFLErrcode RFLiAddReceivedToMiddleDB(RFLMiddleDB *db, u32 *converter, u16 member);  // 802C6574

#ifdef __cplusplus
}
#endif
#endif
