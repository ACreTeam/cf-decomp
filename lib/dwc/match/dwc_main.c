#include <dwc/dwc_main.h>
#include <dwc/dwc_common.h>
#include <dwc/dwc_error.h>
#include <dwc/dwc_friend.h>
#include <dwc/dwc_login.h>
#include <dwc/dwc_match.h>
#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <dwc/dwc_transport.h>
#include <gamespy/GP/gp.h>
#include <gamespy/common/gsAvailable.h>
#include <gamespy/common/gsPlatformSocket.h>
#include <gamespy/gstats/gstats.h>
#include <gamespy/gt2/gt2.h>
#include <gamespy/natneg/natneg.h>
#include <gamespy/qr2/qr2.h>
#include <gamespy/serverbrowsing/sb_serverbrowsing.h>
#include <stdlib.h>
#include <string.h>

typedef struct DWCMatchControlView
{
    u8 _000[0xD];
    u8 gt2NumConnection;
    u8 _00E[2];
    qr2_t qr2Obj;
    u8 qr2NNFinishCount;
    volatile u8 qr2MatchType;
    u8 qr2NumEntry;
    u8 qr2IsReserved;
    u8 qr2ShutdownFlag;
    u8 _019[0xCB];
    ServerBrowser sbObj;
    u8 _0E8[0x10];
    int sbPidList[32];
    u8 _178[0x2C];
    DWCMatchState state;
    u8 _1A8[0x138];
    u8 aidList[DWC_MAX_CONNECTIONS];
    u32 validAidBitmap;
    u8 _304[0x194];
} DWCMatchControlView;

typedef struct DWCstControl
{
    GT2Socket gt2Socket;
    GT2ConnectionCallbacks gt2Callbacks;
    int gt2SendBufSize;
    int gt2RecvBufSize;
    void* gpObj;
    DWCUserData* userdata;
    DWCState state;
    DWCState lastState;
    u8 aid;
    u8 ownCloseFlag;
    u16 playerName[DWC_MAX_PLAYER_NAME];
    int profileID;
    const char* gameName;
    const char* secretKey;
    DWCLoginCallback loginCallback;
    void* loginParam;
    DWCUpdateServersCallback updateServersCallback;
    void* updateServersParam;
    DWCMatchedCallback matchedCallback;
    void* matchedParam;
    DWCMatchedSCCallback matchedSCCallback;
    void* matchedSCParam;
    DWCConnectionClosedCallback closedCallback;
    void* closedParam;
    DWCLoginControl logcnt;
    u8 friendcnt[0x60];
    DWCMatchControlView matchcnt;
    DWCTransportInfo transinfo;
} DWCControl;

typedef struct DWCConnectionInfo
{
    u8 index;
    u8 aid;
    u16 reserve;
    void* param;
} DWCConnectionInfo;

void DWCi_NNFreeNegotiateList(void); // dwc_match.c's deferred NNFreeNegotiateList

static GT2Connection stGt2ConnectionList[32];
static DWCConnectionInfo stConnectionInfoList[32];
static DWCControl* stpDwcCnt = NULL;
static int stLastSocketError = 0;

int DWCi_GetNumAllConnection(void);
int DWCi_GetNumValidConnection(void);
int DWCi_GetValidAIDList(u8** aidList);
void DWCi_FriendInit(void* friendcnt, void* pGpObj, const u16* playerName,
    DWCFriendData* friendList, int friendListLen);
void DWCi_MatchInit(void* matchcnt, void* pGpObj, GT2Socket* pGt2Socket,
    GT2ConnectionCallbacks* gt2Callbacks, const char* gameName,
    const char* secretKey, const DWCFriendData friendList[],
    int friendListLen);
void DWCi_ShutdownFriend(void);
void DWCi_ShutdownMatch(void);
void DWCi_ShutdownTransport(void);
void DWCi_MatchProcess(BOOL fullSpec);
void DWCi_TransportProcess(void);
void DWCi_GPRecvBuddyRequestCallback(GPConnection* connection,
    GPRecvBuddyRequestArg* arg, void* param);
BOOL DWCi_GPRecvBuddyAuthCallback(GPConnection* connection,
    GPRecvBuddyMessageArg* arg, void* param);
void DWCi_GPRecvBuddyStatusCallback(GPConnection* connection,
    GPRecvBuddyStatusArg* arg, void* param);
void DWCi_GT2ConnectedCallback(GT2Connection connection, GT2Result result,
    GT2Byte* message, int len);
static void DWCi_GT2ClosedCallback(
    GT2Connection connection, GT2CloseReason reason);
static GT2Result DWCi_HandleGT2Error(GT2Result result);
void DWCi_StopLogin(DWCError error, int errorCode);
static void DWCi_MatchedCallback(DWCError error, BOOL cancel, BOOL self,
    BOOL isServer, int index, void* param);
void DWCi_ConnectToAnybodyAsync(u8 numEntry, const char* addFilter,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam);
GT2Bool DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, unsigned int ip,
    unsigned short port, GT2Byte* message, int len);
void DWCi_GT2ConnectAttemptCallback(GT2Socket socket,
    GT2Connection connection, unsigned int ip, unsigned short port,
    int latency, GT2Byte* message, int len);
void DWCi_SetupGameServer(u8 numEntry,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam);
GPResult DWCi_SetGPStatus(int status, const char* statusString,
    const char* locationString);
static void DWCi_LoginCallback(DWCError error, int profileID, void* param);
void DWCi_InitGPProcessCount(void);
static void DWCi_UpdateServersCallback(
    int error, BOOL isChanged, void* param);
DWCConnectionInfo* DWCi_GetConnectionInfoByIdx(int index);
static void DWCi_GT2ReceivedCallback(
    GT2Connection connection, GT2Byte* message, int len, GT2Bool reliable);
void DWCi_RecvCallback(GT2Connection connection, GT2Byte* message, int len,
    GT2Bool reliable);
static void DWCi_GT2PingCallback(GT2Connection connection, int latency);
void DWCi_PingCallback(GT2Connection connection, int latency);
static void DWCi_SetState(DWCState state);
static int DWCi_DeleteAID(u8 aid);
static u32 DWCi_GetAIDBitmapFromList(u8* aidList, int aidListLen);
static GPResult DWCi_HandleGPError(GPResult result);
static void DWCi_GPErrorCallback(
    GPConnection* pconnection, GPErrorArg* arg, void* param);
static void DWCi_GPRecvBuddyMessageCallback(
    GPConnection* pconnection, GPRecvBuddyMessageArg* arg, void* param);
static void DWCi_GT2SocketErrorCallback(GT2Socket socket);

