#ifndef REVOLUTION_NET_NETAES_H
#define REVOLUTION_NET_NETAES_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NETAESContext;

// inferred: block function of a chaining mode
typedef void (*NETAESBlockFunc)(struct NETAESContext* context, void* dst, const void* src);

// inferred: chaining mode (encrypt/decrypt functions)
typedef struct NETAESMode {
    NETAESBlockFunc encrypt; // 0x00
    NETAESBlockFunc decrypt; // 0x04
} NETAESMode;

// inferred: context layout (0x108 bytes)
typedef struct NETAESContext {
    u32 iv[4];                // 0x000
    u32 roundKey[60];         // 0x010
    u32 rounds : 8;           // 0x100
    u32 needInvKey : 1;       // 0x100 decryption keys not prepared yet
    u32 : 23;
    const NETAESMode* mode;   // 0x104
} NETAESContext;

BOOL NETAESCreateEx(NETAESContext* context, const void* key, u32 keyLength, const void* iv, const NETAESMode* mode);
BOOL NETAESCreate(NETAESContext* context, const void* key, u32 keyLength, const void* iv);
void NETAESDelete(NETAESContext* context);
BOOL NETAESEncrypt(NETAESContext* context, void* dst, const void* src, s32 length);
BOOL NETAESDecrypt(NETAESContext* context, void* dst, const void* src, s32 length);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_NET_NETAES_H
