#pragma once

#include <dwc/dwc_error.h>
#include <dwc/dwc_match_fwd.h>
#include <gamespy/GP/gp.h>
#include <gamespy/qr2/qr2.h>
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DWC_MATCHING_VERSION 3U
#define DWC_MAX_CONNECTIONS  32
#define DWC_MAX_MATCH_IDX_LIST 64
#define DWC_SB_COMMAND_STRING "SBCM"

#define DWC_QR2_ALIVE_DURING_MATCHING
#define DWC_MATCH_ACCEPT_NO_FRIEND
#define DWC_LIMIT_FRIENDS_MATCH_VALID
#define DWC_STOP_SC_SERVER

#define DWC_MATCH_COMMAND_RESERVATION  0x01
#define DWC_MATCH_COMMAND_RESV_OK      0x02
#define DWC_MATCH_COMMAND_RESV_DENY    0x03
#define DWC_MATCH_COMMAND_RESV_WAIT    0x04
#define DWC_MATCH_COMMAND_RESV_CANCEL  0x05
#define DWC_MATCH_COMMAND_TELL_ADDR    0x06
#define DWC_MATCH_COMMAND_NEW_PID_AID  0x07
#define DWC_MATCH_COMMAND_LINK_CLS_REQ 0x08
#define DWC_MATCH_COMMAND_LINK_CLS_SUC 0x09
#define DWC_MATCH_COMMAND_CLOSE_LINK   0x0A
#define DWC_MATCH_COMMAND_RESV_PRIOR   0x0B
#define DWC_MATCH_COMMAND_CANCEL       0x0C
#define DWC_MATCH_COMMAND_CANCEL_SYN   0x0D
#define DWC_MATCH_COMMAND_CANCEL_SYN_ACK 0x0E
#define DWC_MATCH_COMMAND_CANCEL_ACK     0x0F
#define DWC_MATCH_COMMAND_SC_CLOSE_CL  0x10
#define DWC_MATCH_COMMAND_POLL_TIMEOUT 0x11
#define DWC_MATCH_COMMAND_POLL_TO_ACK  0x12
#define DWC_MATCH_COMMAND_SC_CONN_BLOCK 0x13
#define DWC_MATCH_COMMAND_FRIEND_ACCEPT 0x20
#define DWC_MATCH_COMMAND_CL_WAIT_POLL  0x40
#define DWC_MATCH_COMMAND_SV_KA_TO_CL   0x41
#define DWC_MATCH_COMMAND_DUMMY         0xFF

#define DWC_MATCH_RESV_DENY_REASON_SV_FULL 0x10
#define DWC_MATCH_CMD_RESV_TIMEOUT_MSEC 6000
#define DWC_MATCH_CMD_RESEND_INTERVAL_MSEC 3000
#define DWC_MATCH_CMD_RESEND_INTERVAL_ADD_MSEC 3000
#define DWC_SB_UPDATE_INTERVAL_MSEC 3000
#define DWC_SB_UPDATE_INTERVAL_SHORT_MSEC 1000
#define DWC_SB_UPDATE_INTERVAL_ADD_MSEC 3000
#define DWC_MATCH_CMD_RTT_TIMEOUT       6000U
#define DWC_MATCH_SB_UPDATE_TIMEOUT     13000
#define DWC_MATCH_CMD_RETRY_MAX         5
#define DWC_MATCH_SYN_ACK_WAIT_TIME     DWC_MATCH_CMD_RTT_TIMEOUT
#define DWC_MATCH_CANCEL_SYN_ACK_WAIT_TIME DWC_MATCH_CMD_RTT_TIMEOUT
#define DWC_MATCH_RESV_KEEP_TIME_ANYBODY DWC_MATCH_CMD_RTT_TIMEOUT
#define DWC_MATCH_RESV_KEEP_TIME_FRIEND                                      \
    (DWC_MATCH_CMD_RTT_TIMEOUT + DWC_MATCH_SB_UPDATE_TIMEOUT)
