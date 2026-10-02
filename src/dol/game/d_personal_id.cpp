// Player and personal IDs. .text 8013E618..8013EBCC.
// First pass: every function is written for equivalence; matching has not started.
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>
#include <cstring>
#include <game/game/d_player_mgr.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
void fn_80116710(dLandID_c *land); // clear
BOOL fn_80116758(const dLandID_c *land); // isValid
int fn_801017EC(const dPersonalID_c *pid);
int fn_801018DC(const dPlayerID_c *id);
BOOL fn_800DCEDC();
u32 fn_800DCF30();
int fn_8017BF90(const dPersonalID_c *pid, dPrivateData_c *player);
int fn_8017C04C(const dPlayerID_c *id, dPrivateData_c *player);
}

// 80750BF0
const u16 dPlayerID_c::ID_UNSET = 0xFFFF;

// 8013E618
void dPersonalID_c::setPlayer(const wchar_t *name, u16 id, u8 gender) {
    player.set(name, id, gender);
}

// 8013E620
u16 dPersonalID_c::generateId(const u16 *used, int num) {
    u16 id = randomId();
    while (id == 0 || containsId(id, used, num) == TRUE) {
        id = randomId();
    }
    return id;
}

// 8013E690
bool dPersonalID_c::containsId(u16 id, const u16 *ids, int num) {
    bool found = false;
    if (num != 0 && ids != NULL) {
        for (int i = 0; i < num; i++, ids++) {
            if (id == *ids) {
                found = true;
                break;
            }
        }
    }
    return found;
}

// 8013E6D0
u16 dPersonalID_c::randomId() {
    return (u16)cM::rndInt(0x7FFF) | 0x8000;
}

// 8013E6FC
void dPersonalID_c::clear() {
    player.clear();
    fn_80116710(&land);
}

// 8013E734
BOOL dPersonalID_c::isSamePlayer(const dPersonalID_c *other) const {
    return player.isSame(&other->player);
}

// 8013E740
BOOL dPersonalID_c::fn_8013E740(const dPersonalID_c *other) const {
    BOOL same = FALSE;
    if (land == other->land && isSamePlayer(other)) {
        same = TRUE;
    }
    if (same) {
        return FALSE;
    }

    if (dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, other) > 0) {
        return FALSE;
    }

    int n = fn_801017EC(other);
    if (fn_800DCEDC() && fn_800DCF30() > 1 && n < 7) {
        return FALSE;
    }

    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current != NULL && fn_8017BF90(other, current) >= 0) {
        return FALSE;
    }
    return TRUE;
}

// 8013E860
void dPersonalID_c::setPlayerName(const wchar_t *name) {
    player.setName(name);
}

// 8013E868
BOOL dPersonalID_c::isValid() const {
    BOOL valid = FALSE;
    if (fn_80116758(&land) && player.isValid()) {
        valid = TRUE;
    }
    return valid;
}

// 8013E8C0
void dPersonalID_c::copy(const dPersonalID_c *other) {
    memcpy(this, other, sizeof(dPersonalID_c));
}

// 8013E8C8
void dPersonalID_c::setWord(dScript::Word_c *word) const {
    player.setWord(word);
}

// 8013E8D0
BOOL dPersonalID_c::isLand(const dLandID_c *other) const {
    return land == *other;
}

// 8013E938
BOOL dPersonalID_c::isFromTown() const {
    const dSaveData_c* save_p = dSaveData_c::getRaw();
    return isLand(&save_p->mLandID);
}

// 8013E978
void dPlayerID_c::set(const wchar_t *name, u16 id, u8 gender) {
    memcpy(mName, name, sizeof(mName));
    mGender = gender;
    mId = id;
}

// 8013E9CC
void dPlayerID_c::clear() {
    memset(mName, 0, sizeof(mName));
    mId = 0;
    mGender = GENDER_OTHER;
}

// 8013EA14
BOOL dPlayerID_c::isValid() const {
    return mId != 0;
}

// 8013EA28
void dPlayerID_c::copy(const dPlayerID_c *other) {
    memcpy(this, other, sizeof(dPlayerID_c));
}

// 8013EA30
BOOL dPlayerID_c::isSame(const dPlayerID_c *other) const {
    BOOL same = FALSE;
    if (mId == other->mId && mGender == other->mGender && memcmp(mName, other->mName, sizeof(mName)) == 0) {
        same = TRUE;
    }
    return same;
}

// 8013EA98
void dPlayerID_c::setName(const wchar_t *name) {
    memcpy(mName, name, sizeof(mName));
}

// 8013EAA4
void dPlayerID_c::setWord(dScript::Word_c *word) const {
    word->set(mName, 0);

    dScript::Inflect_c& inflect = word->mInflect;
    switch (mGender) {
    case GENDER_MALE:
        inflect.setGender(GENDER_MALE);
        break;
    case GENDER_FEMALE:
        inflect.setGender(GENDER_FEMALE);
        break;
    default:
        inflect.setGender(GENDER_OTHER);
        break;
    }
}

// 8013EB28
BOOL dPlayerID_c::fn_8013EB28(const dPlayerID_c *other) const {
    if (isSame(other)) {
        return FALSE;
    }

    int n = fn_801018DC(other);
    if (fn_800DCEDC() && fn_800DCF30() > 1 && n < 7) {
        return FALSE;
    }

    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current != NULL && fn_8017C04C(other, current) >= 0) {
        return FALSE;
    }
    return TRUE;
}
