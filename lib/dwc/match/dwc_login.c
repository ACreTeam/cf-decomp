#include <dwc/dwc_main.h>
#include <dwc/dwc_account.h>
#include <dwc/dwc_auth_interface.h>
#include <dwc/dwc_error.h>
#include <dwc/dwc_login.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <nitro/math/rand.h>
#include <revolution/types.h>
#include <string.h>

static DWCLoginControl* stpLoginCnt;

static void DWCi_CloseLogin(void);
static GPResult DWCi_HandleGPError(GPResult result);
static void DWCi_GPConnectCallback(
    GPConnection* pconnection, GPConnectResponseArg* arg);
static void DWCi_GPConnect(GPCallback callback, DWCLoginState next_state);
static BOOL DWCi_RemoteLogin(void);
static void DWCi_RemoteLoginProcess(void);
static void DWCi_GPGetInfoCallback(
    GPConnection* pconnection, GPGetInfoResponseArg* arg);

GPResult DWCi_SetGPStatus(
    int status, const char* statusString, const char* locationString);

void DWCi_LoginInit(DWCLoginControl* logcnt, DWCUserData* userdata,
    GPConnection* pGpObj, int productID, u32 gamecode, const u16* playerName,
    DWCLoginCallback callback, void* param)
{
    DWC_Printf(0x20, "Login Init\n");

    stpLoginCnt = logcnt;
    memset(stpLoginCnt, 0, sizeof(DWCLoginControl));
    stpLoginCnt->pGpObj = pGpObj;
    stpLoginCnt->state = DWC_LOGIN_STATE_INIT;
    stpLoginCnt->productID = productID;
    stpLoginCnt->gamecode = gamecode;
    stpLoginCnt->playerName = playerName;
    stpLoginCnt->callback = callback;
    stpLoginCnt->param = param;
    stpLoginCnt->userdata = userdata;

    DWC_Printf(0x20, "******************************************\n");
    DWC_Printf(0x20, "  pseudo    UserID   : %016llx\n",
        DWCi_Acc_GetUserId(&userdata->pseudo));
    DWC_Printf(0x20, "  pseudo    PlayerID : %08x\n",
        DWCi_Acc_GetPlayerId(&userdata->pseudo));
    DWC_Printf(0x20, "  authentic UserID   : %016llx\n",
        DWCi_Acc_GetUserId(&userdata->authentic));
    DWC_Printf(0x20, "  authentic PlayerID : %08x\n",
        DWCi_Acc_GetPlayerId(&userdata->authentic));
    DWC_Printf(0x20, "******************************************\n");
}

BOOL DWCi_LoginAsync(void)
{
    if (DWCi_RemoteLogin())
    {
        stpLoginCnt->state = DWC_LOGIN_STATE_REMOTE_AUTH;
        stpLoginCnt->connectFlag = 0;
        return TRUE;
    }
    return FALSE;
}

void DWCi_LoginProcess(void)
{
    if (!stpLoginCnt || DWCi_IsError())
    {
        return;
    }

    switch (stpLoginCnt->state)
    {
    case DWC_LOGIN_STATE_REMOTE_AUTH:
        DWCi_RemoteLoginProcess();
        break;
    case DWC_LOGIN_STATE_CONNECTING:
    case DWC_LOGIN_STATE_GPGETINFO:
    case DWC_LOGIN_STATE_GPSETINFO:
        if (stpLoginCnt->pGpObj && *stpLoginCnt->pGpObj)
        {
            gpProcess(stpLoginCnt->pGpObj);
        }
        if (stpLoginCnt->connectFlag
            && DWCi_Np_TicksToMilliSeconds(
                   DWCi_Np_GetTick() - stpLoginCnt->connectTick)
                > DWC_LOGIN_GPCONNECT_TIMEOUT)
        {
            DWCi_StopLogin(DWC_ERROR_NETWORK,
                DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_GP
                    + DWC_ECODE_TYPE_TIMEOUT);
            stpLoginCnt->connectFlag = FALSE;
        }
        break;
    case DWC_LOGIN_STATE_INIT:
    case DWC_LOGIN_STATE_CONNECTED:
    default:
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Ignore invalid login state.\n");
        break;
    }
}

