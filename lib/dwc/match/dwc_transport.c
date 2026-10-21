#include <dwc/dwc_transport.h>
#include <dwc/dwc_error.h>
#include <dwc/dwc_main.h>
#include <dwc/dwc_match.h>
#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_reliable.h>
#include <dwc/dwc_report.h>
#include <dwc/dwci_error.h>
#include <gamespy/gt2/gt2.h>
#include <string.h>

static DWCTransportInfo* sTransInfo;

static DWCTransportConnection* DWCs_GetTransConnection(u8 aid);
static int DWCs_GetSendState(u8 aid);
static int DWCs_GetRecvState(u8 aid);
static void DWCs_Send(
    u8 aid, const u8* buffer, int size, BOOL reliable);
static void DWCs_EncodeHeader(
    DWCTransportHeader* header, u16 type, int size);
static u16 DWCs_DecodeHeader(const DWCTransportHeader* message);
static s32 DWCs_GetRequiredHeaderSize(u16 type);
static int DWCs_GetOutgoingBufferFreeSize(u8 aid);
static void DWCs_HandleReliableMessage(
    GT2Connection connection, u8* message, int size);
static void DWCs_RecvDataHeader(
    u8 aid, DWCTransportHeader* message, int size);
static void DWCs_RecvDataBody(u8 aid, void* message, int size);
static void DWCs_HandleUnreliableMessage(
    GT2Connection connection, u8* message, int size);

static DWCTransportConnection* DWCs_GetTransConnection(u8 aid)
{
    return &sTransInfo->connections[aid];
}

static int DWCs_GetSendState(u8 aid)
{
    return sTransInfo->connections[aid].sendState;
}

static int DWCs_GetRecvState(u8 aid)
{
    return sTransInfo->connections[aid].recvState;
}

static void DWCs_EncodeHeader(
    DWCTransportHeader* header, u16 type, int size)
{
    strncpy(header->magicStrings, DWC_MAGIC_STRINGS, DWC_MAGIC_STRINGS_LEN);

    header->type = DWCi_HtoLEs(type);
    header->size = (int)DWCi_HtoLEl((u32)size);
}

static u16 DWCs_DecodeHeader(const DWCTransportHeader* message)
{
    DWCTransportHeader header;

    DWCi_Np_CpuCopy8(message, &header, sizeof(DWCTransportHeader));

    if (memcmp(header.magicStrings, DWC_MAGIC_STRINGS,
            DWC_MAGIC_STRINGS_LEN)
        == 0)
    {
        return DWCi_LEtoHs(header.type);
    }

    return DWC_SEND_TYPE_INVALID;
}

static s32 DWCs_GetRequiredHeaderSize(u16 type)
{
    s32 size;

    switch (type)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
        size = sizeof(DWCTransportHeader) + 28;
        break;
    case DWC_SEND_TYPE_MATCH_SYN_ACK:
        size = sizeof(DWCTransportHeader) + DWC_MATCH_SYN_DATA_BODY_SIZE;
        break;
    case DWC_SEND_TYPE_MATCH_ACK:
    {
        int count = DWCi_NodeInfoList_GetCount() - 1;

        if (count == 0)
        {
            size = 16;
        }
        else
        {
            size = count * 24 + 16;
        }
        break;
    }
    default:
        size = sizeof(DWCTransportHeader);
        break;
    }

    return size;
}

static int DWCs_GetOutgoingBufferFreeSize(u8 aid)
{
    static const int gamespyUseSize = 512;
    GT2Connection connection = DWCi_GetGT2Connection(aid);
    int free;

    free = gt2GetOutgoingBufferFreeSpace(connection)
        - DWC_TRANSPORT_GT2HEADER_SIZE - gamespyUseSize;

    return free > 0 ? free : 0;
}

// ===== RANGE A START =====
BOOL DWC_IsSendableReliable(u8 aid)
{
    return DWCi_IsSendableReliable(aid, DWC_SEND_TYPE_USERDATA);
}