#define DWC_WAIT_NN_RETRY_TIMEOUT 10000
#define DWC_WAIT_GT2_CONNECT_TIMEOUT                                         \
    (DWC_GT2_CONNECT_TIMEOUT * DWC_MATCH_CMD_RETRY_MAX)
#define DWC_MATCH_LINK_CLS_TIMEOUT 30000
#define DWC_MATCH_CL_WAIT_TIMEOUT 30000
#define DWC_MATCH_OPT_MIN_COMP_POLL_RETRY_MAX 5
#define DWC_MAX_MATCH_NN_RETRY  1
#define DWC_MATCH_NN_FAILURE_MAX 5
#define DWC_GT2_CONNECT_TIMEOUT 5000
#define DWC_MATCH_SYN_DATA_BODY_SIZE 4
#define DWC_RESV_COMMAND_RETRY_MAX      16
#define DWC_SB_UPDATE_INTERVAL_SHORT    1
#define DWC_SB_UPDATE_INTERVAL_LONG     2

#define DWC_QR2_PID_KEY        NUM_RESERVED_KEYS
#define DWC_QR2_MATCH_TYPE_KEY (NUM_RESERVED_KEYS + 1)
#define DWC_QR2_MATCH_RESV_KEY (NUM_RESERVED_KEYS + 2)
#define DWC_QR2_MATCH_VER_KEY  (NUM_RESERVED_KEYS + 3)
#define DWC_QR2_MATCH_EVAL_KEY (NUM_RESERVED_KEYS + 4)

#define DWC_QR2_RESERVED_KEYS      (100 - NUM_RESERVED_KEYS)
#define DWC_QR2_GAME_RESERVED_KEYS                                             \
    (MAX_REGISTERED_KEYS - NUM_RESERVED_KEYS - DWC_QR2_RESERVED_KEYS)
#define DWC_QR2_GAME_KEY_START 100

#define DWC_QR2_PID_KEY_STR        "dwc_pid"
#define DWC_QR2_MATCH_TYPE_KEY_STR "dwc_mtype"
#define DWC_QR2_MATCH_RESV_KEY_STR "dwc_mresv"
#define DWC_QR2_MATCH_VER_KEY_STR  "dwc_mver"
#define DWC_QR2_MATCH_EVAL_KEY_STR "dwc_eval"

