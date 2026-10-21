#pragma once

#include <dwc/dwci_error.h>

// what the application should do about an error (DWC_GetLastErrorEx)
typedef enum {
  DWC_ETYPE_NO_ERROR = 0,
  DWC_ETYPE_LIGHT = 1,
  DWC_ETYPE_SHOW_ERROR = 2,
  DWC_ETYPE_SHUTDOWN_FM = 3,
  DWC_ETYPE_SHUTDOWN_GHTTP = 4,
  DWC_ETYPE_SHUTDOWN_ND = 5,
  DWC_ETYPE_DISCONNECT = 6,
  DWC_ETYPE_FATAL = 7,
  DWC_ETYPE_SHUTDOWN_TAC = 8,
  DWC_ETYPE_NUM = 9
} DWCErrorType;

#ifdef __cplusplus
extern "C"
{
#endif

DWCError DWC_GetLastError(int* errorCode);
DWCError DWC_GetLastErrorEx(int* errorCode, DWCErrorType* errorType);
void DWC_ClearError();

#ifdef __cplusplus
}
#endif
