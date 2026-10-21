#include <dwc/dwc_match.h>
#include <dwc/dwc_common.h>
#include <dwc/dwc_error.h>
#include <dwc/dwc_friend.h>
#include <dwc/dwc_main.h>
#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <dwc/dwc_transport.h>
#include <gamespy/GP/gp.h>
#include <gamespy/gt2/gt2.h>
#include <gamespy/natneg/natneg.h>
#include <gamespy/qr2/qr2.h>
#include <gamespy/serverbrowsing/sb_serverbrowsing.h>
#include <revolution/os/OSTime.h>
#include <stdio.h>
#include <string.h>

#define DWC_MATCH_COMMAND_ADD_MESSAGE_MAX 512

enum
{
    DWC_MATCH_CANCEL_STATE_INIT = 0,
    DWC_MATCH_CANCEL_STATE_EXEC,
    DWC_MATCH_CANCEL_STATE_NUM
};

enum
{
    DWC_MATCH_CLOSE_STATE_INIT = 0,
    DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT,
    DWC_MATCH_CLOSE_STATE_TIMEOUT,
    DWC_MATCH_CLOSE_STATE_FORCED,
    DWC_MATCH_CLOSE_STATE_NUM
};

typedef enum DWCMatchPpConnectionType
{
    DWC_PP_CONNECTION_SV_CONNECT = 0,
    DWC_PP_CONNECTION_CL_GT2_CONNECT,
    DWC_PP_CONNECTION_CL_GT2_ACCEPT,
    DWC_PP_CONNECTION_SYN_FINISH,
    DWC_PP_CONNECTION_NUM
} DWCMatchPpConnectionType;

typedef enum
{
    DWC_MATCH_RESET_ALL = 0,
    DWC_MATCH_RESET_RESTART,
    DWC_MATCH_RESET_CONTINUE,
    DWC_MATCH_RESET_NUM
} DWCMatchResetLevel;

typedef enum
{
    DWC_TOPOLOGY_TYPE_HYBRID = 0,
    DWC_TOPOLOGY_TYPE_STAR = 1,
    DWC_TOPOLOGY_TYPE_FULLMESH = 2,
    DWC_TOPOLOGY_TYPE_NUM = 3
} DWCTopologyType;

typedef struct DWCstNNInfo
{
    u8 isQR2;
    u8 retryCount;
    u16 port;
    u32 ip;
    int cookie;
} DWCNNInfo;

typedef struct DWCstConnectionInfo
{
    u8 index;
    u8 aid;
    u16 reserve;
    void* param;
} DWCConnectionInfo;

typedef struct DWCstMatchCommandControl
{
    u8 command;
    u8 count;
    u16 port;
    u32 ip;
    u32 data[32];
    int profileID;
    int len;
    OSTime sendTime;
} DWCMatchCommandControl;

typedef struct DWCstNodeInfoList
{
    int count;                                 // 0x000
    u8 _04[4];                                 // 0x004
    DWCNodeInfo nodes[DWC_MAX_CONNECTIONS];    // 0x008
} DWCNodeInfoList;

typedef struct DWCstMatchControl
{
    GPConnection* pGpObj;                // 0x000
    GT2Socket* pGt2Socket;               // 0x004
    GT2ConnectionCallbacks* gt2Callbacks; // 0x008
    u8 gt2ConnectCount;                  // 0x00C
    u8 connectedOnceBefore;                 // 0x00D
    u8 gt2NumValidConn;                  // 0x00E
    u8 pad1;                             // 0x00F
    qr2_t qr2Obj;                        // 0x010
    vu8 qr2MatchType;                // 0x014
    vu8 qr2MatchTypeExt;                    // 0x015
    u8 hostState;                              // 0x016
    vu8 qr2NumEntry;                     // 0x017
    vu8 qr2IsReserved;                   // 0x018
    u8 pad19;                  // 0x019
    u8 pad2[2];                          // 0x01A
    u32 suspendMatchBitmap;                             // 0x01C
    OSTime suspendRequestTick; // 20
    u32 suspendMatch;                             // 0x028
    u8 suspend;                              // 0x02C
    u8 suspendServerResponse;                              // 0x02D
    u8 pad2E[2];                         // 0x02E
    u32 suspendCallback;                             // 0x030
    u32 suspendResultParam;                             // 0x034
    OSTime suspendTimeoutTick; // 38
    u32 suspendRetry;                             // 0x040
    u32 isSuspending;                             // 0x044
    u8 qr2ShutdownFlag;                              // 0x048
    u8 pad49[7];                         // 0x049
    DWCNodeInfoList nodeInfoList;        // 0x050
    DWCNodeInfo tempNewNodeInfo;         // 0x658
    DWCNodeInfo tempNewClientInfo;                    // 0x688
    int inMeshMaking;                  // 0x6B8
    u32 lastStateBeforeStopMeshMaking;                            // 0x6BC
    DWCNodeInfo tempNodeInfoBeforeStopMeshMaking;                    // 0x6C0
    u16 qr2Port;                         // 0x6F0
    u8 pad6F2[2];                        // 0x6F2
    u32 qr2IP;                           // 0x6F4
    volatile int qr2Reservation;         // 0x6F8
    ServerBrowser sbObj;                 // 0x6FC
    int sbUpdateFlag;                    // 0x700
    u8 _704[4];                          // 0x704
    s64 sbUpdateTick;                    // 0x708
    s64 sbUpdateRequestTick;             // 0x710
    u8 nnRecvCount;                      // 0x718
    u8 nnFailureCount;                   // 0x719
    u16 nnCookieRand;                    // 0x71A
    int nnLastCookie;                    // 0x71C
    s64 nnFailedTime;                    // 0x720
    s64 nnFinishTime;                    // 0x728
    DWCNNInfo nnInfo;                    // 0x730
    DWCMatchState state;                 // 0x73C
    u8 clLinkProgress;                   // 0x740
    u8 friendCount;                      // 0x741
    u8 distantFriend;                    // 0x742
    u8 resvWaitCount;                    // 0x743
    u8 closeState;                       // 0x744
    u8 cancelState;                      // 0x745
    u8 scResvRetryCount;                 // 0x746
    u8 synResendCount;                   // 0x747
    u8 cancelSynResendCount;             // 0x748
    u8 clWaitTimeoutCount;               // 0x749
    u8 needServerPoll;                       // 0x74A
    u8 pad3;                             // 0x74B
    u16 baseLatency;                     // 0x74C
    u16 cancelBaseLatency;               // 0x74E
    u32 nnResvCheckValue;                        // 0x750
    int cmdResendFlag;                   // 0x754
    s64 cmdResendTick;                   // 0x758
    u32 cmdTimeoutTime;                  // 0x760
    u8 _764[4];                          // 0x764
    s64 cmdTimeoutStartTick;             // 0x768
    u32 synAckBit;                       // 0x770
    u32 cancelSynAckBit;                 // 0x774
    s64 lastSynSent;                     // 0x778
    s64 lastCancelSynSent;               // 0x780
    s64 closedTime;                      // 0x788
    s64 clWaitTime;                      // 0x790
    volatile int profileID;              // 0x798
    int reqProfileID;                    // 0x79C
    int cbEventPid;                      // 0x7A0
    const char* gameName;                // 0x7A4
    const char* secretKey;               // 0x7A8
    const DWCFriendData* friendList;     // 0x7AC
    int friendListLen;                   // 0x7B0
    u8 friendIdxList[DWC_MAX_MATCH_IDX_LIST]; // 0x7B4
    int friendIdxListLen;                // 0x7F4
    DWCMatchCommandControl cmdCnt;       // 0x7F8
    DWCMatchedSCCallback matchedCallback; // 0x890
    void* matchedParam;                  // 0x894
    DWCNewClientCallback newClientCallback; // 0x898
    void* newClientParam;                // 0x89C
    u32 evalCallback;                            // 0x8A0
    u32 evalParam;                            // 0x8A4
    u32 serverDownBuff;                            // 0x8A8
    u32 serverDownBase;                            // 0x8AC
    u32 groupID;                            // 0x8B0
    u8 searchGroupIDRetryHostCount;                             // 0x8B4
    u8 connectGroupID;                             // 0x8B5
    u8 serverDownQueryCount;                             // 0x8B6
    u8 serverDownNakCount;                             // 0x8B7
    OSTime serverPollingTime; // 8B8
    u8 serverFind;                             // 0x8C0
    u8 pad8C1[3];                        // 0x8C1
    DWCTopologyType topologyType;                            // 0x8C4
    u8 denyReason;                             // 0x8C8
    u8 pad8C9[3];                        // 0x8C9
    DWCEvalPlayerCallback connectAttemptCallback;  // 0x8CC
    u32 _8D0;                            // 0x8D0
    void* connectAttemptParam;                     // 0x8D4
    u32 firstServer;                            // 0x8D8
    u32 serverPollingUID;                            // 0x8DC
    OSTime suspendWaitTime; // 8E0
    u32 serverLocked;                            // 0x8E8
    u8 pad8EC[4];                        // 0x8EC
} DWCMatchControl; // size 0x8F0

typedef struct DWCstMatchOptSCBlock
{
    u8 valid;
    u8 lock;
    u16 pad;
} DWCMatchOptSCBlock;

GPResult DWCi_SetGPStatus(int status, const char* statusString,
    const char* locationString);
int DWCi_GetFriendListIndex(int profileID);
void DWCi_StopLogin(DWCError error, int errorCode);

static const int stEvalRate[DWC_SB_UPDATE_MAX_SERVERS] = {
    3,
    3,
    2,
    2,
    1,
    1,
};

void SBServerAddIntKeyValue(SBServer server, const char* keyname, int value);


static void DWCi_NNCompletedCallback(NegotiateResult result, SOCKET gamesocket,
    struct sockaddr_in* remoteaddr, void* userdata);

DWCConnectionInfo* DWCi_GetConnectionInfoByIdx(int index);
DWCAccUserData* DWCi_GetUserData(void);
int DWCi_GetProfileIDFromList(int index);


void DWCi_ResetLastSomeDataRecvTimeByAid(u8 aid);

// the GameSpy calls of this file are tried once on a DNS error (each loop runs once: QR2Startup, SBUpdateAsync,
// NNStartupAsync, SendSBMsgCommand); dwc_friend.c keeps DWC_DNS_ERROR_RETRY_MAX (5)
#define DWC_MATCH_DNS_RETRY_MAX 1

static char* stpAddFilter = NULL;
static int s_sbCallbackLevel = 0;
static BOOL s_needSbFree = FALSE;
// Later DWC only: set around NNThink; a free requested inside its callbacks waits
static BOOL s_nnThinking = FALSE;
static BOOL s_needNnFree = FALSE;
static DWCMatchControl* stpMatchCnt = NULL;
static DWCTick sMatchStateTick; // time of the last match state change

// retry limit of the mesh creation attempts (first .sdata word, 5)
static int stMeshMakeNNRetryMax = 5;

// (-1: alternate the host state filter on every search, otherwise the chance in percent of searching hosts)
static int s_anybodyServerSearchRatio = -1;

static DWCGameMatchKeyData stGameMatchKeys[DWC_QR2_GAME_RESERVED_KEYS];

// a command's data words travel little-endian in SB / QR2 messages (anybody matching, TELL_ADDR to an SC
// client or a distant friend) and in host order in GP buddy messages, where the IP address is byte-swapped instead
static inline BOOL DWCi_IsLEMatchCommand(u8 command)
{
    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY
        || ((stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL || stpMatchCnt->distantFriend)
            && command == DWC_MATCH_COMMAND_TELL_ADDR))
    {
        return TRUE;
    }
    return FALSE;
}

static inline u32 DWCi_MatchDataToLE(u8 command, u32 data)
{
    if (DWCi_IsLEMatchCommand(command) == TRUE)
    {
        return DWCi_HtoLEl(data);
    }
    return data;
}

static inline u32 DWCi_MatchIPToLE(u8 command, u32 ip)
{
    if (DWCi_IsLEMatchCommand(command) == TRUE)
    {
        return ip;
    }
    return DWCi_HtoLEl(ip);
}

// the receiving side; hostOrder is TRUE for a GP buddy message
// if / return, not ?: (it sets case 2's registers in DWCi_ProcessRecvMatchCommand); the send pair alike
static inline u32 DWCi_MatchDataFromLE(BOOL hostOrder, u32 data)
{
    if (hostOrder == TRUE)
    {
        return data;
    }
    return DWCi_LEtoHl(data);
}

static inline u32 DWCi_MatchIPFromLE(BOOL hostOrder, u32 ip)
{
    if (hostOrder == TRUE)
    {
        return DWCi_LEtoHl(ip);
    }
    return ip;
}

// Callback of DWC_RequestSuspendMatchAsync: (result, suspend, param).
typedef void (*DWCiSuspendCallback)(int result, u8 suspend, void* param);

// ---- prototypes (generated from the definitions below) ----
static inline BOOL DWCi_IsLEMatchCommand(u8 command);
static inline u32 DWCi_MatchDataToLE(u8 command, u32 data);
static inline u32 DWCi_MatchIPToLE(u8 command, u32 ip);
static inline u32 DWCi_MatchDataFromLE(BOOL hostOrder, u32 data);
static inline u32 DWCi_MatchIPFromLE(BOOL hostOrder, u32 ip);
void DWCi_NNFreeNegotiateList(void);
BOOL DWC_CancelMatching(void);
u8 DWC_AddMatchKeyString(u8 keyID, const char* keyString, const char* valueSrc);
int DWC_GetLastMatchingType(void);
BOOL DWC_RequestSuspendMatchAsync(BOOL suspend, DWCiSuspendCallback callback, void* param);
static inline BOOL DWCi_IsSuspendMatchReady(void);
BOOL DWC_GetSuspendMatch(void);
void DWCi_MatchInit(DWCMatchControl* matchcnt, GPConnection pGpObj, GT2Socket* pGt2Socket, GT2ConnectionCallbacks* gt2Callbacks, const char* gameName, const char* secretKey, const DWCFriendData friendList[], int friendListLen);
qr2_error_t DWCi_QR2Startup(int profileID);
static int DWCi_GetDefaultMatchFilter(char* filter, int profileID, u8 numEntry, u8 matchType, u8 hostState);
void DWCi_ConnectToAnybodyAsync(int mode, u8 numEntry, const char* addFilter, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, u32 param8, u32 param9, DWCEvalPlayerCallback evalCallback, const u32* evalData, void* evalParam);
void DWCi_SetupGameServer(int mode, u8 maxEntry, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback, const u32* evalData, void* evalParam);
void DWCi_ConnectToGameServerAsync(int mode, int serverPid, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback, const u32* evalData, void* evalParam);
void DWCi_MatchProcess(BOOL fullSpec);
GT2Bool DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, unsigned int ip, unsigned short port, GT2Byte* message, int len);
static inline GPResult DWCi_GPSetServerStatusInline(int status);
static inline BOOL DWCi_CancelPreConnectedServerProcess(int clientPid);
void DWCi_GT2ConnectAttemptCallback(GT2Socket socket, GT2Connection connection, unsigned int ip, unsigned short port, int latency, GT2Byte* message, int len);
void DWCi_GT2ConnectedCallback( GT2Connection connection, GT2Result result, GT2Byte* message, int len);
void DWCi_MatchGPRecvBuddyMsgCallback( GPConnection* connection, u32 profileId, char* message);
void DWCi_StopMatching(DWCError error, int errorCode);
void DWCi_ClearQR2Key(void);
void DWCi_ProcessMatchSynPacket(u8 aid, u16 type, u8* data);
static inline void DWCi_NodeInfo_FromData(DWCNodeInfo* node, const u32* d);
static inline void DWCi_NodeInfoList_InsertToTop(const DWCNodeInfo *node);
static void DWCi_ProcessMatchSynPacketSub_Hybrid(u8 aid, u16 type, u8* data);
static void DWCi_ProcessMatchPollingPacket(const u32* data);
BOOL DWCi_ProcessMatchClosing( DWCError error, int errorCode, int profileID);
void DWCi_ProcessMatchSCClosing(int clientPid);
GPResult DWCi_GPSetServerStatus(int status);
void DWCi_ShutdownMatch(void);
BOOL DWCi_IsShutdownMatch(void);
static void DWCi_ResetMatchParam(DWCMatchResetLevel level);
static void DWCi_SetMatchCommonParam(u8 matchType, u8 numEntry, DWCMatchedSCCallback callback, void* param);
void DWCi_CloseMatch(void);
static SBError DWCi_SBUpdateAsync(int profileID);
static NegotiateError DWCi_NNStartupAsync( int isQR2, int cookie, DWCNodeInfo* node);
static int DWCi_SendMatchCommand(u8 command, int profileID, u32 ip, u16 port, const u32 data[], int len);
static void DWCi_TempNewNodeInfo_Clear(void);
static const char* DWCi_GetResvDenyReasonString(u32 reason);
static BOOL DWCi_ProcessRecvMatchCommand(u8 command, int srcPid, u32 srcIP, u16 srcPort, const u32 data[], int len);
static u8 DWCi_CheckResvCommand(int profileID, u32 qr2IP, u16 qr2Port, u32 matchType, BOOL priorFlag, u32 topology, const void* newClientInfo);
static u8 DWCi_CheckResvCommand_Sub(int profileID, u32 qr2IP, u16 qr2Port, u32 matchType, BOOL priorFlag, u32 topology, const void* newClientInfo);
static int DWCi_ProcessResvOK(int profileID, u32 ip, u16 port);
static int DWCi_SendResvCommand(int profileID, BOOL delay);
static int DWCi_SendResvCommandToFriend(BOOL delay, BOOL init, int resendPid);
static BOOL DWCi_RetryReserving(int resendPid);
static BOOL DWCi_CancelPreConnectedServerProcessWithResetLevel( int clientPid, DWCMatchResetLevel level);
static BOOL DWCi_CancelPreConnectedClientProcess( int serverPid, DWCError error, int errorCode);
static inline void DWCi_NodeInfoList_Append(const DWCNodeInfo* node);
static inline void DWCi_NodeInfoList_InsertFront(const DWCNodeInfo* node);
static void DWCi_PostProcessConnection(DWCMatchPpConnectionType type);
BOOL DWCi_PostProcessSVConnectionHybrid(void);
BOOL DWCi_PostProcessSVConnectionFullmesh(void);
static inline int DWCi_HandleMatchCommandError(int error);
static void DWCi_FinishCancelMatching(void);
static void DWCi_DoCancelMatch(void);
static void DWCi_RestartFromCancel(DWCMatchResetLevel level);
static inline DWCNodeInfo* DWCi_GetNodeInfoForProfileIDOrNew(int profileID);
static BOOL DWCi_CloseShutdownClientSC(u32 bitmap);
static inline void DWCi_SynEmptyPayload(u8* buf);
static void DWCi_SendMatchSynPacket(u8 aid, u16 type);
static BOOL DWCi_ProcessMatchSynTimeout(void);
static inline BOOL DWCi_SendCancelMatchSynCommand(int profileID, u8 command);
static BOOL DWCi_ProcessCancelMatchSynCommand( int profileID, u8 command, u32 data);
static BOOL DWCi_ProcessCancelMatchSynTimeout(void);
static GPResult DWCi_HandleGPError(GPResult result);
static SBError DWCi_HandleSBError(SBError error);
static qr2_error_t DWCi_HandleQR2Error(qr2_error_t error);
static NegotiateError DWCi_HandleNNError(NegotiateError error);
static GT2Result DWCi_HandleGT2Error(GT2Result error);
static BOOL DWCi_CheckDWCServer(SBServer server);
static inline void DWCi_NodeInfoList_AppendNodeInfo(DWCNodeInfo* info);
static void DWCi_SBCallback(ServerBrowser sb, SBCallbackReason reason, SBServer server, void* instance);
static void DWCi_SBPrintServerData(SBServer server);
static inline DWCNodeInfo* DWCi_FindNodeByProfileID(int profileID);
static int DWCi_EvaluateServers(int sort);
static void DWCi_RandomizeServers(void);
static inline const char* DWCi_GetQR2KeyName(int keyid);
static void DWCi_QR2ServerKeyCallback( int keyid, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2PlayerKeyCallback( int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2TeamKeyCallback( int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2KeyListCallback( qr2_key_type keytype, qr2_keybuffer_t keybuffer, void* userdata);
static int DWCi_QR2CountCallback(qr2_key_type keytype, void* userdata);
static void DWCi_QR2AddErrorCallback( qr2_error_t error, gsi_char* errmsg, void* userdata);
static void DWCi_QR2PublicAddrCallback( unsigned int ip, unsigned short port, void* userdata);
static void DWCi_QR2NatnegCallback(int cookie, void* userdata);
void DWCi_HandleGT2UnreliableMatchCommandMessage(char* data, int len);
static void DWCi_QR2ClientMsgCallback( gsi_char* data, int len, void* userdata);
static void DWCi_NNProgressCallback(NegotiateState state, void* userdata);
BOOL DWCi_SendServerDownQuery(void);
BOOL DWCi_IsBackupMine(void);
void DWCi_SetupGameServer_InterChange(BOOL newGroup);
int DWCi_SendServerPolling(int force);
BOOL DWCi_ProcessRequestMatch(u8 aid, int suspend, int mode);
BOOL DWCi_StartMatch(void);
static inline void DWCi_SetSuspendRequest(int suspend, DWCiSuspendCallback callback, void* param);
BOOL DWCi_RequestSuspendMatchAsync(int suspend, DWCiSuspendCallback callback, void* param);
BOOL DWCi_RequestSuspendMatchProcess(void);
BOOL DWCi_RequestSuspendMatch(int suspend);
BOOL DWCi_IsServerChanging(void);
void DWCi_SetMatchStatus(DWCMatchState state);
BOOL DWCi_IsServerChangeEnabled(void);
void DWCi_NodeInfoList_Clear(void);
int DWCi_NodeInfoList_GetCount(void);
BOOL DWCi_NodeInfoList_RemoveAID(u8 aid);
BOOL DWCi_NodeInfoList_RemoveProfileID(int profileID);
DWCNodeInfo* DWCi_NodeInfoList_GetServerNodeInfo(void);
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForIndex(int index);
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForProfileID(int profileID);
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForAID(u8 aid);
u8 DWCi_NodeInfoList_GetFreeAID(void);
OSTime DWCi_GetNextMeshMakeTryTick(void);
BOOL DWCi_StopMeshMaking(void);
BOOL DWCi_IsFullMesh(void);
int DWCi_IsLockedBuddyStatus(GPBuddyStatus* status);
BOOL DWC_RegisterMatchStatus(void);
BOOL DWC_CancelMatchingAsync(void);
BOOL DWC_IsValidCancelMatching(void);
BOOL DWC_StopSCMatchingAsync(DWCStopSCCallback callback, void* param);
u8 DWC_AddMatchKeyInt(u8 keyID, const char* keyString, const int* valueSrc);
int DWC_GetMatchIntValue(int index, const char* keyString, int idefault);
const char* DWC_GetMatchStringValue( int index, const char* keyString, const char* sdefault);
int DWC_SetMatchingOption( DWCMatchOptType opttype, const void* optval, int optlen);
int DWC_GetMatchingOption( DWCMatchOptType opttype, void* optval, int* optlen);
int DWC_GetMOMinCompState(u64* time);
BOOL DWC_GetMOSCConnectBlockState(void);
void DWC_ClearMOSCConnectBlock(void);
DWCMatchState DWC_GetMatchingState(void);
void DWCi_ConnectToFriendsAsync(const u8 friendIdxList[], int friendIdxListLen, u8 numEntry, BOOL distantFriend, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCEvalPlayerCallback evalCallback, void* evalParam);
static void DWCi_SendStateChanged(qr2_t qr2Obj);
static void DWCi_StopResendingMatchCommand(void);
BOOL DWCi_DeleteHostByProfileID(int profileID, int numHost);
int DWCi_DeleteHostByIndex(int index, int numHost);
int DWCi_GetNumAllConnection(void);
int DWCi_GetNumValidConnection(void);
void DWCi_SetNumValidConnection(void);
int DWCi_GetAllAIDList(u8** aidList);
int DWCi_GetValidAIDList(u8** aidList);
static int IsPrivateAddr(u32 addr_in_netorder);
static NegotiateError DWCi_DoNatNegotiationAsync(DWCNNInfo* nnInfo);
static SBError DWCi_SendSBMsgCommand( u8 command, u32 ip, u16 port, const u32 data[], int len);
static GPResult DWCi_SendGPBuddyMsgCommand(GPConnection* connection, u8 command, int profileID, const char* message);
static int DWCi_GetGPBuddyAdditionalMsg( char* dstMsg, const char* srcMsg, int index);
static void DWCi_MakeBackupServerData(int profileID, const u32 data[]);
static int DWCi_CancelReservation(int profileID);
static int DWCi_ChangeToClient(void);
static int DWCi_InvalidateReservation(void);
static void DWCi_RestartFromTimeout(void);
static int DWCi_ResumeMatching(void);
static BOOL DWCi_CloseCancelHostAsync(int profileID);
static void DWCi_CloseAllConnectionsByTimeout(void);
static u8 DWCi_GetNewMatchKey(void);
static void DWCi_ClearGameMatchKeys(void);
static u8 DWCi_GetAIDFromList(void);
static BOOL DWCi_IsFriendByIdxList(int profileID);
static BOOL DWCi_AreAllBuddies(const u32 pidList[], u32 pidListLen);
static u8 DWCi_GetAIDFromProfileID(int profileID, BOOL idx0);
static u32 DWCi_GetAIDBitmask(BOOL valid);
static void DWCi_InitClWaitTimeout(void);
static void DWCi_InitOptMinCompParam(BOOL reset);
static void DWCi_ProcessOptMinComp(void);
static NegotiateResult DWCi_HandleNNResult(NegotiateResult result);
static BOOL DWCi_ProcessNNFailure(BOOL ignoreError);
static DWCMatchControl* DWCi_GetMatchCnt(void);
static inline int DWCi_GetMatchProfileID(void);

// .text:0x8032091C DWCi_NNFreeNegotiateList (0x1C)
// Later DWC only (the unit's first function; dwc_main.c calls it): NNFreeNegotiateList, deferred while NNThink runs
void DWCi_NNFreeNegotiateList(void)
{
    if (!s_nnThinking)
    {
        NNFreeNegotiateList();
    }
    else
    {
        s_needNnFree = TRUE;
    }
}

// .old:DWC_RegisterMatchStatus
BOOL DWC_RegisterMatchStatus(void)
{
    DWC_Printf(4, "!!DWC_RegisterMatchStatus() was called!!\n");

    if (DWCi_GetMatchCnt() == NULL || DWCi_GetMatchCnt()->profileID == 0
        || DWCi_IsError())
    {
        DWC_Printf(4, "But ignored.\n");
        return FALSE;
    }

    if (DWCi_QR2Startup(DWCi_GetMatchCnt()->profileID))
    {
        return FALSE;
    }

    return TRUE;
}

// .text:0x80320938 DWC_CancelMatching (0x90)
BOOL DWC_CancelMatching(void)
{
    if (DWC_IsValidCancelMatching())
    {
        DWCi_DoCancelMatch();
        return TRUE;
    }
    DWC_Printf(4, "Now unable to cancel.\n");
    return FALSE;
}

// .old:DWC_CancelMatchingAsync
BOOL DWC_CancelMatchingAsync(void)
{
    if (DWC_IsValidCancelMatching())
    {
        DWCi_DoCancelMatch();
        return TRUE;
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Now unable to cancel.\n");
        return FALSE;
    }
}

// .old:DWC_IsValidCancelMatching
BOOL DWC_IsValidCancelMatching(void)
{
    if (DWCi_IsError())
    {
        return FALSE;
    }
    if (stpMatchCnt == NULL || DWC_GetState() != DWC_STATE_MATCHING
        || stpMatchCnt->cancelState != 0)
    {
        return FALSE;
    }
    return TRUE;
}

// .old:DWC_AddMatchKeyInt
u8 DWC_AddMatchKeyInt(u8 keyID, const char* keyString, const int* valueSrc)
{
    int index;

    if (!keyString || !valueSrc)
    {
        return 0;
    }

    if (keyID >= DWC_QR2_GAME_KEY_START
        && stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyID)
    {
        if (stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyStr != NULL
            && strcmp(stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyStr,
                   keyString)
                != 0)
        {
            return 0;
        }
    }
    else
    {
        keyID = DWCi_GetNewMatchKey();
        if (!keyID)
        {
            return 0;
        }
    }

    index = keyID - DWC_QR2_GAME_KEY_START;
    stGameMatchKeys[index].keyID = keyID;
    stGameMatchKeys[index].isStr = 0;
    stGameMatchKeys[index].pad = 0;

    if (stGameMatchKeys[index].keyStr)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stGameMatchKeys[index].keyStr, 0);
    }

    stGameMatchKeys[index].keyStr
        = DWC_Alloc(DWC_ALLOCTYPE_BASE, strlen(keyString) + 1);
    if (!stGameMatchKeys[index].keyStr)
    {
        DWCi_StopMatching(DWC_ERROR_FATAL,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
        return 0;
    }

    strcpy(stGameMatchKeys[index].keyStr, keyString);

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "DWC_AddMatchKeyInt: key='%s', value=%d\n", keyString, *valueSrc);

    stGameMatchKeys[index].value = valueSrc;

    qr2_register_keyA(keyID, stGameMatchKeys[index].keyStr);

    return keyID;
}

// .text:0x803209C8 DWC_AddMatchKeyString (0x450)
u8 DWC_AddMatchKeyString(u8 keyID, const char* keyString,
    const char* valueSrc)
{
    int index;

    if (!keyString || !valueSrc)
    {
        return 0;
    }

    if (keyID >= DWC_QR2_GAME_KEY_START
        && stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyID)
    {
        if (stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyStr != NULL
            && strcmp(stGameMatchKeys[keyID - DWC_QR2_GAME_KEY_START].keyStr,
                   keyString)
                != 0)
        {
            return 0;
        }
    }
    else
    {
        keyID = DWCi_GetNewMatchKey();
        if (!keyID)
        {
            return 0;
        }
    }

    index = keyID - DWC_QR2_GAME_KEY_START;
    stGameMatchKeys[index].keyID = keyID;
    stGameMatchKeys[index].isStr = 1;
    stGameMatchKeys[index].pad = 0;

    if (stGameMatchKeys[index].keyStr)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stGameMatchKeys[index].keyStr, 0);
    }

    stGameMatchKeys[index].keyStr
        = DWC_Alloc(DWC_ALLOCTYPE_BASE, strlen(keyString) + 1);
    if (!stGameMatchKeys[index].keyStr)
    {
        DWCi_StopMatching(DWC_ERROR_FATAL,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
        return 0;
    }

    strcpy(stGameMatchKeys[index].keyStr, keyString);

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "DWC_AddMatchKeyString: key='%s' value='%s'\n", keyString, valueSrc);

    stGameMatchKeys[index].value = valueSrc;

    qr2_register_keyA(keyID, stGameMatchKeys[index].keyStr);

    return keyID;
}

// .text:0x80320E18 DWC_GetLastMatchingType (0x1C)
int DWC_GetLastMatchingType(void)
{
    if (DWCi_GetMatchCnt())
    {
        return DWCi_GetMatchCnt()->qr2MatchTypeExt;
    }
    else
    {
        return -1;
    }
}

// .text:0x80320E34 DWC_RequestSuspendMatchAsync (0x70)
BOOL DWC_RequestSuspendMatchAsync(BOOL suspend, DWCiSuspendCallback callback, void* param)
{
    DWC_Printf(4,
        "!!DWC_RequestSuspendMatchAsync(%s) was called!!\n",
        suspend ? "TRUE" : "FALSE");
    return DWCi_RequestSuspendMatchAsync(suspend, callback, param);
}

// .text:0x80320EA4 DWC_GetSuspendMatch (0x114)
// helper: DWCi_IsSuspendMatchReady (inlined in CF)
static inline BOOL DWCi_IsSuspendMatchReady(void)
{
    if (DWC_IsServerMyself())
    {
        if (stpMatchCnt->state == 13 || stpMatchCnt->state == 22)
        {
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

BOOL DWC_GetSuspendMatch(void)
{
    int result = -1;
    u32 bitmap;

    if (!stpMatchCnt)
    {
        return FALSE;
    }
    if (stpMatchCnt->state == 22)
    {
        return TRUE;
    }
    if (!stpMatchCnt->hostState)
    {
        return FALSE;
    }

    bitmap = stpMatchCnt->suspendMatchBitmap;
    bitmap &= DWC_GetAIDBitmap();
    if (!DWCi_IsSuspendMatchReady())
    {
        bitmap &= ~(1 << DWC_GetMyAID());
    }

    if (!(bitmap ^ DWC_GetAIDBitmap()))
    {
        result = 1;
    }
    else if (!(bitmap & DWC_GetAIDBitmap()))
    {
        result = 0;
    }
    if (result != -1)
    {
        stpMatchCnt->suspendMatch = (result == 1);
    }
    return stpMatchCnt->suspendMatch;
}

// .text:0x80320FB8 DWCi_MatchInit (0x2A4)
void DWCi_MatchInit(DWCMatchControl* matchcnt, GPConnection pGpObj,
    GT2Socket* pGt2Socket, GT2ConnectionCallbacks* gt2Callbacks,
    const char* gameName, const char* secretKey,
    const DWCFriendData friendList[], int friendListLen)
{
    stpMatchCnt = matchcnt;
    sMatchStateTick = OSGetTime();
    stpMatchCnt->pGpObj = pGpObj;
    stpMatchCnt->pGt2Socket = pGt2Socket;
    stpMatchCnt->gt2Callbacks = gt2Callbacks;
    stpMatchCnt->qr2Obj = NULL;
    stpMatchCnt->qr2IP = 0;
    stpMatchCnt->qr2Port = 0;
    stpMatchCnt->sbObj = NULL;
    DWCi_SetMatchStatus(DWC_MATCH_STATE_INIT);
    stpMatchCnt->pad19 = 0;
    stpMatchCnt->profileID = 0;
    stpMatchCnt->gameName = gameName;
    stpMatchCnt->secretKey = secretKey;
    stpMatchCnt->friendList = friendList;
    stpMatchCnt->friendListLen = friendListLen;
    memset(stpMatchCnt->friendIdxList, 0, sizeof(stpMatchCnt->friendIdxList));
    stpMatchCnt->friendIdxListLen = 0;
    stpMatchCnt->matchedCallback = NULL;
    stpMatchCnt->matchedParam = NULL;
    stpMatchCnt->evalCallback = 0;
    stpMatchCnt->evalParam = 0;
    stpMatchCnt->serverDownBuff = 0;
    stpMatchCnt->serverDownBase = 0;
    stpMatchCnt->groupID = 0;
    stpMatchCnt->searchGroupIDRetryHostCount = 0;
    stpMatchCnt->connectGroupID = 0;
    stpMatchCnt->qr2MatchTypeExt = 0;
    stpMatchCnt->hostState = 0;
    stpMatchCnt->sbUpdateRequestTick = 0;
    stpMatchCnt->serverPollingUID = 0;
    stpMatchCnt->serverLocked = 1;
    DWC_Printf(4, "ServerBrowserLimitUpdate timeout reset.(%s)\n", __FUNCTION__);

    stpMatchCnt->topologyType = 0;
    memset(&stpMatchCnt->nodeInfoList, 0, sizeof(stpMatchCnt->nodeInfoList));
    memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(stpMatchCnt->tempNewNodeInfo));

    DWCi_ClearGameMatchKeys();

    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);
}

// .text:0x8032125C DWCi_QR2Startup (0x1E4)
qr2_error_t DWCi_QR2Startup(int profileID)
{
    int i;
    qr2_error_t qr2Error = e_qrnoerror;

    if (DWCi_GetMatchCnt()->qr2Obj)
    {
        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "QR2 is already set up.\n");
        return e_qrnoerror;
    }

    DWCi_GetMatchCnt()->profileID = profileID;

    for (i = 0; i < DWC_MATCH_DNS_RETRY_MAX; i++)
    {
        qr2Error = qr2_init_socketA(&DWCi_GetMatchCnt()->qr2Obj,
            gt2GetSocketSOCKET(*DWCi_GetMatchCnt()->pGt2Socket),
            gt2GetLocalPort(*DWCi_GetMatchCnt()->pGt2Socket),
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->secretKey, 1, 1,
            DWCi_QR2ServerKeyCallback, DWCi_QR2PlayerKeyCallback,
            DWCi_QR2TeamKeyCallback, DWCi_QR2KeyListCallback,
            DWCi_QR2CountCallback, DWCi_QR2AddErrorCallback, NULL);
        if (!qr2Error)
        {
            break;
        }

        qr2_shutdown(DWCi_GetMatchCnt()->qr2Obj);
        DWCi_GetMatchCnt()->qr2Obj = NULL;

        if ((qr2Error != e_qrdnserror)
            || (i == DWC_MATCH_DNS_RETRY_MAX - 1))
        {
            DWCi_HandleQR2Error(qr2Error);
            return qr2Error;
        }
    }

    DWCi_GetMatchCnt()->qr2IP = 0;
    DWCi_GetMatchCnt()->qr2Port = 0;

    qr2_register_publicaddress_callback(
        DWCi_GetMatchCnt()->qr2Obj, DWCi_QR2PublicAddrCallback);
    qr2_register_natneg_callback(
        DWCi_GetMatchCnt()->qr2Obj, DWCi_QR2NatnegCallback);
    qr2_register_clientmessage_callback(
        DWCi_GetMatchCnt()->qr2Obj, DWCi_QR2ClientMsgCallback);
    qr2_register_keyA(0x32, "dwc_pid");
    qr2_register_keyA(0x33, "dwc_mtype");
    qr2_register_keyA(0x34, "dwc_mver");
    qr2_register_keyA(0x35, "dwc_eval");
    qr2_register_keyA(0x36, "dwc_groupid");
    qr2_register_keyA(0x37, "dwc_hoststate");
    qr2_register_keyA(0x38, "dwc_suspend");
    qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);

    return qr2Error;
}

// .text:0x80321440 DWCi_ConnectToAnybodyAsync (0x628)
// helper: DWCi_GetDefaultMatchFilter (inlined in CF; the older body kept at the end of the file is replaced by this)
static int DWCi_GetDefaultMatchFilter(
    char* filter, int profileID, u8 numEntry, u8 matchType, u8 hostState)
{
    return snprintf(filter, 0x100,
        "%s = %d and %s != %u and maxplayers = %d and numplayers < %d and %s "
        "= %d and %s = %u and %s = %u",
        DWC_QR2_MATCH_VER_KEY_STR, 90,
        DWC_QR2_PID_KEY_STR, profileID, numEntry, numEntry,
        DWC_QR2_MATCH_TYPE_KEY_STR, matchType,
        "dwc_hoststate", hostState, "dwc_suspend", 0);
}

