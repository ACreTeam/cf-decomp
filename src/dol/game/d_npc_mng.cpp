// The npc manager (dNpcMng_c): spawns each scene's villagers and special npcs.
// .text 801A6468..801AD6E0, .ctors 8046583C, .rodata 8047DB98..8047E1F0, .data
// 804FB7C8..804FB8A0, .bss 80625190..80625370, .sdata 8074C178..8074C1E0, .sbss
// 8074EAB8..8074EAD8, .sdata2 80751780..80751800. See
// include/game/game/d_npc_mng.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_a_npc_town.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_fd_block_type.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mdl_mng.hpp>
#include <game/game/d_npc_setup.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_weather.hpp>
#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d.h>
#include <revolution/OS.h>
#include <cstdio>

CUSTOM_BASE_PROFILE(NPC_MNG, dNpcMng_c, fProf::NPC_MNG,
                    fProf::DRAW_ORDER::NPC_MNG);

dNpcMng_c::dNpcMng_c() : m_halloweenModelHeap_p(NULL) {
}

dNpcMng_c::~dNpcMng_c() {
}

// Bit n: week n since the week of 2000-01-01 is a visit week (isWeekVisitDay).
static const u32 l_weekBits[] = {
    0x94A8A888, 0x494922A8, 0x22289244, 0x1444492A, 0x2888A525, 0xA44A94A5,
    0xA24952AA, 0x8A451148, 0x45454544, 0x22912224, 0x52448889, 0x49552222,
    0x5224494A, 0x912252A4, 0x29111488, 0x2A522491, 0x22248952, 0x52A8AA22,
    0x8AA88A94, 0x14524A28, 0xA5111111, 0x24492928, 0x4912A222, 0x9244A24A,
    0x8A48888A, 0x25225224, 0x8A224512, 0x44944A24, 0x22924449, 0x44924911,
    0x91222489, 0x22A2894A, 0x945288AA, 0x11114448, 0x24924449, 0x22A4A222,
    0x22548A49, 0x55488889, 0x89144491, 0x92914488, 0x89224888, 0x22AA8888,
    0x12894451, 0x28948889, 0x9128A449, 0xA2244A22, 0x4892A28A, 0x5152224A,
    0xA5245224, 0x24914448, 0x24445222, 0x28A29545, 0x22444A4A, 0x11222A91,
    0x2A491225, 0x45222AA9, 0x44894922, 0x512894A2, 0x00222492,
};

// The city's square, where its guests stand (pickCitySpots).
extern const f32 cCityMinX = 512.0f;
extern const f32 cCityMinZ = 288.0f;
extern const f32 cCityMaxX = 1024.0f;
extern const f32 cCityMaxZ = 672.0f;

int dNpcMng_c::sHideAndSeekState = HIDE_AND_SEEK_NONE;
u16 dNpcMng_c::sGraceGuest = 0xFFFF;
u16 dNpcMng_c::sTheaterGuests[2] = {0xFFFF, 0xFFFF};

dNpcMng_c *dNpcMng_c::sInstance;
EGG::FrmHeap *dNpcMng_c::m_stageHeap_p;
u8 dNpcMng_c::sResetGraceSpots;
u8 dNpcMng_c::sResetTheaterSpots;
void *dNpcMng_c::sSpecialModel;
int dNpcMng_c::sTheaterMsg;
u8 dNpcMng_c::sShowTelop;
u8 dNpcMng_c::_8074EACD;
u8 dNpcMng_c::sResetVisited;
u8 dNpcMng_c::_8074EACF;
u8 dNpcMng_c::_8074EAD0;

static mVec3_c l_graceSpot;
static mVec3_c l_theaterSpots[2];
static u16 l_guestExclude[23]; // npc indices already in town / the city

// The villager key (dItem::Item) of town villager idx.
static inline u16 getAnimalKey(int idx) {
    return 0xE000 + (idx & 0x3FF);
}

static inline bool getBuildingPos(dSaveBuildingList_c *list, int *x, int *z,
                                  const dItem::Item *item) {
    return list->getPos(x, z, *item, 1) != FALSE;
}

static inline bool hasNoBuildingPos(dSaveBuildingList_c *list, int *x, int *z,
                                    const dItem::Item *item) {
    return list->getPos(x, z, *item, 1) == FALSE;
}

// The block isn't one of the num blocks in used.
static inline BOOL isFreeBlock(int blockX, int blockZ, const s8 *used,
                               u32 num) {
    BOOL ok = TRUE;
    for (u32 i = 0; i < num; i++) {
        if (blockX == used[i * 2] && blockZ == used[i * 2 + 1]) {
            ok = FALSE;
            break;
        }
    }
    return ok;
}

static inline BOOL isNewSpot(const mVec3_c &pos, const mVec3_c *used, u32 num) {
    BOOL ok = TRUE;
    if (used != NULL) {
        for (u32 i = 0; i < num; i++) {
            if (pos == used[i]) {
                ok = FALSE;
                break;
            }
        }
    }
    return ok;
}

void dNpcMng_c::reset() {
    sHideAndSeekState = HIDE_AND_SEEK_NONE;
    sResetGraceSpots = 0;
    sResetTheaterSpots = 0;
    sShowTelop = 0;
    _8074EACF = 0;
    dNpc::clearAnimalBuffer();
}

void dNpcMng_c::createStageHeap() {
    if (m_stageHeap_p != NULL) {
        return;
    }
    u8 scene = dScene::getCurrentScene();
    u32 size;
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN)) {
        size = dAcNpcNml_c::calcHeapSize(TRUE) * 5 +
               dAcNpcSp_c::calcHeapSize(TRUE) * 3;
        u32 mdlSize = dNpcMdlMng_c::getSpMdlSize();
        size += ROUND_UP(ROUND_UP(mdlSize, 32) + mHeap::frmHeapCost(0, 32), 32);
    } else if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_CITY_FIELD)) {
        size = dAcNpcTown_c::calcHeapSize(TRUE) * 5 +
               dAcNpcSp_c::calcHeapSize(TRUE) * 3;
        size += dHmnToolMng_c::getBankHeapSize();
    } else if (scene == dScene::SCENE_RM_GRACE) {
        u32 guestNum = 1;
        size = dAcNpcTown_c::calcHeapSize(TRUE) * guestNum +
               dAcNpcSp_c::calcHeapSize(TRUE) * 4;
    } else if (scene == dScene::SCENE_RM_THEATER_SHOW) {
        size = dAcNpcTown_c::calcHeapSize(TRUE) * 2 +
               dAcNpcSp_c::calcHeapSize(TRUE) * 4;
    } else {
        size = dAcNpcNml_c::calcHeapSize(TRUE) * 2 +
               dAcNpcSp_c::calcHeapSize(TRUE) * 4;
    }
    if (size != 0) {
        m_stageHeap_p =
            mHeap::createFrmHeap(size, dHeap::localHeap_p,
                                 "dNpcMng_c::stageHeap", 0x20, mHeap::OPT_NONE);
    }
}

void dNpcMng_c::destroyStageHeap() {
    if (m_stageHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_stageHeap_p);
        m_stageHeap_p = NULL;
    }
}

void *dNpcMng_c::getSpecialModel() {
    return sSpecialModel;
}

BOOL dNpcMng_c::setHideAndSeekState(int state) {
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    if (state >= HIDE_AND_SEEK_NONE) {
        return FALSE;
    }
    sHideAndSeekState = state;
    return TRUE;
}

void dNpcMng_c::placeHidersAtTownHall() {
    static const u8 unitX[] = {1, 1, 2};
    static const u8 unitZ[] = {6, 5, 5};
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    for (int i = 0; i < 3; i++) {
        int idx = block->getHiderIdx(i);
        dAnimal_c *animal = block->getAnimalConst(idx);
        if (animal != NULL && animal->mID.isValid() && animal->mPlace == 0) {
            mVec3_c pos = mVec3_c::Zero;
            if (getTownHallPos(&pos, unitX[i], unitZ[i])) {
                u16 key = getAnimalKey(idx);
                dNpc::daub_c *daub = dNpc::getDaubByKey(&key);
                if (daub != NULL) {
                    dNpc::daub_c::packXZ((u8 *)daub, &pos);
                }
            }
        }
    }
}

BOOL dNpcMng_c::pickHideSpotBuilding(mVec3_c *pos, const s8 *usedBlocks,
                                     u32 usedNum) {
    static dItem::Item l_buildings[] = {
        BUILDING_PLAYER_HOUSE_0, BUILDING_PLAYER_HOUSE_1,
        BUILDING_PLAYER_HOUSE_2, BUILDING_PLAYER_HOUSE_3,
        BUILDING_NPC_HOUSE_0,    BUILDING_NPC_HOUSE_1,
        BUILDING_NPC_HOUSE_2,    BUILDING_NPC_HOUSE_3,
        BUILDING_NPC_HOUSE_4,    BUILDING_NPC_HOUSE_5,
        BUILDING_NPC_HOUSE_6,    BUILDING_NPC_HOUSE_7,
        BUILDING_NPC_HOUSE_8,    BUILDING_NPC_HOUSE_9,
        BUILDING_TOWN_HALL,      BUILDING_SHOP,
        BUILDING_TAILOR,         BUILDING_MUSEUM,
        BUILDING_BULLETIN_BOARD,
    };
    u32 count = 0;
    BOOL found = FALSE;
    dItem::Item *item = l_buildings;
    for (u32 i = 0; i < ARRAY_SIZE(l_buildings); i++, item++) {
        int x = 0;
        int z = 0;
        dItem::Item id = *item;
        if (getBuildingPos(dSaveBuildingList_c::get(), &x, &z, &id)) {
            int blockX = x >> 4;
            int blockZ = z >> 4;
            const s8 *p = usedBlocks;
            BOOL free = TRUE;
            for (u32 n = 0; n < usedNum; n++, p += 2) {
                if (blockX == p[0] && blockZ == p[1]) {
                    free = FALSE;
                    break;
                }
            }
            if (free) {
                for (int side = 0; side < 2; side++) {
                    mVec3_c spot;
                    if (dSTR::getEntranceSidePos(&spot, *item, side)) {
                        f32 chance = 100.0f / (count + 1);
                        if (cM::rndF(100.0f) <= chance) {
                            found = TRUE;
                            *pos = spot;
                        }
                        count++;
                    }
                }
            }
        }
    }
    return found;
}

// An npc can stand at pos (no item may be put there).
static inline bool canStandAtPos(const mVec3_c *pos, dFdBase_c *field) {
    dItem::Item none;
    return dNpc::canPutItemAt(pos, none, field, 0x44, TRUE, TRUE) != FALSE;
}

BOOL dNpcMng_c::pickHideSpotTree(mVec3_c *pos, const s8 *usedBlocks,
                                 u32 usedNum) {
    dFdBase_c *field = dField::getInfo(0);
    if (field == NULL) {
        return FALSE;
    }
    if (field->getKind() != 1) {
        return FALSE;
    }
    BOOL found;
    u32 count;
    int blockW = field->mBlockW - 1;
    int blockH = field->mBlockH - 1;
    found = FALSE;
    count = 0;
    for (int blockZ = 1; blockZ < blockH; blockZ++) {
        for (int blockX = 1; blockX < blockW; blockX++) {
            if (!isFreeBlock(blockX, blockZ, usedBlocks, usedNum)) {
                continue;
            }
            const dFdBlock_c *block =
                ((const dFdBase_c *)field)->getBlock(blockX, blockZ);
            if (block == NULL) {
                continue;
            }
            for (int unitZ = 1; unitZ < 16; unitZ++) {
                for (int unitX = 0; unitX < 16; unitX++) {
                    dItem::Item *item = block->getItem(unitX, unitZ, 0);
                    if (item == NULL || item->mId == dItem::ITEM_ID_NONE) {
                        continue;
                    }
                    dItem::FgInfo *info = item->getFgInfo();
                    if (info == NULL || info->mTreeStage < 4) {
                        continue;
                    }
                    mVec3_c spot = mVec3_c::Zero;
                    dFdBase_c::getUnitCenterPos(&spot, blockX, blockZ, unitX,
                                                unitZ - 1);
                    dItem::Item none;
                    bool ok = dNpc::canPutItemAt(&spot, none, field, 0x44, TRUE,
                                                 TRUE) != FALSE;
                    if (ok) {
                        f32 chance = 100.0f / (count + 1);
                        if (cM::rndF(100.0f) <= chance) {
                            *pos = spot;
                            mVec3_c left = spot;
                            left.x -= 14.4f;
                            mVec3_c right = spot;
                            right.x += 14.4f;
                            dItem::Item none1;
                            if (dNpc::canPutItemAt(&left, none1, field, 0x44,
                                                   TRUE, TRUE)) {
                                *pos = left;
                                dItem::Item none2;
                                BOOL useRight =
                                    dNpc::canPutItemAt(&right, none2, field,
                                                       0x44, TRUE, TRUE) &&
                                    cM::rndInt(2) == 0;
                                if (useRight) {
                                    *pos = right;
                                }
                            } else {
                                dItem::Item none2;
                                if (dNpc::canPutItemAt(&right, none2, field,
                                                       0x44, TRUE, TRUE)) {
                                    *pos = right;
                                }
                            }
                            found = TRUE;
                        }
                        count++;
                    }
                }
            }
        }
    }
    return found;
}