DWCAccUserData* DWCi_GetUserData(void)
{
    if (stpLoginCnt != NULL)
    {
        return stpLoginCnt->userdata;
    }
    return NULL;
}

void DWCi_StopLogin(DWCError error, int errorCode)
{
    if (!stpLoginCnt || error == DWC_ERROR_NONE)
    {
        return;
    }

    DWCi_SetError(error, errorCode);
    if (stpLoginCnt->callback != NULL)
    {
        stpLoginCnt->callback(error, 0, stpLoginCnt->param);
    }
    DWCi_CloseLogin();
}

void DWCi_ShutdownLogin(void)
{
    if (!DWCi_Auth_IsFinished())
    {
        DWCi_Auth_AbortAuthentication();
    }
    stpLoginCnt = NULL;
}

static void DWCi_CloseLogin(void)
{
    if (!stpLoginCnt)
    {
        return;
    }

    stpLoginCnt->state = DWC_LOGIN_STATE_INIT;
    stpLoginCnt->connectFlag = FALSE;
}

static GPResult DWCi_HandleGPError(GPResult result)
{
    int errorCode;
    DWCError dwcError;

    if (result == GP_NO_ERROR)
    {
        return GP_NO_ERROR;
    }

    DWC_Printf(2, "Login, GP error %d\n", result);
    switch (result)
    {
    case 1:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_ALLOC;
        break;
    case 2:
        dwcError = DWC_ERROR_FATAL;
        errorCode = DWC_ECODE_TYPE_PARAM;
        break;
    case 3:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_NETWORK;
        break;
    case 4:
        dwcError = DWC_ERROR_NETWORK;
        errorCode = DWC_ECODE_TYPE_SERVER;
        break;
    }

    errorCode += DWC_ECODE_SEQ_LOGIN + DWC_ECODE_GS_GP;
    DWCi_StopLogin(dwcError, errorCode);
    return result;
}

static void DWCi_GPConnectCallback(
    GPConnection* pconnection, GPConnectResponseArg* arg)
{
    GPResult gpResult;

    DWC_Printf(DWC_REPORTFLAG_LOGIN,
        "Finished connecting to GP server, result = %d\n", arg->result);
    stpLoginCnt->connectFlag = FALSE;

    if (arg->result == GP_NO_ERROR)
    {
        if (stpLoginCnt->state == DWC_LOGIN_STATE_CONNECTING)
        {
            if (stpLoginCnt->userdata->gs_profile_id == arg->profile)
            {
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "  gs profile id is valid.\n");
                stpLoginCnt->state = DWC_LOGIN_STATE_CONNECTED;

                gpResult = DWCi_SetGPStatus(DWC_STATUS_ONLINE, "", NULL);
                if (DWCi_HandleGPError(gpResult))
                {
                    return;
                }

                stpLoginCnt->callback(
                    DWC_ERROR_NONE, arg->profile, stpLoginCnt->param);

                if (!DWCi_GT2Startup())
                {
                    if (DWCi_QR2Startup(arg->profile))
                    {
                        return;
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "  gs profile id is invalid.\n");
                DWCi_StopLogin(DWC_ERROR_NETWORK, DWC_ECODE_SEQ_LOGIN);
            }
        }
        else if (stpLoginCnt->state == DWC_LOGIN_STATE_GPGETINFO)
        {
            gpResult = gpGetInfo(pconnection, arg->profile,
                GP_DONT_CHECK_CACHE, GP_NON_BLOCKING,
                (GPCallback)DWCi_GPGetInfoCallback, NULL);
            if (DWCi_HandleGPError(gpResult))
            {
                return;
            }
        }
    }
    else
    {
        (void)DWCi_HandleGPError(arg->result);
    }
}

static void DWCi_GPConnect(GPCallback callback, DWCLoginState next_state)
{
    GPResult gpResult;

    DWC_Printf(
        DWC_REPORTFLAG_LOGIN, "Succeeded to remote authentication.\n");

    stpLoginCnt->connectTick = DWCi_Np_GetTick();
    stpLoginCnt->connectFlag = TRUE;

    gpResult = gpConnectPreAuthenticated(stpLoginCnt->pGpObj,
        stpLoginCnt->authToken, stpLoginCnt->partnerChallenge, GP_FIREWALL,
        GP_NON_BLOCKING, callback, NULL);
    if (DWCi_HandleGPError(gpResult))
    {
        return;
    }

    stpLoginCnt->state = next_state;
}

