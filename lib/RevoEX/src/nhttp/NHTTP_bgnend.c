#include <private/nhttp.h>

#include <revolution/ncd.h>
#include <revolution/os.h>
#include <revolution/so.h>

#include <stdio.h>

void NHTTPi_InitBgnEndInfo(NHTTPBgnEndInfo* info)
{
    info->sslError = 0;
    info->error = NHTTP_ERROR_NONE;
    info->alloc = NULL;
    info->free = NULL;
    info->started = FALSE;
    info->socket = -1;
    info->_unk7DC = 0;
    info->threadStack = NULL;
    info->sslCallback = NULL;
}

void* NHTTPi_alloc(u32 size, int align)
{
    NHTTPBgnEndInfo* info = (NHTTPBgnEndInfo*)NHTTPi_GetSystemInfoP();

    if (info->alloc != NULL)
    {
        return info->alloc(size, align);
    }
    return NULL;
}

void NHTTPi_free(void* buf)
{
    NHTTPBgnEndInfo* info = (NHTTPBgnEndInfo*)NHTTPi_GetSystemInfoP();

    if (info->free != NULL)
    {
        info->free(buf);
    }
}

void NHTTPi_SetError(NHTTPBgnEndInfo* info, NHTTPErr error)
{
    info->error = error;
}

void NHTTPi_SetSSLError(NHTTPBgnEndInfo* info, s32 error)
{
    info->sslError = error;
}

s32 NHTTPi_GetSSLError(NHTTPBgnEndInfo* info)
{
    return info->sslError;
}

BOOL NHTTPi_Startup(void* systemInfo, NHTTPAlloc alloc, NHTTPFree free,
    u32 priority)
{
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(systemInfo);
    void* listInfo = NHTTPi_GetListInfoP(systemInfo);
    void* reqInfo = NHTTPi_GetReqInfoP(systemInfo);
    void* mutexInfo = NHTTPi_GetMutexInfoP(systemInfo);
    void* threadInfo = NHTTPi_GetThreadInfoP(systemInfo);
    void* stack;
    NCDErr result;

    bgnEndInfo->alloc = alloc;
    bgnEndInfo->free = free;
    bgnEndInfo->error = NHTTP_ERROR_NONE;
    bgnEndInfo->sslError = 0;
    bgnEndInfo->_unk7DC = 0;

    NHTTPi_InitListInfo(listInfo);
    NHTTPi_InitRequestInfo(reqInfo);
    NHTTPi_initLockReqList(mutexInfo);
    NHTTPi_InitConnectionList();
    bgnEndInfo->socket = -1;

    stack = NHTTPi_alloc(0x2000, 8);
    bgnEndInfo->threadStack = stack;
    if (stack == NULL)
    {
        bgnEndInfo->error = NHTTP_ERROR_ALLOC;
        NHTTPi_exitLockReqList();
        return FALSE;
    }

    if (!NHTTPi_createCommThread(threadInfo, priority, stack))
    {
        bgnEndInfo->error = NHTTP_ERROR_REVOLUTIONSDK;
        NHTTPi_free(bgnEndInfo->threadStack);
        bgnEndInfo->threadStack = NULL;
        NHTTPi_exitLockReqList();
        return FALSE;
    }

    result = NCDGetCurrentIpConfig(&bgnEndInfo->ipConfig);
    if (result < 0)
    {
        OSReport("NCDGetCurrentIpConfig err = %d\n", result);
        OSPanic("NHTTP_bgnend.c", 230, "NCDGetCurrentIpConfig");
    }

    bgnEndInfo->started = TRUE;
    return TRUE;
}

void NHTTPi_CleanupAsync(void* systemInfo, NHTTPCleanupCallback callback)
{
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(systemInfo);
    void* threadInfo = NHTTPi_GetThreadInfoP(systemInfo);
    int connectionCount;

    NHTTPi_CheckCurrentThread(threadInfo, TRUE);
    NHTTPi_cancelAllRequests(systemInfo);
    NHTTPi_destroyCommThread(threadInfo, bgnEndInfo);
    NHTTPi_free(bgnEndInfo->threadStack);
    bgnEndInfo->threadStack = NULL;
    NHTTPi_exitLockReqList();
    bgnEndInfo->started = FALSE;

    if (callback != NULL)
    {
        callback();
    }

    connectionCount = NHTTPi_GetConnectionListLength();
    if (connectionCount != 0)
    {
        printf("*warning: %d connections rests! Please free connections.\n",
            connectionCount);
    }

    if (bgnEndInfo->socket >= 0)
    {
        SOClose(bgnEndInfo->socket);
        bgnEndInfo->socket = -1;
    }
}
