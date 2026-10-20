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
    DWC_PP_CONNECTION_CL_FINISH_CONNECT,
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

typedef struct DWCstMatchControl
{
    GPConnection* pGpObj;
    GT2Socket* pGt2Socket;
    GT2ConnectionCallbacks* gt2Callbacks;
    u8 gt2ConnectCount;
    u8 gt2NumConnection;
    u8 gt2NumValidConn;
    u8 pad1;
    qr2_t qr2Obj;
    vu8 qr2NNFinishCount;
    vu8 qr2MatchType;
    vu8 qr2NumEntry;
    vu8 qr2IsReserved;
    u8 qr2ShutdownFlag;
    u8 pad2;
    u16 qr2Port;
    u32 qr2IP;
    volatile int qr2Reservation;
    u32 qr2IPList[DWC_MAX_CONNECTIONS];
    u16 qr2PortList[DWC_MAX_CONNECTIONS];
    ServerBrowser sbObj;
    int sbUpdateFlag;
    u8 _EC[4];
    s64 sbUpdateTick;
    int sbPidList[DWC_MAX_CONNECTIONS];
    s64 sbUpdateRequestTick;
    u8 nnRecvCount;
    u8 nnFailureCount;
    u16 nnCookieRand;
    int nnLastCookie;
    s64 nnFailedTime;
    s64 nnFinishTime;
    DWCNNInfo nnInfo;
    DWCMatchState state;
    u8 clLinkProgress;
    u8 friendCount;
    u8 distantFriend;
    u8 resvWaitCount;
    u8 closeState;
    u8 cancelState;
    u8 scResvRetryCount;
    u8 synResendCount;
    u8 cancelSynResendCount;
    u8 clWaitTimeoutCount;
    u8 stopSCFlag;
    u8 pad3;
    u16 baseLatency;
    u16 cancelBaseLatency;
    u16 searchPort;
    u16 pad4;
    u32 searchIP;
    int cmdResendFlag;
    u8 _1C4[4];
    s64 cmdResendTick;
    u32 cmdTimeoutTime;
    u8 _1D4[4];
    s64 cmdTimeoutStartTick;
    u32 synAckBit;
    u32 cancelSynAckBit;
    u32 friendAcceptBit;
    u8 _1EC[4];
    s64 lastSynSent;
    s64 lastCancelSynSent;
    s64 closedTime;
    s64 clWaitTime;
    volatile int profileID;
    int reqProfileID;
    int priorProfileID;
    int cbEventPid;
    u32 ipList[32];
    u16 portList[32];
    u8 aidList[32];
    u32 validAidBitmap;
    const char* gameName;
    const char* secretKey;
    const DWCFriendData* friendList;
    int friendListLen;
    u8 friendIdxList[DWC_MAX_MATCH_IDX_LIST];
    int friendIdxListLen;
    u32 svDataBak[DWC_MAX_CONNECTIONS + 1];
    DWCMatchCommandControl cmdCnt;
    DWCMatchedSCCallback matchedCallback;
    void* matchedParam;
    DWCNewClientCallback newClientCallback;
    void* newClientParam;
    DWCEvalPlayerCallback evalCallback;
    void* evalParam;
    void (*stopSCCallback)(void* param);
    void* stopSCParam;
} DWCMatchControl;

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
static void DWCi_SBPrintServerData(SBServer server);
static int DWCi_EvaluateServers(int sort);
static void DWCi_RandomizeServers(void);

static const int stEvalRate[DWC_SB_UPDATE_MAX_SERVERS] = {
    3,
    3,
    2,
    2,
    1,
    1,
};

void SBServerAddIntKeyValue(SBServer server, const char* keyname, int value);

static void DWCi_DoCancelMatching(void);
void DWCi_StopMatching(DWCError error, int errorCode);
static SBError DWCi_SBUpdateAsync(int profileID);
static int DWCi_GetDefaultMatchFilter(
    char* filter, int profileID, u8 numEntry, u8 matchType);
static void DWCi_ResetMatchParam(DWCMatchResetLevel level);
static void DWCi_SetMatchCommonParam(u8 matchType, u8 numEntry,
    DWCMatchedSCCallback callback, void* param);
static void DWCi_CloseMatching(void);
static GPResult DWCi_HandleGPError(GPResult result);
static SBError DWCi_HandleSBError(SBError error);
static qr2_error_t DWCi_HandleQR2Error(qr2_error_t error);
static int DWCi_HandleMatchCommandError(int error);
static int DWCi_SendResvCommand(int profileID, BOOL delay);
static BOOL DWCi_RetryReserving(int resendPid);
static int DWCi_CancelReservation(int profileID);
static BOOL DWCi_CancelPreConnectedServerProcess(int clientPid);
static BOOL DWCi_CancelPreConnectedClientProcess(int serverPid);
static void DWCi_PostProcessConnection(DWCMatchPpConnectionType type);
static int DWCi_ResumeMatching(void);
static BOOL DWCi_ProcessMatchSynTimeout(void);
static BOOL DWCi_ProcessCancelMatchSynTimeout(void);
static void DWCi_ProcessOptMinComp(void);
static int DWCi_SendMatchCommand(u8 command, int profileID, u32 ip, u16 port,
    const u32 data[], int len);
static SBError DWCi_SendSBMsgCommand(
    u8 command, u32 ip, u16 port, const u32 data[], int len);
static GPResult DWCi_SendGPBuddyMsgCommand(GPConnection* connection,
    u8 command, int profileID, const char* message);
static void DWCi_QR2ServerKeyCallback(
    int keyid, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2PlayerKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2TeamKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata);
static void DWCi_QR2KeyListCallback(
    qr2_key_type keytype, qr2_keybuffer_t keybuffer, void* userdata);
static int DWCi_QR2CountCallback(qr2_key_type keytype, void* userdata);
static void DWCi_QR2AddErrorCallback(
    qr2_error_t error, gsi_char* errmsg, void* userdata);
static void DWCi_QR2PublicAddrCallback(
    unsigned int ip, unsigned short port, void* userdata);
static void DWCi_QR2NatnegCallback(int cookie, void* userdata);
static void DWCi_QR2ClientMsgCallback(
    gsi_char* data, int len, void* userdata);

static void DWCi_SBCallback(ServerBrowser sb, SBCallbackReason reason,
    SBServer server, void* instance);
static void DWCi_NNProgressCallback(NegotiateState state, void* userdata);
static void DWCi_NNCompletedCallback(NegotiateResult result, SOCKET gamesocket,
    struct sockaddr_in* remoteaddr, void* userdata);
static BOOL DWCi_ProcessNNFailure(BOOL ignoreError);

DWCConnectionInfo* DWCi_GetConnectionInfoByIdx(int index);
DWCAccUserData* DWCi_GetUserData(void);
int DWCi_GetProfileIDFromList(int index);
static u8 DWCi_CheckResvCommand(
    int profileID, u32 qr2IP, u16 qr2Port, u32 matchType, BOOL priorFlag);
static int DWCi_ProcessResvOK(int profileID, u32 ip, u16 port);
static void DWCi_MakeBackupServerData(int profileID, const u32 data[]);
static int DWCi_SendResvCommandToFriend(
    BOOL delay, BOOL init, int resendPid);
static BOOL DWCi_ProcessCancelMatchSynCommand(
    int profileID, u8 command, u32 data);
static void DWCi_SendMatchSynPacket(u8 aid, u16 type);
static NegotiateError DWCi_HandleNNError(NegotiateError error);
static NegotiateResult DWCi_HandleNNResult(NegotiateResult result);
static GT2Result DWCi_HandleGT2Error(GT2Result result);
static void DWCi_RestartFromCancel(DWCMatchResetLevel level);

void DWCi_SetNumValidConnection(void);

GPResult DWCi_GPSetServerStatus(void);
static int DWCi_GetGPBuddyAdditionalMsg(
    char* dstMsg, const char* srcMsg, int index);
static BOOL DWCi_ProcessRecvMatchCommand(u8 command, int srcPid, u32 srcIP,
    u16 srcPort, const u32 data[], int len, BOOL hostOrder);
static NegotiateError DWCi_NNStartupAsync(
    int isQR2, int cookie, SBServer server);
static NegotiateError DWCi_DoNatNegotiationAsync(DWCNNInfo* nnInfo);
BOOL DWCi_IsShutdownMatch(void);
static DWCMatchControl* DWCi_GetMatchCnt(void);
static void DWCi_SetMatchStatus(DWCMatchState state);
static void DWCi_StopResendingMatchCommand(void);
static void DWCi_FinishCancelMatching(void);
static void DWCi_RestartFromTimeout(void);
static BOOL DWCi_CloseCancelHostAsync(int profileID);
static void DWCi_CloseAllConnectionsByTimeout(void);
static BOOL DWCi_SendCancelMatchSynCommand(int profileID, u8 command);
static u8 DWCi_GetNewMatchKey(void);
static void DWCi_ClearGameMatchKeys(void);
static u8 DWCi_GetAIDFromList(void);
static BOOL DWCi_IsFriendByIdxList(int profileID);
static BOOL DWCi_AreAllBuddies(const u32 pidList[], u32 pidListLen);
static u8 DWCi_GetAIDFromProfileID(int profileID, BOOL idx0);
static u32 DWCi_GetAIDBitmask(BOOL valid);
static int DWCi_InvalidateReservation(void);
static void DWCi_InitClWaitTimeout(void);
static void DWCi_InitOptMinCompParam(BOOL reset);
static int DWCi_ChangeToClient(void);

// the GameSpy calls of this file are tried once on a DNS error (each loop runs once: QR2Startup, SBUpdateAsync,
// NNStartupAsync, SendSBMsgCommand); dwc_friend.c keeps DWC_DNS_ERROR_RETRY_MAX (5)
#define DWC_MATCH_DNS_RETRY_MAX 1

static char* stpAddFilter = NULL;
static DWCMatchOptMinCompleteIn* stpOptMinComp = NULL;
static int s_sbCallbackLevel = 0;
static BOOL s_needSbFree = FALSE;
// Later DWC only: set around NNThink; a free requested inside its callbacks waits
static BOOL s_nnThinking = FALSE;
static BOOL s_needNnFree = FALSE;
static DWCMatchControl* stpMatchCnt = NULL;
static DWCMatchOptSCBlock stOptSCBlock;

static DWCGameMatchKeyData stGameMatchKeys[DWC_QR2_GAME_RESERVED_KEYS];

// a command's data words travel little-endian in SB / QR2 messages (anybody matching, TELL_ADDR to an SC
// client or a distant friend) and in host order in GP buddy messages, where the IP address is byte-swapped instead
static inline BOOL DWCi_IsLEMatchCommand(u8 command)
{
    if (stpMatchCnt->qr2MatchType == DWC_MATCH_TYPE_ANYBODY
        || ((stpMatchCnt->qr2MatchType == DWC_MATCH_TYPE_SC_CL || stpMatchCnt->distantFriend)
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

BOOL DWC_RegisterMatchingStatus(void)
{
    DWC_Printf(4, "!!DWC_RegisterMatchingStatus() was called!!\n");

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

BOOL DWC_CancelMatching(void)
{
    if (DWC_IsValidCancelMatching())
    {
        DWCi_DoCancelMatching();
        return TRUE;
    }
    DWC_Printf(4, "Now unable to cancel.\n");
    return FALSE;
}

BOOL DWC_CancelMatchingAsync(void)
{
    if (DWC_IsValidCancelMatching())
    {
        DWCi_DoCancelMatching();
        return TRUE;
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Now unable to cancel.\n");
        return FALSE;
    }
}

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

BOOL DWC_StopSCMatchingAsync(DWCStopSCCallback callback, void* param)
{
    if (DWCi_IsError() || stpMatchCnt == NULL
        || stpMatchCnt->qr2MatchType != 2
        || (DWC_GetState() != DWC_STATE_MATCHING
            && DWC_GetState() != DWC_STATE_CONNECTED))
    {
        return FALSE;
    }

    if (stpMatchCnt->state == 10)
    {
        stpMatchCnt->qr2NumEntry = stpMatchCnt->gt2NumConnection;
        DWCi_GPSetServerStatus();
        if (callback != NULL)
        {
            callback(param);
        }
    }
    else
    {
        stpMatchCnt->stopSCCallback = callback;
        stpMatchCnt->stopSCParam = param;
        stpMatchCnt->stopSCFlag = 1;
    }
    return TRUE;
}

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

int DWC_GetLastMatchingType(void)
{
    if (DWCi_GetMatchCnt())
    {
        return DWCi_GetMatchCnt()->qr2MatchType;
    }
    else
    {
        return -1;
    }
}

int DWC_SetMatchingOption(
    DWCMatchOptType opttype, const void* optval, int optlen)
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

int DWC_GetMatchingOption(
    DWCMatchOptType opttype, void* optval, int* optlen)
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

int DWC_GetMOMinCompState(u64* time)
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

BOOL DWC_GetMOSCConnectBlockState(void)
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

void DWC_ClearMOSCConnectBlock(void)
{
    stOptSCBlock.lock = 0;

    DWC_Printf(
        DWC_REPORTFLAG_DEBUG, "[OPT_SC_BLOCK] ClearMOSCConnectBlock\n");
}

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

void DWCi_MatchInit(DWCMatchControl* matchcnt, GPConnection pGpObj,
    GT2Socket* pGt2Socket, GT2ConnectionCallbacks* gt2Callbacks,
    const char* gameName, const char* secretKey,
    const DWCFriendData friendList[], int friendListLen)
{
    stpMatchCnt = matchcnt;
    stpMatchCnt->pGpObj = pGpObj;
    stpMatchCnt->pGt2Socket = pGt2Socket;
    stpMatchCnt->gt2Callbacks = gt2Callbacks;
    stpMatchCnt->qr2Obj = NULL;
    stpMatchCnt->qr2IP = 0;
    stpMatchCnt->qr2Port = 0;
    stpMatchCnt->sbObj = NULL;
    stpMatchCnt->state = 0;
    stpMatchCnt->pad1 = 0;
    stpMatchCnt->pad2 = 0;
    stpMatchCnt->pad3 = 0;
    stpMatchCnt->pad4 = 0;
    stpMatchCnt->friendAcceptBit = 0;
    stpMatchCnt->profileID = 0;
    stpMatchCnt->gameName = gameName;
    stpMatchCnt->secretKey = secretKey;
    stpMatchCnt->friendList = friendList;
    stpMatchCnt->friendListLen = friendListLen;
    memset(stpMatchCnt->friendIdxList, 0, sizeof(stpMatchCnt->friendIdxList));
    stpMatchCnt->friendIdxListLen = 0;
    stpMatchCnt->matchedCallback = NULL;
    stpMatchCnt->matchedParam = NULL;
    stpMatchCnt->evalCallback = NULL;
    stpMatchCnt->evalParam = NULL;
    stpMatchCnt->stopSCCallback = NULL;
    stpMatchCnt->stopSCParam = NULL;
    stpMatchCnt->sbUpdateRequestTick = 0;

    DWCi_ClearGameMatchKeys();

    stOptSCBlock.valid = 0;
    stOptSCBlock.lock = 0;
    stOptSCBlock.pad = 0;

    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);
}

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
    qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);

    return qr2Error;
}

void DWCi_ConnectToAnybodyAsync(u8 numEntry, const char* addFilter,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam)
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
            - DWCi_GetDefaultMatchFilter(filter, -1, 32, 3)
            - strlen(" and ()");
        stpAddFilter = DWC_Alloc(DWC_ALLOCTYPE_BASE, addFilterLenMax);
        if (stpAddFilter == NULL)
        {
            DWCi_StopMatching(DWC_ERROR_FATAL,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
            return;
        }
        DWCi_Np_CpuCopy8(addFilter, stpAddFilter, addFilterLenMax);
        stpAddFilter[addFilterLenMax - 1] = '\0';
    }

    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);
    DWCi_GetMatchCnt()->qr2MatchType = 0;
    DWCi_GetMatchCnt()->qr2NumEntry = numEntry;
    DWCi_GetMatchCnt()->matchedCallback = matchedCallback;
    DWCi_GetMatchCnt()->matchedParam = matchedParam;
    DWCi_GetMatchCnt()->nnFailureCount = 0;
    DWCi_GetMatchCnt()->aidList[0] = 0;
    qr2_register_keyA(0x32, "dwc_pid");
    qr2_register_keyA(0x33, "dwc_mtype");
    qr2_register_keyA(0x34, "dwc_mresv");
    qr2_register_keyA(0x35, "dwc_mver");
    qr2_register_keyA(0x36, "dwc_eval");
    DWCi_GetMatchCnt()->evalCallback = evalCallback;
    DWCi_GetMatchCnt()->evalParam = evalParam;
    DWCi_GetMatchCnt()->state = 2;

    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNewA(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, 1, SBFalse, DWCi_SBCallback,
            NULL);
    }
    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        if (DWCi_HandleSBError(5) != 0)
        {
            return;
        }
    }

    DWCi_SetGPStatus(3, "", NULL);
    if (DWCi_HandleSBError(DWCi_SBUpdateAsync(DWCi_GetMatchCnt()->profileID)) != 0)
    {
        return;
    }
    if (DWCi_GetMatchCnt()->qr2Obj == NULL)
    {
        DWCi_QR2Startup(DWCi_GetMatchCnt()->profileID);
    }

    if (stpOptMinComp != NULL && stpOptMinComp->valid != 0)
    {
        stpOptMinComp->recvBit = 0;
        stpOptMinComp->timeoutBit = 0;
        stpOptMinComp->retry = 0;
        stpOptMinComp->lastPollTime = OSGetTime();
        stpOptMinComp->startTime = OSGetTime();
    }
}

void DWCi_ConnectToFriendsAsync(const u8 friendIdxList[],
    int friendIdxListLen, u8 numEntry, BOOL distantFriend,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam)
{
    char valueStr[12], keyValueStr[32];
    int result;
    GPResult gpResult;

    DWCi_SetMatchCommonParam(
        DWC_MATCH_TYPE_FRIEND, numEntry, matchedCallback, matchedParam);

    DWCi_GetMatchCnt()->distantFriend = (u8)(distantFriend ? 1 : 0);
    DWCi_GetMatchCnt()->evalCallback = evalCallback;
    DWCi_GetMatchCnt()->evalParam = evalParam;

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

void DWCi_SetupGameServer(u8 maxEntry, DWCMatchedSCCallback matchedCallback,
    void* matchedParam, DWCNewClientCallback newClientCallback,
    void* newClientParam)
{
    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);
    stpMatchCnt->qr2MatchType = 2;
    stpMatchCnt->qr2NumEntry = maxEntry;
    stpMatchCnt->matchedCallback = matchedCallback;
    stpMatchCnt->matchedParam = matchedParam;
    stpMatchCnt->nnFailureCount = 0;
    stpMatchCnt->aidList[0] = 0;
    qr2_register_keyA(0x32, "dwc_pid");
    qr2_register_keyA(0x33, "dwc_mtype");
    qr2_register_keyA(0x34, "dwc_mresv");
    qr2_register_keyA(0x35, "dwc_mver");
    qr2_register_keyA(0x36, "dwc_eval");
    stpMatchCnt->newClientCallback = newClientCallback;
    stpMatchCnt->newClientParam = newClientParam;
    stpMatchCnt->sbPidList[0] = stpMatchCnt->profileID;
    stpMatchCnt->validAidBitmap = 1;
    stpMatchCnt->gt2NumValidConn = 0;
    stOptSCBlock.lock = 0;
    stpMatchCnt->state = 10;

    if (DWCi_HandleGPError(DWCi_GPSetServerStatus()) == 0)
    {
        if (stpMatchCnt->qr2Obj == NULL)
        {
            DWCi_QR2Startup(stpMatchCnt->profileID);
        }
    }
}

