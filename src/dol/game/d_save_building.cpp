// The town's buildings in the town save (dSaveBuilding_c, dSaveTown_c+0x5E260): the town flag, the gate
// and house looks, and the building positions (dSaveBuildingList_c), with the search candidates that pick
// a new building's spot. .text 80149B70..8014BD88.
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/game/d_field_info.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_crc.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_8006CD4C(int *blockX, int *blockZ, int unitX, int unitZ); // unit -> acre
void *fn_801683D8(); // the town's building registry
void *fn_801683E4(); // the city's
void fn_801679A8(void *registry, u16 id);
void fn_80167BAC(void *registry);
}

static inline bool isBuildingId(u16 id) {
    return id >= BUILDING_BUILD_SITE && id < BUILDING_END;
}

// The vtables come out in the reverse order of the class definitions.
struct dPlHsFlags_c {
    u32 mNeed;
    u32 mAvoid;
};

// A free build site for a player's house: in an acre with the mNeed block flags (if any) and none of
// the mAvoid ones.
class plHsCand_c : public dFdGutSearchCand_c {
public:
    plHsCand_c(dFdBase_c *fd, const dPlHsFlags_c &flags)
        : dFdGutSearchCand_c(fd->mUnitW, fd->mUnitH), mNeed(flags.mNeed), mAvoid(flags.mAvoid) {
        mInfo = fn_80190C44(FD_ID_TOWN);
    }
    virtual BOOL check(int x, int z);

    /* 0x640 */ u32 mNeed;
    /* 0x644 */ u32 mAvoid;
}; // size 0x648

// A free build site: in the field, a build site and no building on it.
class reserveCand_c : public dFdGutSearchCand_c {
public:
    reserveCand_c(int width, int height) : dFdGutSearchCand_c(width, height) {
        mInfo = fn_80190C44(FD_ID_TOWN);
    }
    reserveCand_c(dFdBase_c *fd) : dFdGutSearchCand_c(fd->mUnitW, fd->mUnitH) {
        mInfo = fn_80190C44(FD_ID_TOWN);
    }
    virtual BOOL check(int x, int z);

    BOOL isFree(int x, int z) const {
        int blockX, blockZ;
        fn_8006CD4C(&blockX, &blockZ, x, z);
        if (mInfo->getBlock(blockX, blockZ) != NULL && dSaveBuildingList_c::get()->isBuildSite(x, z)) {
            if (isBuildingId(dSaveBuildingList_c::get()->getAt(x, z, 1).mId)) {
                return FALSE;
            }
            return TRUE;
        }
        return FALSE;
    }

    // The same; its own copy only for the stack layout of check().
    BOOL isFreeSite(int x, int z) const {
        int blockZ, blockX;
        fn_8006CD4C(&blockX, &blockZ, x, z);
        if (mInfo->getBlock(blockX, blockZ) != NULL && dSaveBuildingList_c::get()->isBuildSite(x, z)) {
            if (isBuildingId(dSaveBuildingList_c::get()->getAt(x, z, 1).mId)) {
                return FALSE;
            }
            return TRUE;
        }
        return FALSE;
    }
};

// The lighthouse / windmill spot (BG_ATTR_LIGHTHOUSE).
class lightHouseCand_c : public reserveCand_c {
public:
    lightHouseCand_c(dFdBase_c *fd) : reserveCand_c(fd->mUnitW, fd->mUnitH) {
        mInfo = fn_80190C44(FD_ID_TOWN);
    }
    virtual BOOL check(int x, int z);
};

// The UFO's spot (BG_ATTR_UFO).
class ufoCand_c : public reserveCand_c {
public:
    ufoCand_c(dFdBase_c *fd) : reserveCand_c(fd->mUnitW, fd->mUnitH) {
        mInfo = fn_80190C44(FD_ID_TOWN);
    }
    virtual BOOL check(int x, int z);
};

// A free build site in the acre of a player's house (mPlayers: a bit per player).
class insidePlHsBkCand_c : public reserveCand_c {
public:
    insidePlHsBkCand_c(int width, int height) : reserveCand_c(width, height) {
        mInfo = fn_80190C44(FD_ID_TOWN);
        mPlayers = 0;
    }
    virtual BOOL check(int x, int z);

    /* 0x640 */ u32 mPlayers;
}; // size 0x644

