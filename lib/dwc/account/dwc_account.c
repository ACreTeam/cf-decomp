#include <dwc/dwc_account.h>
#include <dwc/dwc_auth_interface.h>
#include <dwc/dwc_init.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <nitro/math/crc.h>
#include <nitro/math/rand.h>
#include <revolution/os/OSTime.h>
#include <stdio.h>
#include <string.h>

#define DWCi_CHECK_FLAG(flag, mask) (((flag) & (mask)) == (mask))

static inline u32
DWCi_Acc_GetMaskBits(u32 data,
    u32 shift,
    u32 mask)
{
    return ((data >> shift) & mask);
}

static BOOL
DWCi_Acc_SetMaskBits(u32* ptr,
    u32 data,
    u32 shift,
    u32 mask)
{
    // Checks whether or not the data is included in the mask.
    if ((data & ~mask) != 0)
    {
        return FALSE;
    }

    *ptr = (*ptr & ~(mask << shift)) | (data << shift);

    return TRUE;
}

u64 DWCi_Acc_GetUserId(const DWCAccLoginId* userdata)
{
    u32 hi32;

    hi32 = DWCi_Acc_GetMaskBits(userdata->id_data, DWC_ACC_USERID_HI32_SHIFT, DWC_ACC_USERID_HI32_MASK);

    return (u64)hi32 << 32 | userdata->userid_lo32;
}

u32 DWCi_Acc_GetPlayerId(const DWCAccLoginId* userdata)
{
    return userdata->playerid;
}

u64 DWCi_Acc_GetFriendKey(const DWCAccFriendKey* data)
{
    return (u64)data->friendkey_hi32 << 32 | data->friendkey_lo32;
}

int DWCi_Acc_GetGsProfileId(const DWCAccGsProfileId* data)
{
    return data->id;
}

void DWCi_Acc_SetUserId(DWCAccLoginId* userdata,
    u64 userid)
{
    if (!DWCi_Acc_SetMaskBits(&userdata->id_data, (u32)(userid >> 32), DWC_ACC_USERID_HI32_SHIFT, DWC_ACC_USERID_HI32_MASK))
    {
        // userid value out of bounds
    }
    userdata->userid_lo32 = (u32)userid;
}

void DWCi_Acc_SetPlayerId(DWCAccLoginId* userdata,
    u32 playerid)
{
    userdata->playerid = playerid;
}

void DWCi_Acc_SetFriendKey(DWCAccFriendKey* data,
    u64 friendkey)
{
    data->friendkey_lo32 = (u32)friendkey;
    data->friendkey_hi32 = (u32)(friendkey >> 32);
}

void DWCi_Acc_SetGsProfileId(DWCAccGsProfileId* data, int gs_profile_id)
{
    data->id = gs_profile_id;
}

static u32
DWCi_Acc_GetFlags(const DWCAccFlag* userdata)
{
    return DWCi_Acc_GetMaskBits(userdata->flags, DWC_ACC_FLAGS_SHIFT, DWC_ACC_FLAGS_MASK);
}

// Data type
static u32
DWCi_Acc_GetFlag_DataType(const DWCAccFlag* userdata)
{
    return DWCi_Acc_GetFlags(userdata) & DWC_ACC_FRIENDDATA_MASK;
}

BOOL DWC_IsBuddyFriendData(const DWCAccFriendData* frienddata)
{
    if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_GS_PROFILE_ID)
    {
        u32 type;

        type = DWCi_Acc_GetFlags(&frienddata->flags);

        return (DWCi_CHECK_FLAG(type, DWC_ACC_FRIENDDATA_ISBUDDY)) ? TRUE : FALSE;
    }

    return FALSE;
}

int DWC_GetFriendDataType(const DWCAccFriendData* frienddata)
{
    return (int)DWCi_Acc_GetFlag_DataType(&frienddata->flags);
}