void DWCi_ConnectToGameServerAsync(int serverPid,
    DWCMatchedSCCallback matchedCallback,
    void* matchedParam, DWCNewClientCallback newClientCallback,
    void* newClientParam)
{
    int result;
    GPResult gpResult;

    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);
    DWCi_GetMatchCnt()->qr2MatchType = 3;
    DWCi_GetMatchCnt()->qr2NumEntry = 0;
    DWCi_GetMatchCnt()->matchedCallback = matchedCallback;
    DWCi_GetMatchCnt()->matchedParam = matchedParam;
    DWCi_GetMatchCnt()->nnFailureCount = 0;
    DWCi_GetMatchCnt()->aidList[0] = 0;
    qr2_register_keyA(0x32, "dwc_pid");
    qr2_register_keyA(0x33, "dwc_mtype");
    qr2_register_keyA(0x34, "dwc_mresv");
    qr2_register_keyA(0x35, "dwc_mver");
    qr2_register_keyA(0x36, "dwc_eval");
    DWCi_GetMatchCnt()->newClientCallback = newClientCallback;
    DWCi_GetMatchCnt()->newClientParam = newClientParam;
    DWCi_GetMatchCnt()->qr2IsReserved = 1;
    DWCi_GetMatchCnt()->qr2Reservation = DWCi_GetMatchCnt()->profileID;
    DWCi_GetMatchCnt()->sbPidList[0] = serverPid;
    DWCi_GetMatchCnt()->state = 4;

    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        DWCi_GetMatchCnt()->sbObj = ServerBrowserNewA(
            DWCi_GetMatchCnt()->gameName, DWCi_GetMatchCnt()->gameName,
            DWCi_GetMatchCnt()->secretKey, 0, 20, 1, SBFalse, DWCi_SBCallback,
            NULL);
    }
    if (DWCi_GetMatchCnt()->sbObj == NULL)
    {
        if (DWCi_HandleSBError(5) != 0)
        {
            return;
        }
    }

    gpResult = DWCi_SetGPStatus(5, "", NULL);
    if (DWCi_HandleGPError(gpResult) != 0)
    {
        return;
    }
    if (DWCi_GetMatchCnt()->qr2Obj == NULL)
    {
        if (DWCi_QR2Startup(DWCi_GetMatchCnt()->profileID) != 0)
        {
            return;
        }
    }

    result = DWCi_SendResvCommand(DWCi_GetMatchCnt()->sbPidList[0], FALSE);
    if (DWCi_GetMatchCnt()->qr2MatchType == 0)
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

static void DWCi_SendStateChanged(qr2_t qr2Obj)
{
    if (qr2Obj == NULL)
        return;

    qr2_think(qr2Obj);

    if (qr2Obj->userstatechangerequested)
        return;

    switch (DWCi_GetMatchCnt()->qr2MatchType)
    {
    case DWC_MATCH_TYPE_ANYBODY:
    case DWC_MATCH_TYPE_FRIEND:
        switch (DWCi_GetMatchCnt()->state)
        {
        case DWC_MATCH_STATE_CL_WAITING:
        case DWC_MATCH_STATE_CL_SEARCH_OWN:
        case DWC_MATCH_STATE_CL_SEARCH_HOST:
        case DWC_MATCH_STATE_CL_WAIT_RESV:
        case DWC_MATCH_STATE_CL_NN:
        case DWC_MATCH_STATE_SV_OWN_NN:
            qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
            break;
        }
        break;
    case DWC_MATCH_TYPE_SC_SV:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_OWN_NN)
            qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
        break;
    }
}

void DWCi_MatchProcess(BOOL fullSpec)
{
    u32 resendInterval;
    u32 sbInterval;
    int result;
    SBError sbError;

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

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_INIT)
        return;

    switch (DWCi_GetMatchCnt()->state)
    {
    case DWC_MATCH_STATE_CL_WAIT_RESV:
        if (DWCi_GetMatchCnt()->cmdTimeoutTime > 0)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick()
                    - DWCi_GetMatchCnt()->cmdTimeoutStartTick)
                > DWCi_GetMatchCnt()->cmdTimeoutTime)
            {
                DWCi_GetMatchCnt()->cmdTimeoutTime = 0;

                if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_SC_CL)
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "Timeout: wait server response %d/%d.\n",
                        DWCi_GetMatchCnt()->scResvRetryCount,
                        DWC_MATCH_CMD_RETRY_MAX);

                    DWCi_GetMatchCnt()->scResvRetryCount++;
                    if (DWCi_GetMatchCnt()->scResvRetryCount
                        > DWC_MATCH_CMD_RETRY_MAX)
                    {
                        DWCi_StopMatching(DWC_ERROR_NETWORK,
                            DWC_ECODE_SEQ_MATCH
                                + DWC_ECODE_TYPE_SC_CL_FAIL);
                        return;
                    }
                    else
                    {
                        result = DWCi_SendResvCommand(
                            DWCi_GetMatchCnt()->sbPidList[0], FALSE);
                        if (DWCi_HandleMatchCommandError(result))
                            return;
                    }
                }
                else
                {
                    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                        "NN resv(with %u) timed out. Try next server.\n",
                        DWCi_GetMatchCnt()->sbPidList[0]);
                    if (!DWCi_RetryReserving(0))
                        return;
                }
            }
        }

        if (DWCi_GetMatchCnt()->cmdResendFlag)
        {
            resendInterval = (u32)DWC_MATCH_CMD_RESEND_INTERVAL_MSEC
                + DWC_MATCH_CMD_RESEND_INTERVAL_ADD_MSEC
                    * DWCi_GetMatchCnt()->gt2NumConnection;

            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->cmdResendTick)
                >= resendInterval)
            {
                result = DWCi_SendResvCommand(
                    DWCi_GetMatchCnt()->sbPidList[0], FALSE);
                if (DWCi_HandleMatchCommandError(result))
                    return;
            }
        }
        break;

    case DWC_MATCH_STATE_CL_SEARCH_OWN:
    case DWC_MATCH_STATE_CL_SEARCH_HOST:
    case DWC_MATCH_STATE_CL_SEARCH_NN_HOST:
        if (DWCi_GetMatchCnt()->sbUpdateFlag <= 0)
            break;

        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_SEARCH_HOST)
        {
            sbInterval = (u32)DWC_SB_UPDATE_INTERVAL_MSEC
                + DWC_SB_UPDATE_INTERVAL_ADD_MSEC
                    * DWCi_GetMatchCnt()->gt2NumConnection;
        }
        else
        {
            if (DWCi_GetMatchCnt()->sbUpdateFlag
                == DWC_SB_UPDATE_INTERVAL_SHORT)
                sbInterval = DWC_SB_UPDATE_INTERVAL_SHORT_MSEC;
            else
                sbInterval = DWC_SB_UPDATE_INTERVAL_MSEC;
        }

        if (DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - DWCi_GetMatchCnt()->sbUpdateTick)
            > sbInterval)
        {
            sbError = DWCi_SBUpdateAsync(DWCi_GetMatchCnt()->reqProfileID);
            if (DWCi_HandleSBError(sbError))
                return;
            DWCi_GetMatchCnt()->sbUpdateFlag = 0;
        }
        break;

    case DWC_MATCH_STATE_CL_GT2:
        if (DWCi_GetMatchCnt()->nnFinishTime)
        {
            if (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->nnFinishTime)
                > DWC_WAIT_GT2_CONNECT_TIMEOUT)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: wait gt2Connect().\n");

                DWCi_GetMatchCnt()->nnFinishTime = 0;
                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()->sbPidList[0]))
                    return;
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
            DWCi_GetMatchCnt()->cmdCnt.count++;
            if (DWCi_GetMatchCnt()->cmdCnt.count
                > DWC_MATCH_CMD_RETRY_MAX)
            {
                DWCi_StopResendingMatchCommand();

                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Stop resending command %d.\n",
                    DWC_MATCH_COMMAND_TELL_ADDR);

                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()->sbPidList[0]))
                    return;
                break;
            }

            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_TELL_ADDR,
                DWCi_GetMatchCnt()->cmdCnt.profileID,
                DWCi_GetMatchCnt()->cmdCnt.ip,
                DWCi_GetMatchCnt()->cmdCnt.port,
                DWCi_GetMatchCnt()->cmdCnt.data,
                DWCi_GetMatchCnt()->cmdCnt.len);
            if (DWCi_HandleMatchCommandError(result))
                return;
        }
        break;

    case DWC_MATCH_STATE_SV_OWN_NN:
        if ((DWCi_GetMatchCnt()->cmdCnt.command
                == DWC_MATCH_COMMAND_RESV_OK)
            && (((DWCi_GetMatchCnt()->qr2MatchType
                     == DWC_MATCH_TYPE_ANYBODY)
                    && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                            - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                        > DWC_MATCH_RESV_KEEP_TIME_ANYBODY))
                || ((DWCi_GetMatchCnt()->qr2MatchType
                         != DWC_MATCH_TYPE_ANYBODY)
                    && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                            - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                        > DWC_MATCH_RESV_KEEP_TIME_FRIEND))))
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Reservation timeout. Cancel reservation.\n");

            DWCi_StopResendingMatchCommand();

            if (!DWCi_CancelPreConnectedServerProcess(
                    DWCi_GetMatchCnt()
                        ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection + 1]))
                return;
        }
        break;

    case DWC_MATCH_STATE_SV_WAIT_CL_LINK:
        if ((DWCi_GetMatchCnt()->cmdCnt.command
                == DWC_MATCH_COMMAND_LINK_CLS_REQ)
            && (DWCi_Np_TicksToMilliSeconds(DWCi_Np_GetTick()
                    - DWCi_GetMatchCnt()->cmdCnt.sendTime)
                > DWC_MATCH_LINK_CLS_TIMEOUT))
        {
            DWCi_GetMatchCnt()->cmdCnt.count++;
            if (DWCi_GetMatchCnt()->cmdCnt.count > 0)
            {
                DWCi_StopResendingMatchCommand();

                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Wait clients connecting timeout.\n");

                if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_SC_SV)
                {
                    if (!DWCi_CancelPreConnectedServerProcess(
                            DWCi_GetMatchCnt()
                                ->sbPidList[DWCi_GetMatchCnt()
                                               ->gt2NumConnection]))
                        return;
                }
                else
                {
                    DWCi_RestartFromTimeout();
                }
                break;
            }

            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_LINK_CLS_REQ,
                DWCi_GetMatchCnt()->cmdCnt.profileID,
                DWCi_GetMatchCnt()->cmdCnt.ip,
                DWCi_GetMatchCnt()->cmdCnt.port,
                DWCi_GetMatchCnt()->cmdCnt.data,
                DWCi_GetMatchCnt()->cmdCnt.len);
            if (DWCi_HandleMatchCommandError(result))
                return;
        }
        break;

    case DWC_MATCH_STATE_CL_WAITING:
        if ((DWC_GetState() == DWC_STATE_MATCHING)
            && (DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - DWCi_GetMatchCnt()->clWaitTime)
                > DWC_MATCH_CL_WAIT_TIMEOUT))
        {
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
                        DWCi_GetMatchCnt()->sbPidList[0]))
                    return;
            }
            else
            {
                result = DWCi_SendMatchCommand(
                    DWC_MATCH_COMMAND_CL_WAIT_POLL,
                    DWCi_GetMatchCnt()->sbPidList[0],
                    DWCi_GetMatchCnt()->qr2IPList[0],
                    DWCi_GetMatchCnt()->qr2PortList[0], NULL, 0);
                if (DWCi_HandleMatchCommandError(result))
                    return;

                DWCi_GetMatchCnt()->clWaitTimeoutCount++;
                DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick()
                    - DWCi_Np_MilliSecondsToTicks(
                        DWC_MATCH_CL_WAIT_TIMEOUT
                        - DWC_MATCH_CMD_RTT_TIMEOUT);
            }
        }
        break;

    default:
        break;
    }

    if (((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_OWN_NN)
            || (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_NN))
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

    if (DWCi_GetMatchCnt()->sbObj)
    {
#if 1
        s_sbCallbackLevel = 0;
        s_needSbFree = FALSE;

        (void)ServerBrowserThink(DWCi_GetMatchCnt()->sbObj);

        if (s_needSbFree)
        {
            ServerBrowserFree(DWCi_GetMatchCnt()->sbObj);
            DWCi_GetMatchCnt()->sbObj = NULL;
        }
#else
        sbError = ServerBrowserThink(DWCi_GetMatchCnt()->sbObj);
        if (DWCi_HandleSBError(sbError))
            return;
#endif
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

    if ((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_WAIT_CLOSE)
        && (DWCi_Np_TicksToMilliSeconds(
                DWCi_Np_GetTick() - DWCi_GetMatchCnt()->closedTime)
            > (DWC_MATCH_CMD_RTT_TIMEOUT >> 1)))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "RTT Timeout with DWCi_MatchProcess.\n");
        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Timeout : Wait prior profileID.\n");

        if (DWCi_ResumeMatching())
            return;
    }

    if (!DWCi_ProcessMatchSynTimeout())
        return;

    if (!DWCi_ProcessCancelMatchSynTimeout())
        return;

    DWCi_ProcessOptMinComp();

#ifdef DWC_STOP_SC_SERVER
    if (DWCi_GetMatchCnt()->stopSCFlag
        && (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_WAITING))
    {
        DWCi_GetMatchCnt()->qr2NumEntry
            = DWCi_GetMatchCnt()->gt2NumConnection;
        (void)DWCi_GPSetServerStatus();

        DWCi_GetMatchCnt()->stopSCFlag = 0;

        if (DWCi_GetMatchCnt()->stopSCCallback)
            DWCi_GetMatchCnt()->stopSCCallback(
                DWCi_GetMatchCnt()->stopSCParam);
    }
#endif
}

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

void DWCi_GT2ConnectAttemptCallback(GT2Socket socket,
    GT2Connection connection, unsigned int ip, unsigned short port,
    int latency, GT2Byte* message, int len)
{
#pragma unused(socket)
#pragma unused(len)
    int index;
    GT2Bool result;
    GT2Connection* pGt2Con;
    DWCConnectionInfo* pConInfo;

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_NN)
    {
        return;
    }

    if (!DWCi_GetMatchCnt()
        || (DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_CL_GT2)
        || (DWCi_GetMatchCnt()->cancelState != DWC_MATCH_CANCEL_STATE_INIT))
    {
        gt2Reject(connection, (const GT2Byte*)"Init state", -1);
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "gt2Reject was called : Init state\n");
        return;
    }

    index = DWCi_GT2GetConnectionListIdx();
    if (index == -1)
    {
        gt2Reject(connection, (const GT2Byte*)"Server full", -1);
        DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
            "gt2Reject was called : Server full\n");
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GT2
                + DWC_ECODE_TYPE_CONN_OVER);
        return;
    }

    if ((ip != DWCi_GetMatchCnt()->ipList[DWCi_GetMatchCnt()->gt2NumConnection])
        || (SONtoHs(port)
            != DWCi_GetMatchCnt()->portList[DWCi_GetMatchCnt()->gt2NumConnection]))
    {
        if (message[0]
            && (strtoul((const char*)message, NULL, 10)
                == DWCi_GetMatchCnt()->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection]))
        {
            DWC_Printf(DWC_REPORTFLAG_WARNING,
                "gt2Connect() came before NN complete.\n");
            DWCi_GetMatchCnt()->ipList[DWCi_GetMatchCnt()->gt2NumConnection] = ip;
            DWCi_GetMatchCnt()->portList[DWCi_GetMatchCnt()->gt2NumConnection]
                = SONtoHs(port);
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

    result = gt2Accept(connection, DWCi_GetMatchCnt()->gt2Callbacks);
    if (result == GT2False)
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

    if (DWCi_GetMatchCnt()->gt2NumConnection == 0)
    {
        DWCi_GetMatchCnt()->baseLatency
            = (u16)(((latency >> 1) < 0xffff) ? (latency >> 1) : 0xffff);
    }

    pGt2Con = DWCi_GetGT2ConnectionByIdx(index);
    pConInfo = DWCi_GetConnectionInfoByIdx(index);
    *pGt2Con = connection;
    DWCi_GetMatchCnt()->gt2NumConnection++;

    pConInfo->index = (u8)index;
    pConInfo->aid = DWCi_GetMatchCnt()->aidList[DWCi_GetMatchCnt()->gt2NumConnection - 1];
    pConInfo->reserve = 0;
    pConInfo->param = NULL;

    gt2SetConnectionData(connection, pConInfo);

    DWCi_PostProcessConnection(DWC_PP_CONNECTION_CL_GT2_ACCEPT);
}

