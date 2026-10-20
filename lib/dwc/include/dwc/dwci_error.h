#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DWC_ERROR_NONE = 0,
  DWC_ERROR_DS_MEMORY_ANY,
  DWC_ERROR_AUTH_ANY,
  DWC_ERROR_AUTH_OUT_OF_SERVICE,
  DWC_ERROR_AUTH_STOP_SERVICE,
  DWC_ERROR_AC_ANY,
  DWC_ERROR_NETWORK,
  DWC_ERROR_GHTTP_ANY,
  DWC_ERROR_DISCONNECTED,
  DWC_ERROR_FATAL = 9,
  DWC_ERROR_FRIENDS_SHORTAGE = 10,
  DWC_ERROR_NOT_FRIEND_SERVER = 11,
  DWC_ERROR_MO_SC_CONNECT_BLOCK = 12,
  DWC_ERROR_SERVER_FULL = 13,
  DWC_ERROR_ND_ANY,
  DWC_ERROR_ND_HTTP,
  DWC_ERROR_SVL_ANY,
  DWC_ERROR_SVL_HTTP,
  DWC_ERROR_PROF_PARSEERR,
  DWC_ERROR_PROF_HTTP,
  DWC_ERROR_NUM,
} DWCErrorType;

//! @brief @return Return if there is an error.
//!
int DWCi_IsError();

//! @brief Set the static error data.
//!
//! @details   Cannot proceed if the last error before this function is called
//! is fatal.
//!
//! @param[in] lastError The last error encountered.
//! @param[in] errorCode Error code.
//!
void DWCi_SetError(int lastError, int errorCode);

#ifdef __cplusplus
}
#endif