BOOL dNpcMng_c::pickHideSpotBuildSite(mVec3_c *pos, const s8 *usedBlocks,
                                      u32 usedNum) {
    dFdBase_c *field = getCurrentField();
    if (field == NULL) {
        return FALSE;
    }
    if (field->getKind() != 1) {
        return FALSE;
    }
    int blockW = field->mBlockW - 1;
    int blockH = field->mBlockH - 1;
    BOOL found = FALSE;
    u32 count = 0;
    for (int blockZ = 1; blockZ < blockH; blockZ++) {
        for (int blockX = 1; blockX < blockW; blockX++) {
            if (!isFreeBlock(blockX, blockZ, usedBlocks, usedNum)) {
                continue;
            }
            const dFdBlock_c *block =
                ((const dFdBase_c *)field)->getBlock(blockX, blockZ);
            if (block == NULL) {
                continue;
            }
            for (int unitZ = 1; unitZ < 16; unitZ++) {
                for (int unitX = 0; unitX < 16; unitX++) {
                    dItem::Item *item = block->getItem(unitX, unitZ, 0);
                    if (item == NULL) {
                        continue;
                    }
                    int x = (blockX << 4) + unitX;
                    int z = (blockZ << 4) + unitZ;
                    if (!dSaveBuildingList_c::get()->isBuildSite(x, z)) {
                        continue;
                    }
                    if (dSaveBuildingList_c::get()->getAt(x, z, 1).mId !=
                        dItem::ITEM_ID_NONE) {
                        continue;
                    }
                    item->getFgInfo();
                    mVec3_c spot = mVec3_c::Zero;
                    dFdBase_c::getUnitCenterPos(&spot, blockX, blockZ, unitX,
                                                unitZ - 1);
                    if (canStandAtPos(&spot, field)) {
                        f32 chance = 100.0f / (count + 1);
                        if (cM::rndF(100.0f) <= chance) {
                            found = TRUE;
                            *pos = spot;
                        }
                        count++;
                    }
                }
            }
        }
    }
    return found;
}

void dNpcMng_c::hideHiders() {
    static BOOL (dNpcMng_c::*const l_pickSpot[])(mVec3_c *, const s8 *, u32) = {
        &dNpcMng_c::pickHideSpotBuilding,
        &dNpcMng_c::pickHideSpotTree,
        &dNpcMng_c::pickHideSpotBuildSite,
    };
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    s8 used[3][2] = {0};
    u32 usedNum = 0;
    for (int i = 0; i < 3; i++) {
        int idx = block->getHiderIdx(i);
        dAnimal_c *animal = block->getAnimalConst(idx);
        if (animal == NULL || !animal->mID.isValid() || animal->mPlace != 0) {
            continue;
        }
        u8 weights[3] = {45, 45, 10};
        u32 total = 100;
        mVec3_c pos = mVec3_c::Zero;
        while (total != 0) {
            u32 r = cM::rndInt(total);
            u32 kind = 3;
            for (u32 k = 0; k < 3; k++) {
                if (weights[k] != 0) {
                    if (r < weights[k]) {
                        kind = k;
                        break;
                    }
                    r -= weights[k];
                }
            }
            if (kind >= 3) {
                break;
            }
            if ((this->*l_pickSpot[kind])(&pos, used[0], usedNum)) {
                u16 key = getAnimalKey(idx);
                dNpc::daub_c *daub = dNpc::getDaubByKey(&key);
                if (daub != NULL) {
                    dNpc::daub_c::packXZ((u8 *)daub, &pos);
                    mAng angle = 0;
                    daub->setAngleY((u16 *)&angle);
                }
                if (usedNum < 3) {
                    int xi = pos.x;
                    // Keep the second conversion in a compiler temporary.
                    struct {
                        int value;
                    } zi = {0};
                    zi.value = pos.z;
                    used[usedNum][0] = xi >> 9;
                    used[usedNum][1] = zi.value >> 9;
                    usedNum++;
                }
                break;
            }
            if (weights[kind] > total) {
                break;
            }
            total -= weights[kind];
            weights[kind] = 0;
        }
    }
}

void dNpcMng_c::updateHideAndSeek() {
    static void (dNpcMng_c::*const l_steps[])() = {
        &dNpcMng_c::placeHidersAtTownHall,
        &dNpcMng_c::hideHiders,
        &dNpcMng_c::placeHidersAtTownHall,
    };
    if (sHideAndSeekState < HIDE_AND_SEEK_NONE) {
        if (l_steps[sHideAndSeekState] != NULL) {
            (this->*l_steps[sHideAndSeekState])();
        }
        sHideAndSeekState = HIDE_AND_SEEK_NONE;
    }
}

BOOL dNpcMng_c::createVillagers() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    u32 num = 0;
    u8 scene = dScene::getCurrentScene();
    BOOL online = dNpc::isPermitSceneOwner();
    for (int i = 0; i < 10 && num < 5; i++) {
        dAnimal_c *animal = block->getAnimal(i);
        if (animal == NULL || !animal->mID.isValid() ||
            animal->mDayPlace != 0) {
            continue;
        }
        mVec3_c pos = mVec3_c::Zero;
        mAng3_c angle = mAng3_c::Zero;
        BOOL placed = FALSE;
        u16 key = getAnimalKey(i);
        const dNpc::daub_c *recv = dNpc::getReceivedDaub(&key);
        if (recv != NULL) {
            dNpc::daub_c daub(*recv);
            if (daub.mValid) {
                if (scene == daub.mScene) {
                    dNpc::daub_c::unpackXZ((const u8 *)&daub, &pos);
                    angle.y = daub.getAngleY();
                } else {
                    block->getHouseFrontPos(&pos, i);
                }
                placed = TRUE;
            }
        }
        if (!placed && online) {
            dNpc::daub_c *daub = dNpc::getDaubByKey(&key);
            if (daub != NULL && daub->mValid) {
                if (daub->mScene == scene) {
                    dNpc::daub_c::unpackXZ((const u8 *)daub, &pos);
                    angle.y = daub->getAngleY();
                } else {
                    block->getHouseFrontPos(&pos, i);
                }
                placed = TRUE;
            }
        }
        BOOL blocked = FALSE;
        BOOL check = FALSE;
        if (placed && online && animal->mPlace == 0) {
            check = TRUE;
        }
        if (check) {
            // Post-increment preserves MWCC's argument evaluation order.
            BOOL checkA = TRUE;
            dItem::Item none;
            if (!dNpc::canPutItemAt(&pos, none, NULL, 0x44, FALSE, checkA++)) {
                blocked = TRUE;
            }
        }
        if (blocked) {
            mVec3_c free = pos;
            if (dNpc::pickNearbyFreePos(&free, &pos, 2)) {
                pos = free;
            } else {
                block->getHouseFrontPos(&pos, i);
            }
        }
        if (!placed) {
            block->getHouseFrontPos(&pos, i);
        }
        if (online) {
            block->updateHalloweenFlag(i);
            block->updateHeldItem(i);
        }
        dActor_c::construct(fProf::A_NPC_OUT, this, key, &pos, &angle);
        num++;
    }
    return TRUE;
}

BOOL dNpcMng_c::pickHouseVillagerPos(mVec3_c *pos) {
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        return FALSE;
    }
    dFdBase_c *field = dField::getInfo(0);
    if (field == NULL) {
        return FALSE;
    }
    u16 used[16];
    memset(used, 0, sizeof(used));
    used[8] |= 0x420;
    u16 *row = used;
    for (u32 z = 0; z < 14; z++, row++) {
        for (int x = 0; x < 16; x++) {
            BOOL blocked = FALSE;
            if (!(*row >> x & 1)) {
                dItem::Item none;
                if (!dNpc::canPutItemOnUnit(x, z, &none, field, 0x44, TRUE,
                                            TRUE)) {
                    blocked = TRUE;
                }
            }
            if (!blocked) {
                continue;
            }
            *row |= 1 << x;
            dItem::Item *item = field->getItem(x, z, 0);
            if (item == NULL || item->mId == dItem::ITEM_ID_NONE) {
                continue;
            }
            dFTR::shape_c shape;
            shape.set(item->mId);
            for (u32 i = 0; i < shape.getNum(); i++) {
                const int *ofs = shape.getOfs(i);
                u32 unitX = x + ofs[0];
                u32 unitZ = z + ofs[1];
                if (unitX < 16 && unitZ < 16) {
                    used[unitZ] |= 1 << unitX;
                }
            }
        }
    }
    u16 *freeRow = used;
    int pickX = -1;
    int pickZ = -1;
    u32 count = 0;
    for (u32 z = 0; z < 14; z++, freeRow++) {
        for (int x = 0; x < 16; x++) {
            if (!(*freeRow >> x & 1)) {
                f32 chance = 100.0f / (count + 1);
                if (cM::rndF(100.0f) <= chance) {
                    pickX = x;
                    pickZ = z;
                }
                count++;
            }
        }
    }
    if (count != 0) {
        dFdBase_c::getUnitCenterPos(pos, pickX, pickZ);
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::createHouseVillager() {
    u8 scene = dScene::getCurrentScene();
    if (!dScene::isSceneAttr(scene, dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        return;
    }
    dAnimalBlock_c *block;
    dQuestPlayerItem_c *appointment;
    int idx =
        dScene::getSceneAttrIndex(scene, dScene::SCENE_ATTR_VILLAGER_HOUSE);
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    dAnimal_c *animal = block->getAnimal(idx);
    if (animal == NULL) {
        return;
    }
    if (!animal->mID.isValid()) {
        return;
    }
    u16 profile = fProf::A_NPC_IN;
    u16 key = getAnimalKey(idx);
    if (!dNet::isSessionActive() && block->isAppointmentAnimal(&animal->mID)) {
        appointment = &block->mAppointment;
        int kind = appointment->mBase.getKind();
        u8 state = appointment->mBase.getState();
        if (kind == 0x11 && state != 2) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
            if (player != NULL && player->mPID.isFromTown() &&
                player->mPID.player.isSame(&appointment->mPlayer)) {
                profile = fProf::A_NPC_VISIT_NPC_HOUSE;
            }
        }
    } else if (animal->mPlace == 0 &&
               dPlayerMgr::Mgr_c::getCurrentPlayer() != NULL &&
               !dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
                   PRIVATE_FLAG0_PART_TIME_JOB) &&
               dEvent::isActive(EVENT_TOY_DAY)) {
        profile = fProf::A_NPC_SP_JINGLE;
        key = 0x8052 + idx;
    }
    mVec3_c pos = mVec3_c::Zero;
    mAng3_c angle = mAng3_c::Zero;
    BOOL placed = FALSE;
    if (dNpc::isPermitSceneOwner()) {
        pickHouseVillagerPos(&pos);
        angle.y = cM::rndInt(0xFFFF);
        placed = TRUE;
    }
    const dNpc::daub_c *recv = !placed ? dNpc::getReceivedDaub(&key) : NULL;
    if (recv != NULL) {
        dNpc::daub_c daub(*recv);
        if (daub.mValid) {
            dNpc::daub_c::unpackXZ((const u8 *)&daub, &pos);
            angle.y = daub.getAngleY();
            placed = TRUE;
        }
    }
    if (!placed) {
        pickHouseVillagerPos(&pos);
        placed = TRUE;
    }
    if (placed) {
        dActor_c::construct(profile, this, key, &pos, &angle);
    }
}