void DWCi_GT2ConnectedCallback(
    GT2Connection connection, GT2Result result, GT2Byte* message,
    int len)
{
    char pidStr[12];
    int index;
    GT2Connection* pGt2Con;
    GT2Result gt2Result;
    DWCConnectionInfo* pConInfo;

    if (DWCi_GetMatchCnt() == NULL
        || (DWCi_GetMatchCnt()->state != 7
            && DWCi_GetMatchCnt()->state != 12))
    {
        DWC_Printf(0x80, "gt2ConnectedCallback: Already cancel\n");
        return;
    }

    if (result != GT2Success)
    {
        if (message == NULL)
        {
            message = (GT2Byte*)"";
        }
        DWC_Printf(0x80, "GT2 connect failed %d: %s\n", result, message);
        if (result == GT2DuplicateAddress)
        {
            return;
        }
        else if (result == GT2TimedOut)
        {
            DWCi_GetMatchCnt()->gt2ConnectCount++;
            if (DWCi_GetMatchCnt()->gt2ConnectCount > 5)
            {
                DWC_Printf(0x80, "gt2Connect() retry over.\n");
                DWCi_GetMatchCnt()->gt2ConnectCount = 0;
                DWCi_CancelPreConnectedServerProcess(DWCi_GetMatchCnt()->sbPidList[DWCi_GetMatchCnt()->qr2NNFinishCount]);
                return;
            }
            DWC_Printf(0x80, "Retry to gt2Connect.\n");
            (void)snprintf(pidStr, sizeof(pidStr), "%u",
                DWCi_GetMatchCnt()->profileID);
            gt2Result = gt2Connect(*DWCi_GetMatchCnt()->pGt2Socket, NULL,
                gt2AddressToString(
                    DWCi_GetMatchCnt()->ipList[DWCi_GetMatchCnt()->qr2NNFinishCount],
                    DWCi_GetMatchCnt()->portList[DWCi_GetMatchCnt()->qr2NNFinishCount], NULL),
                (GT2Byte*)pidStr, -1, 5000, DWCi_GetMatchCnt()->gt2Callbacks, GT2False);
            if (gt2Result == GT2OutOfMemory)
            {
                DWCi_HandleGT2Error(gt2Result);
                return;
            }
            else if (gt2Result == GT2Success)
            {
            }
            else if (DWCi_CancelPreConnectedServerProcess(
                         DWCi_GetMatchCnt()->sbPidList[DWCi_GetMatchCnt()->qr2NNFinishCount])
                == 0)
            {
                return;
            }
        }
        else if (DWCi_CancelPreConnectedServerProcess(DWCi_GetMatchCnt()->sbPidList[
                     DWCi_GetMatchCnt()->gt2NumConnection + 1])
            == 0)
        {
            return;
        }
    }
    else
    {
        DWC_Printf(0x80, "GT2 connected.\n");
        index = DWCi_GT2GetConnectionListIdx();
        if (index == -1)
        {
            DWC_Printf(8,
                "Don't continue matching without closing connections!!\n");
            DWCi_StopMatching(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GT2
                    + DWC_ECODE_TYPE_CONN_OVER);
            return;
        }

        pGt2Con = DWCi_GetGT2ConnectionByIdx(index);
        pConInfo = DWCi_GetConnectionInfoByIdx(index);
        *pGt2Con = connection;
        DWCi_GetMatchCnt()->gt2NumConnection++;
        pConInfo->index = index;
        pConInfo->reserve = 0;
        pConInfo->param = NULL;
        pConInfo->aid = DWCi_GetMatchCnt()->aidList[DWCi_GetMatchCnt()->gt2NumConnection];
        gt2SetConnectionData(connection, pConInfo);

        if (DWCi_GetMatchCnt()->state == 12)
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
        }
        else
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_CL_GT2_CONNECT);
        }
    }
}

void DWCi_MatchGPRecvBuddyMsgCallback(
    GPConnection* connection, u32 profileId, char* message)
{
    char buf[16];
    u32 list[0x80];
    int i;

    DWC_Printf(0x40, "<GP> RECV-0x%02x <- [--------:-----] [pid=%u]\n",
        (s8)message[0], profileId);

    for (i = 0; i < 0x80; i++)
    {
        if (DWCi_GetGPBuddyAdditionalMsg(buf, message + 1, i) == -1)
        {
            break;
        }
        list[i] = strtoul(buf, NULL, 10);
    }
    DWCi_ProcessRecvMatchCommand(message[0], profileId, 0, 0, list, i, TRUE);
}

static void DWCi_StopResendingMatchCommand(void)
{
    DWCi_GetMatchCnt()->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
    DWCi_GetMatchCnt()->cmdCnt.count = 0;
}

void DWCi_StopMatching(DWCError error, int errorCode)
{
    if (DWCi_GetMatchCnt() == NULL || error == 0)
    {
        return;
    }

    DWCi_CloseAllConnectionsByTimeout();
    DWCi_SetError(error, errorCode);
    DWCi_SetGPStatus(1, "", NULL);
    DWCi_GetMatchCnt()->matchedCallback(error, FALSE,
        DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE,
        DWCi_GetMatchCnt()->qr2MatchType == 2 ? TRUE : FALSE,
        DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
        DWCi_GetMatchCnt()->matchedParam);
    DWCi_CloseMatching();
}

void DWCi_ClearQR2Key(void)
{
    if (stpMatchCnt->qr2MatchType == 2)
    {
        return;
    }
    stpMatchCnt->qr2NNFinishCount = 0;
    stpMatchCnt->qr2NumEntry = 0;
    qr2_send_statechanged(stpMatchCnt->qr2Obj);
}

void DWCi_ProcessMatchSynPacket(u8 aid, u16 type, u8* data)
{
    u8 aidIdx;
    u32 bitmask;
    int i;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Received SYN %d packet from aid %d.\n",
        type - DWC_SEND_TYPE_MATCH_SYN, aid);

    switch (type)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_WAITING)
        {
            if (data[0] == 1)
            {
                DWCi_GetMatchCnt()->cbEventPid = 0;
            }

            aidIdx = data[1];
            DWCi_GetMatchCnt()->aidList[aidIdx] = data[2];
            DWCi_GetMatchCnt()->sbPidList[aidIdx]
                = DWCi_GetMatchCnt()->profileID;

            if ((DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_ANYBODY)
                || (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_FRIEND))
            {
                DWCi_GetMatchCnt()->qr2NumEntry
                    = (u8)DWCi_GetMatchCnt()->gt2NumConnection;
            }

            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SYN);
        }

        DWCi_SendMatchSynPacket(aid, DWC_SEND_TYPE_MATCH_SYN_ACK);
        break;

    case DWC_SEND_TYPE_MATCH_SYN_ACK:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_SYN)
        {
            DWCi_GetMatchCnt()->synAckBit |= 1 << aid;

            if ((data[0] | (data[1] << 8))
                > DWCi_GetMatchCnt()->baseLatency)
            {
                DWCi_GetMatchCnt()->baseLatency
                    = (u16)(data[0] | (data[1] << 8));
            }

            bitmask = DWCi_GetAIDBitmask(FALSE);
            if (DWCi_GetMatchCnt()->synAckBit == bitmask)
            {
                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
                {
                    DWCi_SendMatchSynPacket(DWCi_GetMatchCnt()->aidList[i],
                        DWC_SEND_TYPE_MATCH_ACK);
                }

                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_SYN_WAIT);

                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "Wait max latency %d msec.\n",
                    DWCi_GetMatchCnt()->baseLatency);
            }
        }
        else
        {
            DWCi_SendMatchSynPacket(aid, DWC_SEND_TYPE_MATCH_ACK);
        }
        break;

    case DWC_SEND_TYPE_MATCH_ACK:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_SYN)
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SYN_FINISH);
        }
        break;
    }
}

BOOL DWCi_ProcessMatchClosing(
    DWCError error, int errorCode, int profileID)
{
#pragma unused(profileID)
    if (DWC_GetState() != DWC_STATE_MATCHING)
    {
        return FALSE;
    }

    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV)
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

    DWCi_GetMatchCnt()->aidList[0] = 0;

    if ((DWCi_GetMatchCnt()->cancelState == DWC_MATCH_CANCEL_STATE_EXEC)
        || (DWCi_GetMatchCnt()->closeState
            == DWC_MATCH_CLOSE_STATE_CHANGE_TO_CLIENT)
        || (DWCi_GetMatchCnt()->closeState == DWC_MATCH_CLOSE_STATE_TIMEOUT))
    {
        return TRUE;
    }

    if (DWCi_GetMatchCnt()->nnInfo.cookie)
    {
        NNCancel(DWCi_GetMatchCnt()->nnInfo.cookie);
        DWCi_GetMatchCnt()->nnInfo.cookie = 0;
    }

    if (DWCi_GetMatchCnt()->gt2NumConnection)
    {
        if (DWCi_GetMatchCnt()->closeState == DWC_MATCH_CLOSE_STATE_INIT)
        {
            DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_FORCED;
            gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
        }
    }
    else
    {
        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        {
            DWCi_StopMatching(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
        }
        else if (DWCi_GetMatchCnt()->priorProfileID)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Reserve NN to %u.\n",
                DWCi_GetMatchCnt()->priorProfileID);

            (void)DWCi_ResumeMatching();
        }
        else if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_WAITING)
        {
            DWC_Printf(
                DWC_REPORTFLAG_MATCH_NN, "Wait prior profileID.\n");

            DWCi_SetMatchStatus(DWC_MATCH_STATE_WAIT_CLOSE);
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

void DWCi_ProcessMatchSCClosing(int clientPid)
{
    if (DWCi_GetMatchCnt()->closeState != DWC_MATCH_CLOSE_STATE_TIMEOUT)
    {
        DWCi_CancelPreConnectedServerProcess(clientPid);
    }
}

BOOL DWCi_DeleteHostByProfileID(int profileID, int numHost)
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

int DWCi_DeleteHostByIndex(int index, int numHost)
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

int DWCi_GetNumAllConnection(void)
{
    if (!DWCi_GetMatchCnt())
        return 0;
    return DWCi_GetMatchCnt()->gt2NumConnection;
}

int DWCi_GetNumValidConnection(void)
{
    if (!DWCi_GetMatchCnt())
        return 0;
    return DWCi_GetMatchCnt()->gt2NumValidConn;
}

void DWCi_SetNumValidConnection(void)
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

int DWCi_GetAllAIDList(u8** aidList)
{
    if (stpMatchCnt == NULL)
    {
        return 0;
    }
    *aidList = stpMatchCnt->aidList;
    return stpMatchCnt->gt2NumConnection + 1;
}

int DWCi_GetValidAIDList(u8** aidList)
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

GPResult DWCi_GPSetServerStatus(void)
{
    char value[12];
    char status[32];

    if (stpMatchCnt->qr2MatchType != 2)
    {
        return GP_NO_ERROR;
    }
    snprintf(value, sizeof(value), "%u", stpMatchCnt->qr2NumEntry + 1);
    DWC_SetCommonKeyValueString("SCM", value, status, '/');
    snprintf(value, sizeof(value), "%u", stpMatchCnt->gt2NumConnection + 1);
    DWC_AddCommonKeyValueString("SCN", value, status, '/');
    snprintf(value, sizeof(value), "%u", 3);
    DWC_AddCommonKeyValueString("VER", value, status, '/');
    return DWCi_SetGPStatus(6, status, NULL);
}

void DWCi_ShutdownMatch(void)
{
    stpMatchCnt = NULL;
    if (stpAddFilter != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpAddFilter, 0);
        stpAddFilter = NULL;
    }

    DWCi_ClearGameMatchKeys();

    if (stpOptMinComp != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpOptMinComp, 0);
        stpOptMinComp = NULL;
    }

    stOptSCBlock.valid = 0;
    stOptSCBlock.lock = 0;
}

BOOL DWCi_IsShutdownMatch(void)
{
    return stpMatchCnt == NULL;
}

static void DWCi_ResetMatchParam(DWCMatchResetLevel level)
{
    DWCi_GetMatchCnt()->gt2ConnectCount = 0;
    DWCi_GetMatchCnt()->nnRecvCount = 0;
    DWCi_GetMatchCnt()->nnCookieRand = (u16)DWCi_GetMathRand32(0x10000);
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
    DWCi_GetMatchCnt()->searchPort = 0;
    DWCi_GetMatchCnt()->searchIP = 0;
    DWCi_GetMatchCnt()->lastSynSent = 0;
    DWCi_GetMatchCnt()->closedTime = 0;
    DWCi_Np_CpuClear32(
        &DWCi_GetMatchCnt()->cmdCnt, sizeof(DWCi_GetMatchCnt()->cmdCnt));

    if (level == DWC_MATCH_RESET_CONTINUE)
    {
        DWCi_GetMatchCnt()->qr2NNFinishCount = (u8)DWCi_GetMatchCnt()->gt2NumConnection;

        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        {
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAITING);
        }
        else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV)
        {
            DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_WAITING);
        }
    }
    else
    {
        DWCi_GetMatchCnt()->gt2NumConnection = 0;
        DWCi_GetMatchCnt()->gt2NumValidConn = 0;
        DWCi_GetMatchCnt()->qr2NNFinishCount = 0;
        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
        DWCi_GetMatchCnt()->sbUpdateFlag = FALSE;
        DWCi_GetMatchCnt()->friendCount = 0;
        DWCi_GetMatchCnt()->baseLatency = 0;
        DWCi_GetMatchCnt()->cmdResendFlag = FALSE;
        DWCi_GetMatchCnt()->cmdResendTick = 0;
        DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
        DWCi_GetMatchCnt()->cmdTimeoutStartTick = 0;
        DWCi_GetMatchCnt()->synAckBit = 0;
        DWCi_GetMatchCnt()->reqProfileID = 0;
        DWCi_GetMatchCnt()->priorProfileID = 0;
        DWCi_GetMatchCnt()->validAidBitmap = 0;

        DWCi_Np_CpuClear32(DWCi_GetMatchCnt()->qr2IPList,
            sizeof(DWCi_GetMatchCnt()->qr2IPList));
        DWCi_Np_CpuClear16(DWCi_GetMatchCnt()->qr2PortList,
            sizeof(DWCi_GetMatchCnt()->qr2PortList));
        DWCi_Np_CpuClear32(DWCi_GetMatchCnt()->sbPidList,
            sizeof(DWCi_GetMatchCnt()->sbPidList));
        DWCi_Np_CpuClear32(
            &DWCi_GetMatchCnt()->nnInfo, sizeof(DWCNNInfo));
        DWCi_Np_CpuClear32(DWCi_GetMatchCnt()->ipList,
            sizeof(DWCi_GetMatchCnt()->ipList));
        DWCi_Np_CpuClear16(DWCi_GetMatchCnt()->portList,
            sizeof(DWCi_GetMatchCnt()->portList));
        DWCi_Np_CpuClear8(DWCi_GetMatchCnt()->aidList,
            sizeof(DWCi_GetMatchCnt()->aidList));
        DWCi_Np_CpuClear32(DWCi_GetMatchCnt()->svDataBak,
            sizeof(DWCi_GetMatchCnt()->svDataBak));

        if (level == DWC_MATCH_RESET_RESTART)
        {
            if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);
            }
            else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
            }
        }
        else
        {
            DWCi_GetMatchCnt()->qr2MatchType = 0;
            DWCi_GetMatchCnt()->qr2NumEntry = 0;
            DWCi_GetMatchCnt()->qr2ShutdownFlag = 0;
            DWCi_GetMatchCnt()->cbEventPid = 0;
            DWCi_GetMatchCnt()->distantFriend = 0;
            DWCi_GetMatchCnt()->stopSCFlag = 0;
            DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
            DWCi_GetMatchCnt()->clWaitTime = 0;
            DWCi_GetMatchCnt()->newClientCallback = NULL;
            DWCi_GetMatchCnt()->newClientParam = NULL;
        }
    }
}

static void DWCi_SetMatchCommonParam(u8 matchType, u8 numEntry,
    DWCMatchedSCCallback callback, void* param)
{
    DWCi_ResetMatchParam(DWC_MATCH_RESET_ALL);

    DWCi_GetMatchCnt()->qr2MatchType = matchType;
    DWCi_GetMatchCnt()->qr2NumEntry = numEntry;
    DWCi_GetMatchCnt()->matchedCallback = callback;
    DWCi_GetMatchCnt()->matchedParam = param;
    DWCi_GetMatchCnt()->nnFailureCount = 0;

    DWCi_GetMatchCnt()->aidList[0] = 0;

    qr2_register_key(DWC_QR2_PID_KEY, DWC_QR2_PID_KEY_STR);
    qr2_register_key(DWC_QR2_MATCH_TYPE_KEY, DWC_QR2_MATCH_TYPE_KEY_STR);
    qr2_register_key(DWC_QR2_MATCH_RESV_KEY, DWC_QR2_MATCH_RESV_KEY_STR);
    qr2_register_key(DWC_QR2_MATCH_VER_KEY, DWC_QR2_MATCH_VER_KEY_STR);
    qr2_register_key(DWC_QR2_MATCH_EVAL_KEY, DWC_QR2_MATCH_EVAL_KEY_STR);
}

static void DWCi_CloseMatching(void)
{
    DWC_Printf(0x40, " Close Matching....\n");
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
    stpMatchCnt->state = 0;

    if (stpAddFilter != NULL)
    {
        DWC_Free(DWC_ALLOCTYPE_BASE, stpAddFilter, 0);
        stpAddFilter = NULL;
    }

    DWCi_ClearGameMatchKeys();

    stpMatchCnt->qr2ShutdownFlag = 1;
}

#define DWC_NUM_QR2_MATCH_KEY 7

static SBError DWCi_SBUpdateAsync(int profileID)
{
    char filter[MAX_FILTER_LEN];
    char* pFilter = filter;
    u8 basicFields[DWC_NUM_QR2_MATCH_KEY + DWC_QR2_GAME_RESERVED_KEYS];
    int len;
    int numBasicFields = DWC_NUM_QR2_MATCH_KEY;
    int i;
    SBError sbError = sbe_noerror;

    basicFields[0] = NUMPLAYERS_KEY;
    basicFields[1] = MAXPLAYERS_KEY;
    basicFields[2] = DWC_QR2_PID_KEY;
    basicFields[3] = DWC_QR2_MATCH_TYPE_KEY;
    basicFields[4] = DWC_QR2_MATCH_RESV_KEY;
    basicFields[5] = DWC_QR2_MATCH_VER_KEY;
    basicFields[6] = DWC_QR2_MATCH_EVAL_KEY;

    if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
        || (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND))
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
    case DWC_MATCH_STATE_CL_SEARCH_HOST:
        if (!DWCi_GetMatchCnt()->priorProfileID)
        {
            len = DWCi_GetDefaultMatchFilter(filter,
                DWCi_GetMatchCnt()->profileID,
                DWCi_GetMatchCnt()->qr2NumEntry,
                DWCi_GetMatchCnt()->qr2MatchType);

            if (stpAddFilter)
            {
                (void)snprintf(filter, sizeof(filter), "%s and (%s)", filter,
                    stpAddFilter);
            }
            break;
        }
        else
        {
            profileID = DWCi_GetMatchCnt()->priorProfileID;
        }

    case DWC_MATCH_STATE_CL_SEARCH_OWN:
    case DWC_MATCH_STATE_CL_WAIT_RESV:
    case DWC_MATCH_STATE_CL_SEARCH_NN_HOST:
        (void)snprintf(filter, sizeof(filter), "%s = %u",
            DWC_QR2_PID_KEY_STR, profileID);
        DWCi_GetMatchCnt()->reqProfileID = profileID;
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
        sbError = ServerBrowserLimitUpdate(DWCi_GetMatchCnt()->sbObj, SBTrue,
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
    }

    return sbError;
}

static int DWCi_GetDefaultMatchFilter(
    char* filter, int profileID, u8 numEntry, u8 matchType)
{
    return snprintf(filter, MAX_FILTER_LEN,
        "%s = %d and %s != %u and maxplayers = %d and numplayers < %d and %s "
        "= %d and %s != %s",
        DWC_QR2_MATCH_VER_KEY_STR, DWC_MATCHING_VERSION,
        DWC_QR2_PID_KEY_STR, profileID, numEntry, numEntry,
        DWC_QR2_MATCH_TYPE_KEY_STR, matchType,
        DWC_QR2_MATCH_RESV_KEY_STR, DWC_QR2_PID_KEY_STR);
}

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

