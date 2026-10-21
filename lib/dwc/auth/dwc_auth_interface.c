#include <dwc/dwc_auth_interface.h>
#include <dwc/dwc_base64.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <private/nand.h>
#include <revolution/nand.h>
#include <revolution/ncd.h>
#include <revolution/nhttp.h>
#include <revolution/os.h>
#include <revolution/os/OSError.h>
#include <revolution/os/OSThread.h>
#include <revolution/os/OSTime.h>
#include <revolution/sc.h>
#include <revolution/so.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

enum
{
    DWCi_AUTH_STATE_NOT_STARTED = 0,
    DWCi_AUTH_STATE_FILE_OPEN_FOR_READ = 1,
    DWCi_AUTH_STATE_FILE_OPEN_FOR_READ_WAIT = 2,
    DWCi_AUTH_STATE_FILE_READ = 3,
    DWCi_AUTH_STATE_FILE_READ_WAIT = 4,
    DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE = 5,
    DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE_WAIT = 6,
    DWCi_AUTH_STATE_FILE_DELETE = 7,
    DWCi_AUTH_STATE_FILE_DELETE_WAIT = 8,
    DWCi_AUTH_STATE_CREATE_ACCOUNT = 9,
    DWCi_AUTH_STATE_CREATE_ACCOUNT_WAIT = 10,
    DWCi_AUTH_STATE_FILE_CREATE = 11,
    DWCi_AUTH_STATE_FILE_CREATE_WAIT = 12,
    DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE = 13,
    DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE_WAIT = 14,
    DWCi_AUTH_STATE_FILE_WRITE = 15,
    DWCi_AUTH_STATE_FILE_WRITE_WAIT = 16,
    DWCi_AUTH_STATE_FILE_CLOSE = 17,
    DWCi_AUTH_STATE_FILE_CLOSE_WAIT = 18,
    DWCi_AUTH_STATE_GET_TOKEN = 19,
    DWCi_AUTH_STATE_GET_TOKEN_WAIT = 20,
    DWCi_AUTH_STATE_CANCEL_NHTTP = 21,
    DWCi_AUTH_STATE_CANCEL_NHTTP_WAIT = 22,
    DWCi_AUTH_STATE_CLEANUP_NHTTP = 23,
    DWCi_AUTH_STATE_CLEANUP_NHTTP_WAIT = 24,
    DWCi_AUTH_STATE_FINISHED = 25,
    DWCi_AUTH_STATE_IDLE = 26,
    DWCi_AUTH_STATE_NONE = 27,
};

enum
{
    DWCi_AUTH_KIND_ACCTCREATE = 0,
    DWCi_AUTH_KIND_LOGIN = 1,
    DWCi_AUTH_KIND_SVCLOC = 2,
    DWCi_AUTH_KIND_PROFCHECK = 3,
};

static const char* s_filename = "/shared2/DWC_AUTHDATA";

static char* s_authserver_hosts[3] = {
    "https://naswii.test.nintendowifi.net/ac",
    "https://naswii.nintendowifi.net/ac",
    "https://naswii.dev.nintendowifi.net/ac",
};

// the profanity check's servers
static char* s_profserver_hosts[3] = {
    "https://naswii.test.nintendowifi.net/pr",
    "https://naswii.nintendowifi.net/pr",
    "https://naswii.dev.nintendowifi.net/pr",
};

static struct
{
    volatile s32 errorcode; // set from the NHTTP callback
    char token[301];
    char challenge[9];
    char cookie[65];
    char locator[51];
    u8 _1AE[2];
    u64 userid;
    DWCIngamesnCheckResult ingamesn_check_result;
    u8 _1BC[4];
    OSTime server_timediff;
    NHTTPResponse* res;
} s_auth_result;

static struct
{
    s32 status;
    char svlhost[65];
    char svltoken[301];
} s_svl_result;

// the console's own user id, saved in NAND (32-byte aligned NAND buffer)
static struct
{
    u64 userid;
    u8 padding[24];
} s_auth_console_save_data ATTRIBUTE_ALIGN(32);

// the profanity check's reply (HandleResponse's "prwords=")
static struct
{
    char prwords[51];
    BOOL connerr;
} s_prof_result;

// changed from the NAND and NHTTP callbacks
static volatile s32 s_auth_state;

static struct
{
    u8 nwc24_work[0x4000];
    NCDIfConfig ifconfig;
    u16 ingamesn[26];
    char gsbrcd[12];
    char svc[5];
    const u16* prof_words;   // profanity check request
    int prof_words_bytes;
    s32 prof_userid_valid;   // 1: the profanity request also sends userid
    char prof_region[2];
    char read_buf[4096];
    char header_buf[2048];
    s32 retry_num;
    BOOL is_nand_end;
    s32 nand_result;
    BOOL is_aborting;
    s32 request_id;
    s32 auth_kind;
    NANDCommandBlock nand_command_block;
    NANDFileInfo fileinfo;
    DWCAllocFunc auth_alloc;
    DWCFreeFunc auth_free;
    OSTime start_time;
    u64 game_save_userid;
}* s_auth_work;

static DWC_AuthServer s_authserver;
// TRUE: authenticate with the user id the game passed in instead of the console's saved one
static BOOL s_useGameSaveUserId;

