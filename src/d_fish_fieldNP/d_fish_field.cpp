// dFishField_c: the fish in the town (REL d_fish_fieldNP). See include/game/game/d_fish_field.hpp.
#include <game/game/d_fish_field.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <game/game/d_field_info.hpp>
#include <game/game/d_block_kind.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/cLib/c_counter.hpp>
#include <game/sLib/s_lib.hpp>

extern "C" {
BOOL fn_800DCEDC();                            // net play
BOOL fn_800DD960();                            // net: this machine is the host
int fn_800DCF58();                             // net: own member index
void *fn_800DD64C(int id);                     // net: shared record id
void fn_800DD5F8(int id, void *data, int arg); // net: send shared record id
dItem::Item fn_80153410(const void *obj);           // the item at obj + 0x1F8
// The player's fishing float while it is free for fish (or fish is on it), NULL if none.
dFishingFloat_c *fn_801710A4(int player, dFishFldShadow_c *fish);
void fn_801710BC(int player, dFishFldShadow_c *fish); // put fish on the player's float (NULL: off)
BOOL fn_8016FED4(const dFishingFloat_c *fl);          // the float just hit the water
void fn_8016FD60(dFishingFloat_c *fl);                // the float bobs (ripple)
void fn_8016FC44(dFishingFloat_c *fl, int state);
BOOL fn_80190970(dHoldItemMgr_c *mgr, int player, int type, int kind, int a, int b); // hold up an item
void fn_801909D4(dHoldItemMgr_c *mgr, int player);   // stop holding
void fn_8019022C(dHoldItemMgr_c::Mdl_c *mdl, const char *anm);
void fn_80087790(const char *name, const mVec3_c *pos, int arg, const mVec3_c *scale); // effect
void fn_80087844(const char *name, const mVec3_c *pos, int arg, const mVec3_c *scale,
                 void (*cb)(dEffectTarget_c *, u32), u32 kind); // effect with a callback
void fn_80087B40(EGG::Effect *effect, const char *name, const mMtx_c *mtx, int arg);
void fn_80285110(void *obj, dEffectTarget_c *target);
BOOL fn_111_AEA4(int a, void *b, void *c, int d); // d_fgobj_managerNP
void fn_111_6770(u16 item, const mVec3_c *pos, const mVec3_c *scale, const mAng3_c *angle,
                 int arg);                         // d_fgobj_managerNP
}

extern EGG::ExpHeap *lbl_8074E440;
extern nw4r::math::VEC3 lbl_80623FEC; // the camera's target (the view center)

void *dFishField_c_classInit();

fProf::fBaseProfile_c g_profile_FISH_FIELD = {&dFishField_c_classInit, 0xB2, 0x13};

dFishParam_c sFishParam[77] = {
    {0, 2, 1}, {1, 3, 3}, {1, 3, 4}, {2, 3, 3}, {2, 2, 3}, {3, 2, 3}, {3, 2, 3}, {0, 2, 3},
    {0, 2, 3}, {0, 2, 2}, {1, 4, 4}, {1, 4, 3}, {2, 2, 4}, {1, 3, 3}, {3, 2, 4}, {7, 1, 1},
    {4, 2, 1}, {1, 4, 4}, {2, 1, 1}, {3, 2, 2}, {4, 2, 2}, {1, 3, 3}, {2, 2, 1}, {2, 1, 1},
    {2, 1, 1}, {3, 2, 2}, {5, 0, 1}, {3, 2, 1}, {5, 2, 1}, {0, 2, 3}, {0, 2, 2}, {0, 2, 2},
    {1, 4, 3}, {3, 3, 2}, {4, 1, 0}, {5, 3, 1}, {5, 1, 1}, {0, 0, 2}, {1, 2, 4}, {0, 1, 1},
    {0, 2, 1}, {1, 2, 2}, {1, 2, 2}, {5, 2, 1}, {2, 3, 3}, {2, 2, 2}, {1, 3, 3}, {2, 2, 0},
    {4, 3, 2}, {2, 2, 1}, {2, 2, 2}, {3, 1, 1}, {2, 3, 2}, {2, 3, 2}, {4, 2, 2}, {4, 2, 2},
    {3, 3, 3}, {5, 1, 0}, {5, 1, 0}, {5, 2, 2}, {6, 0, 1}, {6, 1, 1}, {6, 1, 0}, {5, 0, 0},
    {1, 3, 3}, {2, 2, 3}, {3, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3},
    {1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {0, 0, 0}, {0, 0, 0},
};

// searchFloat: how far / how wide a fish notices a float (by dFishParam_c::_1).
struct dFishTurnParam_c {
    s16 mAngle;
    f32 mSpeed;
};
dFishTurnParam_c sTurnParam0[5] = {
    {0x1555, 1.2f}, {0x1C72, 1.2f}, {0x238E, 1.2f}, {0x2AAB, 1.3f}, {0x4000, 1.4f},
};
dFishTurnParam_c sTurnParam1[5] = {
    {0x2000, 1.4f}, {0x2AAB, 1.4f}, {0x3555, 1.4f}, {0x4000, 1.6f}, {0x4000, 1.8f},
};

// initBite: the frames the player has to pull (by dFishParam_c::_2; 1: the float was cast long).
s16 sTimeParam0[6] = {17, 25, 30, 40, 80, 0};
s16 sTimeParam1[6] = {22, 28, 35, 50, 80, 0};

// rodata 0x0: draw's item model offset (0xC: 0.0f, unknown).
static const f32 sItemOfs[3] = {0.0f, -2.0f, 2.0f};
// rodata 0x10: the effect shadow's x / z scale, by dFishParam_c::mSize (execute).
static const f32 sShadowScale[8][2] = {
    {1.25f, 1.25f}, {1.65f, 1.0f}, {1.4f, 1.0f}, {1.33f, 0.9f}, {1.28f, 0.81f}, {1.38f, 0.61f}, {1.0f, 1.0f}, {2.66f, 0.72f},
};
// rodata 0x50: the ripples' scale range (dFishRipple_c::execute), by dFishParam_c::mSize.
static const f32 sRippleScaleMin[8] = {2.0f, 2.3f, 1.5f, 1.3f, 1.2f, 1.1f, 0.8f, 2.5f};
static const f32 sRippleScaleMax[8] = {2.5f, 2.8f, 2.0f, 1.8f, 1.7f, 1.6f, 3.0f, 1.3f};
// rodata 0x90
static const dFishSizeParam_c sSizeParams[8] = {
    {{0.3f, 0.3f, 0.3f}, 1.4f, 30, 60},
    {{0.4f, 0.4f, 0.45f}, 1.3f, 45, 90},
    {{0.5f, 0.5f, 0.65f}, 1.3f, 60, 120},
    {{0.6f, 0.6f, 0.8f}, 1.2f, 75, 150},
    {{0.8f, 0.7f, 1.0f}, 1.2f, 90, 180},
    {{0.8f, 0.7f, 1.2f}, 1.2f, 105, 210},
    {{0.9f, 1.0f, 1.4f}, 1.2f, 120, 240},
    {{0.3f, 0.3f, 1.1f}, 1.2f, 75, 150},
};

dFishPlaceCheck sPlaceChecks[dFishSpawn_c::PLACE_NUM] = {
    isRiverBlock, isPoolBlock, isWaterfallBlock, isPondBlock, isRiverMouthBlock, isOffingBlock, isSeaBlock,
};

// 0x5C
void *dFishField_c_classInit() {
    return new dFishField_c();
}

// 0x8C
dItem::Item getFishItem(int type) {
    dItem::Item item;
    BOOL isKey = FALSE;
    if (type >= 0x43 && type < FISH_TYPE_NUM) {
        isKey = TRUE;
    }
    if (isKey) {
        item.setFromIndex(0xBF, type - 0x43, FALSE);
    } else if (type < FISH_NUM) {
        item.setFromIndex(0x1D2, type, FALSE);
    } else {
        item.setFromIndex(0x212, type - FISH_NUM, FALSE);
    }
    return item;
}

// 0xF0
int getLostItemLikeIdx() {
    dPrivateData_c *player = dPlayerMgr_c::getNetPlayer(0);
    return dSaveData_c::getRaw()->mAnimals.mTown.getLostItemLikeIdx(player);
}

// 0x134
BOOL isRiverBlock(int blockX, int blockZ, void *user) {
    return fn_80190C44(0)->getBlock(blockX, blockZ)->hasFlag(BLOCK_KIND_FLAG_RIVER);
}

// 0x188
BOOL isPoolBlock(int blockX, int blockZ, void *user) {
    return fn_80190C44(0)->getBlock(blockX, blockZ)->hasFlag(BLOCK_KIND_FLAG_RACCO);
}

// 0x1D8
BOOL isWaterfallBlock(int blockX, int blockZ, void *user) {
    return fn_80190C44(0)->getBlock(blockX, blockZ)->hasFlag(BLOCK_KIND_FLAG_FALL);
}

// 0x228
BOOL isPondBlock(int blockX, int blockZ, void *user) {
    return fn_80190C44(0)->getBlock(blockX, blockZ)->fn_80080FC0();
}

// 0x274
BOOL isRiverMouthBlock(int blockX, int blockZ, void *user) {
    dFdBlock_c *block = fn_80190C44(0)->getBlock(blockX, blockZ);
    static const u32 cRiverFlags = BLOCK_KIND_FLAG_RIVER_S0 | BLOCK_KIND_FLAG_RIVER_S1 | BLOCK_KIND_FLAG_RIVER_S2;
    BOOL ret = FALSE;
    if (block->hasFlag(cRiverFlags) &&
        block->hasFlag(BLOCK_KIND_FLAG_BEACH)) {
        ret = TRUE;
    }
    return ret;
}

static inline BOOL isWeatherIn(const dUnk8074EBE8_c *weather, int want) {
    return weather->_5884 == want;
}

static inline BOOL isRainWeather(const dUnk8074EBE8_c *weather) {
    BOOL ret = FALSE;
    if (isWeatherIn(weather, 4) || isWeatherIn(weather, 3)) {
        ret = TRUE;
    }
    return ret;
}

static inline BOOL isSnowWeather(const dUnk8074EBE8_c *weather) {
    BOOL ret = FALSE;
    if (isWeatherIn(weather, 6) || isWeatherIn(weather, 5)) {
        ret = TRUE;
    }
    return ret;
}

// 0x2F4
BOOL isOffingBlock(int blockX, int blockZ, void *user) {
    if (isRainWeather(lbl_8074EBE8) || isSnowWeather(lbl_8074EBE8)) {
        return fn_80190C44(0)->getBlock(blockX, blockZ)->hasFlag(BLOCK_KIND_FLAG_BEACH);
    }
    return FALSE;
}

// 0x39C
BOOL isSeaBlock(int blockX, int blockZ, void *user) {
    return fn_80190C44(0)->getBlock(blockX, blockZ)->hasFlag(BLOCK_KIND_FLAG_BEACH);
}

// 0x3EC
dFishPlaceCheck getPlaceCheck(int place) {
    return sPlaceChecks[place];
}

// 0x400
BOOL isWaterfallUnit(int blockX, int blockZ, int unitX, int unitZ, void *user) {
    switch (dBGCF::getAttr((blockX << 4) + unitX, (blockZ << 4) + unitZ)) {
    case BG_ATTR_FALL_S:
    case BG_ATTR_FALL_SW:
    case BG_ATTR_FALL_SE:
        return TRUE;
    default:
        return FALSE;
    }
}

// 0x450
BOOL isOpenWater(const mVec3_c *pos) {
    nw4r::math::VEC3 p;
    p.x = pos->x;
    p.y = pos->y;
    p.z = pos->z;
    p.z -= 112.0f;
    int unitX = (int)p.x >> 5;
    int unitZ = (int)p.z >> 5;
    static const int cNum = 5;
    int ofs[cNum][2] = {{0, 0}, {-2, 0}, {2, 0}, {-2, 3}, {2, 3}};
    int *o = ofs[0];
    for (int i = 0; i < cNum; i++, o += 2) {
        if (dBGCF::getWaterKind(unitX + o[0], unitZ + o[1]) != BG_WATER_SEA) {
            return FALSE;
        }
    }
    return TRUE;
}

// 0x564
dFishField_c::dFishField_c() {
    mSpawnTimer = 150;
    mKeyOut = 0;
}

// 0x610
dFishField_c::~dFishField_c() {}

// 0x6B4
int dFishField_c::create() {
    EGG::Heap *heap;
    int i;
    int j;
    int blockX, blockZ, unitX, unitZ;
    int num;
    BOOL ok;
    int n;
    heap = lbl_8074E440;
    if (m_ResHeap == NULL) {
        m_ResHeap = mHeap::createFrmHeap(0x8000, heap, "dFishField_c::m_ResHeap", 0x20, mHeap::OPT_NONE);
    }
    ok = TRUE;
    if (!mShadowRes.load("/Fish/fsh_shadow.brres", m_ResHeap, 0)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }

    for (i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        m_childHeap[i] = mHeap::createFrmHeap(0x1000, heap, "dFishField_c::m_childHeap", 0x20, mHeap::OPT_NONE);
        mFish[i] = NULL;
    }
    for (j = 0; j < FISH_FIELD_PLAYER_NUM; j++) {
        m_childHeapForPlayer[j] =
            mHeap::createFrmHeap(0x1000, heap, "dFishField_c::m_childHeapForPlayer", 0x20, mHeap::OPT_NONE);
        mPlayerFish[j] = NULL;
    }

    // The waterfall basin: the center of the first waterfall acre's basin units.
    dFdBkSearchCandCb_c falls(fn_80190C44(0)->getBlockW(), fn_80190C44(0)->getBlockH(), isWaterfallBlock, NULL);
    falls.clear();
    falls.search();
    n = falls.getCount();
    while (n-- > 0 && falls.getNthXZ(n, &blockX, &blockZ)) {
        dBkUnitSearchCandCb_c units(blockX, blockZ, isWaterfallUnit, NULL);
        units.clear();
        units.search();
        num = units.getCount();
        if (num < 0) {
            continue;
        }
        int cnt = 0;
        mFallPos = mVec3_c::Zero;
        mFallNormal = mVec3_c::Ez;
        for (int i = 0; i < num; i++) {
            if (units.getNthXZ(i, &unitX, &unitZ)) {
                mVec3_c pos;
                dFdBase_c::getUnitCenterPos(&pos, blockX, blockZ, unitX, unitZ);
                cnt++;
                mFallPos.x += pos.x;
                mFallPos.y += pos.y;
                mFallPos.z += pos.z;
            }
        }
        if (cnt != 0) {
            mFallPos.x *= 1.0f / cnt;
            mFallPos.y *= 1.0f / cnt;
            mFallPos.z *= 1.0f / cnt;
            if (cnt == 2) {
                mFallPos.z -= 16.0f;
            }
            dBGCF::groundChk_c check(&mFallPos, dBGCF::LAYER_WATER, 0, 0);
            mFallNormal = check.mDir;
            mVec3_c pos = mFallPos;
            pos += mFallNormal;
            dBGCF::groundChk_c check2(&pos, dBGCF::LAYER_WATER, 0, 0);
            mFallPos.y = check2.mWaterY;
        }
        break;
    }

    lbl_8074E840 = this;
    initSpawn();
    return TRUE;
}

// 0xAEC
int dFishField_c::execute() {
    recvRecs();
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        dFishFldShadow_c *fish = mFish[i];
        if (fish != NULL) {
            if (isRecChanged(i)) {
                deleteFish(i);
            } else if (!fish->mDead) {
                fish->execute();
                fish->mSound.setPos(&fish->mPos);
            }
        }
    }
    for (int i = 0; i < FISH_FIELD_PLAYER_NUM; i++) {
        dFishFldShadow_c *fish = mPlayerFish[i];
        if (fish != NULL) {
            fish->mSound.setPos(&fish->mPos);
            fish->execute();
        }
    }
    calcSpawn();
    checkFish();
    sendRecs();
    return TRUE;
}

