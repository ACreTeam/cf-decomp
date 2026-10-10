// The town info (dSaveTownInfo_c, dSaveTown_c::mTownInfo). .text 8014D0BC..8014D900.
// See include/game/game/d_sv_town_info.hpp.
#include <game/game/d_sv_town_info.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <revolution/OS.h>

// 8014D0BC
dSaveTownInfo_c::dSaveTownInfo_c() {
    mLampDate.month = 0;
    mLampDate.day = 1;
}

// 8014D27C
void dSaveTownInfo_c::init() {
    if (mFruit != dItem::ITEM_ID_NONE && mFruit != 0) {
        return;
    }
    dTime_c today = *dTime_c::getCurrent();
    today.add(0, -TIME_DAY_START_HOUR, 0, 0);
    today.hour = TIME_DAY_START_HOUR;
    today.sec = 0;
    today.min = 0;
    mLastDay.set(OSCalendarTimeToTicks(&today));
    dItem::Item fruit(dItem::ITEM_IDX_APPLE, (u32)cM::rndF(5.0f), FALSE);
    mFruit = fruit.mId;
    mPerfectDays = -1;
    mEventDay.clear();
    plantFruitTrees();
    removeSouthCedars();
    clearEndEvents();
    for (int i = 0; i < PLAYER_NUM; i++) {
        mMoneyTreeDates[i].clear();
    }
    mEggPlayer.clear();
    mLampDate.day = 0;
    mLampDate.month = 12;
    fgMngProc_initTownField();
}

// 8014D3F4
void dSaveTownInfo_c::updatePerfectDays(int rank, int days) {
    if (rank == dFdAssess_c::TOWN_RANK_PERFECT) {
        if (mPerfectDays < 0) {
            mPerfectDays = 1;
        } else {
            mPerfectDays += days;
        }
    } else {
        mPerfectDays = -1;
    }
}

// 8014D42C
void dSaveTownInfo_c::plantFruitTrees() {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return;
    }

    dItem::Item* item;
    int w;
    int h;
    int x;
    int z;
    u16 tree = mFruit;
    if (dItem::Item(dItem::ITEM_IDX_APPLE) == tree) {
        tree = dItem::FG_APPLE_TREE_FRUIT;
    } else if (dItem::Item(dItem::ITEM_IDX_ORANGE) == tree) {
        tree = dItem::FG_ORANGE_TREE_FRUIT;
    } else if (dItem::Item(dItem::ITEM_IDX_PEAR) == tree) {
        tree = dItem::FG_PEAR_TREE_FRUIT;
    } else if (dItem::Item(dItem::ITEM_IDX_PEACH) == tree) {
        tree = dItem::FG_PEACH_TREE_FRUIT;
    } else if (dItem::Item(dItem::ITEM_IDX_CHERRY) == tree) {
        tree = dItem::FG_CHERRY_TREE_FRUIT;
    }

    w = fd->getBlockW() * UT_X_NUM;
    h = fd->getBlockH() * UT_Z_NUM;

    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            item = fd->getItem(x, z, 0);
            if (item != NULL && item->isFruitTree()) {
                if ((item->mId >= dItem::FG_PALM_SAPLING && item->mId <= dItem::FG_PALM_FRUIT) == FALSE) {
                    fd->setItemFromId(tree, x, z, 0);
                    fd->clearBuried(x, z);
                }
            }
        }
    }
}

// 8014D5C4
void dSaveTownInfo_c::removeSouthCedars() {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return;
    }

    int w = fd->getBlockW() * UT_X_NUM;
    int h = fd->getBlockH() * UT_Z_NUM;
    int x;
    int z;

    for (z = (1 + FG_CEDAR_BLOCK_Z_NUM) * UT_Z_NUM; z < h; z++) {
        for (x = 0; x < w; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && (item->getId() >= dItem::FG_CEDAR_SAPLING && item->getId() <= dItem::FG_CEDAR_LIGHTS) != FALSE) {
                fd->setItemFromId(dItem::FG_TREE, x, z, 0);
            }
        }
    }
}

// 8014D69C
void dSaveTownInfo_c::setMoneyTreeRolled() {
    int idx = dPlayerMgr_c::getCurrentPlayer()->findInSave();
    dTime_c now = *dTime_c::getCurrent();
    mMoneyTreeDates[idx].set(&now);
}

// 8014D740
BOOL dSaveTownInfo_c::canRollMoneyTree() {
    int idx = dPlayerMgr_c::getCurrentPlayer()->findInSave();
    dTime_c now = *dTime_c::getCurrent();
    return !mMoneyTreeDates[idx].isDate(&now);
}

// 8014D828
void dSaveTownInfo_c::clearEndEvents() {
    mEndEvents[0] = TOWN_INFO_END_EVENT_NONE;
    mEndEvents[1] = TOWN_INFO_END_EVENT_NONE;
    mEndEvents[2] = TOWN_INFO_END_EVENT_NONE;
    mEndEvents[3] = TOWN_INFO_END_EVENT_NONE;
}

// 8014D844
int dSaveTownInfo_c::findEndEvent(int id) {
    int slot = -1;
    if (id == mEndEvents[0]) {
        slot = 0;
    } else if (id == mEndEvents[1]) {
        slot = 1;
    } else if (id == mEndEvents[2]) {
        slot = 2;
    } else if (id == mEndEvents[3]) {
        slot = 3;
    }
    return slot;
}

// 8014D89C
int dSaveTownInfo_c::addEndEvent(int id) {
    int slot = findEndEvent(id);
    if (slot < 0) {
        slot = findEndEvent(TOWN_INFO_END_EVENT_NONE);
        if (slot >= 0) {
            mEndEvents[slot] = id;
        }
    }
    return slot;
}
