#pragma once

// d_wifi (src/dol/game/d_wifi.cpp + d_wifi_friend.inc, .text 80177A7C..8017C4EC): WiiConnect24 letters (the
// background thread jobs, sends / receives, the downloaded distribution files) and the friend list. Names are inferred.

#include <types.h>
#include <lib/dwc/dwc.h>

class dFriend_c;
struct dLandID_c;
class dPersonalID_c;
struct dPlayerID_c;
class dPrivateData_c;


// The current player's Wi-Fi Connection friend list (dFriendList_c): offline on the save, or through
// dNetMgr_c requests once the manager runs (mState > NET_MGR_STATE_SETUP). 8017B894..8017C4EC.
namespace dWifi {

// sReqState: the dNetMgr_c request a friend list operation waits for.
enum ReqState_e {
    REQ_NONE,
    REQ_REGISTER,   // reqRegister sent
    REQ_REGISTERED, // its result arrived (onRegistered)
    REQ_REMOVE,     // reqRemove sent
    REQ_REMOVED,    // its result arrived (onRemoved)
    REQ_FIND_USER,  // reqFindUser sent
    REQ_FIND_DATA,  // reqFindData sent
};

// The return value of the request functions while their request is pending.
#define WIFI_PENDING -2

DWCFriendData *getFriendData(int idx);                   // 8017B894: the current player's mKeys[idx]
void onRegistered(int idx);                              // 8017B8CC
void onRemoved(int idx);                                 // 8017B93C
DWCUserData *getUserData();                              // 8017B980: the current player's account
u64 getMyFriendKey();                                    // 8017B9A4
int getMyProfileId();                                    // 8017B9E0
BOOL isFriendValid(int idx);                             // 8017BA1C
int getFriendNum();                                      // 8017BA58
u64 getFriendKey(int idx);                               // 8017BAB0
int getFriendProfileId(int idx);                         // 8017BAEC
int findFreeFriend();                                    // 8017BB3C: -1 if full
int findMyself(const DWCUserData *userData);             // 8017BB8C
int findUser(const DWCUserData *userData);               // 8017BBB8: WIFI_PENDING while waiting
int findFriendData(const DWCFriendData *data);           // 8017BC40
int findFriendDataReq(const DWCFriendData *data);        // 8017BCC4: WIFI_PENDING while waiting
int findFriendKey(const u64 *key);                       // 8017BD4C: valid friends of the current player
int findFriendKey(const u64 *key, const dPrivateData_c *player);          // 8017BDEC
int findFriend(const dPersonalID_c *pid);                // 8017BEC0: valid friends of the current player
int findFriend(const dPersonalID_c *pid, const dPrivateData_c *player);   // 8017BF90
int findFriend(const dPlayerID_c *id, const dPrivateData_c *player);      // 8017C04C
int findFriend(const dLandID_c *land, const dPrivateData_c *player);      // 8017C0CC
BOOL registerFriend(int idx, const DWCFriendData *data, const dFriend_c *fr);    // 8017C170: offline only
BOOL registerFriendReq(int idx, const DWCFriendData *data, const dFriend_c *fr); // 8017C1F8
BOOL removeFriend(int idx);                              // 8017C29C: offline only
BOOL removeFriendReq(int idx);                           // 8017C2EC
void updateReq();                                        // 8017C35C: d_net_mgr execute, after copying the friend data
void updateFriend(u32 idx, const dPersonalID_c *pid, const wchar_t *message, const u64 *key); // 8017C3EC
BOOL updateSave();                                       // 8017C478: saves a changed list; TRUE when nothing is pending

} // namespace dWifi

// ---- region A (80177A7C..80178604) ----
// fn_80177A7C / fn_80177AE0 are declared in the region D block (C names kept for now).
struct dPostRequest_c;