// 0xC04
int dFishField_c::draw() {
    mVec3_c center = lbl_80623FEC;
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        dFishFldShadow_c *fish = mFish[i];
        if (fish != NULL && isInView(&center, &fish->mPos)) {
            fish->draw();
        }
    }
    for (int i = 0; i < FISH_FIELD_PLAYER_NUM; i++) {
        dFishFldShadow_c *fish = mPlayerFish[i];
        if (fish != NULL && isInView(&center, &fish->mPos)) {
            fish->draw();
        }
    }
    return TRUE;
}

// 0xCFC
int dFishField_c::doDelete() {
    BOOL ok = TRUE;
    if (!mShadowRes.unload(FALSE)) {
        ok = FALSE;
    }
    if (!ok) {
        return ok;
    }
    if (m_ResHeap != NULL) {
        mHeap::destroyFrmHeap(m_ResHeap);
        m_ResHeap = NULL;
    }
    deleteAll();
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        if (m_childHeap[i] != NULL) {
            m_childHeap[i]->destroy();
            m_childHeap[i] = NULL;
        }
    }
    for (int i = 0; i < FISH_FIELD_PLAYER_NUM; i++) {
        if (m_childHeapForPlayer[i] != NULL) {
            m_childHeapForPlayer[i]->destroy();
            m_childHeapForPlayer[i] = NULL;
        }
    }
    lbl_8074E840 = NULL;
    return TRUE;
}

// 0xE0C
BOOL dFishField_c::canSpawnKey() const {
    if (getLostItemLikeIdx() != -1 && !mKeyOut) {
        return TRUE;
    }
    return FALSE;
}

// 0xE58
BOOL dSearchFishPos::check(int x, int z) {
    if (mAwayFromPlayers && !dFishField_c::isAwayFromPlayers(mBlockX, mBlockZ, x, z)) {
        return FALSE;
    }
    int unitX = (mBlockX << 4) + x;
    int unitZ = (mBlockZ << 4) + z;
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, unitX, unitZ);
    if (dFishFldShadow_c::isInFallBasin(&pos)) {
        return FALSE;
    }
    switch (mPlace) {
    case dFishSpawn_c::PLACE_WATERFALL: {
        mVec3_c fallPos = lbl_8074E840->mFallPos;
        static const f32 cFallAreaR = 128.0f;
        static const f32 cFallAreaRSq = cFallAreaR * cFallAreaR;
        mVec3_c d = pos - fallPos;
        if (d.x * d.x + d.z * d.z > cFallAreaRSq) {
            return FALSE;
        }
        f32 depth = 0.5f * (dBGCF::cWaterY1 + dBGCF::cWaterY2);
        dBGCF::groundChk_c check(&pos, dBGCF::LAYER_WATER, 0, 0);
        if (check.mWaterY > depth) {
            return FALSE;
        }
        break;
    }
    }
    switch (mWater) {
    case FISH_WATER_RIVER: {
        BOOL ret = FALSE;
        if (dBGCF::isRiver(unitX, unitZ) && !dBGCF::isSea(unitX, unitZ + 1) && !dBGCF::isSea(unitX, unitZ + 2)) {
            ret = TRUE;
        }
        return ret;
    }
    case FISH_WATER_SEA: {
        BOOL ret = FALSE;
        if (dBGCF::isSea(unitX, unitZ) && dBGCF::isSea(unitX, unitZ - 1)) {
            ret = TRUE;
        }
        return ret;
    }
    default:
        return FALSE;
    }
}

// 0x1074
BOOL dBkAttrSearchCand_c::check(int x, int z) {
    return fn_80190C44(0)->getBlock(x, z)->hasFlag(mMask);
}

// 0x10D0
void dFishField_c::calcSpawn() {
    if ((!fn_800DCEDC() || fn_800DD960()) && sLib::calcTimer(&mSpawnTimer) == 0) {
        mSpawnTimer = 300;
        int river = 0;
        int sea = 0;
        for (int i = 0; i < 4; i++) {
            if (mFish[i] == NULL) {
                river++;
            }
        }
        for (int i = 4; i < 8; i++) {
            if (mFish[i] == NULL) {
                sea++;
            }
        }
        if (river != 0 || sea != 0) {
            spawn(river, sea, TRUE);
        }
    }
    if (fn_800DCEDC() && !fn_800DD960()) {
        respawnFromRecs();
    }
}

// 0x11EC
void dFishField_c::respawnFromRecs() {
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        int type = mRecs[i].mRec.getA();
        if (mFish[i] == NULL && type != FISH_TYPE_NUM) {
            addFishFromRec(type, i);
        }
    }
}

// 0x1274
void dFishField_c::checkFish() {
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        dFishFldShadow_c *fish = mFish[i];
        if (fish != NULL) {
            if (fish->mDead) {
                deleteFish(i);
            } else {
                int unitZ, unitX, blockZ, blockX;
                dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, &fish->mPos);
                if (isAwayFromPlayers(blockX, blockZ, unitX, unitZ)) {
                    if (sLib::calcTimer(&mFish[i]->mLifeTimer) == 0 && isRecMine(i)) {
                        deleteFish(i);
                    }
                } else {
                    mFish[i]->mLifeTimer = 3600;
                }
            }
        }
    }
    for (int i = 0; i < FISH_FIELD_PLAYER_NUM; i++) {
        if (mPlayerFish[i] != NULL && mPlayerFish[i]->mDead) {
            deletePlayerFish(i);
        }
    }
}

