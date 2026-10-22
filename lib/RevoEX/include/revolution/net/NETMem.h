#ifndef RVL_SDK_NET_MEM_H
#define RVL_SDK_NET_MEM_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void* NETMemCpy(void* dst, const void* src, u32 size);
void* NETMemSet(void* dst, int value, u32 size);

#ifdef __cplusplus
}
#endif

#endif