static NegotiateError DWCi_NNStartupAsync(
    int isQR2, int cookie, SBServer server)
{
    u32 ip;
    u32 senddata[2];
    int index = DWCi_GetMatchCnt()->qr2NNFinishCount;
    int result;
    BOOL nnValid;
    NegotiateError nnError = ne_noerror;

    if (!isQR2)
    {
        cookie = (DWCi_GetMatchCnt()->profileID & 0x0ffff)
            | (DWCi_GetMatchCnt()->nnCookieRand << 16);
        if (SBServerHasPrivateAddress(server))
        {
            if (SBServerGetPublicInetAddress(server)
                == ServerBrowserGetMyPublicIPAddr(DWCi_GetMatchCnt()->sbObj))
            {
                DWC_Printf(0x40, "Server[%d] is behind same NAT as me.\n",
                    index);
                DWCi_GetMatchCnt()->ipList[index]
                    = SBServerGetPrivateInetAddress(server);
                DWCi_GetMatchCnt()->portList[index]
                    = SBServerGetPrivateQueryPort(server);
                nnValid = FALSE;
            }
            else
            {
                DWC_Printf(0x40, "Server[%d] is behind NAT.\n", index);
                nnValid = TRUE;
            }
        }
        else
        {
            ip = SOGetHostID();
            if (IsPrivateAddr(ip))
            {
                DWC_Printf(0x40,
                    "Server[%d] is not behind NAT. But I'm behind NAT.\n",
                    index);
                nnValid = TRUE;
            }
            else
            {
                DWC_Printf(0x40,
                    "Both I and Server[%d] are not behind NAT.\n", index);
                DWCi_GetMatchCnt()->ipList[index]
                    = SBServerGetPublicInetAddress(server);
                DWCi_GetMatchCnt()->portList[index]
                    = SBServerGetPublicQueryPort(server);
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
            senddata[0] = DWCi_MatchIPToLE(DWC_MATCH_COMMAND_TELL_ADDR, (u32)SOGetHostID());
            senddata[1] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_TELL_ADDR,
                gt2GetLocalPort(*DWCi_GetMatchCnt()->pGt2Socket));
            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_TELL_ADDR,
                DWCi_GetMatchCnt()->sbPidList[index],
                SBServerGetPublicInetAddress(server),
                SBServerGetPublicQueryPort(server), senddata, 2);
            DWCi_GetMatchCnt()->cmdCnt.count = 0;
            if (result)
                return ne_socketerror;
            DWCi_GetMatchCnt()->nnInfo.cookie = 0;
        }

        DWCi_GetMatchCnt()->nnInfo.isQR2 = 0;
        DWCi_GetMatchCnt()->nnInfo.retryCount = 0;
        DWCi_GetMatchCnt()->nnInfo.port
            = SBServerGetPublicQueryPort(server);
        DWCi_GetMatchCnt()->nnInfo.ip = SBServerGetPublicInetAddress(server);
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

static int DWCi_SendMatchCommand(u8 command, int profileID, u32 ip, u16 port,
    const u32 data[], int len)
{
    char message[DWC_MATCH_COMMAND_ADD_MESSAGE_MAX];
    char tmpStr[16];
    int msgLen = 0, tmpLen;
    int result;
    int i;

    // the two SB cases are separate branches (two calls)
    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
    {
        result = (int)DWCi_SendSBMsgCommand(command, ip, port, data, len);
    }
    else if (((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
                 || DWCi_GetMatchCnt()->distantFriend)
             && (command == DWC_MATCH_COMMAND_TELL_ADDR))
    {
        result = (int)DWCi_SendSBMsgCommand(command, ip, port, data, len);
    }
    else
    {
        if (data && len)
        {
            msgLen = snprintf(message, sizeof(message), "%u", data[0]);

            for (i = 1; i < len; i++)
            {
                tmpLen = snprintf(tmpStr, sizeof(tmpStr), "/%u", data[i]);
                DWCi_Np_CpuCopy8(
                    tmpStr, &message[msgLen], (u32)tmpLen);
                msgLen += tmpLen;
            }
        }
        message[msgLen] = '\0';

        result = (int)DWCi_SendGPBuddyMsgCommand(
            DWCi_GetMatchCnt()->pGpObj, command, profileID, message);
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
            DWCi_Np_CpuCopy32(
                data, DWCi_GetMatchCnt()->cmdCnt.data, (u32)len * 4);
        }
    }

    return result;
}

static SBError DWCi_SendSBMsgCommand(
    u8 command, u32 ip, u16 port, const u32 data[], int len)
{
    int i;
    DWCSBMessage senddata;
    SBError sbError;

    if (data && len)
    {
        DWCi_Np_CpuCopy32(data, senddata.data, sizeof(u32) * len);
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

static GPResult DWCi_SendGPBuddyMsgCommand(GPConnection* connection,
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

// hostOrder (TRUE from GP buddy messages) says how the data words are ordered.
static BOOL DWCi_ProcessRecvMatchCommand(u8 command, int srcPid, u32 srcIP,
    u16 srcPort, const u32 data[], int len, BOOL hostOrder)
{
    u8 sendCommand;
    u8 aid;
    u16 port = 0;
    u32 ip = 0;
    u32 aidIdx;
    u32 senddata[DWC_MAX_MATCH_IDX_LIST + 1];
    int profileID;
    int sendlen = 0;
    u32* closeData;
    u32* hostData = NULL;
    int result;
    int i;
    SBError sbError;
    NegotiateError nnError;
    struct sockaddr_in remoteaddr;

    if (stpMatchCnt == NULL || stpMatchCnt->state == 0)
    {
        return TRUE;
    }

    if (DWC_GetState() == DWC_STATE_MATCHING && stpMatchCnt->state == 1
        && (int)srcPid == stpMatchCnt->sbPidList[0])
    {
        DWCi_InitClWaitTimeout();
    }

    switch (command)
    {
    case 1:
    case 11:
    {
        if (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_ANYBODY)
        {
            srcIP = DWCi_MatchIPFromLE(hostOrder, data[1]);
            srcPort = (u16)DWCi_MatchDataFromLE(hostOrder, data[2]);
            DWC_Printf(DWC_REPORTFLAG_DEBUG, "friend IP:%d.%d.%d.%d, port:%d\n", srcIP >> 24,
                (srcIP >> 16) & 0xff, (srcIP >> 8) & 0xff, srcIP & 0xff, srcPort); // four bytes
        }
        sendCommand = DWCi_CheckResvCommand(srcPid, srcIP, srcPort,
            DWCi_MatchDataFromLE(hostOrder, data[0]),
            command == DWC_MATCH_COMMAND_RESV_PRIOR ? TRUE : FALSE);
        if (sendCommand == DWC_MATCH_COMMAND_RESV_OK)
        {
            DWCi_GetMatchCnt()->friendAcceptBit = 0;
            result = DWCi_ProcessResvOK(srcPid, srcIP, srcPort);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }

            if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV
                && DWCi_GetMatchCnt()->newClientCallback)
            {
                DWCi_GetMatchCnt()->newClientCallback(
                    DWCi_GetFriendListIndex(srcPid),
                    DWCi_GetMatchCnt()->newClientParam);
            }

            senddata[0] = DWCi_MatchDataToLE(sendCommand, DWCi_GetMatchCnt()->qr2NNFinishCount);
            for (sendlen = 1; sendlen <= DWCi_GetMatchCnt()->qr2NNFinishCount;
                 sendlen++)
            {
                senddata[sendlen]
                    = DWCi_MatchDataToLE(sendCommand, DWCi_GetMatchCnt()->sbPidList[sendlen]);
            }
            senddata[sendlen++] = DWCi_MatchIPToLE(sendCommand, DWCi_GetMatchCnt()->qr2IP);
            senddata[sendlen++] = DWCi_MatchDataToLE(sendCommand, DWCi_GetMatchCnt()->qr2Port);
            DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_OWN_NN);
        }
        else if (sendCommand == DWC_MATCH_COMMAND_RESV_DENY)
        {
            if ((DWCi_GetMatchCnt()->qr2NumEntry > 0)
                && (DWCi_GetMatchCnt()->qr2NNFinishCount
                    == DWCi_GetMatchCnt()->qr2NumEntry))
            {
                if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_SC_SV)
                {
                    senddata[0] = DWCi_MatchDataToLE(
                        sendCommand, DWC_MATCH_RESV_DENY_REASON_SV_FULL);
                    sendlen = 1;
                }
            }
        }

        if (sendCommand != DWC_MATCH_COMMAND_DUMMY)
        {
            result = DWCi_SendMatchCommand(
                sendCommand, srcPid, srcIP, srcPort, senddata, sendlen);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }
        }
        break;
    }

    case 2:
    {
        u32 num = DWCi_MatchDataFromLE(hostOrder, data[0]);

        if (stpMatchCnt->state != 4)
        {
            break;
        }
        DWC_Printf(0x40, "Succeeded NN reservation.\n");
        if ((int)srcPid != stpMatchCnt->reqProfileID)
        {
            break;
        }
        stpMatchCnt->priorProfileID = 0;
        stpMatchCnt->resvWaitCount = 0;
        stpMatchCnt->cmdTimeoutTime = 0;
        stpMatchCnt->cmdResendFlag = 0;
        stpMatchCnt->qr2IPList[0] = DWCi_MatchIPFromLE(hostOrder, data[num + 1]);
        stpMatchCnt->qr2PortList[0] = DWCi_MatchDataFromLE(hostOrder, data[num + 2]);
        stpMatchCnt->searchIP = DWCi_MatchIPFromLE(hostOrder, data[num + 1]);
        stpMatchCnt->searchPort = DWCi_MatchDataFromLE(hostOrder, data[num + 2]);
        DWC_Printf(4, "Server IP:%d.%d.%d.%d, port:%d\n", stpMatchCnt->searchIP >> 24,
            (stpMatchCnt->searchIP >> 16) & 0xff, (stpMatchCnt->searchIP >> 8) & 0xff,
            stpMatchCnt->searchIP & 0xff, stpMatchCnt->searchPort); // four bytes

        // the server data is converted to host order into an allocated copy
        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
        {
            hostData = DWC_Alloc(DWC_ALLOCTYPE_BASE, (num + 1) * sizeof(u32));
            if (hostData == NULL)
            {
                DWCi_StopMatching(DWC_ERROR_FATAL, DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
                return FALSE;
            }
            for (i = 0; i < num + 1; i++)
            {
                hostData[i] = DWCi_MatchDataFromLE(hostOrder, data[i]);
            }
            if (DWCi_AreAllBuddies(&hostData[1], num))
            {
                if (DWCi_GetMatchCnt()->gt2NumConnection)
                {
                    DWCi_MakeBackupServerData(srcPid, hostData);
                }
            }
            else
            {
                DWC_Printf(
                    DWC_REPORTFLAG_MATCH_NN, "But some clients are not friends.\n");

                result = DWCi_CancelReservation(srcPid);
                if (DWCi_HandleMatchCommandError(result))
                {
                    if (hostData != NULL)
                    {
                        DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                    }
                    return FALSE;
                }

                result = DWCi_SendResvCommandToFriend(FALSE, FALSE, srcPid);
                if (DWCi_HandleMatchCommandError(result))
                {
                    if (hostData != NULL)
                    {
                        DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                    }
                    return FALSE;
                }
                if (hostData != NULL)
                {
                    DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                }
                break;
            }
        }

        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
        {
            if (DWCi_GetMatchCnt()->gt2NumConnection)
            {
                if (hostData == NULL)
                {
                    hostData = DWC_Alloc(DWC_ALLOCTYPE_BASE, (num + 1) * sizeof(u32));
                    if (hostData == NULL)
                    {
                        DWCi_StopMatching(DWC_ERROR_FATAL, DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
                        return FALSE;
                    }
                    for (i = 0; i < num + 1; i++)
                    {
                        hostData[i] = DWCi_MatchDataFromLE(hostOrder, data[i]);
                    }
                }
                DWCi_MakeBackupServerData(srcPid, hostData);

                result = DWCi_ChangeToClient();
                if (DWCi_HandleMatchCommandError(result))
                {
                    if (hostData != NULL)
                    {
                        DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                    }
                    return FALSE;
                }
            }

            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_NN);
            nnError = DWCi_NNStartupAsync(
                0, 0, ServerBrowserGetServer(DWCi_GetMatchCnt()->sbObj, 0));
            if (DWCi_HandleNNError(nnError) != 0)
            {
                if (hostData != NULL)
                {
                    DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                }
                return FALSE;
            }
        }
        else
        {
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_NN_HOST);
            sbError = DWCi_SBUpdateAsync(srcPid);
            if (DWCi_HandleSBError(sbError) != 0)
            {
                if (hostData != NULL)
                {
                    DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
                }
                return FALSE;
            }
        }
        if (hostData != NULL)
        {
            DWC_Free(DWC_ALLOCTYPE_BASE, hostData, 0);
        }
        break;
    }

    case 3:
        if (stpMatchCnt->state != 4)
        {
            break;
        }
        if ((int)srcPid != stpMatchCnt->reqProfileID)
        {
            break;
        }
        DWC_Printf(0x40, "Reservation was denied by %u.\n", srcPid);
        if (len > 0 && DWCi_MatchDataFromLE(hostOrder, data[0]) == 0x10)
        {
            DWC_Printf(0x40, "Game server is fully occupied.\n");
            DWCi_StopMatching(DWC_ERROR_SERVER_FULL, 0);
            return FALSE;
        }
        return DWCi_RetryReserving(stpMatchCnt->reqProfileID);

    case 4:
        if (DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_CL_WAIT_RESV)
        {
            break;
        }
        if (srcPid == DWCi_GetMatchCnt()->reqProfileID)
        {
            DWCi_GetMatchCnt()->cmdTimeoutStartTick = DWCi_Np_GetTick();
            if ((DWCi_GetMatchCnt()->priorProfileID
                    && DWCi_GetMatchCnt()->resvWaitCount
                        < DWC_RESV_COMMAND_RETRY_MAX)
                || DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
            {
                DWCi_GetMatchCnt()->cmdResendFlag = TRUE;
                DWCi_GetMatchCnt()->cmdResendTick = DWCi_Np_GetTick();
                if (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_SC_CL)
                {
                    DWCi_GetMatchCnt()->resvWaitCount++;
                }
                break;
            }

            DWCi_GetMatchCnt()->priorProfileID = 0;
            DWCi_GetMatchCnt()->resvWaitCount = 0;
            if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);
                DWCi_GetMatchCnt()->sbUpdateFlag
                    = DWC_SB_UPDATE_INTERVAL_SHORT;
                DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
            }
            else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
            {
                (void)DWCi_SendResvCommandToFriend(TRUE, FALSE, 0);
            }
        }
        break;

    case 5:
        if (stpMatchCnt->qr2IsReserved == 0)
        {
            break;
        }
        if ((int)srcPid != stpMatchCnt->qr2Reservation)
        {
            break;
        }
        if (stpMatchCnt->qr2MatchType == 2 && stpMatchCnt->gt2NumConnection == 1
            && (int)srcPid == stpMatchCnt->sbPidList[1])
        {
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
        }
        if (DWCi_CancelPreConnectedServerProcess(srcPid) == 0)
        {
            return FALSE;
        }
        break;

    case 6:
        ip = DWCi_MatchIPFromLE(hostOrder, data[0]);
        port = DWCi_MatchDataFromLE(hostOrder, data[1]);
        DWC_Printf(0x40,
            "NN parent is behind same NAT as me. Received IP %x & "
            "port %d\n",
            ip, port);
        if (stpMatchCnt->state == 1)
        {
            stpMatchCnt->state = 6;
        }
        else if ((stpMatchCnt->state != 6 && stpMatchCnt->state != 11)
            || (int)srcPid != stpMatchCnt->qr2Reservation)
        {
            DWC_Printf(0x40, "But already canceled reservation.\n");
            break;
        }
        stpMatchCnt->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
        if ((int)srcPid
            != stpMatchCnt->sbPidList[stpMatchCnt->gt2NumConnection + 1])
        {
            stpMatchCnt->sbPidList[stpMatchCnt->gt2NumConnection + 1] = srcPid;
        }
        remoteaddr.sin_addr.addr = ip;
        remoteaddr.sin_port = SOHtoNs(port);
        stpMatchCnt->nnInfo.isQR2 = 1;
        DWCi_NNCompletedCallback(nr_success,
            gt2GetSocketSOCKET(*stpMatchCnt->pGt2Socket), &remoteaddr,
            &stpMatchCnt->nnInfo);
        stpMatchCnt->nnFinishTime = 0;
        break;

    case 7:
        if (stpMatchCnt->state != 1
            || (int)srcPid != stpMatchCnt->sbPidList[0])
        {
            DWC_Printf(4, "Ignore delayed NEW_PID_AID command.\n");
            break;
        }
        profileID = (int)DWCi_MatchDataFromLE(hostOrder, data[0]);
        aid = (u8)DWCi_MatchDataFromLE(hostOrder, data[1]);
        if (stpMatchCnt->qr2MatchType == 1 && stpMatchCnt->distantFriend == 0)
        {
            senddata[0] = DWCi_MatchDataToLE(
                DWC_MATCH_COMMAND_FRIEND_ACCEPT, (u32)DWCi_IsFriendByIdxList(profileID));
            DWC_Printf(0x40, "profileID %d is acceptable? - %d.\n",
                profileID, senddata[0]);
            result = DWCi_SendMatchCommand(
                0x20, srcPid, srcIP, srcPort, senddata, 1);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }
        }
        stpMatchCnt->sbPidList[stpMatchCnt->qr2NNFinishCount + 1] = profileID;
        stpMatchCnt->aidList[stpMatchCnt->qr2NNFinishCount + 1] = aid;
        qr2_send_statechanged(stpMatchCnt->qr2Obj);
        if (DWCi_GetMatchCnt()->newClientCallback)
        {
            DWCi_GetMatchCnt()->newClientCallback(
                DWCi_GetFriendListIndex(profileID),
                DWCi_GetMatchCnt()->newClientParam);
        }
        DWC_Printf(0x40,
            "Received new client's profileID = %u & aid = %d.\n", profileID,
            aid);
        break;

    case 8:
        if (DWCi_GetMatchCnt()->state != 1
            || (int)srcPid != DWCi_GetMatchCnt()->sbPidList[0])
        {
            DWC_Printf(4, "Ignore delayed LINK_CLS_REQ command.\n");
            break;
        }
        profileID = (int)DWCi_MatchDataFromLE(hostOrder, data[0]);
        if (profileID == 0)
        {
            aidIdx = DWCi_MatchDataFromLE(hostOrder, data[1]);
            aid = (u8)DWCi_MatchDataFromLE(hostOrder, data[2]);
            DWCi_GetMatchCnt()->aidList[aidIdx] = aid;
            DWCi_GetMatchCnt()->sbPidList[aidIdx] = DWCi_GetMatchCnt()->profileID;
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_CL_FINISH_CONNECT);
        }
        else
        {
            aidIdx = DWCi_MatchDataFromLE(hostOrder, data[1]);
            aid = (u8)DWCi_MatchDataFromLE(hostOrder, data[2]);
            if (DWCi_GetMatchCnt()->sbPidList[aidIdx] == profileID
                && aidIdx == (u32)(DWCi_GetMatchCnt()->gt2NumConnection - 1))
            {
                u32 senddata[1];

                senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_SUC, (u32)profileID);
                DWC_Printf(0x40,
                    "Resend command %d for delayed command %d.\n", 9, 8);
                result = DWCi_SendMatchCommand(9, srcPid,
                    DWCi_GetMatchCnt()->qr2IPList[0], DWCi_GetMatchCnt()->qr2PortList[0],
                    senddata, 1);
                if (DWCi_HandleMatchCommandError(result))
                {
                    return FALSE;
                }
                break;
            }
            DWCi_GetMatchCnt()->sbPidList[aidIdx] = profileID;
            DWCi_GetMatchCnt()->aidList[aidIdx] = aid;
            DWCi_GetMatchCnt()->qr2IPList[aidIdx] = DWCi_MatchIPFromLE(hostOrder, data[3]);
            DWCi_GetMatchCnt()->qr2PortList[aidIdx] = (u16)DWCi_MatchDataFromLE(hostOrder, data[4]);
            DWCi_GetMatchCnt()->searchIP = DWCi_MatchIPFromLE(hostOrder, data[3]);
            DWCi_GetMatchCnt()->searchPort = (u16)DWCi_MatchDataFromLE(hostOrder, data[4]);
            DWC_Printf(4, "Client IP:%d.%d.%d.%d, port:%d\n", DWCi_GetMatchCnt()->searchIP >> 24,
                (DWCi_GetMatchCnt()->searchIP >> 16) & 0xff, (DWCi_GetMatchCnt()->searchIP >> 8) & 0xff,
                DWCi_GetMatchCnt()->searchIP & 0xff, DWCi_GetMatchCnt()->searchPort); // four bytes
            DWCi_GetMatchCnt()->state = 5;
            DWC_Printf(0x40, "Next, try to connect to %u.\n", profileID);
            sbError = DWCi_SBUpdateAsync(profileID);
            if (DWCi_HandleSBError(sbError) != 0)
            {
                return FALSE;
            }
            DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
            DWCi_GetMatchCnt()->cmdResendFlag = 0;
        }
        break;

    case 9:
        if (DWCi_GetMatchCnt()->state == 13
            && (u32)DWCi_GetMatchCnt()->sbPidList[DWCi_GetMatchCnt()->clLinkProgress + 1]
                == DWCi_MatchDataFromLE(hostOrder, data[0]))
        {
            DWCi_GetMatchCnt()->clLinkProgress++;
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
        }
        else
        {
            DWC_Printf(0x40, "Ignore delayed command %d.\n", 9);
        }
        break;

    case 10:
    {
        u32 num = DWCi_MatchDataFromLE(hostOrder, data[0]);

        if (stpMatchCnt->state != 1 && stpMatchCnt->state != 18)
        {
            DWC_Printf(4, "Ignore delayed CLOSE_LINK command.\n");
            break;
        }

        // the profile IDs are converted to host order into an allocated copy
        closeData = DWC_Alloc(DWC_ALLOCTYPE_BASE, (num + 1) * sizeof(u32));
        if (closeData == NULL)
        {
            DWCi_StopMatching(DWC_ERROR_FATAL, DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_ALLOC);
            return FALSE;
        }
        for (i = 0; i < num + 1; i++)
        {
            closeData[i] = DWCi_MatchDataFromLE(hostOrder, data[i]);
        }

        if (stpMatchCnt->qr2MatchType == DWC_MATCH_TYPE_ANYBODY
            || DWCi_AreAllBuddies(&closeData[1], num))
        {
            stpMatchCnt->priorProfileID = (int)closeData[1];
            stpMatchCnt->resvWaitCount = 0;
            DWC_Printf(0x40, "Received close command. Next try to %u.\n",
                closeData[1]);
        }
        else
        {
            stpMatchCnt->priorProfileID = 0;
            DWC_Printf(0x40,
                "Received close command. Server %u or its clients are "
                "not friends.\n",
                closeData[1]);
        }
        if (stpMatchCnt->gt2NumConnection != 0)
        {
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
        }
        else if (DWCi_ResumeMatching() != 0)
        {
            if (closeData != NULL)
            {
                DWC_Free(DWC_ALLOCTYPE_BASE, closeData, 0);
            }
            return FALSE;
        }
        if (closeData != NULL)
        {
            DWC_Free(DWC_ALLOCTYPE_BASE, closeData, 0);
        }
        break;
    }

    case 12:
        if ((int)srcPid != stpMatchCnt->sbPidList[0])
        {
            DWC_Printf(4, "Ignore delayed CANCEL command.\n");
            break;
        }
        DWC_Printf(0x40, "Received cancel command from %u data[0] = %d.\n",
            srcPid, DWCi_MatchDataFromLE(hostOrder, data[0]));
        DWC_Printf(4, "numHost nn=%d gt2=%d, state %d\n",
            stpMatchCnt->qr2NNFinishCount, stpMatchCnt->gt2NumConnection, stpMatchCnt->state);
        if (stpMatchCnt->qr2MatchType == 0 || stpMatchCnt->qr2MatchType == 1)
        {
            if (DWCi_CancelPreConnectedClientProcess(srcPid) == 0)
            {
                return FALSE;
            }
        }
        else if (stpMatchCnt->qr2MatchType == 3)
        {
            stpMatchCnt->cbEventPid = srcPid;
            stpMatchCnt->closeState = 2;
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
            stpMatchCnt->closeState = 0;
            DWCi_RestartFromCancel(0);
        }
        break;

    case 13:
    case 14:
    case 15:
        if (DWCi_ProcessCancelMatchSynCommand(srcPid, command, DWCi_MatchDataFromLE(hostOrder, data[0])) == 0)
        {
            return FALSE;
        }
        break;

    case 16:
        if ((int)srcPid != stpMatchCnt->sbPidList[0])
        {
            return TRUE;
        }
        DWC_Printf(4, "Close shutdown client.\n");
        for (i = 0; i < len; i++)
        {
            aid = DWCi_GetAIDFromProfileID(
                (int)DWCi_MatchDataFromLE(hostOrder, data[i]), FALSE);
            if (aid != 0xFF)
            {
                DWC_CloseConnectionHard(aid);
            }
        }
        break;

    case 17:
        if (stpOptMinComp != NULL && stpOptMinComp->valid != 0
            && DWCi_Np_TicksToMilliSeconds(
                   DWCi_Np_GetTick() - stpOptMinComp->startTime)
                >= stpOptMinComp->timeout)
        {
            senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_POLL_TO_ACK, 1);
            DWC_Printf(0x80, "[OPT_MIN_COMP] time is %lu.\n",
                DWCi_Np_TicksToMilliSeconds(
                    DWCi_Np_GetTick() - stpOptMinComp->startTime));
        }
        else
        {
            senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_POLL_TO_ACK, 0);
        }
        result = DWCi_SendMatchCommand(
            18, srcPid, srcIP, srcPort, senddata, 1);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
        break;

    case 18:
        if (stpMatchCnt->state != 19)
        {
            break;
        }
        aid = DWCi_GetAIDFromProfileID(srcPid, FALSE);
        if (aid == 0xFF)
        {
            break;
        }
        stpOptMinComp->recvBit |= 1 << aid;
        if (DWCi_MatchDataFromLE(hostOrder, data[0]) != 0)
        {
            stpOptMinComp->timeoutBit |= 1 << aid;
        }
        break;

    case 19:
        DWCi_StopMatching(DWC_ERROR_MO_SC_CONNECT_BLOCK, 0);
        return FALSE;

    case 0x20:
        if (stpMatchCnt->qr2MatchType != 1 || stpMatchCnt->distantFriend != 0)
        {
            break;
        }
        for (i = 1; i <= stpMatchCnt->gt2NumConnection; i++)
        {
            if ((int)srcPid != stpMatchCnt->sbPidList[i])
            {
                continue;
            }
            DWC_Printf(0x40, "New client was accepted? - %d.\n",
                DWCi_MatchDataFromLE(hostOrder, data[0]));
            if (DWCi_MatchDataFromLE(hostOrder, data[0]) == 0)
            {
                if ((DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_OWN_NN)
                    || (DWCi_GetMatchCnt()->state
                        == DWC_MATCH_STATE_SV_OWN_GT2))
                {
                    result = DWCi_InvalidateReservation();
                    if (DWCi_HandleMatchCommandError(result))
                    {
                        return FALSE;
                    }
                }
                if (stpMatchCnt->nnInfo.cookie != 0)
                {
                    NNCancel(stpMatchCnt->nnInfo.cookie);
                    stpMatchCnt->nnInfo.cookie = 0;
                }
                DWCi_RestartFromTimeout();
            }
            else
            {
                stpMatchCnt->friendAcceptBit
                    |= 1 << DWCi_GetAIDFromProfileID(srcPid, FALSE);
            }
            break;
        }
        break;

    case 0x40:
        for (i = 1; i <= stpMatchCnt->gt2NumConnection; i++)
        {
            if ((int)srcPid != stpMatchCnt->sbPidList[i])
            {
                continue;
            }
            result = DWCi_SendMatchCommand(
                0x41, srcPid, srcIP, srcPort, NULL, 0);
            if (DWCi_HandleMatchCommandError(result))
            {
                return FALSE;
            }
            break;
        }
        break;

    case 0x41:
        break;

    default:
        DWC_Printf(2, "Received unexpected matching command 0x%02x.\n",
            command);
        break;
    }
    return TRUE;
}