void DWC_InitFriendsMatch(DWCFriendsMatchControl* dwccnt,
    DWCUserData* userdata, int productID, const char* gameName,
    const char* secretKey, int sendBufSize, int recvBufSize,
    DWCFriendData friendList[], int friendListLen)
{
    u32 cpySize;

    DWC_Printf(4, "!!DWC_InitFriendsMatch() was called!!\n");

    stpDwcCnt = DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(DWCControl));
    memset(stpDwcCnt, 0, sizeof(DWCControl));

    DWC_ClearError();

    stpDwcCnt->gt2Socket = NULL;
    stpDwcCnt->gt2Callbacks.connected = DWCi_GT2ConnectedCallback;
    stpDwcCnt->gt2Callbacks.received = DWCi_GT2ReceivedCallback;
    stpDwcCnt->gt2Callbacks.closed = DWCi_GT2ClosedCallback;
    stpDwcCnt->gt2Callbacks.ping = DWCi_GT2PingCallback;
    stpDwcCnt->gt2SendBufSize = sendBufSize ? sendBufSize : 8 * 1024;
    stpDwcCnt->gt2RecvBufSize = recvBufSize ? recvBufSize : 8 * 1024;

    stpDwcCnt->gpObj = NULL;
    stpDwcCnt->userdata = userdata;

    stpDwcCnt->state = DWC_STATE_INIT;
    stpDwcCnt->lastState = DWC_STATE_INIT;
    stpDwcCnt->aid = 0;
    stpDwcCnt->ownCloseFlag = FALSE;
    stpDwcCnt->profileID = 0;
    stpDwcCnt->gameName = gcd_gamename;
    stpDwcCnt->secretKey = gcd_secret_key;

    stpDwcCnt->loginCallback = NULL;
    stpDwcCnt->loginParam = NULL;
    stpDwcCnt->updateServersCallback = NULL;
    stpDwcCnt->updateServersParam = NULL;
    stpDwcCnt->matchedCallback = NULL;
    stpDwcCnt->matchedParam = NULL;
    stpDwcCnt->matchedSCCallback = NULL;
    stpDwcCnt->matchedSCParam = NULL;
    stpDwcCnt->closedCallback = NULL;
    stpDwcCnt->closedParam = NULL;

    DWCi_ClearGT2ConnectionList();

    DWCi_LoginInit(&stpDwcCnt->logcnt, userdata, &stpDwcCnt->gpObj,
        productID, userdata->gamecode, stpDwcCnt->playerName,
        DWCi_LoginCallback, NULL);

    DWCi_FriendInit(stpDwcCnt->friendcnt, &stpDwcCnt->gpObj,
        stpDwcCnt->playerName, friendList, friendListLen);

    DWCi_MatchInit(&stpDwcCnt->matchcnt, &stpDwcCnt->gpObj,
        &stpDwcCnt->gt2Socket, &stpDwcCnt->gt2Callbacks,
        gcd_gamename, gcd_secret_key, friendList, friendListLen);

    DWCi_InitTransport(&stpDwcCnt->transinfo);

    cpySize = strlen(secretKey) < sizeof(gcd_secret_key)
        ? strlen(secretKey)
        : sizeof(gcd_secret_key) - 1;
    DWCi_Np_CpuCopy8(secretKey, gcd_secret_key, cpySize);
    gcd_secret_key[cpySize] = '\0';
}

void DWC_ShutdownFriendsMatch(void)
{
    DWC_Printf(4,
        "!!DWC_ShutdownFriendsMatch() was called!! stpDwcCnt = 0x%x\n",
        stpDwcCnt);

    if (!stpDwcCnt)
    {
        return;
    }

    if (stpDwcCnt->matchcnt.qr2Obj)
    {
        qr2_shutdown(stpDwcCnt->matchcnt.qr2Obj);
        stpDwcCnt->matchcnt.qr2Obj = NULL;
    }
    stpDwcCnt->matchcnt.qr2ShutdownFlag = 0;

    if (stpDwcCnt->matchcnt.sbObj)
    {
        ServerBrowserFree(stpDwcCnt->matchcnt.sbObj);
        stpDwcCnt->matchcnt.sbObj = NULL;
    }

    DWCi_NNFreeNegotiateList();

    CloseStatsConnection();

    if (stpDwcCnt->gpObj)
    {
        (void)gpSetCallback(
            &stpDwcCnt->gpObj, GP_ERROR, (GPCallback)NULL, NULL);
        (void)gpSetCallback(&stpDwcCnt->gpObj, GP_RECV_BUDDY_MESSAGE,
            (GPCallback)NULL, NULL);
        (void)gpSetCallback(&stpDwcCnt->gpObj, GP_RECV_BUDDY_REQUEST,
            (GPCallback)NULL, NULL);
        (void)gpSetCallback(&stpDwcCnt->gpObj, GP_RECV_BUDDY_STATUS,
            (GPCallback)NULL, NULL);
        (void)gpProcess(&stpDwcCnt->gpObj);
        gpDestroy(&stpDwcCnt->gpObj);
        stpDwcCnt->gpObj = NULL;
    }

    DWCi_ShutdownLogin();
    DWCi_ShutdownFriend();
    DWCi_ShutdownMatch();
    DWCi_ShutdownTransport();

    if (stpDwcCnt->gt2Socket)
    {
        gt2CloseSocket(stpDwcCnt->gt2Socket);
        stpDwcCnt->gt2Socket = NULL;
    }

    DWC_Free(DWC_ALLOCTYPE_BASE, stpDwcCnt, 0);
    stpDwcCnt = NULL;
}

