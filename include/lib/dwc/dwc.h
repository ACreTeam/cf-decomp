#ifndef DWC_DWC_H
#define DWC_DWC_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// Nintendo Wi-Fi Connection APIs used by the network manager and worker.
// Names identified from SDK documentation and this binary's debug strings.
// Matchmaking signatures reflect this game's later Wii DWC revision; they
// include arguments absent from the older Nitro / Revolution manuals.

// The console's own Wi-Fi Connection account (dFriendList_c::mUserData).
typedef struct DWCUserData {
    u8 _00[0x40];
} DWCUserData; // size 0x40

// One friend's account data (or a friend key token) (dFriendList_c::mKeys).
typedef struct DWCFriendData {
    u8 _00[0xC];
} DWCFriendData; // size 0xC

// Callback layouts used by this Wii DWC revision.
typedef void *(*DWCAllocEx)(int name, u32 size, int align);
typedef void (*DWCFreeEx)(int name, void *ptr, u32 size);
typedef void (*DWCLoginCallback)(int error, int profileId, void *param);
typedef void (*DWCUpdateServersCallback)(int error, BOOL changed, void *param);
typedef void (*DWCDeleteFriendListCallback)(int deletedIndex, int sourceIndex,
                                            void *param);
typedef void (*DWCBuddyFriendCallback)(int index, void *param);
typedef void (*DWCFriendStatusCallback)(int index, u8 status,
                                        const char *statusString, void *param);
typedef void (*DWCMatchedSCCallback)(int error, BOOL cancel, BOOL self,
                                     BOOL isServer, int index, int profileId,
                                     void *param);
typedef void (*DWCNewClientCallback)(int index, int profileId, void *param);
typedef int (*DWCEvalPlayerCallback)(int index, void *param);
typedef void (*DWCStopSCCallback)(int error, void *param);
typedef void (*DWCConnectionClosedCallback)(int error, BOOL local, BOOL server,
                                            u8 aid, int index, void *param);
typedef void (*DWCUserSendCallback)(u32 size, u8 aid, void *param);
typedef void (*DWCUserRecvCallback)(u8 aid, const void *data, u32 size,
                                    void *param);
typedef void (*DWCUserRecvTimeoutCallback)(u8 aid, void *param);
typedef void (*DWCUserPingCallback)(int latency, u8 aid, void *param);

// common/dwc_init.c, dwc_error.c and dwc_report.c
int DWC_Init(int authServer, const char *gameName, u32 gameCode,
             DWCAllocEx alloc, DWCFreeEx free);
void DWC_Shutdown(void);
void DWC_SetReportLevel(int level);
int DWC_GetLastErrorEx(int *errorCode, int *errorType);
void DWC_ClearError(void);

// match/dwc_main.c
void DWC_InitFriendsMatch(void *control, DWCUserData *userData, int productId,
                          const char *gameName, const char *secretKey,
                          int sendBufSize, int recvBufSize,
                          DWCFriendData *friendList, int friendListLen);
void DWC_ShutdownFriendsMatch(void);
void DWC_ProcessFriendsMatch(void);
BOOL DWC_LoginAsync(const wchar_t *nickname, int reserved,
                    DWCLoginCallback callback, void *param);
BOOL DWC_UpdateServersAsync(int reserved, DWCUpdateServersCallback callback,
                            void *param,
                            DWCDeleteFriendListCallback deleteCallback,
                            void *deleteParam,
                            DWCBuddyFriendCallback buddyCallback,
                            void *buddyParam);
BOOL DWC_ConnectToAnybodyAsync(int mode, int numEntry, const char *filter,
                               DWCMatchedSCCallback matchedCallback,
                               void *matchedParam,
                               DWCNewClientCallback newClientCallback,
                               void *newClientParam, void *reservedCallback,
                               void *reservedParam,
                               DWCEvalPlayerCallback evalCallback,
                               void *evalParam, void *reserved);
BOOL DWC_SetupGameServer(int mode, int maxEntry,
                         DWCMatchedSCCallback matchedCallback,
                         void *matchedParam,
                         DWCNewClientCallback newClientCallback,
                         void *newClientParam,
                         DWCEvalPlayerCallback evalCallback, void *evalParam,
                         void *reserved);
