#include <private/nhttp.h>

void NHTTPi_InitListInfo(NHTTPListInfo* info)
{
    info->reqQueue = NULL;
    info->nextRequestId = 0;
}

s32 NHTTPi_setReqQueue(NHTTPListInfo* info, NHTTPRequestInfo* request)
{
    s32 requestId = -1;
    NHTTPReqQueue* queue = NHTTPi_alloc(sizeof(NHTTPReqQueue), 4);

    if (queue != NULL)
    {
        if (info->reqQueue != NULL)
        {
            queue->next = info->reqQueue->next;
            queue->prev = info->reqQueue;
            info->reqQueue->next->prev = queue;
            info->reqQueue->next = queue;
        }
        else
        {
            queue->next = queue;
            queue->prev = queue;
            info->reqQueue = queue;
        }

        queue->requestId = info->nextRequestId++;
        queue->request = request;
        queue->_unk10 = -1;
        requestId = queue->requestId;

        if (info->nextRequestId < 0)
        {
            info->nextRequestId = 0;
        }
    }

    return requestId;
}

BOOL NHTTPi_freeReqQueue(NHTTPListInfo* info, void* mutexInfo, s32 requestId)
{
    NHTTPReqQueue* current;
    NHTTPReqQueue* queue = info->reqQueue;
    NHTTPReqQueue* found = NULL;
    BOOL result = FALSE;

    if (queue != NULL)
    {
        if (queue->requestId == requestId)
        {
            found = queue;
        }
        else
        {
            current = queue->prev;

            while (current != queue)
            {
                if (current->requestId == requestId)
                {
                    found = current;
                    break;
                }
                current = current->prev;
            }
        }
    }

    if (found != NULL)
    {
        NHTTPConnectionInfo* connection;

        if (queue != queue->next)
        {
            found->next->prev = found->prev;
            found->prev->next = found->next;

            if (info->reqQueue == found)
            {
                info->reqQueue = found->prev;
            }
        }
        else
        {
            info->reqQueue = NULL;
        }

        connection = NHTTPi_Request2Connection(mutexInfo, found->request);
        NHTTPi_destroyRequestObject(mutexInfo, found->request);
        NHTTPi_free(found);

        if (connection != NULL)
        {
            connection->state = 8;
            NHTTPi_CompleteCallback(mutexInfo, connection);
        }
        result = TRUE;
    }

    return result;
}

void NHTTPi_allFreeReqQueue(NHTTPListInfo* info, void* mutexInfo)
{
    while (info->reqQueue != NULL)
    {
        NHTTPi_freeReqQueue(info, mutexInfo, info->reqQueue->requestId);
    }
}

NHTTPReqQueue* NHTTPi_getReqFromReqQueue(NHTTPReqQueue** queue)
{
    return (NHTTPReqQueue*)NHTTPi_getHdrFromList((NHTTPHeader**)queue);
}
