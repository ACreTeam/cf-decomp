// This town's news for its friends' towns (dSvVillagerMove::Info_c). .text 8014EB3C..8014F284.
// See include/game/game/d_sv_villager_move.hpp.
#include <game/game/d_sv_villager_move.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <string.h>

namespace dSvVillagerMove {

// 8014EB3C
void Info_c::init() {
    memset(this, 0, sizeof(Info_c));
    mVersion = VILLAGER_MOVE_VERSION;
    mOtherTown._103 = ASSESSMENT_INITIAL;
    mOtherTown.mLand.clear();
    mOtherTown.mPlayer.clear();
    mOtherTown.mVillager.clear();
    mOtherTown.mImpression = IMPRESSION_INITIAL;
    for (int i = 0; i < 4; i++) {
        mPlayerValue[i] = 0;
    }
    mCatherinePlayer = CATHERINE_PLAYER_NONE;
    mCatherineCategory = -1;
    mCatherineSub = -1;
    for (int i = 0; i < 4; i++) {
        mPlayerCategory[i] = -1;
        mPlayerSub[i] = -1;
    }
}

// 8014EBE8
void Info_c::clearOutfitFlag() {
    mFlags &= ~VILLAGER_MOVE_FLAG_OUTFIT;
}

// 8014EBF8
void Info_c::setCatherine(int player, u32 category, u32 sub) {
    if (player >= 0 && player < PLAYER_NUM && category < CATHERINE_CATEGORY_NUM && sub < CATHERINE_SUB_NUM) {
        mCatherinePlayer = player;
        mCatherineCategory = category;
        mCatherineSub = sub;
        mPlayerCategory[player] = category;
        mPlayerSub[player] = sub;
        mFlags |= VILLAGER_MOVE_FLAG_CATHERINE;
    }
}

// 8014EC40
void Info_c::setPlayerBit(int player, BOOL on) {
    if (on) {
        mPlayerBits |= 1 << player;
    } else {
        mPlayerBits &= ~(1 << player);
    }
    mFlags |= VILLAGER_MOVE_FLAG_PLAYER_BITS;
}

// 8014EC84
BOOL Info_c::isPlayerBit(int player) const {
    return (mPlayerBits & (1 << player)) != 0;
}

// 8014ECA4
void Info_c::setOutfit(const dOutfit_c *outfit) {
    mOutfit.copy(outfit);
    mFlags |= VILLAGER_MOVE_FLAG_OUTFIT;
}

// 8014ECE0
BOOL Info_c::getOutfit(dOutfit_c *outfit) const {
    if (mOutfit.isValid()) {
        outfit->copy(&mOutfit);
        return TRUE;
    }
    return FALSE;
}

// 8014ED24
void Info_c::setPlayerValue(int player, u8 value) {
    mPlayerValue[player] = value;
    mFlags |= VILLAGER_MOVE_FLAG_PLAYER_VALUE;
    fn_80116D08(dSaveData_c::getExtra(), dPlayerMgr_c::getPlayer(player)->mPID, value);
}

// 8014EE28
int Info_c::getPlayerValue(int player) const {
    if (player >= PLAYER_NUM) {
        return 0;
    }
    int value = mPlayerValue[player];
    if (value >= PLAYER_VALUE_NUM) {
        value = 0;
    }
    return value;
}

// 8014EE50
void Info_c::setOtherTown(const dPersonalID_c *player, const dAnmPersonalID_c *villager, u8 impression) {
    dSaveData_c *save = dSaveData_c::getRaw();
    mOtherTown.mLand.copy(&save->mLandID);
    mOtherTown.mPlayer.copy(player);
    mOtherTown.mVillager = *villager;
    mOtherTown.mImpression = impression;
    mOtherTown.mValid = TRUE;
    mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
}

// 8014EFC0
void Info_c::setAssessment(u8 assessment) {
    const dLandID_c *land = &dSaveData_c::getRaw()->mLandID;
    if (mOtherTown._103 != assessment) {
        mOtherTown.mLand.copy(land);
        mOtherTown._103 = assessment;
        mOtherTown.mValid = TRUE;
        mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    }
}

// 8014F030
void Info_c::setShopStage(u8 stage) {
    const dLandID_c *land = &dSaveData_c::getRaw()->mLandID;
    if (mOtherTown.mShopStage != stage) {
        mOtherTown.mLand.copy(land);
        mOtherTown.mShopStage = stage;
        mOtherTown.mValid = TRUE;
        mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    }
}

// 8014F0A4
void Info_c::setBridgeBuilt() {
    dSaveData_c *save = dSaveData_c::getRaw();
    mOtherTown.mLand.copy(&save->mLandID);
    mOtherTown.mHasBridge = TRUE;
    mOtherTown.mHasFountain = FALSE;
    mOtherTown.mHasWindmill = FALSE;
    mOtherTown.mHasLighthouse = FALSE;
    mOtherTown.mValid = TRUE;
    mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    mPublicWorksDays = PUBLIC_WORKS_NEWS_DAYS;
}

// 8014F10C
void Info_c::setFountainBuilt() {
    dSaveData_c *save = dSaveData_c::getRaw();
    mOtherTown.mLand.copy(&save->mLandID);
    mOtherTown.mHasBridge = FALSE;
    mOtherTown.mHasFountain = TRUE;
    mOtherTown.mHasWindmill = FALSE;
    mOtherTown.mHasLighthouse = FALSE;
    mOtherTown.mValid = TRUE;
    mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    mPublicWorksDays = PUBLIC_WORKS_NEWS_DAYS;
}

// 8014F178
void Info_c::setWindmillBuilt() {
    dSaveData_c *save = dSaveData_c::getRaw();
    mOtherTown.mLand.copy(&save->mLandID);
    mOtherTown.mHasBridge = FALSE;
    mOtherTown.mHasFountain = FALSE;
    mOtherTown.mHasWindmill = TRUE;
    mOtherTown.mHasLighthouse = FALSE;
    mOtherTown.mValid = TRUE;
    mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    mPublicWorksDays = PUBLIC_WORKS_NEWS_DAYS;
}

// 8014F1E4
void Info_c::setLighthouseBuilt() {
    dSaveData_c *save = dSaveData_c::getRaw();
    mOtherTown.mLand.copy(&save->mLandID);
    mOtherTown.mHasBridge = FALSE;
    mOtherTown.mHasFountain = FALSE;
    mOtherTown.mHasWindmill = FALSE;
    mOtherTown.mHasLighthouse = TRUE;
    mOtherTown.mValid = TRUE;
    mFlags |= VILLAGER_MOVE_FLAG_OTHER_TOWN;
    mPublicWorksDays = PUBLIC_WORKS_NEWS_DAYS;
}

// 8014F248
void Info_c::decPublicWorksDays(int days) {
    if (days < 0) {
        days = -days;
    }
    if (days >= mPublicWorksDays) {
        mPublicWorksDays = 0;
        mOtherTown.mHasBridge = FALSE;
        mOtherTown.mHasFountain = FALSE;
        mOtherTown.mHasWindmill = FALSE;
        mOtherTown.mHasLighthouse = FALSE;
    } else {
        mPublicWorksDays -= days;
    }
}

} // namespace dSvVillagerMove
