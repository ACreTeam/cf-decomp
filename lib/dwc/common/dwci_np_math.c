#include <nitro/math/crc.h>
#include <nitro/math/dgt.h>

#include <string.h>

#define ROTL32(value, shift) \
    (((value) << (shift)) | ((value) >> (32 - (shift))))

void MATH_CalcSHA1(void* Message_Digest, const void* buf, u32 size)
{
    unsigned char finalblock[128];
    unsigned int h0;
    unsigned int h1;
    unsigned int h2;
    unsigned int h3;
    unsigned int h4;
    unsigned int tmp;
    int i;
    int len;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int w[80];
    unsigned int* src;
    unsigned int* dst;
    unsigned int f;
    unsigned int g;
    unsigned int h;

    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;
    h4 = 0xc3d2e1f0;

    len = size;
    i = len & 0x3f;
    if (i > 0)
    {
        memcpy(finalblock, (unsigned char*)buf + (len - i), i);
    }
    finalblock[i++] = 0x80;
    if (i <= 56)
    {
        memset(finalblock + i, 0, 56 - i);
        i = 56;
    }
    else
    {
        memset(finalblock + i, 0, 120 - i);
        i = 120;
    }

    size = len << 3;
    finalblock[i++] = 0;
    finalblock[i++] = 0;
    finalblock[i++] = 0;
    finalblock[i++] = 0;
    finalblock[i++] = size >> 24;
    finalblock[i++] = size >> 16;
    finalblock[i++] = size >> 8;
    finalblock[i++] = size;

    len = (i == 64) ? ((len + 8) | 0x3f) : (((len + 8) | 0x3f) + 1);

    while (len > 0)
    {
        switch (len)
        {
        case 63:
            src = (unsigned int*)finalblock;
            break;
        case 64:
            src = (unsigned int*)finalblock + 16;
            break;
        case 128:
            src = (unsigned int*)finalblock;
            break;
        default:
            src = (unsigned int*)buf;
            buf = (unsigned char*)buf + 64;
            break;
        }

        dst = w;
        for (i = 0; i < 16; i++)
        {
            *dst++ = *src++;
        }

        for (i = 16; i < 80; i++)
        {
            tmp = dst[-3] ^ dst[-8] ^ dst[-14] ^ dst[-16];
            *dst++ = ROTL32(tmp, 1);
        }

        a = h0;
        b = h1;
        c = h2;
        d = h3;
        e = h4;
        dst = w;

        g = 0x5a827999;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (~b & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;

        g = 0x6ed9eba1;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;

        g = 0x8f1bbcdc;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = (b & c) | (b & d) | (c & d);
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;

        g = 0xca62c1d6;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;
        f = b ^ c ^ d;
        h = ROTL32(a, 5);
        h += f + e + g + *dst++;
        e = d;
        d = c;
        c = ROTL32(b, 30);
        b = a;
        a = h;

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        len -= 64;
    }

    ((unsigned int*)Message_Digest)[0] = h0;
    ((unsigned int*)Message_Digest)[1] = h1;
    ((unsigned int*)Message_Digest)[2] = h2;
    ((unsigned int*)Message_Digest)[3] = h3;
    ((unsigned int*)Message_Digest)[4] = h4;
}

void MATHi_CRC8InitTable(MATHCRC8Table* table, u8 poly)
{
    u32 r, i, j;
    u8* t = table->table;

    for (i = 0; i < 256; i++)
    {
        r = i;
        for (j = 0; j < 8; j++)
        {
            if (r & 0x80)
            {
                r = (r << 1) ^ poly;
            }
            else
            {
                r <<= 1;
            }
        }
        t[i] = (u8)r;
    }
}

void MATHi_CRC32InitTableRev(MATHCRC32Table* table, u32 poly)
{
    u32 r, i, j;
    u32* t = table->table;

    for (i = 0; i < 256; i++)
    {
        r = i;
        for (j = 0; j < 8; j++)
        {
            if (r & 1)
            {
                r = (r >> 1) ^ poly;
            }
            else
            {
                r >>= 1;
            }
        }
        t[i] = r;
    }
}

void MATHi_CRC8Update(const MATHCRC8Table* table, MATHCRC8Context* context,
    const void* input, u32 length)
{
    u32 r, i;
    const u8* t = table->table;
    u8* data = (u8*)input;

    r = *context;
    for (i = 0; i < length; i++)
    {
        r = t[(r ^ *data) & 0xff];
        data++;
    }
    *context = (MATHCRC8Context)r;
}

void MATHi_CRC32UpdateRev(const MATHCRC32Table* table,
    MATHCRC32Context* context, const void* input,
    u32 length)
{
    u32 r, i;
    const u32* t = table->table;
    u8* data = (u8*)input;

    r = *context;
    for (i = 0; i < length; i++)
    {
        r = (r >> 8) ^ t[(r ^ *data) & 0xff];
        data++;
    }
    *context = r;
}

u8 MATH_CalcCRC8(const MATHCRC8Table* table, const void* data,
    u32 dataLength)
{
    MATHCRC8Context context = 0;
    MATHi_CRC8Update(table, &context, data, dataLength);
    return context;
}

u32 MATH_CalcCRC32(const MATHCRC32Table* table, const void* data,
    u32 dataLength)
{
    MATHCRC32Context context = (MATHCRC32Context)(~0);
    MATHi_CRC32UpdateRev(table, &context, data, dataLength);
    return ~context;
}