#define DWC_SB_UPDATE_MAX_SERVERS 6


    typedef struct DWCstSBMessageHeader
    {
        char identifier[4];
        u32 version;
        u8 command;
        u8 size;
        u16 qr2Port;
        u32 qr2IP;
        int profileID;
    } DWCSBMessageHeader;

    typedef struct DWCstSBMessage
    {
        DWCSBMessageHeader header;
        u32 data[DWC_MAX_CONNECTIONS];
    } DWCSBMessage;

    typedef struct DWCstGameMatchKeyData
    {
        u8 keyID;
        u8 isStr;
        u16 pad;
        char* keyStr;
        const void* value;
    } DWCGameMatchKeyData;

    typedef struct DWCstMatchOptMinCompleteIn
    {
        u8 valid;
        u8 minEntry;
        u8 retry;
        u8 pad;
        u32 timeout;
        u32 recvBit;
        u32 timeoutBit;
        OSTime startTime;
        OSTime lastPollTime;
    } DWCMatchOptMinCompleteIn;

    typedef enum DWCMatchState
    {
        DWC_MATCH_STATE_INIT = 0,
        DWC_MATCH_STATE_CL_WAITING,
        DWC_MATCH_STATE_CL_SEARCH_OWN,
        DWC_MATCH_STATE_CL_SEARCH_HOST,
        DWC_MATCH_STATE_CL_WAIT_RESV,
        DWC_MATCH_STATE_CL_SEARCH_NN_HOST,
        DWC_MATCH_STATE_CL_NN,
        DWC_MATCH_STATE_CL_GT2,
        DWC_MATCH_STATE_CL_CANCEL_SYN,
        DWC_MATCH_STATE_CL_SYN,
        DWC_MATCH_STATE_SV_WAITING,
        DWC_MATCH_STATE_SV_OWN_NN,
        DWC_MATCH_STATE_SV_OWN_GT2,
        DWC_MATCH_STATE_SV_WAIT_CL_LINK,
        DWC_MATCH_STATE_SV_CANCEL_SYN,
        DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT,
        DWC_MATCH_STATE_SV_SYN,
        DWC_MATCH_STATE_SV_SYN_WAIT,
        DWC_MATCH_STATE_WAIT_CLOSE,
        DWC_MATCH_STATE_SV_POLL_TIMEOUT,
        DWC_MATCH_STATE_NUM
    } DWCMatchState;

    enum
    {
        DWC_MATCH_TYPE_ANYBODY = 0,
        DWC_MATCH_TYPE_FRIEND,
        DWC_MATCH_TYPE_SC_SV,
        DWC_MATCH_TYPE_SC_CL,
        DWC_MATCH_TYPE_NUM
    };


    enum
    {
        DWC_SET_MATCH_OPT_RESULT_SUCCESS = 0,
        DWC_SET_MATCH_OPT_RESULT_E_BAD_STATE,
        DWC_SET_MATCH_OPT_RESULT_E_INVALID,
        DWC_SET_MATCH_OPT_RESULT_E_PARAM,
        DWC_SET_MATCH_OPT_RESULT_E_ALLOC,
        DWC_SET_MATCH_OPT_RESULT_NUM
    };


    typedef struct DWCMatchOptSCConnectBlock
    {
        u32 valid;
    } DWCMatchOptSCConnectBlock;


    BOOL DWC_RegisterMatchingStatus(void);
    u8 DWC_AddMatchKeyString(u8 keyID, const char* keyString,
        const char* valueSrc);
    const char* DWC_GetMatchStringValue(
        int index, const char* keyString, const char* sdefault);
    int DWC_GetLastMatchingType(void);
    int DWC_GetMatchingOption(
        DWCMatchOptType opttype, void* optval, int* optlen);
    int DWC_GetMOMinCompState(u64* time);
    BOOL DWC_GetMOSCConnectBlockState(void);
    void DWC_ClearMOSCConnectBlock(void);
    DWCMatchState DWC_GetMatchingState(void);
    qr2_error_t DWCi_QR2Startup(int profileID);
    void DWCi_ConnectToFriendsAsync(const u8 friendIdxList[],
        int friendIdxListLen, u8 numEntry, BOOL distantFriend,
        DWCMatchedSCCallback matchedCallback, void* matchedParam,
        DWCEvalPlayerCallback evalCallback, void* evalParam);
    void DWCi_ConnectToGameServerAsync(int serverPid,
        DWCMatchedSCCallback matchedCallback, void* matchedParam,
        DWCNewClientCallback newClientCallback, void* newClientParam);
    void DWCi_ProcessMatchSCClosing(int clientPid);
    BOOL DWCi_ProcessMatchClosing(
        DWCError error, int errorCode, int profileID);
    void DWCi_MatchGPRecvBuddyMsgCallback(
        GPConnection* connection, u32 profileId, char* message);
    void DWCi_StopMatching(DWCError error, int errorCode);
    void DWCi_ClearQR2Key(void);
    BOOL DWCi_DeleteHostByProfileID(int profileID, int numHost);
    int DWCi_DeleteHostByIndex(int index, int numHost);
    int DWCi_GetNumAllConnection(void);
    int DWCi_GetNumValidConnection(void);
    int DWCi_GetAllAIDList(u8** aidList);
    void DWCi_SetNumValidConnection(void);
    GPResult DWCi_GPSetServerStatus(void);
    BOOL DWCi_IsShutdownMatch(void);
    void DWCi_ProcessMatchSynPacket(u8 aid, u16 type, u8* data);

#ifdef __cplusplus
}
#endif
