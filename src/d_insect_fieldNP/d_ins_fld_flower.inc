// dInsFldFlower_c: the insects on flowers. See
// include/game/game/d_ins_fld_flower.hpp.

const dInsFldFlower_c::StateFunc dInsFldFlower_c::sExecute[] = {
    &dInsFldFlower_c::executeWait,   &dInsFldFlower_c::executeWalk,
    &dInsFldFlower_c::executeTurn,   &dInsFldFlower_c::executeLook,
    &dInsFldFlower_c::executeUnlook, &dInsFldFlower_c::executeEscape,
    &dInsFldFlower_c::executeLeave,
};

const dInsFldFlower_c::StateFunc dInsFldFlower_c::sInit[] = {
    &dInsFldFlower_c::initWait,   &dInsFldFlower_c::initWalk,
    &dInsFldFlower_c::initTurn,   &dInsFldFlower_c::initLook,
    &dInsFldFlower_c::initUnlook, &dInsFldFlower_c::initEscape,
    &dInsFldFlower_c::onRelease,
};

const int dInsFldFlower_c::sSounds[] = {
    SE_mushi_fly_out_midium,
    SE_mushi_fly_out_small,
    SE_mushi_out_fall,
    SE_mushi_fly_out_small,
};

// .data CE8
f32 dInsFldFlower_c::sTurnAnmRate = 2.0f;

// .data CF0
dInsFldFlower_c::param_c dInsFldFlower_c::sParam[TYPE_NUM] = {
    {7.0f,  17.0f, 1.0f, 3.0f, 1.0f,    3.0f, 1.0f, 2.5f,
     2.5f,  0.01f, 0.3f, 0.5f, -0.007f, 5.0f, 3.0f, 0.6f,
     0.03f, 1.0f,  4.0f, 5.0f, 80.0f,   20,   400},
    {7.0f,  17.0f, 1.0f, 3.0f, 1.0f,    3.0f, 1.0f, 2.5f,
     2.5f,  0.01f, 0.3f, 0.5f, -0.007f, 5.0f, 3.0f, 0.6f,
     0.03f, 1.0f,  3.0f, 4.0f, 80.0f,   20,   300},
    {7.0f, 17.0f, 1.0f, 3.0f, 1.0f,   3.0f, 1.0f, 2.5f,
     2.5f, 0.01f, 0.3f, 0.5f, -0.07f, 5.0f, 0.0f, 0.3f,
     0.0f, 1.0f,  4.0f, 5.0f, 40.0f,  100,  400},
    {7.0f,  17.0f, 1.0f, 3.0f, 1.0f,    3.0f, 1.0f, 2.5f,
     2.5f,  0.01f, 0.3f, 0.5f, -0.007f, 5.0f, 0.0f, 0.4f,
     0.02f, 1.0f,  4.5f, 3.0f, 40.0f,   400,  0},
};

// .text E398
const dInsFldFlower_c::param_c *dInsFldFlower_c::getKindParam(int type) {
    const int types[] = {INSECT_MANTIS, INSECT_ORCHID_MANTIS, INSECT_SNAIL,
                         INSECT_LADYBUG};
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return &sParam[i];
        }
    }
    return &sParam[0];
}

// .text E40C
int dInsFldFlower_c::getEscapeSound(int type) {
    const int types[] = {INSECT_MANTIS, INSECT_ORCHID_MANTIS, INSECT_SNAIL,
                         INSECT_LADYBUG};
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return sSounds[i];
        }
    }
    return SE_mushi_fly_out_midium;
}

// .text E47C
bool dInsFldFlower_c::isSittingFlower(const dItem::Item *item,
                                      const mVec3_c *pos) {
    u16 id = item->mId;
    if (item->isFlower() == true || id == dItem::FG_DANDELION) {
        return true;
    }
    return false;
}

// .text E4C4
dInsFldFlower_c::dInsFldFlower_c()
    : mPrevState(STATE_WAIT), mState(STATE_WAIT), mFlowerPos(0.0f, 0.0f, 0.0f),
      mWaitTime(0), mWalkTime(0), mWalkTotal(0), _332(150), mWalkTimer(0),
      mTargetAngle(0), mBounce(0) {
}

// .text E540
dInsFldFlower_c::~dInsFldFlower_c() {
}

// .text E598
BOOL dInsFldFlower_c::create(const nw4r::g3d::ResFile &res) {
    dInsFldBase_c::create(res);
    dFdBase_c::snapToUnitCenter(mFlowerPos, mInitPos);
    return TRUE;
}

// .text E5D4
int dInsFldFlower_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text E5F8
void dInsFldFlower_c::onBecomeOwner() {
}

// .text E5FC
void dInsFldFlower_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setReq(mState);
        }
    }
}

// .text E67C
void dInsFldFlower_c::setGroundPos() {
    mInitPos.y = getGroundY();
    dInsectUtil::itemSearch_c search(isSittingFlower);
    search.search(mInitPos, 1.0f);
    mVec3_c ofs;
    dItem::Item item = search.getItem();
    dInsectField::getItemOffset(&ofs, &item);
    mPos.x = mInitPos.x + ofs.x;
    mPos.y = ofs.y + getGroundY();
    mPos.z = mInitPos.z + ofs.z;
}