void DWCi_ConnectToAnybodyAsync(int mode, u8 numEntry, const char* addFilter,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam, u32 param8, u32 param9,
    DWCEvalPlayerCallback evalCallback, const u32* evalData, void* evalParam)
{
    char filter[0x100];
    unsigned long addFilterLenMax;

    if (stpAddFilter != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpAddFilter, 0);
        stpAddFilter = NULL;
    }

    if (addFilter != NULL)
    {
        addFilterLenMax = 0x100
            - DWCi_GetDefaultMatchFilter(filter, -1, 32, 3, 2)
            - strlen(" and ()");
        stpAddFilter = DWC_Alloc(DWC_ALLOCTYPE_BASE, addFilterLenMax);
        if (stpAddFilter == NULL)
        {
            DWCi_StopMatching(DWC_ERROR_FATAL,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
            return;
        }
        DWCi_Np_CPUCopyFast(stpAddFilter, addFilter, addFilterLenMax);
        stpAddFilter[addFilterLenMax - 1] = '\0';
    }

    DWCi_SetMatchCommonParam(0, numEntry, matchedCallback, matchedParam);
    stpMatchCnt->newClientCallback = newClientCallback;
    stpMatchCnt->newClientParam = newClientParam;
    stpMatchCnt->evalCallback = param8;
    stpMatchCnt->evalParam = param9;
    stpMatchCnt->topologyType = mode;
    stpMatchCnt->connectAttemptCallback = evalCallback;
    if (evalData != NULL)
    {
        memcpy(&stpMatchCnt->_8D0, evalData, sizeof(stpMatchCnt->_8D0));
    }
    else
    {
        memset(&stpMatchCnt->_8D0, 0, sizeof(stpMatchCnt->_8D0));
    }
    stpMatchCnt->connectAttemptParam = evalParam;
    DWCi_SetMatchStatus((DWCMatchState)22);

    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNewA(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, 1, SBFalse, DWCi_SBCallback,
            NULL);
    }
    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        if (DWCi_HandleSBError(sbe_allocerror) != 0)
        {
            return;
        }
    }

    (void)DWCi_GPSetServerStatus(3);
    if (DWCi_HandleSBError(DWCi_SBUpdateAsync(stpMatchCnt->profileID)) != 0)
    {
        return;
    }
    if (stpMatchCnt->qr2Obj == NULL)
    {
        DWCi_QR2Startup(stpMatchCnt->profileID);
    }
}

static inline void DWCi_CreateNewGroupID(void) {
    u32 groupID = DWCi_GetMatchCnt()->profileID + DWCi_GetMathRand32(10000000) +
                  100000000;
    DWC_Printf(1, "Create New Group ID:%u\n", groupID);
    DWCi_GetMatchCnt()->groupID = groupID;
}

// .text:0x80321A68 DWCi_SetupGameServer (0x390)
void DWCi_SetupGameServer(int mode, u8 maxEntry,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback,
    const u32* evalData, void* evalParam)
{
    DWCi_SetMatchCommonParam(2, maxEntry, matchedCallback, matchedParam);
    stpMatchCnt->newClientCallback = newClientCallback;
    stpMatchCnt->newClientParam = newClientParam;
    stpMatchCnt->topologyType = mode;
    stpMatchCnt->connectAttemptCallback = evalCallback;
    if (evalData != NULL)
    {
        memcpy(&stpMatchCnt->_8D0, evalData, sizeof(stpMatchCnt->_8D0));
    }
    else
    {
        memset(&stpMatchCnt->_8D0, 0, sizeof(stpMatchCnt->_8D0));
    }
    stpMatchCnt->connectAttemptParam = evalParam;
    DWCi_SetMatchStatus((DWCMatchState)22);

    if (DWCi_HandleGPError(DWCi_GPSetServerStatus(6)) != 0)
    {
        return;
    }

    stpMatchCnt->hostState = 2;
    stpMatchCnt->firstServer = 1;
    if (stpMatchCnt->qr2Obj == NULL)
    {
        DWCi_QR2Startup(stpMatchCnt->profileID);
    }

    DWCi_CreateNewGroupID();

    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNewA(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, 1, SBFalse, DWCi_SBCallback,
            NULL);
    }
    qr2_send_statechanged(stpMatchCnt->qr2Obj);
    if (DWCi_HandleSBError(DWCi_SBUpdateAsync(stpMatchCnt->profileID)) != 0)
    {
        return;
    }
}

// .text:0x80321DF8 DWCi_ConnectToGameServerAsync (0xAD4)
// CF's DWCi_NodeInfoList_GetNodeInfoForProfileID (later in the file, inlined here)
void DWCi_ConnectToGameServerAsync(int mode, int serverPid,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback,
    const u32* evalData, void* evalParam)
{
    DWCNodeInfo savedNewNode;
    DWCNodeInfo ownNode;
    DWCNodeInfo blankNode;
    u8 savedRetryCount;
    int i;
    int result;

    if (stpMatchCnt->connectGroupID)
    {
        savedNewNode = stpMatchCnt->tempNewNodeInfo;
        ownNode = *DWCi_NodeInfoList_GetNodeInfoForProfileID(stpMatchCnt->profileID);
        savedRetryCount = stpMatchCnt->scResvRetryCount;
    }

    DWCi_SetMatchCommonParam(3, 0, matchedCallback, matchedParam);
    stpMatchCnt->topologyType = mode;

    if (!stpMatchCnt->connectGroupID)
    {
        memset(&blankNode, 0, sizeof(blankNode));
        memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(stpMatchCnt->tempNewNodeInfo));
        stpMatchCnt->tempNewNodeInfo.aid = 0xFF;
        stpMatchCnt->tempNewNodeInfo.profileID = serverPid;
    }
    else
    {
        stpMatchCnt->tempNewNodeInfo = savedNewNode;
        if (stpMatchCnt->nodeInfoList.count != DWC_MAX_CONNECTIONS)
        {
            for (i = stpMatchCnt->nodeInfoList.count; i > 0; i--)
            {
                stpMatchCnt->nodeInfoList.nodes[i] = stpMatchCnt->nodeInfoList.nodes[i - 1];
            }
            stpMatchCnt->nodeInfoList.nodes[0] = ownNode;
            stpMatchCnt->nodeInfoList.count++;
        }
        DWCi_Np_CPUCopyFast(&stpMatchCnt->_8D0, ownNode.connUserData, sizeof(stpMatchCnt->_8D0));
        stpMatchCnt->scResvRetryCount = savedRetryCount;
    }

    stpMatchCnt->newClientCallback = newClientCallback;
    stpMatchCnt->newClientParam = newClientParam;
    stpMatchCnt->connectAttemptCallback = evalCallback;
    if (evalData != NULL)
    {
        memcpy(&stpMatchCnt->_8D0, evalData, sizeof(stpMatchCnt->_8D0));
    }
    else
    {
        memset(&stpMatchCnt->_8D0, 0, sizeof(stpMatchCnt->_8D0));
    }
    stpMatchCnt->connectAttemptParam = evalParam;
    stpMatchCnt->qr2IsReserved = 1;
    stpMatchCnt->qr2Reservation = stpMatchCnt->profileID;
    stpMatchCnt->hostState = 1;

    if (!stpMatchCnt->connectGroupID)
    {
        DWCi_SetMatchStatus((DWCMatchState)22);
    }

    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNewA(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, 1, SBFalse, DWCi_SBCallback,
            NULL);
    }
    if (stpMatchCnt->sbObj == NULL)
    {
        if (DWCi_HandleSBError(sbe_allocerror) != 0)
        {
            return;
        }
    }

    if (DWCi_HandleGPError(DWCi_GPSetServerStatus(5)) != 0)
    {
        return;
    }
    if (stpMatchCnt->qr2Obj == NULL)
    {
        if (DWCi_QR2Startup(stpMatchCnt->profileID) != 0)
        {
            return;
        }
    }

    if (!stpMatchCnt->connectGroupID)
    {
        if (DWCi_HandleSBError(DWCi_SBUpdateAsync(stpMatchCnt->profileID)) != 0)
        {
            return;
        }
    }
    else
    {
        qr2_send_statechanged(stpMatchCnt->qr2Obj);
        DWCi_SetMatchStatus((DWCMatchState)3);
        result = DWCi_SendResvCommand(serverPid, FALSE);
        if (stpMatchCnt->qr2MatchTypeExt == 0)
        {
            result = DWCi_HandleSBError(result);
        }
        else
        {
            result = DWCi_HandleGPError(result);
        }
        if (result != 0)
        {
            return;
        }
    }
}

// .text:0x803228CC DWCi_MatchProcess (0x2AC8)
// City Folk's DWCMatchState values (the shared header still has the older enum)
enum
{
    DWC_MATCH_STATE_CF_INIT = 0,
    DWC_MATCH_STATE_CF_CL_WAITING,
    DWC_MATCH_STATE_CF_CL_SEARCH_HOST,
    DWC_MATCH_STATE_CF_CL_WAIT_RESV,
    DWC_MATCH_STATE_CF_CL_SEARCH_EVAL_HOST,
    DWC_MATCH_STATE_CF_CL_NN,
    DWC_MATCH_STATE_CF_CL_GT2,
    DWC_MATCH_STATE_CF_CL_CANCEL_SYN,
    DWC_MATCH_STATE_CF_CL_SYN,
    DWC_MATCH_STATE_CF_CL_SVDOWN_1,
    DWC_MATCH_STATE_CF_CL_SVDOWN_2,
    DWC_MATCH_STATE_CF_CL_SVDOWN_3,
    DWC_MATCH_STATE_CF_CL_SEARCH_GROUPID_HOST,
    DWC_MATCH_STATE_CF_SV_WAITING,
    DWC_MATCH_STATE_CF_SV_OWN_NN,
    DWC_MATCH_STATE_CF_SV_OWN_GT2,
    DWC_MATCH_STATE_CF_SV_WAIT_CL_LINK,
    DWC_MATCH_STATE_CF_SV_CANCEL_SYN,
    DWC_MATCH_STATE_CF_SV_CANCEL_SYN_WAIT,
    DWC_MATCH_STATE_CF_SV_SYN,
    DWC_MATCH_STATE_CF_SV_SYN_WAIT,
    DWC_MATCH_STATE_CF_WAIT_CLOSE,
    DWC_MATCH_STATE_CF_SEARCH_OWN
};

// DWCError 24 (not in the shared header yet)
#define DWC_ERROR_MATCH_24 24

void DWCi_ResetLastSomeDataRecvTimeByAid(u8 aid);
// CF signatures (the prototypes above the stubs still have the older single-argument ones):
//   static BOOL DWCi_CancelPreConnectedClientProcess(int serverPid, int error, int errorCode);
//   static BOOL DWCi_CancelPreConnectedServerProcessWithResetLevel(int clientPid, DWCMatchResetLevel level);

static inline void DWCi_ResetAllNodeRecvTimes(void) {
    int i;

    for (i = 0; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++) {
        DWCi_ResetLastSomeDataRecvTimeByAid(
            DWCi_NodeInfoList_GetNodeInfoForIndex(i)->aid);
    }
}

static inline DWCNodeInfo *DWCi_NodeInfoList_GetMyNodeInfo(void) {
    int profileID = DWCi_GetMatchCnt()->profileID;
    int i;
    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++) {
        if (stpMatchCnt->nodeInfoList.nodes[i].profileID == profileID) {
            return &stpMatchCnt->nodeInfoList.nodes[i];
        }
    }
    return NULL;
}

void DWCi_MatchProcess(BOOL fullSpec)
{
    u32 interval;
    SBError sbError;
    DWCNodeInfo* node;
    DWCNodeInfo* self;
    DWCMatchControl* cnt;
    int i;

    if (!DWCi_GetMatchCnt() || DWCi_IsError())
        return;

    if (!fullSpec)
    {
        if (DWCi_GetMatchCnt()->qr2Obj)
            qr2_think(DWCi_GetMatchCnt()->qr2Obj);
        if (DWCi_GetMatchCnt()->pGt2Socket)
            gt2Think(*DWCi_GetMatchCnt()->pGt2Socket);
        return;
    }

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_INIT)
        return;

    switch (DWCi_GetMatchCnt()->state)
    {
    case DWC_MATCH_STATE_CF_CL_WAIT_RESV:
        if (DWCi_GetMatchCnt()->cmdTimeoutTime != 0)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->cmdTimeoutStartTick)
                > DWCi_GetMatchCnt()->cmdTimeoutTime)
            {
                DWCi_GetMatchCnt()->cmdTimeoutTime = 0;

                if (DWCi_GetMatchCnt()->connectGroupID)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "GroupID Timeout: wait server response %d/%d.\n",
                        DWCi_GetMatchCnt()->scResvRetryCount,
                        DWC_MATCH_CMD_RETRY_MAX);

                    DWCi_GetMatchCnt()->scResvRetryCount++;
                    if (DWCi_GetMatchCnt()->scResvRetryCount
                        > DWC_MATCH_CMD_RETRY_MAX)
                    {
                        DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
                        return;
                    }
                    DWCi_RetryReserving(0);
                    return;
                }

                if (DWCi_StopMeshMaking() == TRUE)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "Mesh NN resv timed out.\n");
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAITING);
                    break;
                }

                if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "Timeout: wait server response %d/%d.\n",
                        DWCi_GetMatchCnt()->scResvRetryCount,
                        DWC_MATCH_CMD_RETRY_MAX);

                    DWCi_GetMatchCnt()->scResvRetryCount++;
                    if (DWCi_GetMatchCnt()->scResvRetryCount
                        > DWC_MATCH_CMD_RETRY_MAX)
                    {
                        DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
                        return;
                    }
                    else
                    {
                        if (DWCi_HandleMatchCommandError(
                                DWCi_SendResvCommand(
                            DWCi_GetMatchCnt()->tempNewNodeInfo.profileID, FALSE)))
                            return;
                    }
                }
                else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "<FRIEND> NN resv(with %u) timed out. Try next server.\n",
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID);
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                        sizeof(DWCNodeInfo));
                    if (!DWCi_RetryReserving(0))
                        return;
                }
                else
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "NN resv(with %u) timed out. Try next server.\n",
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID);
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                        sizeof(DWCNodeInfo));
                    if (!DWCi_RetryReserving(0))
                        return;
                }
            }
        }

        if (DWCi_GetMatchCnt()->cmdResendFlag)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->cmdResendTick)
                >= DWC_MATCH_CMD_RESEND_INTERVAL_MSEC)
            {
                if (DWCi_HandleMatchCommandError(
                        DWCi_SendResvCommand(
                    DWCi_GetMatchCnt()->tempNewNodeInfo.profileID, FALSE)))
                    return;
            }
        }
        break;

    case DWC_MATCH_STATE_CF_CL_SEARCH_HOST:
    case DWC_MATCH_STATE_CF_CL_SEARCH_EVAL_HOST:
    case DWC_MATCH_STATE_CF_SEARCH_OWN:
        if (DWCi_GetMatchCnt()->sbUpdateFlag <= 0)
            break;

        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_CL_SEARCH_HOST)
        {
            interval = DWC_SB_UPDATE_INTERVAL_MSEC;
        }
        else
        {
            if (DWCi_GetMatchCnt()->sbUpdateFlag
                == DWC_SB_UPDATE_INTERVAL_SHORT)
                interval = DWC_SB_UPDATE_INTERVAL_SHORT_MSEC;
            else
                interval = DWC_SB_UPDATE_INTERVAL_MSEC;
        }

        if (DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - DWCi_GetMatchCnt()->sbUpdateTick)
            > interval)
        {
            sbError = DWCi_SBUpdateAsync(DWCi_GetMatchCnt()->reqProfileID);
            if (DWCi_HandleSBError(sbError))
                return;
            DWCi_GetMatchCnt()->sbUpdateFlag = 0;
        }
        break;

    case DWC_MATCH_STATE_CF_CL_GT2:
        if (DWCi_GetMatchCnt()->nnFinishTime)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->nnFinishTime)
                > DWC_WAIT_GT2_CONNECT_TIMEOUT)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: wait gt2Connect().\n");

                DWCi_GetMatchCnt()->nnFinishTime = 0;
                if (DWCi_StopMeshMaking() == TRUE)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "Mesh NN resv timed out.\n");
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAITING);
                    break;
                }
                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID,
                        DWC_ERROR_MATCH_24,
                        DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL))
                {
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                        sizeof(DWCNodeInfo));
                    return;
                }
            }
        }
        else if ((DWCi_GetMatchCnt()->cmdCnt.command
                     == DWC_MATCH_COMMAND_TELL_ADDR)
            && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                    - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                > DWC_MATCH_CMD_RTT_TIMEOUT))
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "RTT Timeout with DWC_MATCH_STATE_CL_GT2.\n");
            if (DWCi_StopMeshMaking() == TRUE)
            {
                DWCi_StopResendingMatchCommand();
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Stop resending command %d.\n",
                    DWC_MATCH_COMMAND_TELL_ADDR);
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Mesh NN resv timed out.\n");
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAITING);
                break;
            }

            DWCi_GetMatchCnt()->cmdCnt.count++;
            if (DWCi_GetMatchCnt()->cmdCnt.count
                > DWC_MATCH_CMD_RETRY_MAX)
            {
                DWCi_StopResendingMatchCommand();

                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Stop resending command %d.\n",
                    DWC_MATCH_COMMAND_TELL_ADDR);

                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID,
                        DWC_ERROR_MATCH_24,
                        DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL))
                {
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                        sizeof(DWCNodeInfo));
                    return;
                }
                break;
            }

            if (DWCi_HandleMatchCommandError(
                    DWCi_SendMatchCommand(DWC_MATCH_COMMAND_TELL_ADDR,
                DWCi_GetMatchCnt()->cmdCnt.profileID,
                DWCi_GetMatchCnt()->cmdCnt.ip,
                DWCi_GetMatchCnt()->cmdCnt.port,
                DWCi_GetMatchCnt()->cmdCnt.data,
                DWCi_GetMatchCnt()->cmdCnt.len)))
                return;
        }
        break;

    case DWC_MATCH_STATE_CF_SV_OWN_NN:
        if ((DWCi_GetMatchCnt()->cmdCnt.command
                == DWC_MATCH_COMMAND_RESV_OK)
            && (((DWCi_GetMatchCnt()->qr2MatchTypeExt
                     == DWC_MATCH_TYPE_ANYBODY)
                    && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                            - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                        > DWC_MATCH_RESV_KEEP_TIME_ANYBODY))
                || ((DWCi_GetMatchCnt()->qr2MatchTypeExt
                         != DWC_MATCH_TYPE_ANYBODY)
                    && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                            - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                        > DWC_MATCH_RESV_KEEP_TIME_FRIEND))))
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Reservation timeout. Cancel reservation.\n");

            DWCi_StopResendingMatchCommand();

            if (DWCi_StopMeshMaking() == TRUE)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAITING);
                break;
            }

            if (!DWCi_CancelPreConnectedServerProcess(
                    DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)) {
                return;
            }
        }
        break;

    case DWC_MATCH_STATE_CF_SV_WAIT_CL_LINK:
        if ((DWCi_GetMatchCnt()->cmdCnt.command
                == DWC_MATCH_COMMAND_LINK_CLS_REQ)
            && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                    - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                > DWC_MATCH_LINK_CLS_TIMEOUT))
        {
            DWCi_GetMatchCnt()->cmdCnt.count++;
            if (DWCi_GetMatchCnt()->cmdCnt.count != 0)
            {
                DWCi_StopResendingMatchCommand();

                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Wait clients connecting timeout.\n");

                if (!DWCi_CancelPreConnectedServerProcess(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)) {
                    return;
                }
            }
        }
        break;

    case DWC_MATCH_STATE_CF_CL_WAITING:
        if ((DWC_GetState() == DWC_STATE_MATCHING)
            && DWCi_GetMatchCnt()->needServerPoll
            && (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->clWaitTime)
                > DWC_MATCH_CL_WAIT_TIMEOUT))
        {
            cnt = DWCi_GetMatchCnt();
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "No data from server %d/%d.\n",
                DWCi_GetMatchCnt()->clWaitTimeoutCount,
                DWC_MATCH_CMD_RETRY_MAX);

            if (DWCi_GetMatchCnt()->clWaitTimeoutCount
                >= DWC_MATCH_CMD_RETRY_MAX)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: Connection to server was shut down.\n");

                if (!DWCi_CancelPreConnectedClientProcess(
                        cnt->tempNewNodeInfo.profileID, DWC_ERROR_MATCH_24,
                        DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL)) {
                    return;
                }
            }
            else
            {
                if (DWCi_HandleMatchCommandError(DWCi_SendMatchCommand(
                        DWC_MATCH_COMMAND_CL_WAIT_POLL,
                        cnt->tempNewNodeInfo.profileID,
                        cnt->tempNewNodeInfo.publicInetAddress,
                        cnt->tempNewNodeInfo.publicQueryPort, NULL, 0))) {
                    return;
                }

                DWCi_GetMatchCnt()->clWaitTimeoutCount++;
                DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick()
                    - DWCi_Np_MilliSecondsToTicks(
                        DWC_MATCH_CL_WAIT_TIMEOUT
                        - DWC_MATCH_CMD_RTT_TIMEOUT);
            }
        }
        break;

    case DWC_MATCH_STATE_CF_CL_SVDOWN_1:
        (void)DWCi_StopMeshMaking();
        cnt = DWCi_GetMatchCnt();
        cnt->serverDownBuff = 0;
        cnt->serverDownQueryCount = 0;
        cnt->serverDownNakCount = 0;

        if (!DWCi_IsBackupMine())
        {
            if (!DWCi_GetGT2Connection(
                    DWCi_NodeInfoList_GetNodeInfoForIndex(0)->aid)) {
                DWC_Printf(DWC_REPORTFLAG_INFO,
                    "[shutdown myself] It doesn't have the GT2 connection with next server.\n");
                DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                    DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
                return;
            }
        }

        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(cnt->profileID);
        cnt->serverDownBuff |= 1 << (node == NULL ? 0xFF : node->aid);

        if (DWCi_IsBackupMine())
        {
            if ((int)DWCi_GetMatchCnt()->topologyType == 1)
            {
                DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                    DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_INFO, "Reserve: I am Next Server!!\n");
                DWCi_SendServerDownQuery();
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_SVDOWN_2);
                qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
                DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
                DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
            }
        }
        else
        {
            qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_SVDOWN_2);
            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
        }
        break;

    case DWC_MATCH_STATE_CF_CL_SVDOWN_2:
        if (DWCi_IsBackupMine())
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->clWaitTime)
                > 5000)
            {
                DWCi_SendServerDownQuery();
                if (++DWCi_GetMatchCnt()->serverDownQueryCount > 5)
                {
                    u32 recv = DWCi_GetMatchCnt()->serverDownBuff;
                    u32 all = DWCi_GetMatchCnt()->serverDownBase;
                    u32 present = recv & all;
                    for (i = 0; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++)
                    {
                        node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
                        if ((~present & (1 << node->aid)) != 0) {
                            DWC_Printf(DWC_REPORTFLAG_INFO,
                                "**** [SV Changing] shutdown no response host(PID:%u aid:%d)\n",
                                node->profileID, node->aid);
                            DWC_CloseConnectionHard(node->aid);
                        }
                    }
                    DWCi_GetMatchCnt()->serverDownBuff = DWCi_GetMatchCnt()->serverDownBase;
                }
            }

            if (!(DWCi_GetMatchCnt()->serverDownBuff ^ DWCi_GetMatchCnt()->serverDownBase)) {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_SVDOWN_3);
            }
        }
        else
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->clWaitTime)
                > 60000)
            {
                DWC_Printf(DWC_REPORTFLAG_INFO, "Backup message timed out.\n");
                DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                    DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
            }
        }
        break;

    case DWC_MATCH_STATE_CF_CL_SVDOWN_3:
        if (DWCi_IsBackupMine())
        {
            if ((int)DWCi_GetMatchCnt()->topologyType == 1)
            {
                DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                    DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_INFO, "Server Down Detected!!\n");
                DWCi_SetupGameServer_InterChange(FALSE);
                DWC_Printf(DWC_REPORTFLAG_INFO, "I am New Server!!\n");
                DWCi_GetMatchCnt()->serverPollingTime = 0;
                DWCi_ResetAllNodeRecvTimes();

                if (DWCi_GetMatchCnt()->suspend != 0xFF)
                {
                    if (DWCi_GetMatchCnt()->suspend == 1)
                        DWCi_GetMatchCnt()->suspendMatchBitmap = DWC_GetAIDBitmap();
                    else
                        DWCi_GetMatchCnt()->suspendMatchBitmap = 0;

                    DWC_Printf(DWC_REPORTFLAG_INFO,
                        "DWCi_AutoSuspendComplete - suspend is %s.\n",
                        DWCi_GetMatchCnt()->suspend ? "TRUE" : "FALSE");
                    if (DWCi_GetMatchCnt()->suspendCallback)
                        ((void (*)(int, u8, u32))DWCi_GetMatchCnt()->suspendCallback)(
                            0, DWCi_GetMatchCnt()->suspend, DWCi_GetMatchCnt()->suspendResultParam);
                    qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
                    DWCi_GetMatchCnt()->suspend = 0xFF;
                    DWCi_GetMatchCnt()->isSuspending = 0;
                }
                qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
            }
        }
        else
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->clWaitTime)
                > 20000)
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAITING);
        }
        break;

    case DWC_MATCH_STATE_CF_CL_SEARCH_GROUPID_HOST:
        if (DWCi_GetMatchCnt()->sbUpdateFlag > 0)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->sbUpdateTick)
                > 8000)
            {
                if (++DWCi_GetMatchCnt()->searchGroupIDRetryHostCount > 15)
                {
                    DWCi_StopMatching((DWCError)DWC_ERROR_MATCH_24,
                        DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
                }
                else
                {
                    DWC_Printf(DWC_REPORTFLAG_SB_UPDATE,
                        "Retry search host from groupID[%d/%d].\n",
                        DWCi_GetMatchCnt()->searchGroupIDRetryHostCount, 15);
                    sbError = DWCi_SBUpdateAsync(0);
                    if (DWCi_HandleSBError(sbError))
                        return;
                    DWCi_GetMatchCnt()->sbUpdateFlag = 0;
                }
            }
        }
        break;

    default:
        break;
    }

    if (((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_SV_OWN_NN)
            || (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_CL_NN))
        && DWCi_GetMatchCnt()->nnFailedTime
        && (DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - DWCi_GetMatchCnt()->nnFailedTime)
            > DWC_WAIT_NN_RETRY_TIMEOUT))
    {
        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Timeout : wait NN retry.\n");

        DWCi_NNCompletedCallback(nr_deadbeatpartner, 0, NULL,
            &DWCi_GetMatchCnt()->nnInfo);
    }

    if (DWCi_GetMatchCnt()->topologyType == 0 && DWCi_GetMatchCnt()->inMeshMaking == 0
        && DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_CL_WAITING
        && DWCi_GetMatchCnt()->tempNewClientInfo.profileID == 0
        && DWCi_GetMatchCnt()->nodeInfoList.count > 2)
    {
        for (i = 1; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++)
        {
            node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
            self = DWCi_NodeInfoList_GetMyNodeInfo();
            if (self == node)
                continue;
            if (DWCi_GetGT2Connection(node->aid))
                continue;

            if (node->nextMeshMakeTryTick == 0)
                node->nextMeshMakeTryTick = DWCi_GetNextMeshMakeTryTick();

            if (DWCi_Np_GetTick() > node->nextMeshMakeTryTick
                && (int)node->nnTryCount <= stMeshMakeNNRetryMax)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "*** TRY mesh create: aid(%d)\n", node->aid);
                node->nextMeshMakeTryTick = DWCi_GetNextMeshMakeTryTick();
                DWCi_GetMatchCnt()->tempNewNodeInfo = *node;

                if (DWCi_HandleMatchCommandError(
                        DWCi_SendResvCommand(node->profileID, FALSE)))
                {
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                        sizeof(DWCNodeInfo));
                }
                else
                {
                    DWCi_GetMatchCnt()->inMeshMaking = 1;
                    DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking = 0;
                    memset(&DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCNodeInfo));
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CF_CL_WAIT_RESV);
                }
                break;
            }
        }
    }

    if (DWCi_GetMatchCnt()->sbObj)
    {
        s_sbCallbackLevel = 0;
        s_needSbFree = FALSE;

        (void)ServerBrowserThink(DWCi_GetMatchCnt()->sbObj);

        if (s_needSbFree)
        {
            ServerBrowserFree(DWCi_GetMatchCnt()->sbObj);
            DWCi_GetMatchCnt()->sbObj = NULL;
        }

        if (DWCi_GetMatchCnt()->sbObj)
        {
            if (ServerBrowserState(DWCi_GetMatchCnt()->sbObj)
                    != sb_disconnected
                && DWCi_GetMatchCnt()->sbUpdateRequestTick != 0
                && DWCi_Np_GetTick()
                    > DWCi_GetMatchCnt()->sbUpdateRequestTick)
            {
                DWCi_StopMatching(DWC_ERROR_NETWORK,
                    DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_SB
                        + DWC_ECODE_TYPE_NETWORK);
                DWC_Printf(DWC_REPORTFLAG_SB_UPDATE,
                    "ServerBrowserLimitUpdate timeout.\n");
            }
        }
    }

    DWCi_SendStateChanged(DWCi_GetMatchCnt()->qr2Obj);

    s_needNnFree = FALSE;
    s_nnThinking = TRUE;

    NNThink();

    s_nnThinking = FALSE;

    if (s_needNnFree == TRUE)
    {
        s_needNnFree = FALSE;
        NNFreeNegotiateList();
    }

    if (DWCi_GetMatchCnt()->pGt2Socket)
        gt2Think(*DWCi_GetMatchCnt()->pGt2Socket);

    if ((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CF_WAIT_CLOSE)
        && (DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - DWCi_GetMatchCnt()->closedTime)
            > (DWC_MATCH_CMD_RTT_TIMEOUT >> 1)))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "RTT Timeout with DWCi_MatchProcess.\n");
        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Timeout : Wait prior profileID.\n");

        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
        DWCi_GetMatchCnt()->closeState = 0;
        DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
    }

    DWCi_RequestSuspendMatchProcess();
    DWCi_SendServerPolling(FALSE);

    if (!DWCi_ProcessMatchSynTimeout()) {
        return;
    }
    if (!DWCi_ProcessCancelMatchSynTimeout()) {
        return;
    }
}

// .text:0x80325394 DWCi_GT2UnrecognizedMessageCallback (0x174)
GT2Bool DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, unsigned int ip,
    unsigned short port, GT2Byte* message, int len)
{
    struct sockaddr_in saddr;

    if (!len || !message)
    {
        return GT2False;
    }

    DWCi_Np_CpuClear32(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.addr = ip;
    saddr.sin_port = SOHtoNs(port);

    if ((message[0] == QR_MAGIC_1 && message[1] == QR_MAGIC_2)
        || message[0] == '\\')
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "GT2 Unrecognized : Received QR2 data.\n");
        if (DWCi_GetMatchCnt()->qr2Obj)
        {
            qr2_parse_queryA(DWCi_GetMatchCnt()->qr2Obj, (char*)message, len,
                (struct sockaddr*)&saddr);
        }
        else
        {
            DWC_Printf(
                DWC_REPORTFLAG_MATCH_NN, "  ignore qr2 message.\n");
        }
    }
    else if (!memcmp(message, NNMagicData, NATNEG_MAGIC_LEN))
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "GT2 Unrecognized : Received NN data.\n");
        NNProcessData((char*)message, len, &saddr);
    }
    else if (message[0] == 0xFE && message[1] == 0xFE)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "GT2 Unrecognized : Not Connected gt2 data.\n");
        return GT2False;
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "GT2 Unrecognized : Received unrecognized data.\n");
        return GT2False;
    }

    return GT2True;
}

BOOL DWC_IsServerMyself(void);

// helper: DWCi_GPSetServerStatusInline (inlined into DWCi_StopMatching and DWCi_GPSetServerStatus in CF)

static inline GPResult DWCi_GPSetServerStatusInline(int status)
{
    char value[12];
    char statusStr[37];
    u32 n;
    BOOL locked;

    snprintf(value, sizeof(value), "%u", stpMatchCnt->qr2NumEntry + 1);
    DWC_SetCommonKeyValueString("SCM", value, statusStr, '/');

    n = stpMatchCnt->nodeInfoList.count;
    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV && n == 0)
    {
        n = 1;
    }
    snprintf(value, sizeof(value), "%u", n);
    DWC_AddCommonKeyValueString("SCN", value, statusStr, '/');

    snprintf(value, sizeof(value), "%u", 90);
    DWC_AddCommonKeyValueString("VER", value, statusStr, '/');

    if (stpMatchCnt->serverLocked == 0)
    {
        locked = FALSE;
    }
    else if (DWC_IsServerMyself()
        && (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
        && stpMatchCnt->firstServer == 0)
    {
        locked = TRUE;
    }
    else
    {
        locked = FALSE;
    }
    snprintf(value, sizeof(value), "%u", locked != 0);
    DWC_AddCommonKeyValueString("LCK", value, statusStr, '/');

    return DWCi_SetGPStatus(status, statusStr, NULL);
}

// helper: DWCi_CancelPreConnectedServerProcess (inlined in CF)
static inline BOOL DWCi_CancelPreConnectedServerProcess(int clientPid)
{
    DWCMatchResetLevel level = DWC_MATCH_RESET_CONTINUE;

    if ((DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY
            || DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
        && DWCi_GetMatchCnt()->connectedOnceBefore == 0)
    {
        level = DWC_MATCH_RESET_RESTART;
    }
    return DWCi_CancelPreConnectedServerProcessWithResetLevel(
        clientPid, level);
}

// .text:0x80325508 DWCi_GT2ConnectAttemptCallback (0x780)
void DWCi_GT2ConnectAttemptCallback(GT2Socket socket,
    GT2Connection connection, unsigned int ip, unsigned short port,
    int latency, GT2Byte* message, int len)
{
    int index;
    GT2Connection* pGt2Con;
    DWCConnectionInfo* pConInfo;
    int pid = 0;

    if (DWCi_GetMatchCnt() != NULL
        && DWCi_GetMatchCnt()->state == 5 /* CL_NN */)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "DWCi_GT2ConnectAttemptCallback in DWC_MATCH_STATE_CL_NN\n");
        return;
    }

    if (DWCi_GetMatchCnt() == NULL
        || DWCi_GetMatchCnt()->state != 6 /* CL_GT2 */
        || DWCi_GetMatchCnt()->cancelState != DWC_MATCH_CANCEL_STATE_INIT)
    {
        gt2Reject(connection, (const GT2Byte*)"Init state", -1);
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "gt2Reject was called : Init state\n");
        return;
    }

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Got DWCi_GT2ConnectAttemptCallback latency(%d) msg(%s)\n", latency,
        message);
    if (message && len)
    {
        pid = (int)strtoul((const char*)message, NULL, 10);
    }
    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "remote PID = %d\n", pid);

    index = DWCi_GT2GetConnectionListIdx();
    if (index == -1)
    {
        gt2Reject(connection, (const GT2Byte*)"Server full", -1);
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "gt2Reject was called : Server full\n");
        if (DWCi_StopMeshMaking() == TRUE)
        {
            DWCi_StopResendingMatchCommand();
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "Mesh stop.\n");
            DWCi_SetMatchStatus(DWC_MATCH_STATE_INIT + 1 /* CL_WAITING */);
        }
        else
        {
            DWCi_StopMatching(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GT2
                    + DWC_ECODE_TYPE_CONN_OVER);
        }
        return;
    }

    if (ip != DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP
        || SONtoHs(port) != DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port)
    {
        if (pid == DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
        {
            DWC_Printf(DWC_REPORTFLAG_WARNING,
                "gt2Connect() came before NN complete.\n");
            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP = ip;
            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port = SONtoHs(port);
        }
        else
        {
            gt2Reject(
                connection, (const GT2Byte*)"Unknown connect attempt", -1);
            DWC_Printf(DWC_REPORTFLAG_WARNING,
                "gt2Reject was called : Unknown connect attempt from %s\n",
                gt2AddressToString(ip, port, NULL));
            return;
        }
    }

    DWCi_GetMatchCnt()->nnFinishTime = 0;

    if (gt2Accept(connection, DWCi_GetMatchCnt()->gt2Callbacks) == GT2False)
    {
        DWC_Printf(
            DWC_REPORTFLAG_WARNING, "Unexpected failure to gt2Accept.\n");
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_OTHER);
        return;
    }

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Accepted connection from %s (latency %d)\n",
        gt2AddressToString(ip, port, NULL), latency);

    DWCi_StopResendingMatchCommand();

    if (DWCi_GetMatchCnt()->inMeshMaking == 0
        && DWCi_GetMatchCnt()->nodeInfoList.count == 1)
    {
        DWCi_GetMatchCnt()->baseLatency
            = (u16)(((latency >> 1) < 0xffff) ? (latency >> 1) : 0xffff);
    }

    pGt2Con = DWCi_GetGT2ConnectionByIdx(index);
    pConInfo = DWCi_GetConnectionInfoByIdx(index);
    *pGt2Con = connection;
    pConInfo->index = (u8)index;
    pConInfo->aid = DWCi_GetMatchCnt()->tempNewNodeInfo.aid;
    pConInfo->param = (void*)DWCi_GetMatchCnt()->tempNewNodeInfo.profileID;
    DWCi_ResetLastSomeDataRecvTimeByAid(pConInfo->aid);

    gt2SetConnectionData(connection, pConInfo);

    DWCi_PostProcessConnection(DWC_PP_CONNECTION_CL_GT2_ACCEPT);
}

