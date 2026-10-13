// Town (land) IDs and the town-name script word. .text 8011660C..80116900.
// Matching.
#include <game/game/d_land.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/cLib/c_math.hpp>
#include <cstring>
#include <game/game/d_net.hpp>
#include <game/game/d_wifi.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
}

// 8011660C
u16 dLandID_c::makeRandomId() {
    return (u16)cM::rndInt(0x7FFF) | 0x8000;
}

// 80116638
void dLandID_c::set(const wchar_t *name, u16 id, int region) {
    clear();
    memcpy(mName, name, sizeof(mName));
    mId = id;
    mRegion = region;
}

// 801166A0
void dLandID_c::setRandomId(const wchar_t *name, int region) {
    set(name, makeRandomId(), region);
}

// 801166FC
void dLandID_c::setName(const wchar_t *name, int region) {
    set(name, 0xFFFF, region);
}

// 80116710
void dLandID_c::clear() {
    memset(mName, 0, sizeof(mName));
    mId = 0;
    mRegion = LANGUAGE_NUM;
}

// 80116758
BOOL dLandID_c::isValid() const {
    return mId != 0;
}

// 8011676C
void dLandID_c::copy(const dLandID_c *other) {
    memcpy(this, other, sizeof(dLandID_c));
}

// 80116774
void dLandID_c::setWord(dScript::Word_c *word) {
    word->set(mName, 0);
}

// 80116788
dLandNameWord_c::dLandNameWord_c() {
    clear();
}

// 801167CC
dLandNameWord_c::~dLandNameWord_c() {}

// 80116824
u32 dLandNameWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011682C
wchar_t *dLandNameWord_c::getBuffer() {
    return mBuffer;
}

// 80116834: called on this town's ID
BOOL dLandID_c::isNewTown(const dLandID_c *other) const {
    if (*this == *other) {
        return FALSE;
    }
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && dWifi::findFriend(other, player) >= 0) {
        return FALSE;
    }
    return TRUE;
}
