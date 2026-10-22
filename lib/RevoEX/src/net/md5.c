#include <revolution/net/NETDigest.h>

#include <string.h>

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void ProcessBlock(NETMD5Context* context);

static u8 padding = 0x80;

static const NETHashInterface md5template = {
    NET_MD5_DIGEST_SIZE,
    64,
    sizeof(NETMD5Context),
    0,
    (NETHashInitFunc)NETMD5Init,
    (NETHashUpdateFunc)NETMD5Update,
    (NETHashGetDigestFunc)NETMD5GetDigest,
};

static u32 t[64] = {
    0xd76aa478,
    0xe8c7b756,
    0x242070db,
    0xc1bdceee,
    0xf57c0faf,
    0x4787c62a,
    0xa8304613,
    0xfd469501,
    0x698098d8,
    0x8b44f7af,
    0xffff5bb1,
    0x895cd7be,
    0x6b901122,
    0xfd987193,
    0xa679438e,
    0x49b40821,
    0xf61e2562,
    0xc040b340,
    0x265e5a51,
    0xe9b6c7aa,
    0xd62f105d,
    0x02441453,
    0xd8a1e681,
    0xe7d3fbc8,
    0x21e1cde6,
    0xc33707d6,
    0xf4d50d87,
    0x455a14ed,
    0xa9e3e905,
    0xfcefa3f8,
    0x676f02d9,
    0x8d2a4c8a,
    0xfffa3942,
    0x8771f681,
    0x6d9d6122,
    0xfde5380c,
    0xa4beea44,
    0x4bdecfa9,
    0xf6bb4b60,
    0xbebfbc70,
    0x289b7ec6,
    0xeaa127fa,
    0xd4ef3085,
    0x04881d05,
    0xd9d4d039,
    0xe6db99e5,
    0x1fa27cf8,
    0xc4ac5665,
    0xf4292244,
    0x432aff97,
    0xab9423a7,
    0xfc93a039,
    0x655b59c3,
    0x8f0ccc92,
    0xffeff47d,
    0x85845dd1,
    0x6fa87e4f,
    0xfe2ce6e0,
    0xa3014314,
    0x4e0811a1,
    0xf7537e82,
    0xbd3af235,
    0x2ad7d2bb,
    0xeb86d391,
};

static u32 k[48] = {
    1,
    6,
    11,
    0,
    5,
    10,
    15,
    4,
    9,
    14,
    3,
    8,
    13,
    2,
    7,
    12,
    5,
    8,
    11,
    14,
    1,
    4,
    7,
    10,
    13,
    0,
    3,
    6,
    9,
    12,
    15,
    2,
    0,
    7,
    14,
    5,
    12,
    3,
    10,
    1,
    8,
    15,
    6,
    13,
    4,
    11,
    2,
    9,
};

void NETMD5Init(NETMD5Context* context)
{
    context->a = 0x67452301;
    context->b = 0xefcdab89;
    context->c = 0x98badcfe;
    context->d = 0x10325476;
    context->length = 0;
}

void NETMD5Update(NETMD5Context* context, void* input, u32 length)
{
    u32 buffer_index = (u32)context->length;
    u32 buffer_space;
    s32 i;
    u8* p = input;

    context->length += length;
    buffer_index &= 0x3F;
    buffer_space = 64 - buffer_index;

    if (buffer_space > length)
    {
        if (length != 0)
        {
            memcpy(&context->buffer8[buffer_index], p, length);
        }
        return;
    }

    memcpy(&context->buffer8[buffer_index], p, buffer_space);
    ProcessBlock(context);
    p += buffer_space;
    length -= buffer_space;

    i = length >> 6;
    while (i > 0)
    {
        memcpy(context->buffer8, p, 64);
        p += 64;
        ProcessBlock(context);
        i--;
    }

    length &= 0x3F;
    if (length != 0)
    {
        memcpy(context->buffer8, p, length);
    }
}