// .text:0x80325C88 DWCi_GT2ConnectedCallback (0x67C)
void DWCi_GT2ConnectedCallback(
    GT2Connection connection, GT2Result result, GT2Byte* message, int len)
{
    char pidStr[24];
    int index;
    GT2Connection* pGt2Con;
    GT2Result gt2Result;
    DWCConnectionInfo* pConInfo;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "DWCi_GT2ConnectedCallback(port=%d,result=%d,msg=[%s])\n",
        gt2GetRemotePort(connection), result,
        message ? (char*)message : "");

    if (DWCi_GetMatchCnt() == NULL
        || (DWCi_GetMatchCnt()->state != 6 /* CL_GT2 */
            && DWCi_GetMatchCnt()->state != 15 /* SV_OWN_GT2 */))
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "gt2ConnectedCallback: Already cancel(port=%d,result=%d,msg=[%s]).\n",
            gt2GetRemotePort(connection), result,
            message ? (char*)message : "");
        if ((int)DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking == 0xF)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                "gt2ConnectedCallback: mesh creating.. go next process.\n");
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                "@@@@@ gt2ConnectedCallback: but ignored.\n");
            return;
        }
    }

    if (result != GT2Success)
    {
        if (message == NULL)
        {
            message = (GT2Byte*)"";
        }
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "GT2 connect failed %d: %s\n",
            result, message);
        if (DWCi_StopMeshMaking() == TRUE)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh NN resv timed out.\n");
            DWCi_SetMatchStatus(1 /* CL_WAITING */);
        }
        else if ((int)DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking == 0xF)
        {
            DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking = 0;
            memset(&DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCNodeInfo));
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh gt2 failed.\n");
        }
        else if (result != GT2DuplicateAddress)
        {
            if (result == GT2TimedOut)
            {
                DWCi_GetMatchCnt()->gt2ConnectCount++;
                if (DWCi_GetMatchCnt()->gt2ConnectCount > 5)
                {
                    DWC_Printf(
                        DWC_REPORTFLAG_MATCH_GT2, "gt2Connect() retry over.\n");
                    DWCi_GetMatchCnt()->gt2ConnectCount = 0;
                    DWCi_CancelPreConnectedServerProcess(DWCi_GetMatchCnt()->tempNewNodeInfo.profileID);
                }
                else
                {
                    DWC_Printf(
                        DWC_REPORTFLAG_MATCH_GT2, "Retry to gt2Connect.\n");
                    snprintf(pidStr, sizeof(pidStr), "%u",
                        DWCi_GetMatchCnt()->profileID);
                    gt2Result = gt2Connect(*DWCi_GetMatchCnt()->pGt2Socket,
                        NULL,
                        gt2AddressToString(
                            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP,
                            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port, NULL),
                        (GT2Byte*)pidStr, -1, 5000,
                        DWCi_GetMatchCnt()->gt2Callbacks, GT2False);
                    if (gt2Result == GT2OutOfMemory)
                    {
                        DWCi_HandleGT2Error(gt2Result);
                    }
                    else if (gt2Result != GT2Success)
                    {
                        if (DWCi_CancelPreConnectedServerProcess(DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
                            == 0)
                        {
                            return;
                        }
                    }
                }
            }
            else if (DWCi_CancelPreConnectedServerProcess(DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
                == 0)
            {
                return;
            }
        }
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "GT2 connected.\n");
        index = DWCi_GT2GetConnectionListIdx();
        if (index == -1)
        {
            DWC_Printf(DWC_REPORTFLAG_WARNING,
                "Don't continue matching without closing connections!!\n");
            DWCi_StopMatching(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GT2
                    + DWC_ECODE_TYPE_CONN_OVER);
            return;
        }

        pGt2Con = DWCi_GetGT2ConnectionByIdx(index);
        pConInfo = DWCi_GetConnectionInfoByIdx(index);
        *pGt2Con = connection;
        pConInfo->index = (u8)index;
        if ((int)DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking != 0xF)
        {
            pConInfo->aid = DWCi_GetMatchCnt()->tempNewNodeInfo.aid;
            pConInfo->param
                = (void*)DWCi_GetMatchCnt()->tempNewNodeInfo.profileID;
        }
        else
        {
            pConInfo->aid = DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking.aid;
            pConInfo->param = (void*)DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking.profileID;
        }
        DWCi_ResetLastSomeDataRecvTimeByAid(pConInfo->aid);
        gt2SetConnectionData(connection, pConInfo);

        if (DWCi_GetMatchCnt()->state == 15 /* SV_OWN_GT2 */
            || (int)DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking == 0xF)
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
        }
        else
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_CL_GT2_CONNECT);
        }
    }
}

// .text:0x80326304 DWCi_MatchGPRecvBuddyMsgCallback (0xF0)
void DWCi_MatchGPRecvBuddyMsgCallback(
    GPConnection* connection, u32 profileId, char* message)
{
    u32 list[0x80];
    int len;
    int size;

    DWC_Printf(0x40, "<GP> RECV-0x%02x <- [--------:-----] [pid=%u]\n",
        (u8)message[0], profileId);

    len = (int)strlen(message) - 1;
    if (len == 0)
    {
        size = 0;
    }
    else
    {
        size = DWC_Base64Decode(message + 1, len, (char*)list, sizeof(list));
        if (size == -1 || size % 4 != 0 || size > (int)sizeof(list))
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG, "Ignore illegal data(%d,%d).\n",
                len, size);
            return;
        }
        size /= 4;
    }
    DWCi_ProcessRecvMatchCommand((u8)message[0], profileId, 0, 0, list, size);
}

// .text:0x803263F4 DWCi_StopMatching (0x228)
void DWCi_StopMatching(DWCError error, int errorCode)
{
    DWCMatchControl* matchCnt;

    if (DWCi_GetMatchCnt() == NULL || error == 0)
    {
        return;
    }

    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
    gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
    DWCi_SetError(error, errorCode);
    DWCi_GPSetServerStatusInline(1);
    matchCnt = DWCi_GetMatchCnt();
    matchCnt->matchedCallback(error, FALSE,
        matchCnt->cbEventPid ? FALSE : TRUE,
        matchCnt->qr2MatchType == 2 ? TRUE : FALSE,
        DWCi_GetFriendListIndex(matchCnt->cbEventPid),
        matchCnt->matchedParam);
    DWCi_CloseMatch();
}

// .text:0x8032661C DWCi_ClearQR2Key (0x60)
void DWCi_ClearQR2Key(void)
{
    if (DWC_GetState() == DWC_STATE_ONLINE)
    {
        stpMatchCnt->qr2NumEntry = 0;
        stpMatchCnt->groupID = 0;
        qr2_send_statechanged(stpMatchCnt->qr2Obj);
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWCi_ClearQR2Key ignored.\n");
    }
}

// .text:0x8032667C DWCi_ProcessMatchSynPacket (0x6C)
void DWCi_ProcessMatchSynPacket(u8 aid, u16 type, u8* data)
{
    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Received SYN %d packet from aid %d.\n", type - DWC_SEND_TYPE_MATCH_SYN,
        aid);

    DWCi_ProcessMatchSynPacketSub_Hybrid(aid, type, data);
}

// .text:0x803266E8 DWCi_ProcessMatchSynPacketSub_Hybrid (0x1438)
// TODO: not in the older source; decompile from CF
// helper: DWCi_NodeInfo_FromData (inlined in CF): one node as it travels in a SYN / ACK packet (6 words)
static inline void DWCi_NodeInfo_FromData(DWCNodeInfo* node, const u32* d)
{
    memset(node, 0, sizeof(DWCNodeInfo));
    node->aid = (u8)DWCi_LEtoHl(d[1]);
    node->hasPrivateAddress = (u8)(DWCi_LEtoHl(d[1]) >> 16);
    node->profileID = DWCi_LEtoHl(d[2]);
    node->publicInetAddress = d[3];
    node->privateInetAddress = d[4];
    node->publicQueryPort = (u16)(DWCi_LEtoHl(d[5]) >> 16);
    node->privateQueryPort = (u16)DWCi_LEtoHl(d[5]);
    memcpy(node->connUserData, &d[6], 4);
}

// helper: DWCi_NodeInfoList_InsertToTop (inlined in CF): the new node becomes
// the server (index 0)
static inline void DWCi_NodeInfoList_InsertToTop(const DWCNodeInfo *node) {
    int i;

    if (stpMatchCnt->nodeInfoList.count != DWC_MAX_CONNECTIONS)
    {
        for (i = stpMatchCnt->nodeInfoList.count; i > 0; i--)
        {
            stpMatchCnt->nodeInfoList.nodes[i]
                = stpMatchCnt->nodeInfoList.nodes[i - 1];
        }
        stpMatchCnt->nodeInfoList.nodes[0] = *node;
        stpMatchCnt->nodeInfoList.count++;
    }
}

static inline void DWCi_ReadMatchNodeList(const u32 *d) {
    u32 n;
    int i;
    n = DWCi_LEtoHl(d[0]);
    if (n != 0) {
        u32 lastIndex = n - 1;
        for (i = 0; i < (int)n; i++) {
            DWCNodeInfo node;

            u32 offset = (lastIndex - i) * 6;
            const u32 *nodeData = d + offset;
            DWCi_NodeInfo_FromData(&node, nodeData);
            DWCi_NodeInfoList_InsertToTop(&node);
        }
        {
            u32 offset = n * 6;
            DWCi_GetMatchCnt()->serverPollingUID = DWCi_LEtoHl(d[offset + 1]);
        }
        DWCi_NodeInfoList_InsertToTop(&DWCi_GetMatchCnt()->tempNewNodeInfo);
        memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
    } else {
        DWCi_GetMatchCnt()->serverPollingUID = DWCi_LEtoHl(d[1]);
    }
}

static inline u32 DWCi_NodeInfoList_GetClientAIDBitmap(BOOL includePending) {
    u32 bits = 0;
    int i;
    DWCMatchControl *cnt = DWCi_GetMatchCnt();
    for (i = 1; i < cnt->nodeInfoList.count; i++) {
        bits |= 1 << DWCi_NodeInfoList_GetNodeInfoForIndex(i)->aid;
    }
    if (includePending) {
        bits |= 1 << DWCi_GetMatchCnt()->tempNewNodeInfo.aid;
    }
    return bits;
}

static inline void DWCi_SendTypeMatchAck(void) {
    int i;
    if ((s32)DWCi_GetMatchCnt()->hostState != 2) {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                   "[WARN] DWCi_SendTypeMatchAck: I'm not server!!\n");
    } else {
        for (i = 1; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++) {
            DWCi_SendMatchSynPacket(
                DWCi_NodeInfoList_GetNodeInfoForIndex(i)->aid, 4);
        }
        if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0) {
            DWCi_SendMatchSynPacket(DWCi_GetMatchCnt()->tempNewNodeInfo.aid, 4);
        }
        DWCi_SetMatchStatus(20 /* SV_SYN_WAIT */);
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "Wait max latency %d msec.\n",
                   DWCi_GetMatchCnt()->baseLatency);
    }
}

static void DWCi_ProcessMatchSynPacketSub_Hybrid(u8 aid, u16 type, u8* data)
{
    const u32* d = (const u32*)data;
    u16 latency;
    u32 bits;
    int timeout;

    switch (type)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
        if (DWCi_GetMatchCnt()->state == 1 /* CL_WAITING */) {
            if (DWCi_LEtoHl(d[0]) == 1)
            {
                DWCi_GetMatchCnt()->cbEventPid = 0;
                DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;
                DWCi_GetMatchCnt()->serverPollingUID = DWCi_LEtoHl(d[1]);
                if ((int)DWCi_GetMatchCnt()->topologyType == 2) {
                    DWCi_NodeInfoList_InsertToTop(
                        &DWCi_GetMatchCnt()->tempNewNodeInfo);
                }
            }
            else
            {
                DWCNodeInfo node;

                DWCi_NodeInfo_FromData(&node, d);
                DWCi_GetMatchCnt()->tempNewClientInfo = node;
                DWCi_GetMatchCnt()->serverPollingUID = DWCi_LEtoHl(d[7]);
                DWCi_GetMatchCnt()->cbEventPid = node.profileID;
            }

            if (DWCi_GetMatchCnt()->nodeInfoList.count == 1) {
                timeout = 90000;
            } else {
                timeout = DWCi_GetMatchCnt()->nodeInfoList.count * 12000;
            }
            DWCi_GetMatchCnt()->clWaitTime = timeout;
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "SYN wait time = %d\n",
                       DWCi_GetMatchCnt()->clWaitTime);
            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            DWCi_SetMatchStatus(8 /* CL_SYN */);
            DWCi_SendMatchSynPacket(aid, 3);
        }
        break;

    case DWC_SEND_TYPE_MATCH_SYN_ACK:
        if (DWCi_GetMatchCnt()->state == 19 /* SV_SYN */) {
            latency = data[0] | (data[1] << 8);
            DWCi_GetMatchCnt()->synAckBit |= 1 << aid;
            if (latency > DWCi_GetMatchCnt()->baseLatency) {
                DWCi_GetMatchCnt()->baseLatency = latency;
            }

            bits = DWCi_NodeInfoList_GetClientAIDBitmap(TRUE);

            if (bits == DWCi_GetMatchCnt()->synAckBit) {
                DWCi_GetMatchCnt()->serverPollingUID = DWCi_GetMathRand32((u32)-2) + 1;
                DWCi_SendTypeMatchAck();
                DWCi_GetMatchCnt()->synResendCount = 0;
                DWCi_GetMatchCnt()->lastSynSent = DWCi_Np_GetTick();
            }
        } else {
            DWCi_SendMatchSynPacket(aid, 4);
        }
        break;

    case DWC_SEND_TYPE_MATCH_ACK:
        if (DWCi_GetMatchCnt()->state == 8 /* CL_SYN */) {
            DWCi_ReadMatchNodeList(d);
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SYN_FINISH);
        }
        break;
    }
}

static inline void DWCi_CloseDeadConnections(u32 aids, u32 bitmap) {
    u32 dead = aids & (aids ^ bitmap);
    u8 i;
    if (dead != 0) {
        DWC_Printf(0x10000, "check dead aid: dead aidBitmap is %08x.\n", dead);
        for (i = 0; i < 32; i++) {
            if (dead & (1 << i)) {
                DWC_CloseConnectionHard(i);
            }
        }
    }
}

// .text:0x80327B20 DWCi_ProcessMatchPollingPacket (0x184)
// TODO: not in the older source; decompile from CF
static void DWCi_ProcessMatchPollingPacket(const u32* data)
{
    u32 pid = DWCi_LEtoHl(data[0]);
    u32 bitmap = DWCi_LEtoHl(data[1]);
    u32 uid = DWCi_LEtoHl(data[2]);
    DWCNodeInfo* server;
    u32 aids;

    DWC_Printf(0x10000, "Received Polling packet from pid %d(%08x).\n", pid,
        bitmap);

    if (DWC_IsServerMyself())
    {
        DWC_Printf(0x10000, "I'm a server. ignore.\n");
    }
    else if (uid != stpMatchCnt->serverPollingUID)
    {
        DWC_Printf(1, "ignore serverPollingUID(%d).\n", uid);
    }
    else
    {
        server = DWCi_NodeInfoList_GetServerNodeInfo();
        if (server == NULL || server->profileID != pid)
        {
            DWC_Printf(0x10000, "command from wrong server. ignore.\n");
        }
        else
        {
            aids = DWC_GetAIDBitmap();
            DWCi_CloseDeadConnections(aids, bitmap);
        }
    }
}

// .text:0x80327CA4 DWCi_ProcessMatchClosing (0x5C0)
BOOL DWCi_ProcessMatchClosing(
    DWCError error, int errorCode, int profileID)
{
#pragma unused(profileID)
    if (DWC_GetState() != DWC_STATE_MATCHING)
    {
        return FALSE;
    }

    if (DWCi_GetMatchCnt()->qr2MatchType == 2)
    {
        DWC_Printf(
            DWC_REPORTFLAG_DEBUG, "DWCi_ProcessMatchClosing: SC_SV.\n");
        return TRUE;
    }

    if (error)
    {
        DWCi_StopMatching(error, errorCode + DWC_ECODE_SEQ_MATCH);
        return TRUE;
    }

    if ((DWCi_GetMatchCnt()->cancelState == DWC_MATCH_CANCEL_STATE_EXEC)
        || (DWCi_GetMatchCnt()->closeState
            == DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT))
    {
        return TRUE;
    }

    if (DWCi_GetMatchCnt()->nnInfo.cookie)
    {
        NNCancel(DWCi_GetMatchCnt()->nnInfo.cookie);
        DWCi_GetMatchCnt()->nnInfo.cookie = 0;
    }

    if (DWCi_GetMatchCnt()->nodeInfoList.count > 1)
    {
        if (DWCi_GetMatchCnt()->closeState == DWC_MATCH_CLOSE_STATE_INIT)
        {
            DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_TIMEOUT;
            gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
        }
    }
    else
    {
        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
        {
            DWCi_StopMatching((DWCError)24,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
        }
        else if (DWCi_GetMatchCnt()->state == 1 /* CL_WAITING */)
        {
            DWC_Printf(
                DWC_REPORTFLAG_MATCH_NN, "Wait prior profileID.\n");

            DWCi_SetMatchStatus(21 /* WAIT_CLOSE */);
            DWCi_GetMatchCnt()->closedTime = DWCi_Np_GetTick();
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Restart matching immediately.\n");

            DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
        }
    }

    return TRUE;
}

// .text:0x80328264 DWCi_ProcessMatchSCClosing (0x44)
void DWCi_ProcessMatchSCClosing(int clientPid)
{
    DWCMatchResetLevel level;

    if (stpMatchCnt->closeState == DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT)
    {
        return;
    }
    level = DWC_MATCH_RESET_CONTINUE;
    if ((stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
        && stpMatchCnt->connectedOnceBefore == 0)
    {
        level = DWC_MATCH_RESET_RESTART;
    }
    DWCi_CancelPreConnectedServerProcessWithResetLevel(clientPid, level);
}

// .text:0x803282A8 DWCi_GPSetServerStatus (0x178)
GPResult DWCi_GPSetServerStatus(int status)
{
    return DWCi_GPSetServerStatusInline(status);
}

// .text:0x80328420 DWCi_ShutdownMatch (0xA8)
void DWCi_ShutdownMatch(void)
{
    stpMatchCnt = NULL;
    sMatchStateTick = OSGetTime();
    if (stpAddFilter != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpAddFilter, 0);
        stpAddFilter = NULL;
    }

    DWCi_ClearGameMatchKeys();
}

// .text:0x803284C8 DWCi_IsShutdownMatch (0x10)
BOOL DWCi_IsShutdownMatch(void)
{
    return !stpMatchCnt;
}

static inline int DWCi_GetConnectionRole(const DWCMatchControl *cnt) {
    return cnt->hostState;
}

// .text:0x803284D8 DWCi_ResetMatchParam (0x4B8)
static void DWCi_ResetMatchParam(DWCMatchResetLevel level)
{
    DWCi_GetMatchCnt()->gt2ConnectCount = 0;
    DWCi_GetMatchCnt()->nnRecvCount = 0;
    DWCi_GetMatchCnt()->nnCookieRand = DWCi_GetMathRand32(0x10000);
    DWCi_GetMatchCnt()->nnLastCookie = 0;
    DWCi_GetMatchCnt()->nnFailedTime = 0;
    DWCi_GetMatchCnt()->nnFinishTime = 0;
    DWCi_GetMatchCnt()->clLinkProgress = 0;
    DWCi_GetMatchCnt()->cancelState = 0;
    DWCi_GetMatchCnt()->scResvRetryCount = 0;
    DWCi_GetMatchCnt()->synResendCount = 0;
    DWCi_GetMatchCnt()->cancelSynResendCount = 0;
    DWCi_GetMatchCnt()->resvWaitCount = 0;
    DWCi_GetMatchCnt()->closeState = 0;
    DWCi_GetMatchCnt()->cancelBaseLatency = 0;
    DWCi_GetMatchCnt()->lastSynSent = 0;
    DWCi_GetMatchCnt()->closedTime = 0;
    memset(&DWCi_GetMatchCnt()->cmdCnt, 0, sizeof(DWCi_GetMatchCnt()->cmdCnt));
    DWCi_GetMatchCnt()->inMeshMaking = 0;
    DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking = 0;
    memset(&DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking));

    if (level == DWC_MATCH_RESET_CONTINUE)
    {
        if ((int)DWCi_GetMatchCnt()->hostState == 2)
        {
            DWCi_SetMatchStatus(13); // DWC_MATCH_STATE_SV_WAITING
        } else if (DWCi_GetConnectionRole(DWCi_GetMatchCnt()) == 1) {
            DWCi_SetMatchStatus(1); // DWC_MATCH_STATE_CL_WAITING
        } else {
            DWC_Printf(8, "DWCi_ResetMatchParam: Host is Free!\n");
        }
    }
    else
    {
        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
        DWCi_GetMatchCnt()->sbUpdateFlag = FALSE;
        DWCi_GetMatchCnt()->baseLatency = 0;
        DWCi_GetMatchCnt()->cmdResendFlag = FALSE;
        DWCi_GetMatchCnt()->cmdResendTick = 0;
        DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
        DWCi_GetMatchCnt()->cmdTimeoutStartTick = 0;
        DWCi_GetMatchCnt()->synAckBit = 0;
        DWCi_GetMatchCnt()->reqProfileID = 0;
        memset(&DWCi_GetMatchCnt()->nnInfo, 0, sizeof(DWCNNInfo));
        DWCi_GetMatchCnt()->serverDownBuff = 0;
        DWCi_GetMatchCnt()->serverDownBase = 0;
        DWCi_GetMatchCnt()->searchGroupIDRetryHostCount = 0;
        DWCi_GetMatchCnt()->serverFind = 1;
        DWCi_GetMatchCnt()->hostState = 0;
        DWCi_GetMatchCnt()->connectedOnceBefore = 0;
        DWCi_GetMatchCnt()->connectAttemptCallback = NULL;
        DWCi_GetMatchCnt()->connectAttemptParam = NULL;
        memset(&DWCi_GetMatchCnt()->nodeInfoList, 0, sizeof(DWCi_GetMatchCnt()->nodeInfoList));
        memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0, sizeof(DWCi_GetMatchCnt()->tempNewNodeInfo));
        DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;

        if (level == DWC_MATCH_RESET_RESTART)
        {
            if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY
                || DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
            {
                DWCi_SetMatchStatus(22); // DWC_MATCH_STATE_SEARCH_OWN
            }
        }
        else
        {
            DWCi_GetMatchCnt()->qr2MatchType = 0;
            DWCi_GetMatchCnt()->qr2NumEntry = 0;
            DWCi_GetMatchCnt()->qr2ShutdownFlag = 0;
            DWCi_GetMatchCnt()->cbEventPid = 0;
            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            DWCi_GetMatchCnt()->clWaitTime = 0;
            DWCi_GetMatchCnt()->newClientCallback = 0;
            DWCi_GetMatchCnt()->newClientParam = 0;
            memset(&DWCi_GetMatchCnt()->_8D0, 0, sizeof(DWCi_GetMatchCnt()->_8D0));
        }
    }
}

// .text:0x80328990 DWCi_SetMatchCommonParam (0xE8)
static void DWCi_SetMatchCommonParam(u8 matchType, u8 numEntry,
    DWCMatchedSCCallback callback, void* param)
{
    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);

    stpMatchCnt->qr2MatchType = matchType;
    stpMatchCnt->qr2NumEntry = numEntry;
    stpMatchCnt->matchedCallback = callback;
    stpMatchCnt->matchedParam = param;
    stpMatchCnt->nnFailureCount = 0;
    stpMatchCnt->suspend = 0xFF;
    stpMatchCnt->suspendServerResponse = 0;
    stpMatchCnt->hostState = 0;
    stpMatchCnt->serverFind = 1;
    stpMatchCnt->qr2NumEntry = numEntry;
    stpMatchCnt->suspendMatchBitmap = 0;
    stpMatchCnt->suspendMatch = 0;
    stpMatchCnt->suspendRequestTick = 0;
    stpMatchCnt->isSuspending = 0;
    stpMatchCnt->firstServer = 1;
    stpMatchCnt->suspendWaitTime = 0;
    stpMatchCnt->nnResvCheckValue = 0;
}

// .text:0x80328A78 DWCi_CloseMatch (0x354)
BOOL DWC_IsServerMyself(void);

void DWCi_CloseMatch(void)
{
    DWC_Printf(0x40, " Close Match....\n");
    if (stpMatchCnt == NULL)
    {
        return;
    }

    if (stpMatchCnt->sbObj != NULL)
    {
        if (s_sbCallbackLevel == 0)
        {
            ServerBrowserFree(stpMatchCnt->sbObj);
            stpMatchCnt->sbObj = NULL;
        }
        else
        {
            s_needSbFree = 1;
        }
    }

    DWCi_NNFreeNegotiateList();
    DWCi_SetMatchStatus(0); // DWC_MATCH_STATE_INIT

    if (stpAddFilter != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpAddFilter, 0);
        stpAddFilter = NULL;
    }

    DWCi_ClearGameMatchKeys();

    stpMatchCnt->qr2ShutdownFlag = 1;
    memset(&stpMatchCnt->nodeInfoList, 0, sizeof(stpMatchCnt->nodeInfoList));
    memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(stpMatchCnt->tempNewNodeInfo));
    stpMatchCnt->firstServer = 1;

    DWCi_GPSetServerStatus(1);
}

// .text:0x80328DCC DWCi_SBUpdateAsync (0x3E0)
// qr2 key ids CF registers after the match version / eval keys (names: dwc_groupid, dwc_hoststate)
#define DWC_QR2_GROUPID_KEY   (NUM_RESERVED_KEYS + 5)
#define DWC_QR2_HOSTSTATE_KEY (NUM_RESERVED_KEYS + 6)
#define DWC_NUM_QR2_MATCH_KEY 7

static SBError DWCi_SBUpdateAsync(int profileID)
{
    char filter[MAX_FILTER_LEN];
    u8 basicFields[208];
    int len;
    int numBasicFields;
    int i;
    SBBool async = SBTrue;
    SBError sbError;
    u8 searchHostType;

    basicFields[0] = NUMPLAYERS_KEY;
    basicFields[1] = MAXPLAYERS_KEY;
    basicFields[2] = DWC_QR2_PID_KEY;
    basicFields[3] = DWC_QR2_MATCH_TYPE_KEY;
    basicFields[4] = DWC_QR2_MATCH_RESV_KEY;
    basicFields[5] = DWC_QR2_MATCH_VER_KEY;
    basicFields[6] = DWC_QR2_MATCH_EVAL_KEY;
    basicFields[7] = DWC_QR2_GROUPID_KEY;
    basicFields[8] = DWC_QR2_HOSTSTATE_KEY;
    numBasicFields = DWC_NUM_QR2_MATCH_KEY + 2;

    if ((DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
        || (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND))
    {
        for (i = 0; i < DWC_QR2_GAME_RESERVED_KEYS; i++)
        {
            if (stGameMatchKeys[i].keyID)
            {
                basicFields[numBasicFields] = stGameMatchKeys[i].keyID;
                numBasicFields++;
            }
        }
    }

    switch (DWCi_GetMatchCnt()->state)
    {
    case 2: // DWC_MATCH_STATE_CL_SEARCH_HOST
    {
        int ratio = s_anybodyServerSearchRatio;

        if (ratio == -1)
        {
            // alternate between searching hosts and searching everyone
            searchHostType = 0;
            if (DWCi_GetMatchCnt()->serverFind)
            {
                searchHostType = 2;
            }
            DWCi_GetMatchCnt()->serverFind ^= 1;
        }
        else
        {
            int rnd = (int)DWCi_GetMathRand32(100);

            searchHostType = 0;
            if (rnd < ratio)
            {
                searchHostType = 2;
            }
            DWC_Printf(0x40, "ServerSearchRatio=%d %% (next search->%d)\n",
                ratio, searchHostType);
        }

        len = DWCi_GetDefaultMatchFilter(filter, DWCi_GetMatchCnt()->profileID,
            DWCi_GetMatchCnt()->qr2NumEntry, DWCi_GetMatchCnt()->qr2MatchTypeExt,
            searchHostType);
    }

        if (stpAddFilter)
        {
            (void)snprintf(
                filter + len, sizeof(filter) - len, " and (%s)", stpAddFilter);
            filter[sizeof(filter) - 1] = '\0';
        }
        break;

    case 3:  // DWC_MATCH_STATE_CL_WAIT_RESV
    case 4:  // DWC_MATCH_STATE_CL_SEARCH_EVAL_HOST
    case 22: // DWC_MATCH_STATE_SEARCH_OWN
        (void)snprintf(filter, sizeof(filter), "%s = %u",
            DWC_QR2_PID_KEY_STR, profileID);
        DWCi_GetMatchCnt()->reqProfileID = profileID;
        break;

    case 9:  // DWC_MATCH_STATE_CL_SVDOWN_1
    case 10: // DWC_MATCH_STATE_CL_SVDOWN_2
    case 11: // DWC_MATCH_STATE_CL_SVDOWN_3
        (void)snprintf(filter, sizeof(filter), "%s = %u",
            DWC_QR2_PID_KEY_STR, profileID);
        async = SBFalse;
        break;

    case 12: // DWC_MATCH_STATE_CL_SEARCH_GROUPID_HOST
        (void)snprintf(filter, sizeof(filter),
            "(%s = %u) and (%s = %u) and (%s != %u)", "dwc_groupid",
            DWCi_GetMatchCnt()->groupID, "dwc_hoststate", 2, DWC_QR2_PID_KEY_STR,
            DWCi_GetMatchCnt()->profileID);
        break;

    default:
        DWC_Printf(DWC_REPORTFLAG_ERROR,
            "---DWCi_SBUpdateAsync() illegal state %d.\n",
            DWCi_GetMatchCnt()->state);
        break;
    }

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "ServerBrowserFilter : %s\n", filter);

    ServerBrowserClear(DWCi_GetMatchCnt()->sbObj);

    for (i = 0; i < DWC_MATCH_DNS_RETRY_MAX; i++)
    {
        sbError = ServerBrowserLimitUpdate(DWCi_GetMatchCnt()->sbObj, async,
            SBFalse, basicFields, numBasicFields, filter,
            DWC_SB_UPDATE_MAX_SERVERS);
        if (!sbError)
            break;
        else if (sbError != sbe_dnserror)
            break;
        DWC_Printf(0x400, "Retry\n");
    }

    if (sbError == sbe_noerror)
    {
        DWCi_GetMatchCnt()->sbUpdateRequestTick
            = DWCi_Np_GetTick()
            + (u64)(OS_BUS_CLOCK / 4 / 1000) * 30000;
        DWC_Printf(4, "ServerBrowserLimitUpdate timeout set.(%s)\n",
            __FUNCTION__);
    }

    return sbError;
}

// .text:0x803291AC DWCi_NNStartupAsync (0x370)
// CF passes the candidate's node info (profileID, public / private address + ports) instead of an SBServer
static NegotiateError DWCi_NNStartupAsync(
    int isQR2, int cookie, DWCNodeInfo* node)
{
    u32 ip;
    u32 senddata[2];
    int result;
    BOOL nnValid;
    NegotiateError nnError = ne_noerror;

    if (!isQR2)
    {
        cookie = (DWCi_GetMatchCnt()->profileID & 0x0ffff)
            | (DWCi_GetMatchCnt()->nnCookieRand << 16);
        if (node->hasPrivateAddress)
        {
            if (node->publicInetAddress
                == ServerBrowserGetMyPublicIPAddr(DWCi_GetMatchCnt()->sbObj))
            {
                DWC_Printf(0x40, "Server[%u] is behind same NAT as me.\n",
                    node->profileID);
                node->gt2IP = node->privateInetAddress;
                node->gt2Port = node->privateQueryPort;
                nnValid = FALSE;
            }
            else
            {
                DWC_Printf(0x40, "Server[%u] is behind NAT.\n", node->profileID);
                nnValid = TRUE;
            }
        }
        else
        {
            ip = SOGetHostID();
            if (IsPrivateAddr(ip))
            {
                DWC_Printf(0x40,
                    "Server[%u] is not behind NAT. But I'm behind NAT.\n",
                    node->profileID);
                nnValid = TRUE;
            }
            else
            {
                DWC_Printf(0x40,
                    "Both I and Server[%u] are not behind NAT.\n",
                    node->profileID);
                node->gt2IP = node->publicInetAddress;
                node->gt2Port = node->publicQueryPort;
                nnValid = FALSE;
            }
        }

        if (nnValid)
        {
            DWCi_GetMatchCnt()->nnCookieRand
                = (u16)DWCi_GetMathRand32(0x10000);
            DWCi_GetMatchCnt()->nnInfo.cookie = cookie;
        }
        else
        {
            senddata[0] = (u32)SOGetHostID();
            senddata[1] = DWCi_HtoLEl(
                gt2GetLocalPort(*DWCi_GetMatchCnt()->pGt2Socket));
            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_TELL_ADDR,
                node->profileID, node->publicInetAddress, node->publicQueryPort, senddata, 2);
            DWCi_GetMatchCnt()->cmdCnt.count = 0;
            if (result)
                return ne_socketerror;
            DWCi_GetMatchCnt()->nnInfo.cookie = 0;
        }

        DWCi_GetMatchCnt()->nnInfo.isQR2 = 0;
        DWCi_GetMatchCnt()->nnInfo.retryCount = 0;
        DWCi_GetMatchCnt()->nnInfo.port = node->publicQueryPort;
        DWCi_GetMatchCnt()->nnInfo.ip = node->publicInetAddress;
    }
    else
    {
        DWCi_GetMatchCnt()->nnInfo.isQR2 = 1;
        DWCi_GetMatchCnt()->nnInfo.retryCount = 0;
        DWCi_GetMatchCnt()->nnInfo.port = 0;
        DWCi_GetMatchCnt()->nnInfo.ip = 0;
        DWCi_GetMatchCnt()->nnInfo.cookie = cookie;
        nnValid = TRUE;
    }

    if (nnValid)
    {
        nnError = DWCi_DoNatNegotiationAsync(&DWCi_GetMatchCnt()->nnInfo);
    }
    else
    {
        DWCi_NNCompletedCallback(nr_success,
            gt2GetSocketSOCKET(*DWCi_GetMatchCnt()->pGt2Socket), NULL,
            &DWCi_GetMatchCnt()->nnInfo);
        DWCi_GetMatchCnt()->nnFinishTime = 0;
    }

    return nnError;
}

// .text:0x8032951C DWCi_SendMatchCommand (0x4B8)
// CF matching protocol version (the DWC_MATCHING_VERSION of this revision)
#define DWC_CF_MATCHING_VERSION 90


static inline GPResult DWCi_SendGPBuddyMsgCommand(GPConnection* connection,
    u8 command, int profileID, const char* message)
{
    char tmpStr[DWC_MATCH_COMMAND_ADD_MESSAGE_MAX];
    u32 len, len2;
    GPResult result;

    len = (u32)snprintf(tmpStr, sizeof(tmpStr), "%s%dv%s",
        DWC_GP_COMMAND_STRING, DWC_MATCHING_VERSION,
        DWC_GP_COMMAND_MATCH_STRING);
    tmpStr[len] = (char)command;
    tmpStr[len + 1] = '\0';

    if (message)
    {
        len2 = strlen(message);
        DWCi_Np_CpuCopy8(message, &tmpStr[len + 1], len2);
        tmpStr[len + 1 + len2] = '\0';
    }

    result = gpSendBuddyMessage(connection, profileID, tmpStr);

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
        "<GP> SEND-0x%02x -> [--------:-----] [pid=%u]\n", command,
        profileID);

    return result;
}
static inline SBError DWCi_SendSBMsgCommand(
    u8 command, u32 ip, u16 port, const u32 data[], int len)
{
    int i;
    DWCSBMessage senddata;
    SBError sbError;

    if (data && len)
    {
        DWCi_Np_CPUCopyFast(senddata.data, data, sizeof(u32) * len);
    }
    else
    {
        len = 0;
    }

    strcpy(senddata.header.identifier, DWC_SB_COMMAND_STRING);
    senddata.header.version = DWCi_HtoLEl(DWC_MATCHING_VERSION);
    senddata.header.command = command;
    senddata.header.size = (u8)(sizeof(u32) * len);
    senddata.header.qr2Port = DWCi_HtoLEs(DWCi_GetMatchCnt()->qr2Port);
    senddata.header.qr2IP = DWCi_GetMatchCnt()->qr2IP;
    senddata.header.profileID = DWCi_HtoLEl(DWCi_GetMatchCnt()->profileID);

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
        "<SB> SEND-0x%02x -> [%08x:%d] [pid=--------]\n", command, ip,
        port);

    for (i = 0; i < DWC_MATCH_DNS_RETRY_MAX; i++)
    {
        sbError = ServerBrowserSendMessageToServerA(DWCi_GetMatchCnt()->sbObj,
            gt2AddressToString(ip, 0, NULL), port, (gsi_char*)&senddata,
            (int)(sizeof(DWCSBMessageHeader) + senddata.header.size));

        if (!sbError)
        {
            break;
        }
        else if (sbError != sbe_dnserror)
        {
            break;
        }
    }

    return sbError;
}
static inline void DWCi_EncodeBinaryMsgCommand(DWCSBMessage* message, u8 command, const u32* data, int len)
{
    if (data && len)
    {
        DWCi_Np_CPUCopyFast(message->data, data, (u32)len * 4);
    }
    else
    {
        len = 0;
    }

    strcpy(message->header.identifier, DWC_SB_COMMAND_STRING);
    message->header.version = DWCi_HtoLEl(DWC_CF_MATCHING_VERSION);
    message->header.command = command;
    message->header.size = (u8)(sizeof(u32) * len);
    message->header.qr2Port = DWCi_HtoLEs(DWCi_GetMatchCnt()->qr2Port);
    message->header.qr2IP = DWCi_GetMatchCnt()->qr2IP;
    message->header.profileID = DWCi_HtoLEl(DWCi_GetMatchCnt()->profileID);
}

static int DWCi_SendMatchCommand(u8 command, int profileID, u32 ip, u16 port,
    const u32 data[], int len)
{
    DWCSBMessage gt2Msg;
    char encoded[DWC_MATCH_COMMAND_ADD_MESSAGE_MAX];
    DWCNodeInfo* node;
    int encLen;
    int result;
    BOOL viaGT2;

    if (DWCi_GetGT2ConnectionByProfileID(profileID, TRUE) == NULL)
    {
        viaGT2 = FALSE;
    }
    else
    {
        viaGT2 = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID) != NULL;
    }

    if (viaGT2)
    {
        DWC_Printf(4, "SEND GT2 MSG TO PID:%u\n", profileID);
        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);

        DWCi_EncodeBinaryMsgCommand(&gt2Msg, command, data, len);

        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "<GT2> SEND-0x%02x -> [--------:-----] [pid=%u]\n", command,
            profileID);

        DWCi_SendUnreliable(DWC_GetMyAID(), node->aid, &gt2Msg,
            gt2Msg.header.size + sizeof(DWCSBMessageHeader), 0);
        result = 0;
    } else if (gpIsBuddy(DWCi_GetMatchCnt()->pGpObj, profileID)) {

        encLen = 0;
        DWC_Printf(4, "SEND BUDDY MSG TO PID:%u\n", profileID);
        if (data && len)
        {
            encLen = DWC_Base64Encode(
                (const char*)data, (u32)len * 4, encoded, sizeof(encoded));
        }
        encoded[encLen] = '\0';

        result = (int)DWCi_SendGPBuddyMsgCommand(DWCi_GetMatchCnt()->pGpObj, command, profileID, encoded);
    } else {
        if (ip == 0)
        {
            node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);
            ip = node->publicInetAddress;
            port = node->publicQueryPort;
            DWC_Printf(4, "FROM BM but peer is not buddy.\n");
        }

        DWC_Printf(4, "SEND SB MSG TO IP/PORT:(%08x:%d)\n", ip, port);

        result = (int)DWCi_SendSBMsgCommand(command, ip, port, data, len);
    }

    if ((command == DWC_MATCH_COMMAND_RESV_OK)
        || (command == DWC_MATCH_COMMAND_TELL_ADDR)
        || (command == DWC_MATCH_COMMAND_LINK_CLS_REQ)
        || (command == DWC_MATCH_COMMAND_LINK_CLS_SUC))
    {
        DWCi_GetMatchCnt()->cmdCnt.command = command;
        DWCi_GetMatchCnt()->cmdCnt.port = port;
        DWCi_GetMatchCnt()->cmdCnt.ip = ip;
        DWCi_GetMatchCnt()->cmdCnt.profileID = profileID;
        DWCi_GetMatchCnt()->cmdCnt.len = len;
        DWCi_GetMatchCnt()->cmdCnt.sendTime = DWCi_Np_GetTick();
        if (data && len)
        {
            DWCi_Np_CPUCopyFast(
                DWCi_GetMatchCnt()->cmdCnt.data, data, (u32)len * 4);
        }
    }

    return result;
}