BOOL DWCi_IsSendableReliable(u8 aid, u16 type)
{
    s32 freeSpace;

    if (DWCi_IsError()
        || (type == DWC_SEND_TYPE_USERDATA && !DWC_IsValidAID(aid)))
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING, "aid %d is unavailable.\n", aid);
        return FALSE;
    }

    if (!DWCi_IsValidAID(aid))
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "aid %d doesn't have GT2 connection.\n", aid);
        return FALSE;
    }

    if (DWCs_GetSendState(aid) == DWC_TRANSPORT_SEND_BUSY)
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
            "+++ Cannot send to %d from %d (busy)\n",
            aid,
            DWC_GetMyAID());
        return FALSE;
    }

    freeSpace = DWCs_GetOutgoingBufferFreeSize(aid);

    if (freeSpace < DWCs_GetRequiredHeaderSize(type))
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
            "+++ Cannot send to %d from %d (outgoing buffer is not enough) %d < %d\n",
            aid, DWC_GetMyAID(), freeSpace,
            DWCs_GetRequiredHeaderSize(type));
        return FALSE;
    }

    return DWCi_IsReliableQueueFree() ? TRUE : FALSE;
}

BOOL DWC_SendReliable(u8 aid, const void* buffer, int size)
{
    return DWCi_SendReliable(DWC_SEND_TYPE_USERDATA, aid, buffer, size);
}

static void DWCs_Send(u8 aid, const u8* buffer, int size, BOOL reliable)
{
    GT2Connection connection = DWCi_GetGT2Connection(aid);
    GT2Result result;

    if (DWCi_ProcessReliableQueue())
    {
        result = gt2Send(connection, buffer, size, reliable);
        if (result == GT2Success)
        {
            return;
        }
        DWC_Printf(
            DWC_REPORTFLAG_SEND_INFO, "gt2Send() Failed code:%d\n", result);
    }

    if (!DWCi_InsertReliableQueue(connection, buffer, size))
    {
        DWCi_SetError(DWC_ERROR_NETWORK, DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2 + DWC_ECODE_TYPE_TRANS_SEND);
    }
}

BOOL DWCi_SendReliable(u16 type, u8 aid, const void* buffer, int size)
{
    DWCTransportConnection* transConnection = DWCs_GetTransConnection(aid);
    DWCTransportHeader header;
    int sendSize;
    int freeSpace;

    if (!DWCi_IsSendableReliable(aid, type))
    {
        return FALSE;
    }

    transConnection->sendState = DWC_TRANSPORT_SEND_BUSY;
    transConnection->sendBuffer = buffer;
    transConnection->sendingSize = 0;
    transConnection->requestSendSize = size;

    freeSpace = DWCs_GetOutgoingBufferFreeSize(aid);

    if (type == DWC_SEND_TYPE_USERDATA)
    {
        // header and body in one packet
        u8* packet;

        sendSize = size + sizeof(DWCTransportHeader);

        if (sendSize <= sTransInfo->sendSplitMax && sendSize <= freeSpace)
        {
            packet = DWC_Alloc(DWC_ALLOCTYPE_BASE, sendSize);

            if (packet != NULL)
            {
                DWCs_EncodeHeader((DWCTransportHeader*)packet,
                    DWC_SEND_TYPE_USERDATA_ONE_PACKET, size);
                DWCi_Np_CPUCopyFast(
                    packet + sizeof(DWCTransportHeader), buffer, size);

                DWCs_Send(aid, packet, sendSize, TRUE);
                DWC_Free(DWC_ALLOCTYPE_BASE, packet, 0);

                transConnection->sendState = DWC_TRANSPORT_SEND_READY;
                transConnection->sendBuffer = NULL;
                transConnection->sendingSize = 0;
                transConnection->requestSendSize = 0;

                if (sTransInfo->sendCallback)
                {
                    sTransInfo->sendCallback(
                        size, aid, sTransInfo->sendCallbackParam);
                }

                return TRUE;
            }
        }
    }

    DWCs_EncodeHeader(&header, type, size);
    DWCs_Send(aid, (const u8*)&header, sizeof(DWCTransportHeader), TRUE);

    if (size > sTransInfo->sendSplitMax)
    {
        sendSize = sTransInfo->sendSplitMax;
    }
    else
    {
        sendSize = size;
    }

    // nothing is sent while GT2's outgoing buffer cannot take it
    if (sendSize > freeSpace)
    {
        return TRUE;
    }

    DWCs_Send(aid, buffer, sendSize, TRUE);

    transConnection->sendingSize += sendSize;

    if (transConnection->sendingSize == transConnection->requestSendSize)
    {
        int reqSendSize = transConnection->requestSendSize;

        transConnection->sendState = DWC_TRANSPORT_SEND_READY;
        transConnection->sendBuffer = NULL;
        transConnection->sendingSize = 0;
        transConnection->requestSendSize = 0;

        if (sTransInfo->sendCallback && type == DWC_SEND_TYPE_USERDATA)
        {
            sTransInfo->sendCallback(
                reqSendSize, aid, sTransInfo->sendCallbackParam);
        }
    }

    return TRUE;
}