void NETMD5GetDigest(NETMD5Context* context, void* digest)
{
    u64 total_length = context->length << 3;
    u32 buffer_index;
    u32 buffer_space;
    u32* p = digest;

    NETMD5Update(context, &padding, 1);

    buffer_index = (u32)context->length & 0x3F;
    buffer_space = 64 - buffer_index;
    if (buffer_space < 8)
    {
        memset(&context->buffer8[buffer_index], 0, buffer_space);
        ProcessBlock(context);
        buffer_index = 0;
        buffer_space = 64;
    }
    if (buffer_space > 8)
    {
        memset(&context->buffer8[buffer_index], 0, buffer_space - 8);
    }

    context->buffer32[14] = NETSwapBytes32((u32)total_length);
    context->buffer32[15] = NETSwapBytes32((u32)(total_length >> 32));
    ProcessBlock(context);

    NETWriteSwappedBytes32(p, context->state[0]);
    NETWriteSwappedBytes32(p + 1, context->state[1]);
    NETWriteSwappedBytes32(p + 2, context->state[2]);
    NETWriteSwappedBytes32(p + 3, context->state[3]);
    memset(context, 0, sizeof(*context));
}

const NETHashInterface* NETGetMD5Interface(void)
{
    return &md5template;
}

static inline u32 CalcRound1(u32 a, u32 b, u32 c, u32 d, u32* xp, u32 s, u32 t)
{
    return b + ROTATE_LEFT(a + ((b & c) | (~b & d)) + NETReadSwappedBytes32(xp) + t, s);
}

static inline u32 CalcRound2(u32 a, u32 b, u32 c, u32 d, u32* xp, u32 s, u32 t)
{
    return b + ROTATE_LEFT(a + ((b & d) | (c & ~d)) + NETReadSwappedBytes32(xp) + t, s);
}

static inline u32 CalcRound3(u32 a, u32 b, u32 c, u32 d, u32* xp, u32 s, u32 t)
{
    return b + ROTATE_LEFT(a + (b ^ c ^ d) + NETReadSwappedBytes32(xp) + t, s);
}

static inline u32 CalcRound4(u32 a, u32 b, u32 c, u32 d, u32* xp, u32 s, u32 t)
{
    return b + ROTATE_LEFT(a + (c ^ (b | ~d)) + NETReadSwappedBytes32(xp) + t, s);
}

static void ProcessBlock(NETMD5Context* context)
{
    u32* kp;
    u32 a = context->a;
    u32 b = context->b;
    u32 c = context->c;
    u32 d = context->d;
    u32* x = context->buffer32;
    u32* xp = x;
    u32* tp = t;
    int j;

    for (j = 0; j < 4; j++)
    {
        a = CalcRound1(a, b, c, d, xp++, 7, *tp++);
        d = CalcRound1(d, a, b, c, xp++, 12, *tp++);
        c = CalcRound1(c, d, a, b, xp++, 17, *tp++);
        b = CalcRound1(b, c, d, a, xp++, 22, *tp++);
    }

    kp = k;
    for (j = 0; j < 4; j++)
    {
        a = CalcRound2(a, b, c, d, &x[*kp++], 5, *tp++);
        d = CalcRound2(d, a, b, c, &x[*kp++], 9, *tp++);
        c = CalcRound2(c, d, a, b, &x[*kp++], 14, *tp++);
        b = CalcRound2(b, c, d, a, &x[*kp++], 20, *tp++);
    }

    for (j = 0; j < 4; j++)
    {
        a = CalcRound3(a, b, c, d, &x[*kp++], 4, *tp++);
        d = CalcRound3(d, a, b, c, &x[*kp++], 11, *tp++);
        c = CalcRound3(c, d, a, b, &x[*kp++], 16, *tp++);
        b = CalcRound3(b, c, d, a, &x[*kp++], 23, *tp++);
    }

    for (j = 0; j < 4; j++)
    {
        a = CalcRound4(a, b, c, d, &x[*kp++], 6, *tp++);
        d = CalcRound4(d, a, b, c, &x[*kp++], 10, *tp++);
        c = CalcRound4(c, d, a, b, &x[*kp++], 15, *tp++);
        b = CalcRound4(b, c, d, a, &x[*kp++], 21, *tp++);
    }

    context->a += a;
    context->b += b;
    context->c += c;
    context->d += d;
}