// .text:0x803299D4 DWCi_ProcessRecvMatchCommand (0x29F8)
// CF: the data words are always the little-endian wire words (the host order variant is gone); node infos and
// the reservation answer travel as packed fields (a byte in the first byte of a word, a port in the first two)

// prototypes of the functions CF inlines / calls here
void DWCi_ResetLastSomeDataRecvTimeByAid(u8 aid);


// helper: DWCi_TempNewNodeInfo_Clear (inlined in CF)
static void DWCi_TempNewNodeInfo_Clear(void)
{
    DWC_Printf(4, "DWCi_TempNewNodeInfo_Clear()\n");
    memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
}

// helper: the text of a reservation deny reason (inlined in CF)
static const char* DWCi_GetResvDenyReasonString(u32 reason)
{
    switch (reason)
    {
    case 0x10:
        return "Game server is fully occupied.";
    case 0x11:
        return "This Domain is already closed.";
    case 0x12:
        return "The condition was not satisfied.";
    case 0x13:
        return "This Domain is already locked.";
    case 0x14:
        return "It tried to go to the client for the reservation.";
    case 0x15:
        return "It is a reservation to the friend who doesn't exist in the list.";
    case 0x16:
        return "It was rejected by the attempt callback.";
    case 0x17:
        return "The reservation came from a different other host.";
    default:
        return "";
    }
}

static BOOL DWCi_ProcessRecvMatchCommand(u8 command, int srcPid, u32 srcIP,
    u16 srcPort, const u32 data[], int len)
{
    DWCNodeInfo* node;
    DWCNodeInfo* me;
    u8 aid;
    u16 port;
    u32 ip;
    u32 senddata[DWC_MAX_MATCH_IDX_LIST + 1];
    int profileID;
    int sendlen = 0;
    int result;
    int i;

    if (stpMatchCnt == NULL || DWCi_GetMatchCnt()->state == 0) {
        return TRUE;
    }

    if (DWC_GetState() == DWC_STATE_MATCHING &&
        DWCi_GetMatchCnt()->state == 1 &&
        DWCi_GetMatchCnt()->nodeInfoList.count > 0 &&
        srcPid == DWCi_NodeInfoList_GetNodeInfoForIndex(0)->profileID) {
        DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
        DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
    }

    switch (command)
    {
    case 1: // DWC_MATCH_COMMAND_RESERVATION
    {
        DWCNodeInfo newNode;
        u8 sendCommand;
        DWCMatchType matchType = (DWCMatchType)DWCi_LEtoHl(data[0]);
        u32 groupID;
        DWCTopologyType topologyType;
        BOOL priorFlag;

        memset(&newNode, 0, sizeof(newNode));
        newNode.aid = 0xFF;
        newNode.profileID = srcPid;
        newNode.publicInetAddress = data[1];
        newNode.publicQueryPort = (u16)DWCi_LEtoHl(data[2]);
        newNode.privateInetAddress = data[3];
        newNode.privateQueryPort = (u16)DWCi_LEtoHl(data[4]);
        newNode.hasPrivateAddress = (u8)DWCi_LEtoHl(data[5]);
        priorFlag = DWCi_LEtoHl(data[6]) != 0;
        topologyType = DWCi_LEtoHl(data[7]);
        memcpy(newNode.connUserData, &data[8], 4);
        groupID = DWCi_LEtoHl(data[9]);

        sendCommand = DWCi_CheckResvCommand(newNode.profileID, newNode.publicInetAddress,
            newNode.publicQueryPort, matchType, priorFlag, topologyType,
            newNode.connUserData);

        if (sendCommand == DWC_MATCH_COMMAND_RESV_OK)
        {
            memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                   sizeof(DWCNodeInfo));
            DWCi_GetMatchCnt()->tempNewNodeInfo = newNode;

            result = DWCi_ProcessResvOK(srcPid, srcIP, srcPort);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }

            if (DWCi_GetMatchCnt()->inMeshMaking != 0) {
                DWCi_GetMatchCnt()->tempNewNodeInfo.aid =
                    DWCi_NodeInfoList_GetNodeInfoForProfileID(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
                        ->aid;
            }

            if (DWCi_GetMatchCnt()->inMeshMaking == 0 &&
                DWCi_GetMatchCnt()->qr2MatchType == 2 &&
                DWCi_GetMatchCnt()->state != 14 &&
                ((DWCi_GetMatchCnt()->qr2MatchTypeExt !=
                      DWC_MATCH_TYPE_ANYBODY &&
                  DWCi_GetMatchCnt()->qr2MatchTypeExt !=
                      DWC_MATCH_TYPE_FRIEND) ||
                 DWCi_GetMatchCnt()->connectedOnceBefore != 0) &&
                DWCi_GetMatchCnt()->newClientCallback != 0) {
                (DWCi_GetMatchCnt()->newClientCallback)(
                    DWCi_GetFriendListIndex(srcPid),
                    (void *)DWCi_GetMatchCnt()->newClientParam);
            }

            me = DWCi_NodeInfoList_GetMyNodeInfo();
            senddata[sendlen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->qr2NumEntry);
            senddata[sendlen++] = DWCi_HtoLEl(me->aid);
            senddata[sendlen++] = DWCi_HtoLEl(me->profileID);
            senddata[sendlen++] = me->publicInetAddress;
            senddata[sendlen++] = DWCi_HtoLEl(me->publicQueryPort);
            senddata[sendlen++] = me->privateInetAddress;
            senddata[sendlen++] = DWCi_HtoLEl(me->privateQueryPort);
            senddata[sendlen++] = DWCi_HtoLEl(me->hasPrivateAddress);
            senddata[sendlen++] = *(const u32*)me->connUserData;
            senddata[sendlen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->groupID);
            senddata[sendlen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->tempNewNodeInfo.aid);
            senddata[sendlen++] = DWCi_HtoLEl(groupID);
            senddata[sendlen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->nodeInfoList.count);


            DWC_Printf(1, "Send <DWC_MATCH_COMMAND_RESV_OK> to: [%u]\n",
                newNode.profileID);
            DWC_Printf(1, "Send Server AID(%d)\n", me->aid);
            DWC_Printf(1, "Send Group ID(%d)\n", DWCi_GetMatchCnt()->groupID);
            DWC_Printf(1, "Send Max Entry(%d)\n",
                       DWCi_GetMatchCnt()->qr2NumEntry);
            DWC_Printf(1, "new client aid: [%u]\n",
                       DWCi_GetMatchCnt()->tempNewNodeInfo.aid);
            DWCi_SetMatchStatus(14); // DWC_MATCH_STATE_SV_OWN_NN
        }
        else if (sendCommand == DWC_MATCH_COMMAND_RESV_DENY)
        {
            DWC_Printf(1, "send RESV_DENY. %s\n",
                       DWCi_GetResvDenyReasonString(DWCi_GetMatchCnt()->denyReason));
            sendlen = 1;
            senddata[0] = DWCi_HtoLEl(DWCi_GetMatchCnt()->denyReason);
        }

        if (sendCommand == DWC_MATCH_COMMAND_DUMMY)
        {
            break;
        }

        result = DWCi_SendMatchCommand(sendCommand, newNode.profileID,
            newNode.publicInetAddress, newNode.publicQueryPort, senddata, sendlen);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
        break;
    }

    case 2: // DWC_MATCH_COMMAND_RESV_OK
    {
        DWCNodeInfo newNode;
        u32 groupID;
        u32 maxEntry = DWCi_LEtoHl(data[0]);

        me = NULL;
        if (DWCi_GetMatchCnt()->state != 3) {
            DWC_Printf(0x40, "****** Ignore NN reservation ****** \n");
            break;
        }
        DWC_Printf(0x40, "Succeeded NN reservation.\n");
        if (srcPid != DWCi_GetMatchCnt()->reqProfileID) {
            DWC_Printf(0x40,
                "****** Ignore RESV_OK. from wrong server(%d). ****** \n");
            break;
        }
        if (srcPid != DWCi_LEtoHl(data[2]))
        {
            DWC_Printf(
                0x40, "****** Ignore RESV_OK. wrong server info. ****** \n");
            break;
        }
        if (DWCi_GetMatchCnt()->nnResvCheckValue != DWCi_LEtoHl(data[11])) {
            DWC_Printf(0x40,
                "****** Ignore RESV_OK. wrong resvCheckValue. ****** \n");
            DWC_Printf(0x40, "****** myvalue:%u  wrong value:%u. ****** \n",
                       DWCi_GetMatchCnt()->nnResvCheckValue, DWCi_LEtoHl(data[11]));
            break;
        }

        DWCi_GetMatchCnt()->resvWaitCount = 0;
        DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
        DWCi_GetMatchCnt()->cmdResendFlag = 0;

        memset(&newNode, 0, sizeof(newNode));
        newNode.aid = (u8)DWCi_LEtoHl(data[1]);
        newNode.profileID = DWCi_LEtoHl(data[2]);
        newNode.publicInetAddress = data[3];
        newNode.publicQueryPort = (u16)DWCi_LEtoHl(data[4]);
        newNode.privateInetAddress = data[5];
        newNode.privateQueryPort = (u16)DWCi_LEtoHl(data[6]);
        newNode.hasPrivateAddress = (u8)DWCi_LEtoHl(data[7]);
        memcpy(newNode.connUserData, &data[8], 4);
        groupID = DWCi_LEtoHl(data[9]);

        if (DWCi_GetMatchCnt()->inMeshMaking == 0) {
            me = DWCi_NodeInfoList_GetMyNodeInfo();
            me->aid = (u8)DWCi_LEtoHl(data[10]);
        }

        DWCi_GetMatchCnt()->tempNewNodeInfo = newNode;
        DWC_Printf(4, "Server IP:%x, port:%d\n", newNode.publicInetAddress,
            newNode.privateQueryPort);

        if (DWCi_GetMatchCnt()->qr2MatchType == 3) {
            DWCi_GetMatchCnt()->groupID = groupID;
            DWCi_GetMatchCnt()->qr2NumEntry = (u8)maxEntry;
            DWC_Printf(4, "Recv Server AID:%d\n", newNode.aid);
            DWC_Printf(0x10000, "Recv GroupID:%u.\n", DWCi_GetMatchCnt()->groupID);
            DWC_Printf(0x10000, "Recv Max Entry:%d.\n",
                       DWCi_GetMatchCnt()->qr2NumEntry);
            DWC_Printf(0x10000, "Recv User Data: 0[%d] 1[%d] 2[%d] 3[%d].\n",
                       ((u8 *)DWCi_GetMatchCnt()->tempNewNodeInfo.connUserData)[0],
                       ((u8 *)DWCi_GetMatchCnt()->tempNewNodeInfo.connUserData)[1],
                       ((u8 *)DWCi_GetMatchCnt()->tempNewNodeInfo.connUserData)[2],
                       ((u8 *)DWCi_GetMatchCnt()->tempNewNodeInfo.connUserData)[3]);
            if (me != NULL)
            {
                DWC_Printf(0x10000, "My AID:%d.\n", me->aid);
            }
        }

        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND &&
            DWCi_GetMatchCnt()->evalCallback != 0 &&
            DWCi_GetMatchCnt()->inMeshMaking == 0 &&
            DWCi_GetMatchCnt()->connectGroupID == 0) {
            DWCi_SetMatchStatus(4); // DWC_MATCH_STATE_CL_SEARCH_EVAL_HOST
            if (!(DWCi_HandleSBError(DWCi_SBUpdateAsync(srcPid)) == 0)) {
                return FALSE;
            }
            break;
        }

        DWCi_SetMatchStatus(5); // DWC_MATCH_STATE_CL_NN
        if (!(DWCi_HandleNNError(DWCi_NNStartupAsync(
                  0, 0, &DWCi_GetMatchCnt()->tempNewNodeInfo)) == 0)) {
            return FALSE;
        }
        break;
    }

    case 3: // DWC_MATCH_COMMAND_RESV_DENY
    {
        u32 reason;
        if (DWCi_GetMatchCnt()->state != 3) {
            break;
        }
        if (srcPid != DWCi_GetMatchCnt()->reqProfileID) {
            break;
        }
        DWC_Printf(0x40, "Reservation was denied by %u.\n", srcPid);
        reason = data[0];
        DWC_Printf(0x40, "%s\n",
                   DWCi_GetResvDenyReasonString((u8)DWCi_LEtoHl(reason)));
        if (DWCi_GetMatchCnt()->connectGroupID != 0 ||
            DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL) {
            DWCError error = (DWCError)(DWCi_LEtoHl(reason) == 0x10 ? 0xC : 0x17);
            DWCi_StopMatching(error, 0);
            return FALSE;
        } else {
            return DWCi_RetryReserving(DWCi_GetMatchCnt()->reqProfileID);
        }
    }

    case 4: // DWC_MATCH_COMMAND_RESV_WAIT
        if (DWCi_GetMatchCnt()->state != 3) {
            break;
        }
        if (DWCi_StopMeshMaking() == 1)
        {
            DWCi_SetMatchStatus(1); // DWC_MATCH_STATE_CL_WAITING
            break;
        }
        if (DWCi_GetMatchCnt()->connectGroupID != 0) {
            break;
        }
        if (srcPid != DWCi_GetMatchCnt()->reqProfileID) {
            break;
        }
        DWCi_GetMatchCnt()->cmdTimeoutStartTick = DWCi_Np_GetTick();
        DWCi_GetMatchCnt()->resvWaitCount = 0;
        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY) {
            DWCi_TempNewNodeInfo_Clear();
            DWCi_SetMatchStatus(2); // DWC_MATCH_STATE_CL_SEARCH_HOST
            DWCi_GetMatchCnt()->sbUpdateFlag = 1;
            DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
            break;
        }
        if (DWCi_GetMatchCnt()->qr2MatchTypeExt != DWC_MATCH_TYPE_FRIEND) {
            break;
        }
        DWCi_TempNewNodeInfo_Clear();
        DWCi_SendResvCommandToFriend(TRUE, FALSE, 0);
        break;

    case 5: // DWC_MATCH_COMMAND_RESV_CANCEL
    {
        int resetLevel;

        if (DWCi_GetMatchCnt()->qr2IsReserved == 0) {
            break;
        }
        if (srcPid != DWCi_GetMatchCnt()->qr2Reservation) {
            break;
        }
        if (DWCi_StopMeshMaking() == 1)
        {
            DWCi_SetMatchStatus(1); // DWC_MATCH_STATE_CL_WAITING
            break;
        }
        if (DWCi_GetMatchCnt()->qr2MatchType == 2 &&
            DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0 &&
            DWCi_GetGT2ConnectionByProfileID(
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID, FALSE) &&
            srcPid == DWCi_GetMatchCnt()->tempNewNodeInfo.profileID) {
            gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
        }
        resetLevel = DWC_MATCH_RESET_CONTINUE;
        if ((DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY ||
             DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND) &&
            DWCi_GetMatchCnt()->connectedOnceBefore == 0) {
            resetLevel = DWC_MATCH_RESET_RESTART;
        }
        if (!(DWCi_CancelPreConnectedServerProcessWithResetLevel(srcPid,
                                                                 resetLevel))) {
            return FALSE;
        }
        break;
    }

    case 6: // DWC_MATCH_COMMAND_TELL_ADDR
    {
        struct sockaddr_in remoteaddr;

        ip = data[0];
        port = (u16)DWCi_LEtoHl(data[1]);

        if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID == 0) {
            DWC_Printf(0x40, "new node info is invaild. ignore.\n");
            break;
        }
        DWC_Printf(0x40,
            "NN parent is behind same NAT as me. Received IP %x & port %d\n",
            ip, port);
        if ((s32)DWCi_GetMatchCnt()->topologyType != 2 &&
            DWCi_GetMatchCnt()->qr2MatchType == 3 &&
            DWCi_GetMatchCnt()->inMeshMaking == 0) {
            DWC_Printf(0x40, "I'm client. not in mesh making..ignore.\n");
            break;
        }
        if (DWCi_GetMatchCnt()->state == 1) {
            DWCi_SetMatchStatus(5); // DWC_MATCH_STATE_CL_NN
        } else if ((DWCi_GetMatchCnt()->state != 5 &&
                    DWCi_GetMatchCnt()->state != 14) ||
                   srcPid != DWCi_GetMatchCnt()->qr2Reservation) {
            DWC_Printf(0x40, "But already canceled reservation.\n");
            break;
        }
        DWCi_GetMatchCnt()->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
        remoteaddr.sin_addr.addr = ip;
        remoteaddr.sin_port = SOHtoNs(port);
        DWCi_GetMatchCnt()->nnInfo.isQR2 = 1;
        DWCi_NNCompletedCallback(
            nr_success, gt2GetSocketSOCKET(*DWCi_GetMatchCnt()->pGt2Socket),
            &remoteaddr, &DWCi_GetMatchCnt()->nnInfo);
        DWCi_GetMatchCnt()->nnFinishTime = 0;
        break;
    }

    case 7: // DWC_MATCH_COMMAND_NEW_PID_AID
    {

        if (DWCi_StopMeshMaking() == 1)
        {
            DWCi_SetMatchStatus(1); // DWC_MATCH_STATE_CL_WAITING
        }
        if (DWCi_GetMatchCnt()->state != 1 ||
            srcPid != DWCi_NodeInfoList_GetServerNodeInfo()->profileID) {
            DWC_Printf(4, "Ignore delayed NEW_PID_AID command.\n");
            break;
        }
        profileID = (int)DWCi_LEtoHl(data[0]);
        aid = (u8)DWCi_LEtoHl(data[1]);
        if (DWCi_GetMatchCnt()->tempNewClientInfo.profileID != 0) {
            DWC_Printf(
                4, "Ignore dup NEW_PID_AID command. I'm waiting pid(%u)..\n",
                DWCi_GetMatchCnt()->tempNewClientInfo.profileID);
            break;
        }
        if (DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID) != NULL
            && DWCi_NodeInfoList_GetNodeInfoForAID(aid) != NULL
            && profileID == DWCi_NodeInfoList_GetNodeInfoForAID(aid)->profileID)
        {
            DWC_Printf(4,
                "*** NEW_PID_AID. pid(%u) aid(%d) is already my list... "
                "ignore.\n",
                profileID, aid);
            break;
        }
        if (DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID) != NULL
            || DWCi_NodeInfoList_GetNodeInfoForAID(aid) != NULL)
        {
            DWC_Printf(
                4, "NEW_PID_AID. pid(%u) aid(%d) is in list..close\n",
                profileID, aid);
            DWC_CloseConnectionHard(aid);
        }
        DWCi_GetMatchCnt()->serverDownBuff = 0;
        if ((s32)DWCi_GetMatchCnt()->topologyType == 2) {
            DWCi_GetMatchCnt()->tempNewNodeInfo.profileID = profileID;
            DWCi_GetMatchCnt()->tempNewNodeInfo.aid = aid;
        } else {
            DWCi_GetMatchCnt()->tempNewClientInfo.profileID = profileID;
        }
        if (DWCi_GetMatchCnt()->newClientCallback != 0) {
            (DWCi_GetMatchCnt()->newClientCallback)(
                DWCi_GetFriendListIndex(profileID),
                (void *)DWCi_GetMatchCnt()->newClientParam);
        }
        DWC_Printf(0x40, "Received new client's profileID = %u & aid = %d.\n",
            profileID, aid);
        break;
    }

    case 9: // DWC_MATCH_COMMAND_LINK_CLS_SUC
        node = DWCi_NodeInfoList_GetNodeInfoForIndex(
            DWCi_GetMatchCnt()->nodeInfoList.count - 1 -
            DWCi_GetMatchCnt()->clLinkProgress);
        if (DWCi_GetMatchCnt()->state == 16 && node != NULL &&
            node->profileID == DWCi_LEtoHl(data[0])) {
            DWCi_GetMatchCnt()->clLinkProgress++;
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
            break;
        }
        DWC_Printf(0x40, "Ignore delayed LINK_CLS_SUC command %d.\n",
            DWC_MATCH_COMMAND_LINK_CLS_SUC);
        break;

    case 8: // DWC_MATCH_COMMAND_LINK_CLS_REQ
    {
        u32 resendData[2];


        if (DWCi_GetMatchCnt()->state != 1 ||
            srcPid != (node = &DWCi_GetMatchCnt()->tempNewNodeInfo)->profileID) {
            DWC_Printf(4, "Ignore delayed LINK_CLS_REQ command.\n");
            break;
        }
        profileID = (int)DWCi_LEtoHl(data[0]);
        aid = (u8)DWCi_LEtoHl(data[2]);
        if (DWCi_GetGT2Connection(aid) != NULL)
        {
            resendData[0] = DWCi_HtoLEl((u32)profileID);
            DWC_Printf(0x40, "Resend command %d for delayed command %d.\n",
                DWC_MATCH_COMMAND_LINK_CLS_SUC, DWC_MATCH_COMMAND_LINK_CLS_REQ);
            result = DWCi_SendMatchCommand(
                DWC_MATCH_COMMAND_LINK_CLS_SUC, srcPid,
                node->publicInetAddress,
                node->publicQueryPort, resendData,
                1);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }
            break;
        }
        else
        {
            DWCi_GetMatchCnt()->tempNewClientInfo = DWCi_GetMatchCnt()->tempNewNodeInfo;
            node->profileID = profileID;
            node->aid = aid;
            node->publicInetAddress = data[3];
            node->publicQueryPort = (u16)DWCi_LEtoHl(data[4]);
            node->privateInetAddress = data[5];
            node->privateQueryPort = (u16)DWCi_LEtoHl(data[6]);
            node->hasPrivateAddress = (u8)DWCi_LEtoHl(data[7]);
            memcpy(node->connUserData, &data[8], 4);
            DWC_Printf(4, "Client IP:%d.%d.%d.%d, port:%d\n", node->publicInetAddress >> 24,
                (node->publicInetAddress >> 16) & 0xff, (node->publicInetAddress >> 8) & 0xff,
                node->publicInetAddress & 0xff, node->publicQueryPort);
            DWCi_SetMatchStatus(5); // DWC_MATCH_STATE_CL_NN
            DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
            DWCi_GetMatchCnt()->cmdResendFlag = 0;
            DWCi_NNStartupAsync(0, 0, node);
            break;
        }
    }

    case 10: // DWC_MATCH_COMMAND_CLOSE_LINK
    {

        if (DWCi_GetMatchCnt()->state != 1 ||
            srcPid != DWCi_NodeInfoList_GetServerNodeInfo()->profileID) {
            DWC_Printf(4, "Ignore delayed LINK_CLS_REP command.\n");
            break;
        }
        profileID = DWCi_LEtoHl(data[0]);
        aid = (u8)DWCi_LEtoHl(data[1]);
        if (profileID != DWCi_GetMatchCnt()->tempNewNodeInfo.profileID ||
            aid != DWCi_GetMatchCnt()->tempNewNodeInfo.aid) {
            DWC_Printf(
                1,
                "LINK_CLS_REP: recovery aid(%d->%d) and profileid(%d->%d).\n",
                DWCi_GetMatchCnt()->tempNewNodeInfo.aid, aid,
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID, profileID);
            DWCi_GetMatchCnt()->tempNewNodeInfo.aid = aid;
            DWCi_GetMatchCnt()->tempNewNodeInfo.profileID = profileID;
            break;
        }
        DWC_Printf(1, "LINK_CLS_REP: client info is valid.\n");
        break;
    }

    case 12: // DWC_MATCH_COMMAND_CANCEL
        if (srcPid != DWCi_GetMatchCnt()->tempNewNodeInfo.profileID) {
            DWC_Printf(4, "Ignore delayed CANCEL command.\n");
            break;
        }
        DWC_Printf(0x40, "Received cancel command from %u data[0] = %d.\n",
            srcPid, DWCi_LEtoHl(data[0]));
        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY ||
            DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND) {
            if (!DWCi_CancelPreConnectedClientProcess(srcPid, 0x18, -80430)) {
                return FALSE;
            }
            break;
        }
            if (DWCi_GetMatchCnt()->qr2MatchType != 3) {
                break;
            }
            DWCi_GetMatchCnt()->cbEventPid = srcPid;
            DWCi_GetMatchCnt()->closeState =
                DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
            gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
            DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
            DWCi_RestartFromCancel(DWC_MATCH_RESET_ALL);
            break;

    case 13: // DWC_MATCH_COMMAND_CANCEL_SYN
    case 14: // DWC_MATCH_COMMAND_CANCEL_SYN_ACK
    case 15: // DWC_MATCH_COMMAND_CANCEL_ACK
        if (!(DWCi_ProcessCancelMatchSynCommand(srcPid, command,
                                                DWCi_LEtoHl(data[0])))) {
            return FALSE;
        }
        break;

    case 0x10: // DWC_MATCH_COMMAND_SC_CLOSE_CL
    {
        u8 closeState;

        node = DWCi_NodeInfoList_GetServerNodeInfo();
        if (node == NULL)
        {
            DWC_Printf(4, "server node is NULL!!\n");
            return TRUE;
        }
        if (srcPid != node->profileID)
        {
            return TRUE;
        }
        DWC_Printf(4, "Close shutdown client.\n");
        closeState = DWCi_GetMatchCnt()->closeState;
        DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
        for (i = 0; i < len; i++)
        {
            node = DWCi_NodeInfoList_GetNodeInfoForProfileID(
                (int)DWCi_LEtoHl(data[i]));
            if (node == NULL && DWCi_GetMatchCnt()->tempNewClientInfo.profileID ==
                                    (int)DWCi_LEtoHl(data[i])) {
                DWC_CloseConnectionHard(DWCi_GetMatchCnt()->tempNewClientInfo.aid);
                DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;
            } else if (node != NULL) {
                DWC_CloseConnectionHard(node->aid);
            }
        }
        DWCi_GetMatchCnt()->closeState = closeState;
        break;
    }

    case 0x40: // DWC_MATCH_COMMAND_CL_WAIT_POLL
        if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID == 0) {
            break;
        }
        if (srcPid != DWCi_GetMatchCnt()->tempNewNodeInfo.profileID) {
            break;
        }
        result = DWCi_SendMatchCommand(
            DWC_MATCH_COMMAND_SV_KA_TO_CL, srcPid, srcIP, srcPort, NULL, 0);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
        break;

    case 0x41: // DWC_MATCH_COMMAND_SV_KA_TO_CL
        break;

    case 0x52: // server down query
    {
        u8 myAid;
        u8 srcAid;
        BOOL serverDown;

        if (DWCi_GetMatchCnt()->connectedOnceBefore == 0) {
            break;
        }
        if (DWCi_IsBackupMine())
        {
            DWC_Printf(0x10000, "WARN!! SVDOWNQUERY to Next server!!\n");
            return FALSE;
        }
        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(
            DWCi_GetMatchCnt()->profileID);
        myAid = node == NULL ? 0xFF : node->aid;
        if (myAid == 0xFF)
        {
            return FALSE;
        }
        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(srcPid);
        srcAid = node == NULL ? 0xFF : node->aid;
        if (srcAid == 0xFF)
        {
            DWC_Printf(0x10000, "already next server downed[%u].\n", srcPid);
            return FALSE;
        }
        if (srcPid != DWCi_NodeInfoList_GetServerNodeInfo()->profileID)
        {
            serverDown = FALSE;
        }
        else
        {
            serverDown = ((1 << myAid) & DWCi_GetMatchCnt()->serverDownBuff) != 0;
        }
        result = DWCi_SendMatchCommand((u8)(serverDown ? 0x53 : 0x54), srcPid,
            srcIP, srcPort, NULL, 0);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
        DWC_Printf(0x10000, "Send ServerDown is %s:%u.\n",
            serverDown ? "down" : "alive", srcPid);
        DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
        if (!serverDown)
        {
            break;
        }
        if (DWCi_GetMatchCnt()->state != 10) {
            break;
        }
        DWCi_SetMatchStatus(1); // DWC_MATCH_STATE_CL_WAITING
        DWCi_ResetAllNodeRecvTimes();
        break;
    }

    case 0x53: // server down ACK
    {
        u32 srcAid;

        if (DWCi_GetMatchCnt()->connectedOnceBefore == 0) {
            break;
        }
        if (DWCi_IsBackupMine())
        {
            node = DWCi_NodeInfoList_GetNodeInfoForProfileID(srcPid);
            srcAid = node == NULL ? 0xFF : node->aid;
            if (srcAid == 0xFF)
            {
                DWC_Printf(0x10000, "WARN!! idx of PID %u is 0xff.\n", srcPid);
                return FALSE;
            }
            DWCi_GetMatchCnt()->serverDownBuff |= 1 << srcAid;
            DWC_Printf(0x10000, "Recv ServerDown State: ACK:%u.\n", srcPid);
            result = DWCi_SendMatchCommand(0x53, srcPid, srcIP, srcPort, NULL, 0);
            DWC_Printf(0x10000, "Send ACK to [%u]: IP:%d Port:%d\n", srcPid, srcIP,
                srcPort);
            if (DWCi_HandleMatchCommandError(result))
            {
                DWC_Printf(0x10000, "Send Failed!\n");
                return FALSE;
            }
            break;
        }
        DWC_Printf(0x10000, "Recv ACK From Backup.\n");
        break;
    }

    case 0x54: // server down NAK
        if (DWCi_GetMatchCnt()->connectedOnceBefore == 0) {
            break;
        }
        if (DWCi_GetMatchCnt()->nodeInfoList.count * 5 <
            ++DWCi_GetMatchCnt()->serverDownNakCount) {
            DWC_CloseAllConnectionsHard();
            break;
        }
        DWC_Printf(0x10000, "Recv ServerDown State: NAK:%u.\n", srcPid);
        DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
        break;

    case 0x55: // server down keep
        if (DWCi_GetMatchCnt()->connectedOnceBefore == 0) {
            break;
        }
        DWC_Printf(0x10000, "recv SVDOWN_KEEP.\n");
        break;

    case 0x70:
        DWCi_SBUpdateAsync(0);
        DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
        DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
        DWCi_SetMatchStatus(12); // DWC_MATCH_STATE_CL_SEARCH_GROUPID_HOST
        DWC_Printf(0x10000, "Retry SB Search.\n");
        break;

    case 0x82: // suspend
    {
        u32 suspend;
        u8 hostState;

        profileID = (int)DWCi_LEtoHl(data[0]);
        hostState = (u8)DWCi_LEtoHl(data[1]);
        suspend = (u8)DWCi_LEtoHl(data[2]);

        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(
            profileID);
        if (node == NULL)
        {
            DWC_Printf(4, "Ignore suspend cmd(from [%u] who isn't in list).\n",
                profileID);
            break;
        }
        if ((s32)DWCi_GetMatchCnt()->hostState == 2) {
            aid = (u8)DWCi_LEtoHl(data[3]);
            if (hostState == 1)
            {
                DWC_Printf(
                    4, "Ignore suspend cmd(from SV[%u],I'm SV).\n");
                break;
            }
            DWC_Printf(0x10000, "Server: AID(%u) suspend(%u).\n",
                aid, suspend);
            DWCi_ProcessRequestMatch(
                aid, (int)suspend, 1);
            break;
        }
        if ((s32)DWCi_GetMatchCnt()->hostState != 1) {
            break;
        }
        if (hostState == 0)
        {
            DWC_Printf(
                4, "Ignore suspend cmd(from CL[%u],I'm CL).\n");
            break;
        }
        if (len == 2)
        {
            if (DWCi_GetMatchCnt()->suspendServerResponse != 1) {
                DWCi_GetMatchCnt()->suspendServerResponse = 1;
                DWCi_GetMatchCnt()->suspendWaitTime = 8000;
                DWCi_GetMatchCnt()->suspendRequestTick = DWCi_Np_GetTick();
                DWC_Printf(4,
                    "*** DWC_MATCH_COMMAND_SUSPEND_MATCH: get server responce.\n");
                break;
            }
            DWC_Printf(4,
                "*** DWC_MATCH_COMMAND_SUSPEND_MATCH: It has not completed it "
                "yet.\n");
            break;
        }
        else
        {
            u32 bitmap = DWCi_LEtoHl(data[3]);

            DWCi_GetMatchCnt()->suspendServerResponse = 1;
            DWC_Printf(0x10000, "Client: suspend(%u). Bitmap(%u)\n", suspend,
                bitmap);
            DWCi_GetMatchCnt()->suspendMatchBitmap = bitmap;
            break;
        }
    }

    case 0x83:
        DWCi_ProcessMatchPollingPacket(data);
        break;

    default:
        DWC_Printf(2, "Received unexpected matching command 0x%02x.\n", command);
        break;
    }
    return TRUE;
}


// Query-port accessors retained while matching existing inline call sites.




// .text:0x8032C3CC DWCi_CheckResvCommand (0x2D8)
static inline BOOL DWCi_NodeInfoList_HasNoGT2ConnectionToProfileID(int profileID)
{
    DWCNodeInfo* pNodeInfo = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);
    if (pNodeInfo == NULL)
    {
        return FALSE;
    }
    if (pNodeInfo == DWCi_NodeInfoList_GetMyNodeInfo())
    {
        return FALSE;
    }
    return DWCi_GetGT2Connection(pNodeInfo->aid) == NULL;
}
static u8 DWCi_CheckResvCommand(int profileID, u32 qr2IP, u16 qr2Port,
    u32 matchType, BOOL priorFlag, u32 topology, const void* newClientInfo)
{
    u8 sendCommand;
    DWCNodeInfo* node;
    BOOL ok;

    stpMatchCnt->denyReason = 1;

    switch (stpMatchCnt->hostState)
    {
    case 1:
        ok = DWCi_NodeInfoList_HasNoGT2ConnectionToProfileID(profileID);
        if (ok)
        {
            if (stpMatchCnt->inMeshMaking == 0)
            {
                if (stpMatchCnt->state == 1)
                {
                    if (stpMatchCnt->tempNewClientInfo.profileID == 0)
                    {
                        stpMatchCnt->inMeshMaking = 1;
                        stpMatchCnt->lastStateBeforeStopMeshMaking = 0;
                        memset(&stpMatchCnt->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCNodeInfo));
                        sendCommand = DWC_MATCH_COMMAND_RESV_OK;
                        stpMatchCnt->cmdCnt.sendTime = OSGetTime();
                        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 1-A\n");
                    }
                    else
                    {
                        sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
                        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 1-B\n");
                    }
                }
                else
                {
                    sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 2\n");
                }
            }
            else if (stpMatchCnt->state == 3)
            {
                node = DWCi_NodeInfoList_GetMyNodeInfo();
                if (profileID == stpMatchCnt->tempNewNodeInfo.profileID)
                {
                    if (node->profileID < profileID)
                    {
                        sendCommand = DWC_MATCH_COMMAND_RESV_OK;
                        stpMatchCnt->cmdCnt.sendTime = OSGetTime();
                        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 3\n");
                    }
                    else
                    {
                        sendCommand = DWC_MATCH_COMMAND_DUMMY;
                        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 4\n");
                    }
                }
                else
                {
                    sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 5\n");
                }
            }
            else
            {
                sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "*** 6\n");
            }
        }
        else
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
            stpMatchCnt->denyReason = 0x14;
        }
        break;

    case 0:
    case 2:
        sendCommand = DWCi_CheckResvCommand_Sub(profileID, qr2IP, qr2Port,
            matchType, priorFlag, topology, newClientInfo);
        break;
    }

    return sendCommand;
}