// A free build site in (mAdjacent) or away from (!mAdjacent) the acre of the first building
// in [mFirst, mLast) or one next to it.
class nearBkCand_c : public reserveCand_c {
public:
    nearBkCand_c(int width, int height, u16 first, u16 last, BOOL adjacent)
        : reserveCand_c(width, height), mFirst(dItem::ITEM_ID_NONE), mLast(dItem::ITEM_ID_NONE) {
        mFd = fn_80190C44(FD_ID_TOWN);
        mFirst = first;
        mLast = last;
        mAdjacent = adjacent;
    }
    virtual BOOL check(int x, int z);

    /* 0x640 */ dFdBase_c *mFd;
    /* 0x644 */ u16 mFirst;
    /* 0x646 */ u16 mLast;
    /* 0x648 */ u8 mAdjacent;
}; // size 0x64C


// Unknown 8-byte object built by the static initializer.
class dSaveBuildingUnk_c {
public:
    dSaveBuildingUnk_c(); // 8014B694

    /* 0x0 */ u32 _0;
    /* 0x4 */ u32 _4;
};

static dSaveBuildingUnk_c sUnk;

// Which side of the first villager house the next one goes (alternates).
static bool sNearSide = true;

// 80149B70
int dSaveBuildingList_c::getRangeIndex(int idx, int first, int last) {
    if (idx >= first && idx <= last) {
        return idx - first;
    }
    return -1;
}

// 80149B90
BOOL dSaveBuildingList_c::isPlayerHouse(int idx) {
    return getPlayerHouseNo(idx) != -1;
}

// 80149BC0
int dSaveBuildingList_c::getPlayerHouseNo(int idx) {
    return getRangeIndex(idx, 1, 4);
}

// 80149BCC
BOOL dSaveBuildingList_c::isNpcHouse(int idx) {
    return getNpcHouseNo(idx) != -1;
}

// 80149BFC
int dSaveBuildingList_c::getNpcHouseNo(int idx) {
    return getRangeIndex(idx, 9, 0x12);
}

// 80149C08
void dSaveBuilding_c::clear() {
    mList.clear();
    setGateType();
    mHouseVariant = (u32)cM::rndF(5.0f);
}

// 80149C50
void dSaveBuilding_c::setGateType() {
    mGateType = (u32)cM::rndF(3.0f);
}

// 80149C88
void dSaveBuilding_c::initTownFlag() {
    mTownFlag.setFromItem(0x9DC);
    mTownFlag.loadTextureC();
    dPersonalID_c creator = mTownFlag.mCreator;
    dSaveData_c *raw = dSaveData_c::getRaw();
    creator.land.copy(&raw->mLandID);
    mTownFlag.mCreator = creator;
}

// 80149E38
void dSaveBuildingList_c::init() {
    for (dSaveBuildingPos_c *pos = mPos; pos < mSites + BUILD_SITE_NUM; pos++) {
        pos->mX = 0;
        pos->mZ = 0;
    }
    for (dSaveBuildingPos_c *pos = mSites; pos < mSites + BUILD_SITE_NUM; pos++) {
        pos->mX = 0;
        pos->mZ = 0;
    }
}

// 80149FB4
void dSaveBuildingList_c::clear() {
    init();
    mChecksum = calcChecksum();
}

// 80149FEC
u16 dSaveBuildingList_c::getNpcHouseId(u32 no) {
    u16 id = BUILDING_NPC_HOUSE_0;
    if (no < 10) {
        id = BUILDING_NPC_HOUSE_0 + no;
    }
    return id;
}

// 8014A010
u16 dSaveBuildingList_c::getPlayerHouseId(u32 no) {
    u16 id = BUILDING_PLAYER_HOUSE_0;
    if (no < PLAYER_NUM) {
        id = BUILDING_PLAYER_HOUSE_0 + no;
    }
    return id;
}

// 8014A034
BOOL dSaveBuildingList_c::getPlayerHousePos(int *x, int *z, int no) const {
    dItem::Item id(getPlayerHouseId(no));
    return getPos(x, z, &id, 1);
}

