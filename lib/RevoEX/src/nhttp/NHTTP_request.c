#include <private/nhttp.h>

const char NHTTPi_strMultipartBound[] = STR_POSTBOUND;

void NHTTPi_InitRequestInfo(NHTTPReqInfo* info)
{
    info->reqQueue = NULL;
}

NHTTPRequestInfo* NHTTP_CreateRequest(NHTTPBgnEndInfo* bgnEndInfo_p,
    const char* url_p, s32 method, void* buf_p, u32 len, void* param_p,
    NHTTPResponseCallback bufFull, NHTTPResponseCleanup freeBuf)
{
    int i;
    int suflen, urllen, prelen;
    int nhostenc, deccount;
    int declen, hostend;
    int port;
    int decode_urllen;

    char a_url[LEN_URLBUF];
    char* prefix_p;
    NHTTPRequestInfo* req_p = NULL;

    if (method >= 3 || method < 0)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_UNKNOWN);
        goto errend;
    }

    req_p = NHTTPi_alloc(sizeof(NHTTPRequestInfo), 4);
    if (!req_p)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    NHTTPi_memclr(req_p, sizeof(NHTTPRequestInfo));

    req_p->response = NHTTPi_alloc(sizeof(NHTTPResponseInfo), 4);
    if (!req_p->response)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    NHTTPi_memclr(req_p->response, sizeof(NHTTPResponseInfo));
    req_p->response->recvBuf_p = buf_p;
    req_p->response->recvBufLen = len;
    req_p->response->bufFull = bufFull;
    req_p->response->freeBuf = freeBuf;

    urllen = NHTTPi_strlen(url_p);
    if (urllen <= 7)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }

    NHTTPi_memclr(a_url, urllen);
    NHTTPi_memcpy(a_url, url_p, urllen);

    req_p->port = 80;
    suflen = 7;
    if (NHTTPi_strnicmp(a_url, "http://", 7) != 0)
    {
        if (NHTTPi_strnicmp(a_url, "https://", 8) != 0)
        {
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
            goto errend;
        }
        req_p->secure = TRUE;
        req_p->port = 443;
        suflen = 8;
    }

    prefix_p = a_url + suflen;
    prelen = urllen - suflen;
    if (prelen <= 0)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }

    for (i = 0, nhostenc = 0, deccount = 0;
         (i < prelen) && (prefix_p[i] != '/'); i++)
    {
        if (deccount == 2)
        {
            deccount--;
        }
        else if (deccount == 1)
        {
            s8 c = (s8)NHTTPi_strToHex(&prefix_p[i - 1], 2);
            deccount--;
            if (c < 0)
            {
                NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
                goto errend;
            }
            if (c == '/')
            {
                nhostenc--;
                break;
            }
        }
        else if (prefix_p[i] == '%')
        {
            deccount = 2;
            nhostenc++;
        }
    }

    if (deccount)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }

    decode_urllen = suflen + prelen - nhostenc * 2 + 1;
    req_p->url = NHTTPi_alloc(decode_urllen, 4);
    if (!req_p->url)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    NHTTPi_memclr(req_p->url, decode_urllen);
    NHTTPi_memcpy(req_p->url, a_url, suflen);

    for (i = 0, declen = 0, deccount = 0, hostend = FALSE;
         i < prelen; i++)
    {
        if (deccount == 2)
        {
            deccount--;
        }
        else if (deccount == 1)
        {
            char c = (char)NHTTPi_strToHex(&prefix_p[i - 1], 2);
            deccount--;
            req_p->url[suflen + declen - 1] = c;
            if (c == '/')
            {
                hostend = TRUE;
            }
        }
        else
        {
            if (prefix_p[i] == '/')
            {
                hostend = TRUE;
            }
            if (!hostend & (prefix_p[i] == '%'))
            {
                deccount = 2;
            }
            else
            {
                req_p->url[suflen + declen] = prefix_p[i];
            }
            declen++;
        }
    }
    req_p->url[suflen + declen] = 0;

    prefix_p = &req_p->url[suflen];
    prelen = declen;

    for (i = 0; i < prelen; i++)
    {
        if ((prefix_p[i] == '/') || (prefix_p[i] == ':'))
        {
            req_p->hostEnd = i + suflen;
            break;
        }
    }

    if (i == prelen)
    {
        req_p->pathStart = req_p->hostEnd = i + suflen;
    }
    else if (prefix_p[i] == '/')
    {
        req_p->pathStart = req_p->hostEnd;
    }
    else if (prefix_p[i] == ':')
    {
        for (; i < prelen; i++)
        {
            if (prefix_p[i] == '/')
            {
                req_p->pathStart = i + suflen;
                break;
            }
        }

        if (i == prelen)
        {
            req_p->pathStart = i + suflen;
        }
        else
        {
            port = NHTTPi_strtonum(&req_p->url[req_p->hostEnd + 1],
                (int)(req_p->pathStart - (req_p->hostEnd + 1)));
            if (port < 0)
            {
                port = req_p->port;
            }
            else if (port > 65535)
            {
                NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
                goto errend;
            }
            req_p->port = (u16)port;
        }
    }

    {
        u32 a_hostnameLen = (u32)(req_p->hostEnd - (req_p->secure ? 8 : 7));
        req_p->host = NHTTPi_alloc(a_hostnameLen + 1, 4);
        if (!req_p->host)
        {
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
            goto errend;
        }
        NHTTPi_memclr(req_p->host, (int)(a_hostnameLen + 1));
        NHTTPi_memcpy(req_p->host,
            req_p->url + (req_p->secure ? 8 : 7), (int)a_hostnameLen);
    }

    NHTTPi_memcpy(req_p->multipartBoundary, NHTTPi_strMultipartBound,
        sizeof(req_p->multipartBoundary));
    req_p->method = method;
    req_p->sslId = 0;
    req_p->clientCertData = NULL;
    req_p->clientCertSize = 0;
    req_p->privateKeyData = NULL;
    req_p->privateKeySize = 0;
    req_p->rootCAData = NULL;
    req_p->rootCASize = 0;
    req_p->clientCertDefault = FALSE;
    req_p->verifyOption = 0;
    req_p->sslCallbackParam = NULL;
    req_p->response->param_p = param_p;
    req_p->proxyEnabled = FALSE;
    req_p->recvBufferSize = 0;
    return req_p;

