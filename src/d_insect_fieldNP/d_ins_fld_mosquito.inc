// dInsFldMosquito_c: the mosquito. See
// include/game/game/d_ins_fld_mosquito.hpp.

const dInsFldMosquito_c::StateFunc dInsFldMosquito_c::sExecute[] = {
    &dInsFldMosquito_c::executeWander,
    &dInsFldMosquito_c::executeChase,
    &dInsFldMosquito_c::executeCircle,
    &dInsFldMosquito_c::executeRelease,
};

const dInsFldMosquito_c::StateFunc dInsFldMosquito_c::sInit[] = {
    &dInsFldMosquito_c::initWander,
    &dInsFldMosquito_c::initChase,
    &dInsFldMosquito_c::initCircle,
    &dInsFldMosquito_c::onRelease,
};

const int dInsFldMosquito_c::sTypes[] = {INSECT_MOSQUITO};

dInsFldMosquito_c::param_c dInsFldMosquito_c::sParam[] = {
    {0.0f, 25.0f, 0.0f,  30.0f, 20.0f, 0.1f,  1.0f, 3.0f,  2.0f, 2.0f,
     1.0f, 30.0f, 14.0f, 0.25f, 1.0f,  30.0f, 0.4f, 0.03f, 0.0f, 2.0f},
};

// 0x20340
int dInsFldMosquito_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x20360
const dInsFldMosquito_c::param_c *
dInsFldMosquito_c::getMosquitoParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x20390
dInsFldMosquito_c::dInsFldMosquito_c()
    : mState(STATE_WANDER), mBaseY(0.0f), mBobPhase(0), mBobHeight(0.0f),
      mWanderFrames(0), mChaseFrames(0), mBiteTimer(0), mCircleAngle(0),
      mCenter(0.0f, 0.0f, 0.0f), mRadius(0.0f), mWingPhase(0), _342(0),
      mTurnDir(false) {
}

// 0x20414
dInsFldMosquito_c::~dInsFldMosquito_c() {
}

// 0x2046C
int dInsFldMosquito_c::draw() {
    getMosquitoParam(mType);
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x204A8
void dInsFldMosquito_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x204E0
void dInsFldMosquito_c::setGroundPos() {
    const param_c *param = getMosquitoParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOffsetZ;
    mBaseY = mPos.y;
}

// 0x20554
void dInsFldMosquito_c::initState() {
    setState(STATE_WANDER);
}

// 0x2055C
void dInsFldMosquito_c::executeState() {
    getMosquitoParam(mType);
    mBgCheck = true;
    (this->*sExecute[mState])();
}

// 0x205B4
void dInsFldMosquito_c::initWander() {
    const param_c *param = getMosquitoParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    getTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mWanderFrames = (int)(dist / param->mFlySpeed);
}

// 0x20688
void dInsFldMosquito_c::initChase() {
    const param_c *param = getMosquitoParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, param->mFlySpeed);
    getTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mChaseFrames = (int)(dist / param->mFlySpeed);
}

// 0x20770
void dInsFldMosquito_c::initCircle() {
    getMosquitoParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mCircleAngle =
        mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle -
        DEG_TO_ANGLE(180);
    mPlayerCheck.getPlayerPos(&mCenter, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c diff = mCenter - mPos;
    mRadius = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mBiteTimer = 0;
    mTurnDir = getTurnDir();
}

// 0x2085C
void dInsFldMosquito_c::onRelease() {
    getMosquitoParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mSound.startSound(SE_mushi_fly_out_small);
    mShadowSize = 1.0f;
}

// 0x208FC
void dInsFldMosquito_c::executeWander() {
    const param_c *param = getMosquitoParam(mType);
    mSound.holdSound(SE_mosqueto);
    dInsectUtil::moveTo(&mPos, &mAngle.y, mHomePos, param->mFlySpeed,
                        param->mFlyTurn);
    bob();
    mWingPhase += mAng::fromDegree(param->mWingSpeed);
    mAngle.x =
        nw4r::math::SinIdx(mWingPhase) * mAng::fromDegree(param->mWingAmp);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mWanderFrames <= mFrame) {
        if (isPlayerNear() == true) {
            setState(STATE_CHASE);
        } else {
            setState(STATE_WANDER);
        }
    }
}

// 0x20A4C
void dInsFldMosquito_c::executeChase() {
    const param_c *param = getMosquitoParam(mType);
    mSound.holdSound(SE_mosqueto);
    if (mPlayerCheck.isValid(dPlayerMgr::PLAYER_INDEX_CURRENT) == true &&
        !dPlayerMgr::isSafeFromInsects()) {
        mVec3_c old = mPos;
        dInsectUtil::moveTo(&mPos, &mAngle.y, mHomePos, param->mFlySpeed,
                            param->mFlyTurn);
        mVelocity = mPos - old;
        mVelocity.y = 0.0f;
        mWingPhase += mAng::fromDegree(param->mWingSpeed);
        mAngle.x =
            nw4r::math::SinIdx(mWingPhase) * mAng::fromDegree(param->mWingAmp);
    }
    bob();
    registObjc(&mPos, mBaseRadius, TRUE);
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    bool above = player.y < mPos.y;
    bool close = mPos.y - player.y < 40.0f;
    if (isBiteNear() == true && above == true && close == true) {
        setState(STATE_CIRCLE);
    } else if (!isPlayerNear()) {
        setState(STATE_WANDER);
    } else if (mChaseFrames <= mFrame) {
        setState(STATE_CHASE);
    }
}

