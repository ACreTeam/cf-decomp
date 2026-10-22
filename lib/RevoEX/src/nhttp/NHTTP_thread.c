#include <private/nhttp.h>

#include <revolution/ncd.h>

static int NHTTPi_SendHeaderList(NHTTPThreadData* threadData_p);

void NHTTPi_InitThreadInfo(NHTTPThreadInfo* info)
{
    info->isCreateCommThreadMessageQueue = FALSE;
}

void NHTTPi_IsCreateCommThreadMessageQueueOn(NHTTPThreadInfo* info)
{
    info->isCreateCommThreadMessageQueue = TRUE;
}

BOOL NHTTPi_IsCreateCommThreadMessageQueue(NHTTPThreadInfo* info)
{
    return info->isCreateCommThreadMessageQueue;
}

s32 NHTTPi_CheckHeaderEnd(char* buffer, s32 position)
{
    if (buffer[(position - 2) & 3] == '\r' && buffer[(position - 1) & 3] == '\r')
    {
        return 1;
    }
    if (buffer[(position - 2) & 3] == '\n' && buffer[(position - 1) & 3] == '\n')
    {
        return 1;
    }
    if (buffer[(position - 4) & 3] == '\r' && buffer[(position - 3) & 3] == '\n'
        && buffer[(position - 2) & 3] == '\r'
        && buffer[(position - 1) & 3] == '\n')
    {
        return 1;
    }
    return 0;
}

s32 NHTTPi_SaveBuf(NHTTPRequestInfo* request, char* buffer, s32 socket,
    s32* bufferedBytes, const char* data, s32 length)
{
    s32 chunk;
    s32 room;
    s32 remain = length;
    while (remain > 0)
    {
        if (request->cancel)
        {
            return -1;
        }
        chunk = remain;
        room = 0x100 - *bufferedBytes;
        if (remain > room)
        {
            chunk = room;
        }
        NHTTPi_memcpy(buffer + *bufferedBytes, data, chunk);
        data += chunk;
        remain -= chunk;
        *bufferedBytes += chunk;
        if (*bufferedBytes == 0x100)
        {
            s32 sent = NHTTPi_SocSend(request, socket, buffer, 0x100, 0);
            if (sent <= 0)
            {
                return sent;
            }
            *bufferedBytes -= sent;
        }
    }
    return length;
}

static inline s32 NHTTPi_SkipLineBuf(
    void* mutexInfo_p,
    NHTTPRequestInfo* req_p, s32 socket)
{
    char buf[2];
    s32 p;
    s32 recvbyte;
    s32 rc = 0;

    buf[0] = 0;
    buf[1] = 0;
    for (p = 0;
         !(buf[p & 1] == '\r' && buf[(p - 1) & 1] == '\n'); p++)
    {
        recvbyte = NHTTPi_SocRecv(
            mutexInfo_p,
            req_p, socket, &buf[p & 1], 1, 0);
        if (recvbyte <= 0)
        {
            return recvbyte;
        }
        rc += recvbyte;
    }
    return rc;
}

BOOL NHTTPi_GetPostContentlength(void* mutexInfo, NHTTPRequestInfo* request,
    const char* label, s32* contentLength, s32 format)
{
    u32 total = 0;
    NHTTPConnectionInfo* connection =
        NHTTPi_Request2Connection(mutexInfo, request);
    if (connection == NULL)
    {
        return FALSE;
    }
    connection->postSendBuf = 0;
    for (;;)
    {
        if (request->cancel)
        {
            return FALSE;
        }
        connection->postSendSize = 0;
        if (NHTTPi_PostSendCallback(mutexInfo, connection, label, total) < 0)
        {
            return FALSE;
        }
        {
            u32 size = connection->postSendSize;
            void* data = (void*)connection->postSendBuf;
            if (size == 0)
            {
                break;
            }
            if (data == NULL)
            {
                return FALSE;
            }
            total += size;
            switch (format)
            {
            case 0:
            case 1:
                *contentLength += size;
                break;
            case 2:
                *contentLength += NHTTPi_getUrlEncodedSize2(data, size);
                break;
            default:
                break;
            }
        }
    }
    return TRUE;
}