void DWC_ProcessFriendsMatch(void)
{
    GSIACResult acResult;
    GPResult gpResult;

    if (!stpDwcCnt || stpDwcCnt->state == DWC_STATE_INIT || DWCi_IsError())
    {
        return;
    }

    switch (stpDwcCnt->state)
    {
    case DWC_STATE_AVAILABLE_CHECK:
        acResult = GSIAvailableCheckThink();
        switch (acResult)
        {
        case GSIACAvailable:
            DWC_Printf(DWC_REPORTFLAG_ACHECK,
                "Confirmed the backend of GameSpy server.\n");

            gpResult = gpInitialize(
                &stpDwcCnt->gpObj, stpDwcCnt->logcnt.productID, 16, 11);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            gpResult = gpSetCallback(&stpDwcCnt->gpObj, GP_ERROR,
                (GPCallback)DWCi_GPErrorCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            gpResult = gpSetCallback(&stpDwcCnt->gpObj,
                GP_RECV_BUDDY_MESSAGE,
                (GPCallback)DWCi_GPRecvBuddyMessageCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            gpResult = gpSetCallback(&stpDwcCnt->gpObj,
                GP_RECV_BUDDY_AUTH,
                (GPCallback)DWCi_GPRecvBuddyAuthCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            gpResult = gpSetCallback(&stpDwcCnt->gpObj,
                GP_RECV_BUDDY_REQUEST,
                (GPCallback)DWCi_GPRecvBuddyRequestCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            gpResult = gpSetCallback(&stpDwcCnt->gpObj,
                GP_RECV_BUDDY_STATUS,
                (GPCallback)DWCi_GPRecvBuddyStatusCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }

            DWCi_SetState(DWC_STATE_LOGIN);

            if (!DWCi_LoginAsync())
            {
                DWCi_StopLogin(DWC_ERROR_AUTH_ANY, -20100);
            }
            break;
        case GSIACUnavailable:
            DWCi_StopLogin(DWC_ERROR_AUTH_OUT_OF_SERVICE, -20110);
            return;
        case GSIACTemporarilyUnavailable:
            DWCi_StopLogin(DWC_ERROR_AUTH_STOP_SERVICE, -20101);
            return;
        default:
            break;
        }
        break;

    case DWC_STATE_LOGIN:
        DWCi_LoginProcess();
        break;

    case DWC_STATE_UPDATE_SERVERS:
    case DWC_STATE_ONLINE:
        DWCi_FriendProcess();
        DWCi_MatchProcess(FALSE);
        break;

    case DWC_STATE_MATCHING:
        DWCi_MatchProcess(TRUE);
        DWCi_FriendProcess();
        break;

    case DWC_STATE_CONNECTED:
        DWCi_TransportProcess();
        DWCi_FriendProcess();

        if (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV
            || stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        {
            DWCi_MatchProcess(TRUE);
        }
        else if (stpDwcCnt->gt2Socket)
        {
            DWCi_MatchProcess(FALSE);
        }
        break;

    default:
        break;
    }

#ifdef DWC_QR2_ALIVE_DURING_MATCHING
    if (stpDwcCnt->matchcnt.qr2ShutdownFlag == 1)
    {
        if (stpDwcCnt->matchcnt.qr2Obj != NULL)
        {
            qr2_shutdown(stpDwcCnt->matchcnt.qr2Obj);
            stpDwcCnt->matchcnt.qr2Obj = NULL;
        }
        stpDwcCnt->matchcnt.qr2ShutdownFlag = 0;
    }
#endif
}

BOOL DWC_LoginAsync(const u16* ingamesn, const char* reserved,
    DWCLoginCallback callback, void* param)
{
#pragma unused(reserved)
    u32 len;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "!!DWC_LoginAsync() was called!!\n");
    if (ingamesn == NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING, "ingamesn is NULL!!\n");
        return FALSE;
    }
    if (DWCi_IsError() || stpDwcCnt->state != DWC_STATE_INIT)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    stpDwcCnt->loginCallback = callback;
    stpDwcCnt->loginParam = param;

    if (!ingamesn || ingamesn[0] == '\0')
    {
        len = 0;
    }
    else
    {
        DWCi_Np_CpuClear16(
            stpDwcCnt->playerName, DWC_MAX_PLAYER_NAME * 2);
        len = DWCi_WStrLen(ingamesn) <= DWC_MAX_PLAYER_NAME - 1
            ? DWCi_WStrLen(ingamesn)
            : DWC_MAX_PLAYER_NAME - 1;
        DWCi_Np_CpuCopy16(
            ingamesn, stpDwcCnt->playerName, len * 2);
    }
    stpDwcCnt->playerName[len] = 0;

    DWCi_SetState(DWC_STATE_AVAILABLE_CHECK);
    GSIStartAvailableCheckA(stpDwcCnt->gameName);
    return TRUE;
}

BOOL DWC_UpdateServersAsync(const char* playerName,
    DWCUpdateServersCallback updateCallback, void* updateParam,
    DWCFriendStatusCallback statusCallback, void* statusParam,
    DWCDeleteFriendListCallback deleteCallback, void* deleteParam)
{
#pragma unused(playerName)
    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_UpdateServersAsync() was called!!\n");
    if (DWCi_IsError() || stpDwcCnt->state < DWC_STATE_ONLINE
        || stpDwcCnt->state == DWC_STATE_UPDATE_SERVERS)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    stpDwcCnt->updateServersCallback = updateCallback;
    stpDwcCnt->updateServersParam = updateParam;
    DWCi_SetState(DWC_STATE_UPDATE_SERVERS);

    DWCi_UpdateServersAsync(stpDwcCnt->logcnt.authToken,
        stpDwcCnt->logcnt.partnerChallenge, DWCi_UpdateServersCallback, NULL,
        statusCallback, statusParam, deleteCallback, deleteParam);
    return TRUE;
}

BOOL DWC_ConnectToAnybodyAsync(u8 numEntry, const char* addFilter,
    DWCMatchedCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam)
{
    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_ConnectToAnybodyAsync() was called!!\n");
    if (DWCi_IsError() || stpDwcCnt->state != DWC_STATE_ONLINE)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    DWCi_ClearGT2ConnectionList();
    stpDwcCnt->matchedCallback = matchedCallback;
    stpDwcCnt->matchedParam = matchedParam;
    DWCi_SetState(DWC_STATE_MATCHING);

    DWCi_ConnectToAnybodyAsync((u8)(numEntry - 1), addFilter,
        DWCi_MatchedCallback, NULL, evalCallback, evalParam);
    return TRUE;
}

BOOL DWC_ConnectToFriendsAsync(const u8 friendIdxList[],
    int friendIdxListLen, u8 numEntry, BOOL distantFriend,
    DWCMatchedCallback matchedCallback, void* matchedParam,
    DWCEvalPlayerCallback evalCallback, void* evalParam)
{
    u8 tmpFriendIdxList[DWC_MAX_MATCH_IDX_LIST];
    u8 idxList[DWC_MAX_MATCH_IDX_LIST];
    int i;
    int j;
    u32 randIdx;

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_ConnectToFriendsAsync() was called!!\n");
    if (DWCi_IsError() || stpDwcCnt->state != DWC_STATE_ONLINE)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    DWCi_ClearGT2ConnectionList();
    stpDwcCnt->matchedCallback = matchedCallback;
    stpDwcCnt->matchedParam = matchedParam;
    DWCi_SetState(DWC_STATE_MATCHING);

    if (friendIdxList)
    {
        DWCi_ConnectToFriendsAsync(friendIdxList, friendIdxListLen,
            (u8)(numEntry - 1), distantFriend, DWCi_MatchedCallback, NULL,
            evalCallback, evalParam);
    }
    else
    {
        friendIdxListLen = 0;
        for (i = 0; i < DWCi_GetFriendListLen(); i++)
        {
            idxList[i] = i;
        }

        for (i = 0; i < DWCi_GetFriendListLen(); i++)
        {
            randIdx = DWCi_GetMathRand32(
                (u32)(DWCi_GetFriendListLen() - i));

#ifdef DWC_MATCH_ACCEPT_NO_FRIEND
            tmpFriendIdxList[i] = idxList[randIdx];
            friendIdxListLen++;
#else
            if (DWCi_Acc_IsValidFriendData(
                    &(DWCi_GetFriendList())[idxList[randIdx]]))
            {
                tmpFriendIdxList[i] = idxList[randIdx];
                friendIdxListLen++;
            }
#endif

            for (j = randIdx; j < DWCi_GetFriendListLen() - i - 1; j++)
            {
                idxList[j] = idxList[j + 1];
            }
        }

        DWCi_ConnectToFriendsAsync(tmpFriendIdxList, friendIdxListLen,
            (u8)(numEntry - 1), distantFriend, DWCi_MatchedCallback, NULL,
            evalCallback, evalParam);
    }

    return TRUE;
}

BOOL DWC_SetupGameServer(u8 maxEntry,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam)
{
    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_SetupGameServer() was called!!\n");
    if (DWCi_IsError() || stpDwcCnt->state != DWC_STATE_ONLINE)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    DWCi_ClearGT2ConnectionList();
    stpDwcCnt->matchedSCCallback = matchedCallback;
    stpDwcCnt->matchedSCParam = matchedParam;
    stpDwcCnt->aid = 0;
    DWCi_SetState(DWC_STATE_MATCHING);

    DWCi_SetupGameServer((u8)(maxEntry - 1), DWCi_MatchedCallback, NULL,
        newClientCallback, newClientParam);
    return TRUE;
}

BOOL DWC_ConnectToGameServerAsync(int serverIndex,
    DWCMatchedSCCallback matchedCallback, void* matchedParam,
    DWCNewClientCallback newClientCallback, void* newClientParam)
{
    int profileID;
    int buddyIdx = -1;
    GPBuddyStatus status;
    DWCError dwcError;

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_ConnectToGameServerAsync() was called!!\n");

    if (DWCi_IsError() || stpDwcCnt->state != DWC_STATE_ONLINE)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return FALSE;
    }

    DWCi_ClearGT2ConnectionList();
    stpDwcCnt->matchedSCCallback = matchedCallback;
    stpDwcCnt->matchedSCParam = matchedParam;
    DWCi_SetState(DWC_STATE_MATCHING);

    if (!(profileID = DWCi_GetProfileIDFromList(serverIndex))
        || !gpIsBuddy(&stpDwcCnt->gpObj, profileID))
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR, "pid %d is not buddy.\n", profileID);
        dwcError = DWC_ERROR_NOT_FRIEND_SERVER;
        goto error;
    }

    (void)gpGetBuddyIndex(&stpDwcCnt->gpObj, profileID, &buddyIdx);
    (void)gpGetBuddyStatus(&stpDwcCnt->gpObj, buddyIdx, &status);

    if (status.status != DWC_STATUS_MATCH_SC_SV)
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR,
            "pid %d is not game server.\n", profileID);
        dwcError = DWC_ERROR_NOT_FRIEND_SERVER;
        goto error;
    }

    {
        char valueStr[4];
        u8 maxEntry;
        u8 numEntry;

        valueStr[0] = '0';

        DWC_GetCommonValueString(DWC_GP_SSTR_KEY_MATCH_SC_MAX, valueStr,
            status.statusString, '/');
        maxEntry = (u8)strtoul(valueStr, NULL, 10);

        DWC_GetCommonValueString(DWC_GP_SSTR_KEY_MATCH_SC_NUM, valueStr,
            status.statusString, '/');
        numEntry = (u8)strtoul(valueStr, NULL, 10);

        if (numEntry == maxEntry)
        {
            DWC_Printf(DWC_REPORTFLAG_ERROR,
                "pid %d is fully occupied.\n", profileID);
            dwcError = DWC_ERROR_SERVER_FULL;
            goto error;
        }
    }

    DWCi_ConnectToGameServerAsync(profileID, DWCi_MatchedCallback, NULL,
        newClientCallback, newClientParam);

    return TRUE;

