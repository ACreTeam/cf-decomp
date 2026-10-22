#include <private/nhttp.h>
#include <stdio.h>

static BOOL addHdrList(NHTTPHeader** list, NHTTPBgnEndInfo* info, const char* name,
    const char* value)
{
    BOOL found = FALSE;
    NHTTPHeader* header = *list;

    if (header != NULL)
    {
        if (NHTTPi_compareToken(name, header->name) != 0)
        {
            header = header->prev;
            while (header != *list)
            {
                if (NHTTPi_compareToken(name, header->name) == 0)
                {
                    found = TRUE;
                    break;
                }
                header = header->prev;
            }
        }
        else
        {
            found = TRUE;
        }
    }

    if (found)
    {
        header->value = value;
    }
    else
    {
        header = NHTTPi_alloc(sizeof(NHTTPHeader), 4);
        if (header == NULL)
        {
            printf("failed to allocate memory\n");
            NHTTPi_SetError(info, NHTTP_ERROR_ALLOC);
            return FALSE;
        }

        header->name = name;
        header->value = value;
        header->length = 0;
        header->_unk14 = 0;

        if (*list != NULL)
        {
            header->next = (*list)->next;
            header->prev = *list;
            (*list)->next->prev = header;
            (*list)->next = header;
        }
        else
        {
            header->prev = header;
            header->next = header;
            *list = header;
        }
    }

    return TRUE;
}

static char incAscii(u8 c)
{
    c++;
    if (c == 'z' + 1)
    {
        c = '0';
    }
    else if (c == 'Z' + 1)
    {
        c = 'a';
    }
    else if (c == '9' + 1)
    {
        c = 'A';
    }

    return (char)c;
}

static BOOL checkTagPost(NHTTPRequestInfo* req_p, const char* value_p, u32 length)
{
    int n;
    char c;

    if (NHTTPi_memfind(value_p, (int)length,
            &req_p->multipartBoundary[2], LEN_POSTBOUND)
        < 0)
    {
        return TRUE;
    }

    for (n = 2 + LEN_POSTBOUND - 1; n >= 2; n--)
    {
        for (c = req_p->multipartBoundary[n];;)
        {
            c = incAscii((u8)c);
            req_p->multipartBoundary[n] = c;

            if (c == NHTTPi_strMultipartBound[n])
            {
                break;
            }
            if (NHTTPi_memfind(value_p, (int)length,
                    &req_p->multipartBoundary[2], LEN_POSTBOUND)
                < 0)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

NHTTPHeader* NHTTPi_getHdrFromList(NHTTPHeader** list)
{
    NHTTPHeader* header = *list;

    if (header != NULL)
    {
        if (header != header->next)
        {
            header->next->prev = header->prev;
            header->prev->next = header->next;
            *list = header->prev;
        }
        else
        {
            *list = NULL;
        }
    }

    return header;
}

BOOL NHTTP_AddHeaderField(NHTTPRequestInfo* request, NHTTPBgnEndInfo* info,
    const char* name, const char* value)
{
    if (request->state != 0)
    {
        return FALSE;
    }
    return addHdrList(&request->headers, info, name, value);
}

BOOL NHTTP_AddPostDataAscii(NHTTPRequestInfo* req_p,
    NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p)
{
    int rc = FALSE;
    u32 length = 0;

    if (req_p->state != 0)
    {
        return FALSE;
    }
    if (req_p->isRawData)
    {
        printf("already called NHTTPAddPostDataRaw (exclusive fucntion)\n");
        return FALSE;
    }

    if (value_p)
    {
        length = (u32)NHTTPi_strlen(value_p);
    }
    if (checkTagPost(req_p, value_p, length))
    {
        rc = addHdrList(&req_p->postData, bgnEndInfo_p, label_p, value_p);
        if (rc)
        {
            req_p->postData->next->length = length;
        }
    }

    return rc;
}

BOOL NHTTP_AddPostDataBinary(NHTTPRequestInfo* req_p,
    NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p,
    u32 length)
{
    int rc = FALSE;

    if (req_p->state != 0)
    {
        return FALSE;
    }
    if (req_p->isRawData)
    {
        return FALSE;
    }

    if (checkTagPost(req_p, value_p, length))
    {
        rc = addHdrList(&req_p->postData, bgnEndInfo_p, label_p, value_p);
        if (rc)
        {
            req_p->postData->next->length = length;
        }
    }

    return rc;
}

BOOL NHTTP_AddPostDataRaw(NHTTPRequestInfo* req_p, const void* data_p,
    u32 length)
{
    if (req_p->state != 0)
    {
        return FALSE;
    }
    if (req_p->postData != NULL)
    {
        return FALSE;
    }

    req_p->postBuffer = data_p;
    req_p->postBufferSize = length;
    req_p->isRawData = TRUE;
    return TRUE;
}

BOOL NHTTP_SetPostDataEncoding(NHTTPRequestInfo* req_p,
    NHTTPEncodingType encoding)
{
    if (req_p->state != 0)
    {
        return FALSE;
    }

    req_p->encodingType = encoding;
    return TRUE;
}
