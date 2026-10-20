#pragma once

#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int DWC_Base64Encode(const char* src, u32 srclen, char* dst, u32 dstlen);
    int DWC_Base64Decode(const char* src, u32 srclen, char* dst, u32 dstlen);

#ifdef __cplusplus
}
#endif