BOOL DWC_SendUnreliable(u8 aid, const void* buffer, int size)
{
    if (DWCi_IsError())
    {
        return FALSE;
    }

    if (!DWC_IsValidAID(aid))
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING, "aid %d is now unavailable.\n", aid);
        return FALSE;
    }

    if (size > sTransInfo->sendSplitMax - (int)sizeof(DWCTransportHeader))
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
            "+++ SendUnreliable size is too large ( %d > %d )\n",
            size,
            sTransInfo->sendSplitMax - sizeof(DWCTransportHeader));
        return FALSE;
    }

    DWCi_SendUnreliable(DWC_GetMyAID(), aid, buffer, size, TRUE);

    if (sTransInfo->sendCallback != NULL)
    {
        sTransInfo->sendCallback(size, aid, sTransInfo->sendCallbackParam);
    }
    return TRUE;
}

BOOL DWC_SetRecvBuffer(u8 aid, void* buffer, int size)
{
    DWCTransportConnection* entry = &sTransInfo->connections[aid];
    int recvState = entry->recvState;

    if (recvState == DWC_TRANSPORT_RECV_BODY)
    {
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO, "+++ Cannot set recv buffer\n");
        return FALSE;
    }

    entry->recvBuffer = buffer;
    entry->recvBufferSize = size;
    entry->recvState = DWC_TRANSPORT_RECV_HEADER;
    entry->recvingSize = 0;
    entry->requestRecvSize = 0;
    return TRUE;
}

BOOL DWC_Ping(u8 aid)
{
    GT2Connection connection;

    if (DWCi_IsError())
    {
        return FALSE;
    }

    connection = DWCi_GetGT2Connection(aid);

    if (aid == DWC_GetMyAID() || !connection
        || gt2GetConnectionState(connection) != GT2Connected)
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
            "DWC_Ping:not connected yet:%d\n", aid);
        return FALSE;
    }

    gt2Ping(connection);

    return TRUE;
}

BOOL DWC_SetUserSendCallback(DWCUserSendCallback callback, void* param)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->sendCallback = callback;
    sTransInfo->sendCallbackParam = param;
    return TRUE;
}

BOOL DWC_SetUserRecvCallback(DWCUserRecvCallback callback, void* param)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->recvCallback = callback;
    sTransInfo->recvCallbackParam = param;
    return TRUE;
}

BOOL DWC_SetUserRecvTimeoutCallback(
    DWCUserRecvTimeoutCallback callback, void* param)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->recvTimeoutCallback = callback;
    sTransInfo->recvTimeoutCallbackParam = param;
    return TRUE;
}

BOOL DWC_SetUserPingCallback(DWCUserPingCallback callback, void* param)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->pingCallback = callback;
    sTransInfo->pingCallbackParam = param;
    return TRUE;
}

BOOL DWC_SetSendSplitMax(u16 sendSplitMax)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->sendSplitMax = sendSplitMax;
    return TRUE;
}

BOOL DWC_SetConnectionKeepAliveTime(u32 time)
{
    if (sTransInfo == NULL || time == 0)
    {
        return FALSE;
    }
    sTransInfo->connKeepAliveTime = time;
    return TRUE;
}

u32 DWC_GetConnectionKeepAliveTime(void)
{
    if (sTransInfo != NULL)
    {
        return sTransInfo->connKeepAliveTime;
    }
    return 0;
}

