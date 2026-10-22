#include <private/nhttp.h>

s32 NHTTPi_findNextLineHdrRecvBuf(const NHTTPResponseInfo* recvBuf, s32 start,
    s32 end, s32* separator, s32* lineBreakLength)
{
    s32 c;
    NHTTPi_HDRBUFLIST* block;
    s32 blockOffset;
    s32 i;
    s32 result;
    BOOL foundCR;

    if (separator != NULL)
    {
        *separator = -1;
    }
    if (start < end)
    {
        result = -1;
        foundCR = FALSE;

        if (start < 0x400)
        {
            blockOffset = start;
            block = NULL;
        }
        else
        {
            block = recvBuf->hdrBufBlock_p;
            i = (start - 0x400) >> 9;
            while (i-- != 0)
            {
                block = block->next_p;
            }
            blockOffset = (start - 0x400) & 0x1FF;
        }

        for (i = start; i < end; i++)
        {
            if (block == NULL)
            {
                if (blockOffset < 0x400)
                {
                    c = (s8)recvBuf->hdrBufFirst[blockOffset++];
                    goto check_character;
                }

                block = recvBuf->hdrBufBlock_p;
                blockOffset = 0;
            }
            else if (blockOffset == 0x200)
            {
                blockOffset = 0;
                block = block->next_p;
            }
            c = block->block[blockOffset++];

        check_character:
            if ((s8)c == ':' && separator != NULL && *separator < 0)
            {
                *separator = i;
            }

            if (foundCR)
            {
                if ((s8)c == '\n')
                {
                    result = i == end - 1 ? 0 : i + 1;
                    if (lineBreakLength != NULL)
                    {
                        *lineBreakLength = 2;
                    }
                }
                return result;
            }

            if ((s8)c == '\r')
            {
                result = i == end - 1 ? 0 : i + 1;
                foundCR = TRUE;
                if (lineBreakLength != NULL)
                {
                    *lineBreakLength = 1;
                }
            }

            if ((s8)c == '\n')
            {
                result = i == end - 1 ? 0 : i + 1;
                if (lineBreakLength != NULL)
                {
                    *lineBreakLength = 1;
                }
                return result;
            }
        }
    }

    return -1;
}

s32 NHTTPi_skipSpaceHdrRecvBuf(
    const NHTTPResponseInfo* recvBuf, s32 start, s32 end)
{
    s32 blockOffset;
    NHTTPi_HDRBUFLIST* block;
    s32 i;
    s32 c;

    if (start < end)
    {
        if (start < 0x400)
        {
            blockOffset = start;
            block = NULL;
        }
        else
        {
            block = recvBuf->hdrBufBlock_p;
            i = (start - 0x400) >> 9;
            while (i-- != 0)
            {
                block = block->next_p;
            }
            blockOffset = (start - 0x400) & 0x1FF;
        }

        for (i = start; i < end; i++)
        {
            if (block == NULL)
            {
                if (blockOffset < 0x400)
                {
                    c = (s8)recvBuf->hdrBufFirst[blockOffset++];
                    goto check_space;
                }

                block = recvBuf->hdrBufBlock_p;
                blockOffset = 0;
            }
            else if (blockOffset == 0x200)
            {
                blockOffset = 0;
                block = block->next_p;
            }
            c = block->block[blockOffset++];

        check_space:
            if ((s8)c != ' ')
            {
                return i;
            }
        }
    }

    return -1;
}