// 0x20C74
void dInsFldMosquito_c::executeCircle() {
    const param_c *param = getMosquitoParam(mType);
    mBgCheck = false;
    mSound.holdSound(SE_mosqueto);
    if (!dPlayerMgr::isSafeFromInsects()) {
        if (mTurnDir == true) {
            mCircleAngle += mAng::fromDegree(param->mCircleSpeed);
        } else {
            mCircleAngle -= mAng::fromDegree(param->mCircleSpeed);
        }
        mWingPhase += mAng::fromDegree(param->mCircleWingSpeed);
        mAngle.x = nw4r::math::SinIdx(mWingPhase) *
                   mAng::fromDegree(param->mCircleWingAmp);
    }
    mVec3_c ofs(0.0f, 0.0f, mRadius);
    ofs.rotY(mCircleAngle);
    mVec3_c old = mPos;
    f32 y = mPos.y;
    mPos = mCenter + ofs;
    mPos.y = y;
    mAngle.y = cLib::targetAngleY(old, mPos);
    bob();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (!isBiteNear()) {
        mAcch.checkSteps(mBgRadius, &mPos, &mCenter, mAngle.y, 0,
                         dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                             dBGCF::CHECK_SET_POS);
        setState(STATE_CHASE);
    }
    if (!dPlayerMgr::isSafeFromInsects()) {
        if (mBiteTimer >= BITE_WAIT_TIME &&
            dPlayerMgr::requestInsectBite() == TRUE) {
            setState(STATE_RELEASE);
        }
        mBiteTimer++;
    }
}

// 0x20EF8
void dInsFldMosquito_c::executeRelease() {
    const param_c *param = getMosquitoParam(mType);
    f32 speed = param->mReleaseSpeed;
    f32 accel = param->mReleaseAccelY;
    f32 speedY = mVelocity.y;
    mVelocity.set(0.0f, 0.0f, speed);
    mVelocity.rotY(mAngle.y);
    speedY += accel;
    mVelocity.y = speedY > 5.5f ? 5.5f : speedY;
    mOldPos = mPos;
    mPos += mVelocity;
    mAcch.checkSteps(10.0f, &mPos, &mOldPos, mAngle.y, 0,
                     dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                         dBGCF::CHECK_SET_POS);
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x21030
void dInsFldMosquito_c::bob() {
    const param_c *param = getMosquitoParam(mType);
    f32 height = getFlyHeight();
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (isPlayerNear() == true && mPos.y < player.y + 0.5f * param->mHeight) {
        mBaseY += 0.02f;
        mBaseY = mBaseY > height ? height : mBaseY;
    } else if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
    s16 prev = mBobPhase;
    s16 next = prev + mAng::fromDegree(param->mBobSpeed);
    if (prev > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (prev <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mAng(mBobPhase).sin();
    mBobPhase += mAng::fromDegree(param->mBobSpeed);
}

// 0x21210
mAng dInsFldMosquito_c::getTargetAngle() const {
    if (mState == STATE_CHASE) {
        return mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
    }
    return mAngle.y.mAngle + (int)cM::rndCenterF(21845.0f);
}

// 0x21278
f32 dInsFldMosquito_c::getTargetDist() const {
    const param_c *param = getMosquitoParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x212A8
void dInsFldMosquito_c::getTarget(mVec3_c *out) {
    mAng ang = getTargetAngle();
    f32 dist = getTargetDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    const param_c *param = getMosquitoParam(mType);
    u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = target;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, FALSE, TRUE,
                                       -500.0f, 0.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 1, FALSE);
    }
    *out = pos;
}

// 0x2140C
bool dInsFldMosquito_c::isPlayerNear() {
    getMosquitoParam(mType);
    return mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, 320.0f) ==
           true;
}

// 0x2145C
bool dInsFldMosquito_c::isBiteNear() {
    const param_c *param = getMosquitoParam(mType);
    return mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT,
                               param->mBiteRadius) == true;
}

// 0x214AC
f32 dInsFldMosquito_c::getFlyHeight() {
    const param_c *param = getMosquitoParam(mType);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != 0) {
        return param->mHeight + chk.getWaterY();
    }
    return param->mHeight + getGroundY();
}

// 0x2152C
bool dInsFldMosquito_c::getTurnDir() {
    mVec3_c diff = mPos - mCenter;
    f32 vx = mVelocity.x;
    f32 vz = mVelocity.z;
    vz = __fabsf(vz);
    vx = __fabsf(vx);
    if (diff.x < 0.0f) {
        if (diff.z < 0.0f) {
            if (vx < vz) {
                return true;
            } else {
                return false;
            }
        } else if (vx < vz) {
            return false;
        } else {
            return true;
        }
    } else if (diff.z < 0.0f) {
        if (vx < vz) {
            return false;
        } else {
            return true;
        }
    }
    return vx < vz;
}
