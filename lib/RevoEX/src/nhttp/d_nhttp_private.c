#include <private/nhttp.h>

NHTTPConnectionInfo* __NHTTPCreateConnection(const char* url_p, s32 method,
    void* buf_p, u32 len, NHTTPConnectionCallback callback,
    void* userParam_p)
{
    NHTTPConnectionInfo* connection_p = NULL;
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);

    connection_p = NHTTPi_alloc(sizeof(NHTTPConnectionInfo), 32);
    if (connection_p == NULL)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        return NULL;
    }

    NHTTPi_memclr(connection_p, sizeof(NHTTPConnectionInfo));
    connection_p->request = NHTTP_CreateRequest(bgnEndInfo_p, url_p, method,
        buf_p, len, userParam_p, NULL, NULL);
    if (connection_p->request == NULL)
    {
        NHTTPi_free(connection_p);
        return NULL;
    }

    connection_p->response = connection_p->request->response;
    connection_p->started = FALSE;
    connection_p->callback = callback;
    connection_p->postSendBuf = 0;
    connection_p->postSendSize = 0;
    connection_p->requestId = -1;
    NHTTPi_CommitConnectionList(mutexInfo_p, connection_p);
    connection_p->state = NHTTP_ERROR_BUSY;
    connection_p->sslError = 0;
    connection_p->inProgress = TRUE;
    NHTTPi_SetVirtualContentLength(connection_p, 0);
    connection_p->requestCallback = NULL;
    connection_p->recvBufferOffset = 0;
    connection_p->recvBufferRemaining = 0;
    NHTTPi_memclr(connection_p->recvBuffer, sizeof(connection_p->recvBuffer));
    return connection_p;
}

static inline s32 NHTTPi_GetStartedState(NHTTPConnectionInfo* handle)
{
    NHTTPConnectionInfo* connection_p = NHTTPi_GetConnection(
        NHTTPi_GetMutexInfoP(NHTTPi_GetSystemInfoP()), handle);
    return connection_p != NULL ? connection_p->started : -1;
}

static inline s32 NHTTPi_GetConnState(NHTTPConnectionInfo* handle)
{
    NHTTPConnectionInfo* connection_p = NHTTPi_GetConnection(
        NHTTPi_GetMutexInfoP(NHTTPi_GetSystemInfoP()), handle);
    return connection_p != NULL ? connection_p->state : -1;
}

static inline void NHTTPi_WaitForConnection(NHTTPConnectionInfo* handle)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    void* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPConnectionInfo* connection_p = NHTTPi_GetConnection(
        NHTTPi_GetMutexInfoP(sysInfo_p), handle);

    if (connection_p != NULL)
    {
        s32 started = NHTTPi_GetStartedState(handle);

        NHTTPi_CheckCurrentThread(threadInfo_p, 1);
        if (started != -1 && started != 0)
        {
            if (NHTTPi_GetConnState(handle) == NHTTP_ERROR_BUSY)
            {
                NHTTPi_WaitForCompletion(connection_p);
            }
        }
    }
}

s32 NHTTPDeleteConnection(NHTTPConnectionInfo* handle)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    void* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPConnectionInfo* connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

    if (connection_p == NULL)
    {
        return -1;
    }

    NHTTPi_CheckCurrentThread(threadInfo_p, 1);

    if (connection_p->requestId >= 0)
    {
        NHTTP_CancelRequestAsync(sysInfo_p, connection_p->requestId);
        connection_p->requestId = -1;
    }

    if (connection_p->request != NULL && connection_p->request->state == 0)
    {
        NHTTP_DestroyRequest(sysInfo_p, connection_p->request);
    }

    if (connection_p->response != NULL)
    {
        if (connection_p->request == NULL)
        {
            NHTTP_DestroyResponse(mutexInfo_p, connection_p->response);
        }
        else
        {
            NHTTPi_WaitForConnection(handle);
            NHTTP_DestroyResponse(mutexInfo_p, connection_p->response);
        }
    }

    NHTTPi_OmitConnectionList(mutexInfo_p, connection_p);
    NHTTPi_free(connection_p);
    return 0;
}

s32 NHTTPStartConnection(NHTTPConnectionInfo* handle)
{
    void* systemInfo;
    void* mutexInfo;
    NHTTPConnectionInfo* connection;
    s32 requestId;

    systemInfo = NHTTPi_GetSystemInfoP();
    mutexInfo = NHTTPi_GetMutexInfoP(systemInfo);
    connection = NHTTPi_GetConnection(mutexInfo, handle);
    if (connection == NULL)
    {
        return -1;
    }
    if (connection->request == NULL)
    {
        return -1;
    }

    requestId = NHTTP_SendRequestAsync(systemInfo, connection->request);
    connection->requestId = requestId;
    if (requestId >= 0)
    {
        connection->started = TRUE;
    }
    return 0;
}

s32 NHTTPGetBodyBuffer(NHTTPConnectionInfo* handle, void** buffer,
    u32* bufferSize)
{
    void* mutexInfo;
    NHTTPConnectionInfo* connection;
    NHTTPResponseInfo* response;
    s32 result;

    mutexInfo = NHTTPi_GetMutexInfoP(NHTTPi_GetSystemInfoP());
    connection = NHTTPi_GetConnection(mutexInfo, handle);
    if (connection != NULL)
    {
        response = NHTTPi_Connection2Response(mutexInfo, connection);
        if (response != NULL)
        {
            *buffer = response->recvBuf_p;
            *bufferSize = response->recvBufLen;
            result = response->bodyLen;
        }
        else
        {
            result = -1;
        }
    }
    else
    {
        result = -1;
    }

    return result;
}

void* NHTTPGetUserParam(NHTTPConnectionInfo* handle)
{
    void* mutexInfo;
    NHTTPConnectionInfo* connection;
    NHTTPResponseInfo* response;
    void* result;

    mutexInfo = NHTTPi_GetMutexInfoP(NHTTPi_GetSystemInfoP());
    connection = NHTTPi_GetConnection(mutexInfo, handle);
    if (connection != NULL)
    {
        response = NHTTPi_Connection2Response(mutexInfo, connection);
        if (response != NULL)
        {
            result = response->param_p;
        }
        else
        {
            result = NULL;
        }
    }
    else
    {
        result = NULL;
    }
    return result;
}

s32 NHTTPGetConnectionError(NHTTPConnectionInfo* handle)
{
    void* mutexInfo;
    NHTTPConnectionInfo* connection;
    s32 result;

    mutexInfo = NHTTPi_GetMutexInfoP(NHTTPi_GetSystemInfoP());
    connection = NHTTPi_GetConnection(mutexInfo, handle);
    if (connection != NULL)
    {
        result = connection->state;
    }
    else
    {
        result = -1;
    }
    return result;
}
