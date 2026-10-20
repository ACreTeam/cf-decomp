#include <dwc/dwc_transport.h>
#include <dwc/dwc_error.h>
#include <dwc/dwc_main.h>
#include <dwc/dwc_nonport.h>
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
static void DWCs_RecvSystemDataBody(u8 aid, void* message, int size);

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

static void DWCs_Send(u8 aid, const u8* buffer, int size, BOOL reliable)
{
    GT2Connection connection = DWCi_GetGT2Connection(aid);

    gt2Send(connection, buffer, size, reliable);
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
    case DWC_SEND_TYPE_MATCH_SYN_ACK:
    case DWC_SEND_TYPE_MATCH_ACK:
        size = sizeof(DWCTransportHeader) + DWC_MATCH_SYN_DATA_BODY_SIZE;
        break;
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

BOOL DWCi_IsSendableReliable(u8 aid, u16 type)
{
    s32 freeSpace;

    if (DWCi_IsError()
        || (type == DWC_SEND_TYPE_USERDATA && !DWC_IsValidAID(aid))
        || !DWCi_IsValidAID(aid))
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING, "aid %d is unavailable.\n", aid);
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

    return TRUE;
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

    freeSpace = DWCs_GetOutgoingBufferFreeSize(aid);

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
            sTransInfo->sendCallback(reqSendSize, aid);
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
        DWC_Printf(8, "aid %d is now unavailable.\n", aid);
        return FALSE;
    }

    if (size > sTransInfo->sendSplitMax)
    {
        DWC_Printf(0x8000,
            "+++ SendUnreliable size is too large ( %d > %d )\n",
            size,
            sTransInfo->sendSplitMax);
        return FALSE;
    }

    gt2Send(DWCi_GetGT2Connection(aid), buffer, size, GT2False);
    if (sTransInfo->sendCallback != NULL)
    {
        sTransInfo->sendCallback(size, aid);
    }
    return TRUE;
}

BOOL DWC_SetRecvBuffer(u8 aid, void* buffer, int size)
{
    DWCTransportConnection* entry = &sTransInfo->connections[aid];
    int recvState = entry->recvState;

    if (recvState == 2)
    {
        DWC_Printf(0x10000, "+++ Cannot set recv buffer\n");
        return FALSE;
    }

    entry->recvBuffer = buffer;
    entry->recvBufferSize = size;
    entry->recvState = 1;
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

// Later DWC only
BOOL DWC_SetUserSendCallback(DWCUserSendCallback callback)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->sendCallback = callback;
    return TRUE;
}

BOOL DWC_SetUserRecvCallback(DWCUserRecvCallback callback)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->recvCallback = callback;
    return TRUE;
}

// Later DWC only
BOOL DWC_SetUserRecvTimeoutCallback(DWCUserRecvTimeoutCallback callback)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->recvTimeoutCallback = callback;
    return TRUE;
}

// Later DWC only: a connection's receive timeout, counted from now
BOOL DWC_SetRecvTimeoutTime(u8 aid, u32 time)
{
    if (sTransInfo == NULL)
    {
        return FALSE;
    }
    sTransInfo->connections[aid].recvTimeoutTime = time;
    sTransInfo->connections[aid].previousRecvTick = DWCi_Np_GetTick();
    return TRUE;
}

void DWCi_InitTransport(DWCTransportInfo* info)
{
    sTransInfo = info;
    memset(sTransInfo, 0, sizeof(DWCTransportInfo));
    sTransInfo->sendSplitMax = DWC_TRANSPORT_SEND_MAX;
}

void DWCi_RecvCallback(GT2Connection connection, GT2Byte* message, int len,
    GT2Bool reliable)
{
    DWCTransportConnection* entry;
    u8 aid;

    if (sTransInfo == NULL)
    {
        return;
    }
    if (message == NULL || len == 0)
    {
        DWC_Printf(8, "Recv NULL message %x, size = %d\n", message, len);
        return;
    }
    if (reliable)
    {
        DWCs_HandleReliableMessage(connection, message, len);
        return;
    }

    aid = DWCi_GetConnectionAID(connection);
    entry = &sTransInfo->connections[aid];
    if (entry->recvBuffer != NULL && entry->recvBufferSize >= len)
    {
        DWCi_Np_CpuCopy8(message, entry->recvBuffer, len);
    }
    else
    {
        DWC_Printf(0x10000,
            "Recv data size is too large (%d > %d)\n",
            len,
            entry->recvBufferSize);
        return;
    }

    if (sTransInfo->recvCallback != NULL)
    {
        sTransInfo->recvCallback(aid, entry->recvBuffer, len);
    }
    if (sTransInfo->recvTimeoutCallback != NULL && entry->recvTimeoutTime != 0)
    {
        entry->previousRecvTick = DWCi_Np_GetTick();
    }
}

void DWCi_PingCallback(GT2Connection connection, int latency)
{
    if (sTransInfo->pingCallback != NULL)
    {
        u8 aid = DWCi_GetConnectionAID(connection);
        sTransInfo->pingCallback(latency, aid);
    }
}

