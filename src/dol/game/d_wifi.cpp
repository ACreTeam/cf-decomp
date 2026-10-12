// d_wifi: the WiiConnect24 letters (sending to Wii message board / registered friends, the downloaded
// info.bin / item.bin / letters / NPC / design data) and, at its end, the friend list functions.
// .text 80177A7C..8017C4EC. See include/game/game/d_wifi.hpp. Being matched by agents: build/wifi_work/BRIEF.md.
#include <game/game/d_wifi.hpp>
#include <game/game/d_friend.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_net_wus.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_s_boot_static.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_wc24.hpp>
#include <game/game/d_thunder.hpp>
#include <game/game/d_hr.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/revolution/CX/CXUncompression.h>
#include <game/mLib/m_perf.hpp>
#include <lib/egg/core/eggArchive.h>
#include <lib/egg/core/eggExpHeap.h>
#include <revolution/OS/OSThread.h>
#include <cstring>
#include <cstdio>

#define THREAD_STACK_SIZE 0x6000 // the job thread's stack (from dHeap::netWc24Heap_p)
#define RETRY_MAX 20             // retries of a job that failed with a busy error (dWifi::getThreadResult)
#define RETRY_WAIT 60            // frames before such a retry

// ---- data (in address order; placeholder names / types: refine with the code) ----

// .sdata
static int lbl_8074B580 = 1; // 8074B580: the result of the last thread job (1: done; dWifi::getThreadResult)
static u8 sErrorKind = 4;    // 8074B584: dNetWc24_c::mErrorKind of the last failed check

// .data 804F48A8 / 804F48D0 (their strings are the .sdata literals 8074B588..8074B5C7)
static const char *lbl_804F48A8[10] = {"jj", "ue", "us", "uf", "ee", "es", "ef", "ei", "eg", "kk"};
static const char *lbl_804F48D0[6] = {"BO", "HA", "KO", "FU", "GE", "TA"};

// .bss
static u8 sRecvBuffer[0x2728]; // 8061E460: a received message, decompressed (dWifiTownMsg_c / receiveDesign's 0x8A0 bytes)
static u64 sKeys[WC24_SEND_INDEX_NUM]; // 80620B88: the keys of the sends (region C) and of updateRoster
static OSThread sThread;        // 806212A8: runs the WiiConnect24 jobs (dWifi::createThread)
// 806215C0: the request the thread jobs get (constructed by __sinit 8017B858).
static dPostRequest_c lbl_806215C0;

// .sbss
static void *sThreadStack;       // 8074E938: THREAD_STACK_SIZE bytes
static OSThreadFunc sThreadFunc; // 8074E93C: the last job started (retried by dWifi::getThreadResult)
static u8 sRetryCount;           // 8074E940
static u8 sRetryWait;            // 8074E941: frames until that retry
static dNetWc24Friend_c *lbl_8074E944; // 8074E944: 100 entries from netWc24Heap_p (freed by dWifi::reset)
static u32 lbl_8074E948; // 8074E948
static int sWc24State;           // 8074E94C: dWifi::Wc24State_e (checkThread)
static int sError;               // 8074E950: the NWC24Err of the last failed check
static int sErrorCode;           // 8074E954: its error code

// ---- [A] 80177A7C..80178604 ----

// 80177A7C
u32 dWifi::getGameCode(int area) {
    switch (area) {
    case CONSOLE_REGION_JP:
        return 'RUUJ';
    case CONSOLE_REGION_NA:
        return 'RUUE';
    case CONSOLE_REGION_EU:
        return 'RUUP';
    case CONSOLE_REGION_KR:
        return 'RUUK';
    default:
        return 'RUUE';
    }
}

// 80177AE0
void dWifi::loadDistContent(EGG::Archive *arc, int lang, u8 slot) {
    EGG::Archive::FileInfo info;
    char name[16];
    strcpy(name, "msg");
    strcat(name, lbl_804F48D0[slot]);
    strcat(name, lbl_804F48A8[lang]);
    strcat(name, ".bmg");
    u32 size;
    void *data = arc->getFile(name, &info);
    if (data != NULL) {
        size = info.mFileSize;
        dSaveDistContent_c *content = &dSaveData_c::getExtra()->mDistContent.mContents[lang][slot];
        content->set(data, size);
    } else {
        dSaveData_c::getExtra()->mDistContent.mContents[lang][slot].clear();
    }
}

// 80177BF8
BOOL dWifi::isWc24On() {
    return sWc24State == WC24_STATE_ON;
}

// 80177C0C
int dWifi::getWc24State() {
    return sWc24State;
}

// 80177C14
int dWifi::getErrorCode() {
    return sErrorCode;
}

// 80177C1C
int dWifi::getErrorMessage() {
    return getErrorMessage(sError, sErrorKind);
}

// 80177C28
BOOL dWifi::canSendLetter() {
    return dSaveData_c::getExtra()->mWc24.canSendLetter();
}

// 80177C54
BOOL dWifi::isLetterDayOver() {
    return dSaveWc24_c::getDaysSince(&dPlayerMgr_c::getCurrentPlayer()->mWc24LetterDate) != 0;
}

// 80177C8C
BOOL dWifi::isLetterDayOver2() {
    return isLetterDayOver();
}

// 80177C90
BOOL dWifi::hasSend() {
    return dSaveData_c::getExtra()->mWc24.hasSend();
}

// 80177CBC
BOOL dWifi::enableNet() {
    if (sWc24State == WC24_STATE_ON) {
        dSaveExtra_c *extra = dSaveData_c::getExtra();
        extra->mHeader.mNetEnabled = TRUE;
        return TRUE;
    }
    return FALSE;
}

// 80177CFC
void dWifi::disableNet() {
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    extra->mHeader.mNetEnabled = FALSE;
}

// 80177D24
BOOL dWifi::isNetOn() {
    if (dWifi::isWc24On() && dSaveData_c::getRawExtra()->mHeader.mNetEnabled) {
        return TRUE;
    }
    return FALSE;
}

// 80177D6C
BOOL dWifi::isIdle() {
    if (dWifi::isThreadDone() || fn_801086A4()) {
        return TRUE;
    }
    return FALSE;
}

// 80177DAC
void dWifi::reset() {
    if (!fn_801086A4()) {
        sThreadFunc = NULL;
        lbl_8074B580 = 1;
        sRetryCount = 0;
        sRetryWait = 0;
        if (lbl_8074E944 != NULL) {
            dHeap::netWc24Heap_p->free(lbl_8074E944);
            lbl_8074E944 = NULL;
        }
        lbl_8074E948 = 0;
    }
}

// 80177E20
BOOL dWifi::createThread(ThreadFunc func, const dPostRequest_c *req) {
    if (sRetryWait) {
        return FALSE;
    }
    if (!OSIsThreadTerminated(&sThread) || sThreadStack != NULL) {
        return FALSE;
    }
    if (req != NULL) {
        memcpy(&lbl_806215C0, req, sizeof(lbl_806215C0));
    }
    sThreadStack = dHeap::netWc24Heap_p->alloc(THREAD_STACK_SIZE, 0x20);
    OSCreateThread(&sThread, func, &lbl_806215C0, (u8 *)sThreadStack + THREAD_STACK_SIZE, THREAD_STACK_SIZE, 20,
                   OS_THREAD_DETACHED);
    fn_802B8D30(&sThread, fn_8016CF08(9));
    OSResumeThread(&sThread);
    return TRUE;
}