static void* nhttp_alloc(u32 size, int align);
static void nhttp_free(void* buffer);
static void nand_callback(s32 result, NANDCommandBlock* block);
static char* base64nize(const char* str, char** ptr);
static char* base64nize_with_len(void* str, char** ptr, u32 len);
static void DWCi_Auth_SendRequest(s32 action_kind, const u16* ingamesn,
    const char* gsbrcd, u64 userid);
static void DWCi_Auth_HandleResponse(
    NHTTPErr error, NHTTPResponse* response, void* user_param);
static OSTime DWCi_Auth_DateTimeToOSTime(const char* datetime);
static void DWCi_Auth_EndProcess(void);
static void DWCi_Auth_HandleNandResult(
    s32 ok_state, s32 retry_state, s32 noexists_state, s32 eaccess_state);
static char* DWCi_Auth_GetHost(void);

BOOL DWCi_Auth_InitInterface(DWC_AuthServer authServer)
{
    memset(&s_auth_result, 0, sizeof(s_auth_result));
    memset(&s_svl_result, 0, sizeof(s_svl_result));
    memset(&s_auth_console_save_data, 0, sizeof(s_auth_console_save_data));
    s_auth_work = NULL;
    s_auth_state = DWCi_AUTH_STATE_NOT_STARTED;
    s_authserver = authServer;
    s_auth_result.ingamesn_check_result = DWC_INGAMESN_NOT_CHECKED;
    s_useGameSaveUserId = FALSE;
    return TRUE;
}

DWC_AuthServer DWCi_Auth_GetServer(void)
{
    return s_authserver;
}

int DWCi_Auth_StartAuthentication(const u16* ingamesn, const char* gsbrcd,
    u64 userid, DWCAllocFunc allocator, DWCFreeFunc deallocator)
{
    s32 result;

    if (s_auth_state != DWCi_AUTH_STATE_NOT_STARTED && s_auth_state != DWCi_AUTH_STATE_IDLE)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, " auth is processing\n");
        return FALSE;
    }

    s_auth_work = allocator(DWC_ALLOCTYPE_AUTH, sizeof(*s_auth_work));
    if (s_auth_work == NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, " memory shortage\n");
        return FALSE;
    }

    memset(s_auth_work, 0, sizeof(*s_auth_work));
    s_auth_work->auth_alloc = allocator;
    s_auth_work->auth_free = deallocator;

    result = NCDGetCurrentIfConfig(&s_auth_work->ifconfig);
    if (result != NCD_RESULT_SUCCESS)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, " NCDGetCurrentIfConfig failed.[%d]\n", result);
        goto error;
    }

    result = NHTTPStartup(nhttp_alloc, nhttp_free, 0x11);
    if (result < 0)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, " failed to start NHTTP\n");
        goto error;
    }

    wcsncpy(s_auth_work->ingamesn, ingamesn, 26);
    strncpy(s_auth_work->gsbrcd, gsbrcd, 12);
    s_auth_work->auth_kind = DWCi_AUTH_KIND_LOGIN;
    memset(&s_auth_result, 0, sizeof(s_auth_result));
    s_auth_result.ingamesn_check_result = DWC_INGAMESN_NOT_CHECKED;
    // only used when s_useGameSaveUserId is set; the console's saved id is read from NAND first
    s_auth_work->game_save_userid = userid;
    s_auth_state = DWCi_AUTH_STATE_FILE_OPEN_FOR_READ;

    return TRUE;

error:
    s_auth_work->auth_free(DWC_ALLOCTYPE_AUTH, s_auth_work, 0);
    return FALSE;
}

// service locator request (svc = service code); stripped in CF
BOOL DWCi_Auth_Svl_StartAuthentication(
    const char* svc, DWCAllocFunc allocator, DWCFreeFunc deallocator)
{
    DWCi_Auth_StartAuthentication((const u16*)"", "", 0, allocator, deallocator);
    memset(&s_svl_result, 0, sizeof(s_svl_result));
    strncpy(s_auth_work->svc, svc, 5);
    s_auth_work->auth_kind = DWCi_AUTH_KIND_SVCLOC;
    return TRUE;
}

// profanity check of words (region: one character); stripped in CF
BOOL DWCi_Auth_Prof_StartAuthentication(const u16* words, int words_bytes,
    char region, DWCAllocFunc allocator, DWCFreeFunc deallocator)
{
    memset(&s_prof_result, 0, sizeof(s_prof_result));
    s_prof_result.connerr = FALSE;
    DWCi_Auth_StartAuthentication((const u16*)"", "", 0, allocator, deallocator);
    s_auth_work->prof_words = words;
    s_auth_work->prof_words_bytes = words_bytes;
    s_auth_work->prof_region[0] = region;
    s_auth_work->prof_region[1] = '\0';
    s_auth_work->auth_kind = DWCi_AUTH_KIND_PROFCHECK;
    if (s_auth_console_save_data.userid == 0)
    {
        s_auth_work->prof_userid_valid = 0;
    }
    else
    {
        s_auth_work->prof_userid_valid = 1;
    }
    return TRUE;
}