error:
    DWCi_SetError(dwcError, 0);

    stpDwcCnt->matchedSCCallback(
        dwcError, FALSE, TRUE, FALSE, 0, stpDwcCnt->matchedSCParam);

    if (stpDwcCnt != NULL && stpDwcCnt->state == DWC_STATE_MATCHING)
    {
        DWCi_SetState(DWC_STATE_ONLINE);
        (void)DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);
    }

    return TRUE;
}

BOOL DWC_SetConnectionClosedCallback(DWCConnectionClosedCallback callback,
    void* param)
{
    if (stpDwcCnt == NULL)
    {
        return FALSE;
    }
    stpDwcCnt->closedCallback = callback;
    stpDwcCnt->closedParam = param;
    return TRUE;
}

int DWC_CloseConnectionsAsync(void)
{
    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_CloseConnectionsAsync() was called!!\n");

    if (!stpDwcCnt || DWCi_IsError()
        || (stpDwcCnt->state != DWC_STATE_MATCHING
            && stpDwcCnt->state != DWC_STATE_CONNECTED))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return -1;
    }

    if (!stpDwcCnt->matchcnt.gt2NumConnection)
    {
        DWC_Printf(DWC_REPORTFLAG_TRANSPORT, "Closed 0 connection.\n");

        (void)DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);

        NNFreeNegotiateList();

        DWCi_SetState(DWC_STATE_ONLINE);

        return 1;
    }

    gt2CloseAllConnections(stpDwcCnt->gt2Socket);

    return 0;
}

int DWC_CloseAllConnectionsHard(void)
{
    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_CloseAllConnectionsHard() was called!!\n");

    if (!stpDwcCnt || DWCi_IsError()
        || (stpDwcCnt->state != DWC_STATE_MATCHING
            && stpDwcCnt->state != DWC_STATE_CONNECTED))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return -1;
    }

    if (!stpDwcCnt->matchcnt.gt2NumConnection)
    {
        DWC_Printf(DWC_REPORTFLAG_TRANSPORT, "Closed 0 connection.\n");

        (void)DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);

        NNFreeNegotiateList();

        DWCi_SetState(DWC_STATE_ONLINE);

        return 1;
    }

    stpDwcCnt->ownCloseFlag = TRUE;

    gt2CloseAllConnectionsHard(stpDwcCnt->gt2Socket);

    stpDwcCnt->ownCloseFlag = FALSE;

    return 0;
}

