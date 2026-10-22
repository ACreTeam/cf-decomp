#include <string.h>

#include "vct_main.h"

#include <revolution/net/NETDigest.h>

#define VCT_HMAC_SIZE 16

static char* sHmacKey = "-hC7*:eZ#mo4iD}R";

void vct_add_hmac(void* digest, void* data, u32 size) {
    NETHMACContext context;
    u32 keyLength = strlen(sHmacKey);
    const NETHashInterface* algorithm = NETGetMD5Interface();

    NETHMACInit(&context, algorithm, sHmacKey, keyLength);
    NETHMACUpdate(&context, data, size);
    NETHMACGetDigest(&context, digest);
}

BOOL vct_check_hmac(const void* digest, void* data, u32 size) {
    NETHMACContext context;
    u8 computed[VCT_HMAC_SIZE];
    u32 keyLength = strlen(sHmacKey);
    const NETHashInterface* algorithm = NETGetMD5Interface();

    NETHMACInit(&context, algorithm, sHmacKey, keyLength);
    NETHMACUpdate(&context, data, size);
    NETHMACGetDigest(&context, computed);
    return memcmp(digest, computed, VCT_HMAC_SIZE) == 0;
}