static u8 DWCi_CheckResvCommand(
    int profileID, u32 qr2IP, u16 qr2Port, u32 matchType, BOOL priorFlag)
{
    u8 sendCommand;
    int result;

    switch (DWCi_GetMatchCnt()->qr2MatchType)
    {
    case DWC_MATCH_TYPE_FRIEND:
        if (!gpIsBuddy(DWCi_GetMatchCnt()->pGpObj, profileID))
        {
            sendCommand = DWC_MATCH_COMMAND_DUMMY;
            break;
        }
        else if (!DWCi_IsFriendByIdxList(profileID))
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "This friend doesn't exist in friendIdxList.\n");
            break;
        }

    case DWC_MATCH_TYPE_ANYBODY:
        if ((matchType != DWCi_GetMatchCnt()->qr2MatchType)
            || (DWCi_GetMatchCnt()->cancelState != DWC_MATCH_CANCEL_STATE_INIT)
            || (DWCi_GetMatchCnt()->qr2NNFinishCount == DWCi_GetMatchCnt()->qr2NumEntry)
            || (DWCi_GetMatchCnt()->qr2IsReserved
                && (DWCi_GetMatchCnt()->qr2Reservation == DWCi_GetMatchCnt()->profileID)))
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_DENY;

            if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
                && !DWCi_GetMatchCnt()->qr2Obj->userstatechangerequested
                && DWCi_GetMatchCnt()->qr2IsReserved
                && (DWCi_GetMatchCnt()->qr2Reservation == DWCi_GetMatchCnt()->profileID))
            {
                qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
            }
        }
        else if (((DWCi_GetMatchCnt()->state
                       != DWC_MATCH_STATE_CL_SEARCH_HOST)
                     && (DWCi_GetMatchCnt()->state
                         != DWC_MATCH_STATE_CL_WAIT_RESV))
            || ((DWCi_GetMatchCnt()->qr2IP == 0)
                && (DWCi_GetMatchCnt()->qr2Port == 0))
            || ((qr2IP == 0) && (qr2Port == 0)))
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
        }
        else
        {
            if (DWCi_GetMatchCnt()->reqProfileID)
            {
                if (DWCi_GetMatchCnt()->reqProfileID == profileID)
                {
                    if (priorFlag
                        || ((DWCi_GetMatchCnt()->profileID < profileID)
                            && (profileID != DWCi_GetMatchCnt()->priorProfileID)))
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
                    if (priorFlag
                        || ((DWCi_GetMatchCnt()->profileID < profileID)
                            && !DWCi_GetMatchCnt()->priorProfileID))
                    {
                        result = DWCi_CancelReservation(
                            DWCi_GetMatchCnt()->reqProfileID);
                        if (DWCi_HandleMatchCommandError(result))
                        {
                            return DWC_MATCH_COMMAND_DUMMY;
                        }
                        else
                        {
                            sendCommand = DWC_MATCH_COMMAND_RESV_OK;
                        }
                    }
                    else
                    {
                        sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
                    }
                }
            }
            else
            {
                sendCommand = DWC_MATCH_COMMAND_RESV_OK;
            }
        }
        break;

    case DWC_MATCH_TYPE_SC_SV:
        if (!gpIsBuddy(DWCi_GetMatchCnt()->pGpObj, profileID))
        {
            sendCommand = DWC_MATCH_COMMAND_DUMMY;
        }
        else if ((matchType != DWC_MATCH_TYPE_SC_CL)
            || (DWCi_GetMatchCnt()->qr2NNFinishCount == DWCi_GetMatchCnt()->qr2NumEntry))
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_DENY;
        }
        else if ((stOptSCBlock.valid == 1) && (stOptSCBlock.lock == 1))
        {
            sendCommand = DWC_MATCH_COMMAND_SC_CONN_BLOCK;
        }
        else if ((DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_SV_WAITING)
            || ((DWCi_GetMatchCnt()->qr2IP == 0)
                && (DWCi_GetMatchCnt()->qr2Port == 0))
            || ((qr2IP == 0) && (qr2Port == 0)))
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_WAIT;
        }
        else
        {
            sendCommand = DWC_MATCH_COMMAND_RESV_OK;
        }
        break;
    }

    return sendCommand;
}

static int DWCi_ProcessResvOK(int profileID, u32 ip, u16 port)
{
    u32 senddata[2];
    int result;
    int i;

    if (DWCi_GetMatchCnt()->qr2IsReserved
        && (DWCi_GetMatchCnt()->qr2Reservation == profileID))
    {
        return 0;
    }

    DWCi_GetMatchCnt()->qr2IsReserved = 1;
    DWCi_GetMatchCnt()->qr2Reservation = profileID;
    DWCi_GetMatchCnt()->cmdResendFlag = FALSE;
    DWCi_GetMatchCnt()->cmdTimeoutTime = 0;
    qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
    DWCi_GetMatchCnt()->reqProfileID = 0;
    DWCi_GetMatchCnt()->sbPidList[DWCi_GetMatchCnt()->qr2NNFinishCount + 1] = profileID;
    DWCi_GetMatchCnt()->qr2IPList[DWCi_GetMatchCnt()->qr2NNFinishCount + 1] = ip;
    DWCi_GetMatchCnt()->qr2PortList[DWCi_GetMatchCnt()->qr2NNFinishCount + 1] = port;
    DWCi_GetMatchCnt()->searchIP = ip;
    DWCi_GetMatchCnt()->searchPort = port;

    DWCi_GetMatchCnt()->aidList[DWCi_GetMatchCnt()->qr2NNFinishCount + 1]
        = DWCi_GetAIDFromList();

    senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_NEW_PID_AID, (u32)profileID);
    senddata[1] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_NEW_PID_AID,
        DWCi_GetMatchCnt()->aidList[DWCi_GetMatchCnt()->qr2NNFinishCount + 1]);

    for (i = 1; i <= DWCi_GetMatchCnt()->qr2NNFinishCount; i++)
    {
        result = DWCi_SendMatchCommand(7, DWCi_GetMatchCnt()->sbPidList[i],
            DWCi_GetMatchCnt()->qr2IPList[i], DWCi_GetMatchCnt()->qr2PortList[i],
            senddata, 2);
        if (result)
        {
            return result;
        }
    }

    DWCi_InitOptMinCompParam(TRUE);

    return 0;
}

static void DWCi_MakeBackupServerData(int profileID, const u32 data[])
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

static int DWCi_SendResvCommand(int profileID, BOOL delay)
{
    u8 command;
    u32 senddata[3];
    int sendlen;
    int result;
    SBServer server;

    if (delay
        || ((DWCi_GetMatchCnt()->qr2IP == 0)
            && (DWCi_GetMatchCnt()->qr2Port == 0)))
    {
        DWCi_GetMatchCnt()->cmdResendFlag = TRUE;
        DWCi_GetMatchCnt()->cmdResendTick = DWCi_Np_GetTick();
        DWCi_GetMatchCnt()->sbPidList[0] = profileID;

        if (!delay)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "Delay ResvCommand - qr2IP & qr2Port = 0.\n");
        }
        return 0;
    }

    if (DWCi_GetMatchCnt()->priorProfileID)
    {
        command = DWC_MATCH_COMMAND_RESV_PRIOR;
    }
    else
    {
        command = DWC_MATCH_COMMAND_RESERVATION;
    }

    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
    {
        server = ServerBrowserGetServer(DWCi_GetMatchCnt()->sbObj, 0);
        DWCi_GetMatchCnt()->sbPidList[0]
            = SBServerGetIntValueA(server, DWC_QR2_PID_KEY_STR, 0);
        DWCi_GetMatchCnt()->qr2IPList[0] = SBServerGetPublicInetAddress(server);
        DWCi_GetMatchCnt()->qr2PortList[0] = SBServerGetPublicQueryPort(server);

        DWCi_GetMatchCnt()->reqProfileID = DWCi_GetMatchCnt()->sbPidList[0];
        sendlen = 1;
    }
    else
    {
        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
        {
            DWCi_GetMatchCnt()->sbPidList[0] = profileID;
        }

        DWCi_GetMatchCnt()->reqProfileID = profileID;

        senddata[1] = DWCi_MatchIPToLE(command, DWCi_GetMatchCnt()->qr2IP);
        senddata[2] = DWCi_MatchDataToLE(command, DWCi_GetMatchCnt()->qr2Port);
        sendlen = 3;
    }

    DWCi_GetMatchCnt()->cmdTimeoutTime = DWC_MATCH_CMD_RESV_TIMEOUT_MSEC;
    DWCi_GetMatchCnt()->cmdTimeoutStartTick = DWCi_Np_GetTick();
    DWCi_GetMatchCnt()->cmdResendFlag = FALSE;

    senddata[0] = DWCi_MatchDataToLE(command, DWCi_GetMatchCnt()->qr2MatchType);

    result = DWCi_SendMatchCommand(command, profileID,
        DWCi_GetMatchCnt()->qr2IPList[0], DWCi_GetMatchCnt()->qr2PortList[0], senddata,
        sendlen);

    return result;
}