// .text E728
void dInsFldFlower_c::initState() {
    setState(STATE_WAIT);
}

// .text E730
void dInsFldFlower_c::executeState() {
    const param_c *param = getKindParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    checkState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
    if (mState == STATE_ESCAPE || mState == STATE_LEAVE) {
        mShadowSize = param->mShadowSize;
    } else {
        mShadowSize = 0.0f;
    }
}

// .text E7D8
void dInsFldFlower_c::initWait() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    if (isSnail() == true) {
        setAnmChr(mAnmChrWait, 300.0f);
        setAnmVis(mAnmVisWait);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
}

// .text E8CC
void dInsFldFlower_c::initWalk() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWalkTimeMax);
    int min = dInsectUtil::secToFrame(param->mWalkTimeMin);
    mWalkTotal = cM::rndRange<u16>(min, max);
    startWalk();
}

// .text E990
void dInsFldFlower_c::initTurn() {
    getKindParam(mType);
    mFrame = 0;
    if (isSnail() == true) {
        setAnmChr(mAnmChrStop, 0.0f);
        setAnmVis(mAnmVisStop);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(sTurnAnmRate);
    setAnmFrame(0.0f);
    mTargetAngle = mPlayerCheck.getAngleTo(mPlayerCheck.getNearest());
}

// .text EA58
void dInsFldFlower_c::initLook() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
}

// .text EAD4
void dInsFldFlower_c::initUnlook() {
    getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(-1.0f);
    setAnmFrame(frame);
}

// .text EB64
void dInsFldFlower_c::initEscape() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mAngle.y = getEscapeAngle();
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    if (isSnail() == true) {
        mVelocity.set(0.0f, 0.0f, param->mEscapeSpeed);
        mVelocity.rotY(mAngle.y);
    }
    int sound = getEscapeSound(mType);
    if (sound > 0) {
        mSound.startSound(sound);
    }
}

// .text EC64
void dInsFldFlower_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
    if (isSnail() == true) {
        mVelocity.set(0.0f, 0.0f, param->mEscapeSpeed);
        mVelocity.rotY(mAngle.y);
    }
    int sound = getEscapeSound(mType);
    if (sound > 0) {
        mSound.startSound(sound);
    }
}

// .text ED44
void dInsFldFlower_c::executeWait() {
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text ED54
void dInsFldFlower_c::executeWalk() {
    const param_c *param = getKindParam(mType);
    dInsectUtil::moveTo(&mPos, &mAngle.y, mHomePos, param->mWalkSpeed,
                        param->mWalkTurn);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mWalkTime <= mWalkTimer) {
        startWalk();
    }
}

// .text EDCC
void dInsFldFlower_c::executeTurn() {
    registObjc(&mPos, mBaseRadius, TRUE);
    if (!isSnail()) {
        const param_c *param = getKindParam(mType);
        sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1,
                           mAng::fromDegree(param->mTurnSpeed), 0);
    }
}

// .text EE50
void dInsFldFlower_c::executeLook() {
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text EE60
void dInsFldFlower_c::executeUnlook() {
    setAnmRate(-1.0f);
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text EEA8
void dInsFldFlower_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    if (!isSnail()) {
        moveForward(param->mEscapeSpeed, param->mEscapeAccelY);
    } else {
        fall();
    }
}

// .text EF10
void dInsFldFlower_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    if (!isSnail()) {
        moveForward(param->mEscapeSpeed, param->mEscapeAccelY);
    } else {
        fall();
    }
}

// .text EF78
mAng dInsFldFlower_c::getEscapeAngle() {
    s16 player = mPlayerCheck.getAngleTo(mPlayerCheck.getNearest()).mAngle +
                 INSECT_FIELD_FLEE_ANGLE;
    s16 npc = player;
    if (mNpcCheck.isNear(mScareRadius) == true) {
        npc = mNpcCheck.getAngleTo().mAngle + INSECT_FIELD_FLEE_ANGLE;
    }
    return (player + npc) >> 1;
}

// .text F010
bool dInsFldFlower_c::isSnail() {
    return mType == INSECT_SNAIL;
}

// .text F024
bool dInsFldFlower_c::isNotLadybug() {
    return mType != INSECT_LADYBUG;
}

// .text F03C
void dInsFldFlower_c::fall() {
    const param_c *param = getKindParam(mType);
    mVelocity.y += param->mGravity;
    mPos += mVelocity;
    fadeOut();
    mVelocity.x *= 0.95f;
    mVelocity.z *= 0.95f;
    if (isUnderGround() == true) {
        if (mBounce == 0) {
            mPos.y = getGroundY();
            mVelocity.y = 0.6f;
        } else if (mBounce == 1) {
            mPos.y = getGroundY();
            mVelocity.y = 0.3f;
        } else {
            mPos.y = getGroundY();
            mVelocity.y = 0.0f;
        }
        mBounce++;
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = FALSE;
        requestDelete();
    }
    if (mAlpha == 0) {
        requestDelete();
    }
}

