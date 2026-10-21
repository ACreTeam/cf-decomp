#pragma once

#include <dwc/dwc_init.h>
#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nastime.h>
#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C"
{
#endif

    BOOL DWCi_Auth_InitInterface(DWC_AuthServer authServer);
    DWC_AuthServer DWCi_Auth_GetServer(void);
    int DWCi_Auth_StartAuthentication(const u16* ingamesn, const char* gsbrcd,
        u64 userid, DWCAllocFunc allocator, DWCFreeFunc deallocator);
    BOOL DWCi_Auth_Svl_StartAuthentication(
        const char* svc, DWCAllocFunc allocator, DWCFreeFunc deallocator);
    BOOL DWCi_Auth_Prof_StartAuthentication(const u16* words, int words_bytes,
        char region, DWCAllocFunc allocator, DWCFreeFunc deallocator);
    void DWCi_Auth_AbortAuthentication(void);
    void DWCi_Auth_ProcessAuthentication(void);
    BOOL DWCi_Auth_IsFinished(void);
    BOOL DWCi_Auth_IsSucceeded(void);
    void DWCi_Auth_GetGameSpyToken(char* authToken, char* partnerChallenge);
    int DWCi_Auth_GetErrorCode(void);
    DWCIngamesnCheckResult DWCi_Auth_GetIngamesnCheckResult(void);
    u64 DWCi_Auth_GetConsoleUserId(void);
    void DWCi_Auth_UseGameSaveUserId(BOOL use);
    BOOL DWCi_Auth_GetServerTimeOffsetInSeconds(OSTime* offsetSeconds);

#ifdef __cplusplus
}
#endif