static int DWCi_SendResvCommandToFriend(BOOL delay, BOOL init, int resendPid)
{
    char version[12];
    char numEntry[4];
    char distantFriend[2];
    int profileID;
    int startIdx = init ? DWCi_GetMatchCnt()->friendCount
                        : (DWCi_GetMatchCnt()->friendCount
                                  < DWCi_GetMatchCnt()->friendIdxListLen - 1)
            ? DWCi_GetMatchCnt()->friendCount + 1
            : 0;
    int buddyIdx;
    int initFinished = 0;
    int versionLen, numEntryLen, distantFriendLen;
    int i;
    GPBuddyStatus status;
    GPResult gpResult;

    while (1)
    {
        if (!init || initFinished)
        {
            DWCi_GetMatchCnt()->friendCount++;
            if (DWCi_GetMatchCnt()->friendCount >= DWCi_GetMatchCnt()->friendIdxListLen)
            {
                DWCi_GetMatchCnt()->friendCount = 0;
            }
        }

        if (initFinished)
        {
            if (DWCi_GetMatchCnt()->friendCount == startIdx)
            {
                DWCi_GetMatchCnt()->cmdTimeoutTime
                    = DWC_MATCH_CMD_RESEND_INTERVAL_MSEC;
                DWCi_GetMatchCnt()->cmdTimeoutStartTick = DWCi_Np_GetTick();
                DWCi_GetMatchCnt()->cmdResendFlag = FALSE;
                return 0;
            }
        }
        initFinished = 1;

        profileID = DWC_GetGsProfileId(DWCi_GetUserData(),
            (DWCAccFriendData*)((u8*)DWCi_GetMatchCnt()->friendList
                + DWCi_GetMatchCnt()
                          ->friendIdxList[DWCi_GetMatchCnt()->friendCount]
                    * sizeof(DWCAccFriendData)));

        if ((profileID == 0) || (profileID == -1))
            continue;

        if (!DWCi_Acc_IsValidFriendData(
                (DWCAccFriendData*)((u8*)DWCi_GetMatchCnt()->friendList
                    + DWCi_GetMatchCnt()
                              ->friendIdxList[DWCi_GetMatchCnt()->friendCount]
                        * sizeof(DWCAccFriendData))))
            continue;

        for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
        {
            if (DWCi_GetMatchCnt()->sbPidList[i] == profileID)
                break;
        }

        if (i > DWCi_GetMatchCnt()->gt2NumConnection)
        {
            gpResult = gpGetBuddyIndex(
                DWCi_GetMatchCnt()->pGpObj, profileID, &buddyIdx);
            gpResult |= gpGetBuddyStatus(
                DWCi_GetMatchCnt()->pGpObj, buddyIdx, &status);

            if (!gpResult && (status.status == DWC_STATUS_MATCH_FRIEND))
            {
                versionLen = DWC_GetCommonValueString(
                    DWC_GP_SSTR_KEY_MATCH_VERSION, version,
                    status.statusString, '/');
                numEntryLen = DWC_GetCommonValueString(
                    DWC_GP_SSTR_KEY_MATCH_FRIEND_NUM, numEntry,
                    status.statusString, '/');
                distantFriendLen = DWC_GetCommonValueString(
                    DWC_GP_SSTR_KEY_DISTANT_FRIEND, distantFriend,
                    status.statusString, '/');

                if ((versionLen > 0) && (numEntryLen > 0)
                    && (distantFriendLen > 0)
                    && (strtoul(version, NULL, 10)
                        == DWC_MATCHING_VERSION)
                    && (strtoul(numEntry, NULL, 10)
                        == DWCi_GetMatchCnt()->qr2NumEntry))
                {
                    break;
                }
            }
        }
    }

    if (profileID == resendPid)
        delay = TRUE;

    return DWCi_SendResvCommand(profileID, delay);
}

static int DWCi_HandleMatchCommandError(int error)
{
    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
    {
        return (int)DWCi_HandleSBError((SBError)error);
    }
    else
    {
        return (int)DWCi_HandleGPError((GPResult)error);
    }
}

static BOOL DWCi_RetryReserving(int resendPid)
{
    SBError sbError;
    int result;

    DWCi_GetMatchCnt()->priorProfileID = 0;
    DWCi_GetMatchCnt()->reqProfileID = 0;
    DWCi_GetMatchCnt()->resvWaitCount = 0;
    DWCi_GetMatchCnt()->cmdTimeoutStartTick = DWCi_Np_GetTick();

    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
    {
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);

        sbError = DWCi_SBUpdateAsync(0);
        if (DWCi_HandleSBError(sbError))
        {
            return FALSE;
        }
    }
    else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
    {
        result = DWCi_SendResvCommandToFriend(FALSE, FALSE, resendPid);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
    }
    else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
    {
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_OTHER);
        return FALSE;
    }

    return TRUE;
}

static int DWCi_CancelReservation(int profileID)
{
    int result;

    result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_RESV_CANCEL, profileID,
        DWCi_GetMatchCnt()->qr2IPList[0], DWCi_GetMatchCnt()->qr2PortList[0], NULL, 0);

    DWCi_GetMatchCnt()->reqProfileID = 0;

    return result;
}

static BOOL DWCi_CancelPreConnectedServerProcess(int clientPid)
{
    int i;
    BOOL isServer;

    if (DWCi_GetMatchCnt()->qr2IsReserved
        && DWCi_GetMatchCnt()->qr2Reservation
            == DWCi_GetMatchCnt()->profileID)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "DWCi_CancelPreConnectedServerProcess : client\n");
        isServer = FALSE;
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "DWCi_CancelPreConnectedServerProcess : server\n");
        isServer = TRUE;
    }

    if (isServer)
    {
        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
        qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);
    }

    if (DWCi_GetMatchCnt()->gt2NumConnection < DWC_MAX_CONNECTIONS - 1)
    {
        DWCi_GetMatchCnt()->sbPidList[
            DWCi_GetMatchCnt()->gt2NumConnection + 1] = 0;
    }

    DWCi_GetMatchCnt()->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;

    if (DWCi_GetMatchCnt()->nnInfo.cookie)
    {
        NNCancel(DWCi_GetMatchCnt()->nnInfo.cookie);
        DWCi_GetMatchCnt()->nnInfo.cookie = 0;
    }

    DWCi_GetMatchCnt()->qr2NNFinishCount
        = (u8)DWCi_GetMatchCnt()->gt2NumConnection;
    DWCi_GetMatchCnt()->reqProfileID = 0;

    if (!isServer)
    {
        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        {
        }
        else
        {
            DWCi_RestartFromTimeout();
        }
    }
    else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
    {
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);
        DWCi_GetMatchCnt()->sbUpdateFlag = DWC_SB_UPDATE_INTERVAL_LONG;
        DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
    }
    else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND)
    {
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
        (void)DWCi_SendResvCommandToFriend(TRUE, FALSE, 0);
    }
    else if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV)
    {
        DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_CANCEL_SYN);
        DWCi_GetMatchCnt()->cancelSynAckBit = 0;
        DWCi_GetMatchCnt()->cancelBaseLatency = 0;

        DWCi_CloseCancelHostAsync(clientPid);

        for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
        {
            if (!DWCi_SendCancelMatchSynCommand(
                    DWCi_GetMatchCnt()->sbPidList[i],
                    DWC_MATCH_COMMAND_CANCEL_SYN))
                return FALSE;
        }

        if (DWCi_GetMatchCnt()->gt2NumConnection == 0)
        {
            DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
        }
    }

    return TRUE;
}

static BOOL DWCi_CancelPreConnectedClientProcess(int serverPid)
{
#pragma unused(serverPid)
    BOOL result = TRUE;

    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
    {
        if (DWCi_GetMatchCnt()->gt2NumConnection)
        {
            DWCi_CloseAllConnectionsByTimeout();
        }

        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_TYPE_SC_CL_FAIL);
        return FALSE;
    }

    DWCi_GetMatchCnt()->qr2NNFinishCount = (u8)DWCi_GetMatchCnt()->gt2NumConnection;
    DWCi_GetMatchCnt()->priorProfileID = 0;

    if (DWCi_GetMatchCnt()->nnInfo.cookie)
    {
        NNCancel(DWCi_GetMatchCnt()->nnInfo.cookie);
        DWCi_GetMatchCnt()->nnInfo.cookie = 0;
    }

    if (DWCi_GetMatchCnt()->gt2NumConnection)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "Close all connection and restart matching.\n");

        DWCi_RestartFromTimeout();
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
            "Cancel and restart client process.\n");

        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);

        DWC_Printf(
            DWC_REPORTFLAG_MATCH_NN, "Cancel and retry to reserve.\n");

        result = DWCi_RetryReserving(0);
    }

    return result;
}

// defined here (function and string order), not inlined
static int DWCi_ChangeToClient(void)
{
    u32 leData[128];
    const u32* senddata;
    int result;
    int i;
    int len;

    // the server data goes little-endian to anybody-match clients (SB messages)
    if (stpMatchCnt->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
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

    for (i = 1; i <= stpMatchCnt->gt2NumConnection; i++)
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

static void DWCi_PostProcessConnection(DWCMatchPpConnectionType type)
{
    u32 senddata[5];
    int sendlen = 3;
    int result;
    int i;
    BOOL sbClear = FALSE;
    GPResult gpResult;

    switch (type)
    {
    case DWC_PP_CONNECTION_SV_CONNECT:
        if (DWCi_GetMatchCnt()->clLinkProgress
            < DWCi_GetMatchCnt()->gt2NumConnection - 1)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Send client-client link request.\n");

            DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_WAIT_CL_LINK);

            senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, (u32)DWCi_GetMatchCnt()
                    ->sbPidList[DWCi_GetMatchCnt()->clLinkProgress + 1]);
            senddata[1] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ,
                (u32)(DWCi_GetMatchCnt()->clLinkProgress + 1));
            senddata[2] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, DWCi_GetMatchCnt()
                    ->aidList[DWCi_GetMatchCnt()->clLinkProgress + 1]);
            senddata[3] = DWCi_MatchIPToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, DWCi_GetMatchCnt()
                    ->qr2IPList[DWCi_GetMatchCnt()->clLinkProgress + 1]);
            senddata[4] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, DWCi_GetMatchCnt()
                    ->qr2PortList[DWCi_GetMatchCnt()->clLinkProgress + 1]);
            sendlen = 5;
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "Tell new client completion of matching.\n");

            DWCi_GetMatchCnt()->qr2IsReserved = 0;
            DWCi_GetMatchCnt()->qr2Reservation = 0;

            qr2_send_statechanged(DWCi_GetMatchCnt()->qr2Obj);

            if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_SEARCH_HOST);
            }
            else if (DWCi_GetMatchCnt()->qr2MatchType
                == DWC_MATCH_TYPE_FRIEND)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAIT_RESV);
            }
            else
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_WAITING);
            }

            DWCi_GetMatchCnt()->clLinkProgress = 0;

            if ((DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_SC_SV)
                || (DWCi_GetMatchCnt()->gt2NumConnection
                    == DWCi_GetMatchCnt()->qr2NumEntry))
            {
                if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_SC_SV)
                {
                    DWCi_GetMatchCnt()->cbEventPid = DWCi_GetMatchCnt()
                            ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection];
                }
                else
                {
                    DWCi_GetMatchCnt()->cbEventPid = 0;
                    DWCi_GetMatchCnt()->sbPidList[0]
                        = DWCi_GetMatchCnt()->profileID;
                }

                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_SYN);
                DWCi_GetMatchCnt()->synAckBit = 0;

                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
                {
                    DWCi_SendMatchSynPacket(DWCi_GetMatchCnt()->aidList[i],
                        DWC_SEND_TYPE_MATCH_SYN);
                }
            }
            else
            {
                senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, 0);
                senddata[1] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ,
                    (u8)DWCi_GetMatchCnt()->gt2NumConnection);
                senddata[2] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_REQ, DWCi_GetMatchCnt()
                        ->aidList[DWCi_GetMatchCnt()->gt2NumConnection]);

                if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_ANYBODY)
                {
                    DWCi_GetMatchCnt()->sbUpdateFlag
                        = DWC_SB_UPDATE_INTERVAL_LONG;
                    DWCi_GetMatchCnt()->sbUpdateTick = DWCi_Np_GetTick();
                }
                else if (DWCi_GetMatchCnt()->qr2MatchType
                    == DWC_MATCH_TYPE_FRIEND)
                {
                    (void)DWCi_SendResvCommandToFriend(TRUE, FALSE, 0);

#ifdef DWC_LIMIT_FRIENDS_MATCH_VALID
                    if (!DWCi_GetMatchCnt()->distantFriend
                        && (DWCi_GetMatchCnt()->gt2NumConnection >= 2))
                    {
                        if (DWCi_GetMatchCnt()->friendAcceptBit
                            != (DWCi_GetAIDBitmask(FALSE)
                                & ~(1
                                    << DWCi_GetMatchCnt()
                                           ->aidList[DWCi_GetMatchCnt()
                                                   ->gt2NumConnection])))
                        {
                            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                                "FRIEND_ACCEPT command droped.\n");
                            DWCi_RestartFromTimeout();
                            break;
                        }
                    }
#endif
                }
            }

            if (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_SC_SV)
            {
                sbClear = TRUE;
            }
        }

        if (DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_SV_SYN)
        {
            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_LINK_CLS_REQ,
                DWCi_GetMatchCnt()
                    ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection],
                DWCi_GetMatchCnt()
                    ->qr2IPList[DWCi_GetMatchCnt()->gt2NumConnection],
                DWCi_GetMatchCnt()
                    ->qr2PortList[DWCi_GetMatchCnt()->gt2NumConnection],
                senddata, sendlen);
            if (DWCi_HandleMatchCommandError(result))
                return;

            DWCi_GetMatchCnt()->cmdCnt.count = 0;
        }
        break;

    case DWC_PP_CONNECTION_CL_GT2_CONNECT:
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAITING);

        if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        {
            DWCi_GetMatchCnt()->cbEventPid = DWCi_GetMatchCnt()
                    ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection];
        }

        sbClear = TRUE;
        break;

    case DWC_PP_CONNECTION_CL_GT2_ACCEPT:
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAITING);

        if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
            || (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND))
        {
            DWCi_GetMatchCnt()->qr2IsReserved = 1;
            DWCi_GetMatchCnt()->qr2Reservation = DWCi_GetMatchCnt()->profileID;
        }

        DWCi_InitClWaitTimeout();

        if (DWCi_GetMatchCnt()->gt2NumConnection > 1)
        {
            u32 senddata[1];

            senddata[0] = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_LINK_CLS_SUC, (u32)DWCi_GetMatchCnt()
                    ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection - 1]);
            result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_LINK_CLS_SUC,
                DWCi_GetMatchCnt()->sbPidList[0],
                DWCi_GetMatchCnt()->qr2IPList[0],
                DWCi_GetMatchCnt()->qr2PortList[0], senddata, 1);
            if (DWCi_HandleMatchCommandError(result))
                return;
        }
        break;

    case DWC_PP_CONNECTION_CL_FINISH_CONNECT:
        DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAITING);
        DWCi_InitClWaitTimeout();
        DWCi_GetMatchCnt()->cbEventPid = 0;
        sbClear = TRUE;
        break;

    case DWC_PP_CONNECTION_SYN_FINISH:
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Completed matching!\n");

        if (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_SC_SV)
        {
            (void)DWCi_SetGPStatus(DWC_STATUS_PLAYING, "", NULL);
        }

        DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, FALSE,
            DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
            DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
            DWCi_GetMatchCnt()->matchedParam);

        if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY)
            || (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_FRIEND))
        {
            DWCi_CloseMatching();
        }
        else
        {
#ifndef DWC_BUG_WIFI_DELAY_SEND
            if (DWCi_GetMatchCnt()->sbObj)
            {
                ServerBrowserFree(DWCi_GetMatchCnt()->sbObj);
                DWCi_GetMatchCnt()->sbObj = NULL;
            }
#endif

            DWCi_NNFreeNegotiateList();

            if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV)
            {
                gpResult = DWCi_GPSetServerStatus();
                if (DWCi_HandleGPError(gpResult))
                    return;

                if (stOptSCBlock.valid == 1)
                {
                    stOptSCBlock.lock = 1;
                    DWC_Printf(DWC_REPORTFLAG_DEBUG,
                        "[OPT_SC_BLOCK] Connect block start!\n");
                }

                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_WAITING);
            }
            else
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_WAITING);
            }

            DWCi_GetMatchCnt()->cbEventPid = 0;
        }

        DWCi_GetMatchCnt()->cancelState = DWC_MATCH_CANCEL_STATE_INIT;
        break;
    }

#ifndef DWC_BUG_WIFI_DELAY_SEND
    if (sbClear
        && (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_SC_CL))
    {
        ServerBrowserClear(DWCi_GetMatchCnt()->sbObj);
    }
#endif
}