// .text:0x8032C6A4 DWCi_CheckResvCommand_Sub (0x49C)
static u8 DWCi_CheckResvCommand_Sub(int profileID, u32 qr2IP, u16 qr2Port,
    u32 matchType, BOOL priorFlag, u32 topology, const void* newClientInfo)
{
    u8 sendCommand;
    BOOL locked;
    BOOL found;
    int i;
    int numHost;

    if (stpMatchCnt->serverLocked == 0)
    {
        locked = FALSE;
    }
    else if (DWC_IsServerMyself()
        && (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
        && stpMatchCnt->firstServer == 0)
    {
        locked = TRUE;
    }
    else
    {
        locked = FALSE;
    }

    if (locked)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "*** this server is locked!!!\n");
        stpMatchCnt->denyReason = 0x13;
        return DWC_MATCH_COMMAND_RESV_DENY;
    }

    if (!priorFlag)
    {
        if ((matchType == DWC_MATCH_TYPE_ANYBODY
                && stpMatchCnt->qr2MatchTypeExt != DWC_MATCH_TYPE_ANYBODY)
            || (matchType == DWC_MATCH_TYPE_FRIEND
                && stpMatchCnt->qr2MatchTypeExt != DWC_MATCH_TYPE_FRIEND)
            || (matchType == DWC_MATCH_TYPE_SC_CL
                && stpMatchCnt->qr2MatchTypeExt != DWC_MATCH_TYPE_SC_SV))
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "*** my match type(%d) resv match type(%d).\n",
                stpMatchCnt->qr2MatchTypeExt, matchType);
            stpMatchCnt->denyReason = 0x12;
            return DWC_MATCH_COMMAND_RESV_DENY;
        }

        if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
        {
            if (!gpIsBuddy(stpMatchCnt->pGpObj, profileID)
                || DWCi_GetFriendListIndex(profileID) == -1)
            {
                return DWC_MATCH_COMMAND_DUMMY;
            }

            if (stpMatchCnt->friendList == NULL)
            {
                found = FALSE;
            }
            else
            {
                for (i = 0; i < stpMatchCnt->friendIdxListLen; i++)
                {
                    int pid = DWCi_GetProfileIDFromList(
                        stpMatchCnt->friendIdxList[i]);
                    if (pid > 0 && pid == profileID)
                    {
                        found = TRUE;
                        goto foundLabel;
                    }
                }
                found = FALSE;
            }
        foundLabel:
            if (!found)
            {
                stpMatchCnt->denyReason = 0x15;
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "This friend doesn't exist in friendIdxList.\n");
                return DWC_MATCH_COMMAND_RESV_DENY;
            }
        }
        else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
        {
            if (!gpIsBuddy(stpMatchCnt->pGpObj, profileID)
                || DWCi_GetFriendListIndex(profileID) == -1)
            {
                return DWC_MATCH_COMMAND_DUMMY;
            }
        }
    }

    numHost = DWC_GetNumConnectionHost();
    if (stpMatchCnt->qr2NumEntry + 1 == numHost)
    {
        stpMatchCnt->denyReason = 0x10;
        sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
    }
    else if (stpMatchCnt->state == 0x16)
    {
        sendCommand = DWC_MATCH_COMMAND_DUMMY;
    }
    else if (DWC_GetSuspendMatch() || stpMatchCnt->isSuspending != 0)
    {
        sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
        stpMatchCnt->denyReason = 0x11;
    }
    else if (topology != stpMatchCnt->topologyType)
    {
        sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "*** my topology(%d) resv topology(%d).\n", stpMatchCnt->topologyType,
            topology);
        stpMatchCnt->denyReason = 0x12;
    }
    else if (DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID))
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "*** pid(%u) has already been in list.\n", profileID);
        sendCommand = DWC_MATCH_COMMAND_DUMMY;
    }
    else if ((stpMatchCnt->state != 0xD && stpMatchCnt->state != 2
                 && stpMatchCnt->state != 3)
        || (stpMatchCnt->qr2IP == 0 && stpMatchCnt->qr2Port == 0)
        || (qr2IP == 0 && qr2Port == 0))
    {
        if (stpMatchCnt->state == 0xE && profileID == stpMatchCnt->qr2Reservation)
        {
            sendCommand = DWC_MATCH_COMMAND_DUMMY;
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "please wait: qr2IP:%u qr2Port:%u qr2IPM:%u qr2PortM: %u\n",
                qr2IP, qr2Port, stpMatchCnt->qr2IP, stpMatchCnt->qr2Port);
            sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
        }
    }
    else if (stpMatchCnt->qr2MatchType != 2 && stpMatchCnt->reqProfileID != 0)
    {
        if (profileID == stpMatchCnt->reqProfileID)
        {
            if (stpMatchCnt->profileID < profileID)
            {
                sendCommand = DWC_MATCH_COMMAND_RESV_OK;
            }
            else
            {
                sendCommand = DWC_MATCH_COMMAND_DUMMY;
            }
        }
        else
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
            stpMatchCnt->denyReason = 0x17;
        }
    }
    else
    {
        sendCommand = DWC_MATCH_COMMAND_RESV_OK;
    }

    if (sendCommand == DWC_MATCH_COMMAND_RESV_OK)
    {
        if (stpMatchCnt->connectAttemptCallback != NULL
            && !((BOOL(*)(const void*, void*))stpMatchCnt->connectAttemptCallback)(
                newClientInfo, stpMatchCnt->connectAttemptParam))
        {
            stpMatchCnt->denyReason = 0x16;
            return DWC_MATCH_COMMAND_RESV_DENY;
        }

        if (stpMatchCnt->hostState == 0)
        {
            DWCi_SetupGameServer_InterChange(TRUE);
        }
    }

    return sendCommand;
}

// .text:0x8032CB40 DWCi_ProcessResvOK (0x1B8)
static int DWCi_ProcessResvOK(int profileID, u32 ip, u16 port)
{
    u32 senddata[17];
    int result;
    int i;
    DWCNodeInfo* self;
    DWCNodeInfo* node;

    if (stpMatchCnt->qr2IsReserved && profileID == stpMatchCnt->qr2Reservation)
    {
        return 0;
    }

    stpMatchCnt->qr2IsReserved = 1;
    stpMatchCnt->qr2Reservation = profileID;
    stpMatchCnt->cmdResendFlag = FALSE;
    stpMatchCnt->cmdTimeoutTime = 0;
    qr2_send_statechanged(stpMatchCnt->qr2Obj);
    stpMatchCnt->reqProfileID = 0;
    stpMatchCnt->tempNewNodeInfo.aid = DWCi_NodeInfoList_GetFreeAID();

    if (stpMatchCnt->inMeshMaking == 0)
    {
        senddata[0] = DWCi_HtoLEl(stpMatchCnt->tempNewNodeInfo.profileID);
        senddata[1] = DWCi_HtoLEl(stpMatchCnt->tempNewNodeInfo.aid);

        self = DWCi_NodeInfoList_GetNodeInfoForProfileID(
            stpMatchCnt->profileID);
        for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
        {
            node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
            if (self->profileID != node->profileID)
            {
                result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_NEW_PID_AID,
                    node->profileID, node->publicInetAddress, (node)->publicQueryPort, senddata,
                    2);
                if (result)
                {
                    return result;
                }
            }
        }
    }

    return 0;
}


// .text:0x8032CCF8 DWCi_SendResvCommand (0x2E0)
static int DWCi_SendResvCommand(int profileID, BOOL delay)
{
    u32 senddata[16];
    int datalen;
    int result;
    u32 ip = 0;
    u16 port = 0;
    DWCNodeInfo* self;
    OSTime tick;

    datalen = 0;

    if (delay ||
        (DWCi_GetMatchCnt()->qr2IP == 0 && DWCi_GetMatchCnt()->qr2Port == 0)) {
        DWCi_GetMatchCnt()->cmdResendFlag = TRUE;
        tick = OSGetTime();
        DWCi_GetMatchCnt()->cmdResendTick = tick;
        DWCi_GetMatchCnt()->reqProfileID = 0;

        if (!delay)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "Delay ResvCommand - qr2IP & qr2Port = 0.\n");
        }
        return 0;
    }

    if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY) {
        ip = DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress;
        port = (&DWCi_GetMatchCnt()->tempNewNodeInfo)->publicQueryPort;
        DWCi_GetMatchCnt()->reqProfileID = profileID;
    } else {
        DWCi_GetMatchCnt()->reqProfileID = profileID;
        if (!gpIsBuddy(DWCi_GetMatchCnt()->pGpObj, profileID)) {
            ip = DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress;
            port = (&DWCi_GetMatchCnt()->tempNewNodeInfo)->publicQueryPort;
        }
    }

    DWCi_GetMatchCnt()->cmdTimeoutTime = DWC_MATCH_CMD_RESV_TIMEOUT_MSEC;
    tick = OSGetTime();
    DWCi_GetMatchCnt()->cmdTimeoutStartTick = tick;
    DWCi_GetMatchCnt()->cmdResendFlag = FALSE;

    self = DWCi_NodeInfoList_GetNodeInfoForProfileID(
        DWCi_GetMatchCnt()->profileID);


    senddata[datalen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->qr2MatchTypeExt);
    senddata[datalen++] = self->publicInetAddress;
    senddata[datalen++] = DWCi_HtoLEl((self)->publicQueryPort);
    senddata[datalen++] = self->privateInetAddress;
    senddata[datalen++] = DWCi_HtoLEl((self)->privateQueryPort);
    senddata[datalen++] = DWCi_HtoLEl(self->hasPrivateAddress);
    senddata[datalen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->connectGroupID);
    senddata[datalen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->topologyType);
    senddata[datalen++] = DWCi_GetMatchCnt()->_8D0;
    DWCi_GetMatchCnt()->nnResvCheckValue = DWCi_GetMathRand32(-2) + 1;
    senddata[datalen++] = DWCi_HtoLEl(DWCi_GetMatchCnt()->nnResvCheckValue);


    result = DWCi_SendMatchCommand(
        DWC_MATCH_COMMAND_RESERVATION, profileID, ip, port, senddata, datalen);
    return result;
}

// .text:0x8032CFD8 DWCi_SendResvCommandToFriend (0x290)
static int DWCi_SendResvCommandToFriend(BOOL delay, BOOL init, int resendPid)
{
    int buddyIdx;
    char value[4];
    GPBuddyStatus status;
    int profileID;
    int startIdx = init ? stpMatchCnt->friendCount
                        : (stpMatchCnt->friendCount < stpMatchCnt->friendIdxListLen - 1)
            ? stpMatchCnt->friendCount + 1
            : 0;
    int initFinished = 0;
    int valueLen;
    u8 locked;
    GPResult gpResult;
    OSTime tick;

    stpMatchCnt->qr2MatchType = 3;

    while (1)
    {
        if (!init || initFinished)
        {
            stpMatchCnt->friendCount++;
            if (stpMatchCnt->friendCount >= stpMatchCnt->friendIdxListLen)
            {
                stpMatchCnt->friendCount = 0;
            }
        }

        if (initFinished)
        {
            if (startIdx == stpMatchCnt->friendCount)
            {
                stpMatchCnt->cmdTimeoutTime = DWC_MATCH_CMD_RESEND_INTERVAL_MSEC;
                tick = OSGetTime();
                stpMatchCnt->cmdTimeoutStartTick = tick;
                stpMatchCnt->cmdResendFlag = FALSE;
                return 0;
            }
        }
        initFinished = 1;

        profileID = DWC_GetGsProfileId(DWCi_GetUserData(),
            (DWCAccFriendData*)((u8*)stpMatchCnt->friendList
                + stpMatchCnt->friendIdxList[stpMatchCnt->friendCount]
                    * sizeof(DWCAccFriendData)));

        if ((profileID == 0) || (profileID == -1))
            continue;

        if (!DWCi_Acc_IsValidFriendData(
                (DWCAccFriendData*)((u8*)stpMatchCnt->friendList
                    + stpMatchCnt->friendIdxList[stpMatchCnt->friendCount]
                        * sizeof(DWCAccFriendData))))
            continue;

        if (DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID))
            continue;

        gpResult = gpGetBuddyIndex(stpMatchCnt->pGpObj, profileID, &buddyIdx);
        gpResult |= gpGetBuddyStatus(stpMatchCnt->pGpObj, buddyIdx, &status);

        *(u32*)value = 0;
        valueLen = DWC_GetCommonValueString(
            "LCK", value, status.statusString, '/');
        if (valueLen > 0)
        {
            locked = strtoul(value, NULL, 10);
        }
        else
        {
            locked = 0;
        }

        if (locked)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "this buddy is locked. search next buddy...\n");
            continue;
        }

        if (gpResult || status.status != DWC_STATUS_MATCH_FRIEND)
            continue;

        if (profileID == resendPid)
            delay = TRUE;

        memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
        stpMatchCnt->tempNewNodeInfo.aid = 0xFF;
        stpMatchCnt->tempNewNodeInfo.profileID = profileID;

        return DWCi_SendResvCommand(profileID, delay);
    }
}

// .text:0x8032D268 DWCi_RetryReserving (0x464)
static BOOL DWCi_RetryReserving(int resendPid)
{
    SBError sbError;
    int result;
    OSTime tick;

    stpMatchCnt->reqProfileID = 0;
    stpMatchCnt->resvWaitCount = 0;
    tick = OSGetTime();
    stpMatchCnt->cmdTimeoutStartTick = tick;
    memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));

    if (stpMatchCnt->connectGroupID != 0)
    {
        DWCi_SetMatchStatus(0xC); // CL_SEARCH_GROUPID_HOST

        sbError = DWCi_SBUpdateAsync(0);
        if (DWCi_HandleSBError(sbError))
        {
            return FALSE;
        }
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
    {
        DWCi_SetMatchStatus(2); // CL_SEARCH_HOST

        sbError = DWCi_SBUpdateAsync(0);
        if (DWCi_HandleSBError(sbError))
        {
            return FALSE;
        }
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
    {
        result = DWCi_SendResvCommandToFriend(TRUE, FALSE, resendPid);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_OTHER);
        return FALSE;
    }

    return TRUE;
}

// .text:0x8032D6CC DWCi_CancelPreConnectedServerProcessWithResetLevel (0x350)
static BOOL DWCi_CancelPreConnectedServerProcessWithResetLevel(
    int clientPid, DWCMatchResetLevel level)
{
    int i;
    GT2Connection* connection;

    if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0) {
        memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
    }

    DWCi_GetMatchCnt()->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;

    if (DWCi_GetMatchCnt()->nnInfo.cookie) {
        NNCancel(DWCi_GetMatchCnt()->nnInfo.cookie);
        DWCi_GetMatchCnt()->nnInfo.cookie = 0;
    }

    DWCi_GetMatchCnt()->reqProfileID = 0;

    if ((s32)DWCi_GetMatchCnt()->hostState == 2) {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "DWCi_CancelPreConnectedServerProcess : server\n");

        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;

        DWCi_SetMatchStatus(0x11); // SV_CANCEL_SYN
        DWCi_GetMatchCnt()->cancelSynAckBit = 0;
        DWCi_GetMatchCnt()->cancelBaseLatency = 0;
        DWCi_GetMatchCnt()->cbEventPid = clientPid;

        connection = DWCi_GetGT2ConnectionByProfileID(clientPid, 0);
        if (connection)
        {
            DWCi_GetMatchCnt()->closeState =
                DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
            gt2CloseConnectionHard(*connection);
            DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
            DWCi_NodeInfoList_RemoveProfileID(clientPid);
        }
        else
        {
            DWCi_NodeInfoList_RemoveProfileID(clientPid);
        }

        for (i = 1; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++) {
            if (!DWCi_SendCancelMatchSynCommand(
                    DWCi_NodeInfoList_GetNodeInfoForIndex(i)->profileID,
                    DWC_MATCH_COMMAND_CANCEL_SYN)) {
                return FALSE;
            }
        }

        if (DWCi_GetMatchCnt()->nodeInfoList.count == 1) {
            DWCi_RestartFromCancel(level);
        }
    } else {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "DWCi_CancelPreConnectedServerProcess : client\n");
        DWCi_StopMeshMaking();
    }

    return TRUE;
}

// .text:0x8032DA1C DWCi_CancelPreConnectedClientProcess (0x480)
static BOOL DWCi_CancelPreConnectedClientProcess(
    int serverPid, DWCError error, int errorCode)
{
#pragma unused(serverPid)
    BOOL result = TRUE;

    if (stpMatchCnt->connectGroupID != 0 || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        if (stpMatchCnt->nodeInfoList.count > 1)
        {
            stpMatchCnt->closeState = DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
            stpMatchCnt->closeState = DWC_MATCH_CLOSE_STATE_INIT;
        }

        DWCi_StopMatching(error, errorCode);
        return FALSE;
    }

    if (DWCi_StopMeshMaking() == 1)
    {
        return TRUE;
    }

    if (stpMatchCnt->tempNewClientInfo.profileID != 0)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Clear temp new client info.\n");
        stpMatchCnt->tempNewClientInfo.profileID = 0;
    }

    if (stpMatchCnt->nnInfo.cookie)
    {
        NNCancel(stpMatchCnt->nnInfo.cookie);
        stpMatchCnt->nnInfo.cookie = 0;
    }

    if (((s32)stpMatchCnt->topologyType == 2 && stpMatchCnt->needServerPoll != 0) ||
        (stpMatchCnt->tempNewNodeInfo.profileID != 0 &&
         DWCi_GetGT2ConnectionByProfileID(
             stpMatchCnt->tempNewNodeInfo.profileID, 1)) ||
        stpMatchCnt->nodeInfoList.count > 1) {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "Close all connection and restart matching.\n");

        if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
        {
            DWC_Printf(DWC_REPORTFLAG_WARNING,
                "DWCi_RestartFromTimeout() shouldn't be called.\n");
        }
        else
        {
            stpMatchCnt->closeState = DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT;
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
            stpMatchCnt->closeState = DWC_MATCH_CLOSE_STATE_INIT;
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Closed all connections and restart matching.\n");
            DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
        }
    } else {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "Cancel and restart client process.\n");

        DWCi_SetMatchStatus(3); // CL_WAIT_RESV

        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Cancel and retry to reserve.\n");

        result = DWCi_RetryReserving(0);
    }

    return result;
}

// helper: node list insert/append (inlined in CF)
static inline void DWCi_NodeInfoList_Append(const DWCNodeInfo* node)
{
    if (stpMatchCnt->nodeInfoList.count != DWC_MAX_CONNECTIONS)
    {
        stpMatchCnt->nodeInfoList.nodes[stpMatchCnt->nodeInfoList.count] = *node;
        stpMatchCnt->nodeInfoList.count++;
    }
}

static inline void DWCi_NodeInfoList_InsertFront(const DWCNodeInfo* node)
{
    int i;

    if (stpMatchCnt->nodeInfoList.count != DWC_MAX_CONNECTIONS)
    {
        for (i = stpMatchCnt->nodeInfoList.count; i > 0; i--)
        {
            stpMatchCnt->nodeInfoList.nodes[i]
                = stpMatchCnt->nodeInfoList.nodes[i - 1];
        }
        stpMatchCnt->nodeInfoList.nodes[0] = *node;
        stpMatchCnt->nodeInfoList.count++;
    }
}

// .text:0x8032DE9C DWCi_PostProcessConnection (0x18B8)
static void DWCi_PostProcessConnection(DWCMatchPpConnectionType type)
{
    u32 senddata[1];
    int result;
    GPResult gpResult;
    OSTime tick;

    switch (type)
    {
    case 0:
        if ((s32)DWCi_GetMatchCnt()->topologyType == 2) {
            DWCi_PostProcessSVConnectionFullmesh();
        } else {
            DWCi_PostProcessSVConnectionHybrid();
        }
        break;

    case 1:
        DWCi_SetMatchStatus(1); // CL_WAITING

        if (DWCi_GetMatchCnt()->qr2MatchType == 3) {
            DWCi_GetMatchCnt()->cbEventPid =
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID;
        }
        break;

    case 2:
        DWCi_SetMatchStatus(1); // CL_WAITING

        if (DWCi_StopMeshMaking() == 1)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh create completed.\n");
            DWCi_GetMatchCnt()->lastStateBeforeStopMeshMaking = 0;
            memset(&DWCi_GetMatchCnt()->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCNodeInfo));
        }
        else
        {
            DWCi_GetMatchCnt()->hostState = 1;
            DWCi_GetMatchCnt()->qr2MatchType = 3;

            if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY ||
                DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND) {
                DWCi_GetMatchCnt()->qr2IsReserved = 1;
                DWCi_GetMatchCnt()->qr2Reservation =
                    DWCi_GetMatchCnt()->profileID;
            }

            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            tick = DWCi_Np_GetTick();
            DWCi_GetMatchCnt()->clWaitTime = tick;

            if ((s32)DWCi_GetMatchCnt()->topologyType == 2) {
                if (DWCi_GetMatchCnt()->tempNewClientInfo.profileID != 0) {
                    senddata[0] = DWCi_HtoLEl(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID);

                    DWCi_NodeInfoList_InsertFront(
                        &DWCi_GetMatchCnt()->tempNewNodeInfo);

                    DWCi_GetMatchCnt()->tempNewNodeInfo =
                        DWCi_GetMatchCnt()->tempNewClientInfo;
                    result = DWCi_SendMatchCommand(
                        DWC_MATCH_COMMAND_LINK_CLS_SUC,
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID,
                        DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress,
                        (&DWCi_GetMatchCnt()->tempNewNodeInfo)->publicQueryPort,
                        senddata, 1);
                    memset(&DWCi_GetMatchCnt()->tempNewClientInfo, 0, sizeof(DWCNodeInfo));
                    if (DWCi_HandleMatchCommandError(result))
                    {
                        return;
                    }
                }
                DWCi_GetMatchCnt()->needServerPoll = 1;
            }
        }
        break;

    case DWC_PP_CONNECTION_SYN_FINISH:
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "Accepted connectionCompleted matching!\n");
        DWCi_GetMatchCnt()->connectedOnceBefore = 1;
        DWCi_GetMatchCnt()->nnFailureCount = 0;

        if ((s32)DWCi_GetMatchCnt()->topologyType == 2) {
            if (DWCi_GetMatchCnt()->qr2MatchType == 2) {
                if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0) {
                    DWCi_NodeInfoList_Append(
                        &DWCi_GetMatchCnt()->tempNewNodeInfo);
                    memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                           sizeof(DWCNodeInfo));
                }
            } else if (DWCi_GetMatchCnt()->nodeInfoList.count == 1) {
                DWCi_NodeInfoList_InsertFront(
                    &DWCi_GetMatchCnt()->tempNewNodeInfo);
                memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                       sizeof(DWCNodeInfo));
            } else if (DWCi_GetMatchCnt()->tempNewClientInfo.profileID != 0) {
                DWCi_NodeInfoList_Append(&DWCi_GetMatchCnt()->tempNewClientInfo);
                DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;
            }
            DWCi_GetMatchCnt()->needServerPoll = 0;
        } else if (DWCi_GetMatchCnt()->qr2MatchType == 2) {
            if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0) {
                DWCi_NodeInfoList_Append(&DWCi_GetMatchCnt()->tempNewNodeInfo);
                memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                       sizeof(DWCNodeInfo));
            }
        } else if (DWCi_GetMatchCnt()->nodeInfoList.count == 1) {
            DWCi_NodeInfoList_InsertFront(&DWCi_GetMatchCnt()->tempNewNodeInfo);
            memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                   sizeof(DWCNodeInfo));
        } else if (DWCi_GetMatchCnt()->tempNewClientInfo.profileID != 0) {
            DWCi_NodeInfoList_Append(&DWCi_GetMatchCnt()->tempNewClientInfo);
            DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;
        }

        DWCi_GetMatchCnt()->connectGroupID = 0;

        DWCi_GetMatchCnt()->matchedCallback(
            DWC_ERROR_NONE, FALSE,
            DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
            DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
            DWCi_GetMatchCnt()->matchedParam);

        DWCi_NNFreeNegotiateList();

        if (DWCi_GetMatchCnt()->qr2MatchType == 2) {
            gpResult = DWCi_GPSetServerStatusInline(-1);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }
            DWCi_SetMatchStatus(0xD); // SV_WAITING
        } else {
            (void)DWCi_GPSetServerStatusInline(2);
            DWCi_SetMatchStatus(1); // CL_WAITING
        }

        DWCi_GetMatchCnt()->cbEventPid = 0;
        DWCi_GetMatchCnt()->cancelState = DWC_MATCH_CANCEL_STATE_INIT;
        DWCi_ResetAllNodeRecvTimes();
        break;
    }

    if (DWCi_GetMatchCnt()->sbObj) {
        ServerBrowserClear(DWCi_GetMatchCnt()->sbObj);
        DWCi_GetMatchCnt()->sbUpdateRequestTick = 0;
    }
}

// .text:0x8032F754 DWCi_PostProcessSVConnectionHybrid (0x2D0)
BOOL DWCi_PostProcessSVConnectionHybrid(void)
{
    BOOL result = FALSE;
    int i;
    OSTime tick;

    if (stpMatchCnt->inMeshMaking != 0 || (s32)stpMatchCnt->lastStateBeforeStopMeshMaking == 0xF) {
        stpMatchCnt->qr2IsReserved = 0;
        stpMatchCnt->qr2Reservation = 0;

        DWCi_SetMatchStatus(1); // CL_WAITING
        DWCi_StopMeshMaking();
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh create completed.\n");
        return FALSE;
    }

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Tell new client completion of matching.\n");

    stpMatchCnt->qr2IsReserved = 0;
    stpMatchCnt->qr2Reservation = 0;

    if (stpMatchCnt->connectedOnceBefore == 0 && stpMatchCnt->nodeInfoList.count == 1)
    {
        tick = OSGetTime();
        stpMatchCnt->serverPollingTime = tick;
    }

    qr2_send_statechanged(stpMatchCnt->qr2Obj);

    if (stpMatchCnt->qr2MatchType == 2)
    {
        stpMatchCnt->cbEventPid = stpMatchCnt->tempNewNodeInfo.profileID;
    }

    DWCi_SetMatchStatus(0x13); // SV_SYN
    stpMatchCnt->synAckBit = 0;

    for (i = 1; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        DWCi_SendMatchSynPacket(DWCi_NodeInfoList_GetNodeInfoForIndex(i)->aid,
            DWC_SEND_TYPE_MATCH_SYN);
    }
    DWCi_SendMatchSynPacket(
        stpMatchCnt->tempNewNodeInfo.aid, DWC_SEND_TYPE_MATCH_SYN);

    if (stpMatchCnt->qr2MatchType != 2)
    {
        result = TRUE;
    }
    return result;
}

// .text:0x8032FA24 DWCi_PostProcessSVConnectionFullmesh (0x3E0)
BOOL DWCi_PostProcessSVConnectionFullmesh(void)
{
    u32 senddata[16];
    int sendlen;
    DWCNodeInfo newNode = stpMatchCnt->tempNewNodeInfo;
    u8 idx;
    int result;
    DWCNodeInfo* node;

    sendlen = 0;

    if (stpMatchCnt->clLinkProgress < stpMatchCnt->nodeInfoList.count - 1) {
        idx =
            stpMatchCnt->nodeInfoList.count - 1U - stpMatchCnt->clLinkProgress;
        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Send client-client link request(%d).\n", idx);

        DWCi_SetMatchStatus(0x10); // SV_WAIT_CL_LINK

        node = DWCi_NodeInfoList_GetNodeInfoForIndex(idx);

        senddata[sendlen++] = DWCi_HtoLEl(node->profileID);
        senddata[sendlen++] = DWCi_HtoLEl(idx);
        senddata[sendlen++] = DWCi_HtoLEl(node->aid);
        senddata[sendlen++] = node->publicInetAddress;
        senddata[sendlen++] = DWCi_HtoLEl((node)->publicQueryPort);
        senddata[sendlen++] = node->privateInetAddress;
        senddata[sendlen++] = DWCi_HtoLEl((node)->privateQueryPort);
        senddata[sendlen++] = DWCi_HtoLEl(node->hasPrivateAddress);
        senddata[sendlen++] = *(const u32*)node->connUserData;

    } else {
        stpMatchCnt->clLinkProgress = 0;
        return DWCi_PostProcessSVConnectionHybrid();
    }
    if (stpMatchCnt->state != 0x13) // SV_SYN
    {
        result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_LINK_CLS_REQ,
            newNode.profileID, newNode.publicInetAddress, (&newNode)->publicQueryPort,
            senddata, sendlen);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }

        node = DWCi_NodeInfoList_GetNodeInfoForIndex(idx);
        sendlen = 0;
        senddata[sendlen++] = DWCi_HtoLEl(newNode.profileID);
        senddata[sendlen++] = DWCi_HtoLEl(newNode.aid);
        result = DWCi_SendMatchCommand(
            DWC_MATCH_COMMAND_CLOSE_LINK, node->profileID, node->publicInetAddress,
            (node)->publicQueryPort, senddata, sendlen);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }

        stpMatchCnt->cmdCnt.count = 0;
    }

    return FALSE;
}

// helper: NodeInfoList lookups (inlined in CF; defined later in the file, scratch copy here)
// .text:0x80338BA0 DWCi_NodeInfoList_GetServerNodeInfo (0x20)

// .text:0x80338BC0 DWCi_NodeInfoList_GetNodeInfoForIndex (0x28)

// .text:0x80338BE8 DWCi_NodeInfoList_GetNodeInfoForProfileID (0x4C)

// .text:0x80338C34 DWCi_NodeInfoList_GetNodeInfoForAID (0x4C)


// helper: DWCi_HandleMatchCommandError (inlined in CF)
static inline int DWCi_HandleMatchCommandError(int error)
{
    if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
    {
        return (int)DWCi_HandleSBError((SBError)error);
    }
    else
    {
        return (int)DWCi_HandleGPError((GPResult)error);
    }
}

// helper: DWCi_FinishCancelMatching (inlined in CF)
static void DWCi_FinishCancelMatching(void)
{
    GPResult gpResult;

    gpResult = DWCi_GPSetServerStatus(1);
    if (DWCi_HandleGPError(gpResult))
    {
        return;
    }

    DWCi_CloseMatch();
    DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
        DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE,
        DWCi_GetMatchCnt()->cbEventPid
            ? TRUE
            : ((DWCi_GetMatchCnt()->qr2MatchType == 2) ? TRUE : FALSE),
        DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
        DWCi_GetMatchCnt()->matchedParam);

    DWCi_GetMatchCnt()->cancelState = 0;
}

// HACK: Artificial inline boundary to match register allocation in DWCi_DoCancelMatch.
// No corresponding helper was found in the Conduit 2 map (including UNUSED symbols).
// The helper name and boundary are unverified; the resulting machine code matches.
static inline int DWCi_CancelReservedMatch(void)
{
    int result;
    u32 data;
    data = 0;
    result = 0;
    if (stpMatchCnt->qr2IsReserved != 0
        && stpMatchCnt->qr2Reservation != 0
        && stpMatchCnt->qr2Reservation != stpMatchCnt->profileID)
    {
        result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_CANCEL,
            stpMatchCnt->qr2Reservation,
            stpMatchCnt->tempNewNodeInfo.publicInetAddress,
            stpMatchCnt->tempNewNodeInfo.publicQueryPort, &data, 1);
        stpMatchCnt->qr2IsReserved = 0;
        stpMatchCnt->qr2Reservation = 0;
        memset(&stpMatchCnt->tempNewNodeInfo, 0,
            sizeof(DWCNodeInfo));
    }
    return result;
}

// .text:0x8032FE04 DWCi_DoCancelMatch (0x7FC)
static void DWCi_DoCancelMatch(void)
{
    int result;
    int connIdx;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "CANCEL! state %d, numHost=%d.\n",
        stpMatchCnt->state, stpMatchCnt->nodeInfoList.count);
    stpMatchCnt->cbEventPid = 0;

    if (stpMatchCnt->state == 0x16) // SEARCH_OWN
    {
        DWCi_FinishCancelMatching();
    }
    else
    {
        connIdx = DWCi_GT2GetConnectionListIdx() != 0;
        stpMatchCnt->cancelState = 1;
        if (stpMatchCnt->qr2MatchType == 3)
        {
            stpMatchCnt->cancelState = 1;
            if (connIdx)
            {
                gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
            }
            else
            {
                if (stpMatchCnt->tempNewNodeInfo.profileID == 0)
                {
                    result = 0;
                }
                else
                {
                    result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_RESV_CANCEL,
                        stpMatchCnt->tempNewNodeInfo.profileID,
                        stpMatchCnt->tempNewNodeInfo.publicInetAddress,
                        stpMatchCnt->tempNewNodeInfo.publicQueryPort, NULL, 0);
                    stpMatchCnt->reqProfileID = 0;
                }
                if (DWCi_HandleMatchCommandError(result))
                {
                    return;
                }
            }
            DWCi_FinishCancelMatching();
        }
        else
        {
            if (stpMatchCnt->state == 3 || stpMatchCnt->state == 4 ||
                stpMatchCnt->state == 5 || stpMatchCnt->state == 6 ||
                (u32)(stpMatchCnt->state - 14) <= 1) {
                if (stpMatchCnt->reqProfileID != 0)
                {
                    if (stpMatchCnt->tempNewNodeInfo.profileID == 0)
                    {
                        result = 0;
                    }
                    else
                    {
                        result = DWCi_SendMatchCommand(
                            DWC_MATCH_COMMAND_RESV_CANCEL,
                            stpMatchCnt->tempNewNodeInfo.profileID,
                            stpMatchCnt->tempNewNodeInfo.publicInetAddress,
                            stpMatchCnt->tempNewNodeInfo.publicQueryPort, NULL, 0);
                        stpMatchCnt->reqProfileID = 0;
                    }
                    if (DWCi_HandleMatchCommandError(result))
                    {
                        return;
                    }
                }

                result = DWCi_CancelReservedMatch();
                if (DWCi_HandleMatchCommandError(result))
                {
                    return;
                }
            }

            if (stpMatchCnt->nnInfo.cookie != 0)
            {
                NNCancel(stpMatchCnt->nnInfo.cookie);
                stpMatchCnt->nnInfo.cookie = 0;
            }
            if (connIdx)
            {
                stpMatchCnt->cancelState = 1;
                gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
            }
            DWCi_FinishCancelMatching();
        }
    }
}

// .text:0x80330600 DWCi_RestartFromCancel (0x4FC)
static void DWCi_RestartFromCancel(DWCMatchResetLevel level)
{
    SBError sbError;

    if (level == DWC_MATCH_RESET_ALL)
    {
        DWCi_FinishCancelMatching();
    }
    else
    {
        DWCi_ResetMatchParam(level);

        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 2
            || DWCi_GetMatchCnt()->qr2MatchTypeExt == 3)
        {
            DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
                DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
                DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
                DWCi_GetMatchCnt()->matchedParam);
        }
        else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0)
        {
            if (DWCi_GetMatchCnt()->connectedOnceBefore != 0)
            {
                DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
                    DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
                    DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
                    DWCi_GetMatchCnt()->matchedParam);
            }
            if (level == DWC_MATCH_RESET_RESTART)
            {
                DWCi_GetMatchCnt()->hostState = 0;
                DWCi_SetMatchStatus(22); // DWC_MATCH_STATE_SEARCH_OWN
                sbError = DWCi_SBUpdateAsync(DWCi_GetMatchCnt()->profileID);
                if (DWCi_HandleSBError(sbError))
                {
                    return;
                }
            }
        }
        else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 1)
        {
            if (DWCi_GetMatchCnt()->connectedOnceBefore != 0)
            {
                DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
                    DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
                    DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
                    DWCi_GetMatchCnt()->matchedParam);
            }
            if (level == DWC_MATCH_RESET_RESTART)
            {
                DWCi_GetMatchCnt()->hostState = 0;
                DWCi_GetMatchCnt()->connectGroupID = 0;
                DWCi_SetMatchStatus(22); // DWC_MATCH_STATE_SEARCH_OWN
                sbError = DWCi_SBUpdateAsync(DWCi_GetMatchCnt()->profileID);
                if (DWCi_HandleSBError(sbError))
                {
                    return;
                }
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_ERROR,
                "ERROR - DWCi_RestartFromCancel : matchType %d, level %d\n",
                DWCi_GetMatchCnt()->qr2MatchType, level);
        }
    }
}

// helper: DWCi_GetNodeInfoForProfileIDOrNew (inlined in CF)
static inline DWCNodeInfo* DWCi_GetNodeInfoForProfileIDOrNew(int profileID)
{
    DWCNodeInfo* node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);

    if (stpMatchCnt->tempNewNodeInfo.profileID != 0 && node == NULL
        && profileID == stpMatchCnt->tempNewNodeInfo.profileID)
    {
        node = &stpMatchCnt->tempNewNodeInfo;
    }
    return node;
}

// .text:0x80330AFC DWCi_CloseShutdownClientSC (0x2F0)
static BOOL DWCi_CloseShutdownClientSC(u32 bitmap)
{
    int senddata[DWC_MAX_CONNECTIONS];
    int validPid[DWC_MAX_CONNECTIONS];
    int i;
    int senddataLen = 0;
    int validPidLen = 0;
    BOOL closeTemp = FALSE;
    BOOL ret = TRUE;
    DWCNodeInfo* node;
    u8 aid;
    int profileID;

    for (i = 1; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
        if (node != NULL)
        {
            if (bitmap & (1 << node->aid))
            {
                validPid[validPidLen] = node->profileID;
                validPidLen++;
            }
            else
            {
                senddata[senddataLen] = DWCi_HtoLEl(node->profileID);
                senddataLen++;
            }
        }
    }

    if (stpMatchCnt->tempNewNodeInfo.profileID != 0)
    {
        if (bitmap & (1 << stpMatchCnt->tempNewNodeInfo.aid))
        {
            validPid[validPidLen++] = stpMatchCnt->tempNewNodeInfo.profileID;
        }
        else
        {
            senddata[senddataLen++]
                = DWCi_HtoLEl(stpMatchCnt->tempNewNodeInfo.profileID);
            closeTemp = TRUE;
        }
    }

    for (i = 0; i < validPidLen; i++)
    {
        profileID = validPid[i];
        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);
        if (stpMatchCnt->tempNewNodeInfo.profileID != 0 && node == NULL
            && profileID == stpMatchCnt->tempNewNodeInfo.profileID)
        {
            node = &stpMatchCnt->tempNewNodeInfo;
        }

        if (node != NULL)
        {
            int result = DWCi_SendMatchCommand(
                DWC_MATCH_COMMAND_SC_CLOSE_CL, validPid[i], node->publicInetAddress,
                node->publicQueryPort, (u32 *)senddata, senddataLen);
            if (DWCi_HandleMatchCommandError(result))
            {
                ret = FALSE;
            }
        }
    }

    stpMatchCnt->closeState = 1;

    if (closeTemp)
    {
        DWC_CloseConnectionHard(stpMatchCnt->tempNewNodeInfo.aid);
        memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
    }

    for (i = 0; i < senddataLen; i++)
    {
        node = DWCi_NodeInfoList_GetNodeInfoForProfileID(
            DWCi_LEtoHl(senddata[i]));
        if (node == NULL)
        {
            aid = 0xff;
        }
        else
        {
            aid = node->aid;
        }
        if (aid != 0xff)
        {
            DWC_CloseConnectionHard(aid);
        }
    }

    stpMatchCnt->closeState = 0;

    return ret;
}

// helper: DWCi_SynEmptyPayload (inlined in CF)
static inline void DWCi_SynEmptyPayload(u8* buf)
{
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    *(u32*)&buf[4] = DWCi_HtoLEl(stpMatchCnt->serverPollingUID);
}

// .text:0x80330DEC DWCi_SendMatchSynPacket (0x3D0)
static void DWCi_SendMatchSynPacket(u8 aid, u16 type)
{
    u8 stackBuf[8];
    int len = 4;
    u8* buf = stackBuf;
    int count;
    u8 i;
    int idx;
    DWCNodeInfo* node;
    u32* data;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Sent SYN %d packet to aid %d.\n",
        type - DWC_SEND_TYPE_MATCH_SYN, aid);
    switch (type)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
        if (stpMatchCnt->tempNewNodeInfo.profileID != 0
            && aid == stpMatchCnt->tempNewNodeInfo.aid)
        {
            *(u32*)buf = DWCi_HtoLEl(1);
        }
        else
        {
            len = 0x1c;
            data = DWC_Alloc(DWC_ALLOCTYPE_BASE, len);
            if (data == NULL)
            {
                return;
            }
            buf = (u8*)data;
            data[0] = 0;
            node = &stpMatchCnt->tempNewNodeInfo;
            data[1] = DWCi_HtoLEl(
                (u32)node->aid | ((u32)node->hasPrivateAddress << 16));
            data[2] = DWCi_HtoLEl(node->profileID);
            data[3] = node->publicInetAddress;
            data[4] = node->privateInetAddress;
            data[5] = DWCi_HtoLEl(((u32) node->publicQueryPort << 16)
                | node->privateQueryPort);
            data[6] = *(const u32*)node->connUserData;
        }
        break;

    case DWC_SEND_TYPE_MATCH_SYN_ACK:
        stackBuf[0] = stpMatchCnt->baseLatency;
        stackBuf[1] = (stpMatchCnt->baseLatency >> 8) & 0xFF;
        break;

    case 4:
        if (stpMatchCnt->tempNewNodeInfo.profileID != 0
            && aid == stpMatchCnt->tempNewNodeInfo.aid)
        {
            if ((int)stpMatchCnt->topologyType == 2)
            {
                count = 0;
            }
            else
            {
                count = DWCi_NodeInfoList_GetCount() - 1;
            }

            if (count == 0)
            {
                len = 8;
                DWCi_SynEmptyPayload(buf);
            }
            else
            {
                len = count * 0x18 + 8;
                data = DWC_Alloc(DWC_ALLOCTYPE_BASE, len);
                if (data == NULL)
                {
                    return;
                }
                data[0] = DWCi_HtoLEl(count);
                idx = 1;
                for (i = 0; i < count; i++)
                {
                    node = DWCi_NodeInfoList_GetNodeInfoForIndex(i + 1);
                    data[idx++] = DWCi_HtoLEl(
                        (u32)node->aid | ((u32)node->hasPrivateAddress << 16));
                    data[idx++] = DWCi_HtoLEl(node->profileID);
                    data[idx++] = node->publicInetAddress;
                    data[idx++] = node->privateInetAddress;
                    data[idx++] = DWCi_HtoLEl(((u32) node->publicQueryPort << 16)
                        | node->privateQueryPort);
                    data[idx++] = *(const u32*)node->connUserData;
                }
                data[idx] = DWCi_HtoLEl(stpMatchCnt->serverPollingUID);
                buf = (u8*)data;
            }
        }
        else
        {
            len = 8;
            DWCi_SynEmptyPayload(buf);
        }
        break;
    }
    DWCi_SendReliable(type, aid, buf, len);
    if (buf != stackBuf)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, buf, 0);
    }
    stpMatchCnt->lastSynSent = OSGetTime();
}