// 8014A098
BOOL dSaveBuildingList_c::setNpcHouse(u32 no) {
    if (no < 10) {
        dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
        if (fd == NULL) {
            return FALSE;
        }
        int dimensions[2] = {fd->mUnitW, fd->mUnitH};
        u16 id = getNpcHouseId(no);
        int x, z;
        if (cM::rndInt(10) < 7) {
            insidePlHsBkCand_c cand(dimensions[0], dimensions[1]);
            for (int i = 0; i < PLAYER_NUM; i++) {
                if (dSaveData_c::getTown()->mHomes.getPlayerOfHome(i) >= 0) {
                    cand.mPlayers |= 1 << i;
                }
            }
            cand.clear();
            cand.search();
            if (cand.getRandomXZ(&x, &z)) {
                if (setPos(x, z, id)) {
                    return TRUE;
                }
            }
            insidePlHsBkCand_c cand2(dimensions[0], dimensions[1]);
            cand2.mPlayers = 0xF;
            cand2.clear();
            cand2.search();
            if (cand2.getRandomXZ(&x, &z)) {
                if (setPos(x, z, id)) {
                    return TRUE;
                }
            }
        }
        // The target keeps each nearby-house candidate's address in a register.
        if ((cM::rndInt(32) & 0xF) <= 9) {
            nearBkCand_c cand(dimensions[0], dimensions[1], BUILDING_NPC_HOUSE_0, BUILDING_NPC_HOUSE_9, sNearSide);
            nearBkCand_c *p = &cand;
            p->clear();
            p->search();
            if (p->getRandomXZ(&x, &z)) {
                if (setPos(x, z, id)) {
                    sNearSide = !sNearSide;
                    return TRUE;
                }
            }
        }
        nearBkCand_c cand(dimensions[0], dimensions[1], BUILDING_NPC_HOUSE_0, BUILDING_NPC_HOUSE_9, !sNearSide);
        nearBkCand_c *p = &cand;
        p->clear();
        p->search();
        if (p->getRandomXZ(&x, &z)) {
            if (setPos(x, z, id)) {
                sNearSide = !sNearSide;
                return TRUE;
            }
        }
        if (setAtReserveSpot(dItem::Item(getNpcHouseId(no)))) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8014A4F4
BOOL dSaveBuildingList_c::removeNpcHouse(u32 no) {
    if (no < 10) {
        return remove(dItem::Item(getNpcHouseId(no)));
    }
    return FALSE;
}

// 8014A544
BOOL dSaveBuildingList_c::getNpcHousePos(int *x, int *z, u32 no) const {
    if (no < 10) {
        dItem::Item id(getNpcHouseId(no));
        return getPos(x, z, &id, 1);
    }
    return FALSE;
}

// 8014A5B8
BOOL dSaveBuildingList_c::hasNpcHouse(u32 no) const {
    int x, z;
    return getNpcHousePos(&x, &z, no);
}

// 8014A5E4
BOOL dSaveBuildingList_c::setFountain() {
    dFdBlock_c *block = fn_80190C44(FD_ID_TOWN)->findBlock(BLOCK_KIND_FLAG_GATE);
    if (block != NULL) {
        int bx = block->mBlockX;
        int bz = block->mBlockZ;
        setPos(bx * UT_X_NUM + 7, bz * UT_Z_NUM + 8, dItem::Item((u16)BUILDING_FOUNTAIN));
        return TRUE;
    }
    return FALSE;
}

// 8014A660
BOOL dSaveBuildingList_c::setAtLighthouseSpot(const dItem::Item &id) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return FALSE;
    }
    lightHouseCand_c cand(fd);
    cand.clear();
    cand.search();
    int x, z;
    if (cand.getRandomXZ(&x, &z)) {
        setPos(x, z, id);
        return TRUE;
    }
    return FALSE;
}

// 8014A770
BOOL dSaveBuildingList_c::setAtUfoSpot(const dItem::Item &id) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return FALSE;
    }
    ufoCand_c cand(fd);
    cand.clear();
    cand.search();
    int x, z;
    if (cand.getRandomXZ(&x, &z)) {
        setPos(x, z, id);
        return TRUE;
    }
    return FALSE;
}

// 8014A880
BOOL dSaveBuildingList_c::setAtReserveSpot(const dItem::Item &id) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return FALSE;
    }
    reserveCand_c cand(fd);
    cand.clear();
    cand.search();
    int x, z;
    if (cand.getRandomXZ(&x, &z)) {
        setPos(x, z, id);
        return TRUE;
    }
    return FALSE;
}

// 8014A978
void dSaveBuildingList_c::create() {
    BOOL ok;
    do {
        clear();
        setFromField();
        ok = setPlayerHouses();
        setCityBuildings();
    } while (!ok);
}