void DWCi_InitTransport(DWCTransportInfo* info)
{
    sTransInfo = info;
    memset(sTransInfo, 0, sizeof(DWCTransportInfo));
    sTransInfo->sendSplitMax = DWC_TRANSPORT_SEND_MAX;
    DWC_SetConnectionKeepAliveTime(DWC_TRANSPORT_KEEPALIVE_TIME);
    DWCi_InitReliableQueue();
}

void DWCi_RecvCallback(GT2Connection connection, GT2Byte* message, int len,
    GT2Bool reliable)
{
    u8 aid;

    if (sTransInfo == NULL)
    {
        return;
    }
    if (message == NULL || len == 0)
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "Recv NULL message %x, size = %d\n", message, len);
        return;
    }

    aid = DWCi_GetConnectionAID(connection);

    if (aid < DWC_MAX_CONNECTIONS && aid != DWC_GetMyAID())
    {
        sTransInfo->connections[aid].lastSomeDataRecvTick = DWCi_Np_GetTick();
    }

    if (reliable)
    {
        DWCs_HandleReliableMessage(connection, message, len);
    }
    else
    {
        DWCs_HandleUnreliableMessage(connection, message, len);
    }
}

void DWCi_PingCallback(GT2Connection connection, int latency)
{
    if (sTransInfo->pingCallback != NULL)
    {
        u8 aid = DWCi_GetConnectionAID(connection);
        sTransInfo->pingCallback(latency, aid, sTransInfo->pingCallbackParam);
    }
}
// ===== RANGE A END =====

// ===== RANGE B START =====
void DWCi_TransportProcess(void)
{
    u8* aidList;
    int hostCount;
    int i;
    u32 keepAliveTime;
    DWCTick tick;

    if (!sTransInfo)
    {
        return;
    }

    hostCount = DWC_GetAIDList(&aidList);
    tick = DWCi_Np_GetTick();
    keepAliveTime = DWC_GetConnectionKeepAliveTime();

    if (DWC_IsServerMyself() || (DWCi_IsServerChanging() && DWCi_IsBackupMine()))
    {
        for (i = 0; i < hostCount; i++)
        {
            u8 aid = aidList[i];

            if (aid != DWC_GetMyAID())
            {
                DWCTransportConnection* transConnection
                    = &sTransInfo->connections[aid];
                u32 time = (u32)DWCi_Np_TicksToMilliSeconds(
                    tick - transConnection->lastSomeDataRecvTick);

                if (time > keepAliveTime)
                {
                    DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                        "shutdown client: DWCi_TransportProcess:timeout aid=%d,time=%d[ms],timeout time=%d[ms]\n",
                        aid, time, transConnection->lastSomeDataRecvTick);
                    DWC_CloseConnectionHard(aid);
                    DWCi_SendServerPolling(TRUE);
                }
            }
        }
    }
    else
    {
        u8 serverAid = DWCi_GetServerOrSubServerAID();

        if (serverAid != 0xFF && serverAid != DWC_GetMyAID())
        {
            DWCTransportConnection* transConnection
                = &sTransInfo->connections[serverAid];
            u32 time = (u32)DWCi_Np_TicksToMilliSeconds(
                tick - transConnection->lastSomeDataRecvTick);

            if (time > keepAliveTime)
            {
                DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                    "shutdown server: DWCi_TransportProcess:timeout aid=%d,time=%d[ms],timeout time=%d[ms]\n",
                    serverAid, time, transConnection->lastSomeDataRecvTick);
                DWC_CloseConnectionHard(serverAid);
            }
        }
    }

    for (i = 0; i < hostCount; i++)
    {
        u8 aid;

        aid = aidList[i];

        DWCi_ProcessReliableQueue();

        if (DWC_IsValidAID(aid))
        {
            DWCTransportConnection* transConnection;
            DWCTick currentTick;

            currentTick = DWCi_Np_GetTick();
            transConnection = DWCs_GetTransConnection(aid);

            if (sTransInfo->recvTimeoutCallback
                && transConnection->recvTimeoutTime > 0)
            {
                u32 time;

                time = (u32)DWCi_Np_TicksToMilliSeconds(
                    currentTick - transConnection->previousRecvTick);
                if (time > transConnection->recvTimeoutTime)
                {
                    DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                        "DWCi_TransportProcess:timeout aid=%d,time=%d[ms],timeout time=%d[ms]\n",
                        aid, time, transConnection->recvTimeoutTime);
                    sTransInfo->recvTimeoutCallback(
                        aid, sTransInfo->recvTimeoutCallbackParam);
                    transConnection->previousRecvTick = currentTick;
                }
            }
        }

        if (aid != DWC_GetMyAID()
            && DWCs_GetSendState(aid) == DWC_TRANSPORT_SEND_BUSY)
        {
            s32 sendSize;
            s32 freeSpace;
            DWCTransportConnection* transConnection;

            transConnection = DWCs_GetTransConnection(aid);
            sendSize = transConnection->requestSendSize
                - transConnection->sendingSize;

            if (sendSize > sTransInfo->sendSplitMax)
            {
                sendSize = sTransInfo->sendSplitMax;
            }

            freeSpace = DWCs_GetOutgoingBufferFreeSize(aid);
            if (freeSpace < sendSize)
            {
                DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
                    "DWCi_TransportProcess:freeSpace < sendSize:aid:%d, %d < %d\n",
                    aid, freeSpace, sendSize);
                continue;
            }

            DWCs_Send(aid,
                (u8*)transConnection->sendBuffer
                    + transConnection->sendingSize,
                sendSize, TRUE);

            transConnection->sendingSize += sendSize;

            if (transConnection->sendingSize
                == transConnection->requestSendSize)
            {
                int reqSendSize = transConnection->requestSendSize;

                transConnection->sendState = DWC_TRANSPORT_SEND_READY;
                transConnection->sendBuffer = NULL;
                transConnection->sendingSize = 0;
                transConnection->requestSendSize = 0;

                if (sTransInfo->sendCallback)
                {
                    sTransInfo->sendCallback(
                        reqSendSize, aid, sTransInfo->sendCallbackParam);
                }
            }
        }
    }
}