BOOL dNpcMng_c::createPlayerHouseGuest() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        return FALSE;
    }
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    BOOL fromTown =
        dScene::isSceneAttr(dScene::getPrevScene(), dScene::SCENE_ATTR_TOWN);
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    int idx = block->pickFleaMarketAnimal(fromTown, NULL, NULL);
    dAnimal_c *animal = block->getAnimalConst(idx);
    if (animal != NULL && animal->mID.isValid()) {
        mVec3_c pos(256.0f, 0.0f, 464.0f);
        mAng3_c angle(0, -0x8000, 0);
        pos.z += 64.0f;
        dAnimalMemory_c *memory =
            player != NULL ? animal->findMemory2(&player->mPID) : NULL;
        if (memory != NULL && memory->isEventFlag(0)) {
            if (!dNpc::pickSceneFreePos(&pos, NULL, 4, NULL, 0.0f)) {
                pos.set(256.0f, 0.0f, 464.0f);
            }
        }
        if (dActor_c::construct(fProf::A_NPC_VISIT_PLAYER_HOUSE_FLEAMARKET,
                                this, getAnimalKey(idx), &pos,
                                &angle) != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL dNpcMng_c::createAppointmentGuest() {
    dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
    u8 state;
    dAnimal_c *animal;
    dQuestBase_c *quest;
    int idx = block->getAppointmentAnimalIdx();
    animal = block->getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        quest = &block->mAppointment.mBase;
        state = quest->mState;
        if (quest->getKind() == 0x12 && state != 2) {
            if (dNet::isSessionActive()) {
                quest->mState = 1;
            } else {
                mVec3_c pos(256.0f, 0.0f, 464.0f);
                mAng3_c angle(0, -0x8000, 0);
                pos.z += 64.0f;
                dItem::Item key(getAnimalKey(idx));
                if (state == 1) {
                    quest->mState = 0;
                }
                if (dActor_c::construct(fProf::A_NPC_VISIT_PLAYER_HOUSE, this,
                                        key.mId, &pos, &angle) != NULL) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void dNpcMng_c::createPlayerHouseVisitor() {
    if (createPlayerHouseGuest()) {
        return;
    }
    if (createAppointmentGuest()) {
        return;
    }
}

BOOL dNpcMng_c::createAnimals() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        updateHideAndSeek();
        createVillagers();
    } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        createHouseVillager();
    } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE)) {
        u8 scene = dScene::getCurrentScene();
        int room = -1;
        u32 home = dHomeList_c::getHomeFromScene(&room, scene);
        if (home < 4 && room == 0) {
            int player = dSaveData_c::getRaw()->mHomes.getPlayerOfHome(home);
            if ((u32)player < 4 &&
                player == dPlayerMgr::getCurrentPlayerSlot()) {
                createPlayerHouseVisitor();
            }
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::getTownHallPos(mVec3_c *pos, int unitX, int unitZ) {
    dSaveTown_c *town = dSaveData_c::getTown();
    int x = 0;
    int z = 0;
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    if (getBuildingPos(&town->mBuilding.mList, &x, &z, &hall)) {
        dFdBase_c::getUnitCenterPos(pos, x + unitX, z + unitZ);
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosTownHallA(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        getTownHallPos(pos, 0, 4);
        angle->set(0, 0, 0);
    }
}

void dNpcMng_c::setPosTownHallRight(mVec3_c *pos, mAng3_c *angle,
                                    const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        getTownHallPos(pos, 1, 6);
        angle->set(0, 0, 0);
    }
}

void dNpcMng_c::setPosTownHallB(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        getTownHallPos(pos, 0, 4);
        angle->set(0, 0, 0);
    }
}

void dNpcMng_c::setPosTownHallC(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        getTownHallPos(pos, 0, 4);
        angle->set(0, 0, 0);
    }
}

void dNpcMng_c::setPosTownHallLeft(mVec3_c *pos, mAng3_c *angle,
                                   const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        getTownHallPos(pos, -2, 6);
        angle->set(0, 0, 0);
    }
}

void dNpcMng_c::setPosRandomSpot(mVec3_c *pos, mAng3_c *angle,
                                 const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, TRUE)) {
        dNpc::pickOutdoorFreePos(pos, NULL, 4, NULL, 0.0f);
        angle->y = cM::rndInt(0xFFFF);
    }
}

void dNpcMng_c::setPosRandomPlace(mVec3_c *pos, mAng3_c *angle,
                                  const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, TRUE)) {
        dNpc::pickPosWithNorthClearance(pos);
        angle->y = cM::rndInt(0xFFFF);
    }
}

void dNpcMng_c::setPosHarvest(mVec3_c *pos, mAng3_c *angle,
                              const dItem::Item *npc) {
    if (!getDaubPos(pos, angle, npc, FALSE)) {
        if (npc->isSame(dItem::Item((u16)0x801E))) {
            dNpc::pickOutdoorPlacementPos(pos, NULL, 4, NULL, 0x400, FALSE,
                                          TRUE, 0.0f);
        } else {
            dNpc::pickOutdoorPlacementPos(pos, NULL, 4, NULL, 0, FALSE, TRUE,
                                          0.0f);
        }
        angle->y = cM::rndInt(0xFFFF);
    }
}

BOOL dNpcMng_c::getDaubPos(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc,
                           BOOL checkPut) {
    u8 scene = dScene::getCurrentScene();
    BOOL found = FALSE;
    const dNpc::daub_c *recv = dNpc::getReceivedDaub(&npc->mId);
    if (recv != NULL) {
        dNpc::daub_c daub(*recv);
        if (daub.mValid && scene == daub.mScene) {
            dNpc::daub_c::unpackXZ((const u8 *)&daub, pos);
            angle->y = daub.getAngleY();
            found = TRUE;
        }
    }
    BOOL online = dNpc::isPermitSceneOwner();
    if (!found && online) {
        dNpc::daub_c *daub = dNpc::getDaubByKey(&npc->mId);
        if (daub != NULL && daub->mValid && daub->mScene == scene) {
            mVec3_c spot = mVec3_c::Zero;
            dNpc::daub_c::unpackXZ((const u8 *)daub, &spot);
            BOOL blocked = FALSE;
            if (checkPut) {
                // Post-increment preserves MWCC's argument evaluation order.
                BOOL checkA = TRUE;
                dItem::Item none;
                if (!dNpc::canPutItemAt(&spot, none, NULL, 0x44, FALSE,
                                        checkA++)) {
                    blocked = TRUE;
                }
            }
            if (blocked) {
                mVec3_c free = spot;
                if (dNpc::pickNearbyFreePos(&free, &spot, 2)) {
                    *pos = free;
                    angle->y = daub->getAngleY();
                    found = TRUE;
                }
            } else {
                *pos = spot;
                angle->y = daub->getAngleY();
                found = TRUE;
            }
        }
    }
    return found;
}

BOOL dNpcMng_c::getDaubPos(mVec3_c *pos, mAng3_c *angle,
                           const dItem::Item *npc) {
    return getDaubPos(pos, angle, npc, FALSE);
}

BOOL dNpcMng_c::isTownScene(u8 scene, u8 prevScene) {
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN) &&
        !dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN |
                                        dScene::SCENE_ATTR_DEMO) &&
        !dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN |
                                        dScene::SCENE_ATTR_CHECK_FIELD)) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isTownPlay(u8 scene, u8 prevScene) {
    if (isTownScene(scene, dScene::SCENE_NUM)) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player != NULL && !player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
            return TRUE;
        }
    }
    return FALSE;
}

const dNpcMng_c::Spawn_c *dNpcMng_c::findSpawn(const Spawn_c *list, u32 num,
                                               u8 scene, u8 prevScene) {
    BOOL ok;
    BOOL eventOK;
    BOOL checkOK;
    for (u32 i = 0; i < num; i++, list++) {
        ok = FALSE;
        eventOK = TRUE;
        if (list->mEvent != 0xFFFF) {
            if (!dEvent::isActive((dQuestEvent_e)list->mEvent)) {
                eventOK = FALSE;
            }
        }
        if (eventOK) {
            checkOK = TRUE;
            if (list->mCheck != NULL && !list->mCheck(scene, prevScene)) {
                checkOK = FALSE;
            }
            if (checkOK) {
                ok = TRUE;
            }
        }
        if (ok) {
            return list;
        }
    }
    return NULL;
}

int dNpcMng_c::findSpawnIdx(const dItem::Item *npc, const Spawn_c *list,
                            u32 num) {
    if (list == NULL || num == 0) {
        return -1;
    }
    for (u32 i = 0; i < num; i++, list++) {
        if (npc->isSame(dItem::Item(list->mNpc))) {
            return i;
        }
    }
    return -1;
}

void dNpcMng_c::createSpecialNpcs() {
    u32 num = 0;
    if (createEventNpc()) {
        num = 1;
    }
    if (createHolidayNpc()) {
        num++;
    }
    if (createVisitorNpc()) {
        num++;
    }
    if (num < 3) {
        u8 prevScene = dScene::getPrevScene();
        if (hasFountain(dScene::getCurrentScene(), prevScene)) {
            mVec3_c pos = mVec3_c::Zero;
            mAng3_c angle = mAng3_c::Zero;
            dItem::Item npc((u16)0x804F);
            setPosFountain(&pos, &angle, &npc);
            dActor_c::construct(fProf::A_NPC_SP_GODDESS, this, 0x804F, &pos,
                                &angle);
        }
    }
}

void dNpcMng_c::clearIdList(u16 *list, u32 num) {
    for (int i = 0; i < num; i++) {
        *list++ = 0xFFFF;
    }
}

void dNpcMng_c::addId(u16 *list, u32 num, u16 id) {
    if (findId(id, list, num) == -1) {
        for (u32 i = 0; i < num; i++, list++) {
            if (*list == 0xFFFF) {
                *list = id;
                break;
            }
        }
    }
}

int dNpcMng_c::findId(u16 id, const u16 *list, u32 num) {
    if (id == 0xFFFF) {
        return -1;
    }
    if (list == NULL || num == 0) {
        return -1;
    }
    for (u32 i = 0; i < num; i++, list++) {
        if (id == *list) {
            return i;
        }
    }
    return -1;
}

u32 dNpcMng_c::getLooksToday(u8 *looks, const dTime_c *time) {
    u8 dummy = 0;
    u32 num = 0;
    if (looks == NULL) {
        looks = &dummy;
    }
    if (time == NULL) {
        time = dTime_c::getCurrent();
    }
    *looks = 0;
    for (int i = 0; i < 6; i++) {
        if (dNpc::isAnimalDaytime(i, time)) {
            *looks |= 1 << i;
            num++;
        }
    }
    return num;
}

dAnimalTemplate_c *dNpcMng_c::pickTemplate(u8 looks, const u16 *exclude,
                                           u32 num) {
    u32 count = 0;
    dAnimalTemplate_c *list = dNpcSetup::getList(&count);
    if (count == 0 || list == NULL) {
        return NULL;
    }
    if (looks == 0) {
        return NULL;
    }
    dAnimalTemplate_c *pick = NULL;
    u32 n = 0;
    for (u32 i = 0; i < count; i++, list++) {
        int looksBit = list->mLooks;
        if (list->mIsStarter && (looks >> looksBit & 1) &&
            findId((int)list->mNpcIdx, exclude, num) == -1) {
            f32 chance = 100.0f / (n + 1);
            if (cM::rndF(100.0f) <= chance) {
                pick = list;
            }
            n++;
        }
    }
    return pick;
}