void DWCi_Auth_AbortAuthentication(void)
{
    BOOL isNandState;
    BOOL cleanuped;
    OSTime starttime;

    s_auth_work->is_aborting = TRUE;

    // let a NAND operation in progress run to its end first
    isNandState = TRUE;
    while (isNandState)
    {
        switch (s_auth_state)
        {
        case DWCi_AUTH_STATE_FILE_OPEN_FOR_READ_WAIT:
        case DWCi_AUTH_STATE_FILE_READ:
        case DWCi_AUTH_STATE_FILE_READ_WAIT:
        case DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE:
        case DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE_WAIT:
        case DWCi_AUTH_STATE_FILE_DELETE:
        case DWCi_AUTH_STATE_FILE_DELETE_WAIT:
        case DWCi_AUTH_STATE_FILE_CREATE:
        case DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE:
        case DWCi_AUTH_STATE_FILE_WRITE:
        case DWCi_AUTH_STATE_FILE_WRITE_WAIT:
        case DWCi_AUTH_STATE_FILE_CLOSE:
        case DWCi_AUTH_STATE_FILE_CLOSE_WAIT:
            DWCi_Auth_ProcessAuthentication();
            OSSleepTicks(OSMillisecondsToTicks(1));
            break;
        default:
            isNandState = FALSE;
            break;
        }
    }

    if (s_auth_state == DWCi_AUTH_STATE_GET_TOKEN_WAIT)
    {
        NHTTPCancelRequestAsync(s_auth_work->request_id);
        for (;;)
        {
            if (s_auth_result.errorcode != 0)
            {
                break;
            }
            OSSleepTicks(OSMillisecondsToTicks(1));
        }
    }

    if (s_auth_result.res != NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, "NHTTPDestroyResponse()\n");
        NHTTPDestroyResponse(s_auth_result.res);
        s_auth_result.res = NULL;
    }

    if (s_auth_state != DWCi_AUTH_STATE_NOT_STARTED && s_auth_state != DWCi_AUTH_STATE_CLEANUP_NHTTP_WAIT
        && s_auth_state != DWCi_AUTH_STATE_FINISHED && s_auth_state != DWCi_AUTH_STATE_IDLE)
    {
        cleanuped = FALSE;
        starttime = OSGetTime();
        s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP_WAIT;
        NHTTPCleanupAsync(DWCi_Auth_EndProcess);
        for (;;)
        {
            if (s_auth_state == DWCi_AUTH_STATE_FINISHED)
            {
                break;
            }
            OSSleepTicks(OSMillisecondsToTicks(1));

            // NHTTP cleanup stuck for over 6 seconds: shut the sockets down to release it
            if (!cleanuped
                && OSTicksToMilliseconds(OSGetTime() - starttime) > 6000)
            {
                cleanuped = TRUE;
                SOCleanup();
            }
        }
    }

    s_auth_state = DWCi_AUTH_STATE_IDLE;
    s_auth_result.errorcode = -20100;
}

