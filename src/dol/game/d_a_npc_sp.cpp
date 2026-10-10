// The special npc actor base dAcNpcSp_c and its resMng_c / receptSp_c.
// .text 80061ADC..80063438. See include/game/game/d_a_npc_sp.hpp and notes/d_a_npc_sp.txt.
// As in d_a_npc_nml: these three first (weak RTTI data order).
#include <game/game/d_string.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_a_npc_sp.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_npc_mdl_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_police_box.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/mLib/m_heap.hpp>
#include <revolution/OS.h>
#include <cstdio>

// Tuning values of the special npcs' wandering (literals in the code).
#define SP_WANDER_MAX_DIST 128         // mWanderMaxDist default
#define SP_WANDER_MIN_DIST 46          // mWanderMinDist default
#define SP_STUCK_FRAMES 120            // checkStuck: frames pushed or walking into a wall
#define SP_LOOK_WAIT_MIN 120.0f        // frames until the next look-around: at least ...
#define SP_LOOK_WAIT_RAND 600.0f       // ... plus up to this many
#define SP_ROUTE_DIST_NONE (-32.0f)    // getRouteStep: no route candidate yet
#define SP_ROUTE_BACK_COST 80.0f       // getRouteStep: extra distance for turning back
#define SP_ROUTE_SIDE_COST 48.0f       // getRouteStep: extra distance for turning sideways
#define SP_ROUTE_TARGET_NEAR 80.0f     // getRouteStep: stay put when the target is this close and blocked
#define SP_TWINS_STRIP_X_MIN 64.0f     // getRouteStep: the strip Tommy and Timmy keep clear ...
#define SP_TWINS_STRIP_X_MAX 256.0f    // ... x range ...
#define SP_TWINS_STRIP_Z_RANGE 6.4f    // ... and z distance from the npc's unit row
#define SP_STEP_HEIGHT_MAX 48.0f       // calcFreePosToward: stop at a height change of this much
#define SP_HEAP_EXTRA 0x2800           // calcHeapSize: heap beyond the model file
#define SP_FRM_HEAP_SIZE 0x20          // calcHeapSize: the frame heap's own size

// .bss 805654D0: the unit offsets of the four route directions (getRouteStep, mRouteDir).
static mVec3_c l_routeOfs[4] = {
    mVec3_c(0.0f, 0.0f, -mFI_UNIT_BASE_SIZE_F),
    mVec3_c(-mFI_UNIT_BASE_SIZE_F, 0.0f, 0.0f),
    mVec3_c(0.0f, 0.0f, mFI_UNIT_BASE_SIZE_F),
    mVec3_c(mFI_UNIT_BASE_SIZE_F, 0.0f, 0.0f),
};

// 80061ADC
u32 dAcNpcSp_c::getHeapSize() {
    return calcHeapSize(FALSE);
}

// 80061AE4
u32 dAcNpcSp_c::addToNpcList() {
    return fn_800F9888(this, &mNpcItem.mId);
}

// 80061AEC
void dAcNpcSp_c::removeFromNpcList() {
    fn_800F98A0(this);
}

// 80061AF0
int dAcNpcSp_c::getFaceType() {
    return mNpcListSlot + 9;
}

// 80061AFC
void dAcNpcSp_c::getName(dHmnName::Word_c *name, int len) {
    getNpcName(name, &mNpcItem, len);
}

// 80061B0C
u8 dAcNpcSp_c::getNameKind() {
    return GENDER_OTHER;
}

// 80061B14
f32 dAcNpcSp_c::getHandItemOfsX() const {
    return getSpHandItemOfs(&mNpcItem, 0);
}

// 80061B20
f32 dAcNpcSp_c::getHandItemOfsZ() const {
    return getSpHandItemOfs(&mNpcItem, 1);
}

// 80061B2C
int dAcNpcSp_c::getEarType() {
    return NPC_EAR_TYPE_NONE;
}

// 80061B34
int dAcNpcSp_c::getVoiceType() const {
    return fn_800F8914(&mNpcItem);
}