// 80177F18
BOOL dWifi::isThreadDone() {
    if (OSIsThreadTerminated(&sThread)) {
        if (sThreadStack != NULL) {
            fn_802B8D90(&sThread);
            dHeap::netWc24Heap_p->free(sThreadStack);
            sThreadStack = NULL;
        }
        return TRUE;
    }
    return FALSE;
}

// Starts func and keeps it for the retries of dWifi::getThreadResult.
static inline BOOL start(dWifi::ThreadFunc func, const dPostRequest_c *req) {
    if (dWifi::createThread(func, req)) {
        sRetryCount = 0;
        sRetryWait = 0;
        sThreadFunc = func;
        return TRUE;
    }
    return FALSE;
}

// 80177F90
BOOL dWifi::startCheck() {
    return start(checkThread, NULL);
}

// 80177FEC
BOOL dWifi::startRegisterFriends() {
    return start(dWifi::registerFriendsThread, NULL);
}

// 80178048
BOOL dWifi::startSendLetter(const dPostRequest_c *req) {
    if (canSendLetter()) {
        return start(sendLetterThread, req);
    }
    return FALSE;
}

// 801780C4
BOOL dWifi::startUpdateRoster() {
    return start(updateRosterThread, NULL);
}

// 80178120
BOOL dWifi::startSendBoard(const dPostRequest_c *req) {
    return start(sendBoardThread, req);
}

// 8017817C
BOOL dWifi::startSendSelf(const dPostRequest_c *req) {
    return start(sendSelfThread, req);
}

// 801781D8
int dWifi::startSetupDownload() {
    return start(dWifi::setupDownloadThread, NULL);
}

// 80178234
BOOL dWifi::startDeleteTasks() {
    return start(deleteTasksThread, NULL);
}

// 80178290
BOOL dWifi::startStartThread() {
    return start(startThread, NULL);
}

// 801782EC
int dWifi::startDaily() {
    BOOL enabled = dSaveData_c::getRawExtra()->mHeader.mNetEnabled != 0;
    dSaveWc24_c *wc24 = &dSaveData_c::getExtra()->mWc24;
    wc24->updateClearSend(enabled);
    return start(dWifi::dailyThread, NULL);
}

// 80178374
int dWifi::getThreadResult() {
    int result = -1;
    if (sRetryWait != 0) {
        if (--sRetryWait == 0 && !dWifi::createThread(sThreadFunc, NULL)) {
            sRetryWait = 1;
        }
    } else if (dWifi::isThreadDone()) {
        result = lbl_8074B580;
        if (result == 0) {
            NWC24Err err = lbl_805BE940.mLastError;
            if ((err == NWC24_ERR_MUTEX || err == NWC24_ERR_BUSY || err == NWC24_ERR_INPROGRESS) &&
                sRetryCount < RETRY_MAX) {
                sRetryCount++;
                result = -1;
                sRetryWait = RETRY_WAIT;
            }
        }
    }
    return result;
}

// 80178438
int dWifi::getNetErrorCode() {
    return lbl_805BE940.mErrorCode;
}

// 80178448
int dWifi::getNetErrorMessage() {
    return dWifi::getErrorMessage(lbl_805BE940.mLastError, lbl_805BE940.mErrorKind);
}

// 8017845C
int dWifi::getErrorMessage(int error, u8 kind) {
    int msg;
    switch (error) {
    case NWC24_ERR_NAND_CORRUPT:
        msg = 0xC;
        break;
    case NWC24_ERR_FILE_BROKEN:
    case NWC24_ERR_INTERNAL_VF:
    case NWC24_ERR_FILE_OTHER:
    case NWC24_ERR_FILE_NOEXISTS:
    case NWC24_ERR_FILE_WRITE:
    case NWC24_ERR_FILE_READ:
    case NWC24_ERR_FILE_CLOSE:
    case NWC24_ERR_FILE_OPEN:
    case NWC24_ERR_BROKEN:
        msg = 8;
        if (!kind) {
            msg = 0;
        }
        break;
    case NWC24_ERR_OLD_SYSTEM:
        msg = 1;
        break;
    case NWC24_ERR_INTERNAL_IPC:
    case NWC24_ERR_NOMEM:
    case NWC24_ERR_FATAL:
        msg = 9;
        if (!kind) {
            msg = 2;
        }
        break;
    case NWC24_ERR_INPROGRESS:
    case NWC24_ERR_BUSY:
    case NWC24_ERR_MUTEX:
        msg = 3;
        break;
    case NWC24_ERR_DISABLED:
        msg = 4;
        break;
    case NWC24_ERR_NETWORK:
        msg = 5;
        break;
    case NWC24_ERR_SERVER:
        msg = 6;
        break;
    case NWC24_ERR_FULL:
        msg = 7;
        break;
    case NWC24_ERR_PROTECTED:
        msg = 0xB;
        break;
    default:
        if (lbl_805BE940.mNandState == 5) { // a NAND operation failed
            msg = 0xD;
            if (lbl_805BE940.mNandResult == NAND_RESULT_CORRUPT) {
                msg = 0xC;
            }
        } else {
            msg = 0xE;
        }
        break;
    }
    return msg;
}

// 8017851C
void *dWifi::checkThread(void *arg) {
    BOOL suspended = dNetWc24_c::suspendScheduler();
    if (lbl_805BE940.open()) {
        sWc24State = lbl_805BE940.canUseFriends() ? WC24_STATE_ON : WC24_STATE_OFF;
        sError = lbl_805BE940.mLastError;
        sErrorCode = lbl_805BE940.mErrorCode;
        sErrorKind = lbl_805BE940.mErrorKind;
        lbl_805BE940.close();
        lbl_8074B580 = 1;
    } else {
        NWC24Err err = lbl_805BE940.mLastError;
        if (err != NWC24_ERR_MUTEX && err != NWC24_ERR_BUSY && err != NWC24_ERR_INPROGRESS) {
            sWc24State = WC24_STATE_OFF;
            sError = err;
            sErrorCode = lbl_805BE940.mErrorCode;
            sErrorKind = lbl_805BE940.mErrorKind;
        }
        lbl_8074B580 = 0;
    }
    if (suspended) {
        dNetWc24_c::resumeScheduler();
    }
    return NULL;
}