// .text:0x803311BC DWCi_ProcessMatchSynTimeout (0x55C)
static BOOL DWCi_ProcessMatchSynTimeout(void)
{
    u64 passTime;
    int i;
    DWCNodeInfo* node;
    DWCMatchControl *cnt = stpMatchCnt;

    // 8 = CL_SYN, 0x13 = SV_SYN, 0x14 = SV_SYN_WAIT, 0x17 = SV_SYN_CLOSE_WAIT
    if (cnt->state == 8 || cnt->state == 0x13 || cnt->state == 0x17
        || cnt->state == 0x14)
    {
        passTime = DWCi_Np_TicksToMilliSeconds(
            DWCi_Np_GetTick() - cnt->lastSynSent);
    }
    else
    {
        return TRUE;
    }

    switch (DWCi_GetMatchCnt()->state) {
    case 8:
        if (passTime > DWCi_GetMatchCnt()->clWaitTime) {
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                "Timeout: [SYN] Connection to server was shut down.\n");
            DWCi_GetMatchCnt()->tempNewClientInfo.profileID = 0;
            if (!DWCi_CancelPreConnectedClientProcess(
                    DWCi_NodeInfoList_GetServerNodeInfo()->profileID, 0x18,
                    -0x13A2E))
            {
                return FALSE;
            }
        }
        break;

    case 0x13:
        if (passTime > 6000)
        {
            DWCi_GetMatchCnt()->synResendCount++;
            if (DWCi_GetMatchCnt()->synResendCount > 5) {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                           "Timeout: wait SYN-ACK (aidbitmap 0x%x). "
                           "Restart matching.\n",
                           DWCi_GetMatchCnt()->synAckBit);
                DWCi_GetMatchCnt()->synResendCount = 0;
                DWCi_GetMatchCnt()->lastSynSent = DWCi_Np_GetTick();
                if (DWCi_CloseShutdownClientSC(DWCi_GetMatchCnt()->synAckBit)) {
                    if (DWCi_GetMatchCnt()->nodeInfoList.count > 1 ||
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID != 0) {
                        DWC_Printf(DWC_REPORTFLAG_DEBUG,
                            "DWCi_ServerSynTimeoutExec: Continue Syn "
                            "exec.\n");
                        DWCi_GetMatchCnt()->lastSynSent = DWCi_Np_GetTick();
                        DWCi_SetMatchStatus(0x17);
                    } else {
                        DWC_Printf(DWC_REPORTFLAG_DEBUG,
                            "DWCi_ServerSynTimeoutExec: Restart match.\n");
                        DWCi_CancelPreConnectedServerProcess(
                            DWCi_GetMatchCnt()->cbEventPid);
                    }
                }
            } else {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                           "** Timeout: resent SYN-ACK...(%d/%d)\n",
                           DWCi_GetMatchCnt()->synResendCount, 5);
                {
                    for (i = 1;
                         i < (cnt = DWCi_GetMatchCnt())->nodeInfoList.count;
                         i++) {
                        node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
                        if (!((1 << node->aid) & cnt->synAckBit)) {
                            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                                       "resent SYN-ACK to aid(%d).\n",
                                       node->aid);
                            DWCi_SendMatchSynPacket(node->aid,
                                                    DWC_SEND_TYPE_MATCH_SYN);
                        }
                    }
                    if (!((1 << cnt->tempNewNodeInfo.aid) & cnt->synAckBit)) {
                        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                                   "resent SYN-ACK to aid(%d)(new client).\n",
                                   cnt->tempNewNodeInfo.aid);
                        DWCi_SendMatchSynPacket(cnt->tempNewNodeInfo.aid,
                                                DWC_SEND_TYPE_MATCH_SYN);
                    }
                }
            }
        }
        break;

    case 0x17:
        if (passTime > 3000)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "DWC_MATCH_STATE_SV_SYN_CLOSE_WAIT: send ack command.\n");
            DWCi_SendTypeMatchAck();
        }
        break;

    case 0x14:
        if (passTime > DWCi_GetMatchCnt()->baseLatency) {
            DWCi_PostProcessConnection(3);
        }
        break;
    }

    return TRUE;
}

// helper: DWCi_SendCancelMatchSynCommand (inlined in CF)
static inline BOOL DWCi_SendCancelMatchSynCommand(int profileID, u8 command)
{
    u32 senddata;
    int len;
    int result;
    DWCNodeInfo* node;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Sent CANCEL SYN %d command to %u.\n",
        command - DWC_MATCH_COMMAND_CANCEL_SYN, profileID);

    if (command == DWC_MATCH_COMMAND_CANCEL_SYN)
    {
        senddata = DWCi_HtoLEl((u32)stpMatchCnt->cbEventPid);
        len = 1;
    }
    else
    {
        len = 0;
    }

    node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);
    if (node == NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "DWCi_NodeInfoList_GetNodeInfoForProfileID is NULL\n");
        return FALSE;
    }

    result = DWCi_SendMatchCommand(command, profileID, node->publicInetAddress,
        node->publicQueryPort, &senddata, len);
    if (DWCi_HandleMatchCommandError(result))
    {
        return FALSE;
    }

    stpMatchCnt->lastCancelSynSent = DWCi_Np_GetTick();

    return TRUE;
}

// .text:0x80331718 DWCi_ProcessCancelMatchSynCommand (0x70C)
static BOOL DWCi_ProcessCancelMatchSynCommand(
    int profileID, u8 command, u32 data)
{
    u8 aid;
    u32 bitmask;
    u64 elapsed;
    u64 latency;
    int i;
    DWCNodeInfo* node;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Received CANCEL SYN %d command from %u.\n",
        command - DWC_MATCH_COMMAND_CANCEL_SYN, profileID);

    if (DWC_GetState() != DWC_STATE_CONNECTED)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "Ignore delayed CANCEL SYN.\n");
        return TRUE;
    }

    switch (command)
    {
    case DWC_MATCH_COMMAND_CANCEL_SYN:
        if (stpMatchCnt->state != 7)
        {
            DWCi_SetMatchStatus(7); // CL_CANCEL_SYN
            stpMatchCnt->cbEventPid = (int)data;
            if (stpMatchCnt->tempNewClientInfo.profileID != 0)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Clear temp new client info.\n");
                stpMatchCnt->tempNewClientInfo.profileID = 0;
            }
            memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
        }

        if (!DWCi_SendCancelMatchSynCommand(
                profileID, DWC_MATCH_COMMAND_CANCEL_SYN_ACK))
        {
            return FALSE;
        }
        break;

    case DWC_MATCH_COMMAND_CANCEL_SYN_ACK:
        if (stpMatchCnt->state == 0x11) // SV_CANCEL_SYN
        {
            elapsed = DWCi_Np_TicksToMilliSeconds(
                          (u64)(DWCi_Np_GetTick() - stpMatchCnt->lastCancelSynSent))
                >> 1;
            if (elapsed > 300)
            {
                latency = elapsed - 300;
                if (latency > stpMatchCnt->cancelBaseLatency)
                {
                    stpMatchCnt->cancelBaseLatency = (u16)latency;
                }
            }

            node = DWCi_NodeInfoList_GetNodeInfoForProfileID(profileID);
            if (node == NULL)
            {
                aid = 0xff;
            }
            else
            {
                aid = node->aid;
            }
            if (aid != 0xff)
            {
                stpMatchCnt->cancelSynAckBit |= 1 << aid;
            }

            bitmask = DWCi_NodeInfoList_GetClientAIDBitmap(FALSE);

            if (bitmask == stpMatchCnt->cancelSynAckBit)
            {
                for (i = 1; i < stpMatchCnt->nodeInfoList.count; i++)
                {
                    node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
                    if (!DWCi_SendCancelMatchSynCommand(node->profileID,
                            DWC_MATCH_COMMAND_CANCEL_ACK))
                    {
                        return FALSE;
                    }
                }

                DWCi_SetMatchStatus(0x12); // SV_CANCEL_SYN_WAIT

                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "Wait max latency %d msec.\n",
                    stpMatchCnt->cancelBaseLatency);
            }
        }
        else
        {
            if (!DWCi_SendCancelMatchSynCommand(
                    profileID, DWC_MATCH_COMMAND_CANCEL_ACK))
            {
                return FALSE;
            }
        }
        break;

    case DWC_MATCH_COMMAND_CANCEL_ACK:
        if (stpMatchCnt->state == 7) // CL_CANCEL_SYN
        {
            DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
        }
        break;
    }

    return TRUE;
}

// .text:0x80331E24 DWCi_ProcessCancelMatchSynTimeout (0x408)
static BOOL DWCi_ProcessCancelMatchSynTimeout(void)
{
    u64 passTime;
    int i;
    DWCNodeInfo* node;
    DWCMatchControl* cnt = stpMatchCnt;

    // 7 = CL_CANCEL_SYN, 0x11 = SV_CANCEL_SYN, 0x12 = SV_CANCEL_SYN_WAIT
    if (cnt->state == 7 || cnt->state == 0x11 || cnt->state == 0x12)
    {
        passTime = DWCi_Np_TicksToMilliSeconds(
            DWCi_Np_GetTick() - cnt->lastCancelSynSent);
    }
    else
    {
        return TRUE;
    }

    switch (stpMatchCnt->state)
    {
    case 7:
        if (passTime > 6000)
        {
            if (!DWCi_SendCancelMatchSynCommand(
                    DWCi_NodeInfoList_GetNodeInfoForIndex(0)->profileID,
                    DWC_MATCH_COMMAND_CANCEL_SYN_ACK)) {
                return FALSE;
            }
        }
        break;

    case 0x11:
        if (passTime > 6000)
        {
            stpMatchCnt->cancelSynResendCount++;
            if (stpMatchCnt->cancelSynResendCount > 5)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: wait cancel SYN-ACK (aidbitmap 0x%x).\n",
                    stpMatchCnt->cancelSynAckBit);

                if (!DWCi_CloseShutdownClientSC(stpMatchCnt->cancelSynAckBit))
                {
                    return FALSE;
                }

                if (stpMatchCnt->nodeInfoList.count > 1)
                {
                    stpMatchCnt->cancelSynResendCount = 0;
                    stpMatchCnt->lastCancelSynSent = DWCi_Np_GetTick();
                }
                else
                {
                    DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
                }
            }
            else
            {
                for (i = 1; i < stpMatchCnt->nodeInfoList.count; i++)
                {
                    node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
                    if (!((1 << node->aid) & stpMatchCnt->cancelSynAckBit))
                    {
                        if (!DWCi_SendCancelMatchSynCommand(node->profileID,
                                DWC_MATCH_COMMAND_CANCEL_SYN))
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }
        break;

    case 0x12:
        if (passTime > stpMatchCnt->cancelBaseLatency)
        {
            DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
        }
        break;
    }

    return TRUE;
}

// .text:0x8033222C DWCi_HandleGPError (0x294)
static GPResult DWCi_HandleGPError(GPResult result)
{
    int errorCode;
    DWCError dwcError;

    if (result == GP_NO_ERROR)
    {
        return GP_NO_ERROR;
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Match, GP error %d\n", result);

    switch (result)
    {
    case GP_MEMORY_ERROR:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_ALLOC;
        break;
    case GP_PARAMETER_ERROR:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_PARAM;
        break;
    case GP_NETWORK_ERROR:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_NETWORK;
        break;
    case GP_SERVER_ERROR:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SERVER;
        break;
    }

    errorCode += DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GP;

    DWCi_StopMatching(dwcError, errorCode);

    return result;
}

// .text:0x803324C0 DWCi_HandleSBError (0x2B8)
static SBError DWCi_HandleSBError(SBError error)
{
    int errorCode;
    DWCError dwcError;

    if (error == sbe_noerror)
    {
        return sbe_noerror;
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Match, SB error %d\n", error);

    switch (error)
    {
    case sbe_socketerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SOCKET;
        break;
    case sbe_dnserror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_DNS;
        break;
    case sbe_connecterror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SERVER;
        break;
    case sbe_dataerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_DATA;
        break;
    case sbe_allocerror:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_ALLOC;
        break;
    case sbe_paramerror:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_PARAM;
        break;
    }

    errorCode += DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_SB;

    DWCi_StopMatching(dwcError, errorCode);

    return error;
}

// .text:0x80332778 DWCi_HandleQR2Error (0x308)
static qr2_error_t DWCi_HandleQR2Error(qr2_error_t error)
{
    int errorCode;
    DWCError dwcError;

    if (error == e_qrnoerror)
    {
        return e_qrnoerror;
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Match, QR2 error %d\n", error);

    switch (error)
    {
    case e_qrwsockerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SOCKET;
        break;
    case e_qrbinderror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_BIND;
        break;
    case e_qrdnserror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_DNS;
        break;
    case e_qrconnerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_PEER;
        break;
    case e_qrnochallengeerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SERVER;
        break;
    }

    switch (DWC_GetState())
    {
    case DWC_STATE_LOGIN:
        errorCode += DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_QR2;
        DWCi_StopLogin(dwcError, errorCode);
        break;
    case DWC_STATE_UPDATE_SERVERS:
        errorCode += DWC_ECODE_SEQ_FRIEND + DWC_ECODE_GS_QR2;
        DWCi_StopFriendProcess(dwcError, errorCode);
        break;
    case DWC_STATE_MATCHING:
        errorCode += DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_QR2;
        DWCi_StopMatching(dwcError, errorCode);
        break;
    default:
        errorCode += DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_QR2;
        DWCi_SetError(dwcError, errorCode);
        break;
    }

    return error;
}

// .text:0x80332A80 DWCi_HandleNNError (0x284)
static NegotiateError DWCi_HandleNNError(NegotiateError error)
{
    int errorCode;
    DWCError dwcError;

    if (error == ne_noerror)
    {
        return ne_noerror;
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Match, NN error %d\n", error);

    switch (error)
    {
    case ne_allocerror:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_ALLOC;
        break;
    case ne_socketerror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SOCKET;
        break;
    case ne_dnserror:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_DNS;
        break;
    }

    errorCode += DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_NN;

    DWCi_StopMatching(dwcError, errorCode);

    return error;
}

// .text:0x80332D04 DWCi_HandleGT2Error (0x2AC)
static GT2Result DWCi_HandleGT2Error(GT2Result error)
{
    int type;
    int code;

    if (error == 0)
    {
        return 0;
    }
    DWC_Printf(2, "Match, GT2 error %d\n", error);
    switch (error)
    {
    case GT2OutOfMemory:
        type = DWC_ERROR_FATAL;
        code = DWC_ECODE_TYPE_ALLOC;
        break;
    case GT2Rejected:
    case GT2DuplicateAddress:
        type = 0;
        code = 0;
        error = 0;
        break;
    case GT2NetworkError:
        type = DWC_ERROR_NETWORK;
        code = DWC_ECODE_TYPE_NETWORK;
        break;
    case GT2AddressError:
        type = DWC_ERROR_NETWORK;
        code = -30;
        break;
    case GT2TimedOut:
        type = DWC_ERROR_NETWORK;
        code = -70;
        break;
    case GT2NegotiationError:
        type = DWC_ERROR_NETWORK;
        code = -80;
        break;
    }
    if (type != 0)
    {
        DWCi_StopMatching(type,
            code + DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GT2);
    }
    return error;
}

// helper: DWCi_CheckDWCServer (inlined in CF; the older source has it as a function)
static BOOL DWCi_CheckDWCServer(SBServer server)
{
    if (SBServerGetIntValueA(server, "numplayers", -1) == -1)
    {
        return FALSE;
    }
    if (SBServerGetIntValueA(server, "maxplayers", -1) == -1)
    {
        return FALSE;
    }
    if (SBServerGetIntValueA(server, "dwc_mtype", -1) == -1)
    {
        return FALSE;
    }
    if (SBServerGetIntValueA(server, "dwc_mver", -1) == -1)
    {
        return FALSE;
    }
    return SBServerGetIntValueA(server, "dwc_pid", 0);
}

// helper: DWCi_NodeInfoList_AppendNodeInfo (inlined in CF)
static inline void DWCi_NodeInfoList_AppendNodeInfo(DWCNodeInfo* info)
{
    int i;

    if (stpMatchCnt->nodeInfoList.count != DWC_MAX_CONNECTIONS)
    {
        for (i = stpMatchCnt->nodeInfoList.count; i > 0; i--)
        {
            stpMatchCnt->nodeInfoList.nodes[i]
                = stpMatchCnt->nodeInfoList.nodes[i - 1];
        }
        stpMatchCnt->nodeInfoList.nodes[0] = *info;
        stpMatchCnt->nodeInfoList.count++;
    }
}

static inline int DWCi_GetMatchTopology(void) {
    return DWCi_GetMatchCnt()->topologyType;
}

// .text:0x80332FB0 DWCi_SBCallback (0x1C14)
static void DWCi_SBCallback(ServerBrowser sb, SBCallbackReason reason,
    SBServer server, void* instance)
{
    int count;
    int i;
    int profileID;
    int result;
    DWCNodeInfo info;
    SBServer host;
    NegotiateError nnError;

    DWC_Printf(0x40, "SBCallback : reason %d (state = %d)\n", reason,
               DWCi_GetMatchCnt()->state);
    s_sbCallbackLevel++;

    if (DWCi_GetMatchCnt()->sbUpdateRequestTick != 0) {
        if (reason == sbc_updatecomplete || reason == sbc_queryerror)
        {
            DWCi_GetMatchCnt()->sbUpdateRequestTick = 0;
            DWC_Printf(4, "ServerBrowserLimitUpdate timeout reset.(%s)\n",
                __FUNCTION__);
        }
        else
        {
            DWCi_GetMatchCnt()->sbUpdateRequestTick =
                DWCi_Np_GetTick() + (u64)(OS_BUS_CLOCK / 4 / 1000) * 30000;
            DWC_Printf(4, "ServerBrowserLimitUpdate timeout update.(%s)\n",
                __FUNCTION__);
        }
    }

    switch (reason)
    {
    case sbc_queryerror:
        break;
    case sbc_serveradded:
        DWCi_SBPrintServerData(server);
        break;

    case sbc_updatecomplete:
        DWCi_GetMatchCnt()->sbUpdateRequestTick = 0;
        for (i = 0; i < ServerBrowserCount(sb); i++)
        {
            server = ServerBrowserGetServer(sb, i);
            if (!DWCi_CheckDWCServer(server))
            {
                ServerBrowserRemoveServer(sb, server);
                DWC_Printf(0x400, "Deleted server [%d].\n", i);
                i--;
            }
        }

        switch (DWCi_GetMatchCnt()->state) {
        case DWC_MATCH_STATE_SEARCH_OWN:
            for (i = 0; i < ServerBrowserCount(sb); i++)
            {
                server = ServerBrowserGetServer(sb, i);
                if (DWCi_GetMatchCnt()->qr2IP != 0
                    && DWCi_GetMatchCnt()->qr2IP
                        == SBServerGetPublicInetAddress(server)
                    && DWCi_GetMatchCnt()->qr2Port != 0
                    && DWCi_GetMatchCnt()->qr2Port
                        == SBServerGetPublicQueryPort(server))
                {
                    break;
                }
            }
            if (i < ServerBrowserCount(sb))
            {
                memset(&info, 0, sizeof(DWCNodeInfo));
                info.profileID = DWCi_GetMatchCnt()->profileID;
                info.aid = 0xff;
                info.publicInetAddress = DWCi_GetMatchCnt()->qr2IP;
                info.publicQueryPort = DWCi_GetMatchCnt()->qr2Port;
                info.privateInetAddress = SBServerGetPrivateInetAddress(server);
                info.privateQueryPort = SBServerGetPrivateQueryPort(server);
                info.hasPrivateAddress = SBServerHasPrivateAddress(server);
                DWCi_Np_CPUCopyFast(info.connUserData, &DWCi_GetMatchCnt()->_8D0, 4);
                if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0) {
                    DWCi_NodeInfoList_AppendNodeInfo(&info);
                    DWCi_StartMatch();
                } else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 1) {
                    DWCi_NodeInfoList_AppendNodeInfo(&info);
                    DWCi_StartMatch();
                } else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 2) {
                    info.aid = DWCi_NodeInfoList_GetFreeAID();
                    DWCi_NodeInfoList_AppendNodeInfo(&info);
                    DWCi_StartMatch();
                } else if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 3) {
                    DWCi_NodeInfoList_AppendNodeInfo(&info);
                    DWCi_StartMatch();
                }
            }
            else
            {
                DWCi_GetMatchCnt()->sbUpdateFlag = 2;
                DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
            }
            break;

        case DWC_MATCH_STATE_CL_SEARCH_HOST:
            DWCi_EvaluateServers(1);
            DWCi_RandomizeServers();
            if (ServerBrowserCount(sb) != 0)
            {
                host = ServerBrowserGetServer(sb, 0);
                memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                       sizeof(DWCNodeInfo));
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID
                    = SBServerGetIntValueA(host, "dwc_pid", 0);
                DWCi_GetMatchCnt()->tempNewNodeInfo.aid = 0xff;
                DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress
                    = SBServerGetPublicInetAddress(host);
                DWCi_GetMatchCnt()->tempNewNodeInfo.publicQueryPort
                    = SBServerGetPublicQueryPort(host);
                DWCi_GetMatchCnt()->tempNewNodeInfo.privateInetAddress
                    = SBServerGetPrivateInetAddress(host);
                DWCi_GetMatchCnt()->tempNewNodeInfo.privateQueryPort
                    = SBServerGetPrivateQueryPort(host);
                DWCi_GetMatchCnt()->tempNewNodeInfo.hasPrivateAddress
                    = SBServerHasPrivateAddress(host);
                DWCi_GetMatchCnt()->qr2MatchType = 3;
                {
                    DWCMatchControl* cnt = DWCi_GetMatchCnt();

                    if (cnt->sbObj == NULL)
                    {
                        cnt->sbObj = ServerBrowserNewA(cnt->gameName,
                            cnt->gameName, cnt->secretKey, 0, 20, 1, SBFalse,
                            DWCi_SBCallback, NULL);
                    }
                }
                if (DWCi_GetMatchCnt()->sbObj != NULL ||
                    DWCi_HandleSBError((SBError)5) == 0) {
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
                    result = DWCi_SendResvCommand(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID, FALSE);
                    if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0) {
                        DWCi_HandleSBError(result);
                    } else {
                        DWCi_HandleGPError(result);
                    }
                }
            }
            else
            {
                DWCi_GetMatchCnt()->sbUpdateFlag = 2;
                {
                    OSTime now = DWCi_Np_GetTick();
                    DWCi_GetMatchCnt()->sbUpdateTick = now;
                }
                memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                       sizeof(DWCNodeInfo));
            }
            break;

        case DWC_MATCH_STATE_CL_SEARCH_EVAL_HOST:
            DWC_Printf(4, "searchIP: %d.%d.%d.%d, searchPort: %d\n",
                       DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress >> 24,
                       (DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress >> 16) & 0xff,
                       (DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress >> 8) & 0xff,
                       DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress & 0xff,
                       DWCi_GetMatchCnt()->tempNewNodeInfo.publicQueryPort);
            while (ServerBrowserCount(sb) != 0)
            {
                server = ServerBrowserGetServer(sb, 0);
                if (SBServerGetPublicInetAddress(server)
                        == DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress
                    && SBServerGetPublicQueryPort(server)
                        == DWCi_GetMatchCnt()->tempNewNodeInfo.publicQueryPort)
                {
                    break;
                }
                ServerBrowserRemoveServer(sb, server);
            }
            if (ServerBrowserCount(sb) != 0)
            {
                server = ServerBrowserGetServer(sb, 0);
                profileID = SBServerGetIntValueA(server, "dwc_pid", 0);
                if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 1) {
                    if (profileID != DWCi_GetMatchCnt()->reqProfileID) {
                        DWC_Printf(0x40,
                            "@@@@ @@@ BAD **** profileID != "
                            "DWCi_GetMatchCnt()->reqProfileID ****.\n");
                    }
                    if (profileID != DWCi_GetMatchCnt()->reqProfileID ||
                        DWCi_EvaluateServers(0) == 0) {
                        if (DWCi_GetMatchCnt()->tempNewNodeInfo.profileID ==
                            0) {
                            result = 0;
                        } else {
                            result = DWCi_SendMatchCommand(
                                5,
                                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID,
                                DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress,
                                DWCi_GetMatchCnt()
                                     ->tempNewNodeInfo.publicQueryPort,
                                NULL, 0);
                            DWCi_GetMatchCnt()->reqProfileID = 0;
                        }
                        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0) {
                            result = DWCi_HandleSBError(result);
                        } else {
                            result = DWCi_HandleGPError(result);
                        }
                        if (result == 0)
                        {
                            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
                            memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                                   sizeof(DWCNodeInfo));
                            result = DWCi_SendResvCommandToFriend(
                                FALSE, FALSE, DWCi_GetMatchCnt()->reqProfileID);
                            if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0) {
                                DWCi_HandleSBError(result);
                            } else {
                                DWCi_HandleGPError(result);
                            }
                        }
                        break;
                    }
                }
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_NN);
                ServerBrowserGetServer(sb, 0);
                nnError = DWCi_NNStartupAsync(
                    0, 0, &DWCi_GetMatchCnt()->tempNewNodeInfo);
                DWCi_HandleNNError(nnError);
            }
            else
            {
                DWCi_GetMatchCnt()->sbUpdateFlag = 2;
                DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
            }
            break;

        case DWC_MATCH_STATE_CL_SEARCH_GROUPID_HOST:
            count = ServerBrowserCount(sb);
            if (count != 0)
            {
                SBServer groupServer;
                groupServer = ServerBrowserGetServer(sb, 0);
                memset(&DWCi_GetMatchCnt()->tempNewNodeInfo, 0,
                       sizeof(DWCNodeInfo));
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID =
                    SBServerGetIntValueA(groupServer, "dwc_pid", 0);
                DWCi_GetMatchCnt()->tempNewNodeInfo.aid = 0xff;
                DWCi_GetMatchCnt()->tempNewNodeInfo.publicInetAddress =
                    SBServerGetPublicInetAddress(groupServer);
                DWCi_GetMatchCnt()->tempNewNodeInfo.publicQueryPort =
                    SBServerGetPublicQueryPort(groupServer);
                DWCi_GetMatchCnt()->tempNewNodeInfo.privateInetAddress =
                    SBServerGetPrivateInetAddress(groupServer);
                DWCi_GetMatchCnt()->tempNewNodeInfo.privateQueryPort =
                    SBServerGetPrivateQueryPort(groupServer);
                DWCi_GetMatchCnt()->tempNewNodeInfo.hasPrivateAddress =
                    SBServerHasPrivateAddress(groupServer);
                DWCi_GetMatchCnt()->qr2MatchTypeExt =
                    SBServerGetIntValueA(groupServer, "dwc_mtype", -1);
                DWC_Printf(0x400, "Find server num from groupID: %d.\n", count);
                {
                    int topology = DWCi_GetMatchTopology();
                    DWCi_ConnectToGameServerAsync(
                        topology,
                        SBServerGetIntValueA(ServerBrowserGetServer(sb, 0),
                                             "dwc_pid", 0),
                        DWCi_GetMatchCnt()->matchedCallback,
                        DWCi_GetMatchCnt()->matchedParam,
                        DWCi_GetMatchCnt()->newClientCallback,
                        DWCi_GetMatchCnt()->newClientParam,
                        DWCi_GetMatchCnt()->connectAttemptCallback,
                        &DWCi_GetMatchCnt()->_8D0,
                        DWCi_GetMatchCnt()->connectAttemptParam);
                }
            }
            else
            {
                DWCi_GetMatchCnt()->sbUpdateFlag = 2;
                DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
                DWC_Printf(0x400, "Find server: host not found.\n");
            }
            break;
        }
        break;
    }

    s_sbCallbackLevel--;
}

// .text:0x80334BC4 DWCi_SBPrintServerData (0x2C8)
static void DWCi_SBPrintServerData(SBServer server)
{
    int i;

    DWC_Printf(0x400, "SBServerGetPrivateAddress     = %s\n",
        SBServerGetPrivateAddress(server));
    DWC_Printf(0x400, "SBServerGetPrivateInetAddress = %x\n",
        SBServerGetPrivateInetAddress(server));
    DWC_Printf(0x400, "SBServerGetPrivateQueryPort   = %d\n",
        SBServerGetPrivateQueryPort(server));
    DWC_Printf(0x400, "SBServerGetPublicAddress      = %s\n",
        SBServerGetPublicAddress(server));
    DWC_Printf(0x400, "SBServerGetPublicInetAddres   = %x\n",
        SBServerGetPublicInetAddress(server));
    DWC_Printf(0x400, "SBServerGetPublicQueryPort    = %d\n",
        SBServerGetPublicQueryPort(server));
    DWC_Printf(0x400, "SBServerHasPrivateAddress     = %d\n",
        SBServerHasPrivateAddress(server));
    DWC_Printf(0x400, "numplayers  = %d\n",
        SBServerGetIntValueA(server, "numplayers", -1));
    DWC_Printf(0x400, "maxplayers  = %d\n",
        SBServerGetIntValueA(server, "maxplayers", -1));
    DWC_Printf(0x400, "%s     = %u\n", "dwc_pid",
        SBServerGetIntValueA(server, "dwc_pid", 0));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_mtype",
        SBServerGetIntValueA(server, "dwc_mtype", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_mver",
        SBServerGetIntValueA(server, "dwc_mver", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_groupid",
        SBServerGetIntValueA(server, "dwc_groupid", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_hoststate",
        SBServerGetIntValueA(server, "dwc_hoststate", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_suspend",
        SBServerGetIntValueA(server, "dwc_suspend", -1));

    for (i = 0; i < 154; i++)
    {
        if (stGameMatchKeys[i].keyID != 0)
        {
            if (stGameMatchKeys[i].isStr != 0)
            {
                DWC_Printf(0x400, "%s  = %s\n", stGameMatchKeys[i].keyStr,
                    SBServerGetStringValueA(server,
                        stGameMatchKeys[i].keyStr, "NONE"));
            }
            else
            {
                DWC_Printf(0x400, "%s  = %d\n", stGameMatchKeys[i].keyStr,
                    SBServerGetIntValueA(server, stGameMatchKeys[i].keyStr,
                        -1));
            }
        }
    }
}

// helper: node lookup by profile ID (inlined in CF; the real function is
// DWCi_NodeInfoList_GetNodeInfoForProfileID at 0x80338BE8)
static inline DWCNodeInfo* DWCi_FindNodeByProfileID(int profileID)
{
    int i;

    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        if (profileID == stpMatchCnt->nodeInfoList.nodes[i].profileID)
        {
            return &stpMatchCnt->nodeInfoList.nodes[i];
        }
    }
    return NULL;
}

// .text:0x80334E8C DWCi_EvaluateServers (0x208)
static int DWCi_EvaluateServers(int sort)
{
    int eval;
    int deleteFlag = FALSE;
    int localDelete;
    int profileID;
    int i;
    SBServer server;

    for (i = 0; i < ServerBrowserCount(DWCi_GetMatchCnt()->sbObj); i++)
    {
        server = ServerBrowserGetServer(DWCi_GetMatchCnt()->sbObj, i);

        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == 0)
        {
            profileID = SBServerGetIntValueA(server, "dwc_pid", 0);
            localDelete = FALSE;
            if (DWCi_FindNodeByProfileID(profileID) != NULL)
            {
                ServerBrowserRemoveServer(DWCi_GetMatchCnt()->sbObj, server);
                localDelete = TRUE;
                i--;
            }
            if (localDelete)
            {
                continue;
            }
        }

        if (DWCi_GetMatchCnt()->evalCallback != 0)
        {
            eval = ((int (*)(int, void*))DWCi_GetMatchCnt()->evalCallback)(
                i, (void*)DWCi_GetMatchCnt()->evalParam);

            if (eval > 0)
            {
                if (eval > 0x800000 - 1)
                {
                    eval = 0x800000 - 1;
                }
                SBServerAddIntKeyValue(server, "dwc_eval",
                    (eval << 8) | DWCi_GetMathRand32(0x100));
            }
            else
            {
                ServerBrowserRemoveServer(DWCi_GetMatchCnt()->sbObj, server);
                DWC_Printf(0x400,
                    "Deleted server [%d] (eval point is %d).\n", i, eval);
                deleteFlag = TRUE;
                i--;
            }
        }
        else
        {
            SBServerAddIntKeyValue(server, "dwc_eval", DWCi_GetMathRand32(0x80));
        }
    }

    if (sort != 0 && ServerBrowserCount(DWCi_GetMatchCnt()->sbObj) != 0)
    {
        ServerBrowserSortA(DWCi_GetMatchCnt()->sbObj, SBFalse, "dwc_eval",
            sbcm_int);
    }

    if (deleteFlag && ServerBrowserCount(DWCi_GetMatchCnt()->sbObj) == 0)
    {
        return 0;
    }
    return 1;
}

// .text:0x80335094 DWCi_RandomizeServers (0x1B8)
static void DWCi_RandomizeServers(void)
{
    u32 rand;
    int maxEval = 0;
    int total = 0;
    int i;
    int cumulative[6];

    if (ServerBrowserCount(stpMatchCnt->sbObj) <= 1)
    {
        return;
    }

    for (i = 0; i < ServerBrowserCount(stpMatchCnt->sbObj); i++)
    {
        int value = SBServerGetIntValueA(
            ServerBrowserGetServer(stpMatchCnt->sbObj, i), "dwc_eval", -1);

        if (value > maxEval)
        {
            maxEval = value;
        }
        total += stEvalRate[i];
    }

    rand = DWCi_GetMathRand32(0x64);
    for (i = 0; i < ServerBrowserCount(stpMatchCnt->sbObj); i++)
    {
        if (i == ServerBrowserCount(stpMatchCnt->sbObj) - 1)
        {
            cumulative[i] = 100;
            break;
        }
        cumulative[i] = stEvalRate[i] * 100 / total
            + (i > 0 ? cumulative[i - 1] : 0);
        if (rand < cumulative[i])
        {
            break;
        }
    }

    DWC_Printf(0x40, "Server[%d] is selected (%d/100: rand %d)\n", i,
        cumulative[i], rand);
    if (maxEval < 0x7FFFFFFF)
    {
        maxEval++;
    }
    SBServerAddIntKeyValue(ServerBrowserGetServer(stpMatchCnt->sbObj, i),
        "dwc_eval", maxEval);
    ServerBrowserSortA(stpMatchCnt->sbObj, SBFalse, "dwc_eval", sbcm_int);
}

// helper: DWCi_GetQR2KeyName (inlined in CF)
static inline const char* DWCi_GetQR2KeyName(int keyid)
{
    switch (keyid)
    {
    case NUMPLAYERS_KEY:
        return "NUMPLAYERS";
    case MAXPLAYERS_KEY:
        return "MAXPLAYERS";
    case DWC_QR2_PID_KEY:
        return "PID";
    case DWC_QR2_MATCH_TYPE_KEY:
        return "TYPE";
    case DWC_QR2_MATCH_RESV_KEY:
        return "VER";
    case DWC_QR2_MATCH_VER_KEY:
        return "EVAL";
    case DWC_QR2_MATCH_EVAL_KEY:
        return "GROUPID";
    case DWC_QR2_MATCH_EVAL_KEY + 1:
        return "HOST";
    case DWC_QR2_MATCH_EVAL_KEY + 2:
        return "SUSPEND";
    default:
        return "Unknown";
    }
}

// .text:0x8033524C DWCi_QR2ServerKeyCallback (0x894)
static void DWCi_QR2ServerKeyCallback(
    int keyid, qr2_buffer_t outbuf, void* userdata)
{
#pragma unused(userdata)
    int index;
    int value;

    switch (keyid)
    {
    case NUMPLAYERS_KEY:
    {
        value = DWCi_GetMatchCnt()->nodeInfoList.count;
        --value;
        if (value < 0)
        {
            value = 0;
        }
        qr2_buffer_add_int(outbuf, value);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), value);
        break;
    }
    case MAXPLAYERS_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2NumEntry);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), DWCi_GetMatchCnt()->qr2NumEntry);
        break;
    case DWC_QR2_PID_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->profileID);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), DWCi_GetMatchCnt()->profileID);
        break;
    case DWC_QR2_MATCH_TYPE_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2MatchTypeExt);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), DWCi_GetMatchCnt()->qr2MatchTypeExt);
        break;
    case DWC_QR2_MATCH_RESV_KEY:
        qr2_buffer_add_int(outbuf, DWC_MATCHING_VERSION);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), DWC_MATCHING_VERSION);
        break;
    case DWC_QR2_MATCH_VER_KEY:
        qr2_buffer_add_int(outbuf, 1);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), 1);
        break;
    case DWC_QR2_MATCH_EVAL_KEY:
        qr2_buffer_add_int(
            outbuf,
            ((s32)DWCi_GetMatchCnt()->hostState == 2 ? DWCi_GetMatchCnt()->groupID : 0));
        DWC_Printf(
            DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid),
            ((s32)DWCi_GetMatchCnt()->hostState == 2 ? DWCi_GetMatchCnt()->groupID : 0));
        break;
    case DWC_QR2_MATCH_EVAL_KEY + 1:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->hostState);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), DWCi_GetMatchCnt()->hostState);
        break;
    case DWC_QR2_MATCH_EVAL_KEY + 2:
    {
        value = DWC_GetSuspendMatch();
        qr2_buffer_add_int(outbuf, value);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d(%s) - %d\n", keyid,
            DWCi_GetQR2KeyName(keyid), value);
        break;
    }
    default:
        index = keyid - DWC_QR2_GAME_KEY_START;
        if ((index >= 0 && index < DWC_QR2_GAME_RESERVED_KEYS)
            && stGameMatchKeys[index].keyID)
        {
            if (stGameMatchKeys[index].isStr)
            {
                qr2_buffer_addA(
                    outbuf, (char*)stGameMatchKeys[index].value);
            }
            else
            {
                qr2_buffer_add_int(
                    outbuf, *(const int*)stGameMatchKeys[index].value);
            }
        }
        break;
    }
}

