#include <revolution/net/NETDigest.h>

#include <string.h>

static void NETSHA1iProcessBlock(NETSHA1Context* context);

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static const NETHashInterface sha1template = {
    20,
    64,
    sizeof(NETSHA1Context),
    0,
    (NETHashInitFunc)NETSHA1Init,
    (NETHashUpdateFunc)NETSHA1Update,
    (NETHashGetDigestFunc)NETSHA1GetDigest,
};

static const u8 padding[8] = {0x80};
static const u8 zeros[8];

void NETSHA1Init(NETSHA1Context* context)
{
    context->blocks_low = 0;
    context->blocks_high = 0;
    context->pool = 0;
    context->h[0] = 0x67452301;
    context->h[1] = 0xefcdab89;
    context->h[2] = 0x98badcfe;
    context->h[3] = 0x10325476;
    context->h[4] = 0xc3d2e1f0;
}

void NETSHA1Update(NETSHA1Context* context, void* input, u32 length)
{
    u32 n;

    while (length != 0)
    {
        n = 64 - context->pool;
        if (n > length)
        {
            n = length;
        }
        memcpy(&context->block[context->pool], input, n);
        input = (u8*)input + n;
        length -= n;
        context->pool += n;
        if (context->pool >= 64)
        {
            NETSHA1iProcessBlock(context);
            context->pool = 0;
            context->blocks_low++;
            if (context->blocks_low == 0)
            {
                context->blocks_high++;
            }
        }
    }
}

void NETSHA1GetDigest(NETSHA1Context* context, void* digest)
{
    u32 bits[2];
    u32 fill;
    u32 space;
    u32 n;

    bits[1] = (context->blocks_low << 9) + (context->pool << 3);
    bits[0] = (context->blocks_high << 9) + (context->blocks_low >> 23);

    NETSHA1Update(context, (void*)padding, 1);

    space = 64 - context->pool;
    if (space < 8)
    {
        NETSHA1Update(context, (void*)zeros, space);
    }

    fill = 56 - context->pool;
    while (fill != 0)
    {
        n = 64 - context->pool;
        if (n > fill)
        {
            n = fill;
        }
        memset(&context->block[context->pool], 0, n);
        fill -= n;
        context->pool += n;
        if (context->pool >= 64)
        {
            NETSHA1iProcessBlock(context);
            context->pool = 0;
            context->blocks_low++;
            if (context->blocks_low == 0)
            {
                context->blocks_high++;
            }
        }
    }

    NETSHA1Update(context, bits, 8);
    memcpy(digest, context, 20);
}

const NETHashInterface* NETGetSHA1Interface(void)
{
    return &sha1template;
}

static void NETSHA1iProcessBlock(NETSHA1Context* context)
{
    u32 w[16];
    u32 a, b, c, d, e;
    u32 f;
    int t;

    a = context->h[0];
    b = context->h[1];
    c = context->h[2];
    d = context->h[3];
    e = context->h[4];

    for (t = 0; t < 16; t++)
    {
        w[t] = ((u32*)context->block)[t];
    }

    for (t = 0; t < 20; t++)
    {
        f = ((b & c) | (~b & d)) + 0x5a827999;
        if (t >= 16)
        {
            w[t & 15] = ROTATE_LEFT(w[(t - 16) & 15] ^ w[(t - 14) & 15] ^ w[(t - 8) & 15] ^ w[(t - 3) & 15], 1);
        }
        f = ROTATE_LEFT(a, 5) + (f + e + w[t & 15]);
        e = d;
        d = c;
        c = ROTATE_LEFT(b, 30);
        b = a;
        a = f;
    }
    for (; t < 40; t++)
    {
        f = (b ^ c ^ d) + 0x6ed9eba1;
        if (t >= 16)
        {
            w[t & 15] = ROTATE_LEFT(w[(t - 16) & 15] ^ w[(t - 14) & 15] ^ w[(t - 8) & 15] ^ w[(t - 3) & 15], 1);
        }
        f = ROTATE_LEFT(a, 5) + (f + e + w[t & 15]);
        e = d;
        d = c;
        c = ROTATE_LEFT(b, 30);
        b = a;
        a = f;
    }
    for (; t < 60; t++)
    {
        f = ((b & c) | (b & d) | (c & d)) + 0x8f1bbcdc;
        if (t >= 16)
        {
            w[t & 15] = ROTATE_LEFT(w[(t - 16) & 15] ^ w[(t - 14) & 15] ^ w[(t - 8) & 15] ^ w[(t - 3) & 15], 1);
        }
        f = ROTATE_LEFT(a, 5) + (f + e + w[t & 15]);
        e = d;
        d = c;
        c = ROTATE_LEFT(b, 30);
        b = a;
        a = f;
    }
    for (; t < 80; t++)
    {
        f = (b ^ c ^ d) + 0xca62c1d6;
        if (t >= 16)
        {
            w[t & 15] = ROTATE_LEFT(w[(t - 16) & 15] ^ w[(t - 14) & 15] ^ w[(t - 8) & 15] ^ w[(t - 3) & 15], 1);
        }
        f = ROTATE_LEFT(a, 5) + (f + e + w[t & 15]);
        e = d;
        d = c;
        c = ROTATE_LEFT(b, 30);
        b = a;
        a = f;
    }

    context->h[0] += a;
    context->h[1] += b;
    context->h[2] += c;
    context->h[3] += d;
    context->h[4] += e;
}
