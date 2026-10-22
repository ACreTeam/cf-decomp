#ifndef PRIVATE_NWC24_FRIEND_LIST_H
#define PRIVATE_NWC24_FRIEND_LIST_H

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24Types.h>

#include <revolution/nwc24/NWC24Friend.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Friend List */

typedef struct NWC24FLHeader {
    u32 magic;            // 0x00
    u32 version;          // 0x04
    u32 infoCount;        // 0x08
    u32 registeredCount;  // 0x0C
    u32 unk_0x10[4];
    u32 unk_0x20[7];
    u32 unk_0x3C;
    NWC24UserId friendIds[NWC24_FRIEND_INFO_MAX];  // 0x40
} NWC24FLHeader;

NWC24Err NWC24iInitFriendList(BOOL force);
NWC24Err NWC24iCreateFriendList();

NWC24Err NWC24iOpenFriendList();
NWC24Err NWC24iReadFriendInfoRaw(NWC24FriendInfo* info, u32 index);

/* Secret Friend List */

typedef struct NWC24SecretFLHeader {
    u32 magic;      // 0x00
    u32 version;    // 0x04
    u32 infoCount;        // 0x08
    u32 registeredCount;  // 0x0C
    u32 unk_0x10[4];
    s32 keys[336];   // 0x20
    u16 times[336];  // 0x560
} NWC24SecretFLHeader;

NWC24Err NWC24iInitSecretFriendList(BOOL force);

NWC24Err NWC24iCreateSecretFriendList();
NWC24Err NWC24iOpenSecretFriendList();

NWC24Err NWC24iAddSecretFriendInfo(const NWC24SecretFriendInfo* info, u32* index);
NWC24Err NWC24iFindEmptySecretFriendSlot(u32* index);
NWC24Err NWC24iFindOldestSecretFriendSlot(u32* index);
NWC24Err NWC24iSearchSecretFriendKey(s32 key, u32* index, u32 start);
NWC24Err NWC24iWriteSecretFriendInfo(const NWC24SecretFriendInfo* info, u32 index, BOOL replace);
NWC24Err NWC24iUpdateSecretFriendInfo(const NWC24SecretFriendInfo* info, u32 index);
NWC24Err NWC24iDeleteSecretFriendInfo(u32 index);
NWC24Err NWC24iReadSecretFriendInfoRaw(NWC24SecretFriendInfo* info, u32 index);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_FRIEND_LIST_H