int DWC_CloseConnectionHard(u8 aid)
{
    GT2Connection connection;

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_CloseConnectionHard() was called!! aid = %d.\n", aid);

    if (!stpDwcCnt || DWCi_IsError()
        || (stpDwcCnt->state != DWC_STATE_MATCHING
            && stpDwcCnt->state != DWC_STATE_CONNECTED))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return -1;
    }

    if (!(connection = DWCi_GetGT2Connection(aid)))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "No connection!\n");
        return -2;
    }

    gt2CloseConnectionHard(connection);

    return 0;
}

int DWC_CloseConnectionHardBitmap(u32* bitmap)
{
    u8 aid;
    u32 bitmask;

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "!!DWC_CloseConnectionHardBitmap() was called!! AID bitmap = 0x%x\n",
        bitmap == NULL ? 0 : *bitmap);

    if (!stpDwcCnt || !bitmap || DWCi_IsError()
        || (stpDwcCnt->state != DWC_STATE_MATCHING
            && stpDwcCnt->state != DWC_STATE_CONNECTED))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "But ignored.\n");
        return -1;
    }

    for (aid = 0; aid < DWC_MAX_CONNECTIONS; aid++)
    {
        bitmask = 1U << aid;

        if (*bitmap & bitmask)
        {
            if (aid == DWC_GetMyAID())
            {
                *bitmap &= ~bitmask;
            }
            else if (DWC_CloseConnectionHard(aid))
            {
                *bitmap &= ~bitmask;
            }
        }
    }

    if (!*bitmap)
    {
        return -2;
    }
    return 0;
}

int DWC_GetNumConnectionHost(void)
{
    if (stpDwcCnt == NULL)
    {
        return 0;
    }
    if (stpDwcCnt->matchcnt.qr2MatchType == 2 || stpDwcCnt->matchcnt.qr2MatchType == 3)
    {
        return DWCi_GetNumValidConnection() + 1;
    }
    return DWCi_GetNumAllConnection() + 1;
}

u8 DWC_GetMyAID(void)
{
    if (stpDwcCnt != NULL)
    {
        return stpDwcCnt->aid;
    }
    return 0;
}

int DWC_GetAIDList(u8** aidList)
{
    if (stpDwcCnt == NULL)
    {
        return 0;
    }
    *aidList = stpDwcCnt->matchcnt.aidList;
    if (stpDwcCnt->matchcnt.qr2MatchType == 2 || stpDwcCnt->matchcnt.qr2MatchType == 3)
    {
        return DWCi_GetValidAIDList(aidList);
    }
    return DWCi_GetAllAIDList(aidList);
}

u32 DWC_GetAIDBitmap(void)
{
    u8* pAidList;
    u32 bitmap = 0;
    int numHost;

    if (!stpDwcCnt)
    {
        return 0;
    }

    numHost = DWC_GetAIDList(&pAidList);
    bitmap = DWCi_GetAIDBitmapFromList(pAidList, numHost);

    return bitmap;
}

BOOL DWC_IsValidAID(u8 aid)
{
    if (stpDwcCnt == NULL)
    {
        return FALSE;
    }
    if ((stpDwcCnt->matchcnt.validAidBitmap & (1 << aid)) == 0)
    {
        return FALSE;
    }
    return DWCi_IsValidAID(aid);
}

DWCState DWC_GetState(void)
{
    if (stpDwcCnt != NULL)
    {
        return stpDwcCnt->state;
    }
    return DWC_STATE_INIT;
}

int DWC_GetLastSocketError(void)
{
    return stLastSocketError;
}

GT2Result DWCi_GT2Startup(void)
{
    u16 port;
    GT2Result result;

    if (stpDwcCnt->gt2Socket != NULL)
    {
        DWC_Printf(8, "gt2Socket is already made.\n");
        return GT2Success;
    }

    port = DWCi_GetMathRand32(0x4000) + 0xC000;
    DWC_Printf(0x40, "--- Private port = %d ---\n", port);
    result = gt2CreateSocket(&stpDwcCnt->gt2Socket,
        gt2AddressToString(0, port, NULL),
        stpDwcCnt->gt2SendBufSize,
        stpDwcCnt->gt2RecvBufSize,
        DWCi_GT2SocketErrorCallback);
    if (DWCi_HandleGT2Error(result))
    {
        return result;
    }
    gt2Listen(stpDwcCnt->gt2Socket, DWCi_GT2ConnectAttemptCallback);
    gt2SetUnrecognizedMessageCallback(
        stpDwcCnt->gt2Socket, DWCi_GT2UnrecognizedMessageCallback);
    return result;
}

GT2Connection DWCi_GetGT2Connection(u8 aid)
{
    int i;

    if (stpDwcCnt == NULL)
    {
        return NULL;
    }
    for (i = 0; i < 32; i++)
    {
        if (stGt2ConnectionList[i] != NULL && ((DWCConnectionInfo*)gt2GetConnectionData(stGt2ConnectionList[i]))->aid == aid)
        {
            return stGt2ConnectionList[i];
        }
    }
    return NULL;
}

u8 DWCi_GetConnectionAID(GT2Connection connection)
{
    return ((DWCConnectionInfo*)gt2GetConnectionData(connection))->aid;
}

u8 DWCi_GetConnectionIndex(GT2Connection connection)
{
    return ((DWCConnectionInfo*)gt2GetConnectionData(connection))->index;
}

void* DWCi_GetConnectionUserData(GT2Connection connection)
{
    return ((DWCConnectionInfo*)gt2GetConnectionData(connection))->param;
}

