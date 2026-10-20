#pragma once

#include <dwc/dwc_account.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int DWC_SetCommonKeyValueString(const char* key, const char* value,
        char* string, char separator);
    int DWC_AddCommonKeyValueString(const char* key, const char* value,
        char* string, char separator);
    int DWC_GetCommonValueString(const char* key, char* value, const char* string,
        char separator);
    u32 DWCi_GetMathRand32(u32 max);
    u32 DWCi_WStrLen(const u16* string);

#ifdef __cplusplus
}
#endif
