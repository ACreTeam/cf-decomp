#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct DWCstAccUserData DWCAccUserData;
typedef union DWCstAccFriendData DWCAccFriendData;
typedef DWCAccUserData DWCUserData;
typedef DWCAccFriendData DWCFriendData;

void DWC_CreateUserData(DWCAccUserData* userdata);
int DWC_CheckUserData(const DWCAccUserData* userdata);
void DWC_ReportUserData(const DWCAccUserData* userdata);

#ifdef __cplusplus
}
#endif