// .text F1EC
mAng dInsFldFlower_c::getWalkAngle() const {
    s16 angle = mAngle.y.mAngle;
    int range = DEG_TO_ANGLE(120);
    return angle + (int)cM::rndCenterF(range);
}

// .text F250
f32 dInsFldFlower_c::getWalkDist() const {
    const param_c *param = getKindParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// .text F280
void dInsFldFlower_c::getWalkTarget(mVec3_c *out) {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
    }
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c target(0.0f, 0.0f, 0.0f);
    mVec3_c ofs(0.0f, 0.0f, dist);
    ofs.rotY(angle);
    ofs += mPos;
    ofs.set(0.0f, 0.0f, 0.0f);
    getFlowerTarget(&ofs);
    target = ofs;
    *out = target;
}

// .text F368
void dInsFldFlower_c::getFlowerTarget(mVec3_c *out) {
    mVec3_c max;
    mVec3_c min;
    max.x = mFlowerPos.x + 1.6f;
    max.z = mFlowerPos.z + 1.6f;
    min.x = mFlowerPos.x - 1.6f;
    min.z = mFlowerPos.z - 1.6f;
    out->x = cM::rndRange(min.x, max.x);
    out->z = cM::rndRange(min.z, max.z);
}

// .text F3D8
bool dInsFldFlower_c::isPlayerNear() {
    bool player = mPlayerCheck.isNear(mPlayerCheck.getNearest(), 16.0f);
    bool npc = mNpcCheck.isNear(16.0f);
    if (player == true || npc == true) {
        return true;
    }
    return false;
}

// .text F450
bool dInsFldFlower_c::isOnFlower() {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, mInitPos);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return false;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return false;
    }
    u16 id = item->mId;
    if (item->isFlower() == true || id == dItem::FG_DANDELION) {
        return true;
    }
    return false;
}

// .text F50C
void dInsFldFlower_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            u32 state = data->getReq();
            if (state < STATE_NUM_MAX) {
                mState = state;
            }
        }
        return;
    }

    if (mState == STATE_WAIT) {
        if (mWaitTime <= mFrame) {
            mState = STATE_WALK;
        }
        if (mScare.isScared() == true) {
            if (isNotLadybug() == true) {
                mState = STATE_TURN;
            } else {
                mState = STATE_ESCAPE;
            }
        }
    } else if (mState == STATE_WALK) {
        if (mWalkTotal <= mFrame) {
            mState = STATE_WAIT;
        }
        if (mScare.isScared() == true) {
            if (isNotLadybug() == true) {
                mState = STATE_TURN;
            } else {
                mState = STATE_ESCAPE;
            }
        }
    } else if (mState == STATE_TURN) {
        if (!isSnail()) {
            if (mAngle.y.mAngle == mTargetAngle) {
                mState = STATE_LOOK;
            }
        } else if (!mScare.isScared()) {
            mState = STATE_UNLOOK;
        }
        if (mScare.isEscaping() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_LOOK) {
        if ((u16)dInsectUtil::secToFrame(3.0f) <= mFrame) {
            if ((u16)dInsectUtil::secToFrame(getKindParam(mType)->mLookTime) <=
                mFrame) {
                s16 diff =
                    mPlayerCheck.getAngleTo(mPlayerCheck.getNearest()).mAngle -
                    mAngle.y.mAngle;
                if ((s16)nw4r::math::FAbs(diff) > DEG_TO_ANGLE(30)) {
                    mState = STATE_UNLOOK;
                }
            }
            if (!mScare.isScared()) {
                mState = STATE_UNLOOK;
            }
        }
        if (mScare.isEscaping() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_UNLOOK) {
        if (mAnmChr.getFrame() <= 1.0f) {
            if (mScare.isScared() == true) {
                mState = STATE_TURN;
            } else {
                mState = STATE_WAIT;
            }
        }
    }

    if (mState != STATE_ESCAPE && mState != STATE_LEAVE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.setEscapeMax();
    }

    if (mState != STATE_ESCAPE && mState != STATE_LEAVE) {
        if (isPlayerNear() == true) {
            mState = STATE_ESCAPE;
        }
        if (!isOnFlower()) {
            mState = STATE_ESCAPE;
        }
        if (mFrame % 15 == 0 &&
            dFgobjManager::isSwingPlantAt(mPos.x, mPos.z) == TRUE) {
            mState = STATE_ESCAPE;
        }
    }
}

// .text F8F0
void dInsFldFlower_c::startWalk() {
    const param_c *param = getKindParam(mType);
    mWalkTimer = 0;
    mVelocity.set(0.0f, 0.0f, param->mWalkSpeed);
    mVelocity.rotY(mAngle.y);
    getWalkTarget(&mHomePos);
    mWalkTime = (int)((mHomePos - mPos).xzLen() / param->mWalkSpeed) + 1;
}
