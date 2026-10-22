#include <private/nhttp.h>

#include <revolution/so.h>
#include <revolution/ssl.h>

s32 NHTTPi_checkKeepAlive(s32 socket)
{
    char buffer[1];
    s32 result = 0;
    if (socket == -1)
    {
        result = -1;
    }
    else if (SORecv(socket, buffer, sizeof(buffer), 4) != SO_EAGAIN)
    {
        result = -1;
    }
    return result;
}

s32 NHTTPi_SocOpen(NHTTPRequestInfo* request)
{
    s32 socket;
    s32 recvBufferSize;

    socket = __SOCreateSocket(SO_PF_INET, SO_SOCK_STREAM, 0);
    recvBufferSize = 0;
    if (request != NULL)
    {
        recvBufferSize = request->recvBufferSize;
    }
    if (socket >= 0 && recvBufferSize != 0)
    {
        SOSetSockOpt(socket, SO_SOL_SOCKET, SO_SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize));
    }
    return socket;
}

s32 NHTTPi_SocClose(void* mutexInfo, NHTTPRequestInfo* request, s32 socket)
{
    NHTTPi_lockReqList(mutexInfo);
    if (request->sslId > 0)
    {
        SSLShutdown(request->sslId);
        request->sslId = -1;
    }
    NHTTPi_unlockReqList(mutexInfo);
    return SOClose(socket);
}

s32 NHTTPi_SocConnect(NHTTPBgnEndInfo* info, void* mutexInfo,
    NHTTPRequestInfo* request, s32 socket, u32 address, u32 port)
{
    SOSockAddrIn sockAddr;
    s32 result;

    sockAddr.len = sizeof(sockAddr);
    sockAddr.family = SO_PF_INET;
    sockAddr.port = SOHtoNs((u16)port);
    sockAddr.addr.addr = address;
    result = SOConnect(socket, &sockAddr);
    if (result < 0)
    {
        result = -1001;
        if (request->cancel != FALSE)
        {
            result = -1002;
        }
    }
    else if (request->secure != FALSE && request->proxyEnabled == FALSE)
    {
        result = NHTTPi_SocSSLConnect(info, mutexInfo, request, socket);
    }
    else
    {
        result = 0;
    }
    return result;
}

s32 NHTTPi_SocSSLConnect(NHTTPBgnEndInfo* bgnEndInfo_p, void* mutexInfo_p,
    NHTTPRequestInfo* req_p, const s32 socket)
{
    BOOL done = FALSE;

    req_p->sslId = SSLNew(req_p->verifyOption, req_p->host);
    if (bgnEndInfo_p->sslCallback != NULL && req_p->sslCallbackParam != NULL)
    {
        bgnEndInfo_p->sslCallback(req_p->sslId, req_p->sslCallbackParam);
    }
    if (req_p->clientCertDefault == TRUE)
    {
        if (SSLSetBuiltinClientCert(req_p->sslId, req_p->builtinClientCert) != 0)
        {
            return -1005;
        }
    }
    else if (req_p->clientCertData != NULL && req_p->privateKeyData != NULL)
    {
        if (SSLSetClientCert(req_p->sslId, req_p->clientCertData,
                req_p->clientCertSize, req_p->privateKeyData,
                req_p->privateKeySize) != 0)
        {
            return -1005;
        }
    }
    if (req_p->rootCAData != NULL)
    {
        if (SSLSetRootCA(req_p->sslId, req_p->rootCAData, req_p->rootCASize) != 0)
        {
            return -1004;
        }
    }
    else
    {
        if (SSLSetBuiltinRootCA(req_p->sslId, req_p->builtinRootCA) != 0)
        {
            return -1004;
        }
    }
    if (SSLConnect(req_p->sslId, socket) < -1)
    {
        return -1001;
    }

    while (!done)
    {
        NHTTPConnectionInfo* connection =
            NHTTPi_Request2Connection(mutexInfo_p, req_p);
        s32 result = SSLDoHandshake(req_p->sslId);
        NHTTPi_SetSSLError(bgnEndInfo_p, result);
        if (connection != NULL)
        {
            connection->sslError = result;
        }

        switch (result)
        {
        case 0:
            done = TRUE;
            break;
        case -7:
        case -3:
        case -2:
            break;
        default:
            return -1001;
        }
    }
    return 0;
}

s32 NHTTPi_SocRecv_sub(NHTTPConnectionInfo* connection, s32 socket,
    char* buffer, s32 length, s32 flags)
{
    char* recvBuffer = connection->recvBuffer;
    s32 received = 0;
    if (length > 0)
    {
        if (connection->recvBufferRemaining == 0)
        {
            s32 result = SORecv(socket, recvBuffer, sizeof(connection->recvBuffer), flags);
            if (result > 0)
            {
                connection->recvBufferRemaining = result;
                connection->recvBufferOffset = 0;
            }
            else
            {
                return result;
            }
        }
        if (connection->recvBufferRemaining != 0)
        {
            length = MIN(length, connection->recvBufferRemaining);
            NHTTPi_memcpy(buffer, recvBuffer + connection->recvBufferOffset, length);
            connection->recvBufferRemaining -= length;
            if (connection->recvBufferRemaining == 0)
            {
                NHTTPi_memclr(recvBuffer, sizeof(connection->recvBuffer));
                connection->recvBufferOffset = 0;
            }
            else
            {
                connection->recvBufferOffset += length;
            }
            received = length;
        }
    }
    return received;
}