BOOL DWC_ConnectToGameServerAsync(int mode, u32 serverIndex,
                                  DWCMatchedSCCallback matchedCallback,
                                  void *matchedParam,
                                  DWCNewClientCallback newClientCallback,
                                  void *newClientParam,
                                  DWCEvalPlayerCallback evalCallback,
                                  void *evalParam, void *reserved);
BOOL DWC_SetConnectionClosedCallback(DWCConnectionClosedCallback callback,
                                     void *param);
int DWC_CloseAllConnectionsHard(void);
int DWC_CloseConnectionHard(u8 aid);
int DWC_GetNumConnectionHost(void);
BOOL DWC_IsValidAID(u8 aid);
u8 DWC_GetServerAID(void);
// TRUE if this console is the matchmaking server
BOOL DWC_IsServerMyself(void);

// match/dwc_match.c
BOOL DWC_CancelMatching(void);
int DWC_AddMatchKeyString(int keyId, const char *keyName, const char *value);
int DWC_GetLastMatchingType(void);
BOOL DWC_RequestSuspendMatchAsync(BOOL suspend, DWCStopSCCallback callback,
                                  void *param);
BOOL DWC_GetSuspendMatch(void);

// match/dwc_common.c and dwc_transport.c
u8 DWC_GetFriendStatusDataSC(DWCFriendData *friendData, u8 *maxEntry,
                             u8 *numEntry, void *statusData, int *size);
BOOL DWC_SetBuddyFriendCallback(DWCBuddyFriendCallback callback, void *param);
BOOL DWC_IsSendableReliable(u8 aid);
BOOL DWC_SendReliable(u8 aid, const void *data, u32 size);
BOOL DWC_Ping(u8 aid);
BOOL DWC_SetUserSendCallback(DWCUserSendCallback callback, void *param);
BOOL DWC_SetUserRecvCallback(DWCUserRecvCallback callback, void *param);
BOOL DWC_SetUserRecvTimeoutCallback(DWCUserRecvTimeoutCallback callback,
                                    void *param);
BOOL DWC_SetUserPingCallback(DWCUserPingCallback callback, void *param);
BOOL DWC_SetSendSplitMax(u16 sendSplitMax);
BOOL DWC_SetConnectionKeepAliveTime(u32 time);

// match/dwc_friend.c
// sets the status data the friends see
void DWC_SetOwnStatusData(const void *data, u32 size);
// TRUE when the friend list may be changed
BOOL DWC_CanChangeFriendList(void);
// removes a friend from the server list
void DWC_DeleteBuddyFriendData(DWCFriendData *friendData);

// match/dwc_main.c
u8 DWC_GetMyAID(void); // own aid
int DWC_GetState(void);

// match/dwc_transport.c
void DWC_SetRecvBuffer(u8 aid, void *buf, u32 size); // receive buffer for aid

// account/dwc_account.c
// TRUE if the data holds a profile ID with the buddy (mutual friend) flag set
BOOL DWC_IsBuddyFriendData(const DWCFriendData *friendData);
// kind: 2 friend key, 3 registered, ...
int DWC_GetFriendDataType(const DWCFriendData *friendData);
void DWC_ClearBuddyFlagFriendData(
    DWCFriendData *friendData); // clears its friend-established flag
// checks a friend key
int DWC_CheckFriendKey(const DWCUserData *userData, u64 friendKey);
// TRUE if it holds data
BOOL DWC_IsValidFriendData(const DWCFriendData *friendData);
void DWC_CreateUserData(DWCUserData *userData);
BOOL DWC_CheckUserData(const DWCUserData *userData);
// Checks the saved game code against the code supplied to DWC_Init.
BOOL DWC_IsCompatibleUserData(const DWCUserData *userData);
void DWC_ClearDirtyFlag(DWCUserData *userData);
// friend key of the data
u64 DWC_GetFriendKey(const DWCFriendData *friendData);
// profile id
int DWC_GetGsProfileId(const DWCUserData *userData,
                       const DWCFriendData *friendData);
// own friend key
u64 DWC_CreateFriendKey(const DWCUserData *userData);
// friend data from a friend key
void DWC_CreateFriendKeyToken(DWCFriendData *friendData, u64 friendKey);
// own friend data
void DWC_CreateExchangeToken(const DWCUserData *userData,
                             DWCFriendData *friendData);
BOOL DWC_IsEqualFriendData(const DWCFriendData *a,
                           const DWCFriendData *b); // same friend
void DWC_ReportUserData(DWCUserData *userData);

#ifdef __cplusplus
}
#endif
#endif