s32 NHTTPi_SendPostData(void* mutexInfo, NHTTPRequestInfo* request,
    char* buffer, const char* label, s32 socket, s32* bufferedBytes, s32 format)
{
    char* data;
    u32 total = 0;
    u32 size;
    u32 i;
    NHTTPConnectionInfo* connection =
        NHTTPi_Request2Connection(mutexInfo, request);
    if (connection == NULL)
    {
        return 3;
    }
    connection->postSendBuf = 0;
    for (;;)
    {
        if (request->cancel)
        {
            return 3;
        }
        connection->postSendSize = 0;
        if (NHTTPi_PostSendCallback(mutexInfo, connection, label, total) < 0)
        {
            return 3;
        }
        {
            size = connection->postSendSize;
            data = (char*)connection->postSendBuf;
            if (size == 0)
            {
                break;
            }
            if (data == NULL)
            {
                return 3;
            }
            total += size;
            switch (format)
            {
            case 0:
            case 1:
            {
                s32 result = NHTTPi_SaveBuf(request, buffer, socket,
                    bufferedBytes, data, size);
                if (result < 0)
                {
                    return 1;
                }
                if (result == 0)
                {
                    return 2;
                }
                break;
            }
            case 2:
            {
                for (i = 0; i < size; i++)
                {
                    char encoded[3];
                    s32 result;
                    NHTTPi_memclr(encoded, 3);
                    result = NHTTPi_SaveBuf(request, buffer, socket,
                        bufferedBytes, encoded,
                        NHTTPi_encodeUrlChar(encoded, data[i]));
                    if (result < 0)
                    {
                        return 1;
                    }
                    if (result == 0)
                    {
                        return 2;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return 0;
}

BOOL NHTTPi_BufFull(void* mutexInfo, NHTTPResponseInfo* response)
{
    BOOL result = FALSE;
    BOOL full = NHTTPi_isRecvBufFull(response, response->bodyLen);
    if (response->recvBufLen == 0 || response->recvBuf_p == NULL
        || full)
    {
        NHTTPConnectionInfo* connection =
            NHTTPi_Response2Connection(mutexInfo, response);
        if (connection != NULL)
        {
            NHTTPi_BufferFullCallback(mutexInfo, connection);
            if (response->recvBuf_p != NULL && response->recvBufLen != 0)
            {
                if (!NHTTPi_isRecvBufFull(response, response->bodyLen))
                {
                    result = TRUE;
                }
            }
        }
    }
    else if (!full)
    {
        result = TRUE;
    }
    return result;
}

static int NHTTPi_SendData(NHTTPThreadData* threadData_p,
    const char* src_p, int srclen)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    char* commBuf_p = &threadInfo_p->commBuf[0];
    int stat = 0;

    if (srclen == 0)
    {
        return 0;
    }
    stat = NHTTPi_SaveBuf(req_p, commBuf_p, bgnEndInfo_p->socket,
        &threadData_p->sendBufwp, src_p, srclen);
    if (stat < 0)
    {
        return 1;
    }
    else if (stat == 0)
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

static inline s32 NHTTPi_SendProxyAuthorization(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    s32 sendStatus;

    if (req_p->proxyAuthorizationLength == 0)
    {
        return 0;
    }

    sendStatus = NHTTPi_SendData(threadData_p,
        "Proxy-Authorization: Basic ", 27);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        req_p->proxyAuthorization, req_p->proxyAuthorizationLength);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    return 0;
}

static inline s32 NHTTPi_SendBasicAuthorization(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    s32 sendStatus;

    if (req_p->authorizationLength == 0)
    {
        return 0;
    }

    sendStatus = NHTTPi_SendData(threadData_p, "Authorization: Basic ", 21);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        req_p->authorization, req_p->authorizationLength);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    return 0;
}

s32 NHTTPi_SendProxyConnectMethod(NHTTPThreadData* threadData_p)
{
    char portString[8];
    s32 err;
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPRequestInfo* request =
        NHTTPi_GetReqInfoP(system)->reqQueue->request;
    NHTTPThreadInfo* threadInfo = NHTTPi_GetThreadInfoP(system);
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    char* buffer = threadInfo->commBuf;
    s32 portLength;
    portLength = NHTTPi_intToStr(portString, request->port);

    err = NHTTPi_SendData(threadData_p, "CONNECT ", 8);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, request->url + 8, request->hostEnd - 8);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, ":", 1);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, portString, portLength);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, " HTTP/1.1\r\n", 11);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, "Host: ", 6);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, request->url + 8, request->hostEnd - 8);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, ":", 1);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, portString, portLength);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p,
        "Content-Length: 0\r\nPragma: no-cache\r\n", 37);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendProxyAuthorization(threadData_p);
    if (err != 0)
    {
        return err;
    }
    err = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (threadData_p->sendBufwp > 0)
    {
        s32 sent = NHTTPi_SocSend(request, bgnEndInfo->socket, buffer,
            threadData_p->sendBufwp, 0);
        if (sent < 0)
        {
            return 1;
        }
        if (sent == 0)
        {
            return 2;
        }
    }
    threadData_p->sendBufwp = 0;
    NHTTPi_memclr(buffer, 0x100);
    return 0;
}

BOOL NHTTPi_RecvProxyConnectHeader(NHTTPThreadData* threadData_p)
{
    char buffer[0x200];
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
    NHTTPThreadInfo* threadInfo = NHTTPi_GetThreadInfoP(system);
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    void* mutexInfo = NHTTPi_GetMutexInfoP(system);
    char* sendBuffer = threadInfo->commBuf;
    NHTTPRequestInfo* request = reqInfo->reqQueue->request;
    NHTTPResponseInfo* response = request->response;
    BOOL ok = FALSE;
    s32 received = 0;
    for (;;)
    {
        s32 result = NHTTPi_SocRecv(
            mutexInfo,
            request, bgnEndInfo->socket,
            buffer + received, 0x200 - received, 0);
        received += result;
        response->httpStatus = NHTTPi_strToInt(buffer + 9, 3);
        if (NHTTPi_strnicmp(buffer, "HTTP/", 5) == 0 && buffer[8] == ' '
            && response->httpStatus == 200)
        {
            ok = TRUE;
        }
        {
            char* p = buffer;
            s32 i = 0;
            BOOL end = FALSE;
            for (; i < received; p++, i++)
            {
                if (i > 1 && p[-1] == '\r' && p[0] == '\r')
                {
                    end = TRUE;
                }
                else if (i > 1 && p[-1] == '\n' && p[0] == '\n')
                {
                    end = TRUE;
                }
                else if (i > 3 && p[-3] == '\r' && p[-2] == '\n'
                    && p[-1] == '\r' && p[0] == '\n')
                {
                    end = TRUE;
                }
            }
            if (end)
            {
                return ok != 0;
            }
        }
        if (result < 0)
        {
            return FALSE;
        }
        if (received < 0x200)
        {
            continue;
        }
        result = NHTTPi_SocRecv(
            mutexInfo,
            request, bgnEndInfo->socket, sendBuffer, 1, 0);
        if (result < 0)
        {
            return FALSE;
        }
        if (result == 0)
        {
            continue;
        }
        return FALSE;
    }
}

static int NHTTPi_SendHeaderList(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPHeader* datalist_p;
    s32 sendStatus;

    for (datalist_p = NHTTPi_getHdrFromList(&req_p->headers); datalist_p;
         datalist_p = NHTTPi_getHdrFromList(&req_p->headers))
    {
        sendStatus = NHTTPi_SendData(threadData_p,
            datalist_p->name, NHTTPi_strlen(datalist_p->name));
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p, ": ", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p,
            datalist_p->value, NHTTPi_strlen(datalist_p->value));
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }

        NHTTPi_free(datalist_p);
    }

    return 0;
}

#define TMP_CONTENT_LENGTH_BUF_SIZE (12)