static BOOL DWCi_RemoteLogin(void)
{
    u64 userid;

    DWC_Printf(DWC_REPORTFLAG_LOGIN, "Start Remote Auth\n");

    if (DWCi_Acc_IsAuthentic(stpLoginCnt->userdata))
    {
        DWC_Printf(DWC_REPORTFLAG_LOGIN,
            "  Hmm.. you already have authentic login id.\n");
        DWCi_Acc_LoginIdToUserName(&stpLoginCnt->userdata->authentic,
            stpLoginCnt->userdata->gamecode, stpLoginCnt->username);
        userid = DWCi_Acc_GetUserId(&stpLoginCnt->userdata->authentic);
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_LOGIN,
            "  Hmm.. you need to create authentic login id.\n");

        if (!DWCi_Acc_IsValidLoginId(&stpLoginCnt->tempLoginId))
        {
            DWC_Printf(DWC_REPORTFLAG_LOGIN,
                "    Hmm.. you are the first time to get authentic login id.");
            if (DWCi_Acc_CheckConsoleUserId(&stpLoginCnt->userdata->pseudo))
            {
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "- copy temp loginid from pseudo login id\n");
                stpLoginCnt->tempLoginId = stpLoginCnt->userdata->pseudo;
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "- create temp loginid from console user id\n");
                DWCi_Acc_CreateTempLoginId(&stpLoginCnt->tempLoginId);
            }
        }
        else
        {
            MATHRandContext32 randcontext;

            DWC_Printf(DWC_REPORTFLAG_LOGIN,
                "    Hmm.. you are NOT the first times to get authentic login id.\n");
            MATH_InitRand32(&randcontext, DWCi_Np_GetTick());
            DWCi_Acc_SetPlayerId(&stpLoginCnt->tempLoginId,
                MATH_Rand32(&randcontext, 0));
        }

        DWCi_Acc_LoginIdToUserName(&stpLoginCnt->tempLoginId,
            stpLoginCnt->gamecode, stpLoginCnt->username);
        userid = 0;
    }

    return DWCi_Auth_StartAuthentication(stpLoginCnt->playerName,
        &stpLoginCnt->username[DWC_ACC_USERNAME_GSBRCD_OFFSET], userid,
        DWC_Alloc, DWC_Free);
}

static void DWCi_RemoteLoginProcess(void)
{
    s32 errorcode;

    DWCi_Auth_ProcessAuthentication();
    if (DWCi_Auth_IsFinished())
    {
        if (DWCi_Auth_IsSucceeded())
        {
            DWC_Printf(DWC_REPORTFLAG_LOGIN, " *** Auth Done\n");
            DWCi_Auth_GetGameSpyToken(
                stpLoginCnt->authToken, stpLoginCnt->partnerChallenge);

            if (DWCi_Acc_IsAuthentic(stpLoginCnt->userdata))
            {
                DWCi_GPConnect((GPCallback)DWCi_GPConnectCallback,
                    DWC_LOGIN_STATE_CONNECTING);
            }
            else
            {
                DWCi_Acc_SetUserId(&stpLoginCnt->tempLoginId,
                    DWCi_Auth_GetConsoleUserId());
                DWCi_GPConnect((GPCallback)DWCi_GPConnectCallback,
                    DWC_LOGIN_STATE_GPGETINFO);
            }
        }
        else
        {
            errorcode = DWCi_Auth_GetErrorCode();
            DWC_Printf(DWC_REPORTFLAG_LOGIN,
                " *** Auth Error [%d]\n", errorcode);
            if (errorcode <= -29000)
            {
                DWCi_StopLogin(DWC_ERROR_FATAL, errorcode);
            }
            else
            {
                DWCi_StopLogin(DWC_ERROR_AUTH_ANY, errorcode);
            }
        }
    }
}

