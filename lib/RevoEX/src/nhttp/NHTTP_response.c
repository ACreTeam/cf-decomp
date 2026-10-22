#include <private/nhttp.h>

void NHTTP_DestroyResponse(void* mutexInfo_p, NHTTPResponseInfo* res_p)
{
    NHTTPi_HDRBUFLIST* list_p = NULL;

    while (res_p->hdrBufBlock_p)
    {
        list_p = res_p->hdrBufBlock_p->next_p;
        NHTTPi_free(res_p->hdrBufBlock_p);
        res_p->hdrBufBlock_p = list_p;
    }

    if (res_p->allHeader_p)
    {
        NHTTPi_free(res_p->allHeader_p);
    }

    if (res_p->foundHeader_p)
    {
        NHTTPi_free(res_p->foundHeader_p);
    }

    if (res_p->freeBuf != NULL)
    {
        res_p->freeBuf(res_p->recvBuf_p, NHTTPi_free, res_p->param_p);
        res_p->recvBuf_p = NULL;
        res_p->recvBufLen = 0;
    }

    {
        NHTTPConnectionInfo* connection_p =
            NHTTPi_Response2Connection(mutexInfo_p, res_p);

        if (connection_p != NULL)
        {
            connection_p->response = NULL;
        }
    }

    NHTTPi_free(res_p);
}

s32 NHTTPi_getHeaderValue(const NHTTPResponseInfo* res_p, const char* label_p,
    s32* pos_p)
{
    s32 linetop, nextlinetop;
    s32 labeltop, labelend;
    s32 valuetop, valueend;
    s32 return_code_size = 0;

    linetop = NHTTPi_findNextLineHdrRecvBuf(
        res_p, 12, res_p->headerLen, &labelend, &return_code_size);

    while (linetop > 0)
    {
        nextlinetop = NHTTPi_findNextLineHdrRecvBuf(res_p, linetop,
            res_p->headerLen, &labelend, &return_code_size);

        if (labelend > 0)
        {
            labeltop = linetop;

            if (NHTTPi_compareTokenN_HdrRecvBuf(
                    res_p, labeltop, labelend, label_p, 0)
                == 0)
            {
                if (labelend + 1 < res_p->headerLen)
                {
                    valueend = NHTTPi_findNextLineHdrRecvBuf(res_p,
                        labelend + 1, res_p->headerLen, NULL,
                        &return_code_size);
                    if (valueend <= 0)
                    {
                        valueend = res_p->headerLen;
                    }
                    else
                    {
                        if (valueend < return_code_size)
                        {
                            return -1;
                        }
                        valueend -= return_code_size;
                    }

                    valuetop = NHTTPi_skipSpaceHdrRecvBuf(
                        res_p, labelend + 1, valueend);
                    if (valuetop < 0)
                    {
                        valuetop = valueend;
                    }

                    *pos_p = valuetop;
                    return valueend - valuetop;
                }
                else
                {
                    return 0;
                }
            }
        }

        linetop = nextlinetop;
    }

    return -1;
}

s32 NHTTP_GetHeaderField(NHTTPResponseInfo* res_p, const char* label_p,
    char** value_pp)
{
    s32 pos;
    s32 len;

    if (!res_p->isHeaderParse)
    {
        return -1;
    }

    if (res_p->foundHeader_p)
    {
        NHTTPi_free(res_p->foundHeader_p);
        res_p->foundHeader_p = NULL;
    }

    len = NHTTPi_getHeaderValue(res_p, label_p, &pos);
    if (len >= 0)
    {
        res_p->foundHeader_p = NHTTPi_alloc(len + 1, 4);
        if (!res_p->foundHeader_p)
        {
            return -1;
        }
        res_p->foundHeader_p[len] = 0;
        NHTTPi_loadFromHdrRecvBuf(res_p, res_p->foundHeader_p, pos, len);
        *value_pp = res_p->foundHeader_p;
        return len;
    }

    if (NHTTPi_strcmp("HTTPSTATUSCODE", label_p) == 0)
    {
        res_p->foundHeader_p = NHTTPi_alloc(4, 4);
        if (!res_p->foundHeader_p)
        {
            return -1;
        }
        res_p->foundHeader_p[3] = 0;
        NHTTPi_loadFromHdrRecvBuf(res_p, res_p->foundHeader_p, 9, 3);
        *value_pp = res_p->foundHeader_p;
        return 3;
    }

    return -1;
}
