#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// values from the DWARF of RVLDWC 2.1.3 (Tokyo Friend Park II)
typedef enum {
  DWC_ERROR_NONE = 0,
  DWC_ERROR_DS_MEMORY_ANY = 1,
  DWC_ERROR_AUTH_ANY = 2,
  DWC_ERROR_AUTH_OUT_OF_SERVICE = 3,
  DWC_ERROR_AUTH_STOP_SERVICE = 4,
  DWC_ERROR_AC_ANY = 5,
  DWC_ERROR_NETWORK = 6,
  DWC_ERROR_GHTTP_ANY = 7,
  DWC_ERROR_DISCONNECTED = 8,
  DWC_ERROR_FATAL = 9,
  DWC_ERROR_FRIENDS_SHORTAGE = 10,
  DWC_ERROR_NOT_FRIEND_SERVER = 11,
  DWC_ERROR_SERVER_FULL = 12,
  DWC_ERROR_ND_ANY = 13,
  DWC_ERROR_ND_HTTP = 14,
  DWC_ERROR_SVL_ANY = 15,
  DWC_ERROR_SVL_HTTP = 16,
  DWC_ERROR_PROF_PARSEERR = 17,
  DWC_ERROR_PROF_HTTP = 18,
  DWC_ERROR_DB_ANY = 19,
  DWC_ERROR_TAC_ANY = 20,
  DWC_ERROR_TAC_HTTP = 21,
  DWC_ERROR_TAC_NWC24 = 22,
  DWC_ERROR_SC_CONNECT_BLOCK = 23,
  DWC_ERROR_NETWORK_LIGHT = 24,
  DWC_ERROR_NUM = 25
} DWCError;

//! @brief @return Return if there is an error.
//!
int DWCi_IsError();

//! @brief Set the static error data.
//!
//! @details   Cannot proceed if the last error before this function is called
//! is fatal.
//!
//! @param[in] error The error encountered.
//! @param[in] errorCode Error code.
//!
void DWCi_SetError(DWCError error, int errorCode);

#ifdef __cplusplus
}
#endif
