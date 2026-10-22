#ifndef PRIVATE_NWC24_SYSTEM_H
#define PRIVATE_NWC24_SYSTEM_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24iPrepareShutdown();
NWC24Err NWC24iRequestShutdown(u32 event, NWC24Err* pResult);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_SYSTEM_H
