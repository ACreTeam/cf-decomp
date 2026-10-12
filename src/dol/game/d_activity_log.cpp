// The jinx progress log (dActivityLog). .text 800C76C0..800C7BE4, .ctors 804656C0, .data 804EA9E0..804EAA10
// (count's jump table), .bss 8059FF90..8059FFA8. See include/game/game/d_activity_log.hpp.
#include <game/game/d_activity_log.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>

namespace dActivityLog {

#define NO_VALUE -1

// 800C76C0
Log_c::Log_c() {
    mJinx.clear();
}

Log_c g_log;

// 800C7704
void create() {}

// 800C7708
void destroy() {}

// 800C770C
void clear() {
    dSaveJinx_c jinx;
    jinx.clear();
    g_log.set(&jinx);
}

// 800C7754
void setJinx(const dSaveJinx_c *jinx) {
    g_log.set(jinx);
}

// 800C7764
void Log_c::set(const dSaveJinx_c *jinx) {
    mJinx = *jinx;
    mCount = 0;
    mDone = FALSE;
}

// 800C7784
const dSaveJinx_c *getJinx() {
    return &g_log.mJinx;
}

// 800C7790
u8 isDone() {
    return g_log.mDone;
}

// 800C77A0
void onHeartLetter(int player) {
    g_log.count(JINX_COND_HEART_LETTER, player, NO_VALUE);
}

// 800C77B8
void onCutTree(int player) {
    g_log.count(JINX_COND_CUT_TREE, player, NO_VALUE);
}

// 800C77D0
void onPullWeeds(int player) {
    g_log.count(JINX_COND_PULL_WEEDS, player, NO_VALUE);
}

// 800C77E8
void onPlantTree(int player) {
    g_log.count(JINX_COND_PLANT_TREES, player, NO_VALUE);
}

// 800C7800
void onPlantFlower(int player) {
    g_log.count(JINX_COND_PLANT_FLOWERS, player, NO_VALUE);
}

// 800C7818
void onFindClover(int player) {
    g_log.count(JINX_COND_FOUR_LEAF_CLOVER, player, NO_VALUE);
}

// 800C7830
void onPlantRedTurnip(int player) {
    g_log.count(JINX_COND_RED_TURNIP, player, NO_VALUE);
}

// 800C7848
void onCatchBug(int bug, int player) {
    g_log.count(JINX_COND_CATCH_BUG, player, bug);
}

// 800C7860
void onCatchFish(int fish, int player) {
    g_log.count(JINX_COND_CATCH_FISH, player, fish);
}

// 800C7878
void onPlantMoneyTree(int player) {
    g_log.count(JINX_COND_MONEY_TREE, player, NO_VALUE);
}

// 800C7890
void Log_c::count(int cond, int player, int value) {
    if (player != dNet::getMyNo()) {
        return;
    }
    dLandID_c *townLand = dSaveData_c::getTownLand();
    dPrivateData_c *me = dPlayerMgr_c::getCurrentPlayer();
    if (me->mPID.land == *townLand) {
        return; // only another town's jinx counts
    }
    if (cond != mJinx.mCondition) {
        return;
    }
    switch (cond) {
    case JINX_COND_HEART_LETTER:
    case JINX_COND_FOUR_LEAF_CLOVER:
    case JINX_COND_RED_TURNIP:
    case JINX_COND_MONEY_TREE:
        break;
    case JINX_COND_CUT_TREE:
    case JINX_COND_PULL_WEEDS:
    case JINX_COND_PLANT_TREES:
    case JINX_COND_PLANT_FLOWERS:
        if (++mCount != mJinx.mValue) {
            mDone = FALSE;
            return;
        }
        break;
    case JINX_COND_CATCH_BUG:
    case JINX_COND_CATCH_FISH:
        if (value != mJinx.mValue) {
            return;
        }
        break;
    }
    mDone = TRUE;}

// 800C7994
void applyVisitorDone(BOOL done) {
    if (dNet::isHost()) {
        dSaveData_c::getTown()->mActivity.setVisitorDone(done);
    }
}

// 800C79DC
void applyResult() {
    // Copy through a fixed source pointer to preserve the byte-wise copy.
    const dSaveJinx_c* source = g_log.getJinx();
    dSaveJinx_c jinx = *source;
    if (jinx.mCondition < JINX_COND_NUM) {
        dSaveTown_c *town = dSaveData_c::getTown();
        dPrivateData_c *me = dPlayerMgr_c::getCurrentPlayer();
        u8 done = g_log.mDone;
        town->mActivity.setDone(jinx.mOutcome, done, me->findInSave());
        clear();
    }
}

// 800C7A5C
void applyTravelResult(int outcome) {
    dSaveTown_c *town = dSaveData_c::getTown();
    dPrivateData_c *me = dPlayerMgr_c::getCurrentPlayer();
    town->mActivity.setDone(outcome, outcome != JINX_OUTCOME_NUM, me->findInSave());
}

// 800C7AC0
void backupTownBits() {
    dSaveTown_c *town = dSaveData_c::getTown();
    g_log.setTownBits(town->mActivity.mToday.mOutcomes);
}

// 800C7AF8
void Log_c::setTownBits(u16 bits) {
    mTownBits = bits;
}

// 800C7B00
u16 getTownBits() {
    return g_log.mTownBits;
}

// 800C7B10
void restoreTownBits() {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mActivity.setTodayOutcomes(getTownBits());
}

} // namespace dActivityLog

#include "d_activity_log_boot.inc"
