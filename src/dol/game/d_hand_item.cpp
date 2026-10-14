// The hand item process (dHandItem_c). .text 80191500..80195290, .rodata 8047CC50..8047CE10,
// .data 804F9DB0..804FA098, .bss 80624098..80624220, .sdata 8074BF90..8074BFA0, .sbss 8074E9F8..8074EA18,
// .sdata2 80751600..80751678. See include/game/game/d_hand_item.hpp.
#include <game/game/d_hand_item.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fi_mdl_mgr.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/game/d_item_def.hpp>
#include <game/snd/snd_id.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <cmath>

CUSTOM_BASE_PROFILE(HAND_ITEM, dHandItem_c, 0xAE, 0xF);

dHandItem_c *dHandItem_c::sInstance;

const f32 dHandItem::cDefaultReach = 16.0f;

static const u8 l_appearHandEnd[] = {27, 33, 39, 27, 33, 39};
static const u8 l_eatHandEnd[] = {15, 30, 66};
static const u8 l_appearGroundEnd[] = {33, 57, 63, 69, 72, 93, 64, 79};
static const u8 l_stowGroundEnd[] = {18, 24, 60};

static const f32 l_appearHandScale[] = {1.125f, 0.875f, 1.0f, 1.125f, 0.875f, 1.0f};
static const f32 l_appearGroundScale[] = {0.0f, 1.125f, 0.875f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f};

static const dHandItem_c::stateFunc_c l_stateFuncs[dHandItem_c::STATE_NUM][dHandItem_c::VARIANT_NUM] = {
    {{&dHandItem_c::initLoad, &dHandItem_c::execLoad}, {&dHandItem_c::initLoad, &dHandItem_c::execLoad}},
    {{&dHandItem_c::initAppearHand, &dHandItem_c::execAppearHand}, {&dHandItem_c::initAppearGround, &dHandItem_c::execAppearGround}},
    {{&dHandItem_c::initFallHand, NULL}, {&dHandItem_c::initFallGround, &dHandItem_c::execFallGround}},
    {{NULL, NULL}, {&dHandItem_c::initStowGround, &dHandItem_c::execStowGround}},
    {{&dHandItem_c::initTakeHand, &dHandItem_c::execTakeHand}, {&dHandItem_c::initTakeGround, &dHandItem_c::execTakeGround}},
    {{&dHandItem_c::initHoldHand, &dHandItem_c::execHoldHand}, {&dHandItem_c::initHoldGround, &dHandItem_c::execHoldGround}},
    {{&dHandItem_c::initShrinkHand, &dHandItem_c::execShrinkHand}, {&dHandItem_c::initShrinkGround, &dHandItem_c::execShrinkGround}},
    {{&dHandItem_c::initKeepHand, &dHandItem_c::execKeepHand}, {NULL, NULL}},
    {{&dHandItem_c::initFollowHand, &dHandItem_c::execFollowHand}, {&dHandItem_c::initFollowGround, &dHandItem_c::execFollowGround}},
    {{NULL, NULL}, {NULL, NULL}},
    {{&dHandItem_c::initEatHand, &dHandItem_c::execEatHand}, {NULL, NULL}},
    {{&dHandItem_c::initUnwrapHand, &dHandItem_c::execUnwrapHand}, {NULL, NULL}},
    {{&dHandItem_c::initShowHand, &dHandItem_c::execShowHand}, {NULL, NULL}},
};

