#pragma once

#include <dwc/dwc_match.h>
#include <dwc/dwc_transport_fwd.h>
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DWC_MAGIC_STRINGS      "DT"
#define DWC_MAGIC_STRINGS_LEN  2
#define DWC_TRANSPORT_SEND_MAX 1465
#define DWC_TRANSPORT_GT2HEADER_SIZE 7

    enum
    {
        DWC_TRANSPORT_SEND_READY,
        DWC_TRANSPORT_SEND_BUSY,
        DWC_TRANSPORT_SEND_LAST
    };

    enum
    {
        DWC_TRANSPORT_RECV_NOBUF,
        DWC_TRANSPORT_RECV_HEADER,
        DWC_TRANSPORT_RECV_BODY,
        DWC_TRANSPORT_RECV_SYSTEM_DATA,
        DWC_TRANSPORT_RECV_ERROR,
        DWC_TRANSPORT_RECV_LAST
    };

    enum
    {
        DWC_SEND_TYPE_INVALID,
        DWC_SEND_TYPE_USERDATA,
        DWC_SEND_TYPE_MATCH_SYN,
        DWC_SEND_TYPE_MATCH_SYN_ACK,
        DWC_SEND_TYPE_MATCH_ACK,
        DWC_SEND_TYPE_MAX
    };

    typedef struct DWCstTransportInfo DWCTransportInfo;
    typedef struct DWCstTransportHeader DWCTransportHeader;
    typedef struct DWCstTransportConnection DWCTransportConnection;

    typedef void (*DWCUserSendCallback)(int size, u8 aid);
    typedef void (*DWCUserRecvTimeoutCallback)(u8 aid);
    typedef void (*DWCUserPingCallback)(int latency, u8 aid);

    struct DWCstTransportConnection
    {
        const u8* sendBuffer;
        u8* recvBuffer;
        int recvBufferSize;
        int sendingSize;
        int recvingSize;
        int requestSendSize;
        int requestRecvSize;
        u8 sendState;
        u8 recvState;
        u8 lastRecvState;
        u8 pads[3];
        u16 lastRecvType;
        OSTime previousRecvTick;
        u32 recvTimeoutTime;
    };

    struct DWCstTransportInfo
    {
        DWCTransportConnection connections[DWC_MAX_CONNECTIONS];
        DWCUserSendCallback sendCallback;
        DWCUserRecvCallback recvCallback;
        DWCUserRecvTimeoutCallback recvTimeoutCallback;
        DWCUserPingCallback pingCallback;
        u16 sendSplitMax;
    };

    struct DWCstTransportHeader
    {
        int size;
        u16 type;
        char magicStrings[DWC_MAGIC_STRINGS_LEN];
    };

    BOOL DWC_Ping(u8 aid);
    void DWCi_InitTransport(DWCTransportInfo* info);
    void DWCi_ClearTransConnection(u8 aid);
    BOOL DWCi_IsSendableReliable(u8 aid, u16 type);
    BOOL DWCi_SendReliable(
        u16 type, u8 aid, const void* buffer, int size);

#ifdef __cplusplus
}
#endif