static void
DWCi_Acc_SetFlags(DWCAccFlag* userdata,
    u32 flags)
{
    DWCi_Acc_SetMaskBits(&userdata->flags, flags, DWC_ACC_FLAGS_SHIFT, DWC_ACC_FLAGS_MASK);
}

// Data type
static void
DWCi_Acc_SetFlag_DataType(DWCAccFlag* userdata,
    int type)
{
    u32 flags;

    flags = DWCi_Acc_GetFlags(userdata);

    flags = (flags & ~DWC_ACC_FRIENDDATA_MASK) | type;

    DWCi_Acc_SetFlags(userdata, flags);
}

void DWCi_SetBuddyFriendData(DWCAccFriendData* frienddata)
{
    if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_GS_PROFILE_ID)
    {
        u32 flags;

        flags = DWCi_Acc_GetFlags(&frienddata->flags);

        flags = (flags & ~DWC_ACC_FRIENDDATA_ISBUDDY_MASK) | DWC_ACC_FRIENDDATA_ISBUDDY;

        DWCi_Acc_SetFlags(&frienddata->flags, flags);
    }
}

void DWC_ClearBuddyFlagFriendData(DWCAccFriendData* frienddata)
{
    if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_GS_PROFILE_ID)
    {
        u32 flags;

        flags = DWCi_Acc_GetFlags(&frienddata->flags);

        flags &= ~DWC_ACC_FRIENDDATA_ISBUDDY_MASK;

        DWCi_Acc_SetFlags(&frienddata->flags, flags);
    }
}

u64 DWC_Acc_CreateFriendKey(int gs_profile_id,
    u32 gamecode)
{
    MATHCRC8Table table;
    u32 code[2];
    int crc;
    u32 reversed[2];

    code[0] = (u32)gs_profile_id;
    code[1] = gamecode;
    MATH_CRC8InitTable(&table);
    DWCi_Np_ToLE(code, reversed, sizeof(code));
    crc = MATH_CalcCRC8(&table, reversed, sizeof(reversed)) & 0x7f; // Only 7 bits valid.

    return (((u64)crc << 32) | (u32)gs_profile_id);
}

BOOL DWC_CheckFriendKey(const DWCAccUserData* userdata, u64 friendkey)
{
    return DWC_Acc_CheckFriendKey(friendkey, userdata->gamecode);
}

BOOL DWC_Acc_CheckFriendKey(u64 friendkey,
    u32 gamecode)
{
    MATHCRC8Table table;
    u32 code[2];
    int crc;
    u32 reversed[2];

    // negative GS profile IDs are invalid
    if (friendkey & 0x80000000)
        return FALSE;

    code[0] = (u32)friendkey;
    code[1] = gamecode;
    MATH_CRC8InitTable(&table);
    DWCi_Np_ToLE(code, reversed, sizeof(code));
    crc = MATH_CalcCRC8(&table, reversed, sizeof(reversed)) & 0x7f; // Only 7 bits valid.

    if (crc != (friendkey >> 32))
    {
        return FALSE;
    }

    return TRUE;
}

int DWC_Acc_FriendKeyToGsProfileId(u64 friendkey,
    u32 gamecode)
{
    if (!DWC_Acc_CheckFriendKey(friendkey, gamecode))
    {
        return 0;
    }

    return (int)(friendkey & 0xffffffff);
}

void DWC_Acc_FriendKeyToNumericKey(u64 friendkey,
    char* numerickey)
{
    snprintf(numerickey, DWC_ACC_FRIENDKEY_STRING_BUFSIZE, "%012llu", friendkey);
}

u64 DWC_Acc_NumericKeyToFriendKey(const char* numerickey)
{
    int i;
    u64 result = 0;
    u64 dec = 1;

    for (i = 0; i < DWC_ACC_FRIENDKEY_STRING_LENGTH; ++i)
    {
        result += (*(numerickey + DWC_ACC_FRIENDKEY_STRING_LENGTH - 1 - i) - '0') * dec;

        dec *= 10;
    }

    return result;
}