int DWCi_GT2GetConnectionListIdx(void)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        if (stGt2ConnectionList[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}

void DWCi_ClearGT2ConnectionList(void)
{
    memset(stGt2ConnectionList, 0, sizeof(stGt2ConnectionList));
    memset(stConnectionInfoList, 0, sizeof(stConnectionInfoList));
}

GT2Connection* DWCi_GetGT2ConnectionByIdx(int index)
{
    return &stGt2ConnectionList[index];
}

GT2Connection* DWCi_GetGT2ConnectionByProfileID(int profileID, int numHost)
{
    u8 i;

    for (i = 0; i < numHost; i++)
    {
        if (stpDwcCnt->matchcnt.sbPidList[i] == profileID)
        {
            break;
        }
    }

    if (i >= numHost)
    {
        return NULL;
    }

    return DWCi_GetGT2ConnectionByIdx(DWCi_GetConnectionIndex(
        DWCi_GetGT2Connection(stpDwcCnt->matchcnt.aidList[i])));
}

DWCConnectionInfo* DWCi_GetConnectionInfoByIdx(int index)
{
    return &stConnectionInfoList[index];
}

BOOL DWCi_IsValidAID(u8 aid)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        if (stGt2ConnectionList[i] != NULL && ((DWCConnectionInfo*)gt2GetConnectionData(stGt2ConnectionList[i]))->aid == aid)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void DWCi_SetState(DWCState state)
{
    stpDwcCnt->lastState = stpDwcCnt->state;
    stpDwcCnt->state = state;
}

static int DWCi_DeleteAID(u8 aid)
{
    u8* pAidList;
    int numHost;
    int i;

    numHost = DWCi_GetAllAIDList(&pAidList);

    for (i = 0; i < numHost; i++)
    {
        if (pAidList[i] == aid)
        {
            break;
        }
    }

    if (i == numHost)
    {
        return 0;
    }

    return DWCi_DeleteHostByIndex(i, numHost);
}

static u32 DWCi_GetAIDBitmapFromList(u8* aidList, int aidListLen)
{
    u32 bitmap = 0;
    int i;

    for (i = 0; i < aidListLen; i++)
    {
        bitmap |= 1 << aidList[i];
    }

    return bitmap;
}

static GPResult DWCi_HandleGPError(GPResult result)
{
    int errorCode;
    DWCError dwcError;

    if (result == GP_NO_ERROR)
    {
        return GP_NO_ERROR;
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Main, GP error %d\n", result);

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

    switch (stpDwcCnt->state)
    {
    case DWC_STATE_AVAILABLE_CHECK:
        errorCode += DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_GP;
        DWCi_StopLogin(dwcError, errorCode);
        break;
    case DWC_STATE_LOGIN:
        errorCode += DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_GP;
        if (stpDwcCnt->logcnt.state < DWC_LOGIN_STATE_REMOTE_AUTH)
        {
            DWCi_StopLogin(dwcError, errorCode);
        }
        else
        {
            DWC_Printf(
                DWC_REPORTFLAG_ERROR, "Not handle an error here.\n");
        }
        break;
    case DWC_STATE_MATCHING:
        errorCode += DWC_ECODE_SEQ_MATCH + DWC_ECODE_GS_GP;
        DWCi_StopMatching(dwcError, errorCode);
        break;
    case DWC_STATE_UPDATE_SERVERS:
        errorCode += DWC_ECODE_SEQ_FRIEND + DWC_ECODE_GS_GP;
        break;
    default:
        errorCode += DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GP;
        break;
    }

    DWCi_StopFriendProcess(dwcError, errorCode);
    return result;
}

static GT2Result DWCi_HandleGT2Error(GT2Result result)
{
    int error;
    int errorCode;

    if (result == GT2Success)
    {
        return FALSE;
    }
    DWC_Printf(2, "Main, GT2 error %d\n", result);
    switch (result)
    {
    case GT2OutOfMemory:
        error = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_ALLOC;
        break;
    case GT2Rejected:
    case GT2DuplicateAddress:
        error = DWC_ERROR_NONE;
        errorCode = 0;
        result = FALSE;
        break;
    case GT2NetworkError:
        error = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_NETWORK;
        break;
    case GT2AddressError:
        error = DWC_ERROR_NETWORK;
        errorCode = -30;
        break;
    case GT2TimedOut:
        error = DWC_ERROR_NETWORK;
        errorCode = -70;
        break;
    case GT2NegotiationError:
        error = DWC_ERROR_NETWORK;
        errorCode = -80;
        break;
    }
    if (error != 0)
    {
        DWCi_StopLogin(error,
            DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_GT2 + errorCode);
    }
    return result;
}

static void DWCi_LoginCallback(
    DWCError error, int profileID, void* param)
{
    if (error == 0)
    {
        stpDwcCnt->profileID = profileID;
        stpDwcCnt->lastState = stpDwcCnt->state;
        stpDwcCnt->state = DWC_STATE_ONLINE;
        DWCi_InitGPProcessCount();
    }
    else
    {
        stpDwcCnt->lastState = stpDwcCnt->state;
        stpDwcCnt->state = DWC_STATE_INIT;
    }
    if (stpDwcCnt->loginCallback != NULL)
    {
        stpDwcCnt->loginCallback(error, profileID, stpDwcCnt->loginParam);
    }
}

static void DWCi_UpdateServersCallback(
    int error, BOOL isChanged, void* param)
{
    DWCState savedState = stpDwcCnt->lastState;

    if (savedState != DWC_STATE_UPDATE_SERVERS)
    {
        DWCState state = stpDwcCnt->state;

        stpDwcCnt->lastState = state;
        stpDwcCnt->state = savedState;
    }
    stpDwcCnt->updateServersCallback(
        error, isChanged, stpDwcCnt->updateServersParam);
}

static void DWCi_MatchedCallback(DWCError error, BOOL cancel, BOOL self,
    BOOL isServer, int index, void* param)
{
#pragma unused(param)
    int i;

    if (error == DWC_ERROR_NONE && cancel)
    {
        if (stpDwcCnt->matchcnt.state == DWC_MATCH_STATE_INIT)
        {
            DWCi_ClearQR2Key();
            DWCi_SetState(DWC_STATE_ONLINE);
        }
    }
    else if (error == DWC_ERROR_NONE)
    {
        DWCi_SetState(DWC_STATE_CONNECTED);

        for (i = 0; i <= stpDwcCnt->matchcnt.gt2NumConnection; i++)
        {
            if (stpDwcCnt->matchcnt.sbPidList[i] == stpDwcCnt->profileID)
            {
                stpDwcCnt->aid = stpDwcCnt->matchcnt.aidList[i];
                break;
            }
        }
    }

#ifndef SDK_FINALROM
    if (!error)
    {
        int i;

        for (i = 0; i < stpDwcCnt->matchcnt.gt2NumConnection + 1; i++)
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "aid[%d] = %d, pid[%d] = %u\n", i,
                stpDwcCnt->matchcnt.aidList[i], i,
                stpDwcCnt->matchcnt.sbPidList[i]);
        }
    }
#endif

    stpDwcCnt->matchcnt.validAidBitmap = DWCi_GetAIDBitmapFromList(
        stpDwcCnt->matchcnt.aidList,
        stpDwcCnt->matchcnt.gt2NumConnection + 1);
    DWCi_SetNumValidConnection();

    if (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV
        || stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_CL)
    {
        stpDwcCnt->matchedSCCallback(error, cancel, self, isServer, index,
            stpDwcCnt->matchedSCParam);
    }
    else
    {
        stpDwcCnt->matchedCallback(
            error, cancel, stpDwcCnt->matchedParam);
    }

    if (error != DWC_ERROR_NONE && stpDwcCnt != NULL
        && stpDwcCnt->state == DWC_STATE_MATCHING)
    {
        DWCi_SetState(DWC_STATE_ONLINE);
    }
}

