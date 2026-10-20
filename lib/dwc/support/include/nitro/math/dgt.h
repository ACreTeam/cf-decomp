#ifndef DWC_SUPPORT_NITRO_MATH_DGT_H
#define DWC_SUPPORT_NITRO_MATH_DGT_H

#include <revolution/types.h>

void MATH_CalcMD5(void *digest, const void *data, u32 dataLength);
void MATH_CalcSHA1(void *digest, const void *data, u32 dataLength);

#endif
