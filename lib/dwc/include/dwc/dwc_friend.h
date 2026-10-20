#pragma once

#include <dwc/dwc_common.h>
#include <dwc/dwc_error.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DWC_GP_PROCESS_INTERVAL 300
#define DWC_FRIEND_UPDATE_WAIT_COUNT 7
#define DWC_GP_SSTR_KEY_MATCH_SC_MAX "SCM"
#define DWC_GP_SSTR_KEY_MATCH_SC_NUM "SCN"
#define DWC_GP_SSTR_KEY_MATCH_VERSION "VER"
#define DWC_GP_SSTR_KEY_MATCH_FRIEND_NUM "FME"
#define DWC_GP_SSTR_KEY_DISTANT_FRIEND "MDF"
#define DWC_GP_STATUS_NO_CHANGE -1

    typedef void (*DWCBuddyFriendCallback)(int index, void* param);
    typedef void (*DWCFriendStatusCallback)(int index, u8 status,
        const char* statusString, void* param);
    typedef void (*DWCUpdateServersCallback)(int error, BOOL isChanged,
        void* param);
    typedef void (*DWCDeleteFriendListCallback)(int deletedIndex,
        int srcIndex, void* param);
    typedef enum DWCFriendState
    {
        DWC_FRIEND_STATE_INIT,
        DWC_FRIEND_STATE_PERS_LOGIN,
        DWC_FRIEND_STATE_LOGON,
        DWC_FRIEND_STATE_NUM
    } DWCFriendState;

    enum
    {
        DWC_BUDDY_UPDATE_STATE_WAIT,
        DWC_BUDDY_UPDATE_STATE_CHECK,
        DWC_BUDDY_UPDATE_STATE_PSEARCH,
        DWC_BUDDY_UPDATE_STATE_COMPLETE,
        DWC_BUDDY_UPDATE_STATE_NUM
    };

    enum
    {
        DWC_STATUS_OFFLINE,
        DWC_STATUS_ONLINE,
        DWC_STATUS_PLAYING,
        DWC_STATUS_MATCH_ANYBODY,
        DWC_STATUS_MATCH_FRIEND,
        DWC_STATUS_MATCH_SC_CL,
        DWC_STATUS_MATCH_SC_SV,
        DWC_STATUS_NUM
    };

    u8 DWC_GetFriendStatus(const DWCFriendData* friendData, char* statusString);
    u8 DWC_GetFriendStatusSC(const DWCFriendData* friendData, u8* maxEntry,
        u8* numEntry, char* statusString);
    u8 DWC_GetFriendStatusData(const DWCFriendData* friendData,
        char* statusData, int* size);
    u8 DWC_GetFriendStatusDataSC(const DWCFriendData* friendData, u8* maxEntry,
        u8* numEntry, char* statusData, int* size);
    int DWC_GetNumFriend(const DWCFriendData friendList[], int friendListLen);
    BOOL DWC_SetOwnStatusString(const char* statusString);
    BOOL DWC_GetOwnStatusString(char* statusString);
    BOOL DWC_SetOwnStatusData(const char* statusData, u32 size);
    int DWC_GetOwnStatusData(char* statusData);
    BOOL DWC_CanChangeFriendList(void);
    void DWC_DeleteBuddyFriendData(DWCFriendData* friendData);
    BOOL DWC_SetBuddyFriendCallback(DWCBuddyFriendCallback callback, void* param);
    BOOL DWC_SetFriendStatusCallback(DWCFriendStatusCallback callback, void* param);
    BOOL DWC_UpdateServersAsync(const char* reserved,
        DWCUpdateServersCallback updateCallback, void* updateParam,
        DWCFriendStatusCallback statusCallback, void* statusParam,
        DWCDeleteFriendListCallback deleteCallback, void* deleteParam);
    void DWCi_StopFriendProcess(DWCError error, int errorCode);
    void DWCi_FriendProcess(void);
    int DWCi_GetProfileIDFromList(int index);
    int DWCi_GetFriendListLen(void);
    void DWCi_UpdateServersAsync(const char* authToken,
        const char* partnerChallenge, DWCUpdateServersCallback updateCallback,
        void* updateParam, DWCFriendStatusCallback statusCallback,
        void* statusParam, DWCDeleteFriendListCallback deleteCallback,
        void* deleteParam);
    int DWCi_GetFriendListIndex(int profileID);

#ifdef __cplusplus
}
#endif
