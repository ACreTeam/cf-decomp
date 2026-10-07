// dSaveTown_c, the town part of the save file (dSaveData_c's base, up to mExtra): its checksum,
// the new-town clear, deleting a player, and the 64 town flags. .text 80115CDC..8011660C.
#include <game/game/d_save_data.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_crc.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_80149C08(void *obj);
void fn_801503E4(void *obj);
void fn_8014EB3C(void *obj);
void fn_80150524(void *obj);
void fn_801505E4(void *obj);
void fn_80150628(void *obj);
void fn_801510A0(void *obj);
void fn_80150BDC(void *obj);
void fn_8010C0A4(void *obj);
void fn_80150E74(void *obj, int player);
void fn_8010CDCC(void *obj, int player);
void fn_80150088(void *obj, int player);
BOOL fn_8014B0F0(void *fg, int *x, int *z, dItem::Item *item, int);
int fn_80177C0C();
}

// 80115CDC
BOOL dSaveTown_c::isHeaderVersionOK() {
    return mHeader.isVersionOK();
}

// 80115CE0
BOOL dSaveTown_c::isHeaderState2() {
    return mHeader.isState2();
}

// 80115CE4
void dSaveTown_c::updateTownChecksum() {
    mHeader.updateChecksum();
    dPrivateData_c::updateChecksumAll(mPlayers);
    mAnimals.updateChecksum();
    mTownChecksum = calcTownChecksum();
}

static inline BOOL checkTownCrc(const dSaveTown_c *save) {
    u32 checksum = save->calcTownChecksum();
    if (checksum == save->mTownChecksum) {
        return TRUE;
    }
    return FALSE;
}

// 80115D34
BOOL dSaveTown_c::isTownChecksumOK(int arg) {
    if (!mHeader.isChecksumOK()) {
        return FALSE;
    }
    if (!dPrivateData_c::isChecksumValidAll(mPlayers, arg)) {
        return FALSE;
    }
    if (!mAnimals.isChecksumValid(arg)) {
        return FALSE;
    }
    if (checkTownCrc(this)) {
        return TRUE;
    }
    return FALSE;
}

// 80115DDC: everything after mTownChecksum up to mExtra
u32 dSaveTown_c::calcTownChecksum() const {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32((const u8 *)&mTownChecksum + sizeof(mTownChecksum),
                           sizeof(dSaveTown_c) - sizeof(mTownChecksum) - ((const u8 *)&mTownChecksum - base), -1, -1);
}

// 80115E04
void dSaveTown_c::clearTown() {
    mSaveTime.reset();
    mMainField.clear();
    fn_80149C08(&_05E260);
    dPrivateData_c::clearAll(mPlayers);
    mHomes.initAll();
    _0636F0.clear();
    mShops.clear();
    mMuseum.init();
    fn_801503E4(&_0683F8);
    mAnimals.clear();
    fn_8014EB3C(&_064078[0x50]);
    fn_80150524(_06673C);
    fn_801505E4(&_066740[1]);
    fn_80150628(&_066740[5]);
    fn_801510A0(_072CC0);
    ((dTimeStamp_c *)_068372)->reset();
    mNoticeBoard.clear();
    fn_80150BDC(&_072E0A);
    mVillageMelody.clear();
    fn_8010C0A4(_063C40);
    mItemVersion = dItem::BITM::getVersion();
    _072CEE.clear();
    setNewConstruction(NEW_CONSTRUCTION_NONE, 0, 0);
}

static inline BOOL isInRange(u16 id, u16 first, u16 last) {
    return id >= first && id <= last;
}

// 80115F44: also removes the player's designs laid out in the town
void dSaveTown_c::deletePlayer(int player) {
    mHomes.initPlayerHome(player);
    mMuseum.deletePlayer(player);
    fn_80150E74(&_072E0A, player);
    dPrivateData_c::clearPlayer(mPlayers, player);
    fn_8010CDCC(_063C40, player);
    mNoticeBoard.clearRead(player);
    fn_80150088(&_05EC64, player);

    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd != NULL) {
        dItem::Item none;
        int x, z;
        int w = (fd->mBlockW - 1) * 16;
        int h = (fd->mBlockH - 1) * 16;
        for (z = 16; z < h; z++) {
            for (x = 16; x < w; x++) {
                dItem::Item *item = fd->getItem(x, z, 0);
                if (item != NULL) {
                    switch (player) {
                    case 0:
                        if (isInRange(item->mId, dItem::FG_DESIGN_P0_0, dItem::FG_DESIGN_P0_7)) {
                            fd->setItem(&none, x, z, 0);
                        }
                        break;
                    case 1:
                        if (isInRange(item->mId, dItem::FG_DESIGN_P1_0, dItem::FG_DESIGN_P1_7)) {
                            fd->setItem(&none, x, z, 0);
                        }
                        break;
                    case 2:
                        if (isInRange(item->mId, dItem::FG_DESIGN_P2_0, dItem::FG_DESIGN_P2_7)) {
                            fd->setItem(&none, x, z, 0);
                        }
                        break;
                    case 3:
                        if (isInRange(item->mId, dItem::FG_DESIGN_P3_0, dItem::FG_DESIGN_P3_7)) {
                            fd->setItem(&none, x, z, 0);
                        }
                        break;
                    }
                }
            }
        }
    }
}