// 80061B3C
mVec3_c dAcNpcSp_c::getRouteStep(const mVec3_c *target) {
    static const int l_dirBits[4] = {1, 2, 4, 8};

    f32 best;
    int i;
    f32 dist;
    mVec3_c pos = *target;
    dFdBase_c *fd = getCurrentField();
    int dirs = fd->getRouteDirs(&mPos);
    int targetDirs = fd->getRouteDirs(target);
    mVec3_c center;
    mVec3_c targetCenter;
    mVec3_c p;
    mVec3_c q;
    dFdBase_c::snapToUnitCenter(center, &mPos);
    dFdBase_c::snapToUnitCenter(targetCenter, target);
    center.y = 0.0f;
    targetCenter.y = 0.0f;

    if (dirs == 0) {
        best = 0.0f;
        for (i = 0; i < 4; i++) {
            p = center + l_routeOfs[i];
            dist = p.xzDistTo(mPos);
            int pDirs = fd->getRouteDirs(&p);
            int unitX = worldToUnit(p.x);
            int unitZ = worldToUnit(p.z);
            dPlayerActor_c *player = getPlayerOnUnit(unitX, unitZ);
            if (pDirs != 0 && (player != NULL || canPutItemAt(&p, &mNpcItem, NULL, SCENE_NUM, TRUE, TRUE))) {
                if (!best || best > dist) {
                    best = dist;
                    mRouteDir = i;
                }
            }
        }
        if (best) {
            pos = center + l_routeOfs[mRouteDir];
        } else {
            pos = *target;
        }
    } else {
        int cur = mRouteDir;
        int back = 0;
        best = SP_ROUTE_DIST_NONE;
        f32 targetDist = nw4r::math::FAbs(center.x - target->x) + nw4r::math::FAbs(center.z - target->z);
        if (mNpcItem.getId() == 0x8010 || mNpcItem.getId() == 0x800F) { // Timmy, Tommy
            if (targetCenter.x >= SP_TWINS_STRIP_X_MIN && targetCenter.x < SP_TWINS_STRIP_X_MAX &&
                nw4r::math::FAbs(targetCenter.z - center.z) < SP_TWINS_STRIP_Z_RANGE) {
                mVec3_c p = center + l_routeOfs[cur];
                int unitX = worldToUnit(p.x);
                int unitZ = worldToUnit(p.z);
                dActor_c *actor = getActorOnUnit(unitX, unitZ);
                if (actor != NULL && mNpcItem.mId != actor->mParam) {
                    return mPos;
                }
                p.x += mFI_UNIT_BASE_SIZE_F;
            }
        }
        switch (cur) {
        case 0:
            back = 2;
            break;
        case 1:
            back = 3;
            break;
        case 2:
            back = 0;
            break;
        case 3:
            back = 1;
            break;
        }
        for (int i = 0; i < 4; i++) {
            if (dirs & l_dirBits[i]) {
                q = center + l_routeOfs[i];
                dist = nw4r::math::FAbs(targetCenter.x - q.x) + nw4r::math::FAbs(targetCenter.z - q.z);
                if (i != cur) {
                    if (i == back) {
                        dist += SP_ROUTE_BACK_COST;
                    } else {
                        dist += SP_ROUTE_SIDE_COST;
                    }
                }
                int unitX = worldToUnit(q.x);
                int unitZ = worldToUnit(q.z);
                dPlayerActor_c *player = getPlayerOnUnit(unitX, unitZ);
                BOOL ok = player != NULL || canPutItemAt(&q, &dItem::Item(), NULL, SCENE_NUM, TRUE, TRUE);
                if (ok && (best < 0.0f || best > dist)) {
                    best = dist;
                    mRouteDir = i;
                }
            }
        }
        if (targetDirs == 0 && best > targetDist && targetDist < SP_ROUTE_TARGET_NEAR) {
            pos = mPos;
        } else {
            pos = center + l_routeOfs[mRouteDir];
        }
    }
    return pos;
}

// 800620A4
int dAcNpcSp_c::getSoundId() {
    return fn_800F88E0(&mNpcItem);
}

// 800620AC
void dAcNpcSp_c::playSound() {
    if (!mSoundPlayed) {
        dAcNpc_c::playSound();
        mSoundPlayed = TRUE;
    }
}

// 800620EC
const Vec *dAcNpcSp_c::searchPosTable(const posTable97_s *table, int num, int key, const dItem::Item *item) {
    if (table == NULL || num == 0) {
        return NULL;
    }
    if (ITEM_NAME_TYPE(item->mId) != 8) {
        return NULL;
    }
    u32 idx = ITEM_NAME_INDEX(item->mId);
    if (idx >= NPC_SPECIAL_NUM) {
        return NULL;
    }
    for (u32 i = 0; i < num; i++, table++) {
        if (table->mKey == key) {
            return &table->mPos[idx];
        }
    }
    return NULL;
}