void DWCi_Auth_ProcessAuthentication(void)
{
    switch (s_auth_state)
    {
        case DWCi_AUTH_STATE_FILE_OPEN_FOR_READ:
            if (s_auth_work->auth_kind == DWCi_AUTH_KIND_PROFCHECK)
            {
                s_auth_state = DWCi_AUTH_STATE_GET_TOKEN; // no saved userid needed
                break;
            }
            if (s_auth_console_save_data.userid != 0)
            {
                s_auth_state = DWCi_AUTH_STATE_GET_TOKEN; // already read
                break;
            }
            NANDPrivateOpenAsync(s_filename, &s_auth_work->fileinfo,
                NAND_ACCESS_READ, nand_callback, &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_OPEN_FOR_READ_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_OPEN_FOR_READ_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_READ,
                DWCi_AUTH_STATE_FILE_OPEN_FOR_READ, DWCi_AUTH_STATE_CREATE_ACCOUNT,
                DWCi_AUTH_STATE_FILE_DELETE);
            break;

        case DWCi_AUTH_STATE_FILE_READ:
            NANDReadAsync(&s_auth_work->fileinfo, &s_auth_console_save_data,
                sizeof(s_auth_console_save_data), nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_READ_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_READ_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }

            if (s_auth_work->nand_result == sizeof(s_auth_console_save_data))
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH, " read userid = %llu\n",
                    s_auth_console_save_data.userid);
                s_auth_work->nand_result = NAND_RESULT_OK;
            }
            else if (s_auth_work->nand_result >= 0)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH,
                    " illegal size userid read = %d\n", s_auth_work->nand_result);
                s_auth_work->nand_result = NAND_RESULT_ACCESS;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_CLOSE,
                DWCi_AUTH_STATE_FILE_READ, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE);
            break;

        case DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE:
            NANDCloseAsync(&s_auth_work->fileinfo, nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_DELETE,
                DWCi_AUTH_STATE_FILE_CLOSE_FOR_DELETE, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_FILE_DELETE:
            NANDPrivateDeleteAsync(s_filename, nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_DELETE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_DELETE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            if (s_auth_work->nand_result == NAND_RESULT_OK)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH, " delete illegal userid.\n");
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_CREATE_ACCOUNT,
                DWCi_AUTH_STATE_FILE_DELETE, DWCi_AUTH_STATE_CREATE_ACCOUNT,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_CREATE_ACCOUNT:
            DWCi_Auth_SendRequest(DWCi_AUTH_KIND_ACCTCREATE, NULL, NULL, 0);
            s_auth_work->start_time = OSGetTime();
            s_auth_state = DWCi_AUTH_STATE_CREATE_ACCOUNT_WAIT;
            break;

        case DWCi_AUTH_STATE_CREATE_ACCOUNT_WAIT:
            if (s_auth_result.errorcode == 1)
            {
                s_auth_state = DWCi_AUTH_STATE_FILE_CREATE;
            }
            else if (s_auth_result.errorcode >= -20999
                && s_auth_result.errorcode <= -20102)
            {
                s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
            }
            else if (s_auth_result.errorcode < 0)
            {
                s_auth_work->retry_num++;
                if (s_auth_work->retry_num < 3)
                {
                    s_auth_state = DWCi_AUTH_STATE_CREATE_ACCOUNT;
                }
                else
                {
                    s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
                }
            }
            else if (DWCi_Np_TicksToMilliSeconds(
                         OSGetTime() - s_auth_work->start_time)
                > 30000)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH, " acctcreate timeout.\n");
                s_auth_state = DWCi_AUTH_STATE_CANCEL_NHTTP;
            }
            break;

        case DWCi_AUTH_STATE_FILE_CREATE:
            NANDPrivateCreateAsync(s_filename, NAND_PERM_ALL_RW, 0, nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_CREATE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_CREATE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE,
                DWCi_AUTH_STATE_FILE_CREATE, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE:
            NANDPrivateOpenAsync(s_filename, &s_auth_work->fileinfo,
                NAND_ACCESS_WRITE, nand_callback, &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_WRITE,
                DWCi_AUTH_STATE_FILE_OPEN_FOR_WRITE, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_FILE_WRITE:
            NANDWriteAsync(&s_auth_work->fileinfo, &s_auth_console_save_data,
                sizeof(s_auth_console_save_data), nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_WRITE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_WRITE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            if (s_auth_work->nand_result == sizeof(s_auth_console_save_data))
            {
                s_auth_work->nand_result = NAND_RESULT_OK;
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH,
                    " illegal size userid write = %d\n", s_auth_work->nand_result);
                s_auth_work->nand_result = NAND_RESULT_ACCESS;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_FILE_CLOSE,
                DWCi_AUTH_STATE_FILE_WRITE, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_FILE_CLOSE:
            NANDCloseAsync(&s_auth_work->fileinfo, nand_callback,
                &s_auth_work->nand_command_block);
            s_auth_state = DWCi_AUTH_STATE_FILE_CLOSE_WAIT;
            break;

        case DWCi_AUTH_STATE_FILE_CLOSE_WAIT:
            if (!s_auth_work->is_nand_end)
            {
                break;
            }
            DWCi_Auth_HandleNandResult(DWCi_AUTH_STATE_GET_TOKEN,
                DWCi_AUTH_STATE_FILE_CLOSE, DWCi_AUTH_STATE_NONE,
                DWCi_AUTH_STATE_NONE);
            break;

        case DWCi_AUTH_STATE_GET_TOKEN:
            if (s_useGameSaveUserId)
            {
                DWCi_Auth_SendRequest(s_auth_work->auth_kind, s_auth_work->ingamesn,
                    s_auth_work->gsbrcd, s_auth_work->game_save_userid);
            }
            else
            {
                DWCi_Auth_SendRequest(s_auth_work->auth_kind, s_auth_work->ingamesn,
                    s_auth_work->gsbrcd, s_auth_console_save_data.userid);
            }
            s_auth_work->start_time = OSGetTime();
            s_auth_state = DWCi_AUTH_STATE_GET_TOKEN_WAIT;
            break;

        case DWCi_AUTH_STATE_GET_TOKEN_WAIT:
            if (s_auth_result.errorcode == 1)
            {
                s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
            }
            else if (s_auth_result.errorcode >= -20999
                && s_auth_result.errorcode <= -20102)
            {
                s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
            }
            else if (s_auth_result.errorcode < 0)
            {
                s_auth_work->retry_num++;
                if (s_auth_work->retry_num < 3)
                {
                    s_auth_state = DWCi_AUTH_STATE_GET_TOKEN;
                }
                else
                {
                    s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
                }
            }
            else if (DWCi_Np_TicksToMilliSeconds(
                         OSGetTime() - s_auth_work->start_time)
                > 30000)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH, " login timeout.\n");
                s_auth_state = DWCi_AUTH_STATE_CANCEL_NHTTP;
            }
            break;

        case DWCi_AUTH_STATE_CANCEL_NHTTP:
            s_auth_state = DWCi_AUTH_STATE_CANCEL_NHTTP_WAIT;
            NHTTPCancelRequestAsync(s_auth_work->request_id);
            break;

        case DWCi_AUTH_STATE_CANCEL_NHTTP_WAIT:
            if (s_auth_result.errorcode != 0)
            {
                s_auth_result.errorcode = -20100;
                s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP;
            }
            break;

        case DWCi_AUTH_STATE_CLEANUP_NHTTP:
            s_auth_state = DWCi_AUTH_STATE_CLEANUP_NHTTP_WAIT;
            if (s_auth_result.res != NULL)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH, "NHTTPDestroyResponse()\n");
                NHTTPDestroyResponse(s_auth_result.res);
                s_auth_result.res = NULL;
            }
            NHTTPCleanupAsync(DWCi_Auth_EndProcess);
            break;

        case DWCi_AUTH_STATE_NOT_STARTED:
        case DWCi_AUTH_STATE_CLEANUP_NHTTP_WAIT:
        case DWCi_AUTH_STATE_FINISHED:
        case DWCi_AUTH_STATE_IDLE:
        default:
            break;
    }
}