s32 NHTTPi_SendProcPostDataRaw(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char* commBuf_p = threadInfo_p->commBuf;
    s32 contentLength = 0;
    int contentLengthBufLen = 0;
    char contentLengthBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    int sendStatus;
    if (req_p->postBuffer == NULL)
    {
        if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p, NULL,
                &contentLength, 0))
        {
            return 3;
        }
    }
    else
    {
        contentLength = req_p->postBufferSize;
    }
    contentLengthBufLen = NHTTPi_intToStr(contentLengthBuf, contentLength);
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        contentLengthBuf, contentLengthBufLen);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    if (req_p->postBuffer == NULL)
    {
        s32 result = NHTTPi_SendPostData(mutexInfo_p, req_p, commBuf_p, NULL,
            bgnEndInfo_p->socket, &threadData_p->sendBufwp, 0);
        if (result != 0)
        {
            return result;
        }
    }
    else
    {
        sendStatus = NHTTPi_SendData(threadData_p,
            req_p->postBuffer, req_p->postBufferSize);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    return 0;
}

static const char NHTTPi_strContentDisposition[] =
    "Content-Disposition: form-data; name=\"";
static const char NHTTPi_strContentTypeBinary[] =
    "Content-Type: application/octet-stream\r\n"
    "Content-Transfer-Encoding: binary\r\n";
static const char NHTTPi_strContentTypeAscii[] =
    "Content-Type: application/x-www-form-urlencoded\r\n";
static const char NHTTPi_strContentTypeMultipart[] =
    "Content-Type: multipart/form-data; boundary=";

#define TMP_HEADER_BUF_SIZE (14)

s32 NHTTPi_SendProcPostDataBinary(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char* commBuf_p = threadInfo_p->commBuf;
    s32 contentLength = 0;
    s32 contentLengthBufLen = 0;
    char contentLengthBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    s32 sendStatus;
    NHTTPHeader* datalist_p;
    for (datalist_p = req_p->postData; datalist_p != NULL;)
    {
        contentLength += sizeof(req_p->multipartBoundary) + 2;
        contentLength += (sizeof(NHTTPi_strContentDisposition) - 1) +
            NHTTPi_strlen(datalist_p->name) + 1 + 2;
        if (datalist_p->_unk14 != 0)
        {
            contentLength += sizeof(NHTTPi_strContentTypeBinary) - 1;
        }
        contentLength += 2;
        if (datalist_p->value == NULL)
        {
            if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p,
                    datalist_p->name,
                    &contentLength, 1))
            {
                return 3;
            }
        }
        else
        {
            contentLength += datalist_p->length;
        }
        contentLength += 2;
        if (datalist_p == req_p->postData->next)
        {
            break;
        }
        datalist_p = datalist_p->prev;
    }
    contentLength += sizeof(req_p->multipartBoundary) + 2 + 2;
    contentLengthBufLen = NHTTPi_intToStr(contentLengthBuf, contentLength);
    sendStatus = NHTTPi_SendData(threadData_p,
        NHTTPi_strContentTypeMultipart, sizeof(NHTTPi_strContentTypeMultipart) - 1);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        req_p->multipartBoundary + 2, sizeof(req_p->multipartBoundary) - 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        contentLengthBuf, contentLengthBufLen);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    for (datalist_p = req_p->postData; datalist_p != NULL;)
    {
        sendStatus = NHTTPi_SendData(threadData_p,
            req_p->multipartBoundary, sizeof(req_p->multipartBoundary));
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p,
            NHTTPi_strContentDisposition, sizeof(NHTTPi_strContentDisposition) - 1);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p,
            datalist_p->name, NHTTPi_strlen(datalist_p->name));
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\"\r\n", 3);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        if (datalist_p->_unk14 != 0)
        {
            sendStatus = NHTTPi_SendData(threadData_p,
                NHTTPi_strContentTypeBinary, sizeof(NHTTPi_strContentTypeBinary) - 1);
            if (sendStatus != 0)
            {
                return sendStatus;
            }
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        if (datalist_p->value == NULL)
        {
            s32 result = NHTTPi_SendPostData(mutexInfo_p, req_p, commBuf_p,
                datalist_p->name, bgnEndInfo_p->socket,
                &threadData_p->sendBufwp, 1);
            if (result != 0)
            {
                return result;
            }
        }
        else
        {
            sendStatus = NHTTPi_SendData(threadData_p,
                datalist_p->value, datalist_p->length);
            if (sendStatus != 0)
            {
                return sendStatus;
            }
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        if (datalist_p == req_p->postData->next)
        {
            break;
        }
        datalist_p = datalist_p->prev;
    }
    sendStatus = NHTTPi_SendData(threadData_p,
        req_p->multipartBoundary, sizeof(req_p->multipartBoundary));
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "--\r\n", 4);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    return 0;
}

s32 NHTTPi_SendProcPostDataAscii(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char* commBuf_p = threadInfo_p->commBuf;
    s32 contentLength = 0;
    s32 tmpBufLen = 0;
    char tmpBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    s32 sendStatus;
    NHTTPHeader* datalist_p;
    for (datalist_p = req_p->postData; datalist_p != NULL;)
    {
        contentLength += NHTTPi_getUrlEncodedSize(datalist_p->name);
        contentLength++;
        if (datalist_p->value == NULL)
        {
            if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p,
                    datalist_p->name, &contentLength, 2))
            {
                return 3;
            }
        }
        else
        {
            contentLength += NHTTPi_getUrlEncodedSize(datalist_p->value);
        }
        if (datalist_p == req_p->postData->next)
        {
            break;
        }
        contentLength += 1;
        datalist_p = datalist_p->prev;
    }
    tmpBufLen = NHTTPi_intToStr(tmpBuf, contentLength);
    sendStatus = NHTTPi_SendData(threadData_p,
        NHTTPi_strContentTypeAscii, sizeof(NHTTPi_strContentTypeAscii) - 1);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, tmpBuf, tmpBufLen);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    {
        s32 p;
        for (datalist_p = req_p->postData; datalist_p != NULL;)
        {
            for (p = 0; datalist_p->name[p] != 0; p++)
            {
                tmpBufLen =
                    NHTTPi_encodeUrlChar(tmpBuf, datalist_p->name[p]);
                sendStatus = NHTTPi_SendData(threadData_p, tmpBuf, tmpBufLen);
                if (sendStatus != 0)
                {
                    return sendStatus;
                }
            }
            sendStatus = NHTTPi_SendData(threadData_p, "=", 1);
            if (sendStatus != 0)
            {
                return sendStatus;
            }
            if (datalist_p->value == NULL)
            {
                s32 result = NHTTPi_SendPostData(mutexInfo_p, req_p,
                    commBuf_p, datalist_p->name, bgnEndInfo_p->socket,
                    &threadData_p->sendBufwp, 2);
                if (result != 0)
                {
                    return result;
                }
            }
            else
            {
                for (p = 0; datalist_p->value[p] != 0; p++)
                {
                    tmpBufLen =
                        NHTTPi_encodeUrlChar(tmpBuf, datalist_p->value[p]);
                    sendStatus = NHTTPi_SendData(threadData_p,
                        tmpBuf, tmpBufLen);
                    if (sendStatus != 0)
                    {
                        return sendStatus;
                    }
                }
            }
            if (datalist_p == req_p->postData->next)
            {
                break;
            }
            sendStatus = NHTTPi_SendData(threadData_p, "&", 1);
            if (sendStatus != 0)
            {
                return sendStatus;
            }
            datalist_p = datalist_p->prev;
        }
    }
    return 0;
}