// 80062168
void dAcNpcSp_c::getManpuOfs(mVec3_c *ofs, mVec3_c *ofsL, mVec3_c *ofsR, u8 type) {
    if (ofs != NULL && ofsL != NULL && ofsR != NULL && type < MANPU_TYPE_NUM) {
        const dItem::Item *item = &mNpcItem;
        if (ITEM_NAME_TYPE(item->mId) == 8) {
            const Vec *pos = searchPosTable(l_manpuOfsSp, l_manpuOfsSpNum, type, item);
            if (pos != NULL) {
                ofs->x = pos->x;
                ofs->y = pos->y;
                ofs->z = pos->z;
            }
            pos = searchPosTable(l_manpuOfsSpL, l_manpuOfsSpLNum, type, item);
            if (pos != NULL) {
                ofsL->x = pos->x;
                ofsL->y = pos->y;
                ofsL->z = pos->z;
            }
            pos = searchPosTable(l_manpuOfsSpR, l_manpuOfsSpRNum, type, item);
            if (pos != NULL) {
                ofsR->x = pos->x;
                ofsR->y = pos->y;
                ofsR->z = pos->z;
            }
        }
    }
}

// 80062280
int dAcNpcSp_c::preCreate() {
    if (dAcNpc_c::preCreate() == NOT_READY) {
        return NOT_READY;
    }
    mpRes = &mRes;
    return SUCCEEDED;
}

// 800622C8
int dAcNpcSp_c::create() {
    if (dAcNpc_c::create() == NOT_READY) {
        return NOT_READY;
    }
    mWaitTimer = 0;
    mWandering = 0;
    mHomePos = mPos;
    mWanderMaxDist = SP_WANDER_MAX_DIST;
    mWanderMinDist = SP_WANDER_MIN_DIST;
    mRouteDir = 2;
    _1C6C = 0;
    mObjc.setDefWeight(0xFF);
    if (!isLocalOwner()) {
        getDaubPos(&mPos, &mAngle.y);
        mAngle3D.y = mAngle.y;
    }
    return SUCCEEDED;
}

// 8006237C
int dAcNpcSp_c::doDelete() {
    return dAcNpc_c::doDelete() != NOT_READY;
}

// 800623A8
void dAcNpcSp_c::demoHook50() {
    dAcNpc_c::demoHook50();
}

// 800623AC
bool dAcNpcSp_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpc_c::canInteract(actor)) {
        return false;
    }
    if (isInUse(NULL)) {
        return false;
    }
    return !mTalk.isBusy();
}

// 80062414
BOOL dAcNpcSp_c::getPointedShopItem(dItem::Item *item, int *unitX, int *unitZ) {
    if (!getPointedUnit(unitX, unitZ, &l_pointAngle, 0, l_pointRange, l_pointDist)) {
        return FALSE;
    }
    dFdBase_c *fd = fn_80190C44(0);
    if (fd == NULL) {
        return FALSE;
    }
    *item = fd->getUnitItem(*unitX, *unitZ);
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (!dItem::isRealItemId(item->mId)) {
        return FALSE;
    }
    if (dShopLayout_isSlotSold(*unitX, *unitZ)) {
        return FALSE;
    }
    return TRUE;
}

// 80062504
mVec3_c dAcNpcSp_c::calcFreePosToward(const mVec3_c *target) {
    mVec3_c step = *target;
    step -= mPos;
    if (!step.normalizeRS()) {
        return mPos;
    }
    step.x *= mFI_UNIT_BASE_SIZE_F;
    step.y = 0.0f;
    step.z *= mFI_UNIT_BASE_SIZE_F;
    mVec3_c pos = mPos;
    mVec3_c last = mPos;
    do {
        f32 dist = (pos - *target).xzLen();
        last = pos;
        if (dist > mFI_UNIT_BASE_SIZE_F) {
            pos += step;
        } else {
            f32 d = (pos - *target).xzLen();
            if (d > 0.0f) {
                pos = *target;
            } else {
                break;
            }
        }
    } while (std::fabs(mPos.y - pos.y) < SP_STEP_HEIGHT_MAX && canPutItemAt(&pos, &mNpcItem, NULL, SCENE_NUM, TRUE, TRUE));
    int unitX = worldToUnit(last.x);
    int unitZ = worldToUnit(last.z);
    dFdBase_c::getUnitCenterPos(last, unitX, unitZ);
    return last;
}