static void
DWCi_Acc_U64ToString32(u64 num,
    int bitnum,
    char* result)
{
    int i;
    const int loopnum = (bitnum + 4) / 5;
    const char* const sStringTable = "0123456789abcdefghijklmnopqrstuv";
    //                                          0123456789012345678901

    for (i = 0; i < loopnum; ++i)
    {
        *(result + loopnum - 1 - i) = *(sStringTable + (num & 0x1f));

        num >>= 5;
    }

    *(result + loopnum) = 0; // NULL termination inserted.
}

void DWCi_Acc_LoginIdToUserName(const DWCAccLoginId* loginid,
    u32 gamecode,
    char* username)
{
    char userid[DWC_ACC_USERNAME_STRING_BUFSIZE];
    char playerid[DWC_ACC_USERNAME_STRING_BUFSIZE];

    DWCi_Acc_U64ToString32(DWCi_Acc_GetUserId(loginid), DWC_ACC_USERID_BITS, userid);
    DWCi_Acc_U64ToString32(DWCi_Acc_GetPlayerId(loginid), DWC_ACC_PLAYERID_BITS, playerid);

    snprintf(username, DWC_ACC_USERNAME_STRING_BUFSIZE, "%s%c%c%c%c%s", userid, (u8)(gamecode >> 24), (u8)(gamecode >> 16), (u8)(gamecode >> 8), (u8)gamecode, playerid);
}

void DWCi_Acc_CreateUserData(DWCAccUserData* userdata, u32 gamecode)
{
    MATHCRC32Table table;
    DWCAccUserData reversed;

    memset(userdata, 0, DWC_ACC_USERDATA_BUFSIZE);

    // Sets user data
    userdata->size = DWC_ACC_USERDATA_BUFSIZE;
    userdata->gs_profile_id = 0;
    userdata->gamecode = gamecode;

    // Generates a temporary LoginId.
    DWCi_Acc_CreateTempLoginId(&userdata->pseudo);

    // Since an authenticated ID cannot be generated at the time user data is created, nothing is stored here.
    DWCi_Acc_SetFlag_DataType((DWCAccFlag*)&userdata->authentic, DWC_ACC_FRIENDDATA_NODATA);

    // CRC calculation
    MATH_CRC32InitTable(&table);
    DWCi_Np_ToLE(userdata, &reversed, sizeof(DWCAccUserData));
    userdata->crc32 = MATH_CalcCRC32(&table, &reversed, sizeof(DWCAccUserData) - 4);

    // Enable the dirty flag.
    userdata->flag |= DWC_ACC_USERDATA_DIRTY;
}

void DWCi_Acc_CreateTempLoginId(DWCAccLoginId* loginid)
{
    MATHRandContext32 randcontext;

    MATH_InitRand32(&randcontext, (u32)OSGetTime());
    DWCi_Acc_SetUserId(loginid, DWCi_Auth_GetConsoleUserId());
    DWCi_Acc_SetPlayerId(loginid, MATH_Rand32(&randcontext, 0));
    DWCi_Acc_SetFlag_DataType((DWCAccFlag*)loginid, DWC_ACC_FRIENDDATA_LOGIN_ID);
}

BOOL DWCi_Acc_CheckConsoleUserId(const DWCAccLoginId* loginid)
{
    return (DWCi_Acc_GetUserId(loginid) == DWCi_Auth_GetConsoleUserId()) ? TRUE : FALSE;
}

BOOL DWCi_Acc_IsValidLoginId(const DWCAccLoginId* loginid)
{
    return (DWCi_Acc_GetFlag_DataType((DWCAccFlag*)loginid) == DWC_ACC_FRIENDDATA_LOGIN_ID) ? TRUE : FALSE;
}

BOOL DWCi_Acc_IsAuthentic(const DWCAccUserData* userdata)
{
    return DWCi_Acc_IsValidLoginId(&userdata->authentic);
}

BOOL DWC_IsValidFriendData(const DWCAccFriendData* frienddata)
{
    u32 type = DWCi_Acc_GetFlag_DataType(&frienddata->flags);

    if (type == DWC_ACC_FRIENDDATA_NODATA)
    {
        return FALSE;
    }

    return TRUE;
}

