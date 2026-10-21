#pragma once

#include <dwc/dwc_memfunc_fwd.h>

#ifdef __cplusplus
extern "C"
{
#endif

// names from the DWARF of RVLDWC 2.1.3
typedef enum {
  DWC_AUTHSERVER_DEBUG = 0,
  DWC_AUTHSERVER_RELEASE = 1,
  DWC_AUTHSERVER_TEST = 2
} DWC_AuthServer;

int DWC_Init(DWC_AuthServer authserver, const char* gameName, unsigned long gameCode,
             DWCAllocEx alloc, DWCFreeEx free);
void DWC_Shutdown();

#ifdef __cplusplus
}
#endif