// ---- [B] 80178604..801795C4 ----
namespace dWifi {

// 80178604
BOOL onReceive(const u64 *key, const void *data, u32 size, const dWifiMsgHeader_c *header) {
    BOOL result = TRUE;
    switch (header->mType) {
    case WIFI_MSG_TOWN:
        if (dSaveData_c::isNetEnabled()) {
            if (header->mCompressed && size != 0 && CXGetUncompressedSize(data) == sizeof(dWifiTownMsg_c)) {
                if (CXSecureUncompressRL(data, size, sRecvBuffer) == 0) {
                    receiveTown((dWifiTownMsg_c *)sRecvBuffer, key);
                }
            } else if (!header->mCompressed && size == sizeof(dWifiTownMsg_c)) {
                receiveTown((dWifiTownMsg_c *)data, key);
            }
        }
        break;
    case WIFI_MSG_LETTER: {
        if (dSaveData_c::isNetEnabled() && size != 0) {
            if (header->mPartNum == 0) {
                break;
            }
            const void *mailData;
            if (header->mPartNum > 1) {
                mailData = (const u8 *)data + header->mOffset;
                dSaveDLItemList_c::get()->add((const dSaveDLItem_c *)data, 0);
            } else {
                mailData = data;
            }
            dMail_c mail;
            cLib::memCpy(&mail, mailData, sizeof(dMail_c));
            bool deliver = false;
            dPersonalID_c *from = mail.getFromPlayer();
            if (from != NULL) {
                dPersonalID_c *to = mail.getToPlayer();
                if (to != NULL) {
                    dPrivateData_c *player = dPrivateData_c::get(dSaveData_c::getTown()->mPlayers, to);
                    if (player != NULL && findFriend(from, player) >= 0) {
                        deliver = true;
                    }
                }
            } else {
                deliver = true;
            }
            if (deliver && !dPostOffice::deliver(&mail) && !dPostOffice::add(&mail)) {
                result = FALSE;
            }
            int idx = dSaveData_c::getExtra()->mWc24.findKey(key);
            if (idx >= 0 && idx < ARRAY_SIZE(dSaveData_c::getExtra()->mWc24.mKeys)) {
                if (from != NULL) {
                    dSaveData_c::getWc24()->mLands[idx].copy(&from->land);
                } else {
                    dAnmPersonalID_c *animal = mail.getFromAnimal();
                    if (animal != NULL) {
                        dSaveData_c::getWc24()->mLands[idx].copy(&animal->mLand);
                    }
                }
            }
        }
        break;
    }
    case WIFI_MSG_AUC_LIST:
        if (dSaveData_c::isNetEnabled() && size == 0x1E4) {
            receiveAucListings((const dSvAucList_c *)data, key);
        }
        break;
    case WIFI_MSG_AUC_BIDS:
        if (dSaveData_c::isNetEnabled() && size == 0x1E4) {
            receiveAucBids((const dSvAucList_c *)data, key);
        }
        break;
    case WIFI_MSG_AUC_SALES:
        if (dSaveData_c::isNetEnabled() && size == 0x1E4) {
            receiveAucSales((const dSvAucList_c *)data, key);
        }
        break;
    case WIFI_MSG_DESIGN:
        if (dSaveData_c::isNetEnabled()) {
            if (header->mCompressed && size != 0 && CXGetUncompressedSize(data) == 0x8A0) {
                if (CXSecureUncompressRL(data, size, sRecvBuffer) == 0) {
                    result = receiveDesign((const dRelayDesign_c *)sRecvBuffer, key);
                }
            } else if (!header->mCompressed && size == 0x8A0) {
                result = receiveDesign((const dRelayDesign_c *)data, key);
            }
        }
        break;
    case WIFI_MSG_POST:
        if (dSaveData_c::isNetEnabled() && size == 0x5A) {
            receivePostPid((const dSavePostSend_c *)data, key);
        }
        break;
    case WIFI_MSG_ANIMAL:
        if (dSaveData_c::isNetEnabled() && header->mPartNum >= 2) {
            if (header->mOffset == 0x3040) {
                fn_800F4A34((dAnimal_c *)data);
            }
            if (size - header->mOffset == sizeof(dLandID_c)) {
                int idx = dSaveData_c::getExtra()->mWc24.findKey(key);
                if (idx >= 0 && idx < ARRAY_SIZE(dSaveData_c::getExtra()->mWc24.mKeys)) {
                    dLandID_c land;
                    cLib::memCpy(&land, (const u8 *)data + header->mOffset, sizeof(dLandID_c));
                    dSaveData_c::getWc24()->mLands[idx].copy(&land);
                }
            }
        }
        break;
    }
    return result;
}

// An item the outfit can show: a known BITM, not a pattern.
static inline BOOL isOutfitItem(const dItem::Item &item) {
    BOOL ok = FALSE;
    if (!item.isOrgDesign()) {
        if (dItem::infoBank_c::get()->getBITM(item)) {
            ok = TRUE;
        }
    }
    return ok;
}

// The outfit, or the default clothes if it is not valid.
static inline const dOutfit_c *getShownOutfit(dOutfit_c *outfit) {
    if (outfit->isValid()) {
        return outfit;
    }
    outfit->mEquip.mShirt = dItem::Item(dItem::ITEM_IDX_GRACIES_TOP);
    outfit->mEquip.mHat = dItem::Item(dItem::ITEM_IDX_GRACIE_HAT);
    outfit->mEquip.mHeld = dItem::Item();
    outfit->mEquip.mAcc = dItem::Item();
    return outfit;
}

// isOutfitItem, or no item.
static inline BOOL isOutfitItemOrNone(const dItem::Item &item) {
    BOOL ok = TRUE;
    if (item.getId() != dItem::ITEM_ID_NONE) {
        if (!isOutfitItem(item)) {
            ok = FALSE;
        }
    }
    return ok;
}

// Other town slot idx (< 4) becomes town.
static inline void setOtherTown(u32 idx, const dSaveOtherTown_c *town) {
    if (idx < 4) {
        dSaveData_c::getTown()->mOtherTowns[idx] = *town;
    }
}

// 80178AFC
void receiveTown(dWifiTownMsg_c *msg, const u64 *key) {
    if (msg->mTown.mVersion != VILLAGER_MOVE_VERSION) {
        return;
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_CATHERINE) {
        int category = msg->mTown.mCatherineCategory;
        int sub = msg->mTown.mCatherineSub;
        if (category >= 0 && sub >= 0) {
            int player = msg->mTown.mCatherinePlayer;
            dSaveCatherine_c *catherine = &dSaveData_c::getTown()->mShops.mCatherine;
            catherine->set(&msg->mPlayers[player], category, sub);
        }
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (msg->mPlayers[i].isValid()) {
                category = msg->mTown.mPlayerCategory[i];
                sub = msg->mTown.mPlayerSub[i];
                if (category >= 0 && sub >= 0) {
                    dSaveData_c::getTown()->mShops.mCatherine.count(category, sub);
                }
            }
        }
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_PLAYER_BITS) {
        int num = 0;
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (msg->mPlayers[i].isValid() && msg->mTown.isPlayerBit(i)) {
                num++;
            }
        }
        int pick = cM::rndInt(num);
        int n = 0;
        dPersonalID_c *player = msg->mPlayers;
        for (int i = 0; i < PLAYER_NUM; i++, player++) {
            if (player->isValid()) {
                if (msg->mTown.isPlayerBit(i)) {
                    if (n == pick) {
                        dSaveData_c::getTown()->_072CC2 = *player;
                    }
                    n++;
                } else if (dSaveData_c::getTown()->_072CC2 == *player) {
                    dSaveData_c::getTown()->_072CC2.clear();
                }
            }
        }
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_OUTFIT) {
        dOutfit_c outfit;
        if (msg->mTown.getOutfit(&outfit)) {
            const dOutfit_c *o = getShownOutfit(&outfit);
            dItem::Item shirt = o->mEquip.mShirt;
            dItem::Item hat = o->mEquip.mHat;
            dItem::Item acc = o->mEquip.mAcc;
            // Every part must be a known item (not a pattern); the hat and the accessory may be missing.
            BOOL partOk = FALSE;
            if (!shirt.isOrgDesign()) {
                if (dItem::infoBank_c::get()->getBITM(shirt)) {
                    partOk = TRUE;
                }
            }
            bool ok = partOk;
            partOk = TRUE;
            if (hat.getId() != dItem::ITEM_ID_NONE) {
                BOOL known = FALSE;
                if (!hat.isOrgDesign()) {
                    if (dItem::infoBank_c::get()->getBITM(hat)) {
                        known = TRUE;
                    }
                }
                if (!known) {
                    partOk = FALSE;
                }
            }
            ok &= partOk;
            partOk = TRUE;
            if (acc.getId() != dItem::ITEM_ID_NONE) {
                BOOL known = FALSE;
                if (!acc.isOrgDesign()) {
                    if (dItem::infoBank_c::get()->getBITM(acc)) {
                        known = TRUE;
                    }
                }
                if (!known) {
                    partOk = FALSE;
                }
            }
            ok &= partOk;
            if (ok) {
                dSaveData_c::getExtra()->mOutfitLand = msg->getSender()->land;
                dSaveData_c::getExtra()->mOutfit.copy(&outfit);
            }
        }
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_MODEL_ROOM) {
        dHR::receiveModelRoom(&msg->mTown.mModelRoomCandidate);
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_DL_ITEM && msg->mTown.mDLItem.isUsed()) {
        dSaveDLItemList_c::get()->add(&msg->mTown.mDLItem, 0);
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_JINX) {
        dPersonalID_c *sender = msg->getSender();
        dSaveData_c::getTown()->mTownList.set(&sender->land, msg->mTown.mJinx);
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_PLAYER_VALUE) {
        int i;
        dPersonalID_c *player;
        int value;
        player = msg->mPlayers;
        for (i = 0; i < PLAYER_NUM; i++, player++) {
            if (player->isValid()) {
                value = msg->mTown.getPlayerValue(i);
                if (value != 0) {
                    dSaveData_c::getExtra()->addRumor(*player, value);
                }
            }
        }
    }
    if (msg->mTown.mFlags & dSvVillagerMove::VILLAGER_MOVE_FLAG_OTHER_TOWN) {
        int slot;
        int i;
        int j;
        dSaveOtherTown_c *other;
        other = &msg->mTown.mOtherTown;
        if (other->mLand.isValid()) {
            slot = -1;
            for (i = 0; i < 4; i++) {
                if (other->mLand == dSaveData_c::getTown()->mOtherTowns[i].mLand) {
                    slot = i;
                    break;
                }
            }
            if (slot == -1) {
                for (j = 0; j < 3; j++) {
                    dSaveData_c::getTown()->mOtherTowns[j] = dSaveData_c::getTown()->mOtherTowns[j + 1];
                }
                slot = 3;
            }
            setOtherTown(slot, other);
        }
    }
    int idx = dSaveData_c::getExtra()->mWc24.findKey(key);
    if (idx >= 0 && idx < ARRAY_SIZE(dSaveData_c::getExtra()->mWc24.mKeys)) {
        dSaveData_c::getExtra()->mWc24.mLands[idx].copy(&msg->getSender()->land);
    }
}

} // namespace dWifi