// For backward compatibility.
BOOL DWCi_Acc_IsValidFriendData(const DWCAccFriendData* frienddata)
{
    return DWC_IsValidFriendData(frienddata);
}

void DWC_CreateUserData(DWCAccUserData* userdata)
{
    DWCi_Acc_CreateUserData(userdata, DWCi_GetGamecode());
}

BOOL DWC_CheckUserData(const DWCAccUserData* userdata)
{
    u32 crc32;
    MATHCRC32Table table;
    DWCAccUserData reversed;

    MATH_CRC32InitTable(&table);
    DWCi_Np_ToLE(userdata, &reversed, sizeof(DWCAccUserData));
    crc32 = MATH_CalcCRC32(&table, &reversed, sizeof(DWCAccUserData) - 4);

    return (crc32 == userdata->crc32) ? TRUE : FALSE;
}

BOOL DWC_CheckHasProfile(const DWCAccUserData* userdata)
{
    return (DWCi_Acc_IsValidLoginId(&userdata->authentic) && userdata->gs_profile_id > 0) ? TRUE : FALSE;
}

BOOL DWC_CheckValidConsole(const DWCAccUserData* userdata)
{
    if (DWCi_Acc_GetFlag_DataType((DWCAccFlag*)&userdata->authentic) == DWC_ACC_FRIENDDATA_NODATA)
    {
        // TRUE if there is no authenticated Login ID
        return TRUE;
    }

    return (DWCi_Acc_GetUserId(&userdata->authentic) == DWCi_Auth_GetConsoleUserId()) ? TRUE : FALSE;
}

void DWCi_Acc_SetLoginIdToUserData(DWCAccUserData* userdata,
    const DWCAccLoginId* loginid,
    int gs_profile_id)
{
    MATHCRC32Table table;
    DWCAccUserData reversed;

    // Set the LoginID.
    userdata->authentic = *loginid;

    // Set the gs_profile_id
    userdata->gs_profile_id = gs_profile_id;

    // CRC calculation
    MATH_CRC32InitTable(&table);
    DWCi_Np_ToLE(userdata, &reversed, sizeof(DWCAccUserData));
    userdata->crc32 = MATH_CalcCRC32(&table, &reversed, sizeof(DWCAccUserData) - 4);

    // Enable the dirty flag.
    userdata->flag |= DWC_ACC_USERDATA_DIRTY;
}

BOOL DWC_CheckDirtyFlag(const DWCAccUserData* userdata)
{
    return DWCi_Acc_IsDirty(userdata);
}

BOOL DWCi_Acc_IsDirty(const DWCAccUserData* userdata)
{
    return DWCi_CHECK_FLAG(userdata->flag, DWC_ACC_USERDATA_DIRTY) ? TRUE : FALSE;
}

void DWC_ClearDirtyFlag(DWCAccUserData* userdata)
{
    DWCi_Acc_ClearDirty(userdata);
}

void DWCi_Acc_ClearDirty(DWCAccUserData* userdata)
{
    MATHCRC32Table table;
    DWCAccUserData reversed;

    userdata->flag &= ~DWC_ACC_USERDATA_DIRTY;

    // CRC calculation
    MATH_CRC32InitTable(&table);
    DWCi_Np_ToLE(userdata, &reversed, sizeof(DWCAccUserData));
    userdata->crc32 = MATH_CalcCRC32(&table, &reversed, sizeof(DWCAccUserData) - 4);
}