void DWCi_TransportProcess(void)
{
    u8* aidList;
    s32 hostCount;
    s32 i;

    if (!sTransInfo)
    {
        return;
    }

    hostCount = DWC_GetAIDList(&aidList);

    for (i = 0; i < hostCount; i++)
    {
        u8 aid;

        aid = aidList[i];

        if (DWC_IsValidAID(aid))
        {
            DWCTransportConnection* transConnection;
            transConnection = DWCs_GetTransConnection(aid);

            if (sTransInfo->recvTimeoutCallback
                && transConnection->recvTimeoutTime > 0)
            {
                u32 time;
                DWCTick currentTick;

                currentTick = DWCi_Np_GetTick();
                time = (u32)DWCi_Np_TicksToMilliSeconds(
                    currentTick - transConnection->previousRecvTick);
                if (time > transConnection->recvTimeoutTime)
                {
                    DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                        "DWCi_TransportProcess:timeout aid=%d,time=%d[ms],timeout time=%d[ms]\n",
                        aid, time, transConnection->recvTimeoutTime);
                    sTransInfo->recvTimeoutCallback(aid);
                    transConnection->previousRecvTick = currentTick;
                }
            }
        }

        if (aid != DWC_GetMyAID()
            && DWCs_GetSendState(aid) == DWC_TRANSPORT_SEND_BUSY)
        {
            s32 restSize;
            s32 sendSize;
            s32 freeSpace;
            DWCTransportConnection* transConnection;

            transConnection = DWCs_GetTransConnection(aid);
            restSize = transConnection->requestSendSize
                - transConnection->sendingSize;

            if (restSize > sTransInfo->sendSplitMax)
            {
                sendSize = sTransInfo->sendSplitMax;
            }
            else
            {
                sendSize = restSize;
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
                    sTransInfo->sendCallback(reqSendSize, aid);
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
    if (sTransInfo->connections[aid].recvState)
    {
        sTransInfo->connections[aid].recvState = 1;
    }
    sTransInfo->connections[aid].lastRecvType = 0;
}

void DWCi_ShutdownTransport(void)
{
    sTransInfo = NULL;
}

static void DWCs_HandleReliableMessage(
    GT2Connection connection, u8* message, int size)
{
    u8 aid = DWCi_GetConnectionAID(connection);
    u16 type;

    switch (DWCs_GetRecvState(aid))
    {
    case DWC_TRANSPORT_RECV_NOBUF:
        type = DWCs_DecodeHeader((DWCTransportHeader*)message);
        if (type >= DWC_SEND_TYPE_MATCH_SYN
            && type <= DWC_SEND_TYPE_MATCH_ACK)
        {
            DWCs_RecvDataHeader(aid, (DWCTransportHeader*)message, size);
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
                "+++ Recv buffer is not set\n");
        }
        break;
    case DWC_TRANSPORT_RECV_HEADER:
        DWCs_RecvDataHeader(aid, (DWCTransportHeader*)message, size);
        break;
    case DWC_TRANSPORT_RECV_BODY:
        DWCs_RecvDataBody(aid, message, size);
        break;
    case DWC_TRANSPORT_RECV_SYSTEM_DATA:
        DWCs_RecvSystemDataBody(aid, message, size);
        break;
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

    case DWC_SEND_TYPE_MATCH_SYN:
    case DWC_SEND_TYPE_MATCH_SYN_ACK:
    case DWC_SEND_TYPE_MATCH_ACK:
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO, "Received system header.\n");
        connection->recvState = DWC_TRANSPORT_RECV_SYSTEM_DATA;
        break;

    default:
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO,
            "+++ Invalid header from aid %d\n", aid);
        break;
    }

    connection->lastRecvType = type;
}

static void DWCs_RecvDataBody(u8 aid, void* message, int size)
{
    DWCTransportConnection* connection;
    int requestSize;

    connection = &sTransInfo->connections[aid];

    if (DWCs_GetRecvState(aid) == DWC_TRANSPORT_RECV_BODY)
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
            sTransInfo->recvCallback(aid, connection->recvBuffer, requestSize);
        }
    }

    if (sTransInfo->recvTimeoutCallback && connection->recvTimeoutTime > 0)
    {
        connection->previousRecvTick = DWCi_Np_GetTick();
    }
}

static void DWCs_RecvSystemDataBody(u8 aid, void* message, int size)
{
#pragma unused(size)
    DWCTransportConnection* transConnection = DWCs_GetTransConnection(aid);

    transConnection->recvState = transConnection->lastRecvState;

    switch (transConnection->lastRecvType)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
    case DWC_SEND_TYPE_MATCH_SYN_ACK:
    case DWC_SEND_TYPE_MATCH_ACK:
        DWCi_ProcessMatchSynPacket(
            aid, transConnection->lastRecvType, (u8*)message);
        break;
    }
}