void NHTTPi_ThreadReqEnd(NHTTPThreadData* threadData_p)
{
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
    void* mutexInfo = NHTTPi_GetMutexInfoP(system);
    NHTTPRequestInfo* request = reqInfo->reqQueue->request;
    NHTTPResponseInfo* response = request->response;
    NHTTPConnectionInfo* connection =
        NHTTPi_Request2Connection(mutexInfo, request);
    if (request->cancel)
    {
        threadData_p->result = NHTTP_ERROR_CANCELED;
        threadData_p->isKeepAlive = 0;
    }
    if (threadData_p->isKeepAlive == 0)
    {
        if (bgnEndInfo->socket >= 0)
        {
            if (NHTTPi_SocClose(mutexInfo, request, bgnEndInfo->socket) < 0)
            {
                threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
            }
            bgnEndInfo->socket = -1;
        }
    }
    if (threadData_p->result == 0)
    {
        response->isSuccess = TRUE;
    }
    else
    {
        response->isSuccess = FALSE;
        NHTTPi_SetError(bgnEndInfo, (NHTTPErr)threadData_p->result);
        if (response->recvBuf_p == threadData_p->tmpBodyBuf)
        {
            response->recvBuf_p = NULL;
            response->recvBufLen = 0;
        }
    }
    if (connection != NULL)
    {
        connection->state = threadData_p->result;
    }
    NHTTPi_lockReqList(mutexInfo);
    NHTTPi_free(reqInfo->reqQueue);
    reqInfo->reqQueue = NULL;
    NHTTPi_unlockReqList(mutexInfo);
    NHTTPi_destroyRequestObject(mutexInfo, request);
    if (connection != NULL)
    {
        if (response->isSuccess)
        {
            connection->started = 5;
        }
    }
    NHTTPi_CompleteCallback(mutexInfo, connection);
    if (connection != NULL)
    {
        NHTTPi_NotifyCompletion(connection);
    }
}

BOOL NHTTPi_ThreadExecReqQueue(NHTTPThreadData* threadData_p)
{
    void* system = NHTTPi_GetSystemInfoP();
    void* mutexInfo = NHTTPi_GetMutexInfoP(system);
    NHTTPReqQueue* queue;
    NHTTPi_lockReqList(mutexInfo);
    queue = NHTTPi_getReqFromReqQueue(&NHTTPi_GetListInfoP(system)->reqQueue);
    if (queue != NULL)
    {
        NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
        threadData_p->id = queue->requestId;
        reqInfo->reqQueue = queue;
    }
    else
    {
        threadData_p->id = -1;
    }
    NHTTPi_unlockReqList(mutexInfo);
    if (threadData_p->id < 0)
    {
        NHTTPi_idleCommThread(NHTTPi_GetThreadInfoP(system));
        return FALSE;
    }
    return TRUE;
}

BOOL NHTTPi_ThreadHostAddrProc(NHTTPThreadData* threadData_p)
{
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPRequestInfo* request =
        NHTTPi_GetReqInfoP(system)->reqQueue->request;
    char* host = request->host;
    if (request->proxyEnabled)
    {
        host = request->proxyServer;
    }
    if (NHTTPi_strlen(host) == 0 || NHTTPi_strcmp(host, threadData_p->preHostname) != 0)
    {
        threadData_p->hostaddr = NHTTPi_resolveHostname(request, host);
        if (threadData_p->hostaddr == 0)
        {
            if (request->proxyEnabled)
            {
                threadData_p->result = NHTTP_ERROR_DNS_PROXY;
                return FALSE;
            }
            threadData_p->result = NHTTP_ERROR_DNS;
            return FALSE;
        }
    }
    else
    {
        threadData_p->hostaddr = threadData_p->preHostaddr;
    }
    NHTTPi_memclr(threadData_p->preHostname, 0x100);
    NHTTPi_memcpy(threadData_p->preHostname, host, NHTTPi_strlen(host));
    threadData_p->port = request->port;
    if (request->proxyEnabled)
    {
        threadData_p->port = request->proxyPort;
    }
    if (threadData_p->hostaddr != threadData_p->preHostaddr || threadData_p->port != threadData_p->prePort
        || request->secure == 1)
    {
        threadData_p->isKeepAlive = 0;
    }
    threadData_p->preHostaddr = threadData_p->hostaddr;
    threadData_p->prePort = threadData_p->port;
    return TRUE;
}

