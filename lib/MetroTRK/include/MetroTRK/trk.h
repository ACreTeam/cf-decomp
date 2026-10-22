#ifndef METROTRK_TRK_H
#define METROTRK_TRK_H

#include <MetroTRK/dserror.h>
#include <MetroTRK/flush_cache.h>
#include <MetroTRK/trkenum.h>
#include <MetroTRK/notify.h>
#include <revolution/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef u32 DefaultType;
    typedef u32 InstructionType;
    typedef u64 FloatType;
    typedef unsigned char u128[16];

    typedef struct StopInfo_PPC
    {
        u32 PC;
        u32 PCInstruction;
        u16 exceptionID;
    } StopInfo_PPC;

    typedef struct ProcessorRestoreFlags_PPC
    {
        u8 TBR;
        u8 DEC;
        u8 linker_padding[7];
    } ProcessorRestoreFlags_PPC;

    typedef struct TRKState_PPC
    {
        u32 GPR[32];
        u32 LR;
        u32 CTR;
        u32 XER;
        u32 MSR;
        u32 DAR;
        u32 DSISR;
        BOOL stopped;
        BOOL inputActivated;
        u8* inputPendingPtr;
    } TRKState_PPC;

    typedef enum ValidMemoryOptions
    {
        kValidMemoryReadable = 0,
        kValidMemoryWriteable = 1,
    } ValidMemoryOptions;

    typedef enum MemoryAccessOptions
    {
        kUserMemory = 0,
        kDebuggerMemory = 1,
    } MemoryAccessOptions;

    typedef enum DSMessageStepOptions
    {
        kDSStepIntoCount = 0x00,
        kDSStepIntoRange = 0x01,
        kDSStepOverCount = 0x10,
        kDSStepOverRange = 0x11,
        DSSTEP_IntoCount = kDSStepIntoCount,
        DSSTEP_IntoRange = kDSStepIntoRange,
        DSSTEP_OverCount = kDSStepOverCount,
        DSSTEP_OverRange = kDSStepOverRange,
    } DSMessageStepOptions;

    typedef enum DSMessageRegisterOptions
    {
        DSREG_Default = 0,
        DSREG_FP = 1,
        DSREG_Extended1 = 2,
        DSREG_Extended2 = 3,
    } DSMessageRegisterOptions;

    typedef enum DSMessageMemoryOptions
    {
        DSMSGMEMORY_Segmented = 0x01,
        DSMSGMEMORY_Extended = 0x02,
        DSMSGMEMORY_Protected = 0x04,
        DSMSGMEMORY_Userview = 0x08,
        DSMSGMEMORY_Space_program = 0x00,
        DSMSGMEMORY_Space_data = 0x40,
        DSMSGMEMORY_Space_io = 0x80,
    } DSMessageMemoryOptions;

    typedef enum NubEventType
    {
        kNullEvent = 0,
        kShutdownEvent = 1,
        kRequestEvent = 2,
        kBreakpointEvent = 3,
        kExceptionEvent = 4,
        kSupportEvent = 5,
        NUBEVENT_Null = kNullEvent,
        NUBEVENT_Shutdown = kShutdownEvent,
        NUBEVENT_Request = kRequestEvent,
        NUBEVENT_Breakpoint = kBreakpointEvent,
        NUBEVENT_Exception = kExceptionEvent,
        NUBEVENT_Support = kSupportEvent,
    } NubEventType;

    typedef enum ReceiverState
    {
        DSRECV_Wait = 0,
        DSRECV_Found = 1,
        DSRECV_InFrame = 2,
        DSRECV_FrameOverflow = 3,
    } ReceiverState;

    typedef enum UARTErrorOptions
    {
        UART_NoError = 0,
        UART_UnknownBaudRate = 1,
        UART_ConfigurationError = 2,
        UART_BufferOverflow = 3,
        UART_NoData = 4,
    } UARTErrorOptions;

    typedef unsigned long NubEventID;
    typedef int MessageBufferID;
    typedef struct TRKBuffer TRKBuffer;
    typedef TRKBuffer MessageBuffer;

    typedef struct TRKEvent
    {
        union
        {
            NubEventType fType;
            NubEventType eventType;
        };
        union
        {
            NubEventID fID;
            NubEventID eventID;
        };
        union
        {
            MessageBufferID fMessageBufferID;
            MessageBufferID msgBufID;
        };
    } TRKEvent;

    typedef TRKEvent NubEvent;

    typedef struct TRKFramingState
    {
        MessageBufferID msgBufID;
        TRKBuffer* buffer;
        ReceiverState receiveState;
        BOOL isEscape;
        u8 fcsType;
    } TRKFramingState;

    typedef struct CommandReply
    {
        u32 _00;
        union
        {
            u8 b;
            MessageCommandID m;
        } commandID;
        union
        {
            u8 b;
            DSReplyError r;
        } replyError;
        u32 _0C;
        u8 _10[0x30];
    } CommandReply;

    typedef enum DSFileHandle
    {
        kDSStdin = 0,
        kDSStdout = 1,
        kDSStderr = 2,
    } DSFileHandle;

    typedef struct msgbuf_t
    {
        u32 msgLength;
        union
        {
            u8 commandId;
            MessageCommandID commandIdInt;
        };
        union
        {
            u8 replyError;
            u32 replyErrorInt;
        };
        u32 unk0C;
        u8 unk10[0x30];
    } msgbuf_t;

#define DSFetch_s32(_p_) (*((s32*)(_p_)))
#define DSFetch_u8(_p_)  (*((u8*)(_p_)))
#define DSFetch_u32(_p_) (*((u32*)(_p_)))
#define DSFetch_u64(_p_) (*((u64*)(_p_)))

#define kDSWriteFile    DSMSG_WriteFile
#define kDSReadFile     DSMSG_ReadFile
#define kDSOpenFile     DSMSG_OpenFile
#define kDSCloseFile    DSMSG_CloseFile
#define kDSPositionFile DSMSG_PositionFile

#define kDSIONoError DS_IONoError
#define kDSIOError   DS_IOError

#define kDSNotifyStopped      0x90
#define kDSNotifyException    0x91
#define TRK_MSG_HEADER_LENGTH 0x40

#define TRK_DEFAULT_XER 36

#define SPR_GQR0  912
#define SPR_HID2  920
#define SPR_FPECR 1022

#define PPC_PROGRAMERROR 0x0700
#define PPC_TRACE        0x0D00

#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(x) ALIGN(x)
#endif

    DSError TRKAppendBuffer1_ui64(MessageBuffer* buffer, u64 data);
    DSError TRKAppendBuffer_ui8(MessageBuffer* buffer, const u8* data, int count);
    DSError TRKAppendBuffer_ui32(MessageBuffer* buffer, const u32* data, int count);
    DSError TRKReadBuffer1_ui64(MessageBuffer* buffer, u64* data);
    DSError TRKReadBuffer_ui32(MessageBuffer* buffer, u32* data, int count);

    void TRKConstructEvent(NubEvent* event, int eventType);
    DSError TRKPostEvent(NubEvent* event);

    DSError HandleOpenFileSupportRequest(const char* path, u8 replyError, u32* param_3, DSIOResult* ioResult);
    DSError HandleCloseFileSupportRequest(int replyError, DSIOResult* ioResult);
    DSError HandlePositionFileSupportRequest(DSReplyError replyErr, u32* param_2, u8 param_3, DSIOResult* ioResult);
    DSError TRKSuppAccessFile(u32 file_handle, u8* data, size_t* count, DSIOResult* io_result, BOOL need_reply, BOOL read);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_TRK_H