namespace dWifi {

// sWc24State: what checkThread found.
// getThreadResult: the last thread job's result.
enum ThreadResult_e {
    THREAD_RESULT_BUSY = -1, // the job still runs
    THREAD_RESULT_FAILED = 0,
    THREAD_RESULT_OK = 1,
};

enum Wc24State_e {
    WC24_STATE_UNKNOWN, // not checked yet
    WC24_STATE_ON,      // WiiConnect24 can be used (dNetWc24::dNetWc24_c::canUseFriends)
    WC24_STATE_OFF,
};

typedef void *(*ThreadFunc)(void *arg); // OSThreadFunc

BOOL isWc24On();                                           // 80177BF8 (d_menu_mng)
int getWc24State();                                        // 80177C0C: Wc24State_e (d_save_town)
int getErrorCode();                                        // 80177C14: of the last failed check
int getErrorMessage();                                     // 80177C1C: of the last failed check
BOOL canSendLetter();                                      // 80177C28
BOOL isLetterDayOver();                                    // 80177C54: the current player's letter date isn't today
BOOL isLetterDayOver2();                                   // 80177C8C: the same
BOOL hasSend();                                            // 80177C90 (d_private_data)
BOOL enableNet();                                          // 80177CBC: dSaveExtra_c::mNetEnabled, while WC24 is on
void disableNet();                                         // 80177CFC
BOOL createThread(ThreadFunc func, const dPostRequest_c *req); // 80177E20: req is copied to 806215C0
BOOL isThreadDone();                                       // 80177F18: frees the thread's stack once it ended
BOOL startCheck();                                         // 80177F90 (boot): checkThread
BOOL startSendLetter(const dPostRequest_c *req);           // 80178048 (d_post_office): sendLetterThread
BOOL startUpdateRoster();                                  // 801780C4: updateRosterThread
BOOL startSendBoard(const dPostRequest_c *req);            // 80178120 (d_menu_mng): sendBoardThread
BOOL startSendSelf(const dPostRequest_c *req);             // 8017817C (d_menu_mng): sendSelfThread
BOOL startDeleteTasks();                                   // 80178234: deleteTasksThread
BOOL startStartThread();                                   // 80178290: startThread (8017B5D0)
int getNetErrorCode();                                     // 80178438: dNetWc24::dNetWc24_c::mErrorCode
int getErrorMessage(int error, u8 kind);                   // 8017845C: an NWC24Err -> its message
void *checkThread(void *arg);                              // 8017851C: sets sWc24State

} // namespace dWifi

// ---- region B (80178604..801795C4): receiving WiiConnect24 messages ----
struct dWifiTownMsg_c; // d_save_town.hpp
// region C's receivers (onReceive passes them the message data)
struct dSvAucList_c;
class dRelayDesign_c;
struct dSavePostSend_c;

// The header of a received WiiConnect24 message part (given to dWifi::onReceive).
struct dWifiMsgHeader_c {
    /* 0x0 */ u16 mPartNum : 2;     // a letter (WIFI_MSG_LETTER) with 2 or more parts: a dSaveDLItem_c first
    /* 0x0 */ u16 mOffset : 14;     // where the last part starts
    /* 0x2 */ u16 mCompressed : 1;  // CX run-length compressed (CXSecureUncompressRL)
    /* 0x2 */ u16 mType : 7;        // WIFI_MSG_*
    /* 0x2 */ u16 _2_lo : 8;
};

// Message types (dWifiMsgHeader_c::mType).
enum {
    WIFI_MSG_TOWN = 0,      // dWifiTownMsg_c (receiveTown)
    WIFI_MSG_LETTER = 1,    // a dMail_c, after a dSaveDLItem_c when mPartNum > 1
    WIFI_MSG_AUC_LIST = 2,  // 0x1E4 bytes (receiveAucListings)
    WIFI_MSG_AUC_BIDS = 3,  // 0x1E4 bytes (receiveAucBids)
    WIFI_MSG_AUC_SALES = 4, // 0x1E4 bytes (receiveAucSales)
    WIFI_MSG_DESIGN = 5,    // 0x8A0 bytes, may be compressed (receiveDesign)
    WIFI_MSG_POST = 6,      // 0x5A bytes (receivePostPid)
    WIFI_MSG_ANIMAL = 7,    // a villager (dNpc::receiveMovedAnimal) when mOffset is 0x3040, then the sender's dLandID_c
    WIFI_MSG_TYPE_NUM
};

namespace dWifi {
// 80178604: the dNetWc24::dNetWc24_c receive callback: handles one message part by its type. FALSE when a letter
// could not be delivered or kept at the post office (or what receiveDesign returns).
BOOL onReceive(const u64 *key, const void *data, u32 size, const dWifiMsgHeader_c *header);
void receiveTown(dWifiTownMsg_c *msg, const u64 *key); // 80178AFC: takes over what msg->mTown.mFlags says
// region C (names by its owner)
void receiveAucListings(const dSvAucList_c *list, const u64 *sender);  // 801795C4
void receiveAucBids(const dSvAucList_c *list, const u64 *sender);      // 80179678
void receiveAucSales(const dSvAucList_c *list, const u64 *sender);     // 80179744
BOOL receiveDesign(const dRelayDesign_c *board, const u64 *sender); // 801797F4
void receivePostPid(const dSavePostSend_c *msg, const u64 *sender);    // 801798A0
} // namespace dWifi

// ---- region C (801795C4..8017A708): the sends (the receivers are declared in region B's block) ----
namespace dNetWc24 { struct dNetWc24Data_c; }
namespace dNetWc24 { union dNetWc24MsgTag_c; }