static void DWCi_DoCancelMatching(void)
{
    DWC_Printf(4, "CANCEL! state %d, numHost nn=%d gt2=%d.\n",
        stpMatchCnt->state, stpMatchCnt->qr2NNFinishCount, stpMatchCnt->gt2NumConnection);
    stpMatchCnt->cbEventPid = 0;

    if (stpMatchCnt->state == 2)
    {
        DWCi_FinishCancelMatching();
        return;
    }

    stpMatchCnt->cancelState = 1;
    if (stpMatchCnt->qr2MatchType == 3)
    {
        stpMatchCnt->cancelState = 1;
        if (stpMatchCnt->gt2NumConnection != 0)
        {
            gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
        }
        else
        {
            GPResult result = DWCi_SendMatchCommand(5, stpMatchCnt->sbPidList[0],
                stpMatchCnt->qr2IPList[0], stpMatchCnt->qr2PortList[0], NULL, 0);
            stpMatchCnt->reqProfileID = 0;
            if (stpMatchCnt->qr2MatchType == 0)
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
        DWCi_FinishCancelMatching();
        return;
    }

    if (stpMatchCnt->state == 4 || stpMatchCnt->state == 5
        || stpMatchCnt->state == 6 || stpMatchCnt->state == 7
        || stpMatchCnt->state == 11 || stpMatchCnt->state == 12)
    {
        GPResult result;

        if (stpMatchCnt->reqProfileID != 0)
        {
            result = DWCi_SendMatchCommand(5, stpMatchCnt->sbPidList[0],
                stpMatchCnt->qr2IPList[0], stpMatchCnt->qr2PortList[0], NULL, 0);
            stpMatchCnt->reqProfileID = 0;
            if (stpMatchCnt->qr2MatchType == 0)
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

        {
            u32 buf = DWCi_MatchDataToLE(DWC_MATCH_COMMAND_CANCEL, 0);

            result = GP_NO_ERROR;
            if (stpMatchCnt->qr2IsReserved != 0 && stpMatchCnt->qr2Reservation != 0
                && stpMatchCnt->qr2Reservation != stpMatchCnt->profileID)
            {
                result = DWCi_SendMatchCommand(12, stpMatchCnt->qr2Reservation,
                    stpMatchCnt->qr2IPList[stpMatchCnt->gt2NumConnection + 1],
                    stpMatchCnt->qr2PortList[stpMatchCnt->gt2NumConnection + 1], &buf, 1);
                stpMatchCnt->qr2IsReserved = 0;
                stpMatchCnt->qr2Reservation = 0;
            }
            if (stpMatchCnt->qr2MatchType == 0)
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

    if (stpMatchCnt->nnInfo.cookie != 0)
    {
        NNCancel(stpMatchCnt->nnInfo.cookie);
        stpMatchCnt->nnInfo.cookie = 0;
    }
    if (stpMatchCnt->gt2NumConnection != 0)
    {
        stpMatchCnt->cancelState = 1;
        gt2CloseAllConnectionsHard(*stpMatchCnt->pGt2Socket);
    }
    DWCi_FinishCancelMatching();
}

static void DWCi_FinishCancelMatching(void)
{
    GPResult gpResult;

    gpResult = DWCi_SetGPStatus(1, "", NULL);
    if (DWCi_HandleGPError(gpResult))
    {
        return;
    }

    DWCi_CloseMatching();
    DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
        DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE,
        DWCi_GetMatchCnt()->cbEventPid
            ? TRUE
            : ((DWCi_GetMatchCnt()->qr2MatchType == 2) ? TRUE : FALSE),
        DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
        DWCi_GetMatchCnt()->matchedParam);

    DWCi_GetMatchCnt()->cancelState = 0;
}

static int DWCi_InvalidateReservation(void)
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
                DWCi_GetMatchCnt()->gt2NumConnection + 1],
            DWCi_GetMatchCnt()->qr2PortList[
                DWCi_GetMatchCnt()->gt2NumConnection + 1],
            &senddata, 1);

        DWCi_GetMatchCnt()->qr2IsReserved = 0;
        DWCi_GetMatchCnt()->qr2Reservation = 0;
    }

    return result;
}

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

        if (DWCi_GetMatchCnt()->qr2MatchType == 2
            || DWCi_GetMatchCnt()->qr2MatchType == 3)
        {
            DWCi_GetMatchCnt()->matchedCallback(DWC_ERROR_NONE, TRUE,
                DWCi_GetMatchCnt()->cbEventPid ? FALSE : TRUE, FALSE,
                DWCi_GetFriendListIndex(DWCi_GetMatchCnt()->cbEventPid),
                DWCi_GetMatchCnt()->matchedParam);
        }
        else if (DWCi_GetMatchCnt()->qr2MatchType == 0)
        {
            if (level == DWC_MATCH_RESET_RESTART)
            {
                sbError = DWCi_SBUpdateAsync(0);
                if (DWCi_HandleSBError(sbError))
                {
                    return;
                }
            }
        }
        else if (DWCi_GetMatchCnt()->qr2MatchType == 1)
        {
            if (level == DWC_MATCH_RESET_RESTART)
            {
                (void)DWCi_SendResvCommandToFriend(FALSE, FALSE, 0);
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

static void DWCi_RestartFromTimeout(void)
{
    if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV
        || DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
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

static int DWCi_ResumeMatching(void)
{
    int result;

    stpMatchCnt->qr2IsReserved = 0;
    stpMatchCnt->qr2Reservation = 0;
    stpMatchCnt->closeState = 0;

    if (stpMatchCnt->priorProfileID != 0)
    {
        if (stpMatchCnt->qr2MatchType == 0)
        {
            stpMatchCnt->state = 3;
            result = DWCi_SBUpdateAsync(0);
            if (DWCi_HandleSBError(result) != 0)
            {
                return result;
            }
        }
        else if (stpMatchCnt->qr2MatchType == 1)
        {
            int error;

            stpMatchCnt->state = 4;
            result = DWCi_SendResvCommand(stpMatchCnt->priorProfileID, FALSE);
            if (stpMatchCnt->qr2MatchType == 0)
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

static BOOL DWCi_CloseCancelHostAsync(int profileID)
{
    BOOL ret;
    GT2Connection* connection;

    DWCi_GetMatchCnt()->cbEventPid = profileID;
    connection = DWCi_GetGT2ConnectionByProfileID(
        profileID, DWCi_GetMatchCnt()->gt2NumConnection + 1);

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
            profileID, DWCi_GetMatchCnt()->gt2NumConnection + 1);
        ret = FALSE;
    }

    return ret;
}

static BOOL DWCi_CloseShutdownClientSC(u32 bitmap)
{
    u8 aid;
    int senddata[DWC_MAX_CONNECTIONS];
    int senddataLen = 0;
    int closePid[DWC_MAX_CONNECTIONS]; // senddata may be byte-swapped
    int validPid[DWC_MAX_CONNECTIONS];
    int validPidLen = 0;
    int result;
    int i;

    for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
    {
        if (bitmap & (1 << DWCi_GetMatchCnt()->aidList[i]))
        {
            validPid[validPidLen] = DWCi_GetMatchCnt()->sbPidList[i];
            validPidLen++;
        }
        else
        {
            senddata[senddataLen] = DWCi_MatchDataToLE(
                DWC_MATCH_COMMAND_SC_CLOSE_CL, DWCi_GetMatchCnt()->sbPidList[i]);
            closePid[senddataLen] = DWCi_GetMatchCnt()->sbPidList[i];
            senddataLen++;
        }
    }

    for (i = 0; i < validPidLen; i++)
    {
        result = DWCi_SendMatchCommand(DWC_MATCH_COMMAND_SC_CLOSE_CL,
            validPid[i], 0, 0, (u32*)senddata, senddataLen);
        if (DWCi_HandleMatchCommandError(result))
        {
            return FALSE;
        }
    }

    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_TIMEOUT;

    for (i = 0; i < senddataLen; i++)
    {
        aid = DWCi_GetAIDFromProfileID(closePid[i], FALSE);
        if (aid != 0xff)
        {
            DWC_CloseConnectionHard(aid);
        }
    }

    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;

    return TRUE;
}

static void DWCi_SendMatchSynPacket(u8 aid, u16 type)
{
    u8 buf[4];

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Sent SYN %d packet to aid %d.\n",
        type - DWC_SEND_TYPE_MATCH_SYN, aid);
    switch (type)
    {
    case DWC_SEND_TYPE_MATCH_SYN:
    {
        u8 i;

        buf[0] = stpMatchCnt->aidList[stpMatchCnt->gt2NumConnection] == aid;
        for (i = 1; i <= stpMatchCnt->gt2NumConnection; i++)
        {
            if (stpMatchCnt->aidList[i] == aid)
            {
                buf[1] = i;
                buf[2] = aid;
                break;
            }
        }
        break;
    }

    case DWC_SEND_TYPE_MATCH_SYN_ACK:
        buf[0] = stpMatchCnt->baseLatency;
        buf[1] = (stpMatchCnt->baseLatency >> 8) & 0xFF;
        break;
    }
    DWCi_SendReliable(type, aid, buf, 4);
    stpMatchCnt->lastSynSent = OSGetTime();
}

static BOOL DWCi_ProcessMatchSynTimeout(void)
{
    u64 passTime;
    int i;

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_SYN
        || DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_SYN
        || DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_SYN_WAIT)
    {
        passTime = DWCi_Np_TicksToMilliSeconds(
            DWCi_Np_GetTick() - DWCi_GetMatchCnt()->lastSynSent);
    }
    else
    {
        return TRUE;
    }

    switch (DWCi_GetMatchCnt()->state)
    {
    case DWC_MATCH_STATE_CL_SYN:
        if (passTime > DWC_MATCH_SYN_ACK_WAIT_TIME)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                "[SYN] No ACK from server %d/%d.\n",
                DWCi_GetMatchCnt()->clWaitTimeoutCount,
                DWC_MATCH_CMD_RETRY_MAX);
            if (DWC_GetState() == DWC_STATE_MATCHING
                && DWCi_GetMatchCnt()->clWaitTimeoutCount
                    >= DWC_MATCH_CMD_RETRY_MAX)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "Timeout: [SYN] Connection to server was shut down.\n");

                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()->sbPidList[0]))
                    return FALSE;
            }
            else
            {
                DWCi_GetMatchCnt()->clWaitTimeoutCount++;

                DWCi_SendMatchSynPacket(DWCi_GetMatchCnt()->aidList[0],
                    DWC_SEND_TYPE_MATCH_SYN_ACK);
            }
        }
        break;

    case DWC_MATCH_STATE_SV_SYN:
        if (passTime > DWC_MATCH_SYN_ACK_WAIT_TIME)
        {
            DWCi_GetMatchCnt()->synResendCount++;
            if (DWCi_GetMatchCnt()->synResendCount > DWC_MATCH_CMD_RETRY_MAX)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: wait SYN-ACK (aidbitmap 0x%x). "
                    "Restart matching.\n",
                    DWCi_GetMatchCnt()->synAckBit);

                if (DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_ANYBODY
                    || DWCi_GetMatchCnt()->qr2MatchType
                        == DWC_MATCH_TYPE_FRIEND)
                {
                    DWCi_CloseAllConnectionsByTimeout();
                    DWCi_RestartFromCancel(DWC_MATCH_RESET_RESTART);
                }
                else
                {
                    if (!DWCi_CloseShutdownClientSC(
                            DWCi_GetMatchCnt()->synAckBit))
                        return FALSE;

                    if (DWCi_GetMatchCnt()->gt2NumConnection != 0)
                    {
                        DWCi_GetMatchCnt()->synResendCount = 0;
                        DWCi_GetMatchCnt()->lastSynSent = DWCi_Np_GetTick();
                    }
                    else
                    {
                        if (!DWCi_CancelPreConnectedServerProcess(
                                DWCi_GetMatchCnt()->cbEventPid))
                            return FALSE;
                    }
                }
            }
            else
            {
                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
                {
                    if (!(DWCi_GetMatchCnt()->synAckBit
                            & (1 << DWCi_GetMatchCnt()->aidList[i])))
                    {
                        DWCi_SendMatchSynPacket(
                            DWCi_GetMatchCnt()->aidList[i],
                            DWC_SEND_TYPE_MATCH_SYN);
                    }
                }
            }
        }
        break;

    case DWC_MATCH_STATE_SV_SYN_WAIT:
        if (passTime > DWCi_GetMatchCnt()->baseLatency)
        {
            DWCi_PostProcessConnection(DWC_PP_CONNECTION_SYN_FINISH);
        }
        break;
    }

    return TRUE;
}

static BOOL DWCi_SendCancelMatchSynCommand(int profileID, u8 command)
{
    u32 senddata;
    int len;
    int result;

    DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
        "Sent CANCEL SYN %d command to %u.\n",
        command - DWC_MATCH_COMMAND_CANCEL_SYN, profileID);

    if (command == DWC_MATCH_COMMAND_CANCEL_SYN)
    {
        senddata = DWCi_MatchDataToLE(command, (u32)DWCi_GetMatchCnt()->cbEventPid);
        len = 1;
    }
    else
    {
        len = 0;
    }

    result = DWCi_SendMatchCommand(
        command, profileID, 0, 0, &senddata, len);
    if (DWCi_HandleMatchCommandError(result))
        return FALSE;

    DWCi_GetMatchCnt()->lastCancelSynSent = DWCi_Np_GetTick();

    return TRUE;
}

static BOOL DWCi_ProcessCancelMatchSynCommand(
    int profileID, u8 command, u32 data)
{
    u8 aid;
    u32 bitmask;
    u64 baseTime, latency;
    int i;

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
        if (DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_CL_CANCEL_SYN)
        {
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_CANCEL_SYN);
            DWCi_CloseCancelHostAsync((int)data);
        }

        if (!DWCi_SendCancelMatchSynCommand(
                profileID, DWC_MATCH_COMMAND_CANCEL_SYN_ACK))
            return FALSE;
        break;

    case DWC_MATCH_COMMAND_CANCEL_SYN_ACK:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_CANCEL_SYN)
        {
            baseTime = DWCi_Np_GetTick();
            if ((DWCi_Np_TicksToMilliSeconds(
                     baseTime - DWCi_GetMatchCnt()->lastCancelSynSent)
                    >> 1)
                > DWC_GP_PROCESS_INTERVAL)
            {
                latency = (DWCi_Np_TicksToMilliSeconds(
                               baseTime - DWCi_GetMatchCnt()->lastCancelSynSent)
                              >> 1)
                    - DWC_GP_PROCESS_INTERVAL;
                if (latency > DWCi_GetMatchCnt()->cancelBaseLatency)
                {
                    DWCi_GetMatchCnt()->cancelBaseLatency = (u16)latency;
                }
            }

            aid = DWCi_GetAIDFromProfileID(profileID, FALSE);
            if (aid != 0xff)
            {
                DWCi_GetMatchCnt()->cancelSynAckBit |= 1 << aid;
            }

            bitmask = DWCi_GetAIDBitmask(TRUE);
            if (DWCi_GetMatchCnt()->cancelSynAckBit == bitmask)
            {
                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
                {
                    if (!DWCi_SendCancelMatchSynCommand(
                            DWCi_GetMatchCnt()->sbPidList[i],
                            DWC_MATCH_COMMAND_CANCEL_ACK))
                        return FALSE;
                }

                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT);

                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "Wait max latency %d msec.\n",
                    DWCi_GetMatchCnt()->cancelBaseLatency);
            }
        }
        else
        {
            if (!DWCi_SendCancelMatchSynCommand(
                    profileID, DWC_MATCH_COMMAND_CANCEL_ACK))
                return FALSE;
        }
        break;

    case DWC_MATCH_COMMAND_CANCEL_ACK:
        if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_CANCEL_SYN)
        {
            DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
        }
        break;
    }

    return TRUE;
}

static BOOL DWCi_ProcessCancelMatchSynTimeout(void)
{
    u64 passTime;
    int i;

    if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_CL_CANCEL_SYN
        || DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_CANCEL_SYN
        || DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT)
    {
        passTime = DWCi_Np_TicksToMilliSeconds(
            DWCi_Np_GetTick() - DWCi_GetMatchCnt()->lastCancelSynSent);
    }
    else
    {
        return TRUE;
    }

    switch (DWCi_GetMatchCnt()->state)
    {
    case DWC_MATCH_STATE_CL_CANCEL_SYN:
        if (passTime > DWC_MATCH_CANCEL_SYN_ACK_WAIT_TIME)
        {
            if (!DWCi_SendCancelMatchSynCommand(
                    DWCi_GetMatchCnt()->sbPidList[0],
                    DWC_MATCH_COMMAND_CANCEL_SYN_ACK))
                return FALSE;
        }
        break;

    case DWC_MATCH_STATE_SV_CANCEL_SYN:
        if (passTime > DWC_MATCH_CANCEL_SYN_ACK_WAIT_TIME)
        {
            DWCi_GetMatchCnt()->cancelSynResendCount++;
            if (DWCi_GetMatchCnt()->cancelSynResendCount
                > DWC_MATCH_CMD_RETRY_MAX)
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                    "Timeout: wait cancel SYN-ACK (aidbitmap 0x%x).\n",
                    DWCi_GetMatchCnt()->cancelSynAckBit);

                if (!DWCi_CloseShutdownClientSC(
                        DWCi_GetMatchCnt()->cancelSynAckBit))
                    return FALSE;

                if (DWCi_GetMatchCnt()->gt2NumConnection != 0)
                {
                    DWCi_GetMatchCnt()->cancelSynResendCount = 0;
                    DWCi_GetMatchCnt()->lastCancelSynSent = DWCi_Np_GetTick();
                }
                else
                {
                    DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
                }
            }
            else
            {
                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
                {
                    if (!(DWCi_GetMatchCnt()->cancelSynAckBit
                            & (1 << DWCi_GetMatchCnt()->aidList[i])))
                    {
                        if (!DWCi_SendCancelMatchSynCommand(
                                DWCi_GetMatchCnt()->sbPidList[i],
                                DWC_MATCH_COMMAND_CANCEL_SYN))
                            return FALSE;
                    }
                }
            }
        }
        break;

    case DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT:
        if (passTime > DWCi_GetMatchCnt()->cancelBaseLatency)
        {
            DWCi_RestartFromCancel(DWC_MATCH_RESET_CONTINUE);
        }
        break;
    }

    return TRUE;
}

static void DWCi_CloseAllConnectionsByTimeout(void)
{
    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_TIMEOUT;
    gt2CloseAllConnectionsHard(*DWCi_GetMatchCnt()->pGt2Socket);
    DWCi_GetMatchCnt()->closeState = DWC_MATCH_CLOSE_STATE_INIT;
}

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

static u8 DWCi_GetAIDFromList(void)
{
    u8 i;
    int j;

    for (i = 0; i < DWC_MAX_CONNECTIONS; i++)
    {
        for (j = 0; j <= DWCi_GetMatchCnt()->qr2NNFinishCount; j++)
        {
            if (DWCi_GetMatchCnt()->aidList[j] == i)
            {
                break;
            }
        }
        if (j > DWCi_GetMatchCnt()->qr2NNFinishCount)
        {
            break;
        }
    }

    return i;
}

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

