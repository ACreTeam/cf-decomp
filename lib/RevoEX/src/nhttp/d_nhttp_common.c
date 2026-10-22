#include <private/nhttp.h>

#include <stdio.h>

struct NHTTPSysInfo
{
    u8 bgnEndInfo[0x800];
    NHTTPListInfo listInfo;
    NHTTPReqInfo reqInfo;
    u8 mutexInfo[0x34];
    u8 threadInfo[0x460];
};

static NHTTPConnectionInfo* sConnectionList = NULL;
static struct NHTTPCompletionInfo
{
    BOOL initialized;
    OSMutex mutex;
    OSCond cond;
} NHTTPiStaticResource;
static NHTTPSysInfo sysInfo ALIGN32;
static NHTTPSysInfo* sysInfo_p = NULL;

void NHTTPi_WaitForCompletion(NHTTPConnectionInfo* connection)
{
    struct NHTTPCompletionInfo* info = &NHTTPiStaticResource;

    if (!info->initialized)
    {
        OSInitMutex(&info->mutex);
        OSInitCond(&info->cond);
        info->initialized = TRUE;
    }
    OSLockMutex(&info->mutex);
    while (connection->inProgress)
    {
        OSWaitCond(&info->cond, &info->mutex);
    }
    OSUnlockMutex(&info->mutex);
}

void NHTTPi_NotifyCompletion(NHTTPConnectionInfo* connection)
{
    struct NHTTPCompletionInfo* info = &NHTTPiStaticResource;

    if (!info->initialized)
    {
        OSInitMutex(&info->mutex);
        OSInitCond(&info->cond);
        info->initialized = TRUE;
    }
    OSLockMutex(&info->mutex);
    connection->inProgress = FALSE;
    OSSignalCond(&info->cond);
    OSUnlockMutex(&info->mutex);
}

NHTTPConnectionInfo* NHTTPi_ControlConnectionList(void* mutexInfo,
    void* handle, u32 mode)
{
    NHTTPConnectionInfo* connection;
    NHTTPConnectionInfo* result;
    NHTTPConnectionInfo** current;

    result = NULL;
    NHTTPi_lockReqList(mutexInfo);
    if (mode == 3)
    {
        connection = handle;
        connection->next = sConnectionList;
        sConnectionList = connection;
        result = connection;
    }
    else
    {
        current = &sConnectionList;
        while (*current != NULL)
        {
            connection = *current;
            switch (mode)
            {
            case 0:
                if (connection == handle)
                {
                    result = connection;
                }
                break;
            case 1:
                if (connection->request == handle)
                {
                    result = connection;
                }
                break;
            case 2:
                if (connection->response == handle)
                {
                    result = connection;
                }
                break;
            case 4:
                if (connection == handle)
                {
                    *current = connection->next;
                    result = connection;
                }
                break;
            }

            if (result != NULL)
            {
                break;
            }
            current = &(*current)->next;
        }
    }
    NHTTPi_unlockReqList(mutexInfo);
    return result;
}

static inline s32 NHTTPi_ConnectionListStatus(
    const NHTTPConnectionInfo* connection,
    const NHTTPConnectionInfo* invalidConnection)
{
    return connection != invalidConnection ? 0 : -1;
}

s32 NHTTPi_CommitConnectionList(void* mutexInfo_p,
    NHTTPConnectionInfo* connection_p)
{
    return NHTTPi_ConnectionListStatus(
        NHTTPi_ControlConnectionList(mutexInfo_p, connection_p, 3), NULL);
}

s32 NHTTPi_OmitConnectionList(void* mutexInfo_p,
    NHTTPConnectionInfo* connection_p)
{
    return NHTTPi_ConnectionListStatus(
        NHTTPi_ControlConnectionList(mutexInfo_p, connection_p, 4), NULL);
}

NHTTPRequestInfo* NHTTPi_Connection2Request(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    connection = NHTTPi_ControlConnectionList(mutexInfo, connection, 0);
    return connection != NULL ? connection->request : NULL;
}

NHTTPResponseInfo* NHTTPi_Connection2Response(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    connection = NHTTPi_ControlConnectionList(mutexInfo, connection, 0);
    return connection != NULL ? connection->response : NULL;
}

NHTTPConnectionInfo* NHTTPi_Request2Connection(void* mutexInfo,
    NHTTPRequestInfo* request)
{
    return NHTTPi_ControlConnectionList(
        mutexInfo, request, 1);
}

NHTTPConnectionInfo* NHTTPi_Response2Connection(void* mutexInfo,
    NHTTPResponseInfo* response)
{
    return NHTTPi_ControlConnectionList(
        mutexInfo, response, 2);
}

NHTTPConnectionInfo* NHTTPi_GetConnection(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    return NHTTPi_ControlConnectionList(mutexInfo, connection, 0);
}

NHTTPRequestInfo* NHTTPi_GetRequest(void* mutexInfo,
    NHTTPRequestInfo* request)
{
    NHTTPConnectionInfo* connection;

    connection = NHTTPi_ControlConnectionList(
        mutexInfo, request, 0);
    return connection != NULL ? connection->request : request;
}

NHTTPResponseInfo* NHTTPi_GetResponse(void* mutexInfo,
    NHTTPResponseInfo* response)
{
    NHTTPConnectionInfo* connection;

    connection = NHTTPi_ControlConnectionList(
        mutexInfo, response, 0);
    return connection != NULL ? connection->response : response;
}

s32 NHTTPi_GetConnectionListLength(void)
{
    NHTTPConnectionInfo* connection;
    s32 length;

    connection = sConnectionList;
    length = 0;
    while (connection != NULL)
    {
        connection = connection->next;
        length++;
    }
    return length;
}