// .text:0x80335AE0 DWCi_QR2PlayerKeyCallback (0x4)
static void DWCi_QR2PlayerKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
}

// .text:0x80335AE4 DWCi_QR2TeamKeyCallback (0x4)
static void DWCi_QR2TeamKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
}

// .text:0x80335AE8 DWCi_QR2KeyListCallback (0x108)
static void DWCi_QR2KeyListCallback(
    qr2_key_type keytype, qr2_keybuffer_t keybuffer, void* userdata)
{
#pragma unused(userdata)
    int i;

    switch (keytype)
    {
    case key_server:
        qr2_keybuffer_add(keybuffer, NUMPLAYERS_KEY);
        qr2_keybuffer_add(keybuffer, MAXPLAYERS_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_PID_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_TYPE_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_RESV_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_VER_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_EVAL_KEY);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_EVAL_KEY + 1);
        qr2_keybuffer_add(keybuffer, DWC_QR2_MATCH_EVAL_KEY + 2);

        for (i = 0; i < DWC_QR2_GAME_RESERVED_KEYS; i++)
        {
            if (stGameMatchKeys[i].keyID)
            {
                qr2_keybuffer_add(keybuffer, stGameMatchKeys[i].keyID);
            }
        }
        break;
    case key_player:
        break;
    case key_team:
        break;
    }

    DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
        "QR2, Received KeyListReq : keytype %d\n", keytype);
}

// .text:0x80335BF0 DWCi_QR2CountCallback (0x8)
static int DWCi_QR2CountCallback(qr2_key_type keytype, void* userdata)
{
    return 0;
}

// .text:0x80335BF8 DWCi_QR2AddErrorCallback (0x68)
static void DWCi_QR2AddErrorCallback(
    qr2_error_t error, gsi_char* errmsg, void* userdata)
{
    DWC_Printf(2, "QR2 Failed query addition to master server %d\n", error);
    DWC_Printf(2, "%s\n", errmsg);
    DWCi_HandleQR2Error(error);
}

// .text:0x80335C60 DWCi_QR2PublicAddrCallback (0x6C)
static void DWCi_QR2PublicAddrCallback(
    unsigned int ip, unsigned short port, void* userdata)
{
    // the address is printed as four bytes
    DWC_Printf(0x40, "Got my query IP %d.%d.%d.%d & port %d.\n", ip >> 24, (ip >> 16) & 0xff,
        (ip >> 8) & 0xff, ip & 0xff, port);
    stpMatchCnt->qr2IP = ip;
    stpMatchCnt->qr2Port = port;
}

// .text:0x80335CCC DWCi_QR2NatnegCallback (0x1EC)
static void DWCi_QR2NatnegCallback(int cookie, void* userdata)
{
    DWC_Printf(0x40, "Got NN request, cookie = %x.\n", cookie);
    if (!DWC_IsServerMyself() && stpMatchCnt->tempNewClientInfo.profileID != 0)
    {
        DWC_Printf(0x40, "But I'm waiting new client...\n", cookie);
    }
    else
    {
        if (stpMatchCnt->state == DWC_MATCH_STATE_CL_WAITING)
        {
            if ((int)stpMatchCnt->topologyType == 2) {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_NN);
            } else {
                DWC_Printf(0x40, "I'm in CL_WAITING stat..ignore.\n");
                return;
            }
        }
        else if (stpMatchCnt->state != DWC_MATCH_STATE_CL_NN
            && stpMatchCnt->state != DWC_MATCH_STATE_SV_OWN_NN)
        {
            DWC_Printf(0x40, "But already canceled reservation.\n");
            return;
        }

        if (cookie == stpMatchCnt->nnLastCookie)
        {
            stpMatchCnt->nnRecvCount++;
        }
        else
        {
            stpMatchCnt->nnRecvCount = 0;
            stpMatchCnt->nnLastCookie = cookie;
        }
        stpMatchCnt->nnFailedTime = 0;
        if (DWCi_HandleNNError(
                DWCi_NNStartupAsync(1, cookie, &stpMatchCnt->tempNewNodeInfo))
            == 0)
        {
            stpMatchCnt->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
        }
    }
}

// .text:0x80335EB8 DWCi_HandleGT2UnreliableMatchCommandMessage (0x170)
void DWCi_HandleGT2UnreliableMatchCommandMessage(char* data, int len)
{
    DWCSBMessage sbMsg;

    if (DWC_GetState() != DWC_STATE_MATCHING)
    {
        if (DWC_GetState() != DWC_STATE_CONNECTED
            || (DWCi_GetMatchCnt()->qr2MatchType != 2
                && DWCi_GetMatchCnt()->qr2MatchType != 3))
        {
            DWC_Printf(4, "Ignore delayed GT2 matching command.\n");
            return;
        }
    }

    DWCi_Np_CpuCopy8(data, &sbMsg, sizeof(DWCSBMessageHeader));
    sbMsg.header.version = DWCi_LEtoHl(sbMsg.header.version);
    sbMsg.header.qr2Port = DWCi_LEtoHs(sbMsg.header.qr2Port);
    sbMsg.header.profileID = DWCi_LEtoHl(sbMsg.header.profileID);
    if (sbMsg.header.version != DWC_MATCHING_VERSION)
    {
        DWC_Printf(8, "Got different version GT2 command.\n");
        return;
    }
    if (len != sbMsg.header.size + sizeof(DWCSBMessageHeader))
    {
        DWC_Printf(8, "Got wrong data size GT2 command.\n");
        return;
    }
    DWCi_Np_CpuCopy8(data + sizeof(DWCSBMessageHeader), sbMsg.data,
        sbMsg.header.size);
    DWC_Printf(0x40, "<GT2> RECV-0x%02x <- [--------:-----] [pid=%u]\n",
        sbMsg.header.command, sbMsg.header.profileID);
    DWCi_ProcessRecvMatchCommand(sbMsg.header.command, sbMsg.header.profileID,
        sbMsg.header.qr2IP, sbMsg.header.qr2Port, sbMsg.data,
        sbMsg.header.size >> 2);
}

// .text:0x80336028 DWCi_QR2ClientMsgCallback (0x1B8)
static void DWCi_QR2ClientMsgCallback(
    gsi_char* data, int len, void* userdata)
{
#pragma unused(userdata)
    int offset = 0;
    BOOL boolResult;
    DWCSBMessage sbMsg;

    if (DWC_GetState() != DWC_STATE_MATCHING)
    {
        if (DWC_GetState() != DWC_STATE_CONNECTED
            || (DWCi_GetMatchCnt()->qr2MatchType != 2
                && DWCi_GetMatchCnt()->qr2MatchType != 3))
        {
            DWC_Printf(4, "Ignore delayed SB matching command.\n");
            return;
        }
    }

    while (offset + (int)sizeof(DWCSBMessageHeader) <= len)
    {
        DWCi_Np_CpuCopy8(data, &sbMsg, sizeof(DWCSBMessageHeader));
        sbMsg.header.version = DWCi_LEtoHl(sbMsg.header.version);
        sbMsg.header.qr2Port = DWCi_LEtoHs(sbMsg.header.qr2Port);
        sbMsg.header.profileID = DWCi_LEtoHl(sbMsg.header.profileID);
        if (strncmp(sbMsg.header.identifier, DWC_SB_COMMAND_STRING, 4) != 0)
        {
            DWC_Printf(8, "Got undefined SBcommand.\n");
            return;
        }
        if (sbMsg.header.version != DWC_MATCHING_VERSION)
        {
            DWC_Printf(8, "Got different version SBcommand.\n");
            return;
        }
        DWCi_Np_CpuCopy8(data + sizeof(DWCSBMessageHeader), sbMsg.data,
            sbMsg.header.size);
        DWC_Printf(0x40, "<SB> RECV-0x%02x <- [%08x:%d] [pid=%u]\n",
            sbMsg.header.command, sbMsg.header.qr2IP, sbMsg.header.qr2Port,
            sbMsg.header.profileID);
        boolResult = DWCi_ProcessRecvMatchCommand(sbMsg.header.command,
            sbMsg.header.profileID, sbMsg.header.qr2IP, sbMsg.header.qr2Port,
            sbMsg.data, sbMsg.header.size >> 2);
        if (!boolResult)
        {
            return;
        }
        offset += sizeof(DWCSBMessageHeader) + sbMsg.header.size;
    }
}

// .text:0x803361E0 DWCi_NNProgressCallback (0x18)
static void DWCi_NNProgressCallback(NegotiateState state, void* userdata)
{
    DWC_Printf(0x40, "NN, Got state update: %d\n", state);
}

// .text:0x803361F8 DWCi_NNCompletedCallback (0x102C)

static void DWCi_NNCompletedCallback(NegotiateResult result, SOCKET gamesocket,
    struct sockaddr_in* remoteaddr, void* userdata)
{
#pragma unused(gamesocket)
    char pidStr[24];
    NegotiateError nnError;
    NegotiateResult nnResult;
    GT2Result gt2Result;
    DWCNNInfo* nnInfo = (DWCNNInfo*)userdata;

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN, Complete NAT Negotiation. result : %d\n", result);
    if (nnInfo)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN cookie = %x.\n", nnInfo->cookie);
    }

    if ((DWCi_GetMatchCnt()->state != 5 && DWCi_GetMatchCnt()->state != 14) ||
        !nnInfo) {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Ignore delayed NN after cancel.\n");
        return;
    }

    if (result == nr_success)
    {
        if (remoteaddr)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN, remote address : %s\n",
                gt2AddressToString(remoteaddr->sin_addr.addr, SO_NtoHs(remoteaddr->sin_port), NULL));
        }

        nnInfo->cookie = 0;

        if (nnInfo->isQR2 != 0) {
            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP = remoteaddr->sin_addr.addr;
            DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port =
                SO_NtoHs(remoteaddr->sin_port);

            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                       "NN child finished Nat Negotiation.\n");

            DWCi_GetMatchCnt()->nnRecvCount = 0;
            DWCi_GetMatchCnt()->nnLastCookie = 0;
            DWCi_GetMatchCnt()->nnFailedTime = 0;

            if (DWCi_GetMatchCnt()->state == 14) {
                DWCi_SetMatchStatus(15);
            } else {
                DWCi_SetMatchStatus(6);
            }

            DWCi_GetMatchCnt()->gt2ConnectCount = 0;

            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2, "gt2Connect() to (%s)\n",
                       gt2AddressToString(
                           DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP,
                           DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port, NULL));

            (void)snprintf(pidStr, sizeof(pidStr), "%u",
                           DWCi_GetMatchCnt()->profileID);
            gt2Result =
                gt2Connect(*DWCi_GetMatchCnt()->pGt2Socket, NULL,
                           gt2AddressToString(
                               DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP,
                               DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port, NULL),
                           (GT2Byte *)pidStr, -1, DWC_GT2_CONNECT_TIMEOUT,
                           DWCi_GetMatchCnt()->gt2Callbacks, GT2False);

            if (DWCi_GetMatchCnt()->inMeshMaking != 0 &&
                gt2Result != GT2Success) {
                DWCi_NodeInfoList_GetNodeInfoForProfileID(
                    DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
                    ->nnTryCount++;
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh NN Count++.\n");
                DWCi_StopMeshMaking();
                DWCi_SetMatchStatus(1);
                return;
            } else {
                if (gt2Result == GT2OutOfMemory) {
                    DWCi_HandleGT2Error(gt2Result);
                    return;
                }
                if (gt2Result == GT2Success) {
                    return;
                }
                if (DWCi_CancelPreConnectedServerProcess(
                        DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)) {
                    return;
                }
                return;
            }
        } else {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN parent finished Nat Negotiation.\n");

            if (remoteaddr)
            {
                DWCi_GetMatchCnt()->tempNewNodeInfo.gt2IP =
                    remoteaddr->sin_addr.addr;
                DWCi_GetMatchCnt()->tempNewNodeInfo.gt2Port =
                    SO_NtoHs(remoteaddr->sin_port);
            }

            DWCi_GetMatchCnt()->nnFinishTime = DWCi_Np_GetTick();
            DWCi_SetMatchStatus(6);
            return;
        }
    }

    if (nnInfo->cookie == 0)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Ignore delayed NN error after cancel.\n");
        return;
    }

    if (DWCi_GetMatchCnt()->inMeshMaking != 0) {
        DWCi_NodeInfoList_GetNodeInfoForProfileID(
            DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)
            ->nnTryCount++;
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Mesh NN Count++.\n");
        DWCi_StopMeshMaking();
        DWCi_SetMatchStatus(1);
        return;
    }

    nnResult = DWCi_HandleNNResult(result);

    if (nnResult != nr_inittimeout && nnResult != nr_deadbeatpartner &&
        nnResult != nr_pingtimeout) {
        return;
    }

    if (nnInfo->isQR2 == 0) {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Failed %d/%d NN send.\n",
                   nnInfo->retryCount, DWC_MAX_MATCH_NN_RETRY);

        if (nnResult == nr_deadbeatpartner || nnResult == nr_pingtimeout ||
            (nnResult == nr_inittimeout &&
             nnInfo->retryCount >= DWC_MAX_MATCH_NN_RETRY)) {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Abort NN.\n");
            nnInfo->cookie = 0;

            if (!DWCi_ProcessNNFailure(FALSE)) {
                return;
            }

            if (DWCi_CancelPreConnectedClientProcess(
                    DWCi_GetMatchCnt()->tempNewNodeInfo.profileID,
                    (DWCError)DWC_ERROR_MATCH_24, -80430)) {
                return;
            }
            return;
        }

        nnInfo->retryCount++;
        nnError = DWCi_DoNatNegotiationAsync(nnInfo);
        if (!DWCi_HandleNNError(nnError)) {
            return;
        }
        return;
    } else {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Failed %d/%d NN recv.\n",
                   DWCi_GetMatchCnt()->nnRecvCount, DWC_MAX_MATCH_NN_RETRY);
        DWCi_GetMatchCnt()->nnFailedTime = DWCi_Np_GetTick();

        if (nnResult != nr_deadbeatpartner && nnResult != nr_pingtimeout) {
            if (nnResult != nr_inittimeout) {
                return;
            }
            if (DWCi_GetMatchCnt()->nnRecvCount < DWC_MAX_MATCH_NN_RETRY) {
                return;
            }
        }

        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Abort NN.\n");
        nnInfo->cookie = 0;

        if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL ||
            DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV) {
            if (!DWCi_ProcessNNFailure(TRUE)) {
                return;
            }
        } else if (!DWCi_ProcessNNFailure(FALSE)) {
            return;
        }

        DWCi_GetMatchCnt()->nnRecvCount = 0;
        DWCi_GetMatchCnt()->nnLastCookie = 0;
        DWCi_GetMatchCnt()->nnFailedTime = 0;
        if (!DWCi_CancelPreConnectedServerProcess(
                DWCi_GetMatchCnt()->tempNewNodeInfo.profileID)) {
            return;
        }
        return;
    }
}

// .text:0x80337224 DWCi_SendServerDownQuery (0x118)
BOOL DWCi_SendServerDownQuery(void)
{
    int i;
    int count = stpMatchCnt->nodeInfoList.count;

    for (i = 1; i < count; i++)
    {
        DWCNodeInfo* node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
        u32 data = DWCi_HtoLEl(DWC_GetAIDBitmap());
        u8 command = 0x52;
        int result;

        if ((1 << node->aid) & stpMatchCnt->serverDownBuff)
        {
            command = 0x55;
        }

        result = DWCi_SendMatchCommand(command, node->profileID, node->publicInetAddress, node->publicQueryPort, &data, 1);

        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
    }

    stpMatchCnt->clWaitTimeoutCount = 0;
    stpMatchCnt->clWaitTime = OSGetTime();
    return TRUE;
}

// .text:0x8033733C DWCi_IsBackupMine (0x84)
BOOL DWCi_IsBackupMine(void)
{
    DWCNodeInfo* server = DWCi_NodeInfoList_GetNodeInfoForIndex(0);

    if (server == NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_INFO, "*** DWCi_IsBackupMine. ignored(state=%d)\n", DWC_GetMatchingState());
        return FALSE;
    }
    return server->profileID == stpMatchCnt->profileID;
}

// .text:0x803373C0 DWCi_SetupGameServer_InterChange (0x378)
void DWCi_SetupGameServer_InterChange(BOOL newGroup)
{
    char value[12];
    char status[44];
    int gpStatus;
    int num;
    int lock;

    if (newGroup)
    {
        DWCi_CreateNewGroupID();
        DWCi_NodeInfoList_GetNodeInfoForProfileID(stpMatchCnt->profileID)->aid = 0;
        stpMatchCnt->firstServer = 1;
    }
    else
    {
        stpMatchCnt->firstServer = 0;
    }

    stpMatchCnt->qr2MatchType = 2;
    DWCi_SetMatchStatus(13);
    stpMatchCnt->hostState = 2;
    stpMatchCnt->qr2Reservation = 0;
    stpMatchCnt->qr2IsReserved = 0;

    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
    {
        gpStatus = 3;
    }
    else
    {
        gpStatus = 6;
        if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
        {
            gpStatus = 4;
        }
    }

    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        stpMatchCnt->qr2MatchTypeExt = DWC_MATCH_TYPE_SC_SV;
    }

    snprintf(value, sizeof(value), "%u", stpMatchCnt->qr2NumEntry + 1);
    DWC_SetCommonKeyValueString("SCM", value, status, '/');

    num = stpMatchCnt->nodeInfoList.count;
    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV && num == 0)
    {
        num = 1;
    }
    snprintf(value, sizeof(value), "%u", num);
    DWC_AddCommonKeyValueString("SCN", value, status, '/');

    snprintf(value, sizeof(value), "%u", 90);
    DWC_AddCommonKeyValueString("VER", value, status, '/');

    if (stpMatchCnt->serverLocked == 0)
    {
        lock = 0;
    }
    else if (DWC_IsServerMyself()
        && (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL
            || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
        && stpMatchCnt->firstServer == 0)
    {
        lock = 1;
    }
    else
    {
        lock = 0;
    }
    snprintf(value, sizeof(value), "%u", lock != 0);
    DWC_AddCommonKeyValueString("LCK", value, status, '/');

    DWCi_HandleGPError(DWCi_SetGPStatus(gpStatus, status, NULL));
    stpMatchCnt->reqProfileID = 0;
}

// .text:0x80337738 DWCi_SendServerPolling (0x210)
int DWCi_SendServerPolling(int force)
{
    u32 data[3];
    int i;
    DWCNodeInfo* node;
    int result;

    if (!DWC_IsServerMyself())
    {
        return 1;
    }

    if ((u32)(DWCi_GetMatchCnt()->state - 19) <= 1) {
        return 1;
    }

    if (force != 0 ||
        OSTicksToMilliseconds(DWCi_Np_GetTick() -
                              DWCi_GetMatchCnt()->serverPollingTime) > 15000) {
        data[0] = DWCi_HtoLEl(DWCi_GetMatchCnt()->profileID);
        data[1] = DWCi_HtoLEl(DWC_GetAIDBitmap());
        data[2] = DWCi_HtoLEl(DWCi_GetMatchCnt()->serverPollingUID);

        for (i = 0; i < DWCi_GetMatchCnt()->nodeInfoList.count; i++) {
            node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
            if (node != DWCi_NodeInfoList_GetNodeInfoForProfileID(
                            DWCi_GetMatchProfileID())) {
                result = DWCi_SendMatchCommand(0x83, node->profileID, node->publicInetAddress, node->publicQueryPort, data, 3);
                if (DWCi_HandleMatchCommandError(result))
                {
                    return -1;
                }
            }
        }

        DWCi_GetMatchCnt()->serverPollingTime = DWCi_Np_GetTick();
    }
    return 1;
}

// .text:0x80337948 DWCi_ProcessRequestMatch (0x4B8)
BOOL DWCi_ProcessRequestMatch(u8 aid, int suspend, int mode)
{
    BOOL done;
    DWCNodeInfo* server;
    u32 bitmap;
    u8 serverAid;
    u32 oldBitmap;
    BOOL valid;
    DWCNodeInfo* node;
    u32 data[4];
    int len;
    int result;
    int i;

    if (stpMatchCnt->nodeInfoList.count > 0)
    {
        server = &stpMatchCnt->nodeInfoList.nodes[0];
    }
    else
    {
        server = NULL;
    }
    serverAid = server->aid;

    oldBitmap = stpMatchCnt->suspendMatchBitmap;
    if (suspend)
    {
        stpMatchCnt->suspendMatchBitmap = oldBitmap | (1 << aid);
    }
    else
    {
        stpMatchCnt->suspendMatchBitmap = oldBitmap & ~(1 << aid);
    }

    if (oldBitmap != stpMatchCnt->suspendMatchBitmap)
    {
        char value[12];
        char status[44];
        int num;
        int lock;

        if (aid == serverAid)
        {
            DWC_Printf(DWC_REPORTFLAG_RECV_INFO, "**Server** AID(%u) suspend(%u) bitmap(%u->%u).\n", aid, suspend, oldBitmap, stpMatchCnt->suspendMatchBitmap);
        }

        snprintf(value, sizeof(value), "%u", stpMatchCnt->qr2NumEntry + 1);
        DWC_SetCommonKeyValueString("SCM", value, status, '/');

        num = stpMatchCnt->nodeInfoList.count;
        if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV && num == 0)
        {
            num = 1;
        }
        snprintf(value, sizeof(value), "%u", num);
        DWC_AddCommonKeyValueString("SCN", value, status, '/');

        snprintf(value, sizeof(value), "%u", 90);
        DWC_AddCommonKeyValueString("VER", value, status, '/');

        if (stpMatchCnt->serverLocked == 0)
        {
            lock = 0;
        }
        else if (DWC_IsServerMyself()
            && (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL
                || stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
            && stpMatchCnt->firstServer == 0)
        {
            lock = 1;
        }
        else
        {
            lock = 0;
        }
        snprintf(value, sizeof(value), "%u", lock != 0);
        DWC_AddCommonKeyValueString("LCK", value, status, '/');

        DWCi_SetGPStatus(-1, status, NULL);
    }

    bitmap = stpMatchCnt->suspendMatchBitmap;
    if (DWC_IsServerMyself())
    {
        if (stpMatchCnt->state == 13 || stpMatchCnt->state == 22)
        {
            valid = TRUE;
        }
        else
        {
            valid = FALSE;
        }
    }
    else
    {
        valid = TRUE;
    }
    if (!valid)
    {
        bitmap &= ~(1 << DWC_GetMyAID());
    }

    {
        BOOL current = DWC_GetSuspendMatch();

        done = FALSE;
        if ((suspend != 0 && current != 0) || (suspend == 0 && current == 0))
        {
            done = TRUE;
        }
    }

    if (mode == 1)
    {
        node = DWCi_NodeInfoList_GetNodeInfoForAID(aid);

        data[0] = DWCi_HtoLEl(stpMatchCnt->profileID);
        data[1] = DWCi_HtoLEl(1);
        len = 2;
        if (done && stpMatchCnt->suspendServerResponse != 0)
        {
            data[2] = DWCi_HtoLEl(suspend);
            data[3] = DWCi_HtoLEl(bitmap);
            len = 4;
        }

        result = DWCi_SendMatchCommand(0x82, node->profileID, node->publicInetAddress, node->publicQueryPort, data, len);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
    }

    if (done && stpMatchCnt->suspendServerResponse != 1)
    {
        stpMatchCnt->suspendServerResponse = 1;

        data[0] = DWCi_HtoLEl(stpMatchCnt->profileID);
        data[1] = DWCi_HtoLEl(1);
        data[2] = DWCi_HtoLEl(suspend);
        data[3] = DWCi_HtoLEl(bitmap);

        for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
        {
            node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);
            if (node != server)
            {
                result = DWCi_SendMatchCommand(0x82, node->profileID, node->publicInetAddress, node->publicQueryPort, data, 4);
                if (DWCi_HandleMatchCommandError(result))
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

// .text:0x80337E00 DWCi_StartMatch (0x500)
BOOL DWCi_StartMatch(void)
{
    int result;

    if (stpMatchCnt->sbObj != NULL)
    {
        ServerBrowserClear(stpMatchCnt->sbObj);
    }

    if (stpMatchCnt->connectGroupID)
    {
        DWCi_SetMatchStatus(12);
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
    {
        DWCi_SetMatchStatus(2);
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_FRIEND)
    {
        DWCi_SetMatchStatus(3);
        result = DWCi_SendResvCommandToFriend(FALSE, TRUE, 0);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
        return TRUE;
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV)
    {
        DWCi_SetMatchStatus(13);
        stpMatchCnt->connectedOnceBefore = 1;
        qr2_send_statechanged(stpMatchCnt->qr2Obj);
        return TRUE;
    }
    else if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        if (gpIsBuddy(stpMatchCnt->pGpObj, stpMatchCnt->tempNewNodeInfo.profileID))
        {
            DWCi_SetMatchStatus(3);
            result = DWCi_SendResvCommand(stpMatchCnt->tempNewNodeInfo.profileID, FALSE);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }
        }
        return TRUE;
    }

    stpMatchCnt->reqProfileID = 0;
    return !DWCi_HandleSBError(DWCi_SBUpdateAsync(stpMatchCnt->reqProfileID));
}

// .text:0x80338300 DWCi_RequestSuspendMatchAsync (0x160)
// helper: SetSuspendRequest (inlined in CF)
static inline void DWCi_SetSuspendRequest(int suspend, DWCiSuspendCallback callback, void* param)
{
    OSTime now;
    stpMatchCnt->suspendServerResponse = 0;
    stpMatchCnt->suspend = suspend;
    stpMatchCnt->suspendCallback = (u32)callback;
    stpMatchCnt->suspendResultParam = (u32)param;
    stpMatchCnt->suspendRequestTick = 0;
    now = OSGetTime();
    stpMatchCnt->suspendTimeoutTick = now;
    stpMatchCnt->isSuspending = 1;
}

BOOL DWCi_RequestSuspendMatchAsync(int suspend, DWCiSuspendCallback callback, void* param)
{
    if (DWCi_IsError() || stpMatchCnt == NULL || stpMatchCnt->hostState == 0
        || (DWC_GetState() != DWC_STATE_MATCHING && DWC_GetState() != DWC_STATE_CONNECTED)
        || stpMatchCnt->hostState == 0 || (stpMatchCnt->state != 13 && stpMatchCnt->state != 1)
        || callback == NULL || stpMatchCnt->connectedOnceBefore == 0)
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR, "but ignored.\n");
        return FALSE;
    }

    if (stpMatchCnt->isSuspending != 0)
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR, "***<SUSPEND> suspend process(%s) has not ended yet!\n",
            stpMatchCnt->suspend ? "TRUE" : "FALSE");
        return FALSE;
    }

    if (suspend != stpMatchCnt->suspend)
    {
        DWCi_SetSuspendRequest(suspend, callback, param);
    }
    return TRUE;
}

// .text:0x80338460 DWCi_RequestSuspendMatchProcess (0x15C)
BOOL DWCi_RequestSuspendMatchProcess(void)
{
    int suspend;
    BOOL current;

    if (stpMatchCnt->isSuspending == 0)
    {
        return FALSE;
    }

    if (stpMatchCnt->suspendRetry != 0)
    {
        DWCi_SetSuspendRequest(stpMatchCnt->suspend, (DWCiSuspendCallback)stpMatchCnt->suspendCallback, (void*)stpMatchCnt->suspendResultParam);
        stpMatchCnt->suspendRetry = 0;
    }

    suspend = stpMatchCnt->suspend;
    if (suspend == 0xFF)
    {
        return FALSE;
    }

    current = DWC_GetSuspendMatch();
    if ((suspend != 0 && current != 0) || (suspend == 0 && current == 0))
    {
        DWC_Printf(DWC_REPORTFLAG_RECV_INFO, "suspend Match(%s) completed!\n", suspend ? "TRUE" : "FALSE");

        if (stpMatchCnt->suspendCallback != 0)
        {
            ((DWCiSuspendCallback)stpMatchCnt->suspendCallback)(0, stpMatchCnt->suspend, (void*)stpMatchCnt->suspendResultParam);
        }

        qr2_send_statechanged(stpMatchCnt->qr2Obj);
        stpMatchCnt->suspend = 0xFF;
        stpMatchCnt->isSuspending = 0;
        return TRUE;
    }

    DWCi_RequestSuspendMatch(suspend);
    return TRUE;
}

static inline int DWCi_GetMatchProfileID(void) {
    return DWCi_GetMatchCnt()->profileID;
}

// .text:0x803385BC DWCi_RequestSuspendMatch (0x2D0)
BOOL DWCi_RequestSuspendMatch(int suspend) {
    DWCNodeInfo* self;
    DWCNodeInfo* server;
    u32 data[4];
    int result;

    if ((s32)DWCi_GetMatchCnt()->hostState == 2) {
        return DWCi_ProcessRequestMatch(
            DWCi_NodeInfoList_GetNodeInfoForProfileID(DWCi_GetMatchProfileID())
                ->aid,
            suspend, 0);
    }

    if (DWCi_GetMatchCnt()->state != 1) {
        return TRUE;
    }

    if (DWCi_GetMatchCnt()->suspendServerResponse != 0) {
        if (OSTicksToMilliseconds(DWCi_Np_GetTick() -
                                  DWCi_GetMatchCnt()->suspendRequestTick) <
            DWCi_GetMatchCnt()->suspendWaitTime) {
            return TRUE;
        }
    }

    if (OSTicksToMilliseconds(DWCi_Np_GetTick() -
                              DWCi_GetMatchCnt()->suspendRequestTick) > 3000) {
        self =
            DWCi_NodeInfoList_GetNodeInfoForProfileID(DWCi_GetMatchProfileID());
        server = DWCi_NodeInfoList_GetNodeInfoForIndex(0);

        data[0] = DWCi_HtoLEl(DWCi_GetMatchCnt()->profileID);
        data[1] = 0;
        data[2] = DWCi_HtoLEl(suspend);
        data[3] = DWCi_HtoLEl(self->aid);

        result = DWCi_SendMatchCommand(0x82, server->profileID, server->publicInetAddress, server->publicQueryPort, data, 4);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }

        DWCi_GetMatchCnt()->suspendRequestTick = DWCi_Np_GetTick();
        DWCi_GetMatchCnt()->suspendWaitTime = 3000;
    }
    return TRUE;
}

// .text:0x8033888C DWCi_IsServerChanging (0x28)
BOOL DWCi_IsServerChanging(void)
{
    return (u32)(stpMatchCnt->state - 9) <= 2;
}

// prints a state change with the time spent in the old state (inlined)
static void disp_match_state(DWCMatchState old, DWCMatchState now)
{
    int m_time;
    int sec;
    DWCTick tick;

    tick = sMatchStateTick;
    sMatchStateTick = DWCi_Np_GetTick();
    tick = sMatchStateTick - tick;
    m_time = (int)DWCi_Np_TicksToMilliSeconds(tick);
    sec = m_time / 1000;
    m_time = (m_time - sec + 50) / 100;
    DWC_Printf(1, "status changed %s(%d.%d sec) -> %s\n",
               match_st_string[old], sec, m_time, match_st_string[now]);
}

// .text:0x803388B4 DWCi_SetMatchStatus (0xE8)
void DWCi_SetMatchStatus(DWCMatchState state)
{
    if (stpMatchCnt->state != state)
    {
        disp_match_state(stpMatchCnt->state, state);
    }
    stpMatchCnt->state = state;
}

// .text:0x8033899C DWCi_IsServerChangeEnabled (0x1C)
BOOL DWCi_IsServerChangeEnabled(void)
{
    return stpMatchCnt->topologyType != 1;
}

// .text:0x803389B8 DWCi_NodeInfoList_Clear (0x14)
void DWCi_NodeInfoList_Clear(void)
{
    memset(&stpMatchCnt->nodeInfoList, 0, sizeof(DWCNodeInfoList));
}

// .text:0x803389CC DWCi_NodeInfoList_GetCount (0xC)
int DWCi_NodeInfoList_GetCount(void)
{
    return stpMatchCnt->nodeInfoList.count;
}

// .text:0x803389D8 DWCi_NodeInfoList_RemoveAID (0xE4)
BOOL DWCi_NodeInfoList_RemoveAID(u8 aid)
{
    int i;

    if (stpMatchCnt->nodeInfoList.count == 0)
    {
        return FALSE;
    }

    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        if (stpMatchCnt->nodeInfoList.nodes[i].aid == aid)
        {
            break;
        }
    }

    if (i == stpMatchCnt->nodeInfoList.count)
    {
        return FALSE;
    }

    for (; i < stpMatchCnt->nodeInfoList.count - 1; i++)
    {
        stpMatchCnt->nodeInfoList.nodes[i] = stpMatchCnt->nodeInfoList.nodes[i + 1];
    }

    stpMatchCnt->nodeInfoList.count--;
    return TRUE;
}

// .text:0x80338ABC DWCi_NodeInfoList_RemoveProfileID (0xE4)
BOOL DWCi_NodeInfoList_RemoveProfileID(int profileID)
{
    int i;

    if (stpMatchCnt->nodeInfoList.count == 0)
    {
        return FALSE;
    }

    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        if (stpMatchCnt->nodeInfoList.nodes[i].profileID == profileID)
        {
            break;
        }
    }

    if (i == stpMatchCnt->nodeInfoList.count)
    {
        return FALSE;
    }

    for (; i < stpMatchCnt->nodeInfoList.count - 1; i++)
    {
        stpMatchCnt->nodeInfoList.nodes[i] = stpMatchCnt->nodeInfoList.nodes[i + 1];
    }

    stpMatchCnt->nodeInfoList.count--;
    return TRUE;
}

// .text:0x80338BA0 DWCi_NodeInfoList_GetServerNodeInfo (0x20)
DWCNodeInfo* DWCi_NodeInfoList_GetServerNodeInfo(void)
{
    if (stpMatchCnt->nodeInfoList.count >= 1)
    {
        return &stpMatchCnt->nodeInfoList.nodes[0];
    }
    return NULL;
}

// .text:0x80338BC0 DWCi_NodeInfoList_GetNodeInfoForIndex (0x28)
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForIndex(int index)
{
    if (index < stpMatchCnt->nodeInfoList.count)
    {
        return &stpMatchCnt->nodeInfoList.nodes[index];
    }
    return NULL;
}

// .text:0x80338BE8 DWCi_NodeInfoList_GetNodeInfoForProfileID (0x4C)
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForProfileID(int profileID)
{
    int i;

    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        if (stpMatchCnt->nodeInfoList.nodes[i].profileID == profileID)
        {
            return &stpMatchCnt->nodeInfoList.nodes[i];
        }
    }
    return NULL;
}

// .text:0x80338C34 DWCi_NodeInfoList_GetNodeInfoForAID (0x4C)
DWCNodeInfo* DWCi_NodeInfoList_GetNodeInfoForAID(u8 aid)
{
    int i;

    for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
    {
        if (stpMatchCnt->nodeInfoList.nodes[i].aid == aid)
        {
            return &stpMatchCnt->nodeInfoList.nodes[i];
        }
    }
    return NULL;
}

// .text:0x80338C80 DWCi_NodeInfoList_GetFreeAID (0x6C)
u8 DWCi_NodeInfoList_GetFreeAID(void)
{
    u8 aid;
    int i;

    if (stpMatchCnt->nodeInfoList.count == DWC_MAX_CONNECTIONS)
    {
        return 0xFF;
    }

    for (aid = 0; aid < DWC_MAX_CONNECTIONS; aid++)
    {
        for (i = 0; i < stpMatchCnt->nodeInfoList.count; i++)
        {
            if (stpMatchCnt->nodeInfoList.nodes[i].aid == aid)
            {
                break;
            }
        }

        if (i != stpMatchCnt->nodeInfoList.count) {
            continue;
        }
        return aid;
    }
    return 0xFF;
}

// .text:0x80338CEC DWCi_GetNextMeshMakeTryTick (0x1A0)
OSTime DWCi_GetNextMeshMakeTryTick(void)
{
    int i;
    DWCNodeInfo *self =
        DWCi_NodeInfoList_GetNodeInfoForProfileID(DWCi_GetMatchProfileID());
    OSTime tick = OSGetTime();
    int connected = 0;
    int selfIndex = 0;

    tick += DWCi_Np_MilliSecondsToTicks(3000);

    if (stpMatchCnt->nodeInfoList.count > 2)
    {
        for (i = 1; i < stpMatchCnt->nodeInfoList.count; i++)
        {
            DWCNodeInfo* node = DWCi_NodeInfoList_GetNodeInfoForIndex(i);

            if (self == node)
            {
                selfIndex = i;
            }
            else
            {
                DWCi_GetGT2Connection(node->aid);
                if (DWCi_GetGT2Connection(node->aid) != NULL)
                {
                    connected++;
                }
            }
        }
    }

    tick += DWCi_Np_MilliSecondsToTicks(connected) * 3000;
    tick += DWCi_Np_MilliSecondsToTicks((stpMatchCnt->nodeInfoList.count - 1 - selfIndex) * 6000);
    return tick;
}