// ---- [C] 801795C4..8017A708 ----
namespace dWifi {

// 801795C4
void receiveAucListings(const dSvAucList_c *list, const u64 *sender) {
    if (list->mVersion == AUC_LIST_VERSION) {
        // The cast goes once dSvAuc_c::receiveListings takes a const list (it only reads it).
        dSvAuc_c::get()->receiveListings((dSvAucList_c *)list, sender, TRUE);
        int idx = dSaveData_c::getExtra()->mWc24.findKey(sender);
        if (idx >= 0 && (u32)idx < WC24_FRIEND_NUM) {
            const dPersonalID_c *seller = list->getSeller();
            if (seller != NULL) {
                dSaveData_c::getExtra()->mWc24.mLands[idx].copy(&seller->land);
            }
        }
    }
}

// 80179678
void receiveAucBids(const dSvAucList_c *list, const u64 *sender) {
    if (list->mVersion == AUC_LIST_VERSION) {
        // The cast goes once dSvAuc_c::receiveBids takes a const list (it only reads it).
        if (dSvAuc_c::get()->receiveBids((dSvAucList_c *)list, TRUE)) {
            dSaveData_c::getExtra()->mWc24.setTargetSent(sender);
        }
        int idx = dSaveData_c::getExtra()->mWc24.findKey(sender);
        if (idx >= 0 && (u32)idx < WC24_FRIEND_NUM) {
            const dPersonalID_c *bidder = list->getHighBidder();
            if (bidder != NULL) {
                dSaveData_c::getExtra()->mWc24.mLands[idx].copy(&bidder->land);
            }
        }
    }
}

// 80179744
void receiveAucSales(const dSvAucList_c *list, const u64 *sender) {
    if (list->mVersion == AUC_LIST_VERSION) {
        dSvAuc_c::get()->receiveSales(list, TRUE);
        int idx = dSaveData_c::getExtra()->mWc24.findKey(sender);
        if (idx >= 0 && (u32)idx < WC24_FRIEND_NUM) {
            const dPersonalID_c *seller = list->getSeller();
            if (seller != NULL) {
                dSaveData_c::getExtra()->mWc24.mLands[idx].copy(&seller->land);
            }
        }
    }
}

// 801797F4
BOOL receiveDesign(const dRelayDesign_c *board, const u64 *sender) {
    if (board->mKind != 2) {
        return TRUE;
    }
    dRelayDesign_c *mine = &dSaveData_c::getExtra()->mDesignBoard;
    if (mine->mDays != 0 && mine->mActive != 0) {
        return mine->_884 == 0;
    }
    if (mine->mDesign.mCreator.isValid()) {
        return TRUE;
    }
    memcpy(mine, board, sizeof(dDesign_c));
    mine->mHops = board->mHops + 1;
    return TRUE;
}

// 801798A0
void receivePostPid(const dSavePostSend_c *msg, const u64 *sender) {
    if (msg->mSending == 1 && msg->mSender.isValid()) {
        if (msg->mTo.isValid()) {
            dPrivateData_c *player = dPrivateData_c::get(dSaveData_c::getTown()->mPlayers, &msg->mTo);
            if (player != NULL) {
                player->mLastWc24Sender.copy(&msg->mSender);
            }
        }
        int idx = dSaveData_c::getExtra()->mWc24.findKey(sender);
        if (idx >= 0 && (u32)idx < WC24_FRIEND_NUM) {
            dSaveData_c::getExtra()->mWc24.mLands[idx].copy(&msg->mSender.land);
        }
    }
}

// 80179968
void updateRoster() {
    dSaveWc24_c *wc24 = dSaveData_c::getWc24();
    BOOL updateRegs = dSaveWc24_c::getDaysSince(&wc24->_C5C) != 0;
    BOOL updateSends = dSaveWc24_c::getDaysSince(&wc24->_C60) != 0;
    if (updateRegs || updateSends) {
        dNetWus_c wus;
        u8 results[WC24_SEND_INDEX_NUM];
        BOOL failed = FALSE;
        if (wus.open()) {
            if (updateRegs) {
                if (wus.fn_800EB584(lbl_805BE940.mUserId, FALSE)) {
                    dSaveWc24_c::setToday(&wc24->_C5C);
                } else {
                    failed = TRUE;
                }
            }
            if (updateSends && !failed) {
                int num = wc24->getKeys(sKeys, WC24_SEND_INDEX_NUM);
                if (wus.fn_800EB700(sKeys, num, 10, results)) {
                    for (int i = 0; i < num; i++) {
                        if (results[i]) {
                            wc24->setSend(&sKeys[i], TRUE);
                        } else {
                            wc24->setSend(&sKeys[i], FALSE);
                        }
                    }
                    dSaveWc24_c::setToday(&wc24->_C60);
                }
            }
        }
        wus.close();
    }
}

// 80179AE8
void updateRegistration() {
    dSaveWc24_c *wc24 = &dSaveData_c::getExtra()->mWc24;
    if (wc24->mSendCleared && dSaveWc24_c::getDaysSince(&wc24->_C5C)) {
        dNetWus_c wus;
        if (wus.open() && wus.fn_800EB584(lbl_805BE940.mUserId, TRUE)) {
            dSaveWc24_c::setToday(&wc24->_C5C);
            wc24->mSendCleared = 0;
        }
        wus.close();
    }
}

// 80179B90
void setMsgData(const void *data, u32 size, dNetWc24Data_c *msg, dNetWc24MsgTag_c *tag) {
    msg->mData[2] = data;
    msg->mSize[2] = size;
    tag->mBits.mFlag = 0;
}

// 80179BA8
int getSendProfileId(u32 idx) {
    if (idx < WC24_FRIEND_NUM) {
        return getMyProfileId();
    }
    dPrivateData_c *player = dPlayerMgr_c::getPlayer((idx - WC24_FRIEND_NUM) / FRIEND_NUM);
    if (player != NULL && player->mPID.isValid()) {
        return dNetMgr_c::getMyProfileId(&player->mFriends.mUserData);
    }
    return 0;
}

// The send loops index the key / roster index buffers through u32 accessors (offset induction registers).
static inline u64 *getSendKey(u32 i) { return &sKeys[i]; }
static inline u32 getSendIndex(const u32 *indices, u32 i) { return indices[i]; }

// 80179C14
BOOL sendTown(u32 max) {
    u32 indices[WC24_SEND_INDEX_NUM];
    if (max > WC24_SEND_INDEX_NUM) {
        max = WC24_SEND_INDEX_NUM;
    }
    int num = dSaveData_c::getWc24()->getNextSend(indices, sKeys, max);
    if (num <= 0) {
        return FALSE;
    }
    dNetWc24MsgTag_c tag;
    dNetWc24MsgTag_c *pTag = &tag;
    pTag->mBits.mKind = WIFI_MSG_TOWN;
    u8 *buf = sRecvBuffer;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPersonalID_c *pid = &dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, i)->mPID;
        memcpy(&buf[i * sizeof(dPersonalID_c)], pid, sizeof(dPersonalID_c));
    }
    memcpy(&buf[PLAYER_NUM * sizeof(dPersonalID_c)], &dSaveData_c::getTown()->mVillagerMove, sizeof(dSvVillagerMove::Info_c));
    dNetWc24Data_c data;
    setMsgData(sRecvBuffer, sizeof(dWifiTownMsg_c), &data, pTag);
    dNetWc24MsgTag_c msgTag = tag;
    u32 size = lbl_805BE940.makeMessage(data.mData[2], data.mSize[2], NULL, 0, msgTag.mRaw);
    BOOL sent = FALSE;
    for (int i = 0; i < num; i++) {
        if (lbl_805BE940.sendMessage(getSendKey(i), size, getSendProfileId(getSendIndex(indices, i)))) {
            dSaveData_c::getExtra()->getWc24()->setNextSend(indices[(u32)i]);
            dSaveData_c::getExtra()->getWc24()->addBoardSendCount(1);
            sent |= TRUE;
        } else if (lbl_805BE940.mLastError == NWC24_ERR_NAND_CORRUPT) {
            break;
        }
    }
    return sent;
}