static void DWCi_GPGetInfoCallback(
    GPConnection* pconnection, GPGetInfoResponseArg* arg)
{
    if (arg->result == GP_NO_ERROR)
    {
        if (stpLoginCnt->state == DWC_LOGIN_STATE_GPGETINFO)
        {
            if (arg->lastname[0] == 0)
            {
                GPResult gpResult;
                char pseudo_name[DWC_ACC_USERNAME_STRING_BUFSIZE];

                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "    login id is authenticated. set lastname field.\n");
                DWCi_Acc_LoginIdToUserName(&stpLoginCnt->userdata->pseudo,
                    stpLoginCnt->gamecode, pseudo_name);
                gpResult = gpSetInfos(pconnection, GP_LASTNAME, pseudo_name);
                if (DWCi_HandleGPError(gpResult))
                {
                    return;
                }

                stpLoginCnt->state = DWC_LOGIN_STATE_GPSETINFO;
                gpResult = gpGetInfo(pconnection, arg->profile,
                    GP_DONT_CHECK_CACHE, GP_NON_BLOCKING,
                    (GPCallback)DWCi_GPGetInfoCallback, NULL);
                if (DWCi_HandleGPError(gpResult))
                {
                    return;
                }

                DWC_Printf(
                    DWC_REPORTFLAG_LOGIN, "    call gpSetInfos\n");
            }
            else
            {
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "    this login id is used by anybody.... retry.\n");
                gpDisconnect(pconnection);
                DWCi_RemoteLogin();
                stpLoginCnt->state = DWC_LOGIN_STATE_REMOTE_AUTH;
            }
        }
        else if (stpLoginCnt->state == DWC_LOGIN_STATE_GPSETINFO)
        {
            char pseudo_name[DWC_ACC_USERNAME_STRING_BUFSIZE];

            DWCi_Acc_LoginIdToUserName(&stpLoginCnt->userdata->pseudo,
                stpLoginCnt->gamecode, pseudo_name);
            if (strcmp(arg->lastname, pseudo_name) == 0)
            {
                char username[DWC_ACC_USERNAME_STRING_BUFSIZE];

                DWCi_Acc_LoginIdToUserName(&stpLoginCnt->tempLoginId,
                    stpLoginCnt->gamecode, username);
                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "    Account is created : %s(%s) - %d.\n", username,
                    pseudo_name, arg->profile);

                DWCi_Acc_SetLoginIdToUserData(stpLoginCnt->userdata,
                    &stpLoginCnt->tempLoginId, arg->profile);
                gpDisconnect(pconnection);
                DWCi_GPConnect((GPCallback)DWCi_GPConnectCallback,
                    DWC_LOGIN_STATE_CONNECTING);
            }
            else
            {
                GPResult gpResult;

                DWC_Printf(DWC_REPORTFLAG_LOGIN,
                    "    Login but gpSetInfo failed... %s : %d retry gpGetInfo.\n",
                    arg->lastname, arg->profile);
                gpResult = gpGetInfo(pconnection, arg->profile,
                    GP_DONT_CHECK_CACHE, GP_NON_BLOCKING,
                    (GPCallback)DWCi_GPGetInfoCallback, NULL);
                if (DWCi_HandleGPError(gpResult))
                {
                    return;
                }
            }
        }
    }
    else
    {
        DWC_Printf(DWC_REPORTFLAG_LOGIN,
            " ERROR: gpGetInfo. why??? : %d\n", arg->result);
    }
}

BOOL DWCi_CheckLogin(void)
{
    if (stpLoginCnt != NULL && stpLoginCnt->state == DWC_LOGIN_STATE_CONNECTED)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL DWCi_GetAuthInfo(char** authToken, char** partnerChallenge)
{
    if (!DWCi_CheckLogin())
    {
        return FALSE;
    }

    *authToken = stpLoginCnt->authToken;
    *partnerChallenge = stpLoginCnt->partnerChallenge;
    return TRUE;
}

BOOL DWCi_GetLoginTicket(char* loginTicket)
{
    if (!DWCi_CheckLogin())
    {
        return FALSE;
    }

    return gpGetLoginTicket(stpLoginCnt->pGpObj, loginTicket) == GP_NO_ERROR;
}