// .text:0x80338E8C DWCi_StopMeshMaking (0x248)
BOOL DWCi_StopMeshMaking(void)
{
    int result;

    DWC_Printf(DWC_REPORTFLAG_INFO, "mesh making stat(%d)\n", stpMatchCnt->inMeshMaking);

    if (stpMatchCnt->inMeshMaking == 0)
    {
        return FALSE;
    }

    stpMatchCnt->inMeshMaking = 0;
    stpMatchCnt->lastStateBeforeStopMeshMaking = stpMatchCnt->state;

    DWC_Printf(DWC_REPORTFLAG_INFO, "temp new node info validity(%d)\n", stpMatchCnt->tempNewNodeInfo.profileID != 0);

    if (stpMatchCnt->tempNewNodeInfo.profileID != 0)
    {
        DWC_Printf(DWC_REPORTFLAG_INFO, "stop mesh create.(aid:%2d pid:%u)\n", stpMatchCnt->tempNewNodeInfo.aid,
            stpMatchCnt->tempNewNodeInfo.profileID);
        stpMatchCnt->tempNodeInfoBeforeStopMeshMaking = stpMatchCnt->tempNewNodeInfo;
    }
    else
    {
        memset(&stpMatchCnt->tempNodeInfoBeforeStopMeshMaking, 0, sizeof(DWCNodeInfo));
    }

    if (stpMatchCnt->nnInfo.cookie != 0)
    {
        NNCancel(stpMatchCnt->nnInfo.cookie);
        stpMatchCnt->nnInfo.cookie = 0;
        stpMatchCnt->nnRecvCount = 0;
        stpMatchCnt->nnLastCookie = 0;
        stpMatchCnt->nnFailedTime = 0;
    }

    if (stpMatchCnt->state == 3 || stpMatchCnt->state == 5 || stpMatchCnt->state == 6
        || stpMatchCnt->state == 14 || stpMatchCnt->state == 15)
    {
        if (stpMatchCnt->reqProfileID != 0)
        {
            if (stpMatchCnt->tempNewNodeInfo.profileID == 0)
            {
                result = 0;
            }
            else
            {
                result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_RESV_CANCEL, stpMatchCnt->tempNewNodeInfo.profileID,
                    stpMatchCnt->tempNewNodeInfo.publicInetAddress, stpMatchCnt->tempNewNodeInfo.publicQueryPort, NULL, 0);
                stpMatchCnt->reqProfileID = 0;
            }

            if (DWCi_HandleMatchCommandError(result))
            {
                memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
                return TRUE;
            }
        }

        stpMatchCnt->qr2IsReserved = 0;
        stpMatchCnt->qr2Reservation = 0;
    }

    memset(&stpMatchCnt->tempNewNodeInfo, 0, sizeof(DWCNodeInfo));
    return TRUE;
}

// .text:0x803390D4 DWCi_IsFullMesh (0x18)
BOOL DWCi_IsFullMesh(void)
{
    return stpMatchCnt->topologyType == 2;
}

// .text:0x803390EC DWCi_IsLockedBuddyStatus (0x60)
#define DWC_GP_SSTR_KEY_LOCK "LCK"

int DWCi_IsLockedBuddyStatus(GPBuddyStatus* status)
{
    u8 locked = 0;
    char value[4] = { 0 };

    if (DWC_GetCommonValueString(DWC_GP_SSTR_KEY_LOCK, value, status->statusString, '/') > 0)
    {
        locked = strtoul(value, NULL, 10);
    }
    return locked;
}

// CF emits the state-name strings after DWCi_StopMeshMaking's, so the table is defined here
const char* match_st_string[DWC_MATCH_STATE_NUM] = {
    "DWC_MATCH_STATE_INIT",
    "DWC_MATCH_STATE_CL_WAITING",
    "DWC_MATCH_STATE_CL_SEARCH_HOST",
    "DWC_MATCH_STATE_CL_WAIT_RESV",
    "DWC_MATCH_STATE_CL_SEARCH_EVAL_HOST",
    "DWC_MATCH_STATE_CL_NN",
    "DWC_MATCH_STATE_CL_GT2",
    "DWC_MATCH_STATE_CL_CANCEL_SYN",
    "DWC_MATCH_STATE_CL_SYN",
    "DWC_MATCH_STATE_CL_SVDOWN_1",
    "DWC_MATCH_STATE_CL_SVDOWN_2",
    "DWC_MATCH_STATE_CL_SVDOWN_3",
    "DWC_MATCH_STATE_CL_SEARCH_GROUPID_HOST",
    "DWC_MATCH_STATE_SV_WAITING",
    "DWC_MATCH_STATE_SV_OWN_NN",
    "DWC_MATCH_STATE_SV_OWN_GT2",
    "DWC_MATCH_STATE_SV_WAIT_CL_LINK",
    "DWC_MATCH_STATE_SV_CANCEL_SYN",
    "DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT",
    "DWC_MATCH_STATE_SV_SYN",
    "DWC_MATCH_STATE_SV_SYN_WAIT",
    "DWC_MATCH_STATE_WAIT_CLOSE",
    "DWC_MATCH_STATE_SEARCH_OWN",
    "DWC_MATCH_STATE_SV_SYN_CLOSE_WAIT",
};

// ---- older-source functions CF inlines or strips ----
// .old:DWC_StopSCMatchingAsync
BOOL DWC_StopSCMatchingAsync(DWCStopSCCallback callback, void* param)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    if (DWCi_IsError() || stpMatchCnt == NULL
        || stpMatchCnt->qr2MatchTypeExt != 2
        || (DWC_GetState() != DWC_STATE_MATCHING
            && DWC_GetState() != DWC_STATE_CONNECTED))
    {
        return FALSE;
    }

    if (stpMatchCnt->state == 10)
    {
        stpMatchCnt->qr2NumEntry = stpMatchCnt->connectedOnceBefore;
        DWCi_GPSetServerStatus(1 /* TESTARG */);
        if (callback != NULL)
        {
            callback(param);
        }
    }
    else
    {
        stpMatchCnt->stopSCCallback = callback;
        stpMatchCnt->stopSCParam = param;
        stpMatchCnt->needServerPoll = 1;
    }
    return TRUE;
}
#endif

// .old:DWC_GetMatchIntValue
int DWC_GetMatchIntValue(int index, const char* keyString, int idefault)
{
    SBServer server;

    if (stpMatchCnt == NULL || DWCi_IsError())
    {
        return idefault;
    }
    server = ServerBrowserGetServer(stpMatchCnt->sbObj, index);
    if (server != NULL)
    {
        return SBServerGetIntValueA(server, keyString, idefault);
    }
    return idefault;
}

// .old:DWC_GetMatchStringValue
const char* DWC_GetMatchStringValue(
    int index, const char* keyString, const char* sdefault)
{
    SBServer server;

    if (!DWCi_GetMatchCnt() || DWCi_IsError())
    {
        return sdefault;
    }

    server = ServerBrowserGetServer(DWCi_GetMatchCnt()->sbObj, index);
    if (!server)
    {
        return sdefault;
    }

    return SBServerGetStringValue(server, keyString, sdefault);
}

// .old:DWC_SetMatchingOption
int DWC_SetMatchingOption(
    DWCMatchOptType opttype, const void* optval, int optlen)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
#pragma unused(optlen)
    DWC_Printf(4, "!!DWC_SetMatchingOption() was called!! type %d\n", opttype);

    if (stpMatchCnt == NULL)
    {
        return 1;
    }
    if (optval == NULL)
    {
        return 3;
    }

    switch (opttype)
    {
    case DWC_MATCH_OPTION_MIN_COMPLETE:
        if (stpMatchCnt->state == 19)
        {
            return 1;
        }
        if (((const DWCMatchOptMinComplete*)optval)->valid != 0)
        {
            if (((const DWCMatchOptMinComplete*)optval)->minEntry <= 1)
            {
                return 3;
            }
        }
        if (stpOptMinComp == NULL)
        {
            stpOptMinComp
                = DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(*stpOptMinComp));
            if (stpOptMinComp == NULL)
            {
                return 4;
            }
        }
        stpOptMinComp->valid
            = ((const DWCMatchOptMinComplete*)optval)->valid;
        stpOptMinComp->minEntry
            = ((const DWCMatchOptMinComplete*)optval)->minEntry;
        stpOptMinComp->retry = 0;
        stpOptMinComp->pad = 0;
        stpOptMinComp->timeout
            = ((const DWCMatchOptMinComplete*)optval)->timeout;
        stpOptMinComp->recvBit = 0;
        stpOptMinComp->timeoutBit = 0;
        stpOptMinComp->startTime = OSGetTime();
        stpOptMinComp->lastPollTime = OSGetTime();
        return 0;

    case DWC_MATCH_OPTION_SC_CONNECT_BLOCK:
        stOptSCBlock.valid
            = ((const DWCMatchOptSCConnectBlock*)optval)->valid != 0;
        stOptSCBlock.lock = 0;
        return 0;

    default:
        return 2;
    }
}
#endif

// .old:DWC_GetMatchingOption
int DWC_GetMatchingOption(
    DWCMatchOptType opttype, void* optval, int* optlen)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    if (!DWCi_GetMatchCnt())
    {
        return 1;
    }
    if (!optval)
    {
        return 3;
    }

    switch (opttype)
    {
    case DWC_MATCH_OPTION_MIN_COMPLETE:
        if (!stpOptMinComp)
        {
            if (optlen)
            {
                *optlen = 0;
            }
        }
        else
        {
            DWCMatchOptMinComplete* pVal = optval;
            pVal->valid = stpOptMinComp->valid;
            pVal->minEntry = stpOptMinComp->minEntry;
            pVal->timeout = stpOptMinComp->timeout;
            pVal->pad[0] = pVal->pad[1] = 0;
            if (optlen)
            {
                *optlen = sizeof(DWCMatchOptMinComplete);
            }
        }
        return 0;

    case DWC_MATCH_OPTION_SC_CONNECT_BLOCK:
        if (stOptSCBlock.valid == 1)
        {
            *(BOOL*)optval = TRUE;
        }
        else
        {
            *(BOOL*)optval = FALSE;
        }
        if (optlen)
        {
            *optlen = sizeof(BOOL);
        }
        return 0;

    default:
        return 2;
    }
}
#endif

// .old:DWC_GetMOMinCompState
int DWC_GetMOMinCompState(u64* time)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u64 passTime;

    if (!stpOptMinComp || !stpOptMinComp->valid)
    {
        return -1;
    }

    passTime = DWCi_Np_TicksToMilliSeconds(
        DWCi_Np_GetTick() - stpOptMinComp->startTime);
    if (time)
    {
        *time = passTime;
    }
    if (passTime >= stpOptMinComp->timeout)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

// .old:DWC_GetMOSCConnectBlockState
BOOL DWC_GetMOSCConnectBlockState(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    if (stOptSCBlock.valid == 0 || stOptSCBlock.lock == 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif

// .old:DWC_ClearMOSCConnectBlock
void DWC_ClearMOSCConnectBlock(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    stOptSCBlock.lock = 0;

    DWC_Printf(
        DWC_REPORTFLAG_DEBUG, "[OPT_SC_BLOCK] ClearMOSCConnectBlock\n");
}
#endif

// .old:DWC_GetMatchingState
DWCMatchState DWC_GetMatchingState(void)
{
    if (DWCi_GetMatchCnt())
    {
        return DWCi_GetMatchCnt()->state;
    }
    else
    {
        return DWC_MATCH_STATE_INIT;
    }
}

// .old:DWCi_ConnectToFriendsAsync
void DWCi_ConnectToFriendsAsync(const u8 friendIdxList[],
    int friendIdxListLen, u8 numEntry, BOOL distantFriend,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    char valueStr[12], keyValueStr[32];
    int result;
    GPResult gpResult;

    DWCi_SetMatchCommonParam(
        DWC_MATCH_TYPE_FRIEND, numEntry, matchedCallback, matchedParam);

    DWCi_GetMatchCnt()->distantFriend = (u8)(distantFriend ? 1 : 0);
    DWCi_GetMatchCnt()->connectAttemptCallback = evalCallback;
    DWCi_GetMatchCnt()->connectAttemptParam = evalParam;

    DWCi_Np_CpuCopy8(friendIdxList, DWCi_GetMatchCnt()->friendIdxList,
        (u32)friendIdxListLen);
    DWCi_GetMatchCnt()->friendIdxListLen = friendIdxListLen;

    if (!friendIdxListLen
        || (friendIdxListLen < numEntry && !distantFriend
            && (!stpOptMinComp || !stpOptMinComp->valid
                || friendIdxListLen < stpOptMinComp->minEntry - 1)))
    {
        DWCi_StopMatching(DWC_ERROR_FRIENDS_SHORTAGE, 0);
        return;
    }

    DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);

    if (!DWCi_GetMatchCnt()->sbObj)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNew(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, QVERSION_QR2, SBFalse,
            DWCi_SBCallback, NULL);
    }
    if (!DWCi_GetMatchCnt()->sbObj)
    {
        if (DWCi_HandleSBError(sbe_allocerror))
        {
            return;
        }
    }

    (void)snprintf(valueStr, sizeof(valueStr), "%u", numEntry);
    (void)DWC_SetCommonKeyValueString(
        DWC_GP_SSTR_KEY_MATCH_FRIEND_NUM, valueStr, keyValueStr, '/');
    (void)DWC_AddCommonKeyValueString(DWC_GP_SSTR_KEY_DISTANT_FRIEND,
        distantFriend ? "Y" : "N", keyValueStr, '/');

    (void)snprintf(valueStr, sizeof(valueStr), "%u", DWC_MATCHING_VERSION);
    (void)DWC_AddCommonKeyValueString(
        DWC_GP_SSTR_KEY_MATCH_VERSION, valueStr, keyValueStr, '/');

    gpResult = DWCi_SetGPStatus(DWC_STATUS_MATCH_FRIEND, keyValueStr, NULL);
    if (DWCi_HandleGPError(gpResult))
    {
        return;
    }

    if (DWCi_GetMatchCnt()->qr2Obj == NULL)
    {
        if (DWCi_QR2Startup(DWCi_GetMatchCnt()->profileID))
        {
            return;
        }
    }

    result = DWCi_SendResvCommandToFriend(FALSE, TRUE, 0);
    if (DWCi_HandleMatchCommandError(result))
    {
        return;
    }

    DWCi_InitOptMinCompParam(FALSE);
}
#endif

// .old:DWCi_SendStateChanged
static void DWCi_SendStateChanged(qr2_t qr2Obj)
{
    if (qr2Obj == NULL)
        return;
    qr2_think(qr2Obj);
    if (qr2Obj->userstatechangerequested) {
        return;
    }
    switch ((int)DWCi_GetMatchCnt()->qr2MatchType) {
    case DWC_MATCH_TYPE_ANYBODY:
    case DWC_MATCH_TYPE_SC_SV:
    case DWC_MATCH_TYPE_SC_CL:
        switch (DWCi_GetMatchCnt()->state) {
        case DWC_MATCH_STATE_CL_SEARCH_HOST:
            qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
            break;
        }
        break;
    }
}

// .old:DWCi_StopResendingMatchCommand
static void DWCi_StopResendingMatchCommand(void)
{
    DWCi_GetMatchCnt()->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
    DWCi_GetMatchCnt()->cmdCnt.count = 0;
}

// .old:DWCi_DeleteHostByProfileID
BOOL DWCi_DeleteHostByProfileID(int profileID, int numHost)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int i;

    if (!DWCi_GetMatchCnt())
        return FALSE;

    for (i = 0; i < numHost; i++)
    {
        if (DWCi_GetMatchCnt()->sbPidList[i] == profileID)
        {
            DWCi_DeleteHostByIndex(i, numHost);
            return TRUE;
        }
    }

    return FALSE;
}
#endif

// .old:DWCi_DeleteHostByIndex
int DWCi_DeleteHostByIndex(int index, int numHost)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int profileID;

    if (!DWCi_GetMatchCnt())
        return 0;

    profileID = DWCi_GetMatchCnt()->sbPidList[index];
    DWCi_GetMatchCnt()->validAidBitmap
        &= ~(1 << DWCi_GetMatchCnt()->aidList[index]);
    DWCi_SetNumValidConnection();

    if (index < numHost - 1)
    {
        int i;
        for (i = 0; i < numHost - index - 1; ++i)
        {
            DWCi_GetMatchCnt()->qr2IPList[index + i]
                = DWCi_GetMatchCnt()->qr2IPList[index + i + 1];
            DWCi_GetMatchCnt()->qr2PortList[index + i]
                = DWCi_GetMatchCnt()->qr2PortList[index + i + 1];
            DWCi_GetMatchCnt()->sbPidList[index + i]
                = DWCi_GetMatchCnt()->sbPidList[index + i + 1];
            DWCi_GetMatchCnt()->ipList[index + i]
                = DWCi_GetMatchCnt()->ipList[index + i + 1];
            DWCi_GetMatchCnt()->portList[index + i]
                = DWCi_GetMatchCnt()->portList[index + i + 1];
            DWCi_GetMatchCnt()->aidList[index + i]
                = DWCi_GetMatchCnt()->aidList[index + i + 1];
        }
    }

    if (numHost > 0)
    {
        DWCi_GetMatchCnt()->qr2IPList[numHost - 1] = 0;
        DWCi_GetMatchCnt()->qr2PortList[numHost - 1] = 0;
        DWCi_GetMatchCnt()->sbPidList[numHost - 1] = 0;
        DWCi_GetMatchCnt()->ipList[numHost - 1] = 0;
        DWCi_GetMatchCnt()->portList[numHost - 1] = 0;
        DWCi_GetMatchCnt()->aidList[numHost - 1] = 0;
    }

    return profileID;
}
#endif

// .old:DWCi_GetNumAllConnection
int DWCi_GetNumAllConnection(void)
{
    if (!DWCi_GetMatchCnt())
        return 0;
    return DWCi_GetMatchCnt()->connectedOnceBefore;
}

// .old:DWCi_GetNumValidConnection
int DWCi_GetNumValidConnection(void)
{
    if (!DWCi_GetMatchCnt())
        return 0;
    return DWCi_GetMatchCnt()->gt2NumValidConn;
}

// .old:DWCi_SetNumValidConnection
void DWCi_SetNumValidConnection(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int count = -1;
    int i;

    for (i = 0; i < DWC_MAX_CONNECTIONS; i++)
    {
        if (DWCi_GetMatchCnt()->validAidBitmap & (1 << i))
            count++;
    }

    if (count == -1)
        DWCi_GetMatchCnt()->gt2NumValidConn = 0;
    else
        DWCi_GetMatchCnt()->gt2NumValidConn = (u8)count;
}
#endif

// .old:DWCi_GetAllAIDList
int DWCi_GetAllAIDList(u8** aidList)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    if (stpMatchCnt == NULL)
    {
        return 0;
    }
    *aidList = stpMatchCnt->aidList;
    return stpMatchCnt->connectedOnceBefore + 1;
}
#endif

// .old:DWCi_GetValidAIDList
int DWCi_GetValidAIDList(u8** aidList)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    static u8 validAidList[DWC_MAX_CONNECTIONS];
    int i;

    if (stpMatchCnt == NULL)
    {
        return 0;
    }

    memset(validAidList, 0, sizeof(validAidList));
    for (i = 0; i <= stpMatchCnt->gt2NumValidConn; i++)
    {
        if (!(stpMatchCnt->validAidBitmap
                & (1 << stpMatchCnt->aidList[i])))
        {
            break;
        }
        validAidList[i] = stpMatchCnt->aidList[i];
    }
    *aidList = validAidList;
    return stpMatchCnt->gt2NumValidConn + 1;
}
#endif

// .old:DWCi_GetDefaultMatchFilter (replaced by the CF version next to DWCi_ConnectToAnybodyAsync)
#if 0
static int DWCi_GetDefaultMatchFilter(
    char* filter, int profileID, u8 numEntry, u8 matchType)
{
    return snprintf(filter, MAX_FILTER_LEN,
        "%s = %d and %s != %u and maxplayers = %d and numplayers < %d and %s "
        "= %d and %s != %s",
        DWC_QR2_MATCH_VER_KEY_STR, 90,
        DWC_QR2_PID_KEY_STR, profileID, numEntry, numEntry,
        DWC_QR2_MATCH_TYPE_KEY_STR, matchType,
        DWC_QR2_MATCH_RESV_KEY_STR, DWC_QR2_PID_KEY_STR);
}
#endif

// .old:IsPrivateAddr
static int IsPrivateAddr(u32 addr_in_netorder)
{
    u32 addr_in_hostorder;
    u8 addr[4];

    addr_in_hostorder = SONtoHl(addr_in_netorder);
    addr[0] = (u8)((addr_in_hostorder & 0xff000000) >> 24);
    addr[1] = (u8)((addr_in_hostorder & 0x00ff0000) >> 16);
    addr[2] = (u8)((addr_in_hostorder & 0x0000ff00) >> 8);
    addr[3] = (u8)(addr_in_hostorder & 0x000000ff);

    if (addr[0] == 10)
        return TRUE;
    if (addr[0] == 172 && addr[1] >= 16 && addr[1] <= 31)
        return TRUE;
    if (addr[0] == 192 && addr[1] == 168)
        return TRUE;
    return FALSE;
}

// .old:DWCi_DoNatNegotiationAsync
static NegotiateError DWCi_DoNatNegotiationAsync(DWCNNInfo* nnInfo)
{
    SBError sbError;
    NegotiateError nnError;
    int i;

    if (!nnInfo->isQR2)
    {
        sbError = ServerBrowserSendNatNegotiateCookieToServer(
            DWCi_GetMatchCnt()->sbObj,
            gt2AddressToString(nnInfo->ip, 0, NULL), nnInfo->port,
            nnInfo->cookie);
        if (DWCi_HandleSBError(sbError))
            return ne_socketerror;
        DWC_Printf(0x40, "Send NN cookie = %x.\n", nnInfo->cookie);
    }

    for (i = 0; i < DWC_MATCH_DNS_RETRY_MAX; ++i)
    {
        nnError = NNBeginNegotiationWithSocket(
            gt2GetSocketSOCKET(*DWCi_GetMatchCnt()->pGt2Socket),
            nnInfo->cookie, nnInfo->isQR2, DWCi_NNProgressCallback,
            DWCi_NNCompletedCallback, nnInfo);
        if (nnError == ne_noerror || nnError != ne_dnserror)
            break;
        DWC_Printf(4,
            " dns error occurs when NatNegotiation begin... retry\n");
    }

    return nnError;
}



// .old:DWCi_GetGPBuddyAdditionalMsg
static int DWCi_GetGPBuddyAdditionalMsg(
    char* dstMsg, const char* srcMsg, int index)
{
    const char* pSrcBegin = srcMsg;
    char* pSrcNext = NULL;
    char* pSrcEnd;
    int len;
    int i;

    pSrcEnd = strchr(pSrcBegin, '\0');

    for (i = 0; i < index; i++)
    {
        pSrcNext = strchr(pSrcBegin, '/');
        if (pSrcNext == NULL)
        {
            return -1;
        }
        pSrcBegin = pSrcNext + 1;
    }

    pSrcNext = strchr(pSrcBegin, '/');
    if (pSrcNext == NULL)
    {
        pSrcNext = pSrcEnd;
    }
    if (pSrcBegin == pSrcNext)
    {
        return -1;
    }
    len = pSrcNext - pSrcBegin;
    DWCi_Np_CpuCopy8(pSrcBegin, dstMsg, len);
    dstMsg[len] = '\0';
    return len;
}

// .old:DWCi_MakeBackupServerData
static void DWCi_MakeBackupServerData(int profileID, const u32 data[])
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int i;
    int len;

    // a word loop
    len = (int)data[0];
    for (i = 0; i < len; i++)
    {
        DWCi_GetMatchCnt()->svDataBak[i + 2] = data[i + 1];
    }

    DWCi_GetMatchCnt()->svDataBak[0] = len + 1;
    DWCi_GetMatchCnt()->svDataBak[1] = (u32)profileID;
}
#endif


// .old:DWCi_CancelReservation
static int DWCi_CancelReservation(int profileID)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int result;

    result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_RESV_CANCEL, profileID,
        DWCi_GetMatchCnt()->qr2IPList[0], DWCi_GetMatchCnt()->qr2PortList[0], NULL, 0);

    DWCi_GetMatchCnt()->reqProfileID = 0;

    return result;
}
#endif

// .old:DWCi_ChangeToClient
// defined here (function and string order), not inlined
static int DWCi_ChangeToClient(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u32 leData[128];
    const u32* senddata;
    int result;
    int i;
    int len;

    // the server data goes little-endian to anybody-match clients (SB messages)
    if (stpMatchCnt->qr2MatchTypeExt == DWC_MATCH_TYPE_ANYBODY)
    {
        senddata = leData;
        len = (int)stpMatchCnt->svDataBak[0] + 1;
        for (i = 0; i < len; i++)
        {
            leData[i] = DWCi_HtoLEl(stpMatchCnt->svDataBak[i]);
        }
    }
    else
    {
        senddata = stpMatchCnt->svDataBak;
    }

    for (i = 1; i <= stpMatchCnt->connectedOnceBefore; i++)
    {
        result = DWCi_SendMatchCommand(10, stpMatchCnt->sbPidList[i],
            stpMatchCnt->qr2IPList[i], stpMatchCnt->qr2PortList[i],
            senddata, stpMatchCnt->svDataBak[0] + 1);
        if (result != 0)
        {
            return result;
        }
    }

    stpMatchCnt->qr2IsReserved = 0;
    stpMatchCnt->qr2Reservation = 0;
    stpMatchCnt->closeState = 1;
    gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
    stpMatchCnt->closeState = 0;
    DWC_Printf(0x40, "Closed all connections. Begin NN to %u\n",
        stpMatchCnt->reqProfileID);

    return 0;
}
#endif


// .old:DWCi_InvalidateReservation
static int DWCi_InvalidateReservation(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u32 senddata = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_CANCEL, 0);
    int result = 0;

    if (DWCi_GetMatchCnt()->qr2IsReserved
        && DWCi_GetMatchCnt()->qr2Reservation
        && (DWCi_GetMatchCnt()->qr2Reservation
            != DWCi_GetMatchCnt()->profileID))
    {
        result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_CANCEL,
            DWCi_GetMatchCnt()->qr2Reservation,
            DWCi_GetMatchCnt()->qr2IPList[
                DWCi_GetMatchCnt()->connectedOnceBefore + 1],
            DWCi_GetMatchCnt()->qr2PortList[
                DWCi_GetMatchCnt()->connectedOnceBefore + 1],
            &senddata, 1);

        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
    }

    return result;
}
#endif

// .old:DWCi_RestartFromTimeout
static void DWCi_RestartFromTimeout(void)
{
    if (DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV
        || DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "DWCi_RestartFromTimeout() shouldn't be called.\n");
        return;
    }

    DWCi_CloseAllConnectionsByTimeout();

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
        "Closed all connections and restart matching.\n");

    DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
}

// .old:DWCi_ResumeMatching
static int DWCi_ResumeMatching(void)
{
    int result;

    stpMatchCnt->qr2IsReserved = 0;
    stpMatchCnt->qr2Reservation = 0;
    stpMatchCnt->closeState = 0;

    if (stpMatchCnt->inMeshMaking != 0)
    {
        if (stpMatchCnt->qr2MatchTypeExt == 0)
        {
            stpMatchCnt->state = 3;
            result = DWCi_SBUpdateAsync(0);
            if (DWCi_HandleSBError(result) != 0)
            {
                return result;
            }
        }
        else if (stpMatchCnt->qr2MatchTypeExt == 1)
        {
            int error;

            stpMatchCnt->state = 4;
            result = DWCi_SendResvCommand(stpMatchCnt->inMeshMaking, FALSE);
            if (stpMatchCnt->qr2MatchTypeExt == 0)
            {
                error = DWCi_HandleSBError(result);
            }
            else
            {
                error = DWCi_HandleGPError(result);
            }
            if (error != 0)
            {
                return result;
            }
        }
    }
    else
    {
        DWCi_RestartFromCancel(1);
    }
    return 0;
}

// .old:DWCi_CloseCancelHostAsync
static BOOL DWCi_CloseCancelHostAsync(int profileID)
{
    BOOL ret;
    GT2Connection* connection;

    DWCi_GetMatchCnt()->cbEventPid = profileID;
    connection = DWCi_GetGT2ConnectionByProfileID(
        profileID, DWCi_GetMatchCnt()->connectedOnceBefore + 1);

    if (connection)
    {
        DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_TIMEOUT;
        gt2CloseConnectionHard(*connection);
        DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
        ret = TRUE;
    }
    else
    {
        (void)DWCi_DeleteHostByProfileID(
            profileID, DWCi_GetMatchCnt()->connectedOnceBefore + 1);
        ret = FALSE;
    }

    return ret;
}


// .old:DWCi_CloseAllConnectionsByTimeout
static void DWCi_CloseAllConnectionsByTimeout(void)
{
    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_TIMEOUT;
    gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
}

// .old:DWCi_GetNewMatchKey
static u8 DWCi_GetNewMatchKey(void)
{
    u8 i;

    for (i = 0; i < DWC_QR2_GAME_RESERVED_KEYS; i++)
    {
        if (!stGameMatchKeys[i].keyID)
        {
            return (u8)(DWC_QR2_GAME_KEY_START + i);
        }
    }

    return 0;
}

// .old:DWCi_ClearGameMatchKeys
static void DWCi_ClearGameMatchKeys(void)
{
    int i;

    for (i = 0; i < 154; i++)
    {
        if (stGameMatchKeys[i].keyStr != NULL)
        {
            DWC_Free(DWC_ALLOCTYPE_BASE, stGameMatchKeys[i].keyStr, 0);
        }
    }
    memset(stGameMatchKeys, 0, sizeof(stGameMatchKeys));
}

// .old:DWCi_GetAIDFromList
static u8 DWCi_GetAIDFromList(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u8 i;
    int j;

    for (i = 0; i < DWC_MAX_CONNECTIONS; i++)
    {
        for (j = 0; j <= DWCi_GetMatchCnt()->qr2MatchType; j++)
        {
            if (DWCi_GetMatchCnt()->aidList[j] == i)
            {
                break;
            }
        }
        if (j > DWCi_GetMatchCnt()->qr2MatchType)
        {
            break;
        }
    }

    return i;
}
#endif

// .old:DWCi_IsFriendByIdxList
static BOOL DWCi_IsFriendByIdxList(int profileID)
{
    int listProfileID;
    int i;

    if (DWCi_GetMatchCnt()->friendList == NULL)
        return FALSE;

    for (i = 0; i < DWCi_GetMatchCnt()->friendIdxListLen; i++)
    {
        if (((listProfileID = DWCi_GetProfileIDFromList(
                  DWCi_GetMatchCnt()->friendIdxList[i]))
                > 0)
            && (listProfileID == profileID))
        {
            return TRUE;
        }
    }

    return FALSE;
}

// .old:DWCi_AreAllBuddies
static BOOL DWCi_AreAllBuddies(const u32 pidList[], u32 pidListLen)
{
    int i;

    if (DWCi_GetMatchCnt()->distantFriend
        && (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_WAIT_RESV))
    {
        return TRUE;
    }

    for (i = 0; i < pidListLen; i++)
    {
        if (!DWCi_IsFriendByIdxList((int)DWCi_LEtoHl(pidList[i])))
        {
            return FALSE;
        }

        if (DWCi_GetMatchCnt()->distantFriend
            && (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_WAITING))
        {
            return TRUE;
        }
    }

    return TRUE;
}

// .old:DWCi_GetAIDFromProfileID
static u8 DWCi_GetAIDFromProfileID(int profileID, BOOL idx0)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    int i;

    for (i = (idx0 ? 0 : 1); i <= DWCi_GetMatchCnt()->connectedOnceBefore; i++)
    {
        if (DWCi_GetMatchCnt()->sbPidList[i] == profileID)
        {
            return DWCi_GetMatchCnt()->aidList[i];
        }
    }

    return 0xff;
}
#endif

// .old:DWCi_GetAIDBitmask
static u32 DWCi_GetAIDBitmask(BOOL valid)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u32 bitmask = 0;
    int i;

    if (valid)
    {
        return DWCi_GetMatchCnt()->validAidBitmap & ~1;
    }
    else
    {
        for (i = 1; i <= DWCi_GetMatchCnt()->connectedOnceBefore; i++)
        {
            bitmask |= 1 << DWCi_GetMatchCnt()->aidList[i];
        }
        return bitmask;
    }
}
#endif

// .old:DWCi_InitClWaitTimeout
static void DWCi_InitClWaitTimeout(void)
{
    DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
    DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
}

// .old:DWCi_InitOptMinCompParam
static void DWCi_InitOptMinCompParam(BOOL reset)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    if (stpOptMinComp && stpOptMinComp->valid)
    {
        stpOptMinComp->recvBit = 0;
        stpOptMinComp->timeoutBit = 0;
        stpOptMinComp->retry = 0;
        stpOptMinComp->lastPollTime = DWCi_Np_GetTick();
        if (!reset)
        {
            stpOptMinComp->startTime = DWCi_Np_GetTick();
        }
    }
}
#endif

// .old:DWCi_ProcessOptMinComp
static void DWCi_ProcessOptMinComp(void)
{
    // TODO: CF revision differs (fields moved/removed); older body kept below
}
#if 0
{
    u32 bitmask;
    int result;
    int i;

    if (!stpOptMinComp || !stpOptMinComp->valid
        || DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_SV
        || DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL)
    {
        return;
    }

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_POLL_TIMEOUT)
    {
        bitmask = DWCi_GetAIDBitmask(FALSE);

        if (stpOptMinComp->recvBit == bitmask)
        {
            if (stpOptMinComp->timeoutBit == bitmask)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "[OPT_MIN_COMP] Timeout occured in all hosts.\n");

                DWCi_GetMatchCnt()->qr2NumEntry
                    = (u8)DWCi_GetMatchCnt()->connectedOnceBefore;
                DWCi_GetMatchCnt()->clLinkProgress
                    = (u8)(DWCi_GetMatchCnt()->connectedOnceBefore - 1);

                DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "[OPT_MIN_COMP] Some clients is in time.\n");

                stpOptMinComp->lastPollTime = DWCi_Np_GetTick();
                stpOptMinComp->recvBit = 0;

                if (DWCi_GetMatchCnt()->qr2MatchTypeExt
                    == DWC_MATCH_TYPE_ANYBODY)
                {
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);
                    DWCi_GetMatchCnt()->sbUpdateFlag
                        = DWC_SB_UPDATE_INTERVAL_LONG;
                    DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
                }
                else
                {
                    DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
                    (void)DWCi_SendResvCommandToFriend(TRUE, FALSE, 0);
                }
            }
        }
        else if (DWCi_Np_TicksToMilliSeconds(
                     DWCi_Np_GetTick() - stpOptMinComp->lastPollTime)
            >= DWC_MATCH_CMD_RTT_TIMEOUT * stpOptMinComp->retry)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "[OPT_MIN_COMP] Timeout: wait poll-ACK %d/%d.\n",
                stpOptMinComp->retry - 1,
                DWC_MATCH_OPT_MIN_COMP_POLL_RETRY_MAX);

            if (stpOptMinComp->retry
                > DWC_MATCH_OPT_MIN_COMP_POLL_RETRY_MAX)
            {
                DWC_Printf(DWC_REPORTFLAG_DEBUG,
                    "[OPT_MIN_COMP] Timeout: aidbitmap 0x%x. "
                    "Restart matching.\n",
                    stpOptMinComp->recvBit);

                DWCi_InitOptMinCompParam(TRUE);

                DWCi_CloseAllConnectionsByTimeout();
                DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
            }
            else
            {
                for (i = 1; i <= DWCi_GetMatchCnt()->connectedOnceBefore; i++)
                {
                    if (!(stpOptMinComp->recvBit
                            & (1 << DWCi_GetMatchCnt()->aidList[i])))
                    {
                        result = DWCi_SendMatchCommand(
                            DWC_MATCH_COMMAND_POLL_TIMEOUT,
                            DWCi_GetMatchCnt()->sbPidList[i],
                            DWCi_GetMatchCnt()->qr2IPList[i],
                            DWCi_GetMatchCnt()->qr2PortList[i], NULL, 0);
                        if (DWCi_HandleMatchCommandError(result))
                        {
                            return;
                        }
                    }
                }
                stpOptMinComp->retry++;
            }
        }
    }
    else if ((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_WAIT_RESV
                 || DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_SEARCH_HOST)
        && DWCi_GetMatchCnt()->connectedOnceBefore >= stpOptMinComp->minEntry - 1
        && ((!stpOptMinComp->retry
                && DWCi_Np_TicksToMilliSeconds(
                       DWCi_Np_GetTick() - stpOptMinComp->startTime)
                    >= stpOptMinComp->timeout)
            || (stpOptMinComp->retry
                && DWCi_Np_TicksToMilliSeconds(
                       DWCi_Np_GetTick() - stpOptMinComp->lastPollTime)
                    >= stpOptMinComp->timeout >> 2)))
    {
        if (DWCi_GetMatchCnt()->reqProfileID)
        {
            result = DWCi_CancelReservation(DWCi_GetMatchCnt()->reqProfileID);
            if (DWCi_HandleMatchCommandError(result))
            {
                return;
            }
        }

        DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_POLL_TIMEOUT);

        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "[OPT_MIN_COMP] Poll timeout (my time is %lu).\n",
            DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - stpOptMinComp->startTime));

        for (i = 1; i <= DWCi_GetMatchCnt()->connectedOnceBefore; i++)
        {
            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_POLL_TIMEOUT,
                DWCi_GetMatchCnt()->sbPidList[i],
                DWCi_GetMatchCnt()->qr2IPList[i],
                DWCi_GetMatchCnt()->qr2PortList[i], NULL, 0);
            if (DWCi_HandleMatchCommandError(result))
            {
                return;
            }
        }

        stpOptMinComp->lastPollTime = DWCi_Np_GetTick();
        stpOptMinComp->retry = 1;
    }
}
#endif

// .old:DWCi_HandleNNResult
static NegotiateResult DWCi_HandleNNResult(NegotiateResult result)
{
    DWCError type;
    int code;
    if (result == nr_success) {
        return nr_success;
    }
    DWC_Printf(DWC_REPORTFLAG_WARNING, "Match, NN result %d\n", result);
    switch (result)
    {
    case nr_deadbeatpartner:
        return nr_deadbeatpartner;
    case nr_inittimeout:
        return nr_inittimeout;
    case nr_pingtimeout:
        return nr_pingtimeout;
    default:
        type = DWC_ERROR_NETWORK;
        code = DWC_ECODE_TYPE_UNEXPECTED;
        break;
    }
    if (type != 0) {
        DWCi_StopMatching(type, code + DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_NN);
    }
    return result;
}

// .old:DWCi_ProcessNNFailure
static BOOL DWCi_ProcessNNFailure(BOOL ignoreError)
{
    if (ignoreError)
    {
        return TRUE;
    }

    if (DWCi_GetMatchCnt()->qr2MatchTypeExt != DWC_MATCH_TYPE_SC_CL)
    {
        DWCi_GetMatchCnt()->nnFailureCount++;

        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN failure %d/%d.\n", DWCi_GetMatchCnt()->nnFailureCount, DWC_MATCH_NN_FAILURE_MAX);
    }

    if ((DWCi_GetMatchCnt()->qr2MatchTypeExt == DWC_MATCH_TYPE_SC_CL) ||
        DWCi_GetMatchCnt()->connectGroupID ||
        (DWCi_GetMatchCnt()->nnFailureCount >= DWC_MATCH_NN_FAILURE_MAX)) {
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_NN
                + DWC_ECODE_TYPE_MUCH_FAILURE);
        return FALSE;
    }

    return TRUE;
}

// .old:DWCi_GetMatchCnt
static DWCMatchControl* DWCi_GetMatchCnt(void)
{
    return stpMatchCnt;
}
