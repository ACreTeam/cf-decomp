// dInsFldLeaf_c: the walking leaf. See include/game/game/d_ins_fld_leaf.hpp.

const dInsFldLeaf_c::StateFunc dInsFldLeaf_c::sExecute[] = {
    &dInsFldLeaf_c::executeHide,
    &dInsFldLeaf_c::executeReveal,
    &dInsFldLeaf_c::executeWalk,
    &dInsFldLeaf_c::executeRelease,
};

const dInsFldLeaf_c::StateFunc dInsFldLeaf_c::sInit[] = {
    &dInsFldLeaf_c::initHide,
    &dInsFldLeaf_c::initReveal,
    &dInsFldLeaf_c::initWalk,
    &dInsFldLeaf_c::onRelease,
};

const int dInsFldLeaf_c::sTypes[] = {INSECT_WALKING_LEAF};

dInsFldLeaf_c::param_c dInsFldLeaf_c::sParam[] = {
    {0.0f, 0.0f, 0.0f, 150.0f, 100.0f, 0.8f, 0.05f, 0.9f, 0.07f, 2.2f, 60.0f,
     -0.15f, 7.0f, 7.0f, 50.0f, 5},
};

// 0x25298
int dInsFldLeaf_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x252B8
const dInsFldLeaf_c::param_c *dInsFldLeaf_c::getLeafParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x252E8
dInsFldLeaf_c::dInsFldLeaf_c()
    : mState(STATE_HIDE), mPrevState(STATE_HIDE), mWalkFrames(0), mLifeTimer(0),
      mLanded(false) {
}

// 0x2533C
dInsFldLeaf_c::~dInsFldLeaf_c() {
}

// 0x25394
BOOL dInsFldLeaf_c::doDelete() {
    dInsFldBase_c::doDelete();
    return TRUE;
}

// 0x253B8
int dInsFldLeaf_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x253DC
void dInsFldLeaf_c::calcMtx() {
    setMtxFromAxis();
}

// 0x253E0
void dInsFldLeaf_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x25418
void dInsFldLeaf_c::setGroundPos() {
    const param_c *param = getLeafParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
}

// 0x25498
void dInsFldLeaf_c::initState() {
    setState(STATE_HIDE);
}

// 0x254A0
void dInsFldLeaf_c::executeState() {
    getLeafParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    if (mState == STATE_HIDE) {
        mShadowSize = 0.0f;
    }
    if (mState != STATE_WALK && mState != STATE_RELEASE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.mValue = (int)(3.0f * (int)mScare.mLimit);
    }
}

// 0x25594
void dInsFldLeaf_c::initHide() {
    getLeafParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
}

// 0x2560C
void dInsFldLeaf_c::initReveal() {
    getLeafParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mAxis.setIdentity();
}

// 0x25690
void dInsFldLeaf_c::initWalk() {
    const param_c *param = getLeafParam(mType);
    if (mPrevState != STATE_WALK) {
        mLifeTimer = 0;
    }
    mState = STATE_WALK;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(0.0f);
    getWalkTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mWalkFrames = (int)(dist / param->mWalkSpeed) + 1;
}

// 0x25780
void dInsFldLeaf_c::onRelease() {
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(3.0f);
    setAnmFrame(0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x25814
void dInsFldLeaf_c::executeHide() {
    getLeafParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mScare.isScared() == true || isItemPutHere() == true) {
        setState(STATE_REVEAL);
    }
}

// 0x25884
void dInsFldLeaf_c::executeReveal() {
    getLeafParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    if ((u16)mAnmChr.getFrame() >= REVEAL_END_FRAME) {
        setState(STATE_WALK);
    }
}

// 0x258F4
void dInsFldLeaf_c::executeWalk() {
    const param_c *param = getLeafParam(mType);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mWalkFrames <= mFrame) {
        setState(STATE_WALK);
    }
    if ((u16)dInsectUtil::secToFrame(param->mLifeTime) <= mLifeTimer) {
        int alpha = mAlpha - FADE_SPEED;
        mAlpha = alpha < 0 ? 0 : alpha;
        if (mAlpha == 0) {
            requestDelete();
        }
    }
    mLifeTimer++;
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
}

// 0x25A70
void dInsFldLeaf_c::executeRelease() {
    mHomePos = mPos + mAxis.getFront() * 100.0f;
    const param_c *param = getLeafParam(mType);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
        if (!mLanded) {
            mSound.startSound(SE_mushi_out_run);
            mLanded = true;
        }
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
}

// 0x25BF0
mAng dInsFldLeaf_c::getWalkAngle() {
    int player = mPlayerCheck.getNearest();
    return mPlayerCheck.getAngleTo(player).mAngle + INSECT_FIELD_FLEE_ANGLE;
}

// 0x25C3C
f32 dInsFldLeaf_c::getWalkDist() const {
    const param_c *param = getLeafParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x25C6C
void dInsFldLeaf_c::getWalkTarget(mVec3_c *out) {
    mAng ang = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    const param_c *param = getLeafParam(mType);
    u8 steps = (int)(param->mWalkDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = target;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, TRUE, TRUE,
                                       -30.0f, 5.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 0, TRUE);
    }
    *out = pos;
}

// 0x25DD0
bool dInsFldLeaf_c::isItemPutHere() {
    if (dInsectInfo::hasPos() == true) {
        mVec3_c *pos = dInsectInfo::getPos();
        int unitX = worldToUnit(pos->x);
        int unitZ = worldToUnit(pos->z);
        int myUnitX = worldToUnit(mPos.x);
        int myUnitZ = worldToUnit(mPos.z);
        if (myUnitX == unitX && myUnitZ == unitZ) {
            return true;
        }
    }
    return false;
}

// 0x25E74
void dInsFldLeaf_c::walk(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    mVec3_c up = chk.mFloor.mNormal;
    up.normalize();
    mAxis.setUp(up);
}