void DWCi_ClearTransConnection(u8 aid)
{
    if (sTransInfo == NULL)
    {
        return;
    }
    sTransInfo->connections[aid].sendingSize = 0;
    sTransInfo->connections[aid].recvingSize = 0;
    sTransInfo->connections[aid].requestSendSize = 0;
    sTransInfo->connections[aid].requestRecvSize = 0;
    sTransInfo->connections[aid].sendState = 0;
    sTransInfo->connections[aid].recvBuffer = NULL;
    sTransInfo->connections[aid].recvBufferSize = 0;
    sTransInfo->connections[aid].recvState = 0;
    sTransInfo->connections[aid].lastRecvType = 0;
}

void DWCi_ShutdownTransport(void)
{
    sTransInfo = NULL;
    DWCi_CleanupReliableQueue();
}

void DWCi_ResetLastSomeDataRecvTimeByAid(u8 aid)
{
    sTransInfo->connections[aid].lastSomeDataRecvTick = DWCi_Np_GetTick();
}

void DWCi_SendUnreliable(
    u8 aidFrom, u8 aidTo, const void* buffer, int size, int fromDWC_SendUnreliable)
{
    GT2Connection connection;
    DWCNodeInfo* node;
    int bNeedFoward = 0;
    u8* message;

    connection = DWCi_GetGT2Connection(aidTo);
    if (connection == NULL)
    {
        if (!DWC_IsServerMyself())
        {
            node = DWCi_NodeInfoList_GetNodeInfoForIndex(0);
            connection = DWCi_GetGT2Connection(node->aid);
            if (connection == NULL)
            {
                DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
                    "+++ drop relay unreliable data. (aid %d->%d)\n", aidFrom,
                    aidTo);
                DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
                    "+++ sv gt2Connection is NULL.(sv aid:%d myaid:%d)\n",
                    node->aid, DWC_GetMyAID());
                return;
            }

            DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
                "+++ forward unreliable. sent(aid %d->%d)\n", aidFrom, aidTo);
            bNeedFoward = 1;
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
                "+++ drop relay unreliable data. (aid %d->%d)\n", aidFrom,
                aidTo);
            return;
        }
    }

    if (fromDWC_SendUnreliable == 1 && bNeedFoward == 1)
    {
        message = DWC_Alloc(DWC_ALLOCTYPE_BASE, (u32)(size + 8));
        if (message == NULL)
        {
            return;
        }

        strncpy((char*)message, DWC_FORWARD_MAGIC_STRINGS, 3);
        message[3] = aidFrom;
        *(u32*)(message + 4) = DWCi_HtoLEl(1U << aidTo);
        DWCi_Np_CPUCopyFast(message + 8, buffer, (u32)size);
        gt2Send(connection, message, size + 8, GT2False);
        if (message != NULL)
        {
            DWC_Free(DWC_ALLOCTYPE_BASE, message, 0);
        }
    }
    else
    {
        gt2Send(connection, buffer, size, GT2False);
    }
}