BOOL NHTTPi_ThreadConnectProc(NHTTPThreadData* threadData_p)
{
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
    NHTTPRequestInfo* request = reqInfo->reqQueue->request;
    void* mutexInfo = NHTTPi_GetMutexInfoP(system);
    BOOL reconnect = FALSE;
    if (threadData_p->isKeepAlive == TRUE)
    {
        if (NHTTPi_checkKeepAlive(bgnEndInfo->socket) == -1)
        {
            threadData_p->isKeepAlive = FALSE;
            reconnect = TRUE;
        }
    }
    if (threadData_p->isKeepAlive == 0)
    {
        if (bgnEndInfo->socket >= 0)
        {
            if (NHTTPi_SocClose(mutexInfo, request, bgnEndInfo->socket) < 0
                && !reconnect
            )
            {
                bgnEndInfo->socket = -1;
                threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
                return FALSE;
            }
        }
        bgnEndInfo->socket = NHTTPi_SocOpen(request);
        if (bgnEndInfo->socket < 0)
        {
            threadData_p->result = NHTTP_ERROR_SOCKET;
            return FALSE;
        }
        NHTTPi_lockReqList(mutexInfo);
        reqInfo->reqQueue->_unk10 = bgnEndInfo->socket;
        NHTTPi_unlockReqList(mutexInfo);
        if (request->cancel)
        {
            return FALSE;
        }
        if (NHTTPi_SocConnect(bgnEndInfo, mutexInfo, request,
                bgnEndInfo->socket, threadData_p->hostaddr, threadData_p->port) < 0)
        {
            if (request->proxyEnabled)
            {
                threadData_p->result = NHTTP_ERROR_CONNECT_PROXY;
                return FALSE;
            }
            if (NHTTPi_GetSSLError(bgnEndInfo) != 0)
            {
                threadData_p->result = NHTTP_ERROR_SSL;
                return FALSE;
            }
            threadData_p->result = NHTTP_ERROR_CONNECT;
            return FALSE;
        }
    }
    else
    {
        NHTTPi_lockReqList(mutexInfo);
        reqInfo->reqQueue->_unk10 = bgnEndInfo->socket;
        NHTTPi_unlockReqList(mutexInfo);
    }
    return TRUE;
}

s32 NHTTPi_ThreadProxyProc(NHTTPThreadData* threadData_p)
{
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
    void* mutexInfo = NHTTPi_GetMutexInfoP(system);
    NHTTPRequestInfo* request = reqInfo->reqQueue->request;
    threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
    threadData_p->sendBufwp = 0;
    if (request->secure != 0 && request->proxyEnabled != 0)
    {
        s32 result = NHTTPi_SendProxyConnectMethod(threadData_p);
        if (result != 0)
        {
            return result;
        }
        if (!NHTTPi_RecvProxyConnectHeader(threadData_p))
        {
            return 1;
        }
        result = NHTTPi_SocSSLConnect(bgnEndInfo, mutexInfo, request, bgnEndInfo->socket);
        if (result != 0)
        {
            if (result == -1004)
            {
                if (NHTTPi_GetSSLError(bgnEndInfo) != 0)
                {
                    threadData_p->result = NHTTP_ERROR_SSL_ROOTCA;
                }
                return 1;
            }
            if (result == -1005)
            {
                if (NHTTPi_GetSSLError(bgnEndInfo) != 0)
                {
                    threadData_p->result = NHTTP_ERROR_SSL_CLIENTCERT;
                }
                return 1;
            }
            if (NHTTPi_GetSSLError(bgnEndInfo) != 0)
            {
                threadData_p->result = NHTTP_ERROR_SSL;
            }
            return 1;
        }
    }
    return 0;
}