errend:
    if (req_p)
    {
        if (req_p->url)
        {
            NHTTPi_free(req_p->url);
        }
        if (req_p->host)
        {
            NHTTPi_free(req_p->host);
        }
        if (req_p->response)
        {
            NHTTPi_free(req_p->response);
        }
        NHTTPi_free(req_p);
    }
    return NULL;
}

void NHTTP_DestroyRequest(void* systemInfo, NHTTPRequestInfo* request)
{
    NHTTPConnectionInfo* connection;
    NHTTPHeader* header;
    NHTTPHeader* next;
    NHTTPHeader* post;
    NHTTPHeader* nextPost;
    void* mutexInfo = NHTTPi_GetMutexInfoP(systemInfo);

    connection = NHTTPi_Request2Connection(mutexInfo, request);
    if (connection != NULL)
    {
        connection->response = NULL;
    }
    NHTTPi_free(request->response);

    connection = NHTTPi_Request2Connection(mutexInfo, request);
    if (connection != NULL)
    {
        connection->request = NULL;
    }

    header = request->headers;
    while (header != NULL)
    {
        if (header != header->next)
        {
            next = header->next->next;
            NHTTPi_free(header->next);
            header->next = next;
        }
        else
        {
            NHTTPi_free(header);
            header = NULL;
        }
    }

    post = request->postData;
    while (post != NULL)
    {
        if (post != post->next)
        {
            nextPost = post->next->next;
            NHTTPi_free(post->next);
            post->next = nextPost;
        }
        else
        {
            NHTTPi_free(post);
            post = NULL;
        }
    }

    NHTTPi_free(request->url);
    NHTTPi_free(request->host);
    NHTTPi_free(request);
}