// Stripped from the final link (only its string survives in .data): the
// bitmap variant of DWC_SendUnreliable hands the message to the server,
// which relays it to every aid in the bitmap.
static void DWCs_SendUnreliableBitmapForwardToServer(
    u32 bitmap, const void* buffer, int size)
{
    DWCNodeInfo* node = DWCi_NodeInfoList_GetNodeInfoForIndex(0);
    GT2Connection connection = DWCi_GetGT2Connection(node->aid);
    u8* message;

    if (connection == NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO, "server is now unavailable.\n");
        return;
    }

    message = DWC_Alloc(DWC_ALLOCTYPE_BASE, (u32)(size + 8));
    if (message == NULL)
    {
        return;
    }

    strncpy((char*)message, DWC_FORWARD_MAGIC_STRINGS, 3);
    message[3] = DWC_GetMyAID();
    *(u32*)(message + 4) = DWCi_HtoLEl(bitmap);
    DWCi_Np_CPUCopyFast(message + 8, buffer, (u32)size);
    gt2Send(connection, message, size + 8, GT2False);
    DWC_Free(DWC_ALLOCTYPE_BASE, message, 0);
}

// Stripped from the final link as well. It sits before DWC_SetRecvBuffer in the
// original; it can only be placed here because the static helper above must
// precede its first use in this file.
BOOL DWC_SendUnreliableBitmap(u32 bitmap, const void* buffer, int size)
{
    if (DWCi_IsError())
    {
        return FALSE;
    }

    if (size > sTransInfo->sendSplitMax)
    {
        DWC_Printf(DWC_REPORTFLAG_SEND_INFO,
            "+++ SendUnreliable size is too large ( %d > %d )\n", size,
            sTransInfo->sendSplitMax);
        return FALSE;
    }

    DWCs_SendUnreliableBitmapForwardToServer(bitmap, buffer, size);
    return TRUE;
}

static void DWCs_HandleReliableMessage(
    GT2Connection connection, u8* message, int size)
{
    u8 aid = DWCi_GetConnectionAID(connection);

    if (aid == 0xFF)
    {
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "+++ Invalid connection. has no DWCConnectionInfo\n");
        DWCi_SetError(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2
                + DWC_ECODE_TYPE_TRANS_CONNECTION);
        return;
    }

    switch (DWCs_GetRecvState(aid))
    {
    case DWC_TRANSPORT_RECV_NOBUF:
    {
        u16 type = DWCs_DecodeHeader((DWCTransportHeader*)message);

        if (type >= DWC_SEND_TYPE_MATCH_SYN
            && type < DWC_SEND_TYPE_MAX)
        {
            DWCs_RecvDataHeader(aid, (DWCTransportHeader*)message, size);
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ Recv buffer is not set\n");
        }
        break;
    }
    case DWC_TRANSPORT_RECV_HEADER:
        DWCs_RecvDataHeader(aid, (DWCTransportHeader*)message, size);
        break;
    case DWC_TRANSPORT_RECV_BODY:
        DWCs_RecvDataBody(aid, message, size);
        break;
    case DWC_TRANSPORT_RECV_SYSTEM_DATA:
    {
        DWCTransportConnection* transConnection = DWCs_GetTransConnection(aid);

        transConnection->recvState = transConnection->lastRecvState;

        switch (transConnection->lastRecvType)
        {
        case DWC_SEND_TYPE_MATCH_SYN:
        case DWC_SEND_TYPE_MATCH_SYN_ACK:
        case DWC_SEND_TYPE_MATCH_ACK:
            DWCi_ProcessMatchSynPacket(
                aid, transConnection->lastRecvType, message);
            break;
        }
        break;
    }
    case DWC_TRANSPORT_RECV_ERROR:
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "+++ Recv size is too large ( buffer size = %d < %d )\n",
            sTransInfo->connections[aid].recvBufferSize, size);
        sTransInfo->connections[aid].recvState = DWC_TRANSPORT_RECV_HEADER;
        sTransInfo->connections[aid].recvingSize = 0;
        sTransInfo->connections[aid].requestRecvSize = 0;
        break;
    default:
        DWC_Printf(DWC_REPORTFLAG_ERROR, "Recv error (state is %d).\n",
            DWCs_GetRecvState(aid));
        DWCi_SetError(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2
                + DWC_ECODE_TYPE_TRANS_HEADER);
        break;
    }
}