// 0x13A8
BOOL dFishField_c::deleteFish(int idx) {
    if (mFish[idx] != NULL && mFish[idx]->isDeleteOk()) {
        BOOL isKey = FALSE;
        int type = mFish[idx]->mKind;
        if (type >= 0x43 && type < FISH_TYPE_NUM) {
            isKey = TRUE;
        }
        if (isKey) {
            mKeyOut = 0;
        }
        mFish[idx]->~dFishFldShadow_c();
        m_childHeap[idx]->free(3);
        mFish[idx] = NULL;
        if (isRecMine(idx)) {
            dPlaySyncRecBuf_c *rec = &mRecs[idx];
            rec->mRec.setA(FISH_TYPE_NUM);
            if (rec->mRec.getMember() != 4) {
                rec->mReleasePending = 1;
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 0x14C8
BOOL dFishField_c::deletePlayerFish(int idx) {
    if (mPlayerFish[idx] != NULL && mPlayerFish[idx]->isDeleteOk()) {
        mPlayerFish[idx]->~dFishFldShadow_c();
        m_childHeapForPlayer[idx]->free(3);
        mPlayerFish[idx] = NULL;
        return TRUE;
    }
    return FALSE;
}

// 0x1568
void dFishField_c::deleteAll() {
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        if (mFish[i] != NULL) {
            deleteFish(i);
        }
    }
    for (int i = 0; i < FISH_FIELD_PLAYER_NUM; i++) {
        if (mPlayerFish[i] != NULL) {
            deletePlayerFish(i);
        }
    }
}

// 0x1600
BOOL dFishField_c::isRecChanged(int idx) {
    if (mFish[idx] != NULL && (!mFish[idx]->isMine() || mRecs[idx].mRemote)) {
        dPlaySyncRec_c *rec = &mRecs[idx].mRec;
        if (rec->getA() != mFish[idx]->getKind() || rec->getC() != mFish[idx]->getRecId()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x16BC
static inline int posToUnit(f32 pos) { return (int)pos >> 5; }

BOOL dFishField_c::addFish(int type, int water, const mVec3_c *pos, const mAng3_c *angle) {
    int i, last;
    switch (water) {
    case FISH_WATER_RIVER:
        i = 0;
        last = 3;
        break;
    case FISH_WATER_SEA:
        i = 4;
        last = 7;
        break;
    }
    for (; i <= last; i++) {
        if (mFish[i] == NULL) {
            mVec3_c p = *pos;
            mAng3_c ang = *angle;
            dBGCF::groundChk_c check(&p, dBGCF::LAYER_WATER, 0, 0);
            p.y = check.mWaterY;
            mFish[i] = new (m_childHeap[i]) dFishFldShadow_c(&sFishParam[type]);
            mFish[i]->mPos = mFish[i]->mHomePos = mFish[i]->mTargetPos = p;
            mFish[i]->mTargetAngle = mFish[i]->mAngle = ang;
            mFish[i]->mAllocator.attach(m_childHeap[i], 0x20);
            mFish[i]->mSlot = i;
            mFish[i]->mKind = type;
            mFish[i]->mIsPlayerFish = 0;
            mFish[i]->mRecId = (mRecs[i].mRec.getC() + 1) & 0x7F;
            nw4r::g3d::ResFile res;
            res = nw4r::g3d::ResFile(mShadowRes.getData());
            mFish[i]->create(res);
            mRecs[i].mRec.setA(type);
            mRecs[i].mRec.setC(mFish[i]->mRecId);
            mRecs[i].mRec.setPosX(p.x);
            mRecs[i].mRec.setPosZ(p.z);
            int unitX = (int)p.x >> 5;
            int unitZ = (int)p.z >> 5;
            mRecs[i].mRec.setHomeUnitX(unitX);
            mRecs[i].mRec.setHomeUnitZ(unitZ);
            mRecs[i].mRec.setFishState(0);
            mRecs[i].mRec.setMember(4);
            mRecs[i].mRec.setDir(0);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x1A50
BOOL dFishField_c::addFishFromRec(int type, int idx) {
    if (mFish[idx] == NULL) {
        dPlaySyncRec_c *rec = &mRecs[idx].mRec;
        mVec3_c pos(rec->getPosX(), 0.0f, rec->getPosZ());
        dBGCF::groundChk_c check(&pos, dBGCF::LAYER_WATER, 0, 0);
        pos.y = check.mWaterY;
        mFish[idx] = new (m_childHeap[idx]) dFishFldShadow_c(&sFishParam[type]);
        mFish[idx]->mPos = mFish[idx]->mTargetPos = pos;
        dFdBase_c::getUnitCenterPos(&mFish[idx]->mHomePos, rec->getHomeUnitX(), rec->getHomeUnitZ());
        mFish[idx]->mHomePos.y = pos.y;
        mFish[idx]->mTargetAngle = mFish[idx]->mAngle = mAng3_c(0, 0, 0);
        mFish[idx]->mAllocator.attach(m_childHeap[idx], 0x20);
        mFish[idx]->mSlot = idx;
        mFish[idx]->mKind = type;
        mFish[idx]->mRecId = rec->getC();
        mFish[idx]->mIsPlayerFish = 0;
        nw4r::g3d::ResFile res;
        res = nw4r::g3d::ResFile(mShadowRes.getData());
        mFish[idx]->create(res);
        return TRUE;
    }
    return FALSE;
}

// 0x1C70
dFishFldShadow_c *dFishField_c::addPlayerFish(int type, int mode, const mVec3_c *pos, int player) {
    if (mPlayerFish[player] == NULL) {
        EGG::Heap *heap = m_childHeapForPlayer[player];
        dFishFldShadow_c *fish = new (heap) dFishFldShadow_c(&sFishParam[type]);
        mPlayerFish[player] = fish;
        fish->mPos = fish->mHomePos = fish->mTargetPos = *pos;
        fish->mAngle = fish->mTargetAngle = mAng3_c(0, 0, 0);
        fish->mAllocator.attach(heap, 0x20);
        fish->mSlot = player;
        fish->mPlayer = player;
        fish->mKind = type;
        fish->mIsPlayerFish = 1;
        nw4r::g3d::ResFile res;
        res = nw4r::g3d::ResFile(mShadowRes.getData());
        fish->create(res);
        if (mode == 1) {
            dPlayerActor_c *actor = fn_800FBC7C(player);
            if (actor != NULL) {
                fish->mTargetAngle.y = actor->mAngle.y;
            }
            fish->setState(dFishFldShadow_c::STATE_HOLD);
        } else {
            fish->setState(dFishFldShadow_c::STATE_SHOW_HOLD);
        }
        return fish;
    }
    return NULL;
}

// 0x1DF4
void dFishField_c::initSpawn() {
    if (fn_800DCEDC()) {
        clearRecs();
        initRecs();
        respawnFromRecs();
    }
    if (!fn_800DCEDC() || lbl_8074E844) {
        int river = 0;
        int sea = 0;
        for (int i = 0; i < 4; i++) {
            if (mFish[i] == NULL) {
                river++;
            }
        }
        for (int i = 4; i < 8; i++) {
            if (mFish[i] == NULL) {
                sea++;
            }
        }
        if (river != 0 || sea != 0) {
            spawn(river, sea, FALSE);
        }
        lbl_8074E844 = 0;
    }
}

// 0x1F0C
void dFishField_c::spawn(int riverNum, int seaNum, bool awayFromPlayers) {
    static dQuestEvent_e sEvent = (dQuestEvent_e)14;

    // The acres without a fish yet.
    int h = fn_80190C44(0)->mBlockH;
    int w = fn_80190C44(0)->mBlockW;
    dFdBkSearchCand_c free(w, h);
    free.setAll();
    free.removeBorder();
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        if (mFish[i] != NULL) {
            dFishFldShadow_c *fish = mFish[i];
            free.remove(((int)fish->mHomePos.x >> 9) + ((int)fish->mHomePos.z >> 9) * free.mWidth);
        }
    }
    bool event = dEvent::isOngoing(sEvent);
    int num;
    for (int water = 0; water < FISH_WATER_NUM; water++) {
        BOOL key = FALSE;
        switch (water) {
        case FISH_WATER_RIVER:
            num = riverNum;
            key = FALSE;
            if (canSpawnKey() && cM::rndRange(0.0f, 100.0f) <= 30.0f) {
                key = TRUE;
            }
            break;
        case FISH_WATER_SEA:
            num = seaNum;
            break;
        }
        for (int n = 0; n < num; n++) {
            dSaveTown_c *town = dSaveData_c::getTown();
            dItem::Item item = fn_80153410(town->_0632F0);
            const dFishSpawn_c *spawn;
            if (event && item.isValid()) {
                spawn = dFishInfo::getRandomSpawnBoosted(water, dItem::Item_getIdxInKind(item));
            } else {
                spawn = dFishInfo::getRandomSpawn(water);
            }
            if (spawn == NULL) {
                continue;
            }
            int type = spawn->mType;
            int place = spawn->mPlace;
            if (key) {
                type = getLostItemLikeIdx() + 0x43;
                place = dFishSpawn_c::PLACE_RIVER;
            }
            dFishPlaceCheck check = getPlaceCheck(place);
            h = fn_80190C44(0)->mBlockH;
            w = fn_80190C44(0)->mBlockW;
            dFdBkSearchCandCb_c blocks(w, h, check, (void *)spawn);
            blocks.clear();
            blocks.search();
            for (int j = 0; j < blocks.mNum; j++) {
                if (!free.isSet(j)) {
                    blocks.remove(j);
                }
            }
            int blockX, blockZ;
            if (!blocks.getRandomXZ(&blockX, &blockZ)) {
                break;
            }
            dSearchFishPos units;
            units.mBlockX = blockX;
            units.mBlockZ = blockZ;
            units.mAwayFromPlayers = awayFromPlayers;
            units.mWater = water;
            units.mPlace = place;
            units.clear();
            units.search();
            int unitX, unitZ;
            if (units.getRandomXZ(&unitX, &unitZ)) {
                mAng3_c angle(0, 0, 0);
                mVec3_c pos;
                dFdBase_c::getUnitCenterPos(&pos, blockX, blockZ, unitX, unitZ);
                addFish(type, water, &pos, &angle);
                if (key) {
                    getLostItemLikeIdx();
                    mKeyOut = 1;
                    key = FALSE;
                }
            }
            free.remove(blockX + blockZ * free.mWidth);
        }
    }
}

// 0x1EC sProcs
dFishFldShadow_c::Proc dFishFldShadow_c::sProcs[11] = {
    {&dFishFldShadow_c::initSwim, &dFishFldShadow_c::executeSwim, NULL},
    {&dFishFldShadow_c::initApproach, &dFishFldShadow_c::executeApproach, &dFishFldShadow_c::endApproach},
    {&dFishFldShadow_c::initNibble, &dFishFldShadow_c::executeNibble, NULL},
    {&dFishFldShadow_c::initBite, &dFishFldShadow_c::executeBite, NULL},
    {&dFishFldShadow_c::initEscape, &dFishFldShadow_c::executeEscape, NULL},
    {&dFishFldShadow_c::initHooked, &dFishFldShadow_c::executeHooked, NULL},
    {&dFishFldShadow_c::initCatch, &dFishFldShadow_c::executeCatch, NULL},
    {&dFishFldShadow_c::initRelease, &dFishFldShadow_c::executeRelease, NULL},
    {&dFishFldShadow_c::initHold, &dFishFldShadow_c::executeHold, NULL},
    {&dFishFldShadow_c::initShowHold, &dFishFldShadow_c::executeShowHold, NULL},
    {&dFishFldShadow_c::initShow, &dFishFldShadow_c::executeShow, NULL},
};

// 0x2304
BOOL dFishField_c::isInView(const mVec3_c *center, const mVec3_c *pos) {
    f32 minX = center->x - 320.0f;
    f32 maxX = 320.0f + center->x;
    f32 minZ = center->z - 448.0f;
    f32 maxZ = 192.0f + center->z;
    if (sLib::isInRange(pos->x, minX, maxX) && sLib::isInRange(pos->z, minZ, maxZ)) {
        return TRUE;
    }
    return FALSE;
}

// 0x23AC
static inline BOOL isNearUnit(int x, int z, int cx, int cz) {
    int dx = x - cx;
    int dz = z - cz;
    BOOL ret;
    if (dx >= -10 && dx <= 10 && dz >= -14 && dz <= 6) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }
    return ret;
}

BOOL dFishField_c::isAwayFromPlayers(int blockX, int blockZ, int unitX, int unitZ) {
    if (fn_800DCEDC()) {
        int x = (blockX << 4) + unitX;
        int z = (blockZ << 4) + unitZ;
        for (int i = 0; i < 4; i++) {
            u32 px, pz;
            if (fn_800FBD5C(&px, &pz, i) && isNearUnit(x, z, px, pz)) {
                return FALSE;
            }
        }
        return TRUE;
    }
    int cx = posToUnit(lbl_80623FEC.x);
    int cz = posToUnit(lbl_80623FEC.z);
    return !isNearUnit((blockX << 4) + unitX, (blockZ << 4) + unitZ, cx, cz);
}

// 0x250C
BOOL isFinFish(int type) {
    switch (type) {
    case 0x3C:
    case 0x3D:
    case 0x3E:
        return TRUE;
    default:
        return FALSE;
    }
}

// 0x2530
BOOL dFishField_c::scareFish(f32 radius, const mVec3_c *pos) {
    BOOL ret = FALSE;
    if (radius < 0.0f) {
        radius = 80.0f;
    }
    f32 radiusSq = radius * radius;
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        dFishFldShadow_c *fish = mFish[i];
        if (fish != NULL && fish->isMine() && fish->mState == dFishFldShadow_c::STATE_SWIM) {
            mVec3_c d = fish->mPos;
            d -= *pos;
            if (d.x * d.x + d.z * d.z < radiusSq) {
                fish->mScared = 1;
                ret = TRUE;
            }
        }
    }
    return ret;
}

// 0x2624
dFishFldShadow_c::dFishFldShadow_c(const dFishParam_c *param)
    : mParam(*param), mState(STATE_NONE), _1DC(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), _20C(0), mSpeed(0.0f),
      mAnmRate(1.0f), mPlayer(-1), mHoldPlayer(-1), mRipple(this), mAlpha(0xFF), mNibbling(0), mHeld(0), mScared(0),
      mLifeTimer(3600), mLinks(0), mHideHeld(0), mDead(0) {}

// 0x2788
dFishFldShadow_c::~dFishFldShadow_c() {}

// 0x283C
BOOL dFishFldShadow_c::create(const nw4r::g3d::ResFile &res) {
    const char *name;
    if (isFinFish() && mIsPlayerFish != 1) {
        name = "fsh_shadow01";
    } else {
        name = "fsh_shadow00";
    }
    nw4r::g3d::ResMdl mdl = res.GetResMdl(name);
    mMdl.create(mdl, &mAllocator, 0x100, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr(name), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    const dFishSizeParam_c *size = &sSizeParams[mParam.mSize];
    mMdl.setScale(size->mScale[0], size->mScale[1], size->mScale[2]);
    setState(STATE_SWIM);
    calcMtx();
    if (mKind == FISH_FROG) {
        mFrog.init();
    }
    return TRUE;
}

// 0x2970
// The states in which a held fish hangs from its player's hand, as a negated "not one of" test: MWCC
// only folds chains of == into a range check, and the target has four separate compares.
#define FISH_NOT_IN_HAND(state) ((state) != STATE_SHOW_HOLD && (state) != STATE_SHOW && (state) != STATE_HOOKED && (state) != STATE_CATCH)

void dFishFldShadow_c::calcMtx() {
    const dFishSizeParam_c *size = &sSizeParams[mParam.mSize];
    mMtx_c mtx;
    mVec3_c pos;
    if (mHeld) {
        pos = mPos;
        dActor_c::makeMtx(&mtx, &pos, mTargetAngle.y);
        if (!FISH_NOT_IN_HAND(mState)) {
            const nw4r::math::MTX34 *hand = (const nw4r::math::MTX34 *)fn_800FBE08(mPlayer);
            if (hand != NULL) {
                _1E8.set(dFishInfo::getHoldParam(mKind), 0.0f, 0.0f);
                mVec3_c handPos;
                PSMTXMultVec(*hand, _1E8, handPos);
                mMtx_c trans;
                trans.trans(handPos.x, handPos.y, handPos.z);
                PSMTXConcat(trans, mtx, mtx);
                mTargetAngle.x = -0x8FC;
            }
        }
        mMtx_c rot;
        rot.XrotS(mTargetAngle.x);
        rot.ZrotM(mTargetAngle.z);
        PSMTXConcat(mtx, rot, mtx);
    } else {
        f32 x = 0.0f;
        if (mState == STATE_HOOKED) {
            x = -8.0f * size->mScale[2];
        }
        f32 y;
        if (isFinFish() && mIsPlayerFish != 1) {
            y = -5.5f;
        } else {
            y = -4.0f;
        }
        pos.set(x, y, 4.0f * size->mScale[2]);
        pos.rotY(mTargetAngle.y);
        pos += mPos;
        mMdl._20 = mPos;
        dActor_c::makeMtx(&mtx, &pos, mTargetAngle.y);
    }
    getMdl()->setLocalMtx(&mtx);
}

// 0x2BB4
BOOL dFishFldShadow_c::isMine() {
    if (mIsPlayerFish == TRUE) {
        return TRUE;
    }
    return lbl_8074E840->isRecMine(mSlot);
}

// 0x2BDC
int dFishFldShadow_c::searchFloat() {
    if (mTurnTimer > 0) {
        return -1;
    }
    for (int i = 0; i <= 3; i++) {
        dFishingFloat_c *fl = fn_801710A4(i, this);
        if (fl == NULL) {
            continue;
        }
        mVec3_c floatPos = fl->mPos;
        mVec3_c diff = floatPos;
        diff.x -= mPos.x;
        diff.y -= mPos.y;
        diff.z -= mPos.z;
        BOOL inWater = TRUE;
        if (fl->mCastState != 1 && fl->mCastState != 2) {
            inWater = FALSE;
        }
        s16 angle = (inWater ? &sTurnParam1[mParam._1] : &sTurnParam0[mParam._1])->mAngle;
        f32 range = 32.0f * (inWater ? &sTurnParam1[mParam._1] : &sTurnParam0[mParam._1])->mSpeed;
        f32 dist = PSVECMag(diff);
        static const f32 cSplashR = 16.0f;
        if (dist < cSplashR && fn_8016FED4(fl)) {
            initSwim();
            mTurnTimer = 30;
            if (diff.normalizeRS()) {
                mCourse = cM::atan2s(diff.x, diff.z) + 0x8000U;
                f32 deg = cM::rndRange(-30.0f, 30.0f);
                mCourse += (s16)(deg * mAng::DegreeToAngleCoefficient);
            }
            return -1;
        }
        if (dist < range) {
            if (!diff.normalizeRS()) {
                if (!isInFallBasin(&floatPos)) {
                    return i;
                }
            } else {
                mVec3_c dir(0.0f, 0.0f, 1.0f);
                dir.rotY(mTargetAngle.y);
                dist = diff.x * dir.x + diff.z * dir.z;
                if (dist > mAng(angle).cos()) {
                    if (!isInFallBasin(&floatPos)) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

// 0x2EC0
BOOL dFishFldShadow_c::checkScared() {
    if (mScared) {
        return TRUE;
    }
    for (int i = 0; i < 4; i++) {
        dPlayerActor_c *player = fn_800FBC7C(i);
        if (player != NULL) {
            static const f32 cScareR = 80.0f;
            static const f32 cScareRSq = cScareR * cScareR;
            mVec3_c diff = mPos - player->mPos;
            if (diff.x * diff.x + diff.z * diff.z < cScareRSq && (fn_8010094C(i) || fn_800FF150(i))) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x2F98
BOOL dFishFldShadow_c::isOpenWater() {
    return ::isOpenWater(&mPos);
}

// 0x2FA0
dPlaySyncRecBuf_c *dFishFldShadow_c::getRec() const {
    if (mIsPlayerFish) {
        return NULL;
    }
    return &lbl_8074E840->mRecs[mSlot];
}

// 0x2FD0
void dFishFldShadow_c::calcRemote() {
    mDestPos.x = getRec()->mRec.getPosX();
    mDestPos.z = getRec()->mRec.getPosZ();
    mDestPos.y = mPos.y;
    mVec3_c diff = mDestPos - mPos;
    sLib::addCalcAngle(&mCourse.mAngle, getRec()->mRec.getDir() << 13, 8, (int)(2.0f * mAng::DegreeToAngleCoefficient));
    if (EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z) > 32.0f) {
        mSpeed = 1.0f;
        mAnmRate = 1.0f;
    } else {
        sLib::addCalc2(&mSpeed, 0.05f, 0.02f, 0.025f);
        _1DC.set(0.0f, 0.0f, mSpeed);
        _1DC.rotY(mCourse);
        sLib::addCalc2(&mAnmRate, 0.5f, 0.25f, 0.1f);
    }
    cLib::chasePosXZ(&mPos, mDestPos, mSpeed);
    sLib::addCalcAngle(&mTargetAngle.y.mAngle, mCourse, 2, (int)(10.0f * mAng::DegreeToAngleCoefficient));
}

// 0x31A4
void dFishFldShadow_c::updateRec() {
    if ((cCounter_c::m_gameFrame & 0x1F) != mSlot && mSpeed < 0.25f) {
        return;
    }
    if (!mIsPlayerFish) {
        getRec()->mRec.setPosX(mPos.x);
        getRec()->mRec.setPosZ(mPos.z);
        u8 dir = (mCourse.mAngle + 0x1000) >> 13;
        getRec()->mRec.setDir(dir);
    }
}

// 0x3314
void dFishFldShadow_c::initSwim() {
    BOOL mine = isMine();
    mTurnTimer = 15;
    mSlowTimer = 10;
    f32 x = mPos.x;
    f32 z = mPos.z;
    int unitX = (int)x >> 5;
    int unitZ = (int)z >> 5;
    if (isSeaSlot() == 0) {
        mTimer = cM::rndRange(100, 600);
        static const f32 cHomeRSq = 96.0f * 96.0f;
        mVec3_c d = mHomePos - mPos;
        f32 distSq = nw4r::math::VEC3LenSq(d);
        if (dBGCF::isSea(unitX, unitZ)) {
            f32 deg = cM::rndRange(-15.0f, 15.0f);
            s16 ofs = deg * mAng::DegreeToAngleCoefficient;
            mCourse = cM::atan2s(d.x, d.z);
            mCourse.mAngle += ofs;
        } else if (distSq > cHomeRSq) {
            dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 1, 0);
            mVec3_c n = check.mDir;
            if (n.x * d.x + n.y * d.y + n.z * d.z < 0.0f) {
                f32 deg = cM::rndRange(-30.0f, 30.0f);
                s16 ofs = deg * mAng::DegreeToAngleCoefficient;
                int ang = cM::atan2s(n.x, n.z);
                mCourse = ang + ofs + 0x8000;
            } else {
                f32 deg = cM::rndRange(-30.0f, 30.0f);
                s16 ofs = deg * mAng::DegreeToAngleCoefficient;
                mCourse = cM::atan2s(n.x, n.z);
                mCourse.mAngle += ofs;
            }
        } else {
            f32 deg = cM::rndRange(-90.0f, 90.0f);
            s16 ofs = deg * mAng::DegreeToAngleCoefficient;
            mCourse = mTargetAngle.y;
            mCourse.mAngle += ofs;
        }
    } else if (isSeaSlot() == 1) {
        mTimer = cM::rndRange(400, 600);
        if (isOpenWater()) {
            mVec3_c d = mHomePos - mPos;
            s16 rel = -0x8000 - mCourse.mAngle;
            BOOL ok = (rel < 0x2000 && rel >= 0 && d.x > -32.0f) || (rel > -0x2000 && rel < 0 && d.x < 32.0f);
            if (!ok) {
                f32 deg = cM::rndRange(30.0f, 45.0f);
                s16 ofs = deg * mAng::DegreeToAngleCoefficient;
                if (d.x > 0.0f) {
                    mCourse = -0x8000 - ofs;
                } else {
                    mCourse = ofs - 0x8000;
                }
            }
        } else {
            static const f32 cHomeRSq = 96.0f * 96.0f;
            mVec3_c d = mHomePos - mPos;
            if (nw4r::math::VEC3LenSq(d) > cHomeRSq) {
                dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 1, 0);
                if (d.x < 0.0f) {
                    s16 ofs;
                    if (d.z < 0.0f) {
                        f32 deg = cM::rndRange(-30.0f, 0.0f);
                        ofs = deg * mAng::DegreeToAngleCoefficient;
                    } else {
                        f32 deg = cM::rndRange(0.0f, 30.0f);
                        ofs = deg * mAng::DegreeToAngleCoefficient;
                    }
                    mCourse = ofs - 0x4000;
                } else {
                    s16 ofs;
                    if (d.z > 0.0f) {
                        f32 deg = cM::rndRange(-30.0f, 0.0f);
                        ofs = deg * mAng::DegreeToAngleCoefficient;
                    } else {
                        f32 deg = cM::rndRange(0.0f, 30.0f);
                        ofs = deg * mAng::DegreeToAngleCoefficient;
                    }
                    mCourse = ofs + 0x4000;
                }
            } else {
                f32 deg = cM::rndRange(-90.0f, 90.0f);
                s16 ofs = deg * mAng::DegreeToAngleCoefficient;
                mCourse = mTargetAngle.y;
                mCourse.mAngle += ofs;
            }
        }
    } else {
        f32 deg = cM::rndRange(-90.0f, 90.0f);
        s16 ofs = deg * mAng::DegreeToAngleCoefficient;
        mCourse = mTargetAngle.y;
        mCourse.mAngle += ofs;
    }

    mVec3_c ahead(0.0f, 0.0f, 80.0f);
    mVec3_c side(1.0f, 0.0f, 0.0f);
    ahead.rotY(mCourse);
    side.rotY(mCourse);
    ahead += mPos;
    dBGCF::groundChk_c check(&ahead, dBGCF::LAYER_WATER, 1, 0);
    if (check.mWater == BG_WATER_NONE) {
        if ((s16)(mCourse.mAngle - mTargetAngle.y.mAngle) > 0) {
            mCourse.mAngle += 0x2AAB;
        } else {
            mCourse.mAngle -= 0x2AAB;
        }
    }
    mSpeed = cM::rndRange(0.5f, 1.0f);
    mAnmRate = 1.0f;
    dPlaySyncRecBuf_c *rec;
    if (mine && (rec = getRec()) != NULL) {
        rec->mRec.setDir(mCourse.mAngle >> 13);
    }
    mRipple.init();
}

// 0x38D4
void dFishFldShadow_c::executeSwim() {
    sLib::calcTimer(&mTimer);
    sLib::calcTimer(&mSlowTimer);
    sLib::calcTimer(&mTurnTimer);
    mRipple.execute();
    if (!isMine()) {
        calcRemote();
        return;
    }
    if (mTimer == 0 || (mTurnTimer == 0 && isOpenWater())) {
        initSwim();
    }
    if (mSlowTimer == 0) {
        sLib::addCalc0(&mSpeed, 0.02f, 0.025f);
    }
    _1DC.set(0.0f, 0.0f, mSpeed);
    _1DC.rotY(mCourse);
    dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 1, 0);
    int ang;
    if (check.mWater == BG_WATER_SEA) {
        mVec3_c d = mHomePos - mPos;
        if (d.x * d.x + d.z * d.z < 64.0f) {
            ang = 0x4000;
        } else {
            ang = cM::atan2s(d.x, d.z);
        }
    } else {
        s16 rev = cM::atan2s(check.mDir.x, check.mDir.z) + 0x8000U;
        ang = rev;
    }
    f32 speed = 0.25f - 0.2f * nw4r::math::CosIdx((s16)(ang - mTargetAngle.y.mAngle) / 2);
    if (isSeaSlot() == 1) {
        speed *= 0.5f;
    }
    _1DC += check.mDir * speed;
    if (mSpeed < 0.1f) {
        s16 step = 2.0f * mAng::DegreeToAngleCoefficient;
        sLib::addCalcAngle(&mCourse.mAngle, ang, 8, step);
        sLib::addCalc2(&mAnmRate, 0.5f, 0.25f, 0.1f);
    }
    s16 step = 10.0f * mAng::DegreeToAngleCoefficient;
    sLib::addCalcAngle(&mTargetAngle.y.mAngle, mCourse, 2, step);
    mPos += _1DC;
    int player = searchFloat();
    if (player >= 0) {
        mPlayer = player;
        getRec()->mClaimMember = player;
        if (cM::rndRange(0.0f, 100.0f) < 95.0f) {
            setState(STATE_NIBBLE);
        } else {
            setState(STATE_APPROACH);
        }
    } else if (checkScared()) {
        setState(STATE_ESCAPE);
    }
    calcBgMove();
    updateRec();
}

// 0x3C28
void dFishFldShadow_c::initApproach() {
    if (!isMine() || getRec()->mRemote) {
        mPlayer = getRec()->mRec.getMember();
    }
    mNibbling = 1;
    mTimer = cM::rndRange<s16>(30, 90);
    mNibbleNum = 0;
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    fn_801710BC(mPlayer, this);
    if (fl != NULL) {
        mVec3_c pos = fl->mPos;
        mSpeed = cM::rndRange(1.25f, 2.0f);
    }
}

// 0x3D00
void dFishFldShadow_c::executeApproach() {
    const mVec3_c *flPos;
    if (mPlayer < 0) {
        sLib::addCalc0(&mSpeed, 0.03f, 0.1f);
        _1DC.set(0.0f, 0.0f, mSpeed);
        _1DC.rotY(mTargetAngle.y);
        mPos += _1DC;
        if (sLib::calcTimer(&mTimer) == 0 && isMine()) {
            setState(STATE_SWIM);
        }
    } else {
        dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
        if (fl != NULL) {
            flPos = &fl->mPos;
            if (isMine() && isInFallBasin(flPos)) {
                setState(STATE_ESCAPE);
                calcBgMove();
                return;
            }
            mVec3_c d = *flPos - mPos;
            if (d.x * d.x + d.z * d.z > 25.0f) {
                sLib::addCalcAngle(&mTargetAngle.y.mAngle, cM::atan2s(d.x, d.z), 4, 0x71C);
                mCourse = mTargetAngle.y;
            }
            if (!mNibbling) {
                sLib::addCalc2(&mAnmRate, 1.0f, 0.2f, 0.1f);
                sLib::addCalc2(&mSpeed, 0.75f, 0.2f, 0.1f);
                cLib::chasePosXZ(&mPos, *flPos, mSpeed);
                mVec3_c d2 = *flPos - mPos;
                f32 reach = 17.6f;
                if (d2.x * d2.x + d2.z * d2.z < reach) {
                    fn_8016FD60(fl);
                    mSound.startSound(0x180C);
                    fn_801710BC(mPlayer, NULL);
                    mPlayer = -1;
                    mTimer = cM::rndRange<s16>(30, 90);
                }
            } else if (sLib::calcTimer(&mTimer) != 0) {
                dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 1, 0);
                s16 ang = cM::atan2s(check.mDir.x, check.mDir.z);
                _1DC = check.mDir * (0.25f - 0.2f * nw4r::math::CosIdx((mAng(ang + 0x8000U) - mTargetAngle.y).mAngle / 2));
                f32 dist = EGG::Mathf::sqrt(d.x * d.x + d.z * d.z);
                if (dist < 32.0f) {
                    f32 rate = (32.0f - dist) / 32.0f;
                    sLib::addCalc0(&mSpeed, 0.2f, 0.05f);
                    mVec3_c back(0.0f, 0.0f, -mSpeed * rate);
                    back.rotY(mTargetAngle.y);
                    _1DC += back;
                    sLib::addCalc2(&mAnmRate, 0.5f + 0.5f * rate, 0.2f, 0.1f);
                } else {
                    sLib::addCalc2(&mAnmRate, 0.5f, 0.2f, 0.1f);
                }
                mPos += _1DC;
            } else {
                mNibbling = 0;
            }
        } else if (isMine()) {
            setState(STATE_ESCAPE);
        }
    }
    calcBgMove();
}

// 0x4130
void dFishFldShadow_c::endApproach() {
    if (mPlayer >= 0) {
        fn_801710BC(mPlayer, NULL);
        mPlayer = -1;
    }
    if (isMine() && getRec()->mRec.getMember() != 4) {
        dPlaySyncRecBuf_c *rec = getRec();
        rec->mReleasePending = 1;
    }
}

// 0x41B0
void dFishFldShadow_c::initNibble() {
    // The original tests the member function pointer isMine (always TRUE, __ptmf_test of the PTMF
    // constants data 0x398 / 0x3A4) instead of the local: mine is unused.
    BOOL mine = isMine();
    if (!isMine || getRec()->mRemote) {
        mPlayer = getRec()->mRec.getMember();
    }
    mNibbleNum = 0;
    mNibbling = 0;
    if (isMine) {
        getRec()->mRec.setDir(mNibbling);
    }
    mTimer = cM::rndRange(70, 220);
    if (fn_801710A4(mPlayer, this) != NULL) {
        mSpeed *= 0.5f;
        fn_801710BC(mPlayer, this);
    }
}

// 0x42A0
void dFishFldShadow_c::executeNibble() {
    const mVec3_c *flPos;
    u8 prev;
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    BOOL mine = isMine();
    if (fl != NULL) {
        flPos = &fl->mPos;
        if (isMine() && isInFallBasin(flPos)) {
            setState(STATE_ESCAPE);
            calcBgMove();
            return;
        }
        mVec3_c d = *flPos - mPos;
        f32 dist = EGG::Mathf::sqrt(d.x * d.x + d.z * d.z);
        if (dist > 8.0f) {
            s16 ang = cM::atan2s(d.x, d.z);
            mCourse = ang;
            sLib::addCalcAngle(&mTargetAngle.y.mAngle, ang, 4, 0x71C);
        }
        if (!mine) {
            prev = mNibbling;
            mNibbling = getRec()->mRec.getDir() != 0;
            if (prev && !mNibbling && dist > 8.0f) {
                mNibbling = prev;
            }
            if (prev != mNibbling) {
                if (mNibbling) {
                    if (mSpeed < 0.0f) {
                        mSpeed = -mSpeed;
                    }
                } else {
                    fn_8016FD60(fl);
                    mSound.startSound(0x180C);
                    sLib::chase(&mNibbleNum, 5, 1);
                    mTimer = cM::rndRange(70, 220);
                    mSpeed = -0.5f;
                }
            }
        }
        if (mNibbling) {
            sLib::addCalc2(&mAnmRate, 1.0f, 0.2f, 0.1f);
            sLib::addCalc2(&mSpeed, 0.75f, 0.2f, 0.1f);
            cLib::chasePosXZ(&mPos, *flPos, mSpeed);
            mVec3_c d2 = *flPos - mPos;
            f32 reach = 17.6f;
            if (mine && d2.x * d2.x + d2.z * d2.z < reach) {
                int left = 5 - mNibbleNum;
                int n = left >= 1 ? left : 1;
                f32 chance = 100.0f - 100.0f / n;
                if (cM::rndRange(0.0f, 100.0f) > chance || sLib::chase(&mNibbleNum, 5, 1)) {
                    if (fl->_447 == 9) {
                        fn_801710BC(mPlayer, NULL);
                        setState(STATE_ESCAPE);
                    } else {
                        setState(STATE_BITE);
                    }
                } else {
                    fn_8016FD60(fl);
                    mSound.startSound(0x180C);
                    mTimer = cM::rndRange(70, 220);
                    mSpeed = -0.5f;
                    mNibbling = 0;
                }
            }
        } else {
            sLib::addCalc2(&mAnmRate, 0.5f, 0.2f, 0.1f);
            if (sLib::calcTimer(&mTimer) != 0 || !mine) {
                dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 1, 0);
                s16 ang = cM::atan2s(check.mDir.x, check.mDir.z);
                _1DC = check.mDir * (0.25f - 0.2f * nw4r::math::CosIdx((mAng(ang + 0x8000U) - mTargetAngle.y).mAngle / 2));
                f32 dist2 = EGG::Mathf::sqrt(d.x * d.x + d.z * d.z);
                if (dist2 < 32.0f) {
                    if (mNibbleNum == 0) {
                        sLib::addCalc0(&mSpeed, 0.05f, 0.02f);
                    } else {
                        sLib::addCalc0(&mSpeed, 0.025f, 0.01f);
                    }
                    f32 rate = (32.0f - dist2) / 32.0f;
                    mVec3_c back(0.0f, 0.0f, mSpeed * rate);
                    back.rotY(mTargetAngle.y);
                    _1DC += back;
                    sLib::addCalc2(&mAnmRate, 0.5f + 0.5f * rate, 0.2f, 0.1f);
                } else {
                    if (dist2 > 64.0f) {
                        f32 s = 128.0f - dist2;
                        s = (s < 0.0f) ? 0.0f : s;
                        _1DC *= s / 64.0f;
                    }
                    sLib::addCalc2(&mAnmRate, 0.5f, 0.2f, 0.1f);
                }
                mPos += _1DC;
            } else {
                if (mSpeed < 0.0f) {
                    mSpeed = -mSpeed;
                }
                mNibbling = 1;
            }
        }
        if (mine) {
            getRec()->mRec.setDir(mNibbling);
        }
    } else if (mine) {
        setState(STATE_ESCAPE);
    }
    calcBgMove();
}

// 0x4858
void dFishFldShadow_c::initBite() {
    if (!isMine() || getRec()->mRemote) {
        mPlayer = getRec()->mRec.getMember();
    }
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    BOOL longCast = FALSE;
    if (fl != NULL) {
        fn_8016FC44(fl, 7);
        longCast = fl->mCastState == 2;
    }
    mCourse = mTargetAngle.y;
    s16 *time;
    if (longCast) {
        time = &sTimeParam1[mParam._2];
    } else {
        time = &sTimeParam0[mParam._2];
    }
    mTimer = *time;
}

// 0x4934
void dFishFldShadow_c::executeBite() {
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    if (fl != NULL) {
        if (sLib::calcTimer(&mTimer) == 0 && isMine()) {
            fn_8016FC44(fl, 5);
            fn_801710BC(mPlayer, NULL);
            setState(STATE_ESCAPE);
        }
    } else if (isMine()) {
        setState(STATE_ESCAPE);
    }
    calcBgMove();
}

// 0x49E8
void dFishFldShadow_c::initEscape() {
    if (mPlayer >= 0) {
        fn_801710BC(mPlayer, NULL);
    }
    if (isFinFish()) {
        mTimer = 120;
        f32 deg = cM::rndRange(-15.0f, 15.0f);
        mCourse = deg * mAng::DegreeToAngleCoefficient;
    } else {
        mTimer = 60;
        mCourse = mTargetAngle.y.mAngle - 0x8000;
    }
    mSpeed = 0.0f;
    setEscapeEffect();
}

// 0x4AA0
void dFishFldShadow_c::executeEscape() {
    sLib::addCalc2(&mSpeed, 1.0f, 0.2f, 0.2f);
    sLib::addCalcAngle(&mTargetAngle.y.mAngle, mCourse, 2, 0x71C);
    _1DC.set(0.0f, 0.0f, mSpeed);
    _1DC.rotY(mCourse);
    mPos += _1DC;
    if (sLib::calcTimer(&mTimer) == 0 && isMine()) {
        mDead = 1;
    }
    if (!isFinFish()) {
        nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
        int matID = m3d::getMatID(mdl, "m0");
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn()), matID);
        if (cma.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor();
            tev.GXGetTevKColor(GX_KCOLOR1, &color);
            color.a = mAlpha = (mTimer * 255) / 60;
            tev.GXSetTevKColor(GX_KCOLOR1, color);
            tev.DCStore(false);
        }
        calcBgMove();
    }
}

// 0x4CA4
void dFishFldShadow_c::initHooked() {
    if (!isMine() || getRec()->mRemote) {
        mPlayer = getRec()->mRec.getMember();
    }
    mAnmRate = 1.0f;
    fn_801710BC(mPlayer, this);
    const dFishSizeParam_c *size = &sSizeParams[mParam.mSize];
    mTimer = cM::rndRange(size->mHookTimeMin, size->mHookTimeMax);
    int kind = mKind;
    BOOL isKey = FALSE;
    if (kind >= 0x43 && kind < FISH_TYPE_NUM) {
        isKey = TRUE;
    }
    if (isKey) {
        dSaveData_c::getTown()->mAnimals.mTown.setLostItemState1();
    }
}

// 0x4D70
void dFishFldShadow_c::executeHooked() {
    BOOL held;
    if (mKind >= FISH_NUM) {
        held = TRUE;
    } else {
        mHoldPlayer = mPlayer;
        held = fn_80190970(lbl_8074E9E8, mPlayer, 2, mKind, 1, 1);
    }
    if (sLib::calcTimer(&mTimer) == 0 && held && isMine()) {
        setState(STATE_CATCH);
    } else {
        dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
        if (fl != NULL) {
            mVec3_c pos = fl->mPos;
            mPos.set(pos.x, mPos.y, pos.z);
        }
        mTargetAngle.y.mAngle += 0x71C;
        mSound.holdSound(0x17C2);
    }
}

// 0x4E64
void dFishFldShadow_c::initCatch() {
    if (!isMine() || getRec()->mRemote) {
        mPlayer = getRec()->mRec.getMember();
    }
    fn_801710BC(mPlayer, this);
    mTimer = 45;
    mDestPos = mPos;
    mStartPos = mPos;
    BOOL held;
    if (mKind >= FISH_NUM) {
        held = TRUE;
    } else {
        mHoldPlayer = mPlayer;
        held = fn_80190970(lbl_8074E9E8, mPlayer, 2, mKind, 1, 1);
    }
    if (held) {
        mHeld = 1;
    }
    mTargetAngle.y = 0;
    setSplashEffect();
}

// 0x4F4C
void dFishFldShadow_c::executeCatch() {
    if (!mHeld) {
        BOOL held;
        if (mKind >= FISH_NUM) {
            held = TRUE;
        } else {
            held = fn_80190970(lbl_8074E9E8, mHoldPlayer, 2, mKind, 1, 1);
        }
        if (held) {
            mHeld = 1;
        }
    }
    sLib::calcTimer(&mTimer);
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    const mVec3_c *hand = (const mVec3_c *)fn_800FBDD4(mPlayer);
    if (fl != NULL && hand != NULL) {
        f32 t = mTimer / 45.0f;
        f32 u = 1.0f - t;
        mPos = fl->mPos * t + *hand * u;
    } else if (hand != NULL) {
        mPos = *hand;
    }
    if (mTimer == 0 && mKind < FISH_NUM) {
        mSound.holdSound(0x180F);
    }
}

// 0x50DC
void dFishFldShadow_c::initShowHold() {
    mHoldPlayer = mPlayer;
    fn_80190970(lbl_8074E9E8, mPlayer, 2, mKind, 1, 1);
    mHeld = 1;
    mTargetAngle.y = 0;
    mHideHeld = 1;
}

// 0x513C
void dFishFldShadow_c::executeShowHold() {
    if (fn_80190970(lbl_8074E9E8, mHoldPlayer, 2, mKind, 1, 1)) {
        mHoldPlayer = mPlayer;
        setState(STATE_SHOW);
    }
}

// 0x51A0
void dFishFldShadow_c::initShow() {
    mHideHeld = 0;
    mTimer = 60;
    setHeldScale(&mVec3_c::Zero);
}

// 0x51BC
void dFishFldShadow_c::executeShow() {
    const mVec3_c *hand = (const mVec3_c *)fn_800FBDD4(mPlayer);
    if (hand != NULL) {
        mPos = *hand;
    }
    if (sLib::calcTimer(&mTimer) == 0) {
        mSound.holdSound(0x180F);
    }
}

// 0x5228
void dFishFldShadow_c::initHold() {
    mHoldPlayer = mPlayer;
    fn_80190970(lbl_8074E9E8, mPlayer, 2, mKind, 1, 1);
    mHeld = 1;
    mHideHeld = 1;
}

// 0x5280
void dFishFldShadow_c::executeHold() {
    const mVec3_c *hand = (const mVec3_c *)fn_800FBDD4(mPlayer);
    if (hand != NULL) {
        mPos = *hand;
    }
    if (fn_80190970(lbl_8074E9E8, mHoldPlayer, 2, mKind, 1, 1)) {
        mScale.set(0.0f, 0.0f, 0.0f);
        setState(STATE_RELEASE);
    }
}

// 0x5318
static inline BOOL isZeroVec(const mVec3_c &v) {
    return std::fabs(nw4r::math::VEC3LenSq(v)) <= FLT_EPSILON;
}

void dFishFldShadow_c::initRelease() {
    if (!isMine() || (getRec() != NULL && getRec()->mRemote)) {
        mPlayer = getRec()->mRec.getMember();
    }
    dHoldItemMgr_c::Mdl_c *mdl = lbl_8074E9E8->getMdl(mHoldPlayer);
    if (mdl == NULL) {
        initHold();
    } else {
        if (!isZeroVec(mScale)) {
            f32 scale = dFishInfo::getModelScale(mKind);
            mScale.set(scale, scale, scale);
        }
        mHideHeld = 0;
        mTimer = 60;
        mStartPos = mPos;
        fn_8019022C(mdl, "swim");
        mSound.startSound(0x17B0);
        setHeldScale(&mScale);
    }
}

// 0x547C
void dFishFldShadow_c::executeRelease() {
    if (mHideHeld) {
        executeHold();
        return;
    }
    if (mHeld) {
        mVec3_c end = mDestPos;
        f32 scale = dFishInfo::getModelScale(mKind);
        f32 step = 0.2f * scale;
        sLib::chase(&mScale.x, scale, step);
        sLib::chase(&mScale.y, scale, step);
        sLib::chase(&mScale.z, scale, step);
        setHeldScale(&mScale);
        f32 t = (1.0f / 60.0f) * mTimer;
        f32 h = 2.0 * t - 1.0;
        f32 height = 1.0f - h * h;
        mPos = mStartPos * t;
        mPos += end * (1.0f - t);
        mPos.y += 32.0f * height;
        if (sLib::calcTimer(&mTimer) == 0) {
            if (mKind < FISH_NUM) {
                fn_801909D4(lbl_8074E9E8, mHoldPlayer);
                mHoldPlayer = -1;
            }
            u32 se;
            switch (mParam.mSize) {
            case 0:
            case 1:
                se = 0x17B1;
                break;
            case 2:
            case 7:
                se = 0x17B2;
                break;
            case 3:
            case 4:
                se = 0x17B3;
                break;
            default:
                se = 0x17B4;
                break;
            }
            mSound.startSound(se);
            setSplashEffect();
            mHeld = 0;
            mTimer = 60;
            mSpeed = 0.0f;
            mPos = mDestPos;
            mTargetPos = mDestPos;
            lbl_8074E840->scareFish(-1.0f, &mPos);
        } else {
            mVec3_c d = mPos - mTargetPos;
            if (!isZeroVec(d)) {
                s16 pitch = cM::atan2s(d.y, d.xzLen());
                s32 target = -pitch;
                sLib::addCalcAngle(&mTargetAngle.x.mAngle, target, 16, 0x71C);
            }
            d = mDestPos - mStartPos;
            if (!isZeroVec(d)) {
                mCourse = d.xzAng();
                sLib::addCalcAngle(&mTargetAngle.y.mAngle, mCourse, 16, 0x71C);
            }
        }
    } else {
        if (sLib::calcTimer(&mTimer) == 0) {
            mDead = 1;
            return;
        }
        nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
        int matID = m3d::getMatID(mdl, "m0");
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn()), matID);
        if (cma.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = cma.GetResMatTevColor();
            tev.GXGetTevKColor(GX_KCOLOR1, &color);
            color.a = (mTimer * 255) / 60;
            tev.GXSetTevKColor(GX_KCOLOR1, color);
            tev.DCStore(false);
        }
        sLib::addCalc2(&mSpeed, 1.0f, 0.2f, 0.2f);
        _1DC.set(0.0f, 0.0f, mSpeed);
        _1DC.rotY(mTargetAngle.y);
        mPos += _1DC;
        calcBgMove();
    }
}

// 0x5998
BOOL dFishFldShadow_c::isFinFish() const {
    return ::isFinFish(mKind);
}

// 0x59A0
int dFishFldShadow_c::getCatchKind() const {
    int kind = mKind;
    BOOL isKey = FALSE;
    if (kind >= 0x43 && kind < FISH_TYPE_NUM) {
        isKey = TRUE;
    }
    if (isKey) {
        return 0x43;
    }
    return kind;
}

// 0x59D0
BOOL dFishFldShadow_c::isSeaSlot() const {
    if (mSlot >= 0 && mSlot <= 3) {
        return FALSE;
    }
    return TRUE;
}

// 0x59F4
dItem::Item dFishFldShadow_c::getItem() const {
    return getFishItem(mKind);
}

// 0x59FC
BOOL dFishFldShadow_c::pull() {
    if (mPlayer < 0) {
        return FALSE;
    }
    if (mState == STATE_BITE) {
        dPrivateData_c *player = dPlayerMgr_c::getNetPlayerRaw(mPlayer);
        BOOL ok;
        int kind = mKind;
        BOOL isKey = FALSE;
        if (kind >= 0x43 && kind < FISH_TYPE_NUM) {
            isKey = TRUE;
        }
        if (isKey && (!player->mPID.isFromTown() || player->findEmptyPocket(0) == -1)) {
            ok = FALSE;
        } else {
            BOOL isTrash = FALSE;
            int kind2 = mKind;
            if (kind2 >= FISH_NUM) {
                BOOL isKey2 = FALSE;
                if (kind2 >= 0x43 && kind2 < FISH_TYPE_NUM) {
                    isKey2 = TRUE;
                }
                if (!isKey2) {
                    isTrash = TRUE;
                }
            }
            u32 b;
            u32 a;
            if (isTrash && !fn_111_AEA4(0x10, &b, &a, 0)) {
                ok = FALSE;
            } else {
                ok = TRUE;
            }
        }
        if (ok) {
            setState(STATE_HOOKED);
        } else {
            dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
            if (fl != NULL) {
                fn_8016FC44(fl, 5);
                fn_801710BC(mPlayer, NULL);
            }
            setState(STATE_ESCAPE);
        }
        return ok;
    }
    dFishingFloat_c *fl = fn_801710A4(mPlayer, this);
    if (fl != NULL) {
        fn_8016FC44(fl, 5);
        fn_801710BC(mPlayer, NULL);
    }
    return FALSE;
}

// 0x5B8C
BOOL dFishFldShadow_c::isCatching() const {
    return mState == STATE_CATCH;
}

// 0x5BA0
BOOL dFishFldShadow_c::isCaught() const {
    BOOL ret = FALSE;
    if (mState == STATE_CATCH && mTimer == 0) {
        ret = TRUE;
    }
    return ret;
}

// 0x5BC8
void dFishFldShadow_c::throwBack(const mVec3_c *pos) {
    if (pos != NULL) {
        mDestPos = *pos;
    }
    if (mState != STATE_HOLD) {
        if (mPlayer >= 0) {
            fn_801710BC(mPlayer, NULL);
            mPlayer = -1;
        }
        setState(STATE_RELEASE);
    }
}

// 0x5C44
void dFishFldShadow_c::requestDelete() {
    if (mPlayer >= 0) {
        fn_801710BC(mPlayer, 0);
        mPlayer = -1;
    }
    if (mHeld) {
        fn_801909D4(lbl_8074E9E8, mHoldPlayer);
        mHoldPlayer = -1;
        mHeld = 0;
    }
    if (mIsPlayerFish == 1) {
        lbl_8074E840->deletePlayerFish(mSlot);
    } else {
        mDead = 1;
    }
}

// 0x5CE0
BOOL dFishFldShadow_c::getMouthPos(mVec3_c *pos) const {
    if (pos == NULL) {
        return FALSE;
    }
    if (mHeld) {
        return FALSE;
    }
    const dFishSizeParam_c *size = &sSizeParams[mParam.mSize];
    f32 x = 0.0f;
    if (mState == STATE_HOOKED) {
        x = -8.0f * size->mScale[2];
    }
    f32 y;
    if (isFinFish() && mIsPlayerFish != 1) {
        y = -5.5f;
    } else {
        y = -4.0f;
    }
    mAng ang = getTargetAngleY();
    mVec3_c ofs(x, y, 4.0f * size->mScale[2]);
    ofs.rotY(ang);
    f32 px = ofs.x + mPos.x;
    f32 py = ofs.y + mPos.y;
    f32 pz = ofs.z + mPos.z;
    ofs.set(px, py, pz);
    *pos = ofs;
    return TRUE;
}

// 0x5E28
void dFishFldShadow_c::setHeldPos(const mVec3_c *pos) {
    if (mHeld) {
        mPos = *pos;
    }
}

// 0x5E50
void dFishFldShadow_c::setHeldScale(const mVec3_c *scale) {
    if (mHeld && !mHideHeld) {
        if (mKind >= FISH_NUM) {
            mScale = *scale;
        } else {
            getMdl()->setScale(*scale);
        }
    }
}

// 0x5EC4
void dFishFldShadow_c::calcBgMove() {
    f32 radius;
    if (mPlayer >= 0) {
        radius = 4.0f;
    } else {
        radius = 32.0f;
    }
    dBGCF::groundChk_c check(&mTargetPos, dBGCF::LAYER_WATER, 0, 0);
    f32 height = check.getHeight(0);
    f32 y = mTargetPos.y;
    mPos.y = height;
    mTargetPos.y = height;
    pushOutOfFall();
    mWallCheck.check(radius, &mPos, &mTargetPos, mTargetAngle.y, 0,
                     dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS, 1);
    mTargetPos.y = y;
    mPos.y = y;
    dBGCF::groundChk_c check2(&mPos, dBGCF::LAYER_WATER, 0, 0);
    f32 h = check2.mWaterY;
    if (mPos.y - h > 1.0f) {
        mPos = mTargetPos;
    }
}

// 0x5FE8
BOOL dFishFldShadow_c::isInFallBasin(const mVec3_c *pos) {
    static const f32 cFallR = 96.0f;
    static const f32 cFallRSq = cFallR * cFallR;
    static const f32 cFallDepth = 48.0f;
    dFishField_c *field = lbl_8074E840;
    mVec3_c d = *pos;
    d -= field->mFallPos;
    if (d.x * d.x + d.z * d.z < cFallRSq) {
        f32 dot = d.x * field->mFallNormal.x + d.z * field->mFallNormal.z;
        if (dot > -cFallDepth && dot < 0.1f) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x6084
BOOL dFishFldShadow_c::pushOutOfFall() {
    static const f32 cFallR = 96.0f;
    static const f32 cFallRSq = cFallR * cFallR;
    static const f32 cFallDepth = 48.0f;
    dFishField_c *field = lbl_8074E840;
    mVec3_c d = mPos;
    d -= field->mFallPos;
    if (d.x * d.x + d.z * d.z < cFallRSq) {
        mVec3_c n = field->mFallNormal;
        if (n.normalizeRS()) {
            f32 dot = d.x * n.x + d.z * n.z;
            if (dot > -cFallDepth && dot < 0.0f) {
                mPos -= n * (cFallDepth + dot);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x61B8
m3d::bmdl_c *dFishFldShadow_c::getMdl() {
    if (mHeld && !mHideHeld && mKind < FISH_NUM) {
        return &lbl_8074E9E8->getMdl(mHoldPlayer)->mMdl;
    }
    return &mMdl;
}

// 0x622C
void dFishFldShadow_c::setState(int state) {
    if (mState != state) {
        if (mState != STATE_NONE && sProcs[mState].mEnd) {
            (this->*sProcs[mState].mEnd)();
        }
        (this->*sProcs[state].mInit)();
        mState = state;
    }
}

// 0x62D8
void dFishFldShadow_c::recvRec() {
    if (mState == STATE_SWIM) {
        dBGCF::groundChk_c check(&mPos, dBGCF::LAYER_WATER, 0, 0);
        dPlaySyncRecBuf_c *buf = getRec();
        if (check.mWater == BG_WATER_NONE && buf != NULL) {
            mVec3_c pos(buf->mRec.getPosX(), mPos.y, buf->mRec.getPosZ());
            dBGCF::groundChk_c check2(&pos, dBGCF::LAYER_WATER, 0, 0);
            if (check2.mWater != BG_WATER_NONE) {
                mTargetPos = pos;
                mPos = mTargetPos;
            } else {
                mTargetPos = mHomePos;
                mPos = mTargetPos;
            }
        }
    }
}

// 0x641C
int dFishFldShadow_c::execute() {
    mTargetPos = mPos;
    if (!isMine() || (getRec() != NULL && getRec()->mRemote)) {
        setState(getRec()->mRec.getFishState());
    }
    if (mKind == FISH_FROG) {
        mFrog.execute(this);
    }
    (this->*sProcs[mState].mExecute)();
    if (isMine() && getRec() != NULL) {
        getRec()->mRec.setFishState(mState);
    }
    mAnm.setRate(mAnmRate);
    calcMtx();
    if (mKind < FISH_NUM || !mHeld) {
        getMdl()->play();
        getMdl()->calc(false);
    }
    if (mState == STATE_HOOKED) {
        s16 target = -60.0f * mAng::DegreeToAngleCoefficient;
        sLib::addCalcAngle(&mMdl.mAngle.mAngle, target, 4, 0x71C);
    } else {
        s16 diff = mTargetAngle.y.mAngle - mCourse;
        s16 target = diff < -0x31C7 ? -0x31C7 : (diff > 0x31C7 ? 0x31C7 : diff);
        sLib::addCalcAngle(&mMdl.mAngle.mAngle, target, 2, 0xE39);
    }
    if (!mHeld && !mHideHeld) {
        const char *nodes[] = {"root", "a"};
        const char *effects[] = {"af_fsh_shadow", "af_fsh_sebire_moya"};
        int fin;
        if (isFinFish() && mIsPlayerFish != 1) {
            fin = 1;
        } else {
            fin = 0;
        }
        int node = m3d::getNodeID(mMdl.getResMdl(), nodes[fin]);
        mMtx_c mtx;
        if (node >= 0 && mMdl.getNodeWorldMtx(node, &mtx)) {
            mVec3_c pos;
            mtx.multVecZero(pos);
            mVec3_c fieldPos;
            dWorld::toFieldPosition(&fieldPos, &pos);
            dBGCF::groundChk_c check(&fieldPos, dBGCF::LAYER_WATER, 0, 0);
            fieldPos.y = check.mWaterY;
            dWorld::curvePosition(&pos, &fieldPos);
            mtx.m[0][3] = pos.x;
            mtx.m[1][3] = pos.y;
            mtx.m[2][3] = pos.z;
            mMtx_c scale;
            PSMTXScale(scale, sShadowScale[mParam.mSize][0], 1.0f, sShadowScale[mParam.mSize][1]);
            PSMTXConcat(mtx, scale, mtx);
            fn_80087B40(&mEffect, effects[fin], &mtx, 1);
            mEffect.setColor(0xFF, 0xFF, 0xFF, mAlpha, EGG::Effect::RECURSIVE_0);
        }
    }
    return TRUE;
}

// 0x67A8
int dFishFldShadow_c::draw() {
    if (mHideHeld) {
        return TRUE;
    }
    if (mKind < FISH_NUM || !mHeld) {
        if (mHeld) {
            getMdl()->setPriorityDraw(2, 0x7F);
        } else {
            getMdl()->setPriorityDraw(2, 8);
        }
        getMdl()->entry();
    } else {
        dItem::Item item = getItem();
        mVec3_c pos(mPos);
        mVec3_c ofs(sItemOfs);
        pos += ofs;
        mAng3_c angle(0, 0, 0);
        fn_111_6770(item.mId, &pos, &mScale, &angle, 0);
    }
    return TRUE;
}

// 0x68CC
BOOL dFishFldShadow_c::isDeleteOk() {
    if (mHoldPlayer != -1) {
        fn_801909D4(lbl_8074E9E8, mHoldPlayer);
        mHoldPlayer = -1;
    }
    if (mPlayer >= 0) {
        fn_801710BC(mPlayer, 0);
        mPlayer = -1;
    }
    if (_20C != NULL) {
        *_20C = NULL;
        _20C = NULL;
    }
    mMdl.remove();
    mAnm.remove();
    while (mLinks != NULL) {
        removeLink(mLinks);
    }
    return TRUE;
}

// 0x698C
void dFishFldShadow_c::addLink(dFishLink_c *link) {
    dFishLink_c **p = &mLinks;
    while (*p != NULL) {
        p = &(*p)->mNext;
    }
    *p = link;
    link->mNext = NULL;
    link->mFish = this;
}

// 0x69B8
void dFishFldShadow_c::removeLink(dFishLink_c *link) {
    dFishLink_c **p = &mLinks;
    while (*p != link) {
        p = &(*p)->mNext;
    }
    if (*p == link) {
        *p = link->mNext;
        link->mNext = NULL;
        link->mFish = NULL;
    }
}

// 0x69EC
void dFishFrog_c::init() {
    mAlarm = 0;
    mCroakTimer = 0;
    mCroakWait = 0;
    mState = 0;
}

// 0x6A04
void dFishFrog_c::execute(dFishFldShadow_c *fish) {
    switch (mState) {
    case 0:
        execAway(fish);
        break;
    case 1:
        execNear(fish);
        break;
    case 2:
        execCroak(fish);
        break;
    }
}

// 0x6A3C
void dFishFrog_c::execAway(dFishFldShadow_c *fish) {
    if (mAlarm != 0) {
        mAlarm -= 3;
        if (mAlarm < 10) {
            mAlarm = 0;
        }
    }
    if (mCroakWait != 0) {
        mCroakWait--;
    }
    dPlayerActor_c *player = fn_800FBC7C(4);
    if (player != NULL) {
        mVec3_c pos = player->mPos;
        f32 dist = (fish->mPos - pos).xzLen();
        if (128.0f > dist) {
            mState = 1;
        }
    }
}

// 0x6B14
void dFishFrog_c::execNear(dFishFldShadow_c *fish) {
    if (mCroakWait != 0) {
        mCroakWait--;
    }
    dPlayerActor_c *player = fn_800FBC7C(4);
    if (player != NULL) {
        mVec3_c pos = player->mPos;
        f32 dist = (fish->mPos - pos).xzLen();
        if (128.0f <= dist) {
            mState = 0;
            return;
        }
    }
    if (fn_8010094C(4)) {
        if (++mAlarm >= 200) {
            mAlarm = 200;
        }
    } else {
        if (mAlarm != 0) {
            mAlarm--;
        }
        if (mAlarm <= 150 && mCroakWait == 0) {
            mCroakTimer = cM::rndRange<s16>(180, 420);
            mState = 2;
        }
    }
}

// 0x6C44
void dFishFrog_c::execCroak(dFishFldShadow_c *fish) {
    if (mCroakTimer != 0) {
        mCroakTimer--;
        fish->mSound.holdSound(0x18C5);
    } else {
        mCroakWait = cM::rndRange<s16>(60, 240);
        mState = 1;
    }
    if (fn_8010094C(4)) {
        mCroakWait = cM::rndRange<s16>(60, 120);
        mState = 1;
        mCroakTimer = 0;
        if (++mAlarm >= 200) {
            mAlarm = 200;
        }
    } else if (mAlarm != 0) {
        mAlarm--;
    }
}

// 0x6D10
void dFishRipple_c::init() {
    mCount = cM::rndRange(3, 4) + 1;
    mFirst = 1;
    mTimer = 0;
}

// 0x6D5C
void dFishRipple_c::execute() {
    if (sLib::calcTimer(&mTimer) != 0) {
        return;
    }
    if (sLib::calcTimer(&mCount) == 0) {
        mFirst = 0;
        mCount = cM::rndRange(1, 1) + 1;
        mTimer = cM::rndRange(150, 300);
        return;
    }
    f32 min = sRippleScaleMin[mOwner->mParam.mSize];
    f32 max = sRippleScaleMax[mOwner->mParam.mSize];
    s32 lo, hi;
    if (!mFirst) {
        lo = 30;
        hi = 30;
    } else {
        lo = 8;
        hi = 16;
    }
    mTimer = cM::rndRange(lo, hi);
    const dFishSizeParam_c *param = &sSizeParams[mOwner->mParam.mSize];
    mVec3_c pos = mOwner->mPos;
    mVec3_c ofs(0.0f, 0.0f, -20.0f * param->mScale[2]);
    ofs.rotY(mOwner->mTargetAngle.y);
    pos += ofs;
    dBGCF::groundChk_c check(&pos, dBGCF::LAYER_WATER, 0, 0);
    f32 h = check.mWaterY;
    pos.y = h;
    mVec3_c scale(param->mScale[0], param->mScale[0], param->mScale[0]);
    f32 r = cM::rndRange(min, max);
    scale *= r;
    f32 range = 0.0f;
    pos.x += cM::rndRange(-range, range);
    pos.z += cM::rndRange(-range, range);
    fn_80087790("af_fsh_hamon", &pos, 0, &scale);
}

// 0x6F8C
dFishShadowMdl_c::dFishShadowMdl_c() : mCallback(this) {
    _14 = 0;
    _18 = 0;
    mAngle = 0;
    _20.set(0.0f, 0.0f, 0.0f);
}

// 0x6FF8
dFishShadowMdl_c::~dFishShadowMdl_c() {}

// 0x7060
BOOL dFishShadowMdl_c::create(nw4r::g3d::ResMdl mdl, mAllocator_c *allocator, ulong bufferOption, int viewCount,
                              size_t *objSize) {
    if (!smdl_c::create(mdl, allocator, bufferOption, viewCount, objSize)) {
        return FALSE;
    }
    nw4r::g3d::ScnMdlSimple *scn = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    scn->SetScnMdlCallback(&mCallback);
    scn->EnableScnMdlCallbackTiming(nw4r::g3d::ScnObj::CALLBACK_TIMING_B);
    scn->SetScnMdlCallbackNodeID(2);
    return TRUE;
}

// 0x7128
void dFishShadowMdl_c::calcRoot(nw4r::g3d::WorldMtxManip *manip, u16 nodeID) {
    mMtx_c mtx;
    manip->GetMatrix(&mtx);
    mMtx_c rot;
    rot.YrotS(mAngle);
    mtx.concat(rot);
    manip->SetMatrix(mtx);
}

// 0x71A0
dFishShadowMdl_c::mdlCallback_c::mdlCallback_c(dFishShadowMdl_c *owner) : mOwner(owner) {}

// 0x71B4
dFishShadowMdl_c::mdlCallback_c::~mdlCallback_c() {}

// 0x71F4
void dFishShadowMdl_c::mdlCallback_c::ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl mdl,
                                                    nw4r::g3d::FuncObjCalcWorld *funcObj) {
    mOwner->calcRoot(manip, funcObj->GetNodeID());
}

// 0x7200
void *dFishFldShadow_c::operator new(size_t size, EGG::Heap *heap) {
    void *p = EGG::Heap::alloc(size, 4, heap);
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

// 0x7260
void dFishFldShadow_c::splashCallback(dEffectTarget_c *target, u32 kind) {
    static const char *const sNames[] = {"af_hny_hamon_S01", "afi_hny_watersplash_b"};
    if (target == NULL) {
        return;
    }
    mVec3_c pos;
    mVec3_c src;
    src = target->_AC;
    dWorld::toFieldPosition(&pos, &src);
    dBGCF::groundChk_c check(&pos, dBGCF::LAYER_WATER, 0, 0);
    if (check.mWater != BG_WATER_NONE) {
        if (check.isUnderWater(pos.y)) {
            if (check.mWaterY - pos.y < 5.0f) {
                pos.y = check.mWaterY;
                fn_80087790(sNames[kind], &pos, 0, NULL);
            }
            if (target->_C8 != NULL) {
                fn_80285110(target->_C8, target);
            }
        }
    } else {
        f32 ground = check.getHeight(0);
        f32 d = ground - pos.y;
        if (d > 0.0f) {
            if (kind == 1 && d < 5.0f) {
                pos.y = ground;
                fn_80087790("afi_hny_watersplash_a", &pos, 0, NULL);
            }
            if (target->_C8 != NULL) {
                fn_80285110(target->_C8, target);
            }
        }
    }
}

// 0x73BC
void dFishFldShadow_c::setSplashEffect() {
    static const char *const sColumnNames[] = {
        "af_fsh_watercolumn_SS", "af_fsh_watercolumn_S",   "af_fsh_watercolumn_M", "af_fsh_watercolumn_L",
        "af_fsh_watercolumn_LL", "af_fsh_watercolumn_LLL", "af_fsh_watercolumn_J", "af_fsh_watercolumn__U",
    };
    static const char *const sDropNames[] = {
        "af_hny_mizutama_S", "af_hny_mizutama_S", "af_hny_mizutama_M", "af_hny_mizutama_M",
        "af_hny_mizutama_M", "af_hny_mizutama_L", "af_hny_mizutama_L", "af_hny_mizutama_M",
    };
    static const f32 sDropScales[] = {1.0f, 1.0f, 0.9f, 1.2f, 1.4f, 1.1f, 1.3f, 1.0f};
    u32 kind;
    switch (mParam.mSize) {
    case 5:
    case 6:
        kind = 1;
        break;
    default:
        kind = 0;
        break;
    }
    mVec3_c pos = mPos;
    dBGCF::groundChk_c check(&pos, dBGCF::LAYER_WATER, 0, 0);
    pos.y = check.mWaterY;
    fn_80087790(sColumnNames[mParam.mSize], &pos, 0, NULL);
    f32 s = sDropScales[mParam.mSize];
    mVec3_c scale(s, s, s);
    fn_80087844(sDropNames[mParam.mSize], &pos, 0, &scale, splashCallback, kind);
}

// 0x74C0
void dFishFldShadow_c::setEscapeEffect() {
    static const f32 sScales[] = {0.7f, 0.8f, 1.0f, 1.1f, 1.2f, 1.3f, 1.6f, 1.0f};
    f32 s = sScales[mParam.mSize];
    mVec3_c scale(s, s, s);
    fn_80087790("af_fsh_escape_hamon", &mPos, 0, &scale);
}

// 0x7518
BOOL dFishField_c::isRecMine(int idx) {
    if (!fn_800DCEDC()) {
        return TRUE;
    }
    dPlaySyncRec_c *rec = &mRecs[idx].mRec;
    int member = rec->getMember();
    if (member == fn_800DCF58()) {
        return TRUE;
    }
    BOOL free = FALSE;
    if (member == 4 && rec->getState() != PLAY_SYNC_STATE_CLAIMED) {
        free = TRUE;
    }
    BOOL ret = FALSE;
    if (free && fn_800DD960()) {
        ret = TRUE;
    }
    return ret;
}

// 0x75C4
void dFishField_c::clearRecs() {
    for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
        mRecs[i].mRec.setA(FISH_TYPE_NUM);
        mRecs[i].mRec.setC(0);
        mRecs[i].mRec.setMember(4);
    }
}

// 0x7698
void dFishField_c::sendRecs() {
    if (fn_800DCEDC()) {
        for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
            if (isRecMine(i) && mRecs[i].mRec.getState() != PLAY_SYNC_STATE_CLAIMED) {
                if (mRecs[i].mReleasePending) {
                    mRecs[i].mRec.setMember(4);
                    claimSyncRec(&mRecs[i].mRec, i);
                    mRecs[i].mReleasePending = 0;
                }
                if (mRecs[i].mClaimMember >= 0) {
                    mRecs[i].mRec.setMember(mRecs[i].mClaimMember);
                    sendSyncRecClaim(&mRecs[i].mRec, i);
                    mRecs[i].mClaimMember = -1;
                }
                fn_800DD5F8(i + PLAY_SYNC_REC_ID, &mRecs[i].mRec, 0);
            }
        }
    }
}

// 0x7788
void dFishField_c::recvRecs() {
    if (fn_800DCEDC()) {
        for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
            if (isRecMine(i)) {
                if (mRecs[i].mRemote && mFish[i] != NULL) {
                    mFish[i]->recvRec();
                }
                mRecs[i].mRemote = 0;
            } else {
                mRecs[i].mRemote = 1;
                cLib::memCpy(&mRecs[i].mRec, fn_800DD64C(i + PLAY_SYNC_REC_ID), sizeof(dPlaySyncRec_c));
            }
        }
    }
}

// 0x7848
void dFishField_c::initRecs() {
    if (fn_800DCEDC()) {
        for (int i = 0; i < FISH_FIELD_FISH_NUM; i++) {
            if (isRecMine(i)) {
                mRecs[i].mRemote = 0;
            } else {
                mRecs[i].mRemote = 1;
            }
            cLib::memCpy(&mRecs[i].mRec, fn_800DD64C(i + PLAY_SYNC_REC_ID), sizeof(dPlaySyncRec_c));
        }
    }
}