static mVec3_c l_handOfs(0.0f, -6.0f, 0.0f);
static const u8 l_appearHandDur[] = {
    l_appearHandEnd[0], l_appearHandEnd[1] - l_appearHandEnd[0], l_appearHandEnd[2] - l_appearHandEnd[1],
    l_appearHandEnd[3], l_appearHandEnd[4] - l_appearHandEnd[3], l_appearHandEnd[5] - l_appearHandEnd[4],
};
static mVec3_c l_appearHandTarget(0.0f, 0.0f, 1.0f);
static mVec3_c l_appearHandFishTarget(0.0f, 0.0f, 1.0f);
static const f32 l_appearHandScaleSpeed[] = {
    std::fabs(l_appearHandScale[0] / l_appearHandDur[0]),
    std::fabs((l_appearHandScale[1] - l_appearHandScale[0]) / l_appearHandDur[1]),
    std::fabs((l_appearHandScale[2] - l_appearHandScale[1]) / l_appearHandDur[2]),
    std::fabs(l_appearHandScale[3] / l_appearHandDur[3]),
    std::fabs((l_appearHandScale[4] - l_appearHandScale[3]) / l_appearHandDur[4]),
    std::fabs((l_appearHandScale[5] - l_appearHandScale[4]) / l_appearHandDur[5]),
};
static const u8 l_appearHandFrames = l_appearHandEnd[0];
static mVec3_c l_takeHandTarget(0.0f, 0.0f, 1.0f);
static mVec3_c l_takeHandFishTarget(0.0f, 0.0f, 1.0f);
static mVec3_c l_shrinkHandTarget(-5.0f, 0.0f, -5.0f);
static const u8 l_eatHandDur[] = {
    l_eatHandEnd[0], l_eatHandEnd[1] - l_eatHandEnd[0], l_eatHandEnd[2] - l_eatHandEnd[1],
};
static mVec3_c l_appearGroundTarget(0.0f, 0.0f, 17.0f);
static mVec3_c l_appearGroundPopOfs(0.0f, 0.0f, 5.0f);
static const u8 l_appearGroundDur[] = {
    l_appearGroundEnd[0], l_appearGroundEnd[1] - l_appearGroundEnd[0], l_appearGroundEnd[2] - l_appearGroundEnd[1],
    l_appearGroundEnd[3] - l_appearGroundEnd[2], l_appearGroundEnd[4] - l_appearGroundEnd[3], l_appearGroundEnd[5] - l_appearGroundEnd[4],
    l_appearGroundEnd[6], l_appearGroundEnd[7] - l_appearGroundEnd[6],
};
static const f32 l_appearGroundScaleSpeed[] = {
    0.0f,
    std::fabs((l_appearGroundScale[1] - l_appearGroundScale[0]) / l_appearGroundDur[1]),
    std::fabs((l_appearGroundScale[2] - l_appearGroundScale[1]) / l_appearGroundDur[2]),
    std::fabs((l_appearGroundScale[3] - l_appearGroundScale[2]) / l_appearGroundDur[3]),
    std::fabs((l_appearGroundScale[4] - l_appearGroundScale[3]) / l_appearGroundDur[4]),
    std::fabs((l_appearGroundScale[5] - l_appearGroundScale[4]) / l_appearGroundDur[5]),
    0.0f,
    std::fabs((l_appearGroundScale[7] - l_appearGroundScale[6]) / l_appearGroundDur[7]),
};
static const u8 l_stowGroundDur[] = {
    l_stowGroundEnd[0], l_stowGroundEnd[1] - l_stowGroundEnd[0], l_stowGroundEnd[2] - l_stowGroundEnd[1],
};
static mVec3_c l_groundHandOfs(-2.0f, -4.0f, 4.0f);
static mVec3_c l_unusedOfs(0.0f, 0.0f, 17.0f);
static mVec3_c l_shrinkGroundTarget(-2.0f, -5.0f, 0.0f);
static mVec3_c l_stowGroundOfs(0.0f, 0.0f, 17.0f);


// 80191530
dHandItem_c::dHandItem_c() {
    mItem = dItem::ITEM_ID_NONE;
}

// 80191580
dHandItem_c::~dHandItem_c() {}

// 801915EC
void dHandItem_c::reset() {
    mpStateFunc = NULL;
    mActors[0] = NULL;
    mOfsY[0] = 0.0f;
    mFishOfsY[0] = 0.0f;
    mOfsZ[0] = 0.0f;
    mActors[1] = NULL;
    mOfsY[1] = 0.0f;
    mFishOfsY[1] = 0.0f;
    mOfsZ[1] = 0.0f;
    mState = STATE_NONE;
    mUseType = STATE_NONE;
    mVariant = VARIANT_NUM;
    mNextState = STATE_NONE;
    mItem = dItem::ITEM_ID_NONE;
    mKind = KIND_NONE;
    mFrame = 0;
    mStep = 0;
    mDone = TRUE;
    mVisible = FALSE;
    mRange = 34.0f;
}

// 80191664
BOOL dHandItem_c::start(const dItem::Item *item, int kind, int useType, int variant, dDemoActor_c *actor,
                        dDemoActor_c *partner, f32 ofsY0, f32 ofsY1, f32 fishOfsY0, f32 fishOfsY1, f32 ofsZ0,
                        f32 ofsZ1) {
    BOOL started = FALSE;
    if ((u32)mState >= STATE_NUM && actor != NULL) {
    mKind = kind;
    mItem = *item;
    mUseType = useType;
    mVariant = variant;
    setActor(actor, 0);
    setActor(partner, 1);
    if (mVariant == VARIANT_GROUND) {
        mRange = 64.0f;
    } else {
        mRange = 34.0f;
    }
    mReach = 16.0f;
    setOfsY(0, ofsY0);
    setOfsY(1, ofsY1);
    setFishOfsY(0, fishOfsY0);
    setFishOfsY(1, fishOfsY1);
    setOfsZ(0, ofsZ0);
    setOfsZ(1, ofsZ1);
    started = TRUE;
    }
    return started;
}

// 801917E8
void dHandItem_c::release() {
    mVisible = FALSE;
    if (mItem.isFish()) {
        dFIMdlMgr_c::sInstance->release(dNet::getMyNo());
    }
    reset();
}