static void DWCi_GPErrorCallback(
    GPConnection* pconnection, GPErrorArg* arg, void* param)
{
#pragma unused(pconnection)
#pragma unused(param)
    gsi_char* errorCodeString;
    gsi_char* resultString;
    GPResult gpResult;

#ifdef DWC_IGNORE_GP_ERROR_ALREADY_BUDDY
    if (arg->errorCode == GP_ADDBUDDY_ALREADY_BUDDY
        || arg->errorCode == GP_BM_NOT_BUDDY
        || arg->errorCode == GP_DELBUDDY_NOT_BUDDY)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "Ignore GP ALLREADY_BUDDY or NOT_BUDDY %d.\n",
            arg->errorCode);
        return;
    }
#endif

#define RESULT(x) case x: resultString = #x; break;
    switch (arg->result)
    {
        RESULT(GP_NO_ERROR)
        RESULT(GP_MEMORY_ERROR)
        RESULT(GP_PARAMETER_ERROR)
        RESULT(GP_NETWORK_ERROR)
        RESULT(GP_SERVER_ERROR)
    default:
        resultString = "Unknown result!\n";
    }

#define ERRORCODE(x) case x: errorCodeString = #x; break;
    switch (arg->errorCode)
    {
        ERRORCODE(GP_GENERAL)
        ERRORCODE(GP_PARSE)
        ERRORCODE(GP_NOT_LOGGED_IN)
        ERRORCODE(GP_BAD_SESSKEY)
        ERRORCODE(GP_DATABASE)
        ERRORCODE(GP_NETWORK)
        ERRORCODE(GP_FORCED_DISCONNECT)
        ERRORCODE(GP_CONNECTION_CLOSED)
        ERRORCODE(GP_LOGIN)
        ERRORCODE(GP_LOGIN_TIMEOUT)
        ERRORCODE(GP_LOGIN_BAD_NICK)
        ERRORCODE(GP_LOGIN_BAD_EMAIL)
        ERRORCODE(GP_LOGIN_BAD_PASSWORD)
        ERRORCODE(GP_LOGIN_BAD_PROFILE)
        ERRORCODE(GP_LOGIN_PROFILE_DELETED)
        ERRORCODE(GP_LOGIN_CONNECTION_FAILED)
        ERRORCODE(GP_LOGIN_SERVER_AUTH_FAILED)
        ERRORCODE(GP_NEWUSER)
        ERRORCODE(GP_NEWUSER_BAD_NICK)
        ERRORCODE(GP_NEWUSER_BAD_PASSWORD)
        ERRORCODE(GP_UPDATEUI)
        ERRORCODE(GP_UPDATEUI_BAD_EMAIL)
        ERRORCODE(GP_NEWPROFILE)
        ERRORCODE(GP_NEWPROFILE_BAD_NICK)
        ERRORCODE(GP_NEWPROFILE_BAD_OLD_NICK)
        ERRORCODE(GP_UPDATEPRO)
        ERRORCODE(GP_UPDATEPRO_BAD_NICK)
        ERRORCODE(GP_ADDBUDDY)
        ERRORCODE(GP_ADDBUDDY_BAD_FROM)
        ERRORCODE(GP_ADDBUDDY_BAD_NEW)
        ERRORCODE(GP_ADDBUDDY_ALREADY_BUDDY)
        ERRORCODE(GP_AUTHADD)
        ERRORCODE(GP_AUTHADD_BAD_FROM)
        ERRORCODE(GP_AUTHADD_BAD_SIG)
        ERRORCODE(GP_STATUS)
        ERRORCODE(GP_BM)
        ERRORCODE(GP_BM_NOT_BUDDY)
        ERRORCODE(GP_GETPROFILE)
        ERRORCODE(GP_GETPROFILE_BAD_PROFILE)
        ERRORCODE(GP_DELBUDDY)
        ERRORCODE(GP_DELBUDDY_NOT_BUDDY)
        ERRORCODE(GP_DELPROFILE)
        ERRORCODE(GP_DELPROFILE_LAST_PROFILE)
        ERRORCODE(GP_SEARCH)
        ERRORCODE(GP_SEARCH_CONNECTION_FAILED)
    default:
        errorCodeString = "Unknown error code!\n";
    }

    if (arg->fatal)
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR, "FATAL ERROR\n");
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_ERROR, "ERROR\n");
    }

    DWC_Printf(DWC_REPORTFLAG_ERROR, "RESULT: %s (%d)\n", resultString,
        arg->result);
    DWC_Printf(DWC_REPORTFLAG_ERROR, "ERROR CODE: %s (0x%X)\n",
        errorCodeString, arg->errorCode);
    DWC_Printf(
        DWC_REPORTFLAG_ERROR, "ERROR STRING: %s\n", arg->errorString);

    gpResult = GP_NETWORK_ERROR;
    (void)DWCi_HandleGPError(gpResult);
}

static void DWCi_GPRecvBuddyMessageCallback(
    GPConnection* pconnection, GPRecvBuddyMessageArg* arg, void* param)
{
#pragma unused(param)
    char version[12] = { 0, };
    char* message = arg->message;
    u32 len;

    if (memcmp(message, DWC_GP_COMMAND_STRING,
            strlen(DWC_GP_COMMAND_STRING)))
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "Received undefined buddy message. '%s'\n", message);
        return;
    }

    message += strlen(DWC_GP_COMMAND_STRING);
    len = (u32)(strchr(message, 'v') - message);
    strncpy(version, message, len);
    if (len > 10
        || strtoul(version, NULL, 10) != DWC_MATCHING_VERSION)
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "Received different version buddy message command. '%s'\n",
            message);
        return;
    }

    message += len + 1;
    if (memcmp(message, DWC_GP_COMMAND_MATCH_STRING,
            strlen(DWC_GP_COMMAND_MATCH_STRING)) == 0)
    {
        if (stpDwcCnt->state == DWC_STATE_MATCHING
            || (stpDwcCnt->state == DWC_STATE_CONNECTED
                && (stpDwcCnt->matchcnt.qr2MatchType
                        == DWC_MATCH_TYPE_SC_SV
                    || stpDwcCnt->matchcnt.qr2MatchType
                        == DWC_MATCH_TYPE_SC_CL)))
        {
            message += strlen(DWC_GP_COMMAND_MATCH_STRING);
            DWCi_MatchGPRecvBuddyMsgCallback(
                pconnection, arg->profile, message);
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_DEBUG,
                "Ignore delayed GP matching command.\n");
        }
    }
}