// 80062780
dAcNpcSp_c::receptSp_c::receptSp_c() {}

// 800627BC
dAcNpcSp_c::receptSp_c::~receptSp_c() {}

// 80062814
dAcNpcSp_c::resMng_c::resMng_c() : mpData(NULL) {}

// 80062858
dAcNpcSp_c::resMng_c::~resMng_c() {}

// 800628B0
BOOL dAcNpcSp_c::resMng_c::create(dAcNpc_c *npc) {
    if (mpData == NULL) {
        char path[32];
        sprintf(path, "/Npc/Special/Model/%d.brres", ITEM_NAME_INDEX(npc->mNpcItem.mId));
        if (!npc->loadRes(&mpData, path)) {
            return FALSE;
        }
        DCFlushRange(mpData, fn_800F9E40());
        nw4r::g3d::ResFile file(getMdlRes());
        file.Init();
        file.Bind(file);
    }
    return TRUE;
}

// 80062974
void *dAcNpcSp_c::resMng_c::getMdlRes() {
    return mpData;
}

// 8006297C
void *dAcNpcSp_c::resMng_c::getTexRes() {
    return mpData;
}

// 80062984
u32 dAcNpcSp_c::calcHeapSize(BOOL withFrm) {
    u32 size = fn_800F9E40() + SP_HEAP_EXTRA;
    if (withFrm) {
        size += mHeap::frmHeapCost(0, SP_FRM_HEAP_SIZE);
    }
    return size;
}

// 800629D0
BOOL dAcNpcSp_c::searchWanderPos(mVec3_c *pos) {
    return searchPosNear(pos, &mPos, getWanderMinDist(), getWanderMaxDist());
}

// 80062A44
BOOL dAcNpcSp_c::checkStuck() {
    if (getSpeedF() != 0.0f) {
        BOOL stuck = FALSE;
        if (mWallTimer > SP_STUCK_FRAMES) {
            stuck = TRUE;
        }
        if (mPushTimer > SP_STUCK_FRAMES) {
            stuck = TRUE;
        }
        if (stuck) {
            mPushTimer = 0;
            mWallTimer = 0;
            return TRUE;
        }
    }
    return FALSE;
}

// 80062A9C
void dAcNpcSp_c::initWander() {
    mWaitTimer = 0;
    mWandering = 0;
    mWaitTimer = getWanderWaitBase() + cM::rndInt(getWanderWaitRand());
    mLookTimer = SP_LOOK_WAIT_MIN + cM::rndF(SP_LOOK_WAIT_RAND);
    mWallTimer = 0;
    mAction.requestWait(0);
}

// 80062B40
BOOL dAcNpcSp_c::isWanderWalk() {
    return cM::rndInt(100) < fn_800F59EC();
}

// 80062B88
int dAcNpcSp_c::executeWander(wanderDaub_s *daub) {
    mWandering = 1;
    if (mWaitTimer != 0) {
        mWaitTimer--;
    }
    if (getSpeedF() != 0.0f) {
        if (mAcch.mHitFlags & (dBGCF::HIT_WALL_FRONT | dBGCF::HIT_STEP_EXT_FRONT)) {
            if (mWallTimer < 0xFFFFFFFF) {
                mWallTimer++;
            }
        } else {
            mWallTimer = 0;
        }
    } else {
        mWallTimer = 0;
    }

    if (!checkStuck()) {
        action_c *action = &mAction;
        if (action->mCanChange && (action->mActionId != ACTION_WAIT || mWaitTimer == 0)) {
            mVec3_c pos = mPos;
            if (isWanderWalk() && searchWanderPos(&pos)) {
                mAng angle = 0;
                if (checkTargetAngle(&angle, &pos, &l_frontAngle)) {
                    action->requestWalk(0, pos, l_walkTurnSpeed, 0, l_moveParamWalk, l_defaultAnmRate, cNpcMorphFrames);
                } else {
                    action->requestTurnWalk(0, pos, angle, l_turnSpeed, 0, l_moveParamWalk, l_defaultAnmRate, cNpcMorphFrames);
                }
            } else {
                mWaitTimer = getWanderWaitBase() + cM::rndInt(getWanderWaitRand());
                if (action->mActionId != ACTION_WAIT) {
                    action->requestWait(0);
                }
            }
        }

        if (lookAt_c::searchPlayer(fn_800DCF58(), this, l_lookFov, cNpcLookRange) == fn_800DCF58()) {
            mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep, cNpcLookRange);
            mLookTimer = SP_LOOK_WAIT_MIN + cM::rndF(SP_LOOK_WAIT_RAND);
        } else {
            if (mLookTimer != 0) {
                mLookTimer--;
            }
            if (mLookTimer == 0) {
                if (mLookAt.mType != lookAt_c::LOOK_AROUND) {
                    int yawStep = l_lookYawStep;
                    mLookAt.setAround(0, mAng(cM::rndInt(l_lookPitchMax >> 1)), mAng(l_lookYawMax >> 1), l_lookPitchStep,
                                      mAng((yawStep >> 1) + cM::rndInt(yawStep)));
                } else {
                    mLookAt.setPlayer(0, true, l_lookPitchStep, l_lookYawStep, cNpcLookRange);
                }
                mLookTimer = SP_LOOK_WAIT_MIN + cM::rndF(SP_LOOK_WAIT_RAND);
            }
        }
    } else {
        mAction.requestWait(0);
        stopManpu();
        if (daub != NULL) {
            daub->mEmotion = 0;
        }
        mWaitTimer = 0;
    }
    return 0;
}