// 80191840
BOOL dHandItem_c::isActor(dDemoActor_c *actor, u32 idx) {
    dDemoActor_c *held;
    if (actor != NULL && (held = getActor(idx)) != NULL) {
        u32 id = held->getDemoID();
        u32 other = actor->getDemoID();
        if (other == id) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801918AC
BOOL dHandItem_c::swapActors(dDemoActor_c *actor) {
    if (isActor(actor, 1)) {
        setActor(getActor(0), 1);
        setActor(actor, 0);
        f32 tmp = getOfsY(0);
        setOfsY(0, getOfsY(1));
        setOfsY(1, tmp);
        tmp = getFishOfsY(0);
        setFishOfsY(0, getFishOfsY(1));
        setFishOfsY(1, tmp);
        tmp = getOfsZ(0);
        setOfsZ(0, getOfsZ(1));
        setOfsZ(1, tmp);
        return TRUE;
    }
    return isActor(actor, 0) != FALSE;
}

// 801919F0
BOOL dHandItem_c::request(int state, dDemoActor_c *actor, f32 reach) {
    if (isActor(actor, 0)) {
        mNextState = state;
        mReach = reach;
        return TRUE;
    }
    return FALSE;
}

// 80191A58
BOOL dHandItem_c::isState(int state) const {
    if ((state == mState && !mDone) || mNextState == state) {
        return TRUE;
    }
    return FALSE;
}

// 80191A8C
BOOL dHandItem_c::isReceiving() const {
    return mNextState == STATE_LOAD || isState(STATE_LOAD);
}

// 80191AD8
BOOL dHandItem_c::setUseType(int type, dDemoActor_c *actor) {
    if ((u32)mState < STATE_NUM && mDone && isActor(actor, 0)) {
        mUseType = type;
        return TRUE;
    }
    return FALSE;
}

// 80191B48
BOOL dHandItem_c::isWaiting() const {
    if (mState == STATE_FALL || (mState == STATE_APPEAR && !isState(STATE_APPEAR)) ||
        (mState == STATE_FOLLOW && !isState(STATE_FOLLOW))) {
        return TRUE;
    }
    return FALSE;
}

// 80191BC0
void dHandItem_c::setActor(dDemoActor_c *actor, u32 idx) {
    if (idx < ACTOR_NUM) {
        mActors[idx] = actor;
    }
}

// 80191BD8
dDemoActor_c *dHandItem_c::getActor(u32 idx) {
    if (idx < ACTOR_NUM) {
        return mActors[idx];
    }
    return NULL;
}

// 80191BF8
void dHandItem_c::setOfsY(u32 idx, f32 value) {
    if (idx < ACTOR_NUM) {
        mOfsY[idx] = value;
    }
}

// 80191C10
f32 dHandItem_c::getOfsY(u32 idx) {
    if (idx < ACTOR_NUM) {
        return mOfsY[idx];
    }
    return 0.0f;
}

// 80191C30
void dHandItem_c::setFishOfsY(u32 idx, f32 value) {
    if (idx < ACTOR_NUM) {
        mFishOfsY[idx] = value;
    }
}

// 80191C48
f32 dHandItem_c::getFishOfsY(u32 idx) {
    if (idx < ACTOR_NUM) {
        return mFishOfsY[idx];
    }
    return 0.0f;
}

// 80191C68
void dHandItem_c::setOfsZ(u32 idx, f32 value) {
    if (idx < ACTOR_NUM) {
        mOfsZ[idx] = value;
    }
}

// 80191C80
f32 dHandItem_c::getOfsZ(u32 idx) {
    if (idx < ACTOR_NUM) {
        return mOfsZ[idx];
    }
    return 0.0f;
}

// 80191CA0
void dHandItem_c::getHandPos(mVec3_c *out, u32 idx) {
    dDemoActor_c *actor = getActor(idx);
    *out = mVec3_c::Zero;
    if (actor != NULL) {
        actor->demoHook64(out);
    }
}

// 80191D0C
void dHandItem_c::toWorld(mVec3_c *out, const mVec3_c *local, u32 idx) {
    *out = *local;
    dDemoActor_c *actor = getActor(idx);
    if (actor != NULL) {
        out->rotY(actor->getAngle().y);
        out->x += actor->getPos().x;
        out->z += actor->getPos().z;
    }
}

// 80191D9C
mVec3_c *dHandItem_c::getOfs(u32 idx) {
    static mVec3_c ofs = l_handOfs;
    ofs.y = getOfsY(idx);
    ofs.z = getOfsZ(idx);
    return &ofs;
}

// 80191E4C
const mVec3_c *dHandItem_c::getFishOfs(dItem::Item *item, u32 idx) {
    if (item->isFish()) {
        static mVec3_c ofs = l_handOfs;
        ofs.y = getFishOfsY(idx);
        return &ofs;
    }
    return &mVec3_c::Zero;
}

// 80191F04
int dHandItem_c::create() {
    reset();
    sInstance = this;
    return 1;
}

// 80191F38
int dHandItem_c::doDelete() {
    release();
    sInstance = NULL;
    return 1;
}

// 80191F64
int dHandItem_c::execute() {
    mSound.setPos(&mPos);
    if ((u32)mVariant < VARIANT_NUM) {
        if ((u32)mNextState < STATE_NUM) {
            mState = mNextState;
            mFrame = 0;
            mStep = 0;
            if (isWaiting()) {
                mTimer = 120;
            }
            mpStateFunc = &l_stateFuncs[mState][mVariant];
            if (mpStateFunc != NULL && mpStateFunc->mInit != NULL) {
                (this->*mpStateFunc->mInit)();
            }
            mNextState = STATE_NONE;
        }
        if (mpStateFunc != NULL && mpStateFunc->mExec != NULL) {
            (this->*mpStateFunc->mExec)();
        }
        if (isWaiting() && mTimer != 0) {
            mTimer--;
        }
    }
    if ((u32)mState < STATE_NUM && mVisible && mItem.isFish()) {
        dFIObj_c *mdl = dFIMdlMgr_c::sInstance->getObj(dNet::getMyNo());
        if (mdl != NULL) {
            mMtx_c mtx;
            dActor_c::makeMtx(&mtx, &mPos, 0);
            mdl->mMdl.setLocalMtx(&mtx);
            mdl->mMdl.play();
        }
    }
    return 1;
}

// 80192128
int dHandItem_c::draw() {
    if ((u32)mState < STATE_NUM && mVisible && mItem.mId != dItem::ITEM_ID_NONE && mScale > 0.0f) {
        mVec3_c scale(mScale, mScale, mScale);
        if (mItem.isFish()) {
            dFIObj_c *mdl = dFIMdlMgr_c::sInstance->getObj(dNet::getMyNo());
            if (mdl != NULL) {
                mdl->mMdl.setScale(scale);
                mdl->mMdl.entry();
            }
        } else {
            dItem::Item item = mItem;
            BOOL present = mKind == KIND_PRESENT;
            if (present || mKind == KIND_WRAPPED) {
                item.setFromIndex(dItem::ITEM_IDX_PRESENT);
            }
            fgMngProc_callFgObjCallback(item.mId, (int)&mPos, (int)&scale, present);
        }
    }
    return 1;
}

// 80192258
void dHandItem_c::initLoad() {
    mDone = FALSE;
}

// 80192264
void dHandItem_c::execLoad() {
    BOOL ready = FALSE;
    if (mItem.isFish()) {
        if (dFIMdlMgr_c::sInstance->request(dNet::getMyNo(), dFIMdlBank_c::TYPE_FISH, mItem, TRUE, dFIMdlBank_c::MODE_GET)) {
            ready = TRUE;
        }
    } else {
        ready = TRUE;
    }
    if (ready) {
        mState = STATE_NONE;
        mDone = TRUE;
    }
}

// 801922F4
void dHandItem_c::initAppearHand() {
    getHandPos(&mPos, 0);
    if (mItem.isFish()) {
        mVec3_c target = l_appearHandFishTarget;
        target.z += mReach;
        const mVec3_c *ofs = getFishOfs(&mItem, 0);
        target += *ofs;
        mSpeed.x = std::fabs(target.x / l_appearHandFrames);
        mSpeed.y = std::fabs(target.y / l_appearHandFrames);
        mSpeed.z = std::fabs(target.z / l_appearHandFrames);
        mTarget = target;
        mOffset = mVec3_c::Zero;
        mStep = 3;
    } else {
        mVec3_c target = l_appearHandTarget;
        target.z += mReach;
        const mVec3_c *ofs = getOfs(0);
        target += *ofs;
        toWorld(&mTarget, &target, 0);
        mSpeed.x = std::fabs((mTarget.x - mPos.x) / l_appearHandFrames);
        mSpeed.y = std::fabs(mTarget.y / l_appearHandFrames);
        mSpeed.z = std::fabs((mTarget.z - mPos.z) / l_appearHandFrames);
        mStep = 0;
    }
    mVisible = TRUE;
    mScale = 0.0f;
    mDone = FALSE;
}

// 80192554
void dHandItem_c::appearHandGrow() {
    sLib::chase(&mScale, l_appearHandScale[mStep], l_appearHandScaleSpeed[mStep]);
    if (mFrame == 12) {
        mSound.startSound(SE_toridashi);
    }
    mVec3_c hand;
    getHandPos(&hand, 0);
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y + hand.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= l_appearHandEnd[mStep]) {
        mStep++;
        if (mStep > 2) {
            mTimer = 120;
            mStep = 6;
            mDone = TRUE;
        }
    }
    mFrame++;
}

// 80192654
void dHandItem_c::appearHandGrowFish() {
    sLib::chase(&mScale, l_appearHandScale[mStep], l_appearHandScaleSpeed[mStep]);
    if (mFrame == 12) {
        mSound.startSound(SE_toridashi);
    }
    mVec3_c hand;
    getHandPos(&hand, 0);
    sLib::chase(&mOffset.x, mTarget.x, mSpeed.x);
    sLib::chase(&mOffset.y, mTarget.y, mSpeed.y);
    sLib::chase(&mOffset.z, mTarget.z, mSpeed.z);
    mVec3_c pos = mOffset;
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mVec3_c local = hand - actor->getPos();
        local.rotY(-actor->getAngle().y);
        pos.x = local.x;
        pos.y += local.y;
        pos.rotY(actor->getAngle().y);
        pos += actor->getPos();
    }
    mPos = pos;
    if (mFrame >= l_appearHandEnd[mStep]) {
        mStep++;
        if (mStep >= 6) {
            mTimer = 120;
            mDone = TRUE;
        }
    }
    mFrame++;
}