BOOL NHTTPi_destroyRequestObject(void* mutexInfo, NHTTPRequestInfo* request)
{
    NHTTPConnectionInfo* connection;
    NHTTPHeader* header;
    NHTTPHeader* next;
    NHTTPHeader* post;
    NHTTPHeader* nextPost;

    connection = NHTTPi_Request2Connection(mutexInfo, request);
    if (connection != NULL)
    {
        connection->request = NULL;
    }

    header = request->headers;
    while (header != NULL)
    {
        if (header != header->next)
        {
            next = header->next->next;
            NHTTPi_free(header->next);
            header->next = next;
        }
        else
        {
            NHTTPi_free(header);
            header = NULL;
        }
    }

    post = request->postData;
    while (post != NULL)
    {
        if (post != post->next)
        {
            nextPost = post->next->next;
            NHTTPi_free(post->next);
            post->next = nextPost;
        }
        else
        {
            NHTTPi_free(post);
            post = NULL;
        }
    }

    NHTTPi_free(request->url);
    NHTTPi_free(request->host);
    NHTTPi_free(request);
    return TRUE;
}

s32 NHTTP_SendRequestAsync(void* systemInfo, NHTTPRequestInfo* request)
{
    NHTTPBgnEndInfo* info = NHTTPi_GetBgnEndInfoP(systemInfo);
    void* threadInfo = NHTTPi_GetThreadInfoP(systemInfo);
    void* mutexInfo = NHTTPi_GetMutexInfoP(systemInfo);
    s32 requestId;

    if (request->state != 0)
    {
        NHTTPi_SetError(info, NHTTP_ERROR_UNKNOWN);
        return -1;
    }

    NHTTPi_lockReqList(mutexInfo);
    requestId = NHTTPi_setReqQueue(NHTTPi_GetListInfoP(systemInfo), request);
    if (requestId >= 0)
    {
        request->state = 1;
        NHTTPi_kickCommThread(threadInfo);
    }
    else
    {
        NHTTPi_SetError(info, NHTTP_ERROR_ALLOC);
    }
    NHTTPi_unlockReqList(mutexInfo);
    return requestId;
}

BOOL NHTTP_CancelRequestAsync(void* sysInfo_p, s32 id)
{
    BOOL rc = FALSE;
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    volatile NHTTPReqQueue* reqCurrent_p = reqInfo_p->reqQueue;

    NHTTPi_lockReqList(mutexInfo_p);

    if (reqCurrent_p)
    {
        if (reqCurrent_p->requestId == id)
        {
            if (!reqCurrent_p->request->cancel)
            {
                reqCurrent_p->request->cancel = TRUE;
                NHTTPi_SocCancel(mutexInfo_p, reqCurrent_p->request,
                    reqCurrent_p->_unk10);
                rc = TRUE;
            }
        }
    }

    if (!rc)
    {
        NHTTPListInfo* listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
        rc = NHTTPi_freeReqQueue(listInfo_p, mutexInfo_p, id);
    }

    NHTTPi_unlockReqList(mutexInfo_p);

    return rc;
}

void NHTTPi_cancelAllRequests(void* sysInfo_p)
{
    NHTTPReqInfo* reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPListInfo* listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
    void* mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    volatile NHTTPReqQueue* reqCurrent_p = reqInfo_p->reqQueue;

    NHTTPi_lockReqList(mutexInfo_p);

    if (reqCurrent_p)
    {
        if (!reqCurrent_p->request->cancel)
        {
            reqCurrent_p->request->cancel = TRUE;
            NHTTPi_SocCancel(mutexInfo_p, reqCurrent_p->request,
                reqCurrent_p->_unk10);
        }
    }

    NHTTPi_allFreeReqQueue(listInfo_p, mutexInfo_p);
    NHTTPi_unlockReqList(mutexInfo_p);
}
