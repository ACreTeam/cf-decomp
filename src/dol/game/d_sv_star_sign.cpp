// Constellations: a player's and the town's (dSvStarSign::Sign_c / List_c). .text 80150A5C..801510A0.
// See include/game/game/d_sv_star_sign.hpp.
#include <game/game/d_sv_star_sign.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_star_draw.hpp>
#include <game/game/d_string.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

namespace dSvStarSign {

// 80150A5C
Sign_c::Sign_c() {}

// 80150A60
Sign_c::~Sign_c() {}

// 80150AA0
Sign_c *Sign_c::copy(const Sign_c *other) {
    memcpy(this, other, sizeof(Sign_c));
    return this;
}

// 80150AD4
void Sign_c::clear() {
    memset(this, 0, sizeof(Sign_c));
    for (int i = 0; i < SIGN_LINE_NUM; i++) {
        mLines[i] = SIGN_LINE_NONE;
    }
    mOwner.clear();
}

// 80150B58
void Sign_c::setName(const wchar_t *name) {
    mName[SIGN_NAME_LEN] = 0;
    memcpy(mName, name, SIGN_NAME_LEN * sizeof(wchar_t));
}

// 80150B6C
const wchar_t *Sign_c::getName() const {
    return mName;
}

// 80150B74
void Sign_c::setLine(int idx, u16 line) {
    mLines[idx] = line;
}

// 80150B84
void Sign_c::clearLine(int idx) {
    setLine(idx, SIGN_LINE_NONE);
}

// 80150B94
List_c::List_c() {}

// 80150BDC
void List_c::clear() {
    for (int i = 0; i < SIGN_NUM; i++) {
        mSigns[i].clear();
    }
}

// 80150C28
void List_c::set(int idx, const Sign_c *sign) {
    u16 bit = 1 << idx;
    mSigns[idx].copy(sign);
    for (int i = 0; i < PLAYER_NUM; i++) {
        mNew[i] |= bit;
    }
}

// 80150CA4
void List_c::remove(int idx) {
    mSigns[idx].clear();
    for (int i = 0; i < PLAYER_NUM; i++) {
        mNew[i] &= ~(u16)(1 << idx);
    }
}

// 80150D24
const Sign_c *List_c::get(int idx) const {
    return &mSigns[idx];
}

// 80150D30
Sign_c *List_c::get(int idx) {
    return &mSigns[idx];
}

// 80150D3C
BOOL List_c::isValid(int idx) const {
    return mSigns[idx].mOwner.isValid();
}

// 80150D48
int List_c::findFree() const {
    for (int i = 0; i < SIGN_NUM; i++) {
        if (!isValid(i)) {
            return i;
        }
    }
    return -1;
}

// 80150DA8
int List_c::getRandom(int exclude) const {
    int num = 0;
    for (int i = 0; i < SIGN_NUM; i++) {
        if (i != exclude && isValid(i)) {
            num++;
        }
    }
    if (num <= 0) {
        return -1;
    }
    int pick = cM::rndInt(num);
    int n = 0;
    for (int i = 0; i < SIGN_NUM; i++) {
        if (i != exclude && isValid(i)) {
            n++;
            if (n >= pick) {
                return i;
            }
        }
    }
    return -1;
}

// 80150E74
void List_c::clearNew(int player) {
    mNew[player] = 0;
}

// 80150E88
void List_c::registerPlayerSign() {
    if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->mStarSign.mOwner.isValid()) {
        return;
    }
    dString::Word_c name;
    name.set(dPlayerMgr::Mgr_c::getCurrentPlayer()->mStarSign.getName(), 0);
    for (int i = 0; i < SIGN_NUM; i++) {
        if (dSaveData_c::getTown()->mStarSigns.isValid(i)) {
            dSaveTown_c *town = dSaveData_c::getTown();
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (dStarDraw_c::isSignLinked(&player->mStarSign, town->mStarSigns.get(i))) {
                return;
            }
            dString::Word_c other;
            other.set(dSaveData_c::getTown()->mStarSigns.get(i)->getName(), 0);
            if (other.isSame(&name)) {
                return;
            }
        }
    }
    for (int i = 0; i < SIGN_NUM; i++) {
        if (!dSaveData_c::getTown()->mStarSigns.isValid(i)) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            dSaveData_c::getTown()->mStarSigns.get(i)->copy(&player->mStarSign);
            for (int p = 0; p < PLAYER_NUM; p++) {
                if (dPlayerMgr::Mgr_c::getPlayer(p)->mPID.isValid()) {
                    u16 *flags = &dSaveData_c::getTown()->mStarSigns.mNew[p];
                    *flags |= 1 << i;
                }
            }
            dPlayerMgr::Mgr_c::getCurrentPlayer()->mStarSign.clear();
            return;
        }
    }
}

} // namespace dSvStarSign