int NHTTPi_SocRecv(void* mutexInfo, NHTTPRequestInfo* req_p, const int socket,
    char* buf_p, const int len, const int flags)
{
    int error;

    if (req_p->sslId > 0)
    {
        error = SSLRead(req_p->sslId, buf_p, len);
    }
    else
    {
        NHTTPConnectionInfo* connection = NHTTPi_Request2Connection(mutexInfo, req_p);
        if (connection != NULL)
        {
            error = NHTTPi_SocRecv_sub(connection, socket, buf_p, len, flags);
        }
        else
        {
            return -1001;
        }
    }
    if (error < 0)
    {
        if (req_p->cancel != FALSE)
        {
            return -1002;
        }
        if (req_p->sslId > 0)
        {
            if (error == -7 || error == -6)
            {
                return 0;
            }
        }
        else if (error == SO_ENOTCONN)
        {
            return 0;
        }
        return -1001;
    }
    return error;
}

s32 NHTTPi_SocSend_sub(s32 socket, const char* i_buf, u32 i_bufSize,
    s32 flags)
{
    s32 ret;
    char tmp[32] ALIGN32;
    u32 align;
    u32 front_bytes;
    s32 rest;
    int total_write;

    align = (u32)i_buf & 31;
    front_bytes = align != 0 ? (32 - align) : 0;
    rest = i_bufSize;
    total_write = 0;
    NHTTPi_memclr(tmp, sizeof(tmp));
    if (front_bytes != 0)
    {
        if (front_bytes > i_bufSize)
        {
            front_bytes = i_bufSize;
        }
        NHTTPi_memcpy(tmp, i_buf, front_bytes);
        ret = SOSend(socket, tmp, front_bytes, flags);
        if (ret > 0)
        {
            total_write += ret;
            if (ret < front_bytes)
            {
                return ret;
            }
            else
            {
                i_buf += ret;
                rest -= ret;
            }
        }
        else
        {
            return ret;
        }
    }
    if (rest > 0)
    {
        u32 body_bytes = rest & ~31;
        if (body_bytes != 0)
        {
            ret = SOSend(socket, i_buf, body_bytes, flags);
            if (ret > 0)
            {
                total_write += ret;
                if (ret < body_bytes)
                {
                    return total_write;
                }
                else
                {
                    i_buf += ret;
                    rest -= ret;
                }
            }
            else
            {
                if (total_write > 0)
                {
                    return total_write;
                }
                else
                {
                    return ret;
                }
            }
        }
    }
    if (rest > 0)
    {
        u32 back_bytes = rest & 31;
        if (back_bytes != 0)
        {
            NHTTPi_memclr(tmp, sizeof(tmp));
            NHTTPi_memcpy(tmp, i_buf, back_bytes);
            ret = SOSend(socket, tmp, back_bytes, flags);
            if (ret > 0)
            {
                total_write += ret;
            }
            else
            {
                if (total_write > 0)
                {
                    return total_write;
                }
                else
                {
                    return ret;
                }
            }
        }
    }
    return total_write;
}

int NHTTPi_SocSend(const NHTTPRequestInfo* req_p, const int socket,
    const char* buf_p, const int len, const int flags)
{
    int error;

    if (req_p->sslId > 0)
    {
        error = SSLWrite(req_p->sslId, buf_p, len);
    }
    else
    {
        error = NHTTPi_SocSend_sub(socket, buf_p, len, flags);
    }
    if (error < 0)
    {
        if (req_p->cancel != FALSE)
        {
            return -1002;
        }
        if (req_p->sslId > 0)
        {
            if (error == -7 || error == -6)
            {
                return 0;
            }
        }
        else if (error == SO_ENOTCONN)
        {
            return 0;
        }
        return -1001;
    }
    return error;
}

void NHTTPi_SocCancel(void* mutexInfo, NHTTPRequestInfo* request, s32 socket)
{
    NHTTPi_lockReqList(mutexInfo);
    if (socket >= 0)
    {
        SOShutdown(socket, 2);
    }
    NHTTPi_unlockReqList(mutexInfo);
}

u32 NHTTPi_resolveHostname(NHTTPRequestInfo* request, const char* hostname)
{
    u32 address;
    SOAddrInfo* result;

    address = 0;
    if (SOGetAddrInfo(hostname, NULL, NULL, &result) == 0)
    {
        NHTTPi_memcpy(&address, (u8*)result->addr + 4, sizeof(address));
        SOFreeAddrInfo(result);
    }
    else
    {
        address = 0;
    }
    return address;
}