void dNpcMng_c::setupCityGuests() {
    u16 *grace = &sGraceGuest;
    clearIdList(grace, 1);
    u16 *theater = sTheaterGuests;
    clearIdList(theater, 2);
    u16 *exclude = l_guestExclude;
    clearIdList(exclude, ARRAY_SIZE(l_guestExclude));
    dAnmPersonalID_c *id;
    dAnimalSave_c *animals = &dSaveData_c::getRaw()->mAnimals;
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = animals->mTown.getAnimalConst(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                addId(exclude, ARRAY_SIZE(l_guestExclude), id->mNpcIdx);
            }
        }
    }
    dMovedAnimalList_c *moved = &animals->mMoved;
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = moved->getAnimalConst(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                addId(exclude, ARRAY_SIZE(l_guestExclude), id->mNpcIdx);
            }
        }
    }
    dTime_c *now = dTime_c::getCurrent();
    u8 looks = 0;
    u32 looksNum = getLooksToday(&looks, now);
    u32 num = cM::rndInt(2);
    if (num > looksNum) {
        num = looksNum;
    }
    for (u32 i = 0; i < num; i++, grace++) {
        dAnimalTemplate_c *tmpl =
            pickTemplate(looks, exclude, ARRAY_SIZE(l_guestExclude));
        if (tmpl == NULL) {
            break;
        }
        s16 npcIdx = tmpl->mNpcIdx;
        *grace = npcIdx;
        if ((u16)npcIdx == 0xFFFF) {
            break;
        }
        addId(exclude, ARRAY_SIZE(l_guestExclude), npcIdx);
        if (tmpl->mLooks >= 6) {
            break;
        }
        looks &= ~(1 << tmpl->mLooks);
    }
    sResetGraceSpots = 1;
    looks = 0;
    looksNum = getLooksToday(&looks, now);
    num = cM::rndInt(3);
    if (num > looksNum) {
        num = looksNum;
    }
    for (u32 i = 0; i < num; i++, theater++) {
        dAnimalTemplate_c *tmpl =
            pickTemplate(looks, exclude, ARRAY_SIZE(l_guestExclude));
        if (tmpl == NULL) {
            break;
        }
        s16 npcIdx = tmpl->mNpcIdx;
        *theater = npcIdx;
        if ((u16)npcIdx == 0xFFFF) {
            break;
        }
        addId(exclude, ARRAY_SIZE(l_guestExclude), npcIdx);
        if (tmpl->mLooks >= 6) {
            break;
        }
        looks &= ~(1 << tmpl->mLooks);
    }
    sResetTheaterSpots = 1;
}

BOOL dNpcMng_c::pickUnit(mVec3_c *pos, const mVec3_c *used, u32 usedNum,
                         BOOL inArea, f32 minX, f32 minZ, f32 maxX, f32 maxZ) {
    if (pos == NULL) {
        return FALSE;
    }
    dFdBase_c *field = dField::getInfo(0);
    if (field == NULL) {
        return FALSE;
    }
    int unitW = field->mUnitW;
    int unitH = field->mUnitH;
    u32 count = 0;
    for (int z = 0; z < unitH; z++) {
        for (int x = 0; x < unitW; x++) {
            mVec3_c spot = mVec3_c::Zero;
            dFdBase_c::getUnitCenterPos(&spot, x, z);
            BOOL ok = FALSE;
            BOOL in = inArea == FALSE || (spot.x > minX && spot.x < maxX &&
                                          spot.z > minZ && spot.z < maxZ);
            if (in) {
                dItem::Item none;
                if (dNpc::canPutItemOnUnit(x, z, &none, field, 0x44, TRUE,
                                           TRUE)) {
                    ok = TRUE;
                }
            }
            if (ok) {
                BOOL isNew = TRUE;
                if (used != NULL) {
                    const mVec3_c *u = used;
                    for (u32 i = 0; i < usedNum; i++, u++) {
                        if (spot == *u) {
                            isNew = FALSE;
                            break;
                        }
                    }
                }
                if (isNew) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        *pos = spot;
                    }
                    count++;
                }
            }
        }
    }
    return count != 0;
}

BOOL dNpcMng_c::pickCitySpots(mVec3_c *pos, u32 num) {
    if (pos == NULL || num == 0) {
        return FALSE;
    }
    mVec3_c *base = pos;
    for (u32 i = 0; i < num; i++, pos++) {
        switch (pickUnit(pos, base, i, TRUE, cCityMinX, cCityMinZ, cCityMaxX,
                         cCityMaxZ)) {
        case FALSE:
            return FALSE;
        }
    }
    return TRUE;
}

void dNpcMng_c::setupCityAnimalItems() {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        return;
    }
    dMovedAnimalList_c *moved = &dSaveData_c::getTown()->mAnimals.mMoved;
    BOOL rain = FALSE;
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL) {
        BOOL raining = dWeather_c::isRainKind(weather->mWeather);
        if (raining) {
            rain = TRUE;
        }
    }
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = moved->getAnimal(i);
        if (animal != NULL && animal->mID.isValid()) {
            dItem::Item item = rain ? animal->getUmbrella() : dItem::Item();
            animal->setHeldItem(&item);
        }
    }
}

