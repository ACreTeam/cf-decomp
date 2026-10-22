#include <revolution/net/NETDigest.h>

#include <revolution/os.h>

#include <string.h>

static void NETHMACiInitPad(NETHMACContext* context, u8 pad)
{
    NETHashInterface* hif = context->hashInterface;
    void* ctx = context->hashContext.buffer;
    u32 keylen = context->keyLength;
    u8 padkey[64];
    u32 i;

    for (i = 0; i < keylen; i++)
    {
        padkey[i] = context->key.buffer[i] ^ pad;
    }
    memset(&padkey[keylen], pad, hif->blockLength - keylen);

    hif->Init(ctx);
    hif->Update(ctx, padkey, hif->blockLength);
}

void NETHMACInit(NETHMACContext* context, const NETHashInterface* hashInterface,
    void* key, u32 keyLength)
{
    NETHashInterface* hif = context->hashInterface;
    void* ctx = context->hashContext.buffer;

    *hif = *hashInterface;

    if (hif->workLength > sizeof(context->hashContext) || hif->blockLength > sizeof(context->key))
    {
        OSReport("%s(%d):[warning in %s]", "hmac.c", 100, __func__);
        OSReport("specified interface needs too large workmemory.");
        OSReport("\n");
        return;
    }

    if (keyLength <= hif->blockLength)
    {
        memcpy(context->key.buffer, key, keyLength);
        context->keyLength = keyLength;
    }
    else
    {
        hif->Init(ctx);
        hif->Update(ctx, key, keyLength);
        hif->GetDigest(ctx, context->key.buffer);
        context->keyLength = hif->hashLength;
    }

    NETHMACiInitPad(context, 0x36);
}

void NETHMACUpdate(NETHMACContext* context, void* message, u32 length)
{
    NETHashInterface* hif = context->hashInterface;
    void* ctx = context->hashContext.buffer;

    hif->Update(ctx, message, length);
}

void NETHMACGetDigest(NETHMACContext* context, void* digest)
{
    NETHashInterface* hif = context->hashInterface;
    void* ctx = context->hashContext.buffer;
    u8 ihash[64];

    hif->GetDigest(ctx, ihash);

    NETHMACiInitPad(context, 0x5C);
    hif->Update(ctx, ihash, hif->hashLength);
    hif->GetDigest(ctx, digest);
}