static void DWCs_HandleUnreliableMessage(
    GT2Connection connection, u8* message, int size)
{
    u8 connAid = DWCi_GetConnectionAID(connection);
    u8 aid;
    int forwarded = 0;
    int receive = 0;
    DWCTransportConnection* transConnection;

    if (connAid == 0xFF)
    {
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "+++ Invalid connection. has no DWCConnectionInfo\n");
        DWCi_SetError(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2
                + DWC_ECODE_TYPE_TRANS_CONNECTION);
        return;
    }

    aid = connAid;

    if (size >= sizeof(DWCSBMessageHeader)
        && strncmp((char*)message, DWC_SB_COMMAND_STRING, 4) == 0)
    {
        DWCi_HandleGT2UnreliableMatchCommandMessage((char*)message, size);
        return;
    }

    if (size >= 8 && memcmp(message, DWC_FORWARD_MAGIC_STRINGS, 3) == 0)
    {
        forwarded = 1;
    }

    if (forwarded == 1)
    {
        u32 bitmap;
        u8 i;

        aid = message[3];
        bitmap = DWCi_LEtoHl(*(u32*)(message + 4));

        if (!DWC_IsServerMyself())
        {
            if (bitmap & (1U << DWC_GetMyAID()))
            {
                receive = 1;
            }
            else
            {
                return;
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ fwd received unreliable. sent(aid %d->%08x)\n", aid,
                bitmap);

            for (i = 0; i < DWC_MAX_CONNECTIONS; i++)
            {
                if (bitmap & (1U << i))
                {
                    if (i == DWC_GetMyAID())
                    {
                        receive = 1;
                    }
                    else
                    {
                        DWCi_SendUnreliable(aid, i, message, size, 0);
                    }
                }
            }
        }

        if (!receive)
        {
            return;
        }
    }

    transConnection = DWCs_GetTransConnection(aid);

    if (forwarded == 1)
    {
        message += 8;
        size -= 8;
    }

    if (transConnection->recvBuffer
        && transConnection->recvBufferSize >= size)
    {
        if (sTransInfo->recvCallback)
        {
            sTransInfo->recvCallback(
                aid, message, (u32)size, sTransInfo->recvCallbackParam);
        }

        if (sTransInfo->recvTimeoutCallback
            && transConnection->recvTimeoutTime > 0)
        {
            transConnection->previousRecvTick = DWCi_Np_GetTick();
        }
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "Recv data size is too large (%d > %d)\n", size,
            transConnection->recvBufferSize);
    }
}