// 8014A9D4
void dSaveBuildingList_c::setCityBuildings() {
    setPos(0x12, 6, dItem::Item((u16)BUILDING_CITY_HAPPY_ROOM), FALSE);
    setPos(0xC, 0xB, dItem::Item((u16)BUILDING_CITY_THEATER), FALSE);
    setPos(3, 0x13, dItem::Item((u16)BUILDING_CITY_BROKER), FALSE);
    setPos(0x24, 0x13, dItem::Item((u16)BUILDING_CITY_BARBER), FALSE);
    setPos(0x2C, 0x12, dItem::Item((u16)BUILDING_CITY_FORTUNE), FALSE);
    setPos(0x17, 4, dItem::Item((u16)BUILDING_CITY_GRACE), FALSE);
    setPos(0x1E, 7, dItem::Item((u16)BUILDING_CITY_AUCTION), FALSE);
    setPos(0xB, 0xC, dItem::Item((u16)BUILDING_CITY_ATM), FALSE);
    setPos(0x21, 0xC, dItem::Item((u16)BUILDING_CITY_SHOE_SHINE), FALSE);
    setPos(0x24, 0xE, dItem::Item((u16)BUILDING_CITY_ETC_0), FALSE);
    setPos(0x28, 0x13, dItem::Item((u16)BUILDING_CITY_ETC_1), FALSE);
    setPos(7, 0x13, dItem::Item((u16)BUILDING_CITY_ETC_2), FALSE);
    setPos(0xA, 0x13, dItem::Item((u16)BUILDING_CITY_ETC_3), FALSE);
    setPos(0xB, 0xF, dItem::Item((u16)BUILDING_CITY_ETC_4), FALSE);
    setPos(0x17, 0xE, dItem::Item((u16)BUILDING_CITY_FOUNTAIN), FALSE);
    setPos(0x16, 0x18, dItem::Item((u16)BUILDING_CITY_BUS_STOP), FALSE);
    setPos(0x2F, 0x1B, dItem::Item((u16)BUILDING_CITY_RESET_CENTER), FALSE);
    setPos(8, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_A), FALSE);
    setPos(0xC, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_B), FALSE);
    setPos(0x14, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_C), FALSE);
    setPos(0xF, 0x12, dItem::Item((u16)BUILDING_CITY_LAMP_D), FALSE);
    setPos(0x10, 0xD, dItem::Item((u16)BUILDING_CITY_LAMP_E), FALSE);
    setPos(0x14, 0xA, dItem::Item((u16)BUILDING_CITY_LAMP_F), FALSE);
    setPos(0x1B, 0xA, dItem::Item((u16)BUILDING_CITY_LAMP_G), FALSE);
    setPos(0x1F, 0xD, dItem::Item((u16)BUILDING_CITY_LAMP_H), FALSE);
    setPos(0x20, 0x12, dItem::Item((u16)BUILDING_CITY_LAMP_I), FALSE);
    setPos(0x1B, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_J), FALSE);
    setPos(0x23, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_K), FALSE);
    setPos(0x28, 0x16, dItem::Item((u16)BUILDING_CITY_LAMP_L), FALSE);
    mChecksum = calcChecksum();
}

// The four player houses: one away from the river, the cliffs and the beach, one by the river, one by
// a cliff, one on the beach.
static const dPlHsFlags_c sPlHsFlags[PLAYER_NUM] = {
    {0, BLOCK_KIND_FLAG_RAMP | BLOCK_KIND_FLAG_CLIFF | BLOCK_KIND_FLAG_RIVER | BLOCK_KIND_FLAG_BEACH},
    {BLOCK_KIND_FLAG_RIVER, BLOCK_KIND_FLAG_RAMP | BLOCK_KIND_FLAG_CLIFF | BLOCK_KIND_FLAG_BEACH},
    {BLOCK_KIND_FLAG_CLIFF, BLOCK_KIND_FLAG_RAMP | BLOCK_KIND_FLAG_RIVER | BLOCK_KIND_FLAG_BEACH},
    {BLOCK_KIND_FLAG_BEACH, BLOCK_KIND_FLAG_RIVER},
};

// 8014ADB0
BOOL dSaveBuildingList_c::setPlayerHouses() {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return FALSE;
    }
    u32 start = cM::rndInt(PLAYER_NUM);
    u32 j;
    for (u32 i = 0; i < PLAYER_NUM; i++) {
        j = i + start;
        int x, z;
        BOOL done = FALSE;
        for (; !done; j++) {
            plHsCand_c cand(fd, sPlHsFlags[j & 3]);
            cand.clear();
            cand.search();
            if (cand.getRandomXZ(&x, &z)) {
                setPos(x, z, dItem::Item((u16)(BUILDING_PLAYER_HOUSE_0 + i)));
                done = TRUE;
            }
        }
    }
    return TRUE;
}