void DWCi_Acc_TestFlagFunc(void)
{
    DWCAccLoginId id;
    u32 flags;

    memset(&id, 0, sizeof(DWCAccLoginId));

    flags = DWCi_Acc_GetFlags((DWCAccFlag*)&id);
    DWC_Printf(-1, "%08x: %08x\n", flags, DWCi_Acc_GetFlag_DataType((DWCAccFlag*)&id));
    if (DWCi_Acc_IsValidLoginId(&id))
    {
        DWC_Printf(-1, "valid\n");
    }
    else
    {
        DWC_Printf(-1, "invalid\n");
    }

    DWCi_Acc_SetFlag_DataType((DWCAccFlag*)&id, DWC_ACC_FRIENDDATA_LOGIN_ID);

    flags = DWCi_Acc_GetFlags((DWCAccFlag*)&id);
    DWC_Printf(-1, "%08x: %08x\n", flags, DWCi_Acc_GetFlag_DataType((DWCAccFlag*)&id));
    if (DWCi_Acc_IsValidLoginId(&id))
    {
        DWC_Printf(-1, "valid\n");
    }
    else
    {
        DWC_Printf(-1, "invalid\n");
    }
}

u64 DWC_GetFriendKey(const DWCAccFriendData* frienddata)
{
    if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_FRIEND_KEY)
    {
        return DWCi_Acc_GetFriendKey(&frienddata->friend_key);
    }

    return 0;
}

int DWC_GetGsProfileId(const DWCAccUserData* userdata,
    const DWCAccFriendData* frienddata)
{
    u32 type = DWCi_Acc_GetFlag_DataType(&frienddata->flags);

    switch (type)
    {
    case DWC_ACC_FRIENDDATA_FRIEND_KEY:
    {
        u64 friend_key = DWCi_Acc_GetFriendKey(&frienddata->friend_key);

        if (DWC_Acc_CheckFriendKey(friend_key, userdata->gamecode))
        {
            return DWC_Acc_FriendKeyToGsProfileId(friend_key, userdata->gamecode);
        }

        return 0;
    }
    // NOT REACH HERE
    break;

    case DWC_ACC_FRIENDDATA_GS_PROFILE_ID:
        return DWCi_Acc_GetGsProfileId(&frienddata->gs_profile_id);
        // NOT REACH HERE
        break;
    case DWC_ACC_FRIENDDATA_LOGIN_ID:
        return -1;
        // NOT REACH HERE
        break;
    default:
        // NOT REACH HERE
        break;
    }

    return 0;
}

u64 DWC_CreateFriendKey(const DWCAccUserData* userdata)
{
    u64 ret = 0;

    if (userdata->gs_profile_id != 0)
    {
        ret = DWC_Acc_CreateFriendKey(userdata->gs_profile_id, userdata->gamecode);
    }

    return ret;
}

void DWC_CreateFriendKeyToken(DWCAccFriendData* token,
    u64 friend_key)
{
    memset(token, 0, sizeof(DWCAccFriendData));
    DWCi_Acc_SetFriendKey(&token->friend_key, friend_key);
    DWCi_Acc_SetFlag_DataType(&token->flags, DWC_ACC_FRIENDDATA_FRIEND_KEY);
}

void DWC_CreateExchangeToken(const DWCAccUserData* userdata,
    DWCAccFriendData* token)
{
    memset(token, 0, sizeof(DWCAccFriendData));

    if (DWCi_Acc_IsAuthentic(userdata))
    {
        DWCi_Acc_SetGsProfileId(&token->gs_profile_id, userdata->gs_profile_id);
        DWCi_Acc_SetFlag_DataType(&token->flags, DWC_ACC_FRIENDDATA_GS_PROFILE_ID);
    }
    else
    {
        // Although various flags have been inserted in an attempt to increase the probability,
        // due to the decision not to tie the console for which a temp ID is being generated to the console connecting to the net for which this ID is to be gotten
        // nearly all of the information is wasted.
        token->login_id = userdata->pseudo;
    }
}

void DWC_SetGsProfileId(DWCAccFriendData* frienddata,
    int gs_profile_id)
{
    memset(frienddata, 0, sizeof(DWCAccFriendData));

    DWCi_Acc_SetGsProfileId(&frienddata->gs_profile_id, gs_profile_id);
    DWCi_Acc_SetFlag_DataType(&frienddata->flags, DWC_ACC_FRIENDDATA_GS_PROFILE_ID);
}

