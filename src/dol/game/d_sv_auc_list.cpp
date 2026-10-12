// The auction records sent to other towns (dSvAucList_c, one slot per player). .text 8014F284..8014F8FC.
// See include/game/game/d_sv_auc.hpp.
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <string.h>

// 8014F284
void dSvAucList_c::init() {
    memset(this, 0, sizeof(dSvAucList_c));
    mVersion = AUC_LIST_VERSION;
}

// 8014F2C0
void dSvAucList_c::clear(u32 player) {
    if (player < PLAYER_NUM) {
        mMask &= ~(1 << player);
        mItems[player].clear();
    }
}

// 8014F2F0
void dSvAucList_c::clearAll() {
    mMask = 0;
    for (dSvAucItem_c *item = mItems; item < mItems + PLAYER_NUM; item++) {
        item->clear();
    }
}

// 8014F344
int dSvAucList_c::addMine(const dSvAucItem_c *item) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player != NULL) {
        u32 idx = dPrivateData_c::find(dSaveData_c::getRaw()->mPlayers, &player->mPID);
        if (idx < PLAYER_NUM) {
            mItems[idx] = *item;
            mMask |= 1 << idx;
            return idx;
        }
    }
    return -1;
}

// 8014F57C
int dSvAucList_c::addSeller(const dSvAucItem_c *item) {
    u32 idx = dPrivateData_c::find(dSaveData_c::getRaw()->mPlayers, &item->mSeller);
    if (idx < 7) {
        mItems[idx] = *item;
        mMask |= 1 << idx;
        return idx;
    }
    return -1;
}

// 8014F79C
int dSvAucList_c::getDay(u32 player) {
    if (player < PLAYER_NUM && (mMask & (1 << player))) {
        dSvAucItem_c *item = &mItems[player];
        if (item->isValid()) {
            return item->mDay;
        }
    }
    return -1;
}

// 8014F804
const dPersonalID_c *dSvAucList_c::getSeller() const {
    const dPersonalID_c *seller;
    for (u32 i = 0; i < PLAYER_NUM; i++) {
        seller = &getItem(i)->mSeller;
        if (seller->isValid()) {
            return seller;
        }
    }
    return NULL;
}

// 8014F880
const dPersonalID_c *dSvAucList_c::getHighBidder() const {
    const dPersonalID_c *bidder;
    for (u32 i = 0; i < PLAYER_NUM; i++) {
        bidder = &getItem(i)->mHighBidder;
        if (bidder->isValid()) {
            return bidder;
        }
    }
    return NULL;
}