BOOL dNpcMng_c::createCityAnimals() {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        return FALSE;
    }
    dLandID_c *land;
    dAnmPersonalID_c *scanID;
    dAnmPersonalID_c *spawnID;
    mVec3_c *templateSpot;
    mVec3_c *spot;
    u16 *exclude = l_guestExclude;
    clearIdList(exclude, ARRAY_SIZE(l_guestExclude));
    dAnimalSave_c *animals = &dSaveData_c::getRaw()->mAnimals;
    dAnmPersonalID_c *id;
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = animals->mTown.getAnimalConst(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                addId(exclude, ARRAY_SIZE(l_guestExclude), id->mNpcIdx);
            }
        }
    }
    dTime_c *now = dTime_c::getCurrent();
    u8 looks = 0;
    u32 looksNum = getLooksToday(&looks, now);
    dMovedAnimalList_c *moved = &animals->mMoved;
    u32 spotNum = 5;
    if (looksNum < 5) {
        spotNum = looksNum;
    }
    u32 movedMask = 0;
    u32 movedNum = 0;
    land = &dSaveData_c::getRaw()->mLandID;
    mVec3_c spots[5];
    if (!pickCitySpots(spots, 5)) {
        mVec3_c *spot = spots;
        for (u32 i = 0; i < 5; i++, spot++) {
            dFdBase_c::getUnitCenterPos(spot, i + 20, 17);
        }
    }
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = moved->getAnimalConst(i);
        if (animal == NULL) {
            continue;
        }
        scanID = &animal->mID;
        if (!scanID->isValid() || !(looks >> scanID->getLooks(1) & 1)) {
            continue;
        }
        if (scanID->mLand != *land) {
            movedMask |= 1 << i;
            movedNum++;
        }
    }
    u32 num = movedNum != 0 ? cM::rndInt(movedNum + 1) : 0;
    if (num > spotNum) {
        num = spotNum;
    }
    spot = spots;
    u32 placed = 0;
    for (u32 i = 0; i < num; i++) {
        int idx = pickRandomBit(movedMask, movedNum, 10);
        dAnimal_c *animal = moved->getAnimalConst(idx);
        if (animal != NULL) {
            spawnID = &animal->mID;
            if (spawnID->isValid()) {
                dItem::Item key((u16)(0xE000 + ((idx & 0x3FF) | 0x800)));
                // Materialize the range before the angle's zero initializer.
                struct {
                    u32 value;
                } angleRange = {0};
                angleRange.value = 0x10000;
                mAng3_c angle = mAng3_c::Zero;
                angle.y = cM::rndInt(angleRange.value - 1);
                if (dActor_c::construct(fProf::A_NPC_TOWN_OUT, this, key.mId,
                                        spot, &angle) != NULL &&
                    spotNum != 0) {
                    spotNum--;
                    spot++;
                    placed++;
                }
                if (looksNum != 0) {
                    looks &= ~(1 << spawnID->getLooks(1));
                    looksNum--;
                }
                if (movedNum != 0) {
                    movedMask &= ~(1 << idx);
                    movedNum--;
                }
            }
        }
        if (movedMask == 0 || movedNum == 0 || looksNum == 0 || looks == 0) {
            break;
        }
    }
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = moved->getAnimalConst(i);
        if (animal != NULL) {
            id = &animal->mID;
            if (id->isValid()) {
                addId(exclude, ARRAY_SIZE(l_guestExclude), id->mNpcIdx);
            }
        }
    }
    addId(exclude, ARRAY_SIZE(l_guestExclude), sGraceGuest);
    for (u32 i = 0; i < 2; i++) {
        addId(exclude, ARRAY_SIZE(l_guestExclude), sTheaterGuests[i]);
    }
    templateSpot = &spots[placed];
    for (u32 i = 0; i < spotNum; i++) {
        dAnimalTemplate_c *tmpl =
            pickTemplate(looks, exclude, ARRAY_SIZE(l_guestExclude));
        if (tmpl != NULL) {
            u16 npcIdx = (int)tmpl->mNpcIdx;
            mAng3_c angle = mAng3_c::Zero;
            angle.y = cM::rndInt(0xFFFF);
            dActor_c::construct(fProf::A_NPC_TOWN_OUT, this, npcIdx,
                                templateSpot, &angle);
            templateSpot++;
            if (looksNum != 0) {
                looksNum--;
                looks &= ~(1 << (s8)tmpl->mLooks);
            }
        }
        if (looksNum == 0 || looks == 0) {
            break;
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::pickRoomSpots(mVec3_c *pos, u32 num) {
    if (pos == NULL || num == 0) {
        return FALSE;
    }
    mVec3_c *base = pos;
    for (u32 i = 0; i < num; i++, pos++) {
        switch (pickUnit(pos, base, i, FALSE, 0.0f, 0.0f, 0.0f, 0.0f)) {
        case FALSE:
            return FALSE;
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::createGraceGuest() {
    if (dScene::getCurrentScene() != dScene::SCENE_RM_GRACE) {
        return FALSE;
    }
    if (sResetGraceSpots) {
        if (!pickRoomSpots(&l_graceSpot, 1)) {
            dFdBase_c::getUnitCenterPos(&l_graceSpot, 9, 10);
        }
        sResetGraceSpots = 0;
    }
    mVec3_c *spot = &l_graceSpot;
    u16 *guest = &sGraceGuest;
    for (u32 i = 0; i < 1; i++, guest++, spot++) {
        if (*guest != 0xFFFF) {
            mAng3_c angle = mAng3_c::Zero;
            angle.y = cM::rndInt(0xFFFF);
            dActor_c::construct(fProf::A_NPC_TOWN_GRACE, this, *guest, spot,
                                &angle);
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::pickTheaterSpot(mVec3_c *pos, const mVec3_c *used,
                                u32 usedNum) {
    static mVec3_c l_seats[] = {
        mVec3_c(176.0f, 0.0f, 336.0f),
        mVec3_c(208.0f, 0.0f, 400.0f),
        mVec3_c(304.0f, 0.0f, 336.0f),
        mVec3_c(336.0f, 0.0f, 400.0f),
    };
    if (pos == NULL) {
        return FALSE;
    }
    u32 count = 0;
    for (u32 i = 0; i < ARRAY_SIZE(l_seats); i++) {
        if (isNewSpot(l_seats[i], used, usedNum)) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                *pos = l_seats[i];
            }
            count++;
        }
    }
    return count != 0;
}

BOOL dNpcMng_c::pickTheaterSpots(mVec3_c *pos, u32 num) {
    if (pos == NULL || num == 0) {
        return FALSE;
    }
    mVec3_c *base = pos;
    for (u32 i = 0; i < num; i++, pos++) {
        switch (pickTheaterSpot(pos, base, i)) {
        case FALSE:
            return FALSE;
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::createTheaterGuests() {
    if (dScene::getCurrentScene() != dScene::SCENE_RM_THEATER_SHOW) {
        return FALSE;
    }
    mVec3_c *spot;
    if (sResetTheaterSpots) {
        if (!pickTheaterSpots(l_theaterSpots, 2)) {
            spot = l_theaterSpots;
            for (u32 i = 0; i < 2; i++, spot++) {
                dFdBase_c::getUnitCenterPos(spot, 4, i + 7);
            }
        }
        sResetTheaterSpots = 0;
    }
    spot = l_theaterSpots;
    u16 *guest = sTheaterGuests;
    for (u32 i = 0; i < 2; i++, spot++, guest++) {
        if (*guest != 0xFFFF) {
            mAng3_c angle = mAng3_c::Zero;
            angle.y = -0x8000;
            dActor_c::construct(fProf::A_NPC_TOWN_THEATER, this, *guest, spot,
                                &angle);
        }
    }
    return TRUE;
}

BOOL dNpcMng_c::createCafeGuest() {
    if (dScene::getCurrentScene() != dScene::SCENE_RM_MM_CAFE) {
        return FALSE;
    }
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    dAnimalHomeStay_c *stay = &block->mHomeStay;
    if (!stay->isValid()) {
        return FALSE;
    }
    u32 idx = stay->mAnimalIdx;
    if (idx >= 10) {
        return FALSE;
    }
    dAnimal_c *animal = block->getAnimalConst(idx);
    if (animal == NULL || !animal->mID.isValid() || (int)animal->mPlace != 3) {
        return FALSE;
    }
    mVec3_c pos(336.0f, 0.0f, 240.0f);
    mAng3_c angle(0, 0x4000, 0);
    if (dActor_c::construct(fProf::A_NPC_CAFE, this, getAnimalKey(idx), &pos,
                            &angle) != NULL) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::updateAnimalPlaces() {
    if (!dNpc::isPermitSceneOwner()) {
        return;
    }
    if (dScene::getCurrentScene() != dScene::SCENE_FIELD) {
        return;
    }
    BOOL toyDay;
    BOOL online;
    BOOL flagD;
    dQuestBase_c *quest;
    int stayIdx;
    dPrivateData_c *player;
    dAnimalBlock_c *block;
    dQuestPlayerItem_c *appointment;
    int appointmentIdx;
    dTime_c *now;
    player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    flagD =
        player != NULL ? player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB) : FALSE;
    online = dNet::isSessionActive();
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    appointmentIdx = -1;
    appointment = block->getAppointment();
    dAnimalHomeStay_c *stay = &block->mHomeStay;
    quest = appointment->getBase();
    stayIdx = stay->isValid() ? stay->mAnimalIdx : -1;
    now = dTime_c::getCurrent();
    toyDay = dEvent::isActive(EVENT_TOY_DAY) != FALSE;
    if (quest->isActive() && quest->getKind() == 0x11 && quest->mState != 2) {
        appointmentIdx = appointment->mAnimalIdx;
    }
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = block->getAnimal(i);
        if (animal == NULL || !animal->mID.isValid()) {
            continue;
        }
        int place = animal->mPlace;
        if (i == appointmentIdx && animal->mDayPlace == 0) {
            if (!online) {
                if (place == 0 && appointment->isPastMeetTime(*now, -60) &&
                    !appointment->isPastMeetTime(*now, 30)) {
                    place = 1;
                    quest->mState = place;
                }
            } else if (place != 0 && place != 4) {
                place = 0;
                quest->mState = place;
            }
        } else if (i == stayIdx) {
            u8 prevScene = dScene::getPrevScene();
            u8 scene = dScene::getCurrentScene();
            BOOL inHouse =
                i == dScene::getSceneAttrIndex(
                         prevScene, dScene::SCENE_ATTR_VILLAGER_HOUSE) ||
                i == dScene::getSceneAttrIndex(
                         scene, dScene::SCENE_ATTR_VILLAGER_HOUSE);
            if (!inHouse) {
                switch (place) {
                case 2:
                    break;
                case 3:
                    if (!block->isHomeStayTime(&animal->mID, player)) {
                        place = 1;
                    }
                    break;
                case 1:
                    if (block->isHomeStayTime(&animal->mID, player)) {
                        place = 3;
                    }
                    break;
                }
            }
        }
        switch (place) {
        case 0:
            if (!toyDay && !flagD && animal->isSleepTime(NULL) &&
                animal->getDayPlace(1, 0, 0) == 3 &&
                block->getHiderSlot(&animal->mID) == 3) {
                place = 4;
            }
            break;
        case 1:
            if (!flagD && animal->isSleepTime(NULL)) {
                if (animal->mDayPlace != 0 ||
                    (!toyDay && animal->isLostItemRequestDone(NULL, 1))) {
                    place = 4;
                }
            }
            if (place != 4 && animal->mDayPlace == 0 && online) {
                place = 0;
            }
            break;
        case 4:
            if (!animal->isSleepTime(NULL)) {
                if (animal->mDayPlace == 0 && online) {
                    place = 0;
                } else {
                    place = 1;
                }
            }
            break;
        }
        if (animal->mPlace != place) {
            switch (animal->mPlace) {
            case 0:
                if (place != 0) {
                    animal->clearQuestStarted();
                    dNpc::sendAnimalQuestClear(i);
                    if (place == 1 && !online) {
                        animal->setPlaceChangeTime(10, NULL);
                    }
                }
                break;
            default:
                if (place == 0) {
                    animal->clearPlaceChangeTime();
                    dNpc::sendAnimalPlaceTimeClear(i);
                }
                break;
            }
            animal->mPlace = place;
            dNpc::sendAnimalPlace(i, place);
        }
    }
}

void dNpcMng_c::applyHouseVillagerItems() {
    if (!dNpc::isPermitSceneOwner()) {
        return;
    }
    if (dEvent::isActive(EVENT_TOY_DAY)) {
        return;
    }
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        return;
    }
    if (!dScene::isSceneAttr(dScene::getPrevScene(), dScene::SCENE_ATTR_TOWN)) {
        return;
    }
    int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(),
                                        dScene::SCENE_ATTR_VILLAGER_HOUSE);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal == NULL || !animal->mID.isValid()) {
        return;
    }
    animal->applyNewItems(idx, TRUE);
}

int dNpcMng_c::preCreate() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) &&
        m_halloweenModelHeap_p == NULL) {
        m_halloweenModelHeap_p = mHeap::createFrmHeap(
            ROUND_UP(dNpcMdlMng_c::getSpMdlSize(), 32), m_stageHeap_p,
            "dNpcMng_c::m_halloweenModelHeap_p : "
            "\x83\x6E\x83\x8D\x83\x45\x83\x42\x83\x93\x97\x70\x83\x82\x83\x66"
            "\x83\x8B\x83\x71\x81\x5B\x83\x76",
            0x20, mHeap::OPT_NONE);
        mAllocator.attach(m_halloweenModelHeap_p, 0x20);
    }
    return 1;
}

int dNpcMng_c::create() {
    if (m_halloweenModelHeap_p != NULL && sSpecialModel == NULL) {
        char path[32];
        dItem::Item jack((u16)0x8006);
        sprintf(path, "/Npc/Special/Model/%d.brres", jack.mId & 0xFFF);
        sSpecialModel = mLoader.request(path, 0, m_halloweenModelHeap_p);
        if (sSpecialModel == NULL) {
            return 0;
        }
        DCFlushRange(sSpecialModel, dNpcMdlMng_c::getSpMdlSize());
        nw4r::g3d::ResFile file(getSpecialModel());
        file.Init();
        file.Bind(file);
    }
    if (dScene::getCurrentScene() == dScene::SCENE_DM_TITLE) {
        _8074EACF = 0;
    }
    dSaveData_c::getTown()->mAnimals.mTown.updateQuestCandidates();
    updateAnimalPlaces();
    u8 scene = dScene::getCurrentScene();
    u8 prevScene = dScene::getPrevScene();
    BOOL fromBus = FALSE;
    if (prevScene == dScene::SCENE_DM_BUS_TO_TOWN &&
        dScene::isSceneAttr(scene, dScene::SCENE_ATTR_CITY_FIELD)) {
        fromBus = TRUE;
    }
    if (fromBus) {
        setupCityGuests();
    }
    createAnimals();
    createSpecialNpcs();
    setupCityAnimalItems();
    createCityAnimals();
    createGraceGuest();
    createTheaterGuests();
    createCafeGuest();
    sTheaterMsg = 0;
    mShowTelop = 0;
    if (sShowTelop) {
        mShowTelop = 1;
    }
    sInstance = this;
    return 1;
}

int dNpcMng_c::doDelete() {
    if (!mLoader.release()) {
        return 0;
    }
    if (m_halloweenModelHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_halloweenModelHeap_p);
        m_halloweenModelHeap_p = NULL;
    }
    sSpecialModel = NULL;
    sInstance = NULL;
    return 1;
}

int dNpcMng_c::execute() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) && mShowTelop) {
        dObjTelop_c::request(TELOP_HIDE_AND_SEEK_START);
        mShowTelop = 0;
        sShowTelop = 0;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) &&
        dNet::isSceneOwner()) {
        dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        BOOL flagD = player != NULL
                         ? player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)
                         : FALSE;
        for (int i = 0; i < 10; i++) {
            dAnimal_c *animal = block->getAnimal(i);
            if (animal == NULL || !animal->mID.isValid() ||
                animal->mDayPlace == 0) {
                continue;
            }
            switch (animal->mPlace) {
            case 4:
                if (!animal->isSleepTime(NULL)) {
                    animal->mPlace = 1;
                    dNpc::sendAnimalPlace(i, 1);
                }
                break;
            case 1:
                if (!flagD && animal->isSleepTime(NULL)) {
                    animal->mPlace = 4;
                    dNpc::sendAnimalPlace(i, 4);
                }
                break;
            }
        }
    }
    return 1;
}

int dNpcMng_c::draw() {
    return 1;
}