static void DWCs_RecvDataHeader(
    u8 aid, DWCTransportHeader* message, int size)
{
    u16 type;
    DWCTransportConnection* connection = &sTransInfo->connections[aid];
    DWCTransportHeader header;

    connection->lastRecvState = (u8)DWCs_GetRecvState(aid);

    switch (type = DWCs_DecodeHeader(message))
    {
    case DWC_SEND_TYPE_USERDATA:
        if (size != sizeof(DWCTransportHeader))
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ Invalid header from aid %d\n", aid);
            return;
        }

        DWCi_Np_CpuCopy8(message, &header, sizeof(DWCTransportHeader));
        header.size = (int)DWCi_LEtoHl((u32)header.size);
        header.type = DWCi_LEtoHs(header.type);

        connection->requestRecvSize = header.size;
        connection->recvingSize = 0;

        if (connection->recvBuffer
            && connection->recvBufferSize >= connection->requestRecvSize)
        {
            connection->recvState = DWC_TRANSPORT_RECV_BODY;
        }
        else
        {
            connection->recvState = DWC_TRANSPORT_RECV_ERROR;
        }
        break;

    case DWC_SEND_TYPE_USERDATA_ONE_PACKET:
        if (size < sizeof(DWCTransportHeader))
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ too small header from aid %d\n", aid);
            return;
        }

        DWCi_Np_CpuCopy8(message, &header, sizeof(DWCTransportHeader));
        header.size = (int)DWCi_LEtoHl((u32)header.size);
        header.type = DWCi_LEtoHs(header.type);

        if (size != header.size + sizeof(DWCTransportHeader))
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ Invalid header from aid %d, combined\n", aid);
            return;
        }

        if (connection->recvBuffer
            && connection->recvBufferSize >= connection->requestRecvSize)
        {
            DWCi_Np_CpuCopy8((u8*)message + sizeof(DWCTransportHeader),
                connection->recvBuffer, (u32)header.size);
            connection->recvState = DWC_TRANSPORT_RECV_HEADER;
            connection->recvingSize = 0;
            connection->requestRecvSize = 0;

            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "combined, aid = %d, size = %d\n", aid, header.size);

            if (sTransInfo->recvCallback)
            {
                sTransInfo->recvCallback(aid, connection->recvBuffer,
                    (u32)header.size, sTransInfo->recvCallbackParam);
            }
        }
        else
        {
            connection->recvState = DWC_TRANSPORT_RECV_ERROR;
        }
        break;

    case DWC_SEND_TYPE_MATCH_SYN:
    case DWC_SEND_TYPE_MATCH_SYN_ACK:
    case DWC_SEND_TYPE_MATCH_ACK:
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO, "Received system header(%d).\n",
            type);
        connection->recvState = DWC_TRANSPORT_RECV_SYSTEM_DATA;
        break;

    default:
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "+++ Invalid header from aid %d (type:%d)\n", aid, type);
        break;
    }

    connection->lastRecvType = type;
}

static void DWCs_RecvDataBody(u8 aid, void* message, int size)
{
    DWCTransportConnection* connection;
    int requestSize;

    connection = &sTransInfo->connections[aid];

    if ((int)connection->recvState == DWC_TRANSPORT_RECV_BODY)
    {
        if (connection->recvingSize + size > connection->recvBufferSize)
        {
            DWC_Printf(DWC_REPORTFLAG_ERROR, "Recv buffer over flow.\n");
            DWCi_SetError(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2
                    + DWC_ECODE_TYPE_TRANS_BODY);
            return;
        }

        DWCi_Np_CpuCopy8(message,
            (u8*)connection->recvBuffer + connection->recvingSize,
            (u32)size);
    }

    connection->recvingSize += size;

    DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
        "aid = %d size = %d/%d state = %d incoming buffer = %d\n", aid,
        connection->recvingSize, connection->requestRecvSize,
        DWCs_GetRecvState(aid),
        gt2GetIncomingBufferFreeSpace(DWCi_GetGT2Connection(aid)));

    if (connection->recvingSize == connection->requestRecvSize)
    {
        requestSize = connection->requestRecvSize;
        connection->recvState = DWC_TRANSPORT_RECV_HEADER;
        connection->recvingSize = 0;
        connection->requestRecvSize = 0;

        if (sTransInfo->recvCallback)
        {
            sTransInfo->recvCallback(aid, connection->recvBuffer,
                (u32)requestSize, sTransInfo->recvCallbackParam);
        }
    }

    if (sTransInfo->recvTimeoutCallback && connection->recvTimeoutTime > 0)
    {
        connection->previousRecvTick = DWCi_Np_GetTick();
    }
}
// ===== RANGE B END =====