s32 NHTTPi_PostSendCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection, const char* label, u32 arg)
{
    u32 postSendBuf;
    u32 postSendSize;
    s32 result;
    NHTTPResponseInfo* response;
    NHTTPPostSendArg param;

    result = -1;
    if (NHTTPi_GetConnection(mutexInfo, connection) != NULL)
    {
        response = NHTTPi_Connection2Response(mutexInfo, connection);
        if (response != NULL && connection->callback != NULL)
        {
            param.label = label;
            param.buf = (char*)connection->postSendBuf;
            param.size = connection->postSendSize;
            param.offset = arg;
            result = connection->callback(connection, 1, &param);
            postSendSize = param.size;
            postSendBuf = (u32)param.buf;

            connection = NHTTPi_GetConnection(mutexInfo, connection);
            if (connection != NULL)
            {
                connection->postSendBuf = postSendBuf;
                connection->postSendSize = postSendSize;
            }
        }
    }
    return result;
}

void NHTTPi_BufferFullCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    void* buffer;
    u32 bufferSize;
    s32 bodyLen;
    NHTTPResponseInfo* response;
    NHTTPConnectionCallbackParam param;

    if (NHTTPi_GetConnection(mutexInfo, connection) != NULL)
    {
        response = NHTTPi_Connection2Response(mutexInfo, connection);
        if (response != NULL && connection->callback != NULL)
        {
            param.value = response->recvBuf_p;
            param._unk4 = response->recvBufLen;
            param._unk8 = response->bodyLen;
            connection->callback(connection, 2, &param);
            bodyLen = param._unk8;
            bufferSize = param._unk4;
            buffer = param.value;

            connection = NHTTPi_GetConnection(mutexInfo, connection);
            if (connection != NULL)
            {
                response = NHTTPi_Connection2Response(mutexInfo, connection);
                if (response != NULL)
                {
                    response->recvBuf_p = buffer;
                    response->recvBufLen = bufferSize;
                    response->bodyLen = bodyLen;
                }
            }
        }
    }
}

void NHTTPi_ReceivedCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    void* buffer;
    u32 bufferSize;
    s32 bodyLen;
    NHTTPResponseInfo* response;
    NHTTPConnectionCallbackParam param;

    if (NHTTPi_GetConnection(mutexInfo, connection) != NULL)
    {
        response = NHTTPi_Connection2Response(mutexInfo, connection);
        if (response != NULL && connection->callback != NULL)
        {
            param.value = response->recvBuf_p;
            param._unk4 = response->recvBufLen;
            param._unk8 = response->bodyLen;
            connection->callback(connection, 3, &param);
            bodyLen = param._unk8;
            bufferSize = param._unk4;
            buffer = param.value;

            connection = NHTTPi_GetConnection(mutexInfo, connection);
            if (connection != NULL)
            {
                response = NHTTPi_Connection2Response(mutexInfo, connection);
                if (response != NULL)
                {
                    response->recvBuf_p = buffer;
                    response->recvBufLen = bufferSize;
                    response->bodyLen = bodyLen;
                }
            }
        }
    }
}

void NHTTPi_CompleteCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection)
{
    if (NHTTPi_GetConnection(mutexInfo, connection) != NULL
        && connection->callback != NULL)
    {
        connection->callback(connection, 4, NULL);
    }
}

static void NHTTPi_InitSystemInfo(NHTTPSysInfo* sysInfo_p)
{
    NHTTPi_InitBgnEndInfo(NHTTPi_GetBgnEndInfoP(sysInfo_p));
    NHTTPi_InitListInfo(NHTTPi_GetListInfoP(sysInfo_p));
    NHTTPi_InitRequestInfo(NHTTPi_GetReqInfoP(sysInfo_p));
    NHTTPi_InitMutexInfo(NHTTPi_GetMutexInfoP(sysInfo_p));
    NHTTPi_InitThreadInfo(NHTTPi_GetThreadInfoP(sysInfo_p));
}

NHTTPSysInfo* NHTTPi_GetSystemInfoP(void)
{

    if (sysInfo_p == NULL)
    {
        sysInfo_p = &sysInfo;
        NHTTPi_InitSystemInfo(sysInfo_p);
    }
    return sysInfo_p;
}

NHTTPBgnEndInfo* NHTTPi_GetBgnEndInfoP(NHTTPSysInfo* sysInfo_p)
{
    return (NHTTPBgnEndInfo*)sysInfo_p->bgnEndInfo;
}

NHTTPListInfo* NHTTPi_GetListInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &sysInfo_p->listInfo;
}

NHTTPReqInfo* NHTTPi_GetReqInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &sysInfo_p->reqInfo;
}

void* NHTTPi_GetThreadInfoP(NHTTPSysInfo* sysInfo_p)
{
    return sysInfo_p->threadInfo;
}

void* NHTTPi_GetMutexInfoP(NHTTPSysInfo* sysInfo_p)
{
    return sysInfo_p->mutexInfo;
}

void NHTTPi_SetVirtualContentLength(NHTTPConnectionInfo* connection,
    u32 length)
{
    if (connection != NULL)
    {
        connection->virtualContentLength = length;
    }
}

u32 NHTTPi_GetVirtualContentLength(NHTTPConnectionInfo* connection)
{
    return connection != NULL ? connection->virtualContentLength : 0;
}

void NHTTPi_InitConnectionList(void)
{
    if (sConnectionList != NULL)
    {
        s32 connectionCount = NHTTPi_GetConnectionListLength();
        if (connectionCount != 0)
        {
            printf("*warning: %d connections rests! Please free connections.\n",
                connectionCount);
        }
    }
    sConnectionList = NULL;
}