// 80179DF0
BOOL sendAucListings() {
    dNetWc24MsgTag_c tag;
    u32 *idx;
    u64 *target;
    u32 size;
    BOOL sent;
    int i;
    u32 targetIdx[WC24_TARGET_NUM];
    u64 targets[WC24_TARGET_NUM];
    u32 indices[WC24_SEND_INDEX_NUM];
    int num = dSaveData_c::getExtra()->getWc24()->getSendKeys(indices, sKeys, WC24_SEND_INDEX_NUM);
    if (num <= WC24_TARGET_NUM) {
        for (int i = 0; i < WC24_TARGET_NUM; i++) {
            targets[i] = sKeys[i];
            targetIdx[i] = indices[i];
        }
    } else {
        for (int i = 0; i < WC24_TARGET_NUM; i++) {
            u32 r = cM::rndInt(num);
            while (sKeys[r] == 0) {
                r++;
                if (r >= WC24_SEND_INDEX_NUM) {
                    r = 0;
                }
            }
            targets[i] = sKeys[r];
            targetIdx[i] = indices[r];
            sKeys[r] = 0;
        }
        num = WC24_TARGET_NUM;
    }
    if (num <= 0) {
        return FALSE;
    }
    tag.mBits.mKind = WIFI_MSG_AUC_LIST;
    tag.mRaw &= 0x7FFF;
    size = lbl_805BE940.makeMessage(dSaveData_c::getExtra()->getAucListings(), sizeof(dSvAucList_c), NULL, 0,
                                        tag.mRaw);
    sent = FALSE;
    idx = targetIdx;
    target = targets;
    for (i = 0; i < num; i++) {
        if (lbl_805BE940.sendMessage(target, size, getSendProfileId(*idx))) {
            sent |= TRUE;
        } else {
            if (lbl_805BE940.mLastError == NWC24_ERR_NAND_CORRUPT) {
                break;
            }
            *target = 0;
        }
        idx++;
        target++;
    }
    for (; i < num; i++) {
        targets[i] = 0;
    }
    if (sent) {
        dSaveData_c::getExtra()->getWc24()->setTargets(targets, num);
    }
    return sent;
}

// 8017A0FC
BOOL sendAucBids() {
    dSvAucItem_c *item;
    dSvAucList_c *const list = dSaveData_c::getExtra()->getAucBids();
    item = list->mItems;
    dNetWc24Data_c data;
    dNetWc24MsgTag_c tag;
    data.mData[2] = list;
    data.mSize[2] = sizeof(dSvAucList_c);
    tag.mBits.mKind = WIFI_MSG_AUC_BIDS;
    tag.mBits.mFlag = 0;
    BOOL sent = FALSE;
    for (int i = 0; i < PLAYER_NUM; i++, item++) {
        if (list->mMask & (1 << i)) {
            u64 key = *dSvAuc_c::findSender(item);
            if (key != 0) {
                int idx = dSaveData_c::getRawExtra()->mWc24.findKey(&key);
                if (idx >= 0 && (u32)idx < WC24_SEND_INDEX_NUM) {
                    if (lbl_805BE940.sendMessage(&key, 1, &data, tag.mRaw, getSendProfileId(idx))) {
                        list->clear(i);
                        sent = TRUE;
                    } else if (lbl_805BE940.mLastError == NWC24_ERR_NAND_CORRUPT) {
                        break;
                    }
                }
            }
        }
    }
    return sent;
}