// 80192818
void dHandItem_c::execAppearHand() {
    static const StateFunc steps[] = {
        &dHandItem_c::appearHandGrow, &dHandItem_c::appearHandGrow, &dHandItem_c::appearHandGrow,
        &dHandItem_c::appearHandGrowFish, &dHandItem_c::appearHandGrowFish, &dHandItem_c::appearHandGrowFish,
    };
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80192858
void dHandItem_c::initFallHand() {
    mDone = FALSE;
}

// 80192864
void dHandItem_c::initTakeHand() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    if (mItem.isFish()) {
        mVec3_c target = l_takeHandFishTarget;
        target.z += mReach;
        const mVec3_c *ofs = getFishOfs(&mItem, 0);
        target += *ofs;
        dDemoActor_c *actor = getActor(0);
        if (actor != NULL) {
            mVec3_c local = hand - actor->getPos();
            local.rotY(-actor->getAngle().y);
            target.x = local.x;
            target.y += local.y;
            target.rotY(actor->getAngle().y);
            target += actor->getPos();
        }
        mOffset = target - hand;
        mTarget = target;
    } else {
        mVec3_c target = l_takeHandTarget;
        target.z += mReach;
        const mVec3_c *ofs = getOfs(0);
        target += *ofs;
        toWorld(&mTarget, &target, 0);
        mTarget.y += hand.y;
    }
    mDone = FALSE;
    mSpeed.x = std::fabs((mTarget.x - mPos.x) / 10.0f);
    mSpeed.y = std::fabs((mTarget.y - mPos.y) / 10.0f);
    mSpeed.z = std::fabs((mTarget.z - mPos.z) / 10.0f);
}

