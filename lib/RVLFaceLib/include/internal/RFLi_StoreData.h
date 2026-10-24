#ifndef RFLi_STORE_DATA_H
#define RFLi_STORE_DATA_H

#include <RFL_Types.h>
#include <internal/RFLi_HiddenDatabase.h>
#include <internal/RFLi_Types.h>

#include <RFL_MiddleDatabase.h>

#ifdef __cplusplus
extern "C" {
#endif

#if RFL_BUILD >= 20080604
typedef struct {
    u8 num;                      // 0x00
    u8 memberNum;                // 0x01
    u16 myNo;                    // 0x02
    u32 receivedMask;            // 0x04
    u32 storedMask;              // 0x08
    RFLiExchangeBuffer* buffer;  // 0x0C
} RFLiReceiveConverter;

u32 RFLiGetExchangeBufferSize(u16 num);
RFLErrcode RFLiInitExchangeBuffer(RFLiExchangeBuffer* buffer, u16 num);
RFLErrcode RFLiSetExchangeData(RFLiExchangeBuffer* buffer, u16 index, RFLDataSource source, u16 srcIndex, u16 num);

u32 RFLiGetReceiveBufferSize(u16 memberNum, u16 num);
RFLErrcode RFLiInitReceiveConverter(RFLiReceiveConverter* converter, RFLiExchangeBuffer* buffer, u16 myNo, u16 memberNum,
                                    u16 num);
RFLErrcode RFLiStoreReceivedData(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member);
RFLErrcode RFLiReceiveData(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member);
RFLErrcode RFLiReceiveDataCore(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member, BOOL write);
RFLErrcode RFLiClearReceivedMember(RFLiReceiveConverter* converter, u16 member);
RFLErrcode RFLiAddReceivedToMiddleDB(RFLMiddleDatabase* db, const RFLiReceiveConverter* converter, u16 member);
#endif  // RFL_BUILD

#ifdef __cplusplus
}
#endif

#endif  // RFLi_STORE_DATA_H
