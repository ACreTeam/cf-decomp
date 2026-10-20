#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum DWCIngamesnCheckResult
{
    DWC_INGAMESN_NOT_CHECKED = 0,
    DWC_INGAMESN_VALID = 1,
    DWC_INGAMESN_INVALID = 2
} DWCIngamesnCheckResult;

DWCIngamesnCheckResult DWC_GetIngamesnCheckResult(void);

#ifdef __cplusplus
}
#endif