// 80192AE8
void dHandItem_c::takeHandMove() {
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= 10) {
        mDone = TRUE;
        mStep = 1;
        if (!mItem.isFish()) {
            mVec3_c hand;
            getHandPos(&hand, 0);
            mOffset = mPos - hand;
        }
    }
    mFrame++;
}

// 80192BAC
void dHandItem_c::execTakeHand() {
    static const StateFunc steps[] = {&dHandItem_c::takeHandMove};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80192BEC
void dHandItem_c::initHoldHand() {
    mItem.isFish();
    mStep = 1;
    mDone = FALSE;
}

// 80192C2C
void dHandItem_c::holdHandMoveTo() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    mPos.y = hand.y + mTarget.y;
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (++mFrame >= 24) {
        mStep = 2;
        mDone = TRUE;
    }
}

// 80192CB8
void dHandItem_c::holdHandTrack() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mPos = hand + mOffset;
    if (++mFrame >= 24) {
        mStep = 2;
        mDone = TRUE;
    }
}

// 80192D3C
void dHandItem_c::execHoldHand() {
    static const StateFunc steps[] = {&dHandItem_c::holdHandMoveTo, &dHandItem_c::holdHandTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80192D7C
void dHandItem_c::initKeepHand() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mDone = TRUE;
}

// 80192DE0
void dHandItem_c::keepHandTrack() {
    getHandPos(&mPos, 0);
    mPos += mOffset;
}

// 80192E44
void dHandItem_c::execKeepHand() {
    static const StateFunc steps[] = {&dHandItem_c::keepHandTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80192E84
void dHandItem_c::initShrinkHand() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mTarget = l_shrinkHandTarget;
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mTarget.rotY(actor->getAngle().y);
    }
    mDone = FALSE;
    mSpeed.x = std::fabs((mTarget.x - mOffset.x) / 18.0f);
    mSpeed.y = std::fabs((mTarget.y - mOffset.y) / 18.0f);
    mSpeed.z = std::fabs((mTarget.z - mOffset.z) / 18.0f);
    mScaleSpeed = std::fabs(mScale / 18.0f);
}

// 80192F9C
void dHandItem_c::shrinkHandAway() {
    sLib::chase(&mScale, 0.0f, mScaleSpeed);
    sLib::chase(&mOffset.x, mTarget.x, mSpeed.x);
    sLib::chase(&mOffset.y, mTarget.y, mSpeed.y);
    sLib::chase(&mOffset.z, mTarget.z, mSpeed.z);
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (mFrame >= 18) {
        mStep = 1;
        mDone = TRUE;
    }
    mFrame++;
}

// 80193068
void dHandItem_c::execShrinkHand() {
    static const StateFunc steps[] = {&dHandItem_c::shrinkHandAway};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 801930A8
void dHandItem_c::initFollowHand() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mDone = FALSE;
}

// 8019310C
void dHandItem_c::followHandTrack() {
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (++mFrame >= 24) {
        mStep = 1;
        mDone = TRUE;
    }
}

// 80193194
void dHandItem_c::execFollowHand() {
    static const StateFunc steps[] = {&dHandItem_c::followHandTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 801931D4
void dHandItem_c::initUnwrapHand() {
    mDone = FALSE;
    if (mKind == KIND_WRAPPED || mKind == KIND_PRESENT) {
        mKind = KIND_ITEM;
    }
    dEffect::create("af_itm_presentbox", &mPos, NULL, NULL);
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mSound.startSound(SE_PresentOpen);
}

// 80193274
void dHandItem_c::unwrapHandTrack() {
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (mFrame >= 30) {
        mDone = TRUE;
    }
    mFrame++;
}

// 801932F8
void dHandItem_c::execUnwrapHand() {
    static const StateFunc steps[] = {&dHandItem_c::unwrapHandTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80193338
void dHandItem_c::initEatHand() {
    mVec3_c target = l_appearHandTarget;
    target.z += mReach;
    if (mItem.isFish()) {
        const mVec3_c *ofs = getFishOfs(&mItem, 0);
        target += *ofs;
    } else {
        const mVec3_c *ofs = getOfs(0);
        target += *ofs;
    }
    toWorld(&mTarget, &target, 0);
    u8 frames = l_eatHandDur[0];
    mSpeed.x = std::fabs((mTarget.x - mPos.x) / frames);
    mVec3_c hand;
    getHandPos(&hand, 0);
    mTarget.y = mPos.y - hand.y;
    mStep = 0;
    mDone = FALSE;
    mSpeed.z = std::fabs((mTarget.z - mPos.z) / frames);
}

// 801934D4
void dHandItem_c::eatHandToMouth() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    mPos.y = hand.y + mTarget.y;
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= l_eatHandEnd[mStep]) {
        mStep = 1;
        mOffset = mPos - hand;
    }
    mFrame++;
}

// 8019358C
void dHandItem_c::eatHandTrack() {
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (mFrame >= l_eatHandEnd[mStep]) {
        u8 dur = l_eatHandDur[2];
        mStep = 2;
        mScaleSpeed = std::fabs(mScale / dur);
    }
    mFrame++;
}

// 80193650
void dHandItem_c::eatHandShrink() {
    sLib::chase(&mScale, 0.0f, mScaleSpeed);
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (mFrame >= l_eatHandEnd[mStep]) {
        mStep = 3;
        mDone = TRUE;
    }
    mFrame++;
}

// 801936FC
void dHandItem_c::execEatHand() {
    static const StateFunc steps[] = {
        &dHandItem_c::eatHandToMouth, &dHandItem_c::eatHandTrack, &dHandItem_c::eatHandShrink,
    };
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 8019373C
void dHandItem_c::initShowHand() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mDone = FALSE;
}

// 801937A0
void dHandItem_c::showHandTrack() {
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (++mFrame >= 160) {
        mStep = 1;
        mDone = TRUE;
    }
}

// 80193828
void dHandItem_c::execShowHand() {
    static const StateFunc steps[] = {&dHandItem_c::showHandTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80193868
void dHandItem_c::initAppearGround() {
    mOffset = mVec3_c::Zero;
    mScale = 0.0f;
    mVisible = TRUE;
    mDone = FALSE;
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        if (actor->mProfName == dPlayerMngr_c::PLAYER_PROFILE) {
            mStep = 0;
        } else {
            mStep = 6;
        }
    }
}

// 801938FC
void dHandItem_c::appearGroundWait() {
    if (mFrame >= l_appearGroundEnd[mStep]) {
        getHandPos(&mPos, 0);
        mVec3_c target = l_appearGroundTarget;
        toWorld(&mTarget, &target, 0);
        f32 frames = (int)(l_appearGroundEnd[3] - l_appearGroundEnd[0]);
        mSpeed.x = std::fabs((mTarget.x - mPos.x) / frames);
        mSpeed.y = std::fabs(mTarget.y / frames);
        mSpeed.z = std::fabs((mTarget.z - mPos.z) / frames);
        mScaleSpeed = l_appearGroundScaleSpeed[1];
        mStep = 1;
    }
    mFrame++;
}

// 80193A0C
void dHandItem_c::appearGroundRise() {
    sLib::chase(&mScale, l_appearGroundScale[mStep], mScaleSpeed);
    if (mFrame == 45) {
        mSound.startSound(SE_toridashi);
    }
    getHandPos(&mPos, 0);
    if (mFrame >= l_appearGroundEnd[mStep]) {
        mStep++;
        if (mStep == 5) {
            getDropPos(&mTarget);
            u8 frames = l_appearGroundDur[mStep];
            mSpeed.x = std::fabs((mTarget.x - mPos.x) / frames);
            mSpeed.y = std::fabs((mTarget.y - mPos.y) / frames);
            mSpeed.z = std::fabs((mTarget.z - mPos.z) / frames);
        } else {
            mScaleSpeed = l_appearGroundScaleSpeed[mStep];
        }
    }
    mFrame++;
}

// 80193B70
void dHandItem_c::getDropPos(mVec3_c *out) {
    if (dScene::getCurrentScene() == dScene::SCENE_RM_FORTUNE) {
        mVec3_c pos(256.0f, 0.0f, 368.0f);
        *out = pos;
    } else {
        mVec3_c local(0.0f, 0.0f, 32.0f);
        mVec3_c pos;
        toWorld(&pos, &local, 0);
        dFdBase_c::snapToUnitCenter(*out, pos);
    }
    out->y += 16.0f;
}

// 80193C20
void dHandItem_c::appearGroundDrop() {
    sLib::chase(&mScale, l_appearGroundScale[mStep], mScaleSpeed);
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= l_appearGroundEnd[mStep]) {
        mStep = 8;
        mDone = TRUE;
    }
    mFrame++;
}

// 80193CCC
void dHandItem_c::appearGroundWaitNpc() {
    if (mFrame >= l_appearGroundEnd[mStep]) {
        getHandPos(&mPos, 0);
        mTarget = l_appearGroundPopOfs;
        dDemoActor_c *actor = getActor(0);
        if (actor != NULL) {
            mTarget.rotY(actor->getAngle().y);
        }
        mStep = 7;
        f32 frames = l_appearGroundDur[mStep];
        mOffset = mVec3_c::Zero;
        mSpeed.x = std::fabs(mTarget.x / frames);
        mSpeed.y = std::fabs(mTarget.y / frames);
        mSpeed.z = std::fabs(mTarget.z / frames);
        mScaleSpeed = l_appearGroundScaleSpeed[mStep];
    }
    mFrame++;
}

// 80193E00
void dHandItem_c::appearGroundPop() {
    f32 scale = l_appearGroundScale[mStep];
    if (mScale < 1.0f) {
        sLib::chase(&mScale, scale, mScaleSpeed);
    }
    if (mScale > 1.0f) {
        mScale = 1.0f;
    }
    if (mFrame == 67) {
        mSound.startSound(SE_toridashi);
    }
    getHandPos(&mPos, 0);
    sLib::chase(&mOffset.x, mTarget.x, mSpeed.x);
    sLib::chase(&mOffset.y, mTarget.y, mSpeed.y);
    sLib::chase(&mOffset.z, mTarget.z, mSpeed.z);
    mPos += mOffset;
    if (mFrame >= l_appearGroundEnd[mStep]) {
        mDone = TRUE;
        mStep = 8;
    }
    mFrame++;
}

// 80193F28
void dHandItem_c::execAppearGround() {
    static const StateFunc steps[] = {
        &dHandItem_c::appearGroundWait, &dHandItem_c::appearGroundRise, &dHandItem_c::appearGroundRise,
        &dHandItem_c::appearGroundRise, &dHandItem_c::appearGroundRise, &dHandItem_c::appearGroundDrop,
        &dHandItem_c::appearGroundWaitNpc, &dHandItem_c::appearGroundPop,
    };
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80193F68
void dHandItem_c::initFallGround() {
    getDropPos(&mTarget);
    mDone = FALSE;
    mSpeed.x = std::fabs((mTarget.x - mPos.x) / 12.0f);
    mSpeed.y = std::fabs((mTarget.y - mPos.y) / 12.0f);
    mSpeed.z = std::fabs((mTarget.z - mPos.z) / 12.0f);
}

// 80193FF8
void dHandItem_c::fallGroundMove() {
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= 12) {
        mStep = 1;
        mDone = TRUE;
    }
    mFrame++;
}

// 80194074
void dHandItem_c::execFallGround() {
    static const StateFunc steps[] = {&dHandItem_c::fallGroundMove};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 801940B4
void dHandItem_c::initTakeGround() {
    getHandPos(&mTarget, 0);
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mVec3_c ofs = l_groundHandOfs;
        ofs.rotY(actor->getAngle().y);
        mTarget += ofs;
    }
    mDone = FALSE;
    mSpeed.x = std::fabs((mTarget.x - mPos.x) / 10.0f);
    mSpeed.y = std::fabs((mTarget.y - mPos.y) / 10.0f);
    mSpeed.z = std::fabs((mTarget.z - mPos.z) / 10.0f);
}

// 801941C0
void dHandItem_c::takeGroundMove() {
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= 10) {
        mDone = TRUE;
        mStep = 1;
    }
    mFrame++;
}

// 8019423C
void dHandItem_c::execTakeGround() {
    static const StateFunc steps[] = {&dHandItem_c::takeGroundMove};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 8019427C
void dHandItem_c::initHoldGround() {
    mDone = FALSE;
}

// 80194288
void dHandItem_c::holdGroundTrack() {
    getHandPos(&mPos, 0);
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mVec3_c ofs = l_groundHandOfs;
        ofs.rotY(actor->getAngle().y);
        mPos += ofs;
    }
    if (++mFrame >= 14) {
        mDone = TRUE;
        mStep = 1;
    }
}

// 80194358
void dHandItem_c::execHoldGround() {
    static const StateFunc steps[] = {&dHandItem_c::holdGroundTrack};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 80194398
void dHandItem_c::initShrinkGround() {
    mVec3_c hand;
    getHandPos(&hand, 0);
    mOffset = mPos - hand;
    mTarget = l_shrinkGroundTarget;
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mTarget.rotY(actor->getAngle().y);
    }
    mDone = FALSE;
    mSpeed.x = std::fabs((mTarget.x - mOffset.x) / 18.0f);
    mSpeed.y = std::fabs((mTarget.y - mOffset.y) / 18.0f);
    mSpeed.z = std::fabs((mTarget.z - mOffset.z) / 18.0f);
    mScaleSpeed = std::fabs(mScale / 18.0f);
}

// 801944B0
void dHandItem_c::shrinkGroundAway() {
    sLib::chase(&mScale, 0.0f, mScaleSpeed);
    sLib::chase(&mOffset.x, mTarget.x, mSpeed.x);
    sLib::chase(&mOffset.y, mTarget.y, mSpeed.y);
    sLib::chase(&mOffset.z, mTarget.z, mSpeed.z);
    getHandPos(&mPos, 0);
    mPos += mOffset;
    if (mFrame >= 18) {
        mStep = 1;
        mDone = TRUE;
    }
    mFrame++;
}

// 8019457C
void dHandItem_c::execShrinkGround() {
    static const StateFunc steps[] = {&dHandItem_c::shrinkGroundAway};
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

// 801945BC
void dHandItem_c::initFollowGround() {
    mDone = FALSE;
}

// 801945C8
void dHandItem_c::execFollowGround() {
    execHoldHand();
}

// 801945CC
void dHandItem_c::initStowGround() {
    mDone = FALSE;
}

// 801945D8
void dHandItem_c::stowGroundWait() {
    if (mFrame >= l_stowGroundEnd[mStep]) {
        getHandPos(&mTarget, 0);
        dDemoActor_c *actor = getActor(0);
        if (actor != NULL) {
            mVec3_c ofs = l_stowGroundOfs;
            ofs.rotY(actor->getAngle().y);
            mTarget += ofs;
        }
        mSpeed.set(std::fabs(mTarget.x - mPos.x), std::fabs(mTarget.y - mPos.y), std::fabs(mTarget.z - mPos.z));
        mSpeed /= l_stowGroundDur[mStep];
        mStep = 1;
    }
    mFrame++;
}

// 80194730
void dHandItem_c::stowGroundToHand() {
    getHandPos(&mTarget, 0);
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mVec3_c ofs = l_stowGroundOfs;
        ofs.rotY(actor->getAngle().y);
        mTarget += ofs;
    }
    sLib::chase(&mPos.x, mTarget.x, mSpeed.x);
    sLib::chase(&mPos.y, mTarget.y, mSpeed.y);
    sLib::chase(&mPos.z, mTarget.z, mSpeed.z);
    if (mFrame >= l_stowGroundEnd[mStep]) {
        mStep = 2;
        mOffset = l_stowGroundOfs;
        mSpeed.set(std::fabs(mOffset.x), std::fabs(mOffset.y), std::fabs(mOffset.z));
        mSpeed /= l_stowGroundDur[2];
        mScaleSpeed = mScale / l_stowGroundDur[mStep];
    }
    mFrame++;
}

// 801948D0
void dHandItem_c::stowGroundShrink() {
    sLib::chase(&mScale, 0.0f, mScaleSpeed);
    getHandPos(&mPos, 0);
    sLib::chase(&mOffset.x, 0.0f, mSpeed.x);
    sLib::chase(&mOffset.y, 0.0f, mSpeed.y);
    sLib::chase(&mOffset.z, 0.0f, mSpeed.z);
    dDemoActor_c *actor = getActor(0);
    if (actor != NULL) {
        mVec3_c ofs = mOffset;
        ofs.rotY(actor->getAngle().y);
        mPos += ofs;
    }
    if (mFrame >= l_stowGroundEnd[mStep]) {
        mStep = 3;
        mDone = TRUE;
    }
    mFrame++;
}

// 801949EC
void dHandItem_c::execStowGround() {
    static const StateFunc steps[] = {
        &dHandItem_c::stowGroundWait, &dHandItem_c::stowGroundToHand, &dHandItem_c::stowGroundShrink,
    };
    if (mStep < ARRAY_SIZE(steps)) {
        (this->*steps[mStep])();
    }
}

namespace dHandItem {

// 80194A2C
BOOL start(const dItem::Item *item, int kind, int useType, int variant, dDemoActor_c *actor, dDemoActor_c *partner,
           f32 ofsY0, f32 ofsY1, f32 fishOfsY0, f32 fishOfsY1, f32 ofsZ0, f32 ofsZ1) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->start(item, kind, useType, variant, actor, partner, ofsY0, ofsY1,
                                             fishOfsY0, fishOfsY1, ofsZ0, ofsZ1);
    }
    return FALSE;
}

// 80194AA0
BOOL request(int state, dDemoActor_c *actor, f32 reach) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->request(state, actor, reach);
    }
    return FALSE;
}