// 8017A244
BOOL sendAucSales() {
    int num = dSaveData_c::getExtra()->getWc24()->getSentTargets(sKeys, WC24_TARGET_NUM);
    if (num <= 0) {
        return FALSE;
    }
    dNetWc24MsgTag_c tag;
    tag.mBits.mKind = WIFI_MSG_AUC_SALES;
    tag.mRaw &= 0x7FFF;
    u32 size = lbl_805BE940.makeMessage(dSaveData_c::getExtra()->getAucSales(), sizeof(dSvAucList_c), NULL, 0,
                                        tag.mRaw);
    BOOL sent = FALSE;
    u64 *key = sKeys;
    for (int i = 0; i < num; i++) {
        int idx = dSaveData_c::getRawExtra()->mWc24.findKey(key);
        if (idx >= 0 && (u32)idx < WC24_SEND_INDEX_NUM) {
            if (lbl_805BE940.sendMessage(key, size, getSendProfileId(idx))) {
                sent |= TRUE;
            } else if (lbl_805BE940.mLastError == NWC24_ERR_NAND_CORRUPT) {
                break;
            }
        }
        key++;
    }
    return sent;
}

// 8017A364
BOOL sendDesign() {
    u32 indices[WC24_SEND_INDEX_NUM];
    int num = dSaveData_c::getExtra()->getWc24()->getSendKeys2(indices, sKeys, WC24_SEND_INDEX_NUM);
    if (num <= 0) {
        return FALSE;
    }
    dNetWc24MsgTag_c tag;
    dNetWc24MsgTag_c *pTag = &tag;
    pTag->mBits.mKind = WIFI_MSG_DESIGN;
    dNetWc24Data_c data;
    setMsgData(dSaveData_c::getDesignBoard(), sizeof(dRelayDesign_c), &data, pTag);
    BOOL sent = FALSE;
    if (num > 0) {
        int r = cM::rndInt(num);
        if (lbl_805BE940.sendMessage(&sKeys[r], 1, &data, pTag->mRaw, getSendProfileId(indices[r]))) {
            sent = TRUE;
        }
    }
    return sent;
}

// 8017A484
BOOL sendPostPid() {
    dPrivateData_c *player;
    u64 key = 0;
    int profileId = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        player = dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player != NULL && player->mPID.isValid()) {
            int f = findFriend(&dSaveData_c::getExtra()->mPostSend.mTo, player);
            if (f >= 0) {
                key = player->mFriends.mFriends[f].mKey;
                profileId = dNetMgr_c::getMyProfileId(&player->mFriends.mUserData);
                break;
            }
        }
    }
    BOOL sent = FALSE;
    if (key != 0) {
        dNetWc24Data_c data;
        dNetWc24MsgTag_c tag;
        data.mData[2] = &dSaveData_c::getExtra()->mPostSend;
        data.mSize[2] = sizeof(dSavePostSend_c);
        tag.mBits.mKind = WIFI_MSG_POST;
        if (lbl_805BE940.sendMessage(&key, 1, &data, tag.mRaw & 0x7FFF, profileId)) {
            sent = TRUE;
        }
    }
    return sent;
}

// 8017A5DC
BOOL sendAnimal(const void *animal) {
    u32 indices[WC24_SEND_INDEX_NUM];
    int num = dSaveData_c::getExtra()->getWc24()->getSendKeys2(indices, sKeys, WC24_SEND_INDEX_NUM);
    if (num <= 0) {
        return FALSE;
    }
    dNetWc24Data_c data;
    dNetWc24MsgTag_c tag;
    data.mData[2] = animal;
    data.mSize[2] = 0x3040;
    data.mData[3] = &dSaveData_c::getRaw()->mLandID;
    data.mSize[3] = sizeof(dLandID_c);
    tag.mBits.mKind = WIFI_MSG_ANIMAL;
    tag.mBits.mFlag = 0;
    BOOL sent = FALSE;
    if (num > 0) {
        int r = cM::rndInt(num);
        if (lbl_805BE940.sendMessage(&sKeys[r], 1, &data, tag.mRaw, getSendProfileId(indices[r]))) {
            sent = TRUE;
        }
    }
    return sent;
}

} // namespace dWifi

// ---- [D] 8017A708..8017B894 (incl. __sinit 8017B858) ----
namespace dWifi {

// 8017A708: the WiiConnect24 updates of a new day (the auction records to other towns, the design
// board, a moving villager, the post office), run by dailyThread.
void updateDaily() {
    dSaveData_c::getExtra()->mWc24.updateRoster();
    if (!lbl_805BE940.canSendMessages()) {
        return;
    }
    if (dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled) {
        updateRoster();
        dSaveWc24_c *wc24 = &dSaveData_c::getExtra()->mWc24;
        u8 sendNum = wc24->startBoardSendDay();
        if (dSaveData_c::getVillagerMove()->mFlags != 0 && sendNum != 0) {
            dSaveData_c::getVillagerMove()->mVersion = VILLAGER_MOVE_VERSION;
            if (sendTown(sendNum)) {
                dSaveData_c::getVillagerMove()->clearOutfitFlag();
                dSaveData_c::getTown()->setFlag(3);
            }
        }

        if (lbl_805BE940.mLastError != NWC24_ERR_NAND_CORRUPT) {
            switch (dSvAuc_c::getTodayType()) {
            case 1:
                if (dSaveData_c::getExtra()->getAucListings()->mMask != 0) {
                    dSaveData_c::getExtra()->getAucListings()->mVersion = AUC_LIST_VERSION;
                    if (sendAucListings()) {
                        dSaveData_c::getExtra()->getAucListings()->clearAll();
                    }
                }
            case 0:
                dSaveData_c::getExtra()->getAucBids()->clearAll();
                dSaveData_c::getExtra()->getAucSales()->clearAll();
                break;
            case 2:
            case 3:
                if (dSaveData_c::getExtra()->getAucBids()->mMask != 0) {
                    dSaveData_c::getExtra()->getAucBids()->mVersion = AUC_LIST_VERSION;
                    sendAucBids();
                }
                dSaveData_c::getExtra()->getAucListings()->clearAll();
                dSaveData_c::getExtra()->getAucSales()->clearAll();
                break;
            case 4:
                if (dSaveData_c::getExtra()->getAucSales()->mMask != 0) {
                    dSaveData_c::getExtra()->getAucSales()->mVersion = AUC_LIST_VERSION;
                    if (sendAucSales()) {
                        dSaveData_c::getExtra()->getAucSales()->clearAll();
                    }
                }
                dSaveData_c::getExtra()->getAucListings()->clearAll();
                dSaveData_c::getExtra()->getAucBids()->clearAll();
                break;
            }
        }

        if (lbl_805BE940.mLastError != NWC24_ERR_NAND_CORRUPT && dSaveData_c::getDesignBoard()->mSendPending) {
            if (dSaveData_c::getDesignBoard()->isHopLimit()) {
                dSaveData_c::getDesignBoard()->mSendPending = 0;
            } else if (dSaveWc24_c::getDaysSince(&wc24->_C70)) {
                dSaveData_c::getDesignBoard()->mKind = 2;
                if (sendDesign()) {
                    dSaveData_c::getDesignBoard()->mSendPending = 0;
                    dSaveData_c::getTown()->setFlag(0x10);
                    dSaveWc24_c::setToday(&wc24->_C70);
                }
            }
        }

        if (lbl_805BE940.mLastError != NWC24_ERR_NAND_CORRUPT && dSaveWc24_c::getDaysSince(&wc24->_C6C)) {
            dAnimal_c *animal = fn_800F4A08();
            if (animal != NULL && sendAnimal(animal)) {
                animal->clear();
                dSaveWc24_c::setToday(&wc24->_C6C);
            }
        }

        if (lbl_805BE940.mLastError != NWC24_ERR_NAND_CORRUPT && dSaveData_c::getPostSend()->mPending) {
            dSaveData_c::getPostSend()->mSending = 1;
            if (sendPostPid()) {
                dSaveData_c::getPostSend()->mPending = 0;
            }
        }
    } else {
        updateRegistration();
    }
}

// The registered friends of the current player as NWC24 friend records.
static inline void registerFriendRecs() {
    if (lbl_805BE940.canUseFriends()) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        for (int i = 0; i < FRIEND_NUM; i++) {
            int profileId = getFriendProfileId(i);
            u64 key = player->mFriends.getFriend(i)->mKey;
            if (profileId > 0 && key != 0) {
                dNetWc24_c::registerFriendRec(
                    &key, profileId, getGameCode(getLanguageArea(player->mFriends.getFriend(i)->mPID.land.mRegion))
                );
            }
        }
    }
}

