// The save's extra block (dSaveExtra_c): its setup, the town rumors and its CRC. .text 80116900..80117118.
// See include/game/game/d_save_data.hpp.
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_extra_header.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/cLib/c_math.hpp>

// 80116900
void dSaveExtra_c::init() {
    mWc24.init();
    mPostBox.clearAll();
    mDistPattern.clear();
    mDesignBoard.clear();
    mDistNgWords.clear();
    mDistContent.clear();
    for (int i = 0; i < PLAYER_NUM; i++) {
        mSavedLetters[i].clear();
        mSavedLetterItems[i].clear();
        mSavedPatterns[i].init();
        mPlayerPostBoxes[i].clear();
    }
    mDistMail.clear();
    mTheater.init();
    for (int i = 0; i < RUMOR_NUM; i++) {
        mRumorPlayers[i].clear();
        mRumorKinds[i] = 0;
    }
    mOutfitLand.clear();
    mOutfit.setDefault();
    getAucListings()->init();
    getAucBids()->init();
    getAucSales()->init();
    mVoiceSetting = 0;
    mVoiceLevel = 3;
    mLangFlags.clearAll();
}

// 80116A68
void dSaveExtra_c::clearDistMailReceived(int player) {
    mDistMail.clearReceived(player);
}

// 80116A74
BOOL dSaveExtra_c::pickRumor(dPersonalID_c *out, u32 *kind, const dPersonalID_c *exclude) const {
    int count = 0;
    for (int i = 0; i < RUMOR_NUM; i++) {
        if (mRumorPlayers[i].isValid() && mRumorKinds[i] != 0) {
            if (exclude == NULL || !(*exclude == mRumorPlayers[i])) {
                count++;
            }
        }
    }
    if (count <= 0) {
        return FALSE;
    }
    int pick = cM::rndInt(count) + 1;
    count = 0;
    for (int i = 0; i < RUMOR_NUM; i++) {
        if (mRumorPlayers[i].isValid() && mRumorKinds[i] != 0) {
            if (exclude == NULL || !(*exclude == mRumorPlayers[i])) {
                count++;
            }
        }
        if (count == pick) {
            *out = mRumorPlayers[i];
            *kind = mRumorKinds[i];
            return TRUE;
        }
    }
    return FALSE;
}

// 80116D08: newest first; a player already there keeps their place
void dSaveExtra_c::addRumor(dPersonalID_c player, int kind) {
    for (int i = 0; i < RUMOR_NUM; i++) {
        if (dSaveData_c::getExtra()->mRumorPlayers[i] == player) {
            dSaveData_c::getExtra()->mRumorPlayers[i] = player;
            dSaveData_c::getExtra()->mRumorKinds[i] = kind;
            return;
        }
    }
    for (int i = RUMOR_NUM - 1; i > 0; i--) {
        dSaveData_c::getExtra()->mRumorPlayers[i] = dSaveData_c::getExtra()->mRumorPlayers[i - 1];
        dSaveData_c::getExtra()->mRumorKinds[i] = *(dSaveData_c::getExtra()->mRumorKinds + i - 1);
    }
    dSaveData_c::getExtra()->mRumorPlayers[0] = player;
    dSaveData_c::getExtra()->mRumorKinds[0] = kind;
}

// 80117044
void dSaveExtra_c::setupNew() {
    mHeader.init();
    mTheater.setup();
}

// 80117078
void dSaveExtra_c::updateChecksum() {
    mHeader.updateChecksum();
    mChecksum = calcChecksum();
}

static inline BOOL checkCrc(const dSaveExtra_c *extra) {
    u32 checksum = extra->calcChecksum();
    if (checksum == extra->mChecksum) {
        return TRUE;
    }
    return FALSE;
}

// 801170B0
BOOL dSaveExtra_c::isChecksumOK(int) {
    if (checkCrc(this)) {
        return TRUE;
    }
    return FALSE;
}

// 801170F4: everything after mChecksum
u32 dSaveExtra_c::calcChecksum() const {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32((const u8 *)&mChecksum + sizeof(mChecksum),
                           sizeof(dSaveExtra_c) - sizeof(mChecksum) - ((const u8 *)&mChecksum - base), -1, -1);
}