// 80062ED8
BOOL dAcNpcSp_c::resetWanderDaub(wanderDaub_s *daub) {
    if (!mWandering) {
        return FALSE;
    }
    if (daub != NULL) {
        daub->mEmotion = 0;
    }
    return TRUE;
}

// 80062F08
int dAcNpcSp_c::vtE4() {
    return 0;
}

// 80062F10
void dAcNpcSp_c::resetLookTimer() {
    mLookTimer = SP_LOOK_WAIT_MIN + cM::rndF(SP_LOOK_WAIT_RAND);
}

// 80062F50
void dAcNpcSp_c::executeRemoteWander(wanderDaub_s *daub) {
    if (isLocalOwner()) {
        return;
    }
    if (daub != NULL) {
        if (daub->mEmotion == 0) {
            if (mAction.mActionId == ACTION_EMOTION) {
                stopManpu();
                mAction.requestMoveDest(0);
            }
            mAcceptMoveReq = 1;
        } else {
            int id = mAction.mActionId;
            mAcceptMoveReq = 0;
            if (id != ACTION_EMOTION || daub->mEmotion != mAction.getEmotion()) {
                stopManpu();
                mAction.requestEmotion(0, daub->mEmotion, cNpcMorphFrames);
            }
        }
    }
    if (mLookTimer != 0) {
        mLookTimer--;
    }
    if (mLookTimer == 0) {
        lookAt_c *look = &mLookAt;
        if (look->mType != lookAt_c::LOOK_AROUND) {
            mLookAt.setAround(0, mAng(cM::rndInt(l_lookPitchMax >> 1)), mAng(l_lookYawMax >> 1), l_lookPitchStep,
                              mAng((l_lookYawStep >> 1) + cM::rndInt(l_lookYawStep)));
        } else {
            look->setPlayer(0, true, l_lookPitchStep, l_lookYawStep, cNpcLookRange);
        }
        mLookTimer = SP_LOOK_WAIT_MIN + cM::rndF(SP_LOOK_WAIT_RAND);
    }
}

// 800630CC
int dAcNpcSp_c::getWanderWaitBase() {
    return fn_800F59FC();
}

// 800630D0
int dAcNpcSp_c::getWanderWaitRand() {
    return fn_800F5A0C();
}

// 800630D4
void dAcNpcSp_c::moveFtrToPoliceBox() {
    if (fn_800DCEDC() == 1 && fn_800DCF30() > 1) {
        return;
    }
    if (!isSceneAttr(getCurrentScene(), SCENE_ATTR_OUTDOOR | SCENE_ATTR_MY_TOWN)) {
        return;
    }
    dFdBase_c *fd = getCurrentField();
    if (fd == NULL) {
        return;
    }
    int unitX = worldToUnit(mPos.x);
    int unitZ = worldToUnit(mPos.z);
    dItem::Item *item = fd->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return;
    }
    u16 id = item->mId;
    if (!dItem::isRealItemId(id)) {
        return;
    }
    dSaveData_c::getTown()->mPoliceBox.push(id);
    fd->setItem(&dItem::Item(), unitX, unitZ, 0);
}