// 8014AEF4
void dSaveBuildingList_c::setFromField() {
    static dItem::Item sNone;
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    for (int z = 0; z < fd->mUnitH; z++) {
        for (int x = 0; x < fd->mUnitW; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && item->isExtId()) {
                if (!item->getExtFlag()) {
                    setPos(x, z, *item);
                }
                if (item->mId == BUILDING_BUILD_SITE) {
                    addBuildSite(x, z, FALSE);
                }
            }
        }
    }
    mChecksum = calcChecksum();
}

// 8014B034
dItem::Item dSaveBuildingList_c::getAt(int x, int z, int mask) const {
    for (u32 i = 0; i < BUILDING_NUM; i++) {
        u16 id = getId(i);
        dItem::Item item(id);
        int bx, bz;
        if (getPos(&bx, &bz, &item, mask) && bx == x && bz == z) {
            return id;
        }
    }
    return dItem::Item();
}

// 8014B0F0
BOOL dSaveBuildingList_c::getPos(int *x, int *z, const dItem::Item *id, int mask) const {
    if (id->isExtId()) {
        int idx = id->getExtIndex();
        if (mask & fn_80167FB4(idx)->mField) {
            const dSaveBuildingPos_c *pos = &mPos[idx];
            if (pos->mX != 0 && pos->mZ != 0) {
                *x = pos->mX;
                *z = pos->mZ;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8014B1C0
BOOL dSaveBuildingList_c::setPos(int x, int z, dItem::Item id, BOOL updateCrc) {
    int idx = id.getExtIndex();
    if ((u32)idx < BUILDING_NUM) {
        int field = fn_80167FB4(idx)->mField;
        dItem::Item item = getAt(x, z, field);
        if (item.mId == dItem::ITEM_ID_NONE || item.mId == id.mId) {
            mPos[idx].mX = x;
            mPos[idx].mZ = z;
            if (field == 1) {
                fn_801679A8(fn_801683D8(), id.mId);
            } else {
                fn_801679A8(fn_801683E4(), id.mId);
            }
            if (updateCrc) {
                mChecksum = calcChecksum();
            }
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 8014B2CC
BOOL dSaveBuildingList_c::setPos(int x, int z, dItem::Item id) {
    return setPos(x, z, id, TRUE);
}

// 8014B2FC
BOOL dSaveBuildingList_c::remove(dItem::Item id) {
    int idx = id.getExtIndex();
    if ((u32)idx < BUILDING_NUM) {
        int field = fn_80167FB4(idx)->mField;
        dSaveBuildingPos_c *pos = &mPos[idx];
        if (pos->mX != 0 || pos->mZ != 0) {
            pos->mX = 0;
            pos->mZ = 0;
            if (field == 1) {
                fn_80167BAC(fn_801683D8());
            } else {
                fn_80167BAC(fn_801683E4());
            }
            mChecksum = calcChecksum();
            return TRUE;
        }
    }
    return FALSE;
}

// 8014B3CC
u32 dSaveBuildingList_c::calcChecksum() const {
    return sCrc::calcCRC32(mPos, sizeof(mPos) + sizeof(mSites), 0x12141018, -1);
}

// 8014B3E4
BOOL dSaveBuildingList_c::addBuildSite(int x, int z, BOOL updateCrc) {
    for (dSaveBuildingPos_c *pos = mSites; pos < mSites + BUILD_SITE_NUM; pos++) {
        if (pos->mX == 0 && pos->mZ == 0) {
            pos->mX = x;
            pos->mZ = z;
            if (updateCrc) {
                mChecksum = calcChecksum();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 8014B474
BOOL dSaveBuildingList_c::isBuildSite(int x, int z) const {
    for (const dSaveBuildingPos_c *pos = mSites; pos < mSites + BUILD_SITE_NUM; pos++) {
        if (x != 0 && z != 0 && pos->mX == (s8)x && pos->mZ == (s8)z) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8014B4E4
BOOL dSaveBuildingList_c::getBuildSite(int *x, int *z, u32 n) const {
    u32 i = 0;
    for (const dSaveBuildingPos_c *pos = mSites; pos < mSites + BUILD_SITE_NUM; i++, pos++) {
        if (i == n) {
            *x = pos->mX;
            *z = pos->mZ;
            return *x != 0 && *z != 0;
        }
    }
    return FALSE;
}

// 8014B55C
int dSaveBuildingList_c::countHouses(int blockX, int blockZ) const {
    int count = 0;
    int x, z;
    for (u32 i = BUILDING_PLAYER_HOUSE_0 - BUILDING_BUILD_SITE; i <= BUILDING_PLAYER_HOUSE_3 - BUILDING_BUILD_SITE; i++) {
        dItem::Item id(getId(i));
        if (getPos(&x, &z, &id, 1)) {
            int bx, bz;
            fn_8006CD4C(&bx, &bz, x, z);
            if (bx == blockX && bz == blockZ) {
                count++;
            }
        }
    }
    for (u32 i = BUILDING_NPC_HOUSE_0 - BUILDING_BUILD_SITE; i <= BUILDING_NPC_HOUSE_9 - BUILDING_BUILD_SITE; i++) {
        dItem::Item id(getId(i));
        if (getPos(&x, &z, &id, 1)) {
            int bx, bz;
            fn_8006CD4C(&bx, &bz, x, z);
            if (bx == blockX && bz == blockZ) {
                count++;
            }
        }
    }
    return count;
}

// 8014B694
dSaveBuildingUnk_c::dSaveBuildingUnk_c() {
    _0 = 0;
}

// 8014B6A0
dSaveBuilding_c *dSaveBuilding_c::get() {
    return &dSaveData_c::getTown()->mBuilding;
}

// 8014B6C8
dSaveBuildingList_c *dSaveBuildingList_c::get() {
    return &dSaveData_c::getTown()->mBuilding.mList;
}

// 8014B6F0
BOOL nearBkCand_c::check(int x, int z) {
    if (isFree(x, z)) {
        int blockX, blockZ;
        fn_8006CD4C(&blockX, &blockZ, x, z);
        if (mFd->getBlock(blockX, blockZ) != NULL) {
            for (u16 id = mFirst; id < mLast; id++) {
                dItem::Item item(id);
                int hx, hz;
                if (dSaveBuildingList_c::get()->getPos(&hx, &hz, &item, 1)) {
                    int hbx, hbz;
                    fn_8006CD4C(&hbx, &hbz, hx, hz);
                    BOOL adjacent = (blockX == hbx && blockZ == hbz) || (blockX == hbx && blockZ == hbz + 1) ||
                                    (blockX == hbx && blockZ == hbz - 1) || (blockX == hbx + 1 && blockZ == hbz) ||
                                    (blockX == hbx - 1 && blockZ == hbz);
                    return mAdjacent == adjacent;
                }
            }
        }
    }
    return FALSE;
}

// 8014B980
BOOL reserveCand_c::check(int x, int z) {
    return isFreeSite(x, z);
}

// 8014BA58
BOOL insidePlHsBkCand_c::check(int x, int z) {
    if (isFree(x, z)) {
        int blockX, blockZ;
        fn_8006CD4C(&blockX, &blockZ, x, z);
        dFdBlock_c *block = mInfo->getBlock(blockX, blockZ);
        u32 i = 0;
        for (u32 id = BUILDING_PLAYER_HOUSE_0; id <= BUILDING_PLAYER_HOUSE_3; id++, i++) {
            if ((mPlayers >> i) & 1) {
                dItem::Item item((u16)id);
                int hx, hz;
                if (dSaveBuildingList_c::get()->getPos(&hx, &hz, &item, 1)) {
                    int hbx, hbz;
                    fn_8006CD4C(&hbx, &hbz, hx, hz);
                    if (blockX == hbx && blockZ == hbz) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

// 8014BBEC
BOOL ufoCand_c::check(int x, int z) {
    if (mInfo != NULL && mInfo->getBgAttr(x, z) == BG_ATTR_UFO) {
        return TRUE;
    }
    return FALSE;
}

// 8014BC2C
BOOL lightHouseCand_c::check(int x, int z) {
    if (mInfo != NULL && mInfo->getBgAttr(x, z) == BG_ATTR_LIGHTHOUSE) {
        return TRUE;
    }
    return FALSE;
}

// 8014BC6C
BOOL plHsCand_c::check(int x, int z) {
    int blockX, blockZ;
    fn_8006CD4C(&blockX, &blockZ, x, z);
    dFdBlock_c *block = mInfo->getBlock(blockX, blockZ);
    if (block != NULL && (block->hasFlag(mNeed) || mNeed == 0) && !block->hasFlag(mAvoid) &&
        dSaveBuildingList_c::get()->isBuildSite(x, z)) {
        dItem::Item item = dSaveBuildingList_c::get()->getAt(x, z, 1);
        if (isBuildingId(item.mId)) {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}
