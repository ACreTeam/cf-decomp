#pragma once

#include <dwc/dwc_error.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        DWC_PERS_STATE_INIT,
        DWC_PERS_STATE_LOGIN,
        DWC_PERS_STATE_CONNECTED,
        DWC_PERS_STATE_NUM
    };

    typedef void (*DWCStorageLoginCallback)(DWCError error, void* param);
    typedef void (*DWCSaveToServerCallback)(BOOL success, BOOL isPublic,
        void* param);
    typedef void (*DWCLoadFromServerCallback)(BOOL success, int index,
        char* data, int len, void* param);

    BOOL DWC_LoginToStorageServerAsync(DWCStorageLoginCallback callback,
        void* param);
    void DWC_LogoutFromStorageServer(void);
    BOOL DWC_SetStorageServerCallback(DWCSaveToServerCallback saveCallback,
        DWCLoadFromServerCallback loadCallback);
    BOOL DWC_SavePublicDataAsync(char* keyvalues, void* param);
    BOOL DWC_SavePrivateDataAsync(char* keyvalues, void* param);
    BOOL DWC_LoadOwnPublicDataAsync(char* keys, void* param);
    BOOL DWC_LoadOwnPrivateDataAsync(char* keys, void* param);
    BOOL DWC_LoadOthersDataAsync(char* keys, int index, void* param);

#ifdef __cplusplus
}
#endif
