#ifndef DWC_DWC_H
#define DWC_DWC_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// DWC: Nintendo Wi-Fi Connection (lib/dwc/*, not decompiled). The real function
// names are unknown, so the functions keep their addresses; the comments say
// how d_net_mgr.cpp uses them. Only what the game uses so far.

// The console's own Wi-Fi Connection account (dFriendList_c::mUserData).
typedef struct DWCUserData {
    u8 _00[0x40];
} DWCUserData; // size 0x40

// One friend's account data (or a friend key token) (dFriendList_c::mKeys).
typedef struct DWCFriendData {
    u8 _00[0xC];
} DWCFriendData; // size 0xC

// match/dwc_friend.c
// sets the status data the friends see
void fn_8031A724(const void *data, u32 size);
// TRUE while the friend list is being synchronised
BOOL fn_8031A7BC(void);
// removes a friend from the server list
void fn_8031A7EC(DWCFriendData *friendData);

// match/dwc_main.c
u8 fn_8031E9C4(void); // own aid

// match/dwc_transport.c
void fn_80339AF0(u8 aid, void *buf, u32 size); // receive buffer for aid

// account/dwc_account.c
// kind: 2 friend key, 3 registered, ...
int fn_8033BC40(const DWCFriendData *friendData);
void fn_8033BC84(DWCFriendData *friendData); // clears it
// checks a friend key
int fn_8033BCB8(const DWCUserData *userData, u64 friendKey);
// TRUE if it holds data
BOOL fn_8033C28C(const DWCFriendData *friendData);
void fn_8033C2BC(DWCUserData *userData, DWCFriendData *friends, u32 num);
BOOL fn_8033C2F4(const DWCUserData *userData);
BOOL fn_8033C600(const DWCUserData *userData);
void fn_8033C63C(DWCUserData *userData);
// friend key of the data
u64 fn_8033C7B4(const DWCFriendData *friendData);
// profile id
int fn_8033C7DC(const DWCUserData *userData, const DWCFriendData *friendData);
// own friend key
u64 fn_8033C944(const DWCUserData *userData);
// friend data from a friend key
void fn_8033C9D8(DWCFriendData *friendData, u64 friendKey);
// own friend data
void fn_8033CA50(const DWCUserData *userData, DWCFriendData *friendData);
BOOL fn_8033CCDC(const DWCFriendData *a, const DWCFriendData *b); // same friend
void fn_8033CFE8(DWCUserData *userData);

#ifdef __cplusplus
}
#endif
#endif