s32 NHTTPi_ThreadSendProc(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPConnectionInfo* connection_p =
        NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char* commBuf_p = threadInfo_p->commBuf;
    s32 urlLength = NHTTPi_strlen(req_p->url);
    s32 sendStatus = 0;
    BOOL isMultipartForm = FALSE;
    NHTTPHeader* datalist_p;
    threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
    if (connection_p != NULL)
    {
        connection_p->started = 2;
    }
    threadData_p->sendBufwp = 0;
    switch (req_p->method)
    {
    case NHTTP_REQMETHOD_GET:
        sendStatus = NHTTPi_SendData(threadData_p, "GET ", 4);
        break;
    case NHTTP_REQMETHOD_POST:
        sendStatus = NHTTPi_SendData(threadData_p, "POST ", 5);
        break;
    case NHTTP_REQMETHOD_HEAD:
        sendStatus = NHTTPi_SendData(threadData_p, "HEAD ", 5);
        break;
    default:
        break;
    }
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    if (req_p->proxyEnabled != 0 && req_p->secure == 0)
    {
        sendStatus = NHTTPi_SendData(threadData_p, req_p->url, urlLength);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    else if (urlLength > req_p->pathStart)
    {
        sendStatus = NHTTPi_SendData(threadData_p,
            req_p->url + req_p->pathStart, urlLength - req_p->pathStart);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    else
    {
        sendStatus = NHTTPi_SendData(threadData_p, "/", 1);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    sendStatus = NHTTPi_SendData(threadData_p, " HTTP/1.1\r\n", 11);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    {
        s32 n = req_p->secure ? 8 : 7;
        sendStatus = NHTTPi_SendData(threadData_p, "Host: ", 6);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p,
            req_p->url + n, req_p->hostEnd - n);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    if (req_p->proxyEnabled != 0 && req_p->secure == 0
    )
    {
        sendStatus = NHTTPi_SendProxyAuthorization(threadData_p);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    {
        sendStatus = NHTTPi_SendBasicAuthorization(threadData_p);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    sendStatus = NHTTPi_SendHeaderList(threadData_p);
    if (sendStatus != 0)
    {
        return sendStatus;
    }
    if (req_p->method == NHTTP_REQMETHOD_POST)
    {
        s32 postResult = 0;
        if (req_p->isRawData)
        {
            postResult = NHTTPi_SendProcPostDataRaw(threadData_p);
        }
        else
        {
            NHTTPEncodingType a_encodingType = req_p->encodingType;
            if (a_encodingType == NHTTP_ENCODING_TYPE_AUTO)
            {
                for (isMultipartForm = FALSE,
                    datalist_p = req_p->postData;
                    datalist_p != NULL; datalist_p = datalist_p->prev)
                {
                    if (datalist_p->_unk14 != 0)
                    {
                        isMultipartForm = TRUE;
                        break;
                    }
                    if (datalist_p == req_p->postData->next)
                    {
                        break;
                    }
                }
            }
            else if (a_encodingType == NHTTP_ENCODING_TYPE_MULTIPART)
            {
                isMultipartForm = TRUE;
            }
            else
            {
                isMultipartForm = FALSE;
            }
            if (isMultipartForm)
            {
                postResult = NHTTPi_SendProcPostDataBinary(threadData_p);
            }
            else
            {
                postResult = NHTTPi_SendProcPostDataAscii(threadData_p);
            }
        }
        if (postResult != 0)
        {
            if (postResult == 3)
            {
                threadData_p->result = NHTTP_ERROR_SOCKET;
            }
            return postResult;
        }
    }
    else
    {
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != 0)
        {
            return sendStatus;
        }
    }
    {
        sendStatus = 0;
        if (threadData_p->sendBufwp > 0)
        {
            s32 stat = NHTTPi_SocSend(req_p, bgnEndInfo_p->socket, commBuf_p,
                threadData_p->sendBufwp, 0);
            threadData_p->sendBufwp = 0;
            NHTTPi_memclr(commBuf_p, 0x100);
            if (stat < 0)
            {
                sendStatus = 1;
            }
            if (stat == 0)
            {
                sendStatus = 2;
            }
        }
        threadData_p->sendBufwp = 0;
        NHTTPi_memclr(commBuf_p, 0x100);
        return sendStatus;
    }
}

BOOL NHTTPi_ThreadRecvHeaderProc(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPResponseInfo* res_p = req_p->response;
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnectionInfo* connection_p =
        NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char tmpBuf[4] = { 0 };
    s32 recvbyte = 0;
    NHTTPi_HDRBUFLIST* headerBuf_p = NULL;

    if (connection_p != NULL)
    {
        connection_p->started = 3;
    }
    res_p->headerLen = 0;
    NHTTPi_memclr(threadData_p->tmpHeaderBuf, TMP_HEADER_BUF_SIZE);

    for (headerBuf_p = res_p->hdrBufBlock_p,
        threadData_p->recvBufwp = 0;;)
    {
        if (req_p->cancel)
        {
            return FALSE;
        }
        if (threadData_p->recvBufwp < NHTTP_HDRRECVBUF_INILEN)
        {
            recvbyte = NHTTPi_SocRecv(
                mutexInfo_p,
                req_p, bgnEndInfo_p->socket,
                (char*)res_p->hdrBufFirst + threadData_p->recvBufwp, 1,
                0);
            tmpBuf[threadData_p->recvBufwp & 3] =
                res_p->hdrBufFirst[threadData_p->recvBufwp];
        }
        else
        {
            s32 wp =
                threadData_p->recvBufwp & NHTTP_HDRRECVBUF_BLOCKMASK;
            if (wp == 0)
            {
                if (headerBuf_p != NULL)
                {
                    headerBuf_p->next_p =
                        (NHTTPi_HDRBUFLIST*)NHTTPi_alloc(0x204, 4);
                    headerBuf_p = headerBuf_p->next_p;
                }
                else
                {
                    headerBuf_p =
                        (NHTTPi_HDRBUFLIST*)NHTTPi_alloc(0x204, 4);
                    res_p->hdrBufBlock_p = headerBuf_p;
                }
                if (headerBuf_p == NULL)
                {
                    threadData_p->result = NHTTP_ERROR_ALLOC;
                    return FALSE;
                }
                headerBuf_p->next_p = NULL;
            }
            recvbyte = NHTTPi_SocRecv(
                mutexInfo_p,
                req_p, bgnEndInfo_p->socket,
                (char*)&headerBuf_p->block[wp], 1, 0);
            tmpBuf[threadData_p->recvBufwp & 3] = headerBuf_p->block[wp];
        }
        if (recvbyte <= 0)
        {
            threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
            return FALSE;
        }
        threadData_p->recvBufwp += recvbyte;
        if (NHTTPi_CheckHeaderEnd(tmpBuf, threadData_p->recvBufwp))
        {
            res_p->headerLen = threadData_p->recvBufwp;
            break;
        }
    }
    if (res_p->headerLen == 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    return TRUE;
}

BOOL NHTTPi_ThreadParseHeaderProc(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPResponseInfo* res_p = req_p->response;
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char* commBuf_p = (char*)&threadInfo_p->commBuf[0];
    s32 labelend;
    s32 n;

    if (!NHTTPi_loadFromHdrRecvBuf(
            res_p, threadData_p->tmpHeaderBuf, 0, TMP_HEADER_BUF_SIZE))
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    if (NHTTPi_strnicmp(threadData_p->tmpHeaderBuf, "HTTP/", 5) != 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    if (threadData_p->tmpHeaderBuf[8] != ' ')
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    res_p->httpStatus = NHTTPi_strToInt(&threadData_p->tmpHeaderBuf[9], 3);
    if (res_p->httpStatus < 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    if (NHTTPi_findNextLineHdrRecvBuf(
            res_p, 12, res_p->headerLen, &labelend, NULL) < 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    threadData_p->contentLength =
        NHTTPi_getHeaderValue(res_p, "Content-Length", &n);
    if (threadData_p->contentLength == 0)
    {
        threadData_p->result = NHTTP_ERROR_NONE;
        return FALSE;
    }
    if (threadData_p->contentLength > 0x100)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    if (threadData_p->contentLength > 0)
    {
        if (!NHTTPi_loadFromHdrRecvBuf(
                res_p, commBuf_p, n, threadData_p->contentLength))
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            return FALSE;
        }
        threadData_p->contentLength =
            NHTTPi_strToInt(commBuf_p, threadData_p->contentLength);
        if (threadData_p->contentLength < 0)
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            return FALSE;
        }
        res_p->contentLength = threadData_p->contentLength;
    }
    else
    {
        res_p->contentLength = 0xFFFFFFFF;
    }
    if (req_p->secure)
    {
        threadData_p->isKeepAlive = FALSE;
    }
    else
    {
        s32 connectionHeaderStrLen =
            NHTTPi_getHeaderValue(res_p, "Connection", &n);
        if (connectionHeaderStrLen == 0)
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            threadData_p->isKeepAlive = FALSE;
            return FALSE;
        }
        if (NHTTPi_strnicmp(threadData_p->tmpHeaderBuf, "HTTP/1.1", 8) == 0)
        {
            threadData_p->isKeepAlive = TRUE;
        }
        else
        {
            threadData_p->isKeepAlive = FALSE;
        }
        if (connectionHeaderStrLen <= 0x100 && connectionHeaderStrLen > 0)
        {
            if (NHTTPi_compareTokenN_HdrRecvBuf(res_p, n,
                    n + connectionHeaderStrLen, "Keep-Alive", 0) == 0)
            {
                threadData_p->isKeepAlive = TRUE;
            }
            if (NHTTPi_compareTokenN_HdrRecvBuf(res_p, n,
                    n + connectionHeaderStrLen, "Close", 0) == 0)
            {
                threadData_p->isKeepAlive = FALSE;
            }
        }
    }
    threadData_p->isChunked =
        NHTTPi_getHeaderValue(res_p, "Transfer-Encoding", &n);
    if (threadData_p->isChunked == 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        return FALSE;
    }
    if (threadData_p->isChunked > 0x100)
    {
        threadData_p->isChunked = FALSE;
    }
    else
    {
        threadData_p->isChunked = (threadData_p->isChunked > 0)
            ? (NHTTPi_compareTokenN_HdrRecvBuf(res_p, n,
                   n + threadData_p->isChunked, "chunked", ';')
                == 0)
            : FALSE;
    }
    threadData_p->result = NHTTP_ERROR_NONE;
    res_p->isHeaderParse = TRUE;
    return TRUE;
}

BOOL NHTTPi_ThreadRecvBodyProc(NHTTPThreadData* threadData_p)
{
    void* sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPRequestInfo* req_p = reqInfo_p->reqQueue->request;
    NHTTPResponseInfo* res_p = req_p->response;
    NHTTPBgnEndInfo* bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnectionInfo* connection_p =
        NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPThreadInfo* threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char* commBuf_p = &threadInfo_p->commBuf[0];
    s32 recvbyte = 0;
    char tmpBuf[2];
    s32 n;
    if (req_p->method == NHTTP_REQMETHOD_HEAD
        || res_p->httpStatus == 204 || res_p->httpStatus == 304
        || (res_p->httpStatus >= 100 && res_p->httpStatus < 200))
    {
        return TRUE;
    }
    NHTTPi_SetVirtualContentLength(connection_p, 0);
    if (connection_p != NULL)
    {
        connection_p->started = 4;
    }
    if (threadData_p->contentLength >= 0)
    {
        NHTTPi_SetVirtualContentLength(connection_p, threadData_p->contentLength);
        while (threadData_p->contentLength > 0)
        {
            if (threadData_p->result != NHTTP_ERROR_BUFFULL)
            {
                if (!NHTTPi_BufFull(mutexInfo_p, res_p))
                {
                    threadData_p->result = NHTTP_ERROR_BUFFULL;
                    res_p->recvBuf_p = threadData_p->tmpBodyBuf;
                    res_p->recvBufLen = 0x200;
                }
            }
            if (threadData_p->result == NHTTP_ERROR_BUFFULL)
            {
                recvbyte = NHTTPi_RecvBufN(
                    mutexInfo_p,
                    req_p, bgnEndInfo_p->socket, 0,
                    threadData_p->contentLength, 0);
            }
            else
            {
                recvbyte = NHTTPi_RecvBufN(
                    mutexInfo_p,
                    req_p, bgnEndInfo_p->socket,
                    res_p->bodyLen, threadData_p->contentLength, 0);
            }
            if (recvbyte < 0)
            {
                return FALSE;
            }
            if (recvbyte == 0)
            {
                break;
            }
            if (threadData_p->result != NHTTP_ERROR_BUFFULL)
            {
                res_p->bodyLen += recvbyte;
                res_p->totalBodyLen += recvbyte;
            }
            threadData_p->contentLength -= recvbyte;
        }
        if (threadData_p->result != NHTTP_ERROR_BUFFULL)
        {
            if (threadData_p->contentLength != 0)
            {
                threadData_p->result =
                    NHTTPi_isRecvBufFull(res_p, res_p->bodyLen)
                    ? NHTTP_ERROR_BUFFULL
                    : NHTTP_ERROR_REVOLUTIONWIFI;
            }
            else
            {
                threadData_p->result = 0;
            }
        }
    }
    else
    {
        threadData_p->result = NHTTP_ERROR_REVOLUTIONWIFI;
        if (threadData_p->isChunked != 0)
        {
            s32 chunkSize = -1;
            for (;;)
            {
                tmpBuf[0] = 0;
                tmpBuf[1] = 0;
                for (threadData_p->recvBufwp = 0;
                     threadData_p->recvBufwp < 0x100;
                     threadData_p->recvBufwp++)
                {
                    recvbyte = NHTTPi_SocRecv(
                        mutexInfo_p,
                        req_p,
                        bgnEndInfo_p->socket,
                        commBuf_p + threadData_p->recvBufwp, 1,
                        0);
                    if (recvbyte < 0)
                    {
                        return FALSE;
                    }
                    tmpBuf[threadData_p->recvBufwp & 1] =
                        commBuf_p[threadData_p->recvBufwp];
                    if (tmpBuf[threadData_p->recvBufwp & 1] == ';'
                        || (tmpBuf[threadData_p->recvBufwp & 1] == '\n'
                            && tmpBuf[(threadData_p->recvBufwp - 1) & 1]
                                == '\r'))
                    {
                        if (tmpBuf[threadData_p->recvBufwp & 1] == '\n')
                        {
                            n = threadData_p->recvBufwp - 1;
                        }
                        else
                        {
                            n = threadData_p->recvBufwp;
                            if (NHTTPi_SkipLineBuf(
                                mutexInfo_p,
                                req_p, bgnEndInfo_p->socket)
                                <= 0)
                            {
                                return FALSE;
                            }
                        }
                        if (n == 0)
                        {
                            return FALSE;
                        }
                        chunkSize = NHTTPi_strToHex(commBuf_p, n);
                        if (chunkSize < 0)
                        {
                            return FALSE;
                        }
                        break;
                    }
                }
                if (threadData_p->recvBufwp == 0x100)
                {
                    threadData_p->result = NHTTP_ERROR_HTTPPARSE;
                    return FALSE;
                }
                if (chunkSize <= 0)
                {
                    break;
                }
                NHTTPi_SetVirtualContentLength(connection_p, chunkSize);
                while (chunkSize > 0)
                {
                    if (threadData_p->result != NHTTP_ERROR_BUFFULL)
                    {
                        if (!NHTTPi_BufFull(mutexInfo_p, res_p))
                        {
                            threadData_p->result = NHTTP_ERROR_BUFFULL;
                            res_p->recvBuf_p = threadData_p->tmpBodyBuf;
                            res_p->recvBufLen = 0x200;
                        }
                    }
                    if (threadData_p->result == NHTTP_ERROR_BUFFULL)
                    {
                        recvbyte = NHTTPi_RecvBufN(
                            mutexInfo_p,
                            req_p,
                            bgnEndInfo_p->socket, 0, chunkSize, 0);
                    }
                    else
                    {
                        recvbyte = NHTTPi_RecvBufN(
                            mutexInfo_p,
                            req_p,
                            bgnEndInfo_p->socket, res_p->bodyLen,
                            chunkSize, 0);
                    }
                    if (recvbyte <= 0)
                    {
                        return FALSE;
                    }
                    chunkSize -= recvbyte;
                    res_p->bodyLen += recvbyte;
                    res_p->totalBodyLen += recvbyte;
                    if (chunkSize == 0)
                    {
                        recvbyte = NHTTPi_SocRecv(
                            mutexInfo_p,
                            req_p,
                            bgnEndInfo_p->socket, commBuf_p, 2, 0);
                        if (recvbyte <= 0)
                        {
                            return FALSE;
                        }
                    }
                }
            }
            NHTTPi_SkipLineBuf(
                mutexInfo_p,
                req_p, bgnEndInfo_p->socket);
            threadData_p->result = 0;
        }
        else
        {
            for (;;)
            {
                if (!NHTTPi_BufFull(mutexInfo_p, res_p))
                {
                    threadData_p->result = NHTTP_ERROR_BUFFULL;
                    res_p->recvBuf_p = threadData_p->tmpBodyBuf;
                    res_p->recvBufLen = 0x200;
                }
                if (threadData_p->result == NHTTP_ERROR_BUFFULL)
                {
                    recvbyte = NHTTPi_RecvBuf(
                        mutexInfo_p,
                        req_p, bgnEndInfo_p->socket, 0,
                        0);
                }
                else
                {
                    recvbyte = NHTTPi_RecvBuf(
                        mutexInfo_p,
                        req_p, bgnEndInfo_p->socket,
                        res_p->bodyLen, 0);
                }
                if (recvbyte < 0)
                {
                    return FALSE;
                }
                if (recvbyte == 0)
                {
                    if (threadData_p->result != NHTTP_ERROR_BUFFULL)
                    {
                        threadData_p->result = 0;
                    }
                    break;
                }
                res_p->bodyLen += recvbyte;
                res_p->totalBodyLen += recvbyte;
            }
        }
    }
    connection_p = NHTTPi_Response2Connection(mutexInfo_p, res_p);
    if (threadData_p->result == 0 && connection_p != NULL)
    {
        NHTTPi_ReceivedCallback(mutexInfo_p, connection_p);
    }
    return TRUE;
}

void NHTTPi_CommThreadProcMain(void* argument)
{
    NHTTPThreadData threadData;
    void* system = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo* bgnEndInfo = NHTTPi_GetBgnEndInfoP(system);
    NHTTPReqInfo* reqInfo = NHTTPi_GetReqInfoP(system);
    threadData.id = -1;
    NHTTPi_memclr(threadData.preHostname, 0x100);
    NHTTPi_memclr(threadData.tmpBodyBuf, 0x200);
    threadData.hostaddr = (u32)-1;
    threadData.preHostaddr = (u32)-1;
    threadData.sendBufwp = 0;
    threadData.isKeepAlive = 0;
    threadData.isChunked = 0;
    threadData.restartConnect = 0;
    threadData.contentLength = 0;
    threadData.result = 0;
    while (bgnEndInfo->_unk7DC == 0)
    {
        if (threadData.restartConnect == 0)
        {
            if (!NHTTPi_ThreadExecReqQueue(&threadData))
            {
                continue;
            }
            if (reqInfo->reqQueue->request->cancel)
            {
                NHTTPi_ThreadReqEnd(&threadData);
                continue;
            }
            if (!NHTTPi_ThreadHostAddrProc(&threadData))
            {
                NHTTPi_ThreadReqEnd(&threadData);
                continue;
            }
        }
        if (threadData.restartConnect == 1)
        {
            threadData.restartConnect = 0;
        }
        if (!NHTTPi_ThreadConnectProc(&threadData))
        {
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        }
        switch (NHTTPi_ThreadProxyProc(&threadData))
        {
        case 0:
            break;
        case 2:
            threadData.restartConnect = 1;
            continue;
        case 1:
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        default:
            break;
        }
        switch (NHTTPi_ThreadSendProc(&threadData))
        {
        case 0:
            break;
        case 2:
            threadData.restartConnect = 1;
            continue;
        case 1:
        case 3:
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        default:
            break;
        }
        if (reqInfo->reqQueue->request->cancel)
        {
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        }
        if (!NHTTPi_ThreadRecvHeaderProc(&threadData))
        {
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        }
        if (!NHTTPi_ThreadParseHeaderProc(&threadData))
        {
            NHTTPi_ThreadReqEnd(&threadData);
            continue;
        }
        NHTTPi_ThreadRecvBodyProc(&threadData);
        NHTTPi_ThreadReqEnd(&threadData);
    }
}