BOOL dNpcMng_c::createEventNpc() {
    u8 scene = dScene::getCurrentScene();
    const Spawn_c *spawn = findSpawn(l_eventNpcs, ARRAY_SIZE(l_eventNpcs),
                                     scene, dScene::getPrevScene());
    if (spawn != NULL) {
        mVec3_c pos = mVec3_c::Zero;
        mAng3_c angle = mAng3_c::Zero;
        dItem::Item npc(spawn->mNpc);
        (this->*spawn->mSetPos)(&pos, &angle, &npc);
        dActor_c::construct(spawn->mProfile, this, spawn->mNpc, &pos, &angle);
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isFishingTourneyTime(dQuestEvent_e event) {
    BOOL ok = TRUE;
    BOOL running = TRUE;
    if (!dEvent::isNotStarted((dQuestEvent_e)event)) {
        if (!dEvent::isOngoing((dQuestEvent_e)event)) {
            running = FALSE;
        }
    }
    if (!running) {
        if (!dEvent::isOverWithin((dQuestEvent_e)event, 6, 0)) {
            ok = FALSE;
        }
    }
    return ok;
}

BOOL dNpcMng_c::isHarvestTime(dQuestEvent_e event) {
    return dEvent::isOngoing((dQuestEvent_e)event) != FALSE;
}

BOOL dNpcMng_c::isToyDayTime(dQuestEvent_e event) {
    return dEvent::isOngoing((dQuestEvent_e)event) != FALSE;
}

BOOL dNpcMng_c::isBugOffTime(dQuestEvent_e event) {
    BOOL ok = TRUE;
    BOOL running = TRUE;
    if (!dEvent::isNotStarted((dQuestEvent_e)event)) {
        if (!dEvent::isOngoing((dQuestEvent_e)event)) {
            running = FALSE;
        }
    }
    if (!running) {
        if (!dEvent::isOverWithin((dQuestEvent_e)event, 6, 0)) {
            ok = FALSE;
        }
    }
    return ok;
}

// The villager hosting the current player's birthday party, or -1.
static inline int getPlayerBirthdayHostIdx() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return save->mAnimals.mTown.getBirthdayHostIdx(
        dPlayerMgr::getCurrentPlayerSlot(), 1, 1, 1);
}

BOOL dNpcMng_c::isVisitorBeforeTortimer(u8 scene, u8 prevScene) {
    dItem::Item npc((u16)0x8001);
    u32 idx = findSpawnIdx(&npc, l_visitorNpcs, ARRAY_SIZE(l_visitorNpcs));
    if (idx >= ARRAY_SIZE(l_visitorNpcs)) {
        return TRUE;
    }
    if (findSpawn(l_visitorNpcs, idx, scene, prevScene) != NULL) {
        return TRUE;
    }
    if (prevScene == dScene::SCENE_DM_PL_SEL &&
        getPlayerBirthdayHostIdx() != -1) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isPlayerSelectVisitor() {
    if (dScene::getCurrentScene() != dScene::SCENE_DM_PL_SEL) {
        return FALSE;
    }
    if ((u32)getPlayerBirthdayHostIdx() < 10) {
        return TRUE;
    }
    if (isHouseWarmingTime(dScene::SCENE_FIELD, dScene::SCENE_DM_PL_SEL)) {
        return TRUE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && player->getLastResetCount()) {
        if (isResetVisitTime(dScene::SCENE_FIELD, dScene::SCENE_DM_PL_SEL)) {
            return TRUE;
        }
        if (isBirthdayVisitTime(dScene::SCENE_FIELD, dScene::SCENE_DM_PL_SEL)) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL dNpcMng_c::isHolidayNpcToday() {
    return findSpawn(l_holidayNpcs, ARRAY_SIZE(l_holidayNpcs),
                     dScene::SCENE_FIELD, dScene::SCENE_NUM) != NULL;
}

BOOL dNpcMng_c::createHolidayNpc() {
    u8 scene = dScene::getCurrentScene();
    const Spawn_c *spawn = findSpawn(l_holidayNpcs, ARRAY_SIZE(l_holidayNpcs),
                                     scene, dScene::getPrevScene());
    if (spawn != NULL) {
        mVec3_c pos = mVec3_c::Zero;
        mAng3_c angle = mAng3_c::Zero;
        dItem::Item npc(spawn->mNpc);
        (this->*spawn->mSetPos)(&pos, &angle, &npc);
        dActor_c::construct(spawn->mProfile, this, spawn->mNpc, &pos, &angle);
        return TRUE;
    }
    return FALSE;
}

// The position of a building of the town (dSaveBuildingList_c::getPos).
static inline BOOL getTownBuildingPos(dSaveTown_c *town, int *x, int *z,
                                      const dItem::Item *item) {
    return town->mBuilding.mList.getPos(x, z, *item, 1);
}

void dNpcMng_c::setPosConstruction(mVec3_c *pos, mAng3_c *angle,
                                   const dItem::Item *npc) {
    int x = 0;
    int z = 0;
    switch (dSaveData_c::getTown()->mNewConstruction) {
    case NEW_CONSTRUCTION_BRIDGE: {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            int blockZ = dSaveData_c::getTown()->mNewConstructionBlockZ;
            dSaveTown_c *town = dSaveData_c::getTown();
            *pos = field->getRaccoPos(town->mNewConstructionBlockX, blockZ);
            break;
        }
        return;
    }
    case NEW_CONSTRUCTION_FOUNTAIN: {
        dItem::Item fountain((u16)BUILDING_FOUNTAIN);
        getTownBuildingPos(dSaveData_c::getTown(), &x, &z, &fountain);
        z += 3;
        dFdBase_c::getUnitGroundPos(pos, x, z);
        break;
    }
    case NEW_CONSTRUCTION_WINDMILL: {
        dItem::Item windmill((u16)BUILDING_WINDMILL);
        getTownBuildingPos(dSaveData_c::getTown(), &x, &z, &windmill);
        dFdBase_c::getUnitGroundPos(pos, x, z);
        break;
    }
    case NEW_CONSTRUCTION_LIGHTHOUSE: {
        dItem::Item lighthouse((u16)BUILDING_LIGHTHOUSE);
        getTownBuildingPos(dSaveData_c::getTown(), &x, &z, &lighthouse);
        dFdBase_c::getUnitGroundPos(pos, x, z);
        break;
    }
    default: {
        int blockZ = dSaveData_c::getTown()->mNewConstructionBlockZ;
        dSaveTown_c *town = dSaveData_c::getTown();
        dFdBase_c::getBlockCenterPos(pos, town->mNewConstructionBlockX, blockZ);
    } break;
    }
    if ((int)dSaveData_c::getTown()->mNewConstruction ==
        NEW_CONSTRUCTION_FOUNTAIN) {
        mVec3_c left;
        left = *pos;
        mVec3_c right;
        right = *pos;
        right.x += 32.0f;
        dFdBase_c *field = dField::getInfo(0);
        BOOL ok = FALSE;
        dItem::Item none;
        if (dNpc::canPutItemAt(&left, none, field, 0x44, TRUE, TRUE)) {
            dItem::Item none1;
            if (dNpc::canPutItemAt(&right, none1, field, 0x44, TRUE, TRUE)) {
                ok = TRUE;
            }
        }
        if (ok) {
            pos->x += 16.0f;
        } else {
            dNpc::pickNearbyFreePos(pos, pos, 16);
        }
    } else {
        dNpc::pickNearbyFreePos(pos, pos, 16);
    }
    angle->set(0, 0, 0);
}

BOOL dNpcMng_c::isConstructionTime(u8 scene, u8 prevScene) {
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    BOOL fromTown = player != NULL ? player->mPID.isFromTown() : FALSE;
    if (isTownPlay(scene, prevScene) && fromTown &&
        dSaveData_c::getTown()->mNewConstruction != NEW_CONSTRUCTION_NONE) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isCatalogTime(u8 scene, u8 prevScene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!dNet::isSessionActive() && isTownPlay(scene, prevScene) &&
        !isVisitorBeforeTortimer(scene, prevScene) && player != NULL &&
        ((!player->isFlag0(0x76) && player->mCatalog.isComplete(8) == 1) ||
         (!player->isFlag0(0x75) && player->mCatalog.isComplete(7) == 1)) &&
        prevScene == dScene::SCENE_DM_PL_SEL && !_8074EACF) {
        return TRUE;
    }
    return FALSE;
}

// The town's player house home (its unit position).
static inline BOOL getPlayerHousePos(dSaveTown_c *town, int *x, int *z,
                                     int home) {
    return town->mBuilding.mList.getPlayerHousePos(x, z, home);
}

void dNpcMng_c::setPosPlayerHouseA(mVec3_c *pos, mAng3_c *angle,
                                   const dItem::Item *npc) {
    u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    if (home < 4) {
        int x = 0;
        int z = 0;
        if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
            angle->set(0, 0, 0);
            x += 1;
            z += 2;
            dFdBase_c::getUnitCenterPos(pos, x, z);
            pos->x += 16.0f;
        }
    }
}

BOOL dNpcMng_c::isHolidayTime(dQuestEvent_e event) {
    BOOL ok = TRUE;
    if (!dEvent::isOngoing((dQuestEvent_e)event)) {
        if (!dEvent::isOverWithin((dQuestEvent_e)event, 1, 0)) {
            ok = FALSE;
        }
    }
    return ok;
}

BOOL dNpcMng_c::isHolidayDay(dQuestEvent_e event) {
    BOOL ok = TRUE;
    if (!dEvent::isNotStarted((dQuestEvent_e)event)) {
        if (!isHolidayTime(event)) {
            ok = FALSE;
        }
    }
    return ok;
}

BOOL dNpcMng_c::isEventActive(dQuestEvent_e event) {
    return dEvent::isActive((dQuestEvent_e)event) != FALSE;
}

BOOL dNpcMng_c::isFireworksTime(dQuestEvent_e event) {
    BOOL ok = TRUE;
    BOOL running = TRUE;
    if (!dEvent::isNotStarted((dQuestEvent_e)event)) {
        if (!dEvent::isOngoing((dQuestEvent_e)event)) {
            running = FALSE;
        }
    }
    if (!running) {
        if (!dEvent::isOverWithin((dQuestEvent_e)event, 3, 0)) {
            ok = FALSE;
        }
    }
    return ok;
}

BOOL dNpcMng_c::createVisitorNpc() {
    u8 scene = dScene::getCurrentScene();
    u8 prevScene = dScene::getPrevScene();
    if (scene == dScene::SCENE_DM_TITLE) {
        sResetVisited = 0;
        _8074EAD0 = 0;
    }
    const Spawn_c *spawn =
        findSpawn(l_visitorNpcs, ARRAY_SIZE(l_visitorNpcs), scene, prevScene);
    if (spawn != NULL) {
        mVec3_c pos = mVec3_c::Zero;
        mAng3_c angle = mAng3_c::Zero;
        dItem::Item npc(spawn->mNpc);
        (this->*spawn->mSetPos)(&pos, &angle, &npc);
        dActor_c::construct(spawn->mProfile, this, spawn->mNpc, &pos, &angle);
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isHouseWarmingTime(u8 scene, u8 prevScene) {
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (player->isFlag0(0xC)) {
        return FALSE;
    }
    if (isTownScene(scene, dScene::SCENE_NUM)) {
        dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
        dPrivateData_c *current;
        if (homes->getHome(homes->findCurrentPlayer()) == NULL &&
            dScene::isSceneAttr(prevScene, dScene::SCENE_ATTR_PLAYER_HOUSE) &&
            (current = dPlayerMgr::Mgr_c::getCurrentPlayer()) != NULL &&
            current->mPID.isFromTown()) {
            u32 home = dHomeList_c::getHomeFromScene(NULL, prevScene);
            if (home < 4) {
                int x = 0;
                int z = 0;
                if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void dNpcMng_c::setPosPrevHouse(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    u32 home = dHomeList_c::getHomeFromScene(NULL, dScene::getPrevScene());
    if (home < 4) {
        int x = 0;
        int z = 0;
        if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
            angle->set(0, 0, 0);
            x += 1;
            z += 2;
            dFdBase_c::getUnitCenterPos(pos, x, z);
            pos->x += 16.0f;
        }
    }
}

BOOL dNpcMng_c::isBirthdayVisitTime(u8 scene, u8 prevScene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isTownPlay(scene, dScene::SCENE_NUM) &&
        prevScene == dScene::SCENE_DM_PL_SEL && !_8074EACF && player != NULL &&
        player->isFlag0(0x57) && getPlayerBirthdayHostIdx() == -1) {
        u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
        if (home < 4) {
            int x = 0;
            int z = 0;
            if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void dNpcMng_c::setPosPlayerHouseB(mVec3_c *pos, mAng3_c *angle,
                                   const dItem::Item *npc) {
    u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    if (home < 4) {
        int x = 0;
        int z = 0;
        if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
            angle->set(0, 0, 0);
            x += 1;
            z += 2;
            dFdBase_c::getUnitCenterPos(pos, x, z);
            pos->x += 16.0f;
        }
    }
}

BOOL dNpcMng_c::isLetterVisitTime(u8 scene, u8 prevScene) {
    if (isTownScene(scene, dScene::SCENE_NUM) &&
        dNpcInfo::resetsan_c::get()->getVisit() &&
        prevScene == dScene::SCENE_DM_PL_SEL &&
        getPlayerBirthdayHostIdx() == -1) {
        u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
        if (home < 4) {
            int x = 0;
            int z = 0;
            if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void dNpcMng_c::setPosPlayerHouseDoor(mVec3_c *pos, mAng3_c *angle,
                                      const dItem::Item *npc) {
    u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    if (home < 4) {
        int x = 0;
        int z = 0;
        if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
            angle->set(0, 0, 0);
            x += 2;
            z += 2;
            dFdBase_c::getUnitCenterPos(pos, x, z);
        }
    }
}

BOOL dNpcMng_c::isLetterVisitFirstTime(u8 scene, u8 prevScene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isLetterVisitTime(scene, prevScene) && player != NULL &&
        !player->getLastResetCount()) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isLetterVisitKind1(u8 scene, u8 prevScene) {
    if (isLetterVisitTime(scene, prevScene) &&
        dNpcInfo::resetsan_c::get()->getVisit() == 1) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isVisitor8Day(u8 scene, u8 prevScene) {
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (isTownPlay(scene, prevScene) && dEvent::getTodayVisitor() == 8) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosRaccoSpot(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    if (getDaubPos(pos, angle, npc, FALSE)) {
        return;
    }
    dFdBase_c *field = dField::getInfo(0);
    if (field == NULL) {
        return;
    }
    int x = 0;
    int z = 0;
    s16 spotAngle = 0;
    if (field->getRaccoSpot(&x, &z, &spotAngle)) {
        dFdBase_c::getUnitGroundPos(pos, x, z);
        angle->set(0, 0, 0);
        angle->y = spotAngle;
    }
}

BOOL dNpcMng_c::isGuestVisitTime(u8 scene, u8 prevScene) {
    if (isTownScene(scene, dScene::SCENE_NUM) &&
        dPrivateData_c::isGuestVisitReady()) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosGuestVisit(mVec3_c *pos, mAng3_c *angle,
                                 const dItem::Item *npc) {
    setPosRandomPlace(pos, angle, npc);
}

BOOL dNpcMng_c::isHostVisitTime(u8 scene, u8 prevScene) {
    if (isTownScene(scene, dScene::SCENE_NUM) &&
        dPrivateData_c::isHostVisitReady()) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosHostVisit(mVec3_c *pos, mAng3_c *angle,
                                const dItem::Item *npc) {
    setPosRandomPlace(pos, angle, npc);
}

void dNpcMng_c::setPosFountain(mVec3_c *pos, mAng3_c *angle,
                               const dItem::Item *npc) {
    int x = 0;
    int z = 0;
    dItem::Item fountain((u16)BUILDING_FOUNTAIN);
    dSaveTown_c *town = dSaveData_c::getTown();
    if (getBuildingPos(&town->mBuilding.mList, &x, &z, &fountain)) {
        dFdBase_c::getUnitGroundPos(pos, x, z);
        pos->x += 16.0f;
        pos->z += 16.0f;
        angle->set(0, 0, 0);
    }
}

BOOL dNpcMng_c::hasFountain(u8 scene, u8 prevScene) {
    int x = 0;
    int z = 0;
    BOOL found = FALSE;
    if (isTownPlay(scene, prevScene)) {
        dItem::Item fountain((u16)BUILDING_FOUNTAIN);
        dSaveTown_c *town = dSaveData_c::getTown();
        if (town->mBuilding.mList.getPos(&x, &z, fountain, 1)) {
            found = TRUE;
        }
    }
    return found;
}

BOOL dNpcMng_c::isRelayDesignVisitTime(u8 scene, u8 prevScene) {
    if (isTownScene(scene, dScene::SCENE_NUM) &&
        dPrivateData_c::isRelayDesignVisitReady()) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isVisitor2Day(u8 scene, u8 prevScene) {
    if (isTownPlay(scene, prevScene) && dEvent::getTodayVisitor() == 2) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isVisitor5Day(u8 scene, u8 prevScene) {
    if (isTownPlay(scene, prevScene) && dEvent::getTodayVisitor() == 5) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isFlag13Visit(u8 scene, u8 prevScene) {
    dTime_c now = *dTime_c::getCurrent();
    BOOL visit = FALSE;
    BOOL here = FALSE;
    if (isTownPlay(scene, prevScene)) {
        BOOL ok = TRUE;
        if (!dSaveData_c::getRaw()->isFlag(0x13)) {
            dItem::Item ufo((u16)BUILDING_UFO);
            int x;
            int z;
            if (!dSaveBuildingList_c::get()->getPos(&x, &z, ufo, 1)) {
                ok = FALSE;
            }
        }
        if (ok) {
            here = TRUE;
        }
    }
    if (here && now.wday != 0) {
        visit = TRUE;
    }
    if (visit) {
        dSaveData_c::getTown()->clearFlag(0x13);
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosVisitor2(mVec3_c *pos, mAng3_c *angle,
                               const dItem::Item *npc) {
    setPosRandomSpot(pos, angle, npc);
}

BOOL dNpcMng_c::isResetVisitTime(u8 scene, u8 prevScene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!dNet::isSessionActive() && isTownPlay(scene, prevScene) &&
        (sResetVisited == 1 ||
         (prevScene == dScene::SCENE_DM_PL_SEL && !_8074EACF)) &&
        player != NULL && !player->isFlag0(PRIVATE_FLAG0_TRAVELLING) && !player->isFlag1(0x3A) &&
        !player->isFlag1(0x3B)) {
        dSaveExtra_c *extra = dSaveData_c::getExtra();
        if (extra->mDistPattern.mVersion != player->mDistPatternVersion &&
            getPlayerBirthdayHostIdx() == -1) {
            sResetVisited = 1;
            return TRUE;
        }
    }
    return FALSE;
}

void dNpcMng_c::setPosResetVisit(mVec3_c *pos, mAng3_c *angle,
                                 const dItem::Item *npc) {
    if (dScene::getPrevScene() == dScene::SCENE_DM_PL_SEL) {
        u32 home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
        if (home < 4) {
            int x = 0;
            int z = 0;
            if (getPlayerHousePos(dSaveData_c::getTown(), &x, &z, home)) {
                angle->set(0, 0, 0);
                x += 1;
                z += 2;
                dFdBase_c::getUnitCenterPos(pos, x, z);
                pos->x += 16.0f;
            }
        }
    } else {
        setPosGateA(pos, angle, npc);
    }
}

BOOL dNpcMng_c::isVisitor7Day(u8 scene, u8 prevScene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (isTownPlay(scene, prevScene) && !dNet::isSessionActive() &&
        dEvent::getTodayVisitor() == 7 && player != NULL &&
        !player->isFlag0(PRIVATE_FLAG0_TRAVELLING) && !player->isFlag1(0x3A) &&
        !player->isFlag1(0x3B)) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosGateA(mVec3_c *pos, mAng3_c *angle,
                            const dItem::Item *npc) {
    dItem::Item gate((u16)BUILDING_GATE);
    int x;
    int z;
    dSaveBuildingList_c::get()->getPos(&x, &z, gate, 1);
    x &= 0xF0;
    z &= 0xF0;
    mVec3_c exclude;
    exclude.x = 32.0f * (x + 8);
    exclude.z = 32.0f * (z + 8);
    dNpc::pickOutdoorFreePos(pos, NULL, 4, &exclude, 384.0f);
    angle->y = cM::rndInt(0xFFFF);
}

BOOL dNpcMng_c::isVisitor4Day(u8 scene, u8 prevScene) {
    dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (isTownPlay(scene, prevScene) && dEvent::getTodayVisitor() == 4) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosGateB(mVec3_c *pos, mAng3_c *angle,
                            const dItem::Item *npc) {
    dItem::Item gate((u16)BUILDING_GATE);
    int x;
    int z;
    dSaveBuildingList_c::get()->getPos(&x, &z, gate, 1);
    x &= 0xF0;
    z &= 0xF0;
    mVec3_c exclude;
    exclude.x = 32.0f * (x + 8);
    exclude.z = 32.0f * (z + 8);
    dNpc::pickOutdoorFreePos(pos, NULL, 4, &exclude, 384.0f);
    angle->y = cM::rndInt(0xFFFF);
}

BOOL dNpcMng_c::isVisitor9Day(u8 scene, u8 prevScene) {
    if (isTownPlay(scene, prevScene) && dEvent::getTodayVisitor() == 9) {
        return TRUE;
    }
    return FALSE;
}

void dNpcMng_c::setPosVisitor9(mVec3_c *pos, mAng3_c *angle,
                               const dItem::Item *npc) {
    setPosRandomSpot(pos, angle, npc);
}

void dNpcMng_c::setPosVisitor5(mVec3_c *pos, mAng3_c *angle,
                               const dItem::Item *npc) {
    angle->set(0, 0, 0);
    if (pickEmptyBlockSpot(pos)) {
        return;
    }
    dItem::Item hall((u16)BUILDING_TOWN_HALL);
    int x;
    int z;
    dSaveTown_c *town = dSaveData_c::getTown();
    if (hasNoBuildingPos(&town->mBuilding.mList, &x, &z, &hall)) {
        dFdBase_c::getUnitCenterPos(pos, 0, 0);
        return;
    }
    x &= ~0xF;
    z &= ~0xF;
    x += 7;
    z += 12;
    if (canStandAt(dField::getInfo(0), x, z)) {
        dFdBase_c::getUnitCenterPos(pos, x, z);
    } else {
        dFdBase_c::getUnitCenterPos(pos, 0, 0);
    }
}

BOOL dNpcMng_c::pickEmptyBlockSpot(mVec3_c *pos) {
    dFdBase_c *field = getCurrentField();
    int num = 0;
    int blocks[50];
    int blockX, blockZ;
    for (blockZ = 1; blockZ < 6; blockZ++) {
        for (blockX = 1; blockX < 6; blockX++) {
            dFdBlock_c *block = field->getBlock(blockX, blockZ);
            if (dFdBlockType::getKind(block->mType) == 0xB ||
                dFdBlockType::getKind(block->mType) == 0x2D) {
                if (dSaveBuildingList_c::get()->countHouses(blockX, blockZ) ==
                    0) {
                    blocks[num * 2] = blockX;
                    blocks[num * 2 + 1] = blockZ;
                    num++;
                }
            }
        }
    }
    if (num == 0) {
        return FALSE;
    }
    BOOL found = FALSE;
    do {
        int r = cM::rndInt(num);
        int *pick = &blocks[r * 2];
        if (pickBlockSpot(pos, pick[0], pick[1])) {
            found = TRUE;
            break;
        }
        if (r < num - 1) {
            for (int i = r; i < num - 1; i++) {
                blocks[i] = blocks[i + 1];
            }
        }
    } while (--num != 0);
    return found;
}

BOOL dNpcMng_c::pickBlockSpot(mVec3_c *pos, int blockX, int blockZ) {
    dFdBase_c *field = dField::getInfo(0);
    int x, x0 = blockX * 16 + 6;
    int z, z0 = blockZ * 16 + 6;
    int num = 0;
    int units[32];
    for (z = z0; z < z0 + 4; z++) {
        for (x = x0; x < x0 + 4; x++) {
            if (canStandAt(field, x, z)) {
                units[num * 2] = x;
                units[num * 2 + 1] = z;
                num++;
            }
        }
    }
    if (num > 0) {
        int r = cM::rndInt(num);
        dFdBase_c::getUnitCenterPos(pos, units[r * 2], units[r * 2 + 1]);
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::canStandAt(dFdBase_c *field, int unitX, int unitZ) {
    dItem::Item *item = field->getItem(unitX, unitZ, 0);
    if (item == NULL || item->mId < 0xE5) {
        return FALSE;
    }
    for (int i = 1; i <= 2; i++) {
        if (dSaveBuildingList_c::get()->isBuildSite(unitX, unitZ + i)) {
            return FALSE;
        }
        dItem::Item *behind = field->getItem(unitX, unitZ + i, 0);
        if (behind != NULL) {
            dItem::FgInfo *info = behind->getFgInfo();
            if (info != NULL && info->mTreeStage > 0) {
                return FALSE;
            }
        }
    }
    dItem::Item none;
    return dNpc::canPutItemOnUnit(unitX, unitZ, &none, field, 0x44, TRUE, TRUE);
}

void dNpcMng_c::setPosBeach(mVec3_c *pos, mAng3_c *angle,
                            const dItem::Item *npc) {
    if (getDaubPos(pos, angle, npc, FALSE)) {
        return;
    }
    int x;
    int z;
    // The aggregate keeps the field pointer in the original temporary
    // allocation.
    struct {
        dFdBase_c *ptr;
    } field = {dField::getInfo(0)};
    dRandom_c rnd(0x9D);
    rnd.initFromDateAndLandId(*dTime_c::getCurrent(), 0x61);
    dItem::Item ufo((u16)BUILDING_UFO);
    dSaveBuildingList_c::get()->getPos(&x, &z, ufo, 1);
    int units[32];
    int x0;
    int unitX;
    int unitZ;
    int num;
    num = 0;
    x0 = x & 0xF0;
    for (unitZ = 0x50; unitZ < 0x60; unitZ++) {
        for (unitX = x0; unitX < x0 + 16; unitX++) {
            if ((u32)(field.ptr->getBgAttr(unitX, unitZ) - 0x24) <= 2) {
                units[num * 2] = unitX;
                units[num * 2 + 1] = unitZ;
                num++;
            }
        }
    }
    int r = rnd.rndF(num);
    dFdBase_c::getUnitCenterPos(pos, units[r * 2], units[r * 2 + 1]);
    angle->y = -0x8000;
}

void dNpcMng_c::setPosCarnival(mVec3_c *pos, mAng3_c *angle,
                               const dItem::Item *npc) {
    if (getDaubPos(pos, angle, npc, FALSE)) {
        return;
    }
    angle->set(0, 0, 0);
    dItem::Item carnival((u16)BUILDING_CARNIVAL);
    int x;
    int z;
    dSaveTown_c *town = dSaveData_c::getTown();
    if (hasNoBuildingPos(&town->mBuilding.mList, &x, &z, &carnival)) {
        dFdBase_c::getUnitCenterPos(pos, 0, 0);
        return;
    }
    z += 1;
    dFdBase_c::getUnitCenterPos(pos, x, z);
}

BOOL dNpcMng_c::isVisitor2Here(dQuestEvent_e event) {
    return dEvent::isVisitorHere(2) != FALSE;
}

BOOL dNpcMng_c::isVisitor9Here(dQuestEvent_e event) {
    return dEvent::isVisitorHere(9) != FALSE;
}

BOOL dNpcMng_c::isNotTownFlag5(dQuestEvent_e event) {
    return dSaveData_c::getRaw()->isFlag(5) == FALSE;
}

BOOL dNpcMng_c::isVisitor7Here(dQuestEvent_e event) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (dEvent::isVisitorHere(7) && player != NULL && !player->isFlag1(0x3A) &&
        !player->isFlag1(0x3B)) {
        return TRUE;
    }
    return FALSE;
}

BOOL dNpcMng_c::isVisitor4Time(dQuestEvent_e event) {
    dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (!dNet::isSessionActive() && dPlayerMgr::getCurrentPlayerSlot() < 4) {
        dTime_c *now = dTime_c::getCurrent();
        if ((now->hour == 9 || now->hour == 17) && now->min < 30) {
            return TRUE;
        }
    }
    return FALSE;
}

void dNpcMng_c::getTodayEventNpc(dItem::Item *npc) {
    *npc = dItem::Item();
    const Spawn_c *spawn = findSpawn(l_eventNpcs, ARRAY_SIZE(l_eventNpcs),
                                     dScene::SCENE_FIELD, dScene::SCENE_NUM);
    if (spawn != NULL) {
        u16 id = spawn->mNpc;
        switch (id) {
        case 0x8025:
        case 0x804C:
            if (dEvent::isNowBetween(6, 0, 6, 0)) {
                *npc = id;
            }
            break;
        case 0x804B:
            break;
        case 0x804A:
        case 0x804D:
            *npc = id;
            break;
        }
    }
    if (npc->mId == dItem::ITEM_ID_NONE) {
        const Spawn_c *visitor =
            findSpawn(l_visitorNpcs, ARRAY_SIZE(l_visitorNpcs),
                      dScene::SCENE_FIELD, dScene::SCENE_NUM);
        BOOL here = FALSE;
        if (visitor != NULL) {
            if (isVisitorNpcHere(dItem::Item(visitor->mNpc))) {
                here = TRUE;
            }
        }
        if (here) {
            *npc = visitor->mNpc;
        } else if (dEvent::getTodayVisitor() == 3 &&
                   dEvent::isNowBetween(6, 0, 6, 0)) {
            *npc = 0x8007;
        }
    }
}

void dNpcMng_c::setShowTelop() {
    sShowTelop = 1;
}

BOOL dNpcMng_c::isEventNpcHere(const dItem::Item &npc) {
    if ((npc.mId >> 12 & 0xF) != 8) {
        return FALSE;
    }
    const Spawn_c *spawn = findSpawn(l_eventNpcs, ARRAY_SIZE(l_eventNpcs),
                                     dScene::SCENE_FIELD, dScene::SCENE_NUM);
    if (spawn != NULL) {
        if (npc.isSame(dItem::Item(spawn->mNpc))) {
            if (spawn->mIsHere == NULL ||
                spawn->mIsHere((dQuestEvent_e)spawn->mEvent)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL dNpcMng_c::isHolidayNpcHere(const dItem::Item &npc) {
    if ((npc.mId >> 12 & 0xF) != 8 && npc.mId != dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    const Spawn_c *spawn = findSpawn(l_holidayNpcs, ARRAY_SIZE(l_holidayNpcs),
                                     dScene::SCENE_FIELD, dScene::SCENE_NUM);
    if (spawn != NULL) {
        if (npc.mId != dItem::ITEM_ID_NONE || spawn->mNpc == 0x8043 ||
            spawn->mNpc == 0x8060) {
            dItem::Item spawnNpc(spawn->mNpc);
            if (npc.isSame(spawnNpc)) {
                goto here;
            }
        } else {
        here:
            if (spawn->mIsHere == NULL ||
                spawn->mIsHere((dQuestEvent_e)spawn->mEvent)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL dNpcMng_c::isVisitorNpcHere(const dItem::Item &npc) {
    if ((npc.mId >> 12 & 0xF) != 8) {
        return FALSE;
    }
    const Spawn_c *spawn = findSpawn(l_visitorNpcs, ARRAY_SIZE(l_visitorNpcs),
                                     dScene::SCENE_FIELD, dScene::SCENE_NUM);
    if (spawn != NULL) {
        if (npc.isSame(dItem::Item(spawn->mNpc))) {
            if (spawn->mIsHere == NULL ||
                spawn->mIsHere((dQuestEvent_e)spawn->mEvent)) {
                return TRUE;
            }
        }
    }
    if (npc.isSame(dItem::Item((u16)0x8007))) {
        if (dEvent::isVisitorHere(3)) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL dNpcMng_c::isWeekVisitDay() {
    if (dSaveData_c::getTown()->mWeek.mTimeChanged) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour < 9 || now.hour >= 19) {
        return FALSE;
    }
    if (!isWeekVisitWeek()) {
        return FALSE;
    }
    dTime_c base;
    base.set(2000, 0, 1, 0, 0, 0);
    base.normalize();
    base.add(-(base.wday - 1), 0, 0, 0);
    int week = dTime_c::diffDays(&now, &base, FALSE) / 7;
    return (l_weekBits[week / 32] & (1 << (week % 32))) != 0;
}

BOOL dNpcMng_c::isWeekVisitWeek() {
    dTime_c now = *dTime_c::getCurrent();
    int kind = dWeather::getCityPattern(&now);
    if (!((kind >= 0 && kind <= 6) || (kind >= 7 && kind <= 8) ||
          (kind >= 16 && kind <= 18) || (kind >= 19 && kind <= 21) ||
          (kind >= 22 && kind <= 25))) {
        return FALSE;
    }
    return (u32)(dWeather::getCityWeather(&now) - 3) > 3;
}

const dNpcMng_c::Spawn_c dNpcMng_c::l_eventNpcs[7] = {
    {&isTownPlay, &dNpcMng_c::setPosTownHallA, &isFishingTourneyTime,
     EVENT_FISHING_TOURNEY, fProf::A_NPC_SP_UOMASA, 0x8025},
    {&isTownPlay, &dNpcMng_c::setPosRandomSpot, NULL, EVENT_HALLOWEEN,
     fProf::A_NPC_SP_PUMPKING, 0x8006},
    {&isTownPlay, &dNpcMng_c::setPosHarvest, &isHarvestTime,
     EVENT_HARVEST_FESTIVAL, fProf::A_NPC_SP_FRANKLIN, 0x801E},
    {&isTownPlay, &dNpcMng_c::setPosRandomSpot, &isToyDayTime, EVENT_TOY_DAY,
     fProf::A_NPC_SP_JINGLE, 0x8024},
    {&isTownPlay, &dNpcMng_c::setPosTownHallA, NULL, EVENT_BUNNY_DAY,
     fProf::A_NPC_SP_PYONTARO, 0x804A},
    {&isTownPlay, &dNpcMng_c::setPosCarnival, NULL, EVENT_FESTIVALE,
     fProf::A_NPC_SP_DANCER, 0x804D},
    {&isTownPlay, &dNpcMng_c::setPosTownHallA, &isBugOffTime, EVENT_BUG_OFF,
     fProf::A_NPC_SP_BUG, 0x804C},
};

const dNpcMng_c::Spawn_c dNpcMng_c::l_holidayNpcs[25] = {
    {&isTownPlay, &dNpcMng_c::setPosTownHallRight, &isFireworksTime,
     EVENT_HARVEST_FESTIVAL, fProf::A_NPC_SP_EVENT_SONCHO_07E, 0x802E},
    {&isTownPlay, &dNpcMng_c::setPosTownHallA, &isEventActive, EVENT_COUNTDOWN,
     fProf::A_NPC_SP_EVENT_SONCHO_07C, 0x802C},
    {&isTownPlay, &dNpcMng_c::setPosTownHallC, &isHolidayTime,
     EVENT_NEW_YEARS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_07D, 0x802D},
    {&isTownPlay, &dNpcMng_c::setPosTownHallB, &isHolidayDay, EVENT_FIREWORKS,
     fProf::A_NPC_SP_EVENT_SONCHO_07F, 0x802F},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_JP_SETSUBUN, fProf::A_NPC_SP_EVENT_SONCHO_080, 0x8030},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_JP_GIRLS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_081, 0x8031},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_JP_CHILDRENS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_082, 0x8032},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_JP_AUTUMN_MOON, fProf::A_NPC_SP_EVENT_SONCHO_083, 0x8033},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_JP_TANABATA, fProf::A_NPC_SP_EVENT_SONCHO_084, 0x8034},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_NA_GROUNDHOG_DAY, fProf::A_NPC_SP_EVENT_SONCHO_085, 0x8035},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_NA_NATURE_DAY, fProf::A_NPC_SP_EVENT_SONCHO_086, 0x8036},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_NA_LABOR_DAY, fProf::A_NPC_SP_EVENT_SONCHO_087, 0x8037},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_NA_EXPLORERS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_088, 0x8038},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_NA_AUTUMN_MOON, fProf::A_NPC_SP_EVENT_SONCHO_089, 0x8039},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_EU_MIDSUMMERS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_08A, 0x803A},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_EU_NAUGHTY_OR_NICE_DAY, fProf::A_NPC_SP_EVENT_SONCHO_08B, 0x803B},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_EU_MIDWINTERS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_08C, 0x803C},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_EU_AUTUMN_MOON, fProf::A_NPC_SP_EVENT_SONCHO_08D, 0x803D},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_KR_LUNAR_NEW_YEAR, fProf::A_NPC_SP_EVENT_SONCHO_08E, 0x803E},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_KR_ARBOR_DAY, fProf::A_NPC_SP_EVENT_SONCHO_08F, 0x803F},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_KR_TEACHERS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_090, 0x8040},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_KR_DAEBOREUM, fProf::A_NPC_SP_EVENT_SONCHO_091, 0x8041},
    {&isTownPlay, &dNpcMng_c::setPosTownHallLeft, &isHolidayTime,
     EVENT_APRIL_FOOLS_DAY, fProf::A_NPC_SP_EVENT_SONCHO_092, 0x8042},
    {&isConstructionTime, &dNpcMng_c::setPosConstruction, NULL, 0xFFFF,
     fProf::A_NPC_SP_CEREMONY_SONCHO, 0x8043},
    {&isCatalogTime, &dNpcMng_c::setPosPlayerHouseA, NULL, 0xFFFF,
     fProf::A_NPC_SP_GOLDITEM_SONCHO, 0x8060},
};

const dNpcMng_c::Spawn_c dNpcMng_c::l_visitorNpcs[15] = {
    {&isHouseWarmingTime, &dNpcMng_c::setPosPrevHouse, NULL, 0xFFFF,
     fProf::A_NPC_SP_OP_TANUKICHI, 0x8002},
    {&isLetterVisitFirstTime, &dNpcMng_c::setPosPlayerHouseDoor, NULL, 0xFFFF,
     fProf::A_NPC_SP_RESETSAN, 0x8011},
    {&isResetVisitTime, &dNpcMng_c::setPosResetVisit, NULL, 0xFFFF,
     fProf::A_NPC_SP_SEIICHI, 0x8016},
    {&isBirthdayVisitTime, &dNpcMng_c::setPosPlayerHouseB, NULL, 0xFFFF,
     fProf::A_NPC_SP_PERIO, 0x8023},
    {&isLetterVisitKind1, &dNpcMng_c::setPosPlayerHouseDoor, NULL, 0xFFFF,
     fProf::A_NPC_SP_RESETSAN, 0x8011},
    {&isVisitor2Day, &dNpcMng_c::setPosVisitor2, &isVisitor2Here, 0xFFFF,
     fProf::A_NPC_SP_KABURIBA, 0x8001},
    {&isFlag13Visit, &dNpcMng_c::setPosBeach, NULL, 0xFFFF,
     fProf::A_NPC_SP_JONNY, 0x8015},
    {&isGuestVisitTime, &dNpcMng_c::setPosGuestVisit, NULL, 0xFFFF,
     fProf::A_NPC_SP_MISSING_MOTHER, 0x801A},
    {&isHostVisitTime, &dNpcMng_c::setPosHostVisit, NULL, 0xFFFF,
     fProf::A_NPC_SP_MISSING_CHILD, 0x8019},
    {&isRelayDesignVisitTime, &dNpcMng_c::setPosRandomSpot, NULL, 0xFFFF,
     fProf::A_NPC_SP_MYSTERYCAT, 0x8013},
    {&isVisitor5Day, &dNpcMng_c::setPosVisitor5, NULL, 0xFFFF,
     fProf::A_NPC_SP_YUUTARO, 0x801B},
    {&isVisitor9Day, &dNpcMng_c::setPosVisitor9, &isVisitor9Here, 0xFFFF,
     fProf::A_NPC_SP_RORAN, 0x801D},
    {&isVisitor8Day, &dNpcMng_c::setPosRaccoSpot, &isNotTownFlag5, 0xFFFF,
     fProf::A_NPC_SP_RACCO, 0x801C},
    {&isVisitor7Day, &dNpcMng_c::setPosGateA, &isVisitor7Here, 0xFFFF,
     fProf::A_NPC_SP_SEIICHI, 0x8016},
    {&isVisitor4Day, &dNpcMng_c::setPosGateB, &isVisitor4Time, 0xFFFF,
     fProf::A_NPC_SP_PERIO, 0x8023},
};