// 8017AA6C
void *registerFriendsThread(void *arg) {
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        registerFriendRecs();
        lbl_805BE940.close();
        lbl_8074B580 = 1;
    }
    return NULL;
}

// 8017AB54: keeps an NWC24 friend record that belongs to a registered friend of a player of this town.
BOOL isFriendRecUsed(const dNetWc24FriendRec_c *rec) {
    int area;
    dPrivateData_c *player;
    switch (rec->mGameCode) {
    case 'RUUJ':
        area = CONSOLE_REGION_JP;
        break;
    case 'RUUE':
        area = CONSOLE_REGION_NA;
        break;
    case 'RUUP':
        area = CONSOLE_REGION_EU;
        break;
    case 'RUUK':
        area = CONSOLE_REGION_KR;
        break;
    default:
        return FALSE;
    }
    for (int p = 0; p < PLAYER_NUM; p++) {
        player = dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, p);
        if (player != NULL && player->mPID.isValid()) {
            const dFriendList_c *friends = &player->mFriends;
            for (int i = 0; i < FRIEND_NUM; i++) {
                const dFriend_c *fr;
                if (rec->mKey == (fr = &friends->mFriends[(u32)i])->mKey && area == getLanguageArea(fr->mPID.land.mRegion))
                {
                    int profileId = dNetMgr_c::getProfileId(&friends->mUserData, friends->getKey(i));
                    if (profileId == rec->mProfileId) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

// 8017ACAC: sends the request (a letter) to the registered friend req->mFriend of req->mSender.
void *sendLetterThread(void *arg) {
    dPostRequest_c *req = (dPostRequest_c *)arg;
    lbl_8074B580 = 0;
    dPrivateData_c *player = req->mSender != NULL ? req->mSender : dPlayerMgr_c::getCurrentPlayer();
    u64 key = player->mFriends.mFriends[req->mFriend].mKey;
    if (key != 0 && lbl_805BE940.open()) {
        if (lbl_805BE940.canSendMessages()) {
            dNetWc24MsgTag_c tag;
            tag.mBits.mKind = 1;
            tag.mBits.mFlag = 0;
            if (lbl_805BE940.sendMessage(&key, 1, req, tag.mRaw, dNetMgr_c::getMyProfileId(&player->mFriends.mUserData))) {
                lbl_8074B580 = 1;
                dSaveData_c::getExtra()->mWc24.addLetterSendCount();
            }
        }
        lbl_805BE940.close();
    }
    return NULL;
}

// 8017ADB4: reads the NWC24 friend list (the Wii friend roster) into sRoster and gives roster entries
// without a town the town of the registered friend with their key.
void *updateRosterThread(void *arg) {
    if (lbl_8074E944 == NULL) {
        lbl_8074E944 = (dNetWc24Friend_c *)dHeap::netWc24Heap_p->alloc(WC24_FRIEND_NUM * sizeof(dNetWc24Friend_c), 4);
    }
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        dSaveData_c::getExtra()->mWc24.updateRoster();
        lbl_8074E948 = lbl_805BE940.getFriends(lbl_8074E944, WC24_FRIEND_NUM);
        for (u32 i = 0; i < lbl_8074E948; i++) {
            u32 idx = getRosterIdx(i);
            if (getRosterType(i) == 1 && !getRosterLand(i)->isValid()) {
                int friendIdx = findFriendKey(&dSaveData_c::getRawExtra()->mWc24.mKeys[idx]);
                if (friendIdx >= 0) {
                    dLandID_c *land = &dPlayerMgr_c::getCurrentPlayer()->mFriends.mFriends[friendIdx].mPID.land;
                    if (land->isValid()) {
                        dSaveData_c::getExtra()->mWc24.mLands[idx].copy(land);
                    }
                }
            }
        }
        lbl_805BE940.close();
    }
    if (lbl_805BE940.mLastError == NWC24_OK) {
        lbl_8074B580 = 1;
    }
    if (lbl_8074B580 != 1) {
        dHeap::netWc24Heap_p->free(lbl_8074E944);
        lbl_8074E944 = NULL;
        lbl_8074E948 = 0;
    }
    return NULL;
}

} // namespace dWifi

// 8017AF60: sRoster was read (d_menu_mng)
BOOL dWifi::hasRoster() {
    return lbl_8074E944 != NULL;
}

namespace dWifi {

// 8017AF74
u32 getRosterNum() {
    return lbl_8074E948;
}

} // namespace dWifi

// 8017AF7C: the entry's type (1: a friend registered with a key; d_menu_mng)
u32 dWifi::getRosterType(int i) {
    return lbl_8074E944[i].mType;
}

// 8017AF90: the entry's data (d_menu_mng)
u8 *dWifi::getRosterInfo(int i) {
    return lbl_8074E944[i].mInfo;
}

namespace dWifi {

// 8017AFA4: the entry's roster index
u32 getRosterIdx(int i) {
    return lbl_8074E944[i].mIdx;
}

// 8017AFB4: the entry's town
dLandID_c *getRosterLand(int i) {
    u32 idx = getRosterIdx(i);
    return &dSaveData_c::getExtra()->mWc24.mLands[idx];
}

// 8017AFF4
void freeRoster() {
    if (lbl_8074E944 != NULL) {
        dHeap::netWc24Heap_p->free(lbl_8074E944);
        lbl_8074E944 = NULL;
    }
    lbl_8074E948 = 0;
}

// 8017B040: sends the request to the message board of roster entry getRosterIdx(req->mFriend).
void *sendBoardThread(void *arg) {
    dPostRequest_c *req = (dPostRequest_c *)arg;
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        if (lbl_805BE940.canSendMessages()) {
            req->mFriend = getRosterIdx(req->mFriend);
            if (lbl_805BE940.sendBoardMessage(req)) {
                lbl_8074B580 = 1;
                dSaveWc24_c::setToday(&dPlayerMgr_c::getCurrentPlayer()->mWc24LetterDate);
                dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x3E);
            }
        }
        lbl_805BE940.close();
    }
    return NULL;
}

// 8017B0F0: sends the request to this console's message board.
void *sendSelfThread(void *arg) {
    dPostRequest_c *req = (dPostRequest_c *)arg;
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        req->mFriend = -1;
        if (lbl_805BE940.sendBoardMessage(req)) {
            lbl_8074B580 = 1;
            dSaveWc24_c::setToday(&dPlayerMgr_c::getCurrentPlayer()->mWc24LetterDate);
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x3E);
        }
        lbl_805BE940.close();
    }
    return NULL;
}

} // namespace dWifi