// 80194AC8
BOOL isHolder(dDemoActor_c *actor) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->isActor(actor, 0);
    }
    return FALSE;
}

// 80194AEC
BOOL isActive() {
    BOOL active = FALSE;
    if (dHandItem_c::sInstance != NULL && (u32)dHandItem_c::sInstance->mState < dHandItem_c::STATE_NUM) {
        active = TRUE;
    }
    return active;
}

// 80194B10
BOOL isState(int state) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->isState(state);
    }
    return FALSE;
}

// 80194B30
int getUseType() {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->mUseType;
    }
    return dHandItem_c::STATE_NONE;
}

// 80194B4C
BOOL setUseType(int type, dDemoActor_c *actor) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->setUseType(type, actor);
    }
    return FALSE;
}

// 80194B74
BOOL isReceiving() {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->isReceiving();
    }
    return FALSE;
}

// 80194B8C
BOOL swapActors(dDemoActor_c *actor) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->swapActors(actor);
    }
    return FALSE;
}

// 80194BAC
dDemoActor_c *getActor(u32 idx) {
    if (dHandItem_c::sInstance != NULL) {
        return dHandItem_c::sInstance->getActor(idx);
    }
    return NULL;
}

// 80194BCC
BOOL isInRange(dDemoActor_c *actor) {
    if (dHandItem_c::sInstance != NULL && actor != NULL) {
        const mVec3_c *pos = actor->getPosP();
        dDemoActor_c *other;
        if (isHolder(actor)) {
            other = getActor(1);
        } else {
            other = getActor(0);
        }
        const mVec3_c *otherPos = other != NULL ? other->getPosP() : &dHandItem_c::sInstance->mPos;
        if (otherPos->xzDistTo(*pos) <= dHandItem_c::sInstance->mRange) {
            return TRUE;
        }
        if (dHandItem_c::sInstance->isWaiting() && dHandItem_c::sInstance->mTimer == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80194CB0
void release(dDemoActor_c *actor) {
    if (dHandItem_c::sInstance != NULL && isHolder(actor)) {
        dHandItem_c::sInstance->release();
    }
}

// 80194CEC
BOOL getPos(mVec3_c *pos) {
    if (dHandItem_c::sInstance != NULL) {
        *pos = dHandItem_c::sInstance->mPos;
        return TRUE;
    }
    return FALSE;
}

// 80194D20
void getItem(dItem::Item *item) {
    if (dHandItem_c::sInstance != NULL) {
        *item = dHandItem_c::sInstance->mItem;
    } else {
        *item = dItem::ITEM_ID_NONE;
    }
}

} // namespace dHandItem
