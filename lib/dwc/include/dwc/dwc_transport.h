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
#define DWC_TRANSPORT_KEEPALIVE_TIME 20000
#define DWC_FORWARD_MAGIC_STRINGS "F\xFC" "W"
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
        DWC_SEND_TYPE_MAX,
        DWC_SEND_TYPE_USERDATA_ONE_PACKET = 100
    };

    typedef struct DWCstTransportInfo DWCTransportInfo;
    typedef struct DWCstTransportHeader DWCTransportHeader;
    typedef struct DWCstTransportConnection DWCTransportConnection;

    typedef void (*DWCUserSendCallback)(u32 size, u8 aid, void* param);
    typedef void (*DWCUserRecvTimeoutCallback)(u8 aid, void* param);
    typedef void (*DWCUserPingCallback)(int latency, u8 aid, void* param);

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
        OSTime lastSomeDataRecvTick;
    };

    struct DWCstTransportInfo
    {
        DWCTransportConnection connections[DWC_MAX_CONNECTIONS];
        DWCUserSendCallback sendCallback;
        DWCUserRecvCallback recvCallback;
        DWCUserRecvTimeoutCallback recvTimeoutCallback;
        DWCUserPingCallback pingCallback;
        void* sendCallbackParam;
        void* recvCallbackParam;
        void* recvTimeoutCallbackParam;
        void* pingCallbackParam;
        u16 sendSplitMax;
        u32 connKeepAliveTime;
    };

    struct DWCstTransportHeader
    {
        int size;
        u16 type;
        char magicStrings[DWC_MAGIC_STRINGS_LEN];
    };

    BOOL DWC_IsSendableReliable(u8 aid);
    BOOL DWC_SendReliable(u8 aid, const void* buffer, int size);
    BOOL DWC_SetUserSendCallback(DWCUserSendCallback callback, void* param);
    BOOL DWC_SetUserRecvTimeoutCallback(DWCUserRecvTimeoutCallback callback, void* param);
    BOOL DWC_SetUserPingCallback(DWCUserPingCallback callback, void* param);
    BOOL DWC_SetSendSplitMax(u16 sendSplitMax);
    BOOL DWC_SetConnectionKeepAliveTime(u32 time);
    BOOL DWC_Ping(u8 aid);
    void DWCi_InitTransport(DWCTransportInfo* info);
    void DWCi_ClearTransConnection(u8 aid);
    BOOL DWCi_IsSendableReliable(u8 aid, u16 type);
    BOOL DWCi_SendReliable(
        u16 type, u8 aid, const void* buffer, int size);
    void DWCi_ResetLastSomeDataRecvTimeByAid(u8 aid);
    void DWCi_SendUnreliable(u8 aidFrom, u8 aidTo, const void* buffer,
        int size, int fromDWC_SendUnreliable);

#ifdef __cplusplus
}
#endif