// 8017B18C (d_sv_mgr)
int dWifi::getNandResourceSize() {
    return fn_800E94B4();
}

// 8017B190 (d_sv_mgr)
int dWifi::getNandAuxSize() {
    return fn_800E94C0();
}

namespace dWifi {

// 8017B194
void *setupDownloadThread(void *arg) {
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        if (lbl_805BE940.setupDownload() && lbl_805BE940.registerTasks()) {
            lbl_8074B580 = 1;
        }
        lbl_805BE940.close();
    }
    return NULL;
}

// 8017B20C
void *deleteTasksThread(void *arg) {
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        if (lbl_805BE940.deleteTasks()) {
            lbl_8074B580 = 1;
        }
        lbl_805BE940.close();
    }
    return NULL;
}

// 8017B274: the downloaded distribution archive (task 0): info.bin, item.bin, the letter, the NPCs,
// the pattern and the per-language files.
void onDistDownload(void *data, u32 size, BOOL arg) {
    EGG::Archive *arc = EGG::Archive::mount(data, dHeap::localHeap_p, 0x20);
    if (arc != NULL) {
        EGG::Archive::FileInfo fileInfo;
        void *design;
        u8 boardSendLimit;
        void *file = arc->getFile("info.bin", &fileInfo);
        const dWifiInfoBin_c *info = (const dWifiInfoBin_c *)file;
        if (info != NULL && fileInfo.mFileSize == sizeof(dWifiInfoBin_c)) {
            boardSendLimit = info->mBoardSendLimit;
            u32 letterVersion = info->mLetterVersion;
            u32 designVersion = info->mDesignVersion;
            u32 contentVersion = info->mContentVersion;
            dSaveData_c::getWc24()->setBoardSendLimit(boardSendLimit);
            u8 letterSendLimit = info->mLetterSendLimit;
            dSaveData_c::getWc24()->setLetterSendLimit(letterSendLimit);

            dItem::Item present;
            void *item = arc->getFile("item.bin", &fileInfo);
            if (item != NULL) {
                present = dSaveDLItemList_c::get()->add((const dSaveDLItem_c *)item, 0);
            }

            char letterName[16];
            int language = getLanguage();
            sprintf(letterName, "ltr%s.bmg", lbl_804F48A8[language]);
            void *letter = arc->getFile(letterName, &fileInfo);
            if (letter != NULL) {
                dSaveDistMail_c *mail = &dSaveData_c::getExtra()->mDistMail;
                if (letterVersion != mail->mVersion) {
                    mail->set(letter, present, letterVersion);
                }
            }

            dAnimal_c *npcs[6];
            u32 npcNum = 0;
            for (u32 i = 0; i < 6; i++) {
                char npcName[16];
                sprintf(npcName, "npc%d.bin", i);
                void *npc = arc->getFile(npcName, &fileInfo);
                if (npc != NULL) {
                    npcs[npcNum] = (dAnimal_c *)npc;
                    npcNum++;
                }
            }
            for (u32 i = npcNum; i < 6; i++) {
                npcs[i] = NULL;
            }
            if (npcNum != 0) {
                fn_800F4AB8(npcs);
            }

            design = arc->getFile("design.bin", &fileInfo);
            if (design != NULL) {
                dSaveDistPattern_c *pattern = &dSaveData_c::getExtra()->mDistPattern;
                if (designVersion != pattern->mVersion) {
                    pattern->set(design, fileInfo.mFileSize, designVersion);
                }
            }

            dSaveExtra_c *extra = dSaveData_c::getExtra();
            if (contentVersion != extra->mDistContent._00000) {
                extra->mDistContent._00000 = contentVersion;
                for (u32 i = 0; i < 6; i++) {
                    loadDistContent(arc, 1, i);
                    loadDistContent(arc, 2, i);
                    loadDistContent(arc, 3, i);
                }
                dSaveData_c::getExtra()->mLangFlags.updateAll();
            }
        }
    }
    arc->unmount();
}

// 8017B584: the downloaded NG-word patterns (task 1).
void onNgWordDownload(void *data, u32 size, BOOL arg) {
    dSaveData_c::getExtra()->mDistNgWords.set(data, size);
}

// 8017B5D0: the first WiiConnect24 run: the friend records, the roster, the received messages and the
// download tasks.
void *startThread(void *arg) {
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        registerFriendRecs();
        dSaveData_c::getExtra()->mWc24.updateRoster();
        lbl_805BE940.receiveMessages(onReceive);
        lbl_8074B580 = 1;
        if (dNetWc24_c::needsDownloadSetup() && !lbl_805BE940.setupDownload()) {
            lbl_8074B580 = 0;
        }
        if (lbl_8074B580 == 1) {
            int tasks = lbl_805BE940.checkTasks();
            if (tasks != 0) {
                if (tasks == 1) {
                    if (!lbl_805BE940.registerTasks()) {
                        lbl_8074B580 = 0;
                    }
                } else {
                    lbl_8074B580 = 0;
                }
            }
        }
        if (lbl_8074B580 == 1 && lbl_805BE940.canDownload()) {
            if (!lbl_805BE940.download(1, onNgWordDownload)) {
                lbl_8074B580 = 0;
            }
            if (dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled && (lbl_8074B580 == 1 || getNetErrorMessage() != 12)) {
                if (!lbl_805BE940.download(0, onDistDownload)) {
                    lbl_8074B580 = 0;
                }
            }
        }
        lbl_805BE940.close();
    }
    return NULL;
}

// 8017B7E4: updateDaily and the friend record cleanup.
void *dailyThread(void *arg) {
    lbl_8074B580 = 0;
    if (lbl_805BE940.open()) {
        updateDaily();
        dNetWc24_c::cleanFriendRecs(isFriendRecUsed);
        lbl_805BE940.close();
        if (lbl_805BE940.mLastError != NWC24_ERR_NAND_CORRUPT) {
            lbl_8074B580 = 1;
        }
    }
    return NULL;
}

} // namespace dWifi

// ---- 8017B894..8017C4EC: the friend list ----
#include "d_wifi_friend.inc"
