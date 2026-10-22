#ifndef PRIVATE_NET_H
#define PRIVATE_NET_H

#include <revolution/types.h>

#include <revolution/net/NETAES.h>

#ifdef __cplusplus
extern "C" {
#endif

// NETAES (aes.c / aes_cbc.c)
void AESiEncryptBlock(NETAESContext* context, u32* out, const u32* in);
void AESiDecryptBlock(NETAESContext* context, u32* out, const u32* in);
void NETiAESEncryptoBlock(NETAESContext* context, void* dst, const void* src);
void NETiAESDecryptoBlock(NETAESContext* context, void* dst, const void* src);

// neterrorcode.c
s32 NETiGetConnectionTypeFromConfigList(u32 wired, u32 wireless, u32 usbap);
s32 GetStartupErrorCode(s32 soErr, s32 connType);

#ifdef __cplusplus
}
#endif

#endif // PRIVATE_NET_H