void DWC_LoginIdToUserName(const DWCAccUserData* userdata,
    const DWCAccFriendData* frienddata,
    char* username)
{
    DWCi_Acc_LoginIdToUserName(&frienddata->login_id, userdata->gamecode, username);
}

BOOL DWC_IsEqualFriendData(const DWCAccFriendData* frienddata1,
    const DWCAccFriendData* frienddata2)
{
    u32 type1, type2;
    type1 = DWCi_Acc_GetFlag_DataType(&frienddata1->flags);
    type2 = DWCi_Acc_GetFlag_DataType(&frienddata2->flags);

    if (type1 != type2)
    {
        return FALSE;
    }
    else if (type1 == DWC_ACC_FRIENDDATA_GS_PROFILE_ID)
    {
        return (DWCi_Acc_GetGsProfileId(&frienddata1->gs_profile_id) == DWCi_Acc_GetGsProfileId(&frienddata2->gs_profile_id)) ? TRUE : FALSE;
    }
    else if (type1 == DWC_ACC_FRIENDDATA_LOGIN_ID)
    {
        return ((DWCi_Acc_GetUserId(&frienddata1->login_id) == DWCi_Acc_GetUserId(&frienddata2->login_id)) && (DWCi_Acc_GetPlayerId(&frienddata1->login_id) == DWCi_Acc_GetPlayerId(&frienddata2->login_id))) ? TRUE : FALSE;
    }
    else if (type1 == DWC_ACC_FRIENDDATA_FRIEND_KEY)
    {
        return (DWCi_Acc_GetFriendKey(&frienddata1->friend_key) == DWCi_Acc_GetFriendKey(&frienddata2->friend_key)) ? TRUE : FALSE;
    }

    return FALSE;
}

void DWC_ReportFriendData(const DWCAccUserData* userdata, const DWCAccFriendData* frienddata)
{
    if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_GS_PROFILE_ID)
    {
        if (DWC_IsBuddyFriendData(frienddata))
        {
            DWC_Printf(-1, " GS_ID : %d (ok)\n", DWCi_Acc_GetGsProfileId(&frienddata->gs_profile_id));
        }
        else
        {
            DWC_Printf(-1, " GS_ID : %d\n", DWCi_Acc_GetGsProfileId(&frienddata->gs_profile_id));
        }
    }
    else if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_FRIEND_KEY)
    {
        char name[DWC_ACC_FRIENDKEY_STRING_BUFSIZE];
        DWC_Acc_FriendKeyToNumericKey(DWCi_Acc_GetFriendKey(&frienddata->friend_key), name);
        DWC_Printf(-1, " F_KEY : %s\n", name);
    }
    else if (DWCi_Acc_GetFlag_DataType(&frienddata->flags) == DWC_ACC_FRIENDDATA_LOGIN_ID)
    {
        char name[DWC_ACC_USERNAME_STRING_BUFSIZE];
        DWCi_Acc_LoginIdToUserName(&frienddata->login_id, userdata->gamecode, name);
        DWC_Printf(-1, " LN_ID : %s\n", name);
    }
    else
    {
        DWC_Printf(-1, " NO_DATA \n");
    }
}

void DWC_ReportUserData(const DWCAccUserData* userdata)
{
    DWC_Printf(-1, "*******************************\n");
    DWC_Printf(-1, " [pseudo login id]\n");
    DWC_ReportFriendData(userdata, (DWCAccFriendData*)&userdata->pseudo);
    DWC_Printf(-1, "+++++++++++++++++++++++++++++++\n");
    DWC_Printf(-1, " [authentic login id]\n");
    DWC_ReportFriendData(userdata, (DWCAccFriendData*)&userdata->authentic);
    DWC_Printf(-1, "+++++++++++++++++++++++++++++++\n");
    DWC_Printf(-1, " GS_ID : %d\n", userdata->gs_profile_id);
    DWC_Printf(-1, "*******************************\n");
}