BOOL DWCi_Auth_IsFinished(void)
{
    if (s_auth_state == DWCi_AUTH_STATE_FINISHED)
    {
        s_auth_state = DWCi_AUTH_STATE_IDLE;
        return TRUE;
    }

    if (s_auth_state == DWCi_AUTH_STATE_NOT_STARTED || s_auth_state == DWCi_AUTH_STATE_IDLE)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL DWCi_Auth_IsSucceeded(void)
{
    return s_auth_result.errorcode == 1;
}

int DWCi_Auth_GetErrorCode(void)
{
    return s_auth_result.errorcode;
}

void DWCi_Auth_GetGameSpyToken(char* authToken, char* partnerChallenge)
{
    strcpy(authToken, s_auth_result.token);
    strcpy(partnerChallenge, s_auth_result.challenge);
}

u64 DWCi_Auth_GetConsoleUserId(void)
{
    return s_auth_console_save_data.userid;
}

DWCIngamesnCheckResult DWCi_Auth_GetIngamesnCheckResult(void)
{
    return s_auth_result.ingamesn_check_result;
}

BOOL DWCi_Auth_GetServerTimeOffsetInSeconds(OSTime* offsetSeconds)
{
    if (s_auth_result.server_timediff == 0)
    {
        return FALSE;
    }

    *offsetSeconds = s_auth_result.server_timediff / OS_TIMER_CLOCK;
    return TRUE;
}

// stripped in CF: authenticate with the user id passed to DWCi_Auth_StartAuthentication
void DWCi_Auth_UseGameSaveUserId(BOOL use)
{
    s_useGameSaveUserId = use;
}

static void* nhttp_alloc(u32 size, int align)
{
    return s_auth_work->auth_alloc(DWC_ALLOCTYPE_NHTTP, size);
}

static void nhttp_free(void* buffer)
{
    s_auth_work->auth_free(DWC_ALLOCTYPE_NHTTP, buffer, 0);
}

static void nand_callback(s32 result, NANDCommandBlock* block)
{
    s_auth_work->is_nand_end = TRUE;
    s_auth_work->nand_result = result;
}

static char* base64nize(const char* str, char** ptr)
{
    return base64nize_with_len((void*)str, ptr, strlen(str));
}

static char* base64nize_with_len(void* str, char** ptr, u32 len)
{
    s32 rc;
    char* out;

    out = *ptr;
    rc = DWC_Base64Encode(str, len, out, sizeof(s_auth_work->header_buf));
    out[rc] = '\0';
    *ptr += rc + 1;
    return out;
}

static void DWCi_Auth_SendRequest(s32 action_kind, const u16* ingamesn,
    const char* gsbrcd, u64 userid)
{
    char strbuf[256];
    u8 macaddr[6];
    NHTTPRequest* request;
    char* b64_ptr;
    char* url;
    char path[64];
    s32 nand_ret;
    NANDStatus nand_status;
    OSCalendarTime td;
    char* product_code;
    u32 ser_no;

    b64_ptr = s_auth_work->header_buf;
    if (s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
    {
        url = s_authserver_hosts[s_authserver];
    }
    else
    {
        url = s_profserver_hosts[s_authserver];
    }
    DWC_Printf(DWC_REPORTFLAG_AUTH, " url = %s\n", url);

    request = NHTTPCreateRequest(url, NHTTP_REQMETHOD_POST,
        s_auth_work->read_buf, sizeof(s_auth_work->read_buf),
        DWCi_Auth_HandleResponse, NULL);

    if (NHTTPSetRootCADefault(request) != 0)
    {
        OSPanic("dwc_auth_interface.c", 1042,
            "\tDWC Auth: failed to set RootCA\n");
    }
    if (NHTTPSetClientCertDefault(request) != 0)
    {
        OSPanic("dwc_auth_interface.c", 1046,
            "\tDWC Auth: failed to set ClientCert\n");
    }

    NHTTPSetProxyDefault(request);
    NHTTPSetVerifyOption(request, 2);
    NHTTPAddHeaderField(request, "User-Agent", "RVL SDK/1.0");
    NHTTPAddHeaderField(request, "Host", DWCi_Auth_GetHost());
    NHTTPAddHeaderField(request, "HTTP_X_GAMECD", OSGetAppGamename());
    DWC_Printf(DWC_REPORTFLAG_AUTH, " HTTP_X_GAMECD = %s\n", OSGetAppGamename());

    switch (action_kind)
    {
        case DWCi_AUTH_KIND_ACCTCREATE:
            NHTTPAddPostDataAscii(request, "action",
                base64nize("acctcreate", &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " action = acctcreate\n");
            break;

        case DWCi_AUTH_KIND_LOGIN:
            NHTTPAddPostDataAscii(request, "action",
                base64nize("login", &b64_ptr));
            NHTTPAddPostDataAscii(request, "gsbrcd",
                base64nize(gsbrcd, &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " action = login\n");
            DWC_Printf(DWC_REPORTFLAG_AUTH, " gsbrcd = %s\n", gsbrcd);

            sprintf(strbuf, "%013llu", userid);
            NHTTPAddPostDataAscii(request, "userid",
                base64nize(strbuf, &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " userid = 0x%016llx\n", userid);
            NHTTPAddPostDataAscii(request, "ingamesn",
                base64nize_with_len(
                    (void*)ingamesn, &b64_ptr, wcslen(ingamesn) * sizeof(u16)));
            break;

        case DWCi_AUTH_KIND_SVCLOC:
            NHTTPAddPostDataAscii(request, "action",
                base64nize("svcloc", &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " action = svcloc\n");
            NHTTPAddPostDataAscii(request, "svc",
                base64nize(s_auth_work->svc, &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " svc = %s\n", s_auth_work->svc);

            sprintf(strbuf, "%013llu", userid);
            NHTTPAddPostDataAscii(request, "userid",
                base64nize(strbuf, &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " userid = 0x%016llx\n", userid);
            break;

        case DWCi_AUTH_KIND_PROFCHECK:
            if (s_auth_work->prof_userid_valid == 1)
            {
                sprintf(strbuf, "%013llu", userid);
                NHTTPAddPostDataAscii(request, "userid",
                    base64nize(strbuf, &b64_ptr));
                DWC_Printf(DWC_REPORTFLAG_AUTH, " userid = 0x%016llx\n", userid);
            }
            NHTTPAddPostDataAscii(request, "wregion",
                base64nize(s_auth_work->prof_region, &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " wregion = %s\n", s_auth_work->prof_region);
            NHTTPAddPostDataAscii(request, "wtype", "");
            DWC_Printf(DWC_REPORTFLAG_AUTH, " wtype = \n");
            NHTTPAddPostDataAscii(request, "wenc",
                base64nize("UTF-16BE", &b64_ptr));
            DWC_Printf(DWC_REPORTFLAG_AUTH, " wenc = UTF-16BE\n");
            NHTTPAddPostDataAscii(request, "words",
                base64nize_with_len((void*)s_auth_work->prof_words, &b64_ptr, s_auth_work->prof_words_bytes));
            break;
    }

    NHTTPAddPostDataAscii(request, "sdkver",
        base64nize("001000", &b64_ptr));
    NHTTPAddPostDataAscii(request, "gamecd",
        base64nize(OSGetAppGamename(), &b64_ptr));

    nand_ret = NANDGetHomeDir(path);
    if (nand_ret == NAND_RESULT_OK)
    {
        nand_ret = NANDGetStatus(path, &nand_status);
        if (nand_ret == NAND_RESULT_OK)
        {
            if (nand_status.groupId == 2)
            {
                strncpy(strbuf, "02", 3);
            }
            else
            {
                sprintf(strbuf, "%c%c",
                    nand_status.groupId >> 8, nand_status.groupId & 0xff);
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH,
                " NANDGetStatus failed.[%d]\n", nand_ret);
            strncpy(strbuf, "00", 3);
        }
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH,
            " NANDGetHomeDir failed.[%d]\n", nand_ret);
        strncpy(strbuf, "00", 3);
    }
    NHTTPAddPostDataAscii(request, "makercd",
        base64nize(strbuf, &b64_ptr));
    DWC_Printf(DWC_REPORTFLAG_AUTH, " makercd = %s\n", strbuf);

    NHTTPAddPostDataAscii(request, "unitcd", base64nize("1", &b64_ptr));

    DWCi_Np_GetMacAddress(macaddr);
    sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", macaddr[0], macaddr[1],
        macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    NHTTPAddPostDataAscii(request, "macadr", base64nize(strbuf, &b64_ptr));
    DWC_Printf(DWC_REPORTFLAG_AUTH, " macadr = %s\n", strbuf);

    sprintf(strbuf, "%02d", SCGetLanguage());
    DWC_Printf(DWC_REPORTFLAG_AUTH, " lang = %s\n", strbuf);
    NHTTPAddPostDataAscii(request, "lang", base64nize(strbuf, &b64_ptr));

    if (s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
    {
        OSTicksToCalendarTime(OSGetTime(), &td);
        sprintf(strbuf, "%02d%02d%02d%02d%02d%02d", td.year % 100,
            td.mon + 1, td.mday, td.hour, td.min, td.sec);
        NHTTPAddPostDataAscii(request, "devtime", base64nize(strbuf, &b64_ptr));
        DWC_Printf(DWC_REPORTFLAG_AUTH, " CalendarTime = %s\n", strbuf);
    }

    if (s_auth_work->ifconfig.selectedMedia == 1 && s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
    {
        sprintf(strbuf, "%02d",
            s_auth_work->ifconfig.netif.wireless.configMethod);
        DWC_Printf(DWC_REPORTFLAG_AUTH, " confmethod = %s\n", strbuf);
        NHTTPAddPostDataAscii(request, "confmethod",
            base64nize(strbuf, &b64_ptr));
    }

    product_code = SCGetProductCode();
    if (product_code != NULL && SCGetProductSN(&ser_no))
    {
        sprintf(strbuf, "%s%09d", product_code, ser_no);
        DWC_Printf(DWC_REPORTFLAG_AUTH, " csnum = %s\n", strbuf);
        NHTTPAddPostDataAscii(request, "csnum",
            base64nize(strbuf, &b64_ptr));
    }

    sprintf(strbuf, "%016lld", DWCi_Np_GetConsoleId());
    DWC_Printf(DWC_REPORTFLAG_AUTH, " cfc = %s\n", strbuf);
    NHTTPAddPostDataAscii(request, "cfc", base64nize(strbuf, &b64_ptr));

    if (s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
    {
        sprintf(strbuf, "%02d", SCGetProductArea());
        DWC_Printf(DWC_REPORTFLAG_AUTH, " region = %s\n", strbuf);
        NHTTPAddPostDataAscii(request, "region", base64nize(strbuf, &b64_ptr));
    }

    s_auth_work->request_id = NHTTPSendRequestAsync(request);
    s_auth_result.errorcode = 0;
}

static void DWCi_Auth_HandleResponse(
    NHTTPErr error, NHTTPResponse* res, void* user_param)
{
    int return_code;

    if (s_auth_result.res != NULL)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, "NHTTPDestroyResponse()\n");
        NHTTPDestroyResponse(s_auth_result.res);
    }
    s_auth_result.res = res;

    DWC_Printf(DWC_REPORTFLAG_AUTH, " request_callback = %d\n", error);

    if (error == NHTTP_ERROR_CANCELED)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH, " nhttp canceled(%d)\n", error);
        s_auth_result.errorcode = 2;
        return;
    }
    else if (error != NHTTP_ERROR_NONE)
    {
        if (error == NHTTP_ERROR_SSL)
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH, " ssl error(%d)\n", NHTTPSSLGetError());
        }
        DWC_Printf(DWC_REPORTFLAG_AUTH, " nhttp error(%d)\n", error);
        s_auth_result.errorcode = -20100;
        s_prof_result.connerr = TRUE;
        return;
    }

    return_code = NHTTPGetResultCode(res);
    if (return_code != 200)
    {
        char* body;

        DWC_Printf(DWC_REPORTFLAG_AUTH,
            " status code is not 200, but %d\n",
            return_code);
        s_auth_result.errorcode = -23000 - return_code;

        // the error page is printed
        if (NHTTPGetBodyAll(res, (void**)&body) > 0)
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH, "%s\n", body);
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH, "no body\n", body);
        }
        return;
    }

    {
        char* s;
        char* token;
        int rc;
        char base64decode[256];
        const char* delim;
        int has_returncd;
        char returncd[4];

        delim = "&\r\n";
        has_returncd = FALSE;

        if (NHTTPGetBodyAll(res, (void**)&s) > 0)
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH, " %s\n", s);
            token = strtok(s, delim);

            while (token != NULL)
            {
                if (strncmp(token, "retry=", strlen("retry=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("retry="),
                        strlen(token) - strlen("retry="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) retry=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "returncd=", strlen("returncd=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("returncd="),
                        strlen(token) - strlen("returncd="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(returncd, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) returncd=%s\n", rc, base64decode);
                    has_returncd = TRUE;
                }
                else if (strncmp(token, "datetime=", strlen("datetime=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("datetime="),
                        strlen(token) - strlen("datetime="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    s_auth_result.server_timediff = DWCi_Auth_DateTimeToOSTime(base64decode) - OSGetTime();
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) datetime=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "locator=", strlen("locator=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("locator="),
                        strlen(token) - strlen("locator="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_auth_result.locator, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) locator=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "token=", strlen("token=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("token="),
                        strlen(token) - strlen("token="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_auth_result.token, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) token=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "challenge=", strlen("challenge=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("challenge="),
                        strlen(token) - strlen("challenge="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_auth_result.challenge, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) challenge=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "userid=", strlen("userid=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("userid="),
                        strlen(token) - strlen("userid="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    sscanf(base64decode, "%llu", &s_auth_result.userid);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) userid=%llu\n", rc, s_auth_result.userid);
                    // the new account's id, saved to NAND by the FILE_WRITE state
                    s_auth_console_save_data.userid = s_auth_result.userid;
                }
                else if (strncmp(token, "svchost=", strlen("svchost=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("svchost="),
                        strlen(token) - strlen("svchost="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_svl_result.svlhost, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) svlhost=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "servicetoken=", strlen("servicetoken="))
                         == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("servicetoken="),
                        strlen(token) - strlen("servicetoken="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_svl_result.svltoken, base64decode);
                    DWC_Printf(DWC_REPORTFLAG_AUTH,
                        " (%d) servicetoken=%s\n",
                        rc,
                        base64decode);
                }
                else if (strncmp(token, "statusdata=", strlen("statusdata=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("statusdata="),
                        strlen(token) - strlen("statusdata="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    if (base64decode[0] == 'Y')
                    {
                        s_svl_result.status = TRUE;
                    }
                    else
                    {
                        s_svl_result.status = FALSE;
                    }
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " (%d) statusdata=%s\n", rc, base64decode);
                }
                else if (strncmp(token, "prwords=", strlen("prwords=")) == 0)
                {
                    rc = DWC_Base64Decode(token + strlen("prwords="),
                        strlen(token) - strlen("prwords="),
                        base64decode,
                        sizeof(base64decode));
                    base64decode[rc] = '\0';
                    strcpy(s_prof_result.prwords, base64decode);
                }
                else
                {
                    DWC_Printf(DWC_REPORTFLAG_AUTH, " unknown token : %s\n", token);
                }

                token = strtok(NULL, delim);
            }
        }

        if (has_returncd)
        {
            int return_code;

            return_code = strtol(returncd, NULL, 10);
            if (return_code == 0 && s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH,
                    " cannot parse returncd(%s)\n",
                    returncd);
                s_auth_result.errorcode = -20101;
                return;
            }
            else if (return_code == 1 && s_auth_work->auth_kind == DWCi_AUTH_KIND_PROFCHECK)
            {
                DWC_Printf(DWC_REPORTFLAG_AUTH,
                    " prof server maintenance(%s)\n",
                    returncd);
                s_auth_result.errorcode = -33001;
                return;
            }
            else if (return_code >= 100)
            {
                if (s_auth_work->auth_kind == DWCi_AUTH_KIND_PROFCHECK && return_code == 100)
                {
                    DWC_Printf(DWC_REPORTFLAG_AUTH,
                        " prof server retruns error (%d) but ignored by DWC library\n",
                        return_code);
                }
                else
                {
                    DWC_Printf(DWC_REPORTFLAG_AUTH,
                        " server retruns error (%d)\n",
                        return_code);
                    s_auth_result.errorcode = -20000 - return_code;
                    return;
                }
            }

            if (return_code == 40)
            {
                s_auth_result.ingamesn_check_result = DWC_INGAMESN_INVALID;
            }
            else
            {
                s_auth_result.ingamesn_check_result = DWC_INGAMESN_VALID;
            }
        }
        else
        {
            DWC_Printf(DWC_REPORTFLAG_AUTH, " no return code.\n");
            s_auth_result.errorcode = -20101;
            return;
        }

        s_auth_result.errorcode = 1;
    }
}

static void DWCi_Auth_EndProcess(void)
{
    DWC_Printf(DWC_REPORTFLAG_AUTH, "DWCi_Auth_EndProcess()\n");
    s_auth_work->auth_free(DWC_ALLOCTYPE_AUTH, s_auth_work, 0);
    s_auth_work = NULL;
    s_auth_state = DWCi_AUTH_STATE_FINISHED;
}

static void DWCi_Auth_HandleNandResult(s32 ok_state, s32 retry_state,
    s32 noexists_state, s32 eaccess_state)
{
    s_auth_work->is_nand_end = FALSE;

    if (retry_state != DWCi_AUTH_STATE_NONE && s_auth_work->nand_result == NAND_RESULT_BUSY && s_auth_work->retry_num < 5)
    {
        s_auth_work->retry_num++;
        s_auth_state = retry_state;
        return;
    }

    s_auth_work->retry_num = 0;

    if (s_auth_work->nand_result == NAND_RESULT_OK)
    {
        s_auth_state = ok_state;
        return;
    }

    if (noexists_state != DWCi_AUTH_STATE_NONE && s_auth_work->nand_result == NAND_RESULT_NOEXISTS)
    {
        s_auth_state = noexists_state;
        return;
    }

    if (eaccess_state != DWCi_AUTH_STATE_NONE && s_auth_work->nand_result == NAND_RESULT_ACCESS)
    {
        s_auth_state = eaccess_state;
        return;
    }

    DWC_Printf(DWC_REPORTFLAG_AUTH, " NAND access failed.[%d]\n", s_auth_work->nand_result);
    if (s_auth_work->nand_result == NAND_RESULT_CORRUPT)
    {
        s_auth_result.errorcode = -0x7149;
    }
    else
    {
        s_auth_result.errorcode = -0x7148;
    }
    DWCi_Auth_EndProcess();
}

static char* DWCi_Auth_GetHost(void)
{
    static char strbuf[255];
    char* found_ptr;

    if (s_auth_work->auth_kind != DWCi_AUTH_KIND_PROFCHECK)
    {
        found_ptr = strstr(s_authserver_hosts[s_authserver], "//");
    }
    else
    {
        found_ptr = strstr(s_profserver_hosts[s_authserver], "//");
    }
    strcpy(strbuf, found_ptr + 2);
    found_ptr = strstr(strbuf, "/");
    *found_ptr = '\0';
    return strbuf;
}

static OSTime DWCi_Auth_DateTimeToOSTime(const char* datetime)
{
    OSCalendarTime server_time;
    int convert_num;

    convert_num = sscanf(datetime, "%04d%02d%02d%02d%02d%02d",
        &server_time.year, &server_time.mon, &server_time.mday,
        &server_time.hour, &server_time.min, &server_time.sec);
    if (convert_num != 6)
    {
        DWC_Printf(DWC_REPORTFLAG_AUTH,
            " cannot parse datetime: %s\n", datetime);
        return 0;
    }

    server_time.mon--;
    server_time.wday = 0;
    server_time.yday = 0;
    server_time.msec = 0;
    server_time.usec = 0;
    return OSCalendarTimeToTicks(&server_time);
}