static u8 DWCi_GetAIDFromProfileID(int profileID, BOOL idx0)
{
    int i;

    for (i = (idx0 ? 0 : 1); i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
    {
        if (DWCi_GetMatchCnt()->sbPidList[i] == profileID)
        {
            return DWCi_GetMatchCnt()->aidList[i];
        }
    }

    return 0xff;
}

static u32 DWCi_GetAIDBitmask(BOOL valid)
{
    u32 bitmask = 0;
    int i;

    if (valid)
    {
        return DWCi_GetMatchCnt()->validAidBitmap & ~1;
    }
    else
    {
        for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
        {
            bitmask |= 1 << DWCi_GetMatchCnt()->aidList[i];
        }
        return bitmask;
    }
}

static void DWCi_InitClWaitTimeout(void)
{
    DWCi_GetMatchCnt()->clWaitTimeoutCount = 0;
    DWCi_GetMatchCnt()->clWaitTime = DWCi_Np_GetTick();
}

static void DWCi_InitOptMinCompParam(BOOL reset)
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

static void DWCi_ProcessOptMinComp(void)
{
    u32 bitmask;
    int result;
    int i;

    if (!stpOptMinComp || !stpOptMinComp->valid
        || DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_SV
        || DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
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
                    = (u8)DWCi_GetMatchCnt()->gt2NumConnection;
                DWCi_GetMatchCnt()->clLinkProgress
                    = (u8)(DWCi_GetMatchCnt()->gt2NumConnection - 1);

                DWCi_PostProcessConnection(DWC_PP_CONNECTION_SV_CONNECT);
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                    "[OPT_MIN_COMP] Some clients is in time.\n");

                stpOptMinComp->lastPollTime = DWCi_Np_GetTick();
                stpOptMinComp->recvBit = 0;

                if (DWCi_GetMatchCnt()->qr2MatchType
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
                for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
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
        && DWCi_GetMatchCnt()->gt2NumConnection >= stpOptMinComp->minEntry - 1
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

        for (i = 1; i <= DWCi_GetMatchCnt()->gt2NumConnection; i++)
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

static NegotiateResult DWCi_HandleNNResult(NegotiateResult result)
{
    int type;
    int code;

    if (result == 0)
    {
        return 0;
    }
    DWC_Printf(8, "Match, NN result %d\n", result);
    switch (result)
    {
    case nr_deadbeatpartner:
        return 1;
    case nr_inittimeout:
        return 2;
    case nr_pingtimeout:
        DWC_Printf(8, "NN Ping Timeout\n");
        type = DWC_ERROR_NETWORK;
        code = -70;
        break;
    default:
        type = DWC_ERROR_NETWORK;
        code = -9;
        break;
    }
    if (type != 0)
    {
        DWCi_StopMatching(type,
            code + DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_NN);
    }
    return result;
}

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

static int DWCi_CheckDWCServer(SBServer server)
{
    if (SBServerGetIntValueA(server, "numplayers", -1) == -1)
    {
        return 0;
    }
    if (SBServerGetIntValueA(server, "maxplayers", -1) == -1)
    {
        return 0;
    }
    if (SBServerGetIntValueA(server, "dwc_mtype", -1) == -1)
    {
        return 0;
    }
    if (SBServerGetIntValueA(server, "dwc_mresv", -1) == -1)
    {
        if (SBServerGetIntValueA(server, "dwc_mresv", 0) == 0)
        {
            return 0;
        }
    }
    if (SBServerGetIntValueA(server, "dwc_mver", -1) == -1)
    {
        return 0;
    }
    return SBServerGetIntValueA(server, "dwc_pid", 0);
}

static void DWCi_SBCallback(ServerBrowser sb, SBCallbackReason reason,
    SBServer server,
    void* instance)
{
    int profileID;
    int result;
    int i;
    NegotiateError nnError;

    DWC_Printf(0x40, "SBCallback : reason %d (state = %d)\n", reason,
        stpMatchCnt->state);
    s_sbCallbackLevel++;

    switch (reason)
    {
    case sbc_serveradded:
        DWCi_SBPrintServerData(server);
        stpMatchCnt->sbUpdateRequestTick = OSGetTime()
            + (u64)(OS_BUS_CLOCK / 4 / 1000) * 30000;
        break;

    case sbc_updatecomplete:
        stpMatchCnt->sbUpdateRequestTick = 0;
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

        switch (stpMatchCnt->state)
        {
        case 2:
            for (i = 0; i < ServerBrowserCount(sb); i++)
            {
                server = ServerBrowserGetServer(sb, i);
                if (DWCi_GetMatchCnt()->qr2IP
                    && (DWCi_GetMatchCnt()->qr2IP
                        == SBServerGetPublicInetAddress(server))
                    && DWCi_GetMatchCnt()->qr2Port
                    && (DWCi_GetMatchCnt()->qr2Port
                        == SBServerGetPublicQueryPort(server)))
                {
                    break;
                }
            }
            if (i < ServerBrowserCount(sb))
            {
                stpMatchCnt->state = 3;
                stpMatchCnt->reqProfileID = 0;
                DWCi_HandleSBError(DWCi_SBUpdateAsync(stpMatchCnt->reqProfileID));
            }
            else
            {
                stpMatchCnt->sbUpdateFlag = 2;
                stpMatchCnt->sbUpdateTick = OSGetTime();
            }
            break;

        case 3:
            DWCi_EvaluateServers(1);
            DWCi_RandomizeServers();
            if (ServerBrowserCount(sb) != 0)
            {
                result = DWCi_SendResvCommand(0, FALSE);

                if (stpMatchCnt->qr2MatchType == 0)
                {
                    result = DWCi_HandleSBError(result);
                }
                else
                {
                    result = DWCi_HandleGPError(result);
                }
                if (result != 0)
                {
                    break;
                }
                stpMatchCnt->state = 4;
                stpMatchCnt->sbUpdateFlag = 0;
            }
            else
            {
                stpMatchCnt->sbUpdateFlag = 2;
                stpMatchCnt->sbUpdateTick = OSGetTime();
            }
            break;

        case 5:
        {
            DWC_Printf(4, "searchIP: %d.%d.%d.%d, searchPort: %d\n", stpMatchCnt->searchIP >> 24,
                (stpMatchCnt->searchIP >> 16) & 0xff, (stpMatchCnt->searchIP >> 8) & 0xff,
                stpMatchCnt->searchIP & 0xff, stpMatchCnt->searchPort); // four bytes
            while (ServerBrowserCount(sb) != 0)
            {
                server = ServerBrowserGetServer(sb, 0);
                if ((SBServerGetPublicInetAddress(server)
                        == DWCi_GetMatchCnt()->searchIP)
                    && (SBServerGetPublicQueryPort(server)
                        == DWCi_GetMatchCnt()->searchPort))
                {
                    break;
                }
                else
                {
                    ServerBrowserRemoveServer(sb, server);
                }
            }
            if (ServerBrowserCount(sb) != 0)
            {
                profileID = SBServerGetIntValueA(
                    ServerBrowserGetServer(sb, 0), "dwc_pid", 0);
                if (stpMatchCnt->qr2MatchType == 1
                    && profileID == stpMatchCnt->sbPidList[0])
                {
                    if (DWCi_EvaluateServers(0) != 0)
                    {
                        if (stpMatchCnt->gt2NumConnection != 0)
                        {
                            result = DWCi_ChangeToClient();
                            if (stpMatchCnt->qr2MatchType == 0)
                            {
                                result = DWCi_HandleSBError(result);
                            }
                            else
                            {
                                result = DWCi_HandleGPError(result);
                            }
                            if (result != 0)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        result = DWCi_SendMatchCommand(5, stpMatchCnt->sbPidList[0],
                            stpMatchCnt->qr2IPList[0], stpMatchCnt->qr2PortList[0],
                            NULL, 0);
                        stpMatchCnt->reqProfileID = 0;
                        if (stpMatchCnt->qr2MatchType == 0)
                        {
                            result = DWCi_HandleSBError(result);
                        }
                        else
                        {
                            result = DWCi_HandleGPError(result);
                        }
                        if (result != 0)
                        {
                            break;
                        }
                        stpMatchCnt->state = 4;
                        result = DWCi_SendResvCommandToFriend(
                            0, 0, stpMatchCnt->sbPidList[0]);
                        if (stpMatchCnt->qr2MatchType == 0)
                        {
                            DWCi_HandleSBError(result);
                        }
                        else
                        {
                            DWCi_HandleGPError(result);
                        }
                        break;
                    }
                }
                stpMatchCnt->state = 6;
                nnError = DWCi_NNStartupAsync(0, 0, ServerBrowserGetServer(sb, 0));
                DWCi_HandleNNError(nnError);
            }
            else
            {
                stpMatchCnt->sbUpdateFlag = 2;
                stpMatchCnt->sbUpdateTick = OSGetTime();
            }
            break;
        }

        default:
            stpMatchCnt->sbUpdateRequestTick = OSGetTime()
                + (u64)(OS_BUS_CLOCK / 4 / 1000) * 30000;
            break;
        }
        break;

    case sbc_queryerror:
        break;

    default:
        break;
    }

    s_sbCallbackLevel--;
}

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
    DWC_Printf(0x400, "%s   = %u\n", "dwc_mresv",
        SBServerGetIntValueA(server, "dwc_mresv", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_mtype",
        SBServerGetIntValueA(server, "dwc_mtype", -1));
    DWC_Printf(0x400, "%s   = %d\n", "dwc_mver",
        SBServerGetIntValueA(server, "dwc_mver", -1));

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

static int DWCi_EvaluateServers(int sort)
{
    int eval;
    int deleteFlag = FALSE;
    int localDelete;
    int profileID;
    int i;
    int j;
    SBServer server;

    for (i = 0; i < ServerBrowserCount(DWCi_GetMatchCnt()->sbObj); i++)
    {
        server = ServerBrowserGetServer(DWCi_GetMatchCnt()->sbObj, i);

        if (DWCi_GetMatchCnt()->qr2MatchType == 0)
        {
            profileID = SBServerGetIntValueA(server, "dwc_pid", 0);
            localDelete = FALSE;

            for (j = 1; j <= DWCi_GetMatchCnt()->gt2NumConnection; j++)
            {
                if (DWCi_GetMatchCnt()->sbPidList[j] == profileID)
                {
                    ServerBrowserRemoveServer(DWCi_GetMatchCnt()->sbObj, server);
                    localDelete = TRUE;
                    i--;
                    break;
                }
            }
            if (localDelete)
            {
                continue;
            }
        }

        if (DWCi_GetMatchCnt()->evalCallback != NULL)
        {
            eval = DWCi_GetMatchCnt()->evalCallback(
                i, DWCi_GetMatchCnt()->evalParam);

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

static void DWCi_QR2ServerKeyCallback(
    int keyid, qr2_buffer_t outbuf, void* userdata)
{
#pragma unused(userdata)
    int index;

    switch (keyid)
    {
    case NUMPLAYERS_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2NNFinishCount);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWCi_GetMatchCnt()->qr2NNFinishCount);
        break;
    case MAXPLAYERS_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2NumEntry);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWCi_GetMatchCnt()->qr2NumEntry);
        break;
    case DWC_QR2_PID_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->profileID);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWCi_GetMatchCnt()->profileID);
        break;
    case DWC_QR2_MATCH_TYPE_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2MatchType);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWCi_GetMatchCnt()->qr2MatchType);
        break;
    case DWC_QR2_MATCH_RESV_KEY:
        qr2_buffer_add_int(outbuf, DWCi_GetMatchCnt()->qr2Reservation);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWCi_GetMatchCnt()->qr2Reservation);
        break;
    case DWC_QR2_MATCH_VER_KEY:
        qr2_buffer_add_int(outbuf, DWC_MATCHING_VERSION);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid,
            DWC_MATCHING_VERSION);
        break;
    case DWC_QR2_MATCH_EVAL_KEY:
        qr2_buffer_add_int(outbuf, 1);
        DWC_Printf(DWC_REPORTFLAG_QR2_REQ,
            "QR2, Received ServerKeyReq : keyID %d - %d\n", keyid, 1);
        break;
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

static void DWCi_QR2PlayerKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
}

static void DWCi_QR2TeamKeyCallback(
    int keyid, int index, qr2_buffer_t outbuf, void* userdata)
{
}

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

static int DWCi_QR2CountCallback(qr2_key_type keytype, void* userdata)
{
    return 0;
}

static void DWCi_QR2AddErrorCallback(
    qr2_error_t error, gsi_char* errmsg, void* userdata)
{
    DWC_Printf(2, "QR2 Failed query addition to master server %d\n", error);
    DWC_Printf(2, "%s\n", errmsg);
    DWCi_HandleQR2Error(error);
}

static void DWCi_QR2PublicAddrCallback(
    unsigned int ip, unsigned short port, void* userdata)
{
    // the address is printed as four bytes
    DWC_Printf(0x40, "Got my query IP %d.%d.%d.%d & port %d.\n", ip >> 24, (ip >> 16) & 0xff,
        (ip >> 8) & 0xff, ip & 0xff, port);
    stpMatchCnt->qr2IP = ip;
    stpMatchCnt->qr2Port = port;
}

static void DWCi_QR2NatnegCallback(int cookie, void* userdata)
{
    DWC_Printf(0x40, "Got NN request, cookie = %x.\n", cookie);
    if (stpMatchCnt->state == 1)
    {
        stpMatchCnt->state = 6;
    }
    else if (stpMatchCnt->state != 6 && stpMatchCnt->state != 11)
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
    if (DWCi_HandleNNError(DWCi_NNStartupAsync(1, cookie, NULL)) == 0)
    {
        stpMatchCnt->cmdCnt.command = DWC_MATCH_COMMAND_DUMMY;
    }
}

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
            sbMsg.data, sbMsg.header.size >> 2, FALSE);
        if (!boolResult)
        {
            return;
        }
        offset += sizeof(DWCSBMessageHeader) + sbMsg.header.size;
    }
}

static void DWCi_NNProgressCallback(NegotiateState state, void* userdata)
{
    DWC_Printf(0x40, "NN, Got state update: %d\n", state);
}

static void DWCi_NNCompletedCallback(NegotiateResult result, SOCKET gamesocket,
    struct sockaddr_in* remoteaddr, void* userdata)
{
#pragma unused(gamesocket)
    char pidStr[12];
    int index;
    NegotiateError nnError;
    NegotiateResult nnResult;
    GT2Result gt2Result;
    DWCNNInfo* nnInfo = (DWCNNInfo*)userdata;

    DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
        "NN, Complete NAT Negotiation. result : %d\n",
        result);
    if (nnInfo)
    {
        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN cookie = %x.\n", nnInfo->cookie);
    }

    if (((DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_CL_NN)
            && (DWCi_GetMatchCnt()->state != DWC_MATCH_STATE_SV_OWN_NN))
        || !nnInfo)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "Ignore delayed NN after cancel.\n");
        return;
    }

    if (result == nr_success)
    {
        if (remoteaddr)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "NN, remote address : %s\n",
                gt2AddressToString(remoteaddr->sin_addr.addr,
                    SO_NtoHs(remoteaddr->sin_port),
                    NULL));
        }

        nnInfo->cookie = 0;

        DWCi_GetMatchCnt()->qr2NNFinishCount++;
        index = DWCi_GetMatchCnt()->qr2NNFinishCount;

        if (nnInfo->isQR2)
        {
            DWCi_GetMatchCnt()->ipList[index] = remoteaddr->sin_addr.addr;
            DWCi_GetMatchCnt()->portList[index]
                = SO_NtoHs(remoteaddr->sin_port);

            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "NN child finished Nat Negotiation.\n");

            DWCi_GetMatchCnt()->nnRecvCount = 0;
            DWCi_GetMatchCnt()->nnLastCookie = 0;
            DWCi_GetMatchCnt()->nnFailedTime = 0;

            if (DWCi_GetMatchCnt()->state == DWC_MATCH_STATE_SV_OWN_NN)
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_SV_OWN_GT2);
            }
            else
            {
                DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_GT2);
            }

            DWCi_GetMatchCnt()->gt2ConnectCount = 0;

            DWC_Printf(DWC_REPORTFLAG_MATCH_GT2,
                "gt2Connect() to pidList[%d] (%s)\n",
                index,
                gt2AddressToString(DWCi_GetMatchCnt()->ipList[index],
                    DWCi_GetMatchCnt()->portList[index],
                    NULL));

            (void)snprintf(pidStr, sizeof(pidStr), "%u", DWCi_GetMatchCnt()->profileID);
            gt2Result = gt2Connect(*DWCi_GetMatchCnt()->pGt2Socket, NULL, gt2AddressToString(DWCi_GetMatchCnt()->ipList[index], DWCi_GetMatchCnt()->portList[index], NULL), (GT2Byte*)pidStr, -1, DWC_GT2_CONNECT_TIMEOUT, DWCi_GetMatchCnt()->gt2Callbacks, GT2False);
            if (gt2Result == GT2OutOfMemory)
            {
                DWCi_HandleGT2Error(gt2Result);
                return;
            }
            else if (gt2Result == GT2Success)
            {
            }
            else if (!DWCi_CancelPreConnectedServerProcess(
                         DWCi_GetMatchCnt()->sbPidList[index]))
            {
                return;
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN,
                "NN parent finished Nat Negotiation.\n");

            if (remoteaddr)
            {
                DWCi_GetMatchCnt()->ipList[index - 1]
                    = remoteaddr->sin_addr.addr;
                DWCi_GetMatchCnt()->portList[index - 1]
                    = SO_NtoHs(remoteaddr->sin_port);
            }

            DWCi_GetMatchCnt()->nnFinishTime = DWCi_Np_GetTick();
            DWCi_SetMatchStatus(DWC_MATCH_STATE_CL_GT2);
        }
    }
    else
    {
        if (!nnInfo->cookie)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "Ignore delayed NN error after cancel.\n");
            return;
        }

        nnResult = DWCi_HandleNNResult(result);

        if ((nnResult != nr_inittimeout)
            && (nnResult != nr_deadbeatpartner))
        {
            return;
        }

        if (!nnInfo->isQR2)
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Failed %d/%d NN send.\n", nnInfo->retryCount, DWC_MAX_MATCH_NN_RETRY);

            if ((nnResult == nr_deadbeatpartner)
                || ((nnResult == nr_inittimeout)
                    && (nnInfo->retryCount >= DWC_MAX_MATCH_NN_RETRY)))
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Abort NN.\n");

                nnInfo->cookie = 0;

                if (!DWCi_ProcessNNFailure(FALSE))
                {
                    return;
                }

                if (!DWCi_CancelPreConnectedClientProcess(
                        DWCi_GetMatchCnt()
                            ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection]))
                {
                    return;
                }
            }
            else
            {
                nnInfo->retryCount++;

                nnError = DWCi_DoNatNegotiationAsync(nnInfo);
                if (DWCi_HandleNNError(nnError))
                {
                    return;
                }
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Failed %d/%d NN recv.\n", DWCi_GetMatchCnt()->nnRecvCount, DWC_MAX_MATCH_NN_RETRY);

            DWCi_GetMatchCnt()->nnFailedTime = DWCi_Np_GetTick();

            if ((nnResult == nr_deadbeatpartner)
                || ((nnResult == nr_inittimeout)
                    && (DWCi_GetMatchCnt()->nnRecvCount
                        >= DWC_MAX_MATCH_NN_RETRY)))
            {
                DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "Abort NN.\n");

                nnInfo->cookie = 0;

                if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
                    || (DWCi_GetMatchCnt()->qr2MatchType
                        == DWC_MATCH_TYPE_SC_SV))
                {
                    if (!DWCi_ProcessNNFailure(TRUE))
                    {
                        return;
                    }
                }
                else
                {
                    if (!DWCi_ProcessNNFailure(FALSE))
                    {
                        return;
                    }
                }

                DWCi_GetMatchCnt()->nnRecvCount = 0;
                DWCi_GetMatchCnt()->nnLastCookie = 0;
                DWCi_GetMatchCnt()->nnFailedTime = 0;

                if (!DWCi_CancelPreConnectedServerProcess(
                        DWCi_GetMatchCnt()
                            ->sbPidList[DWCi_GetMatchCnt()->gt2NumConnection
                                        + 1]))
                {
                    return;
                }
            }
        }
    }
}

static BOOL DWCi_ProcessNNFailure(BOOL ignoreError)
{
    if (ignoreError)
    {
        return TRUE;
    }

    if (DWCi_GetMatchCnt()->qr2MatchType != DWC_MATCH_TYPE_SC_CL)
    {
        DWCi_GetMatchCnt()->nnFailureCount++;

        DWC_Printf(DWC_REPORTFLAG_MATCH_NN, "NN failure %d/%d.\n", DWCi_GetMatchCnt()->nnFailureCount, DWC_MATCH_NN_FAILURE_MAX);
    }

    if ((DWCi_GetMatchCnt()->qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        || (DWCi_GetMatchCnt()->nnFailureCount
            >= DWC_MATCH_NN_FAILURE_MAX))
    {
        DWCi_StopMatching(DWC_ERROR_NETWORK,
            DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_NN
                + DWC_ECODE_TYPE_MUCH_FAILURE);
        return FALSE;
    }

    return TRUE;
}

static DWCMatchControl* DWCi_GetMatchCnt(void)
{
    return stpMatchCnt;
}

static void DWCi_SetMatchStatus(DWCMatchState state)
{
    stpMatchCnt->state = state;
}
