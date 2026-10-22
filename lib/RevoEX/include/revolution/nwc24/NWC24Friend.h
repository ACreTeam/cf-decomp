#ifndef REVOLUTION_NWC24_FRIEND_H
#define REVOLUTION_NWC24_FRIEND_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_FRIEND_INFO_MAX 100

typedef enum NWC24FriendType {
    NWC24_FRIENDTYPE_NONE = 0,
    NWC24_FRIENDTYPE_WII,
    NWC24_FRIENDTYPE_EMAIL,
} NWC24FriendType;

typedef enum NWC24FriendStatus {
    NWC24_FRIENDSTATUS_NONE = 0,
    NWC24_FRIENDSTATUS_PENDING,
    NWC24_FRIENDSTATUS_CONFIRMED,
    NWC24_FRIENDSTATUS_DECLINED,
} NWC24FriendStatus;

#define NWC24_FRIEND_NAME_LENGTH 12

typedef union NWC24FriendAddr {
    NWC24UserId wiiId;           // Wii Friend Code
    NWC24UserMailAddr mailAddr;  // Email Address
} NWC24FriendAddr;

typedef struct NWC24FriendAttr {
    u32 type;                            // 0x00
    u32 status;                          // 0x04
    u16 name[NWC24_FRIEND_NAME_LENGTH];  // 0x08
    NWC24UserId fdId;                    // 0x20
    u32 reserved[6];                     // 0x28
} NWC24FriendAttr;

typedef struct NWC24FriendInfo {
    NWC24FriendAttr attr;  // 0x00
    NWC24FriendAddr addr;  // 0x40
} NWC24FriendInfo;

typedef struct NWC24SecretFriendInfo {
    NWC24UserId id;  // 0x00
    s32 key;         // 0x08
    u32 appId;       // 0x0C
    u32 time;        // 0x10
    u32 unk_0x14;
    u32 reserved[2];  // 0x18
} NWC24SecretFriendInfo;

NWC24Err NWC24AddSecretFriendInfo(const NWC24SecretFriendInfo* info);
NWC24Err NWC24DeleteSecretFriendInfo(const NWC24SecretFriendInfo* info);
NWC24Err NWC24ReadSecretFriendInfo(NWC24SecretFriendInfo* info, u32 index);
NWC24Err NWC24GetNumSecretFriendInfos(u32* num);
NWC24Err NWC24SearchSecretFriendInfo(const NWC24SecretFriendInfo* info, u32* index);

NWC24Err NWC24ReadFriendInfo(NWC24FriendInfo* info, u32 index);
NWC24Err NWC24WriteFriendInfo(const NWC24FriendInfo* friendInfo, u32 index);
NWC24Err NWC24DeleteFriendInfo(u32 index);
NWC24Err NWC24UpdateFriendInfo(const NWC24FriendInfo* friendInfo, u32 index);
NWC24Err NWC24SwapFriendInfos(u32 index1, u32 index2);
NWC24Err NWC24SearchFriendInfoById(NWC24UserId id, u32* index);
NWC24Err NWC24SearchFriendInfoByAddr(const NWC24FriendAddr* addr, u32* index);
NWC24Err NWC24GetNumFriendInfos(u32* numFriendInfo);
NWC24Err NWC24GetNumRegFriendInfos(u32* numRegFriendInfo);

BOOL NWC24IsFriendInfoThere(u32 index);  // This also returns NWC24Err

NWC24Err NWC24CheckPublicMailAddr(const char* addr);
NWC24Err NWC24CheckFriendInfo(const NWC24FriendInfo* friendInfo);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_NWC24_FRIEND_H
