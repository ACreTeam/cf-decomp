#ifndef METROTRK_MSGBUF_H
#define METROTRK_MSGBUF_H

#include <MetroTRK/trk.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TRKMSGBUF_SIZE (0x800 + 0x80)

    struct TRKBuffer
    {
        u32 _00;
        BOOL isInUse;
        u32 length;
        u32 position;
        u8 data[TRKMSGBUF_SIZE];
    };

    extern TRKBuffer gTRKMsgBufs[3];

    DSError TRKInitializeMessageBuffers(void);
    DSError TRKGetFreeBuffer(int* msgID, TRKBuffer** outMsg);
    void* TRKGetBuffer(int idx);
    void TRKReleaseBuffer(int idx);
    void TRKResetBuffer(TRKBuffer* msg, BOOL keepData);
    DSError TRKSetBufferPosition(TRKBuffer* msg, u32 pos);
    DSError TRKAppendBuffer(TRKBuffer* msg, const void* data, size_t length);
    DSError TRKReadBuffer(TRKBuffer* msg, void* data, size_t length);

    DSError TRKAppendBuffer1_ui16(TRKBuffer* buffer, const u16 data);
    DSError TRKAppendBuffer1_ui32(TRKBuffer* buffer, const u32 data);
    DSError TRKAppendBuffer1_ui64(TRKBuffer* buffer, const u64 data);

    DSError TRKAppendBuffer_ui8(TRKBuffer* buffer, const u8* data, int count);
    DSError TRKAppendBuffer_ui16(TRKBuffer* buffer, const u16* data, int count);
    DSError TRKAppendBuffer_ui32(TRKBuffer* buffer, const u32* data, int count);
    DSError TRKAppendBuffer_ui64(TRKBuffer* buffer, const u64* data, int count);

    DSError TRKReadBuffer1_ui8(TRKBuffer* buffer, u8* data);
    DSError TRKReadBuffer1_ui16(TRKBuffer* buffer, u16* data);
    DSError TRKReadBuffer1_ui32(TRKBuffer* buffer, u32* data);
    DSError TRKReadBuffer1_ui64(TRKBuffer* buffer, u64* data);

    DSError TRKReadBuffer_ui8(TRKBuffer* buffer, u8* data, int count);
    DSError TRKReadBuffer_ui16(TRKBuffer* buffer, u16* data, int count);
    DSError TRKReadBuffer_ui32(TRKBuffer* buffer, u32* data, int count);
    DSError TRKReadBuffer_ui64(TRKBuffer* buffer, u64* data, int count);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_MSGBUF_H
