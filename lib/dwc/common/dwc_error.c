#include <dwc/dwc_error.h>

//! @brief The last error code encountered.
//!
static int stDwcErrorCode;

//! @brief The last error encountered.
//!
static DWCError stDwcLastError;

DWCError DWC_GetLastError(int* errorCode) {
  if (errorCode)
    *errorCode = stDwcErrorCode;
  return stDwcLastError;
}

DWCError DWC_GetLastErrorEx(int* errorCode, DWCErrorType* errorType) {
  if (errorCode)
    *errorCode = stDwcErrorCode;
  if (errorType) {
    switch (stDwcLastError) {
    case DWC_ERROR_AUTH_ANY:
    case DWC_ERROR_AUTH_OUT_OF_SERVICE:
    case DWC_ERROR_AUTH_STOP_SERVICE:
    case DWC_ERROR_AC_ANY:
    case DWC_ERROR_DISCONNECTED:
      *errorType = DWC_ETYPE_DISCONNECT;
      break;
    case DWC_ERROR_NETWORK:
      *errorType = DWC_ETYPE_SHUTDOWN_FM;
      break;
    case DWC_ERROR_NETWORK_LIGHT:
      *errorType = DWC_ETYPE_SHOW_ERROR;
      break;
    case DWC_ERROR_GHTTP_ANY:
      *errorType = DWC_ETYPE_SHUTDOWN_GHTTP;
      break;
    case DWC_ERROR_FRIENDS_SHORTAGE:
    case DWC_ERROR_NOT_FRIEND_SERVER:
    case DWC_ERROR_SERVER_FULL:
    case DWC_ERROR_SC_CONNECT_BLOCK:
      *errorType = DWC_ETYPE_LIGHT;
      break;
    case DWC_ERROR_DS_MEMORY_ANY:
    case DWC_ERROR_FATAL:
      *errorType = DWC_ETYPE_FATAL;
      break;
    case DWC_ERROR_ND_ANY:
      *errorType = DWC_ETYPE_SHUTDOWN_ND;
      break;
    case DWC_ERROR_ND_HTTP:
    case DWC_ERROR_SVL_HTTP:
    case DWC_ERROR_PROF_HTTP:
      *errorType = DWC_ETYPE_DISCONNECT;
      break;
    case DWC_ERROR_SVL_ANY:
    case DWC_ERROR_DB_ANY:
      *errorType = DWC_ETYPE_SHOW_ERROR;
      break;
    case DWC_ERROR_PROF_PARSEERR:
      *errorType = DWC_ETYPE_LIGHT;
      break;
    case DWC_ERROR_TAC_ANY:
      *errorType = DWC_ETYPE_SHOW_ERROR;
      break;
    case DWC_ERROR_TAC_HTTP:
      *errorType = DWC_ETYPE_DISCONNECT;
      break;
    case DWC_ERROR_TAC_NWC24:
      *errorType = DWC_ETYPE_SHUTDOWN_TAC;
      break;
    default:
      *errorType = DWC_ETYPE_NO_ERROR;
      break;
    }
  }
  return stDwcLastError;
}

void DWC_ClearError() {
  if (stDwcLastError != DWC_ERROR_FATAL) {
    stDwcLastError = DWC_ERROR_NONE;
    stDwcErrorCode = 0;
  }
}

int DWCi_IsError() { return stDwcLastError != DWC_ERROR_NONE; }

void DWCi_SetError(DWCError error, int errorCode) {
  if (stDwcLastError != DWC_ERROR_FATAL) {
    stDwcLastError = error;
    stDwcErrorCode = errorCode;
  }
}