// 80116180: an hour before 2000-01-01, i.e. never saved
void dSaveTown_c::resetSaveTime() {
    dTime_c time;
    time.set(2000, 0, 1, 0, 0, 0);
    time.add(0, 0, 0, -1);
    mSaveTime.set(&time);
}

// The save pointer is fetched before the item.
static inline BOOL findFgItem(dSaveTown_c *save, int *x, int *z, dItem::Item *item) {
    return fn_8014B0F0(&save->_05EB04, x, z, item, 1);
}

// 801161F0: read by dPrivateData_c's "BBS_office" notices
int dSaveTown_c::getPublicWorks() {
    int x, z;
    dItem::Item fountain((u16)BUILDING_FOUNTAIN);
    BOOL hasFountain = findFgItem(dSaveData_c::getRaw(), &x, &z, &fountain);
    dItem::Item windmill((u16)BUILDING_WINDMILL);
    BOOL hasWindmill = findFgItem(dSaveData_c::getRaw(), &x, &z, &windmill);
    dItem::Item lighthouse((u16)BUILDING_LIGHTHOUSE);
    BOOL hasLighthouse = findFgItem(dSaveData_c::getRaw(), &x, &z, &lighthouse);
    dSaveData_c *raw = dSaveData_c::getRaw();
    BOOL hasBridge = FALSE;
    if (raw->mMainField.mBridgeBlockX == -1 && raw->mMainField.mBridgeBlockZ == -1) {
        hasBridge = TRUE;
    }
    if (hasLighthouse || hasWindmill) {
        return PUBLIC_WORKS_TOWER;
    }
    if (hasFountain) {
        return PUBLIC_WORKS_FOUNTAIN;
    }
    return hasBridge != FALSE;
}

// 80116310: a random player whose record has the flag, or -1
int dSaveTown_c::getRandomOtherTown() const {
    int num = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (mOtherTowns[i].mValid) {
            num++;
        }
    }
    if (num <= 0) {
        return -1;
    }
    int pick = cM::rndInt(num) + 1;
    int n = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (mOtherTowns[i].mValid) {
            n++;
            if (n >= pick) {
                return i;
            }
        }
    }
    return -1;
}

// 8011641C: the windmill / lighthouse are only announced the first time
void dSaveTown_c::setNewConstruction(int kind, int blockX, int blockZ) {
    switch (kind) {
    case NEW_CONSTRUCTION_NONE:
    case NEW_CONSTRUCTION_BRIDGE:
    case NEW_CONSTRUCTION_FOUNTAIN:
        break;
    case NEW_CONSTRUCTION_WINDMILL:
        if (isFlag(SAVE_FLAG_WINDMILL_BUILT)) {
            return;
        }
        setFlag(SAVE_FLAG_WINDMILL_BUILT);
        break;
    case NEW_CONSTRUCTION_LIGHTHOUSE:
        if (isFlag(SAVE_FLAG_LIGHTHOUSE_BUILT)) {
            return;
        }
        setFlag(SAVE_FLAG_LIGHTHOUSE_BUILT);
        break;
    }
    mNewConstruction = kind;
    mNewConstructionBlockX = blockX;
    mNewConstructionBlockZ = blockZ;
}

// 801164D0
BOOL dSaveTown_c::isFlag(int idx) const {
    if ((u32)idx < 64) {
        return (mFlags[idx >> 3] & (1 << (idx & 7))) != 0;
    }
    return FALSE;
}

// 80116510
void dSaveTown_c::setFlag(int idx) {
    if ((u32)idx < 64) {
        mFlags[idx >> 3] |= (1 << (idx & 7));
    }
}

// 80116540
void dSaveTown_c::clearFlag(int idx) {
    if ((u32)idx < 64) {
        mFlags[idx >> 3] &= ~(1 << (idx & 7));
    }
}

// 80116570
BOOL dSaveTown_c::isBusNetEventPending() {
    if (fn_80177C0C() == 1 && !dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        return TRUE;
    }
    return FALSE;
}

// 801165B8
void dSaveTown_c::backupFlag6() {
    if (isFlag(6)) {
        setFlag(SAVE_FLAG_FLAG6_BACKUP);
    } else {
        clearFlag(SAVE_FLAG_FLAG6_BACKUP);
    }
}
