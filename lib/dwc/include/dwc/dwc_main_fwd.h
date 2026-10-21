#pragma once

#include <dwc/dwc_match_fwd.h>
#include <dwc/dwc_account_fwd.h>
#include <dwc/dwc_login_fwd.h>

// Scalar-only APIs usable without importing the SDK's integer typedefs.
#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*DWCConnectionClosedCallback)(int error, int isLocal,
        int isServer, unsigned char aid, int index, void* param);

    int DWC_SetConnectionClosedCallback(DWCConnectionClosedCallback callback,
        void* param);
    int DWC_ConnectToAnybodyAsync(int mode, unsigned char numEntry,
        const char* addFilter, DWCMatchedSCCallback matchedCallback,
        void* matchedParam, DWCNewClientCallback newClientCallback,
        void* newClientParam, unsigned long param8, unsigned long param9,
        DWCEvalPlayerCallback evalCallback, const unsigned long* evalData,
        void* evalParam);
    int DWC_SetupGameServer(int mode, unsigned char maxEntry,
        DWCMatchedSCCallback matchedCallback, void* matchedParam,
        DWCNewClientCallback newClientCallback, void* newClientParam,
        DWCEvalPlayerCallback evalCallback, const unsigned long* evalData,
        void* evalParam);
    int DWC_ConnectToGameServerAsync(int mode, int serverIndex,
        DWCMatchedSCCallback matchedCallback, void* matchedParam,
        DWCNewClientCallback newClientCallback, void* newClientParam,
        DWCEvalPlayerCallback evalCallback, const unsigned long* evalData,
        void* evalParam);
    int DWC_ConnectToGameServerByGroupID(int mode, unsigned long groupID,
        DWCMatchedSCCallback matchedCallback, void* matchedParam,
        DWCNewClientCallback newClientCallback, void* newClientParam,
        DWCEvalPlayerCallback evalCallback, const unsigned long* evalData,
        void* evalParam);

    typedef void* DWCFriendsMatchControl;

    void DWC_InitFriendsMatch(DWCFriendsMatchControl* dwccnt,
        DWCUserData* userdata, int productID, const char* gameName,
        const char* secretKey, int sendBufSize, int recvBufSize,
        DWCFriendData* friendList, int friendListLen);
    int DWC_LoginAsync(const unsigned short* ingamesn, const char* reserved,
        DWCLoginCallback callback, void* param);

    void DWC_ShutdownFriendsMatch(void);
    void DWC_ProcessFriendsMatch(void);
    int DWC_CloseAllConnectionsHard(void);
    int DWC_CloseConnectionHard(unsigned char aid);
    int DWC_GetNumConnectionHost(void);
    unsigned char DWC_GetMyAID(void);
    int DWC_GetAIDList(unsigned char** aidList);
    int DWC_GetLastSocketError(void);

#ifdef __cplusplus
}
#endif
