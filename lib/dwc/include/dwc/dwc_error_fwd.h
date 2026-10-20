#pragma once

#include <dwc/dwci_error.h>

typedef DWCErrorType DWCError;

#ifdef __cplusplus
extern "C"
{
#endif

int DWC_GetLastError(int* errorCode);
long DWC_GetLastErrorEx(long* errorCode, DWCErrorType* errorType);
void DWC_ClearError();

#ifdef __cplusplus
}
#endif