static void DWCi_GT2ReceivedCallback(
    GT2Connection connection, GT2Byte* message, int len, GT2Bool reliable)
{
    DWCi_RecvCallback(connection, message, len, reliable);
}

static void DWCi_GT2ClosedCallback(
    GT2Connection connection, GT2CloseReason reason)
{
    u8 aid;
    int profileID = 0;
    int errorCode;
    int i;
    BOOL isServer = FALSE;
    BOOL aidValid;
    DWCConnectionInfo* conninfo;
    DWCError dwcError;

    if (DWCi_IsShutdownMatch())
    {
        DWC_Printf(DWC_REPORTFLAG_WARNING,
            "Called DWC_ShutdownFriendsMatch() with unexpected status.\n");
        return;
    }

    switch (reason)
    {
    case GT2LocalClose:
    case GT2RemoteClose:
        dwcError = DWC_ERROR_NONE;
        break;
    case GT2CommunicationError:
    case GT2SocketError:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_GS_GT2 + DWC_ECODE_TYPE_CLOSE;
        break;
    case GT2NotEnoughMemory:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_GS_GT2 + DWC_ECODE_TYPE_CLOSE
            + DWC_ECODE_TYPE_ALLOC;
        break;
    }

    DWC_Printf(DWC_REPORTFLAG_DEBUG,
        "Connection was closed (reason %d).\n", reason);

    if (!dwcError)
    {
        conninfo = (DWCConnectionInfo*)gt2GetConnectionData(connection);

        if (!conninfo)
        {
            return;
        }

        aid = conninfo->aid;

        if (stpDwcCnt->matchcnt.validAidBitmap & (1 << aid))
        {
            aidValid = TRUE;
        }
        else
        {
            aidValid = FALSE;
        }

        DWCi_ClearTransConnection(aid);

        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "aid = %d (validity %d).\n", aid, aidValid);

        if ((stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV
                && reason == GT2LocalClose)
            || (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_CL
                && aid == 0))
        {
            isServer = TRUE;
        }

        profileID = DWCi_DeleteAID(aid);

        stGt2ConnectionList[conninfo->index] = NULL;

        stpDwcCnt->matchcnt.gt2NumConnection--;
        stpDwcCnt->matchcnt.qr2NNFinishCount--;
    }

    if (!stpDwcCnt->ownCloseFlag
        && stpDwcCnt->state == DWC_STATE_CONNECTED && !aidValid)
    {
        if (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV
            && dwcError == DWC_ERROR_NONE)
        {
            (void)DWCi_GPSetServerStatus();
            DWCi_ProcessMatchSCClosing(profileID);
        }

        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "Closing process by matching SC.\n");
        return;
    }
    else if (DWCi_ProcessMatchClosing(dwcError, errorCode, profileID))
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "Closing process by matching.\n");
        return;
    }

    if (dwcError)
    {
        DWCi_SetError(dwcError, errorCode);
        return;
    }

    if (!stpDwcCnt->ownCloseFlag
        && (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV
            || stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_CL)
        && stpDwcCnt->matchcnt
                .sbPidList[stpDwcCnt->matchcnt.gt2NumConnection + 2]
            != 0)
    {
        stpDwcCnt->matchcnt
            .aidList[stpDwcCnt->matchcnt.gt2NumConnection + 1]
            = stpDwcCnt->matchcnt
                  .aidList[stpDwcCnt->matchcnt.gt2NumConnection + 2];

        (void)DWCi_DeleteHostByIndex(
            stpDwcCnt->matchcnt.gt2NumConnection + 1,
            stpDwcCnt->matchcnt.gt2NumConnection + 3);
    }

    if (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV)
    {
        if (!stpDwcCnt->ownCloseFlag)
        {
            (void)DWCi_GPSetServerStatus();
        }
        else if (stpDwcCnt->matchcnt.gt2NumConnection == 0)
        {
            (void)DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);
        }
    }
    else if (stpDwcCnt->matchcnt.gt2NumConnection == 0)
    {
        (void)DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);
    }

    for (i = 0; i < stpDwcCnt->matchcnt.gt2NumConnection + 1; i++)
    {
        DWC_Printf(DWC_REPORTFLAG_DEBUG,
            "aid[%d] = %d, pid[%d] = %u\n", i,
            stpDwcCnt->matchcnt.aidList[i], i,
            stpDwcCnt->matchcnt.sbPidList[i]);
    }

    if (stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_ANYBODY
        || stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_FRIEND)
    {
        stpDwcCnt->matchcnt.qr2NumEntry
            = stpDwcCnt->matchcnt.qr2NNFinishCount;
        qr2_send_statechanged(stpDwcCnt->matchcnt.qr2Obj);
    }

    if (stpDwcCnt->closedCallback && aidValid)
    {
        stpDwcCnt->closedCallback(dwcError,
            reason == GT2LocalClose ? TRUE : FALSE, isServer, aid,
            DWCi_GetFriendListIndex(profileID), stpDwcCnt->closedParam);
    }

    if (!stpDwcCnt->ownCloseFlag
        && stpDwcCnt->matchcnt.qr2MatchType == DWC_MATCH_TYPE_SC_SV)
    {
        return;
    }

    if (!stpDwcCnt->matchcnt.gt2NumConnection)
    {
        DWCi_NNFreeNegotiateList();
        DWCi_ClearQR2Key();
        DWCi_SetState(DWC_STATE_ONLINE);
    }
}

static void DWCi_GT2PingCallback(GT2Connection connection, int latency)
{
#pragma unused(connection)
    DWC_Printf(DWC_REPORTFLAG_TRANSPORT, "Ping: %dms\n", latency);
    DWCi_PingCallback(connection, latency);
}

static void DWCi_GT2SocketErrorCallback(GT2Socket socket)
{
    stLastSocketError = GOAGetLastError(gt2GetSocketSOCKET(socket));

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Socket fatal error! (%d)\n",
        stLastSocketError);

    DWCi_SetError(DWC_ERROR_FATAL,
        DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_GT2
            + DWC_ECODE_TYPE_SO_SOCKET);
    stpDwcCnt->gt2Socket = NULL;
}