namespace dWifi {
// Once a day: re-reads the registrations of the NWC24 friend list (dSaveWc24_c::_C5C) and which keys to
// send to (_C60, dSaveWc24_c::setSend).
void updateRoster();                                                        // 80179968
void updateRegistration();                                                  // 80179AE8: after clearSend
void setMsgData(const void *data, u32 size, dNetWc24::dNetWc24Data_c *msg, dNetWc24::dNetWc24MsgTag_c *tag); // 80179B90
int getSendProfileId(u32 idx);                                              // 80179BA8: of a send index
// The sends: TRUE when one went out.
BOOL sendTown(u32 max);                                                     // 80179C14: WIFI_MSG_TOWN
BOOL sendAucListings();                                                     // 80179DF0: WIFI_MSG_AUC_LIST
BOOL sendAucBids();                                                         // 8017A0FC: WIFI_MSG_AUC_BIDS
BOOL sendAucSales();                                                        // 8017A244: WIFI_MSG_AUC_SALES
BOOL sendDesign();                                                          // 8017A364: WIFI_MSG_DESIGN
BOOL sendPostPid();                                                         // 8017A484: WIFI_MSG_POST
BOOL sendAnimal(const void *animal);                                        // 8017A5DC: WIFI_MSG_ANIMAL
} // namespace dWifi

// ---- region D (8017A708..8017B894) ----
namespace EGG {
class Archive;
}
namespace dNetWc24 { struct dNetWc24FriendRec_c; }

// info.bin of the downloaded distribution archive (0x14 bytes).
struct dWifiInfoBin_c {
    /* 0x00 */ u32 mBoardSendLimit;  // dSaveWc24_c::setBoardSendLimit
    /* 0x04 */ u32 mLetterSendLimit; // dSaveWc24_c::setLetterSendLimit
    /* 0x08 */ u32 mLetterVersion;   // ltr<lang>.bmg (dSaveDistMail_c)
    /* 0x0C */ u32 mDesignVersion;   // design.bin (dSaveDistPattern_c)
    /* 0x10 */ u32 mContentVersion;  // the per-language files (dSaveDistContentList_c)
}; // size 0x14

namespace dWifi {

// Region A / D functions other units call.
u32 getGameCode(int area);                               // 80177A7C: the game code ('RUUJ'..) of a getLanguageArea area
void loadDistContent(EGG::Archive *arc, int lang, u8 slot); // 80177AE0: loads a per-language file of the archive
BOOL isNetOn();                                          // 80177D24: WiiConnect24 is on (and the save allows it)
BOOL isIdle();                                           // 80177D6C: idle (d_reset waits for it before a reset / quitting)
void reset();                                            // 80177DAC: (d_reset soft reset)
BOOL startRegisterFriends();                             // 80177FEC: starts registerFriendsThread (the friend list's updateSave)
int getThreadResult();                                   // 80178374: THREAD_RESULT_*; retries busy jobs
int startSetupDownload();                                // 801781D8: (d_sv_mgr)
int startDaily();                                        // 801782EC: (d_sv_mgr)
int getNetErrorMessage();                                // 80178448: a SYS_Save_and_Load code (SV_MSG_NAND_DAMAGED)
BOOL hasRoster();                                        // 8017AF60: the NWC24 friend list was read (dWifi::updateRosterThread)
u32 getRosterType(int i);                                // 8017AF7C: roster entry i's type (1: a friend with a key)
wchar_t *getRosterInfo(int i);                            // 8017AF90: roster entry i's name
int getNandResourceSize();                               // 8017B18C: dNetWc24 dNetWc24::getDownloadResourceSize (d_sv_mgr: NAND resource size)
int getNandAuxSize();                                    // 8017B190: dNetWc24 dNetWc24::getDownloadAuxSize (d_sv_mgr: NAND auxiliary size)

} // namespace dWifi

namespace dWifi {

// The thread functions (OSCreateThread by fn_80177E20 with the dPostRequest_c 806215C0) return NULL and
// leave their result in 8074B580 (1: done).
void updateDaily();                                       // 8017A708
void *registerFriendsThread(void *arg);                   // 8017AA6C
BOOL isFriendRecUsed(const dNetWc24::dNetWc24FriendRec_c *rec);     // 8017AB54: cleanFriendRecs callback
void *sendLetterThread(void *arg);                        // 8017ACAC
void *updateRosterThread(void *arg);                      // 8017ADB4
u32 getRosterNum();                                       // 8017AF74
u32 getRosterIdx(int i);                                  // 8017AFA4
dLandID_c *getRosterLand(int i);                          // 8017AFB4
void freeRoster();                                        // 8017AFF4
void *sendBoardThread(void *arg);                         // 8017B040
void *sendSelfThread(void *arg);                          // 8017B0F0
void *setupDownloadThread(void *arg);                     // 8017B194
void *deleteTasksThread(void *arg);                       // 8017B20C
void onDistDownload(void *data, u32 size, BOOL arg);      // 8017B274: download task 0
void onNgWordDownload(void *data, u32 size, BOOL arg);    // 8017B584: download task 1
void *startThread(void *arg);                             // 8017B5D0
void *dailyThread(void *arg);                             // 8017B7E4

} // namespace dWifi