s32 NHTTPi_compareTokenN_HdrRecvBuf(const NHTTPResponseInfo* recvBuf, s32 start,
    s32 end, const char* token, s8 terminal)
{
    NHTTPi_HDRBUFLIST* block;
    s32 blockOffset;
    s32 i;
    s32 tokenChar;
    s32 recvChar;

    if (start < end)
    {
        if (start < 0x400)
        {
            blockOffset = start;
            block = NULL;
        }
        else
        {
            block = recvBuf->hdrBufBlock_p;
            i = (start - 0x400) >> 9;
            while (i-- != 0)
            {
                block = block->next_p;
            }
            blockOffset = (start - 0x400) & 0x1FF;
        }

        if (block == NULL)
        {
            if (blockOffset < 0x400)
            {
                recvChar = (s8)recvBuf->hdrBufFirst[blockOffset++];
                goto compare_characters;
            }

            block = recvBuf->hdrBufBlock_p;
            blockOffset = 0;
        }
        else if (blockOffset == 0x200)
        {
            blockOffset = 0;
            block = block->next_p;
        }
        recvChar = block->block[blockOffset++];

    compare_characters:
        i = start;
        while (((((s8)recvChar >= 'A') & ((s8)recvChar <= 'Z'))
                       ? (s8)recvChar + ('a' - 'A')
                       : (s8)recvChar)
               == (((((s8)*token >= 'A') & ((s8)*token <= 'Z'))
                        ? (s8)*token + ('a' - 'A')
                        : (s8)*token)))
        {
            tokenChar = (s8)*token;
            if (tokenChar == '\0' || tokenChar == ' ' || tokenChar == terminal
                || i == end - 1)
            {
                return 0;
            }

            if (block == NULL)
            {
                if (blockOffset < 0x400)
                {
                    recvChar = (s8)recvBuf->hdrBufFirst[blockOffset++];
                    goto advance_token;
                }

                block = recvBuf->hdrBufBlock_p;
                blockOffset = 0;
            }
            else if (blockOffset == 0x200)
            {
                blockOffset = 0;
                block = block->next_p;
            }
            recvChar = block->block[blockOffset++];

        advance_token:
            i++;
            token++;
        }
    }

    return -1;
}

int NHTTPi_loadFromHdrRecvBuf(const NHTTPResponseInfo* res_p, char* dst_p,
    int pos, int len)
{
    NHTTPi_HDRBUFLIST* hdrbuf_p;
    int blockidx;
    int blocklen;

    if ((pos + len) <= (int)(res_p->headerLen))
    {
        if (len)
        {
            if (pos < NHTTP_HDRRECVBUF_INILEN)
            {
                blocklen = MIN(len, NHTTP_HDRRECVBUF_INILEN - pos);
                NHTTPi_memcpy(dst_p, &res_p->hdrBufFirst[pos], blocklen);
                pos += blocklen;
                len -= blocklen;
                dst_p += blocklen;
            }

            if (len)
            {
                pos -= NHTTP_HDRRECVBUF_INILEN;
                blockidx = pos >> NHTTP_HDRRECVBUF_BLOCKSHIFT;
                pos &= NHTTP_HDRRECVBUF_BLOCKMASK;

                for (hdrbuf_p = res_p->hdrBufBlock_p; blockidx--;)
                {
                    hdrbuf_p = hdrbuf_p->next_p;
                }

                while (len)
                {
                    blocklen = MIN(len, NHTTP_HDRRECVBUF_BLOCKLEN - pos);
                    NHTTPi_memcpy(dst_p, &hdrbuf_p->block[pos], blocklen);
                    hdrbuf_p = hdrbuf_p->next_p;
                    pos += blocklen;
                    pos &= NHTTP_HDRRECVBUF_BLOCKMASK;
                    len -= blocklen;
                    dst_p += blocklen;
                }
            }
        }
        return TRUE;
    }

    return FALSE;
}

int NHTTPi_isRecvBufFull(const NHTTPResponseInfo* res_p, const int pos)
{
    return res_p->recvBufLen <= (unsigned long)pos;
}

s32 NHTTPi_RecvBuf(
    void* mutexInfo,
    NHTTPRequestInfo* request, s32 socket, s32 offset,
    s32 flags)
{
    NHTTPResponseInfo* response = request->response;
    return NHTTPi_SocRecv(
        mutexInfo,
        request, socket, &response->recvBuf_p[offset],
        response->recvBufLen - offset, flags);
}

int NHTTPi_RecvBufN(
    void* mutexInfo,
    NHTTPRequestInfo* req_p, const int socket, const int pos,
    int len, const int flags)
{
    NHTTPResponseInfo* res_p = req_p->response;

    if (NHTTPi_isRecvBufFull(res_p, pos))
    {
        return -1003;
    }

    len = len > (int)(res_p->recvBufLen - pos)
        ? (int)(res_p->recvBufLen - pos)
        : len;

    return NHTTPi_SocRecv(
        mutexInfo,
        req_p, socket, &res_p->recvBuf_p[pos], len, flags);
}
