#pragma once

#include <dwc/dwc_error_fwd.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*DWCLoginCallback)(DWCError error, int profileID,
    void* param);

#ifdef __cplusplus
}
#endif
