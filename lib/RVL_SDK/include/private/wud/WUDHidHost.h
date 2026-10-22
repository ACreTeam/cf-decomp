#ifndef PRIVATE_WUD_HID_HOST_H
#define PRIVATE_WUD_HID_HOST_H

#include <private/bte/bta_hh_api.h>

#ifdef __cplusplus
extern "C" {
#endif

void WUDHidHostCallback(tBTA_HH_EVT event, tBTA_HH* pData);

#ifdef __cplusplus
}
#endif

#endif // PRIVATE_WUD_HID_HOST_H
