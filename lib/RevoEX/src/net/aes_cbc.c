// AES (Rijndael) block cipher with CBC chaining, written from the target assembly (RevoEX NET
// library, Jun 9 2009 build). The block functions are table driven with a single rotated table
// per direction; the key schedule follows FIPS-197 and the decryption keys are converted with
// InvMixColumns on first use.
#include <revolution/os.h>
#include <revolution/net/NETAES.h>

#include <private/net.h>

#include <string.h>

void NETiAESEncryptoBlock(NETAESContext* context, void* dst, const void* src) {
    const u32* in = (const u32*)src;
    u32* out = (u32*)dst;
    u32 block[4];

    block[0] = in[0] ^ context->iv[0];
    block[1] = in[1] ^ context->iv[1];
    block[2] = in[2] ^ context->iv[2];
    block[3] = in[3] ^ context->iv[3];
    AESiEncryptBlock(context, out, block);
    context->iv[0] = out[0];
    context->iv[1] = out[1];
    context->iv[2] = out[2];
    context->iv[3] = out[3];
}

void NETiAESDecryptoBlock(NETAESContext* context, void* dst, const void* src) {
    u32 iv0 = context->iv[0];
    u32 iv1 = context->iv[1];
    u32 iv2 = context->iv[2];
    u32 iv3 = context->iv[3];
    const u32* in = (const u32*)src;
    u32* out = (u32*)dst;

    context->iv[0] = in[0];
    context->iv[1] = in[1];
    context->iv[2] = in[2];
    context->iv[3] = in[3];
    AESiDecryptBlock(context, out, in);
    out[0] ^= iv0;
    out[1] ^= iv1;
    out[2] ^= iv2;
    out[3] ^= iv3;
}
