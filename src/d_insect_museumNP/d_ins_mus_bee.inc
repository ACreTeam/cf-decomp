// dInsMusBee_c: the honeybee and the fly roam around their locator and return to rest; the bee lands and takes off.
namespace dInsectMuseum {

enum { BEE_TYPE_NUM = 2 };

dInsMusBeeParam_c sBeeParam[BEE_TYPE_NUM] = {
    {{0.0f, 25.0f, 0.0f}, 20.0f, 10.0f, 0.2f, 0.03f, 0.9f, 0.04f, 2.0f, 3.0f, 2.0f, 1.0f, 1.0f, 30.0f, 0.01f, 5.0f,
     3.0f, 0.02f, 15.0f, 10.0f, 0.25f, 0.25f, 0.0f, 5.0f, 60.0f, 400, {0.0f, 15.0f, 0.0f}, 10}, // honeybee
    {{0.0f, 45.0f, 0.0f}, 20.0f, 10.0f, 3.0f, 0.3f, 0.9f, 0.2f, 12.0f, 1.0f, 0.5f, 1.0f, 1.0f, 30.0f, 0.0f, 5.0f,
     3.0f, 0.0f, 15.0f, 10.0f, 0.3f, 0.3f, 0.0f, 0.0f, 100.0f, 100, {0.0f, 0.0f, 0.0f}, 12}, // fly
};

// 0x9344
const dInsMusBeeParam_c *getBeeParam(int type) {
    const int types[BEE_TYPE_NUM] = {INSECT_HONEYBEE, INSECT_FLY};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sBeeParam[i];
        }
    }
    return &sBeeParam[0];
}

} // namespace dInsectMuseum

// 0x93A8
dInsMusBee_c::dInsMusBee_c()
    : mState(NULL), mBaseY(0.0f), mBobHeight(0.0f), mBobAngle(0), mFlyTimeLimit(0), mFlyTimer(0), mFlyTime(0),
      mReturnTime(0), mLandTime(0), mRestTime(0), mTakeOffTime(0), mTargetAngle(0), mWobbleAngle(0), mRoll(0) {}

// 0x9440
dInsMusBee_c::~dInsMusBee_c() {}

// 0x9498
BOOL dInsMusBee_c::create(nw4r::g3d::ResFile &res) {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimeLimit = cM::rndRange<u16>(min, max);
    return TRUE;
}

// 0x9520
BOOL dInsMusBee_c::draw() {
    dInsMusBase_c::draw();
    return TRUE;
}

// 0x9544
void dInsMusBee_c::calcMtx() {
    setAxisMtx();
}

// 0x9548
void dInsMusBee_c::init() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    if (isActiveTime() == true) {
        mPos.x = mHomePos.x + param->mOffset.x;
        mPos.y = mHomePos.y + param->mOffset.y;
        mPos.z = mHomePos.z + param->mOffset.z;
    } else {
        mPos.x = mHomePos.x + param->mRestOffset.x;
        mPos.y = mHomePos.y + param->mRestOffset.y;
        mPos.z = mHomePos.z + param->mRestOffset.z;
    }
    mBaseY = mPos.y;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0x9650
void dInsMusBee_c::initState() {
    if (isActiveTime() == true) {
        initFly();
    } else {
        initSleep();
    }
}

// 0x9698
void dInsMusBee_c::executeState() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    (this->*mState)();
    mFlyTimer++;
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimer = mFlyTimer > (u16)max ? max : mFlyTimer;
}

// 0x9710
void dInsMusBee_c::initFly() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mTimer = 0;
    changeAnm(mAnmChrFly, mAnmVisFly);
    mState = &dInsMusBee_c::stateFly;
    getFlyTarget(&mTargetPos);
    mFlyTime = (int)(mTargetPos.xzDistTo(mPos) / param->mFlySpeed) + 1;
    mStateId = STATE_FLY;
}

// 0x9824
void dInsMusBee_c::initReturn() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mTimer = 0;
    changeAnm(mAnmChrFly, mAnmVisFly);
    mState = &dInsMusBee_c::stateReturn;
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mReturnTime = (int)(0.6f * (mPrevTargetPos.distTo(mTargetPos) / param->mFlySpeed)) + 1;
    mTargetAngle = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mStateId = STATE_RETURN;
}

// 0x9984
void dInsMusBee_c::initLand() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mTimer = 0;
    changeAnm(mAnmChrFly, mAnmVisFly);
    mState = &dInsMusBee_c::stateLand;
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mLandTime = (int)(mPrevTargetPos.distTo(mTargetPos) / param->mFlySpeed) + LAND_EXTRA_TIME;
    mStateId = STATE_LAND;
}

// 0x9AC4
void dInsMusBee_c::initRest() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 30.0f);
    setAnmVis(mAnmVisWait);
    if (isHouseFly() == true) {
        setAnmRate(0.0f);
    } else {
        setAnmRate(1.0f);
    }
    setAnmFrame(0.0f);
    mState = &dInsMusBee_c::stateRest;
    getFlyTarget(&mTargetPos);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    mRestTime = cM::rndRange<u16>(min, max);
    mStateId = STATE_REST;
    mWobbleAngle = 0;
    mRoll = 0;
}

// 0x9BCC
void dInsMusBee_c::initSleep() {
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusBee_c::stateSleep;
    mStateId = STATE_SLEEP;
    mWobbleAngle = 0;
    mRoll = 0;
}

// 0x9C6C
void dInsMusBee_c::initTakeOff() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mTimer = 0;
    mState = &dInsMusBee_c::stateTakeOff;
    setAnmChr(mAnmChrFly, 60.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mTakeOffAnmRate);
    setAnmFrame(0.0f);
    mTakeOffTime = TAKE_OFF_TIME;
    mBobAngle = 0;
    mStateId = STATE_TAKE_OFF;
    mRoll = 0;
}

// 0x9D28
void dInsMusBee_c::stateFly() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    if (isHouseFly() == true) {
        mSnd.holdSound(SE_hae);
    } else {
        mSnd.holdSound(SE_mitsubachi_fly);
    }
    mWobbleAngle += mAng::fromDegree(param->mWobbleSpeed);
    mRoll = mWobbleAngle.sin() * mAng::fromDegree(param->mRollMax);
    fly(param->mFlyAccel, param->mFlyBrake, param->mFlyTurnRate, mRoll);
    bob();
    if (mFlyTime < mTimer) {
        if (isFlyTimeOver() == true && isFacingTarget() == true && isAwayFromHome() == true) {
            if (isActiveTime() == true) {
                initReturn();
            } else if (mScare.isScared() == true) {
                initFly();
            } else {
                initReturn();
            }
        } else {
            initFly();
        }
    }
    mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
}

// 0x9F00
void dInsMusBee_c::stateReturn() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    if (isHouseFly() == true) {
        mSnd.holdSound(SE_hae);
    } else {
        mSnd.holdSound(SE_mitsubachi_fly);
    }
    mRoll = mWobbleAngle.sin() * mAng::fromDegree(param->mRollMax);
    fly(param->mFlyAccel, param->mFlyBrake, 2.0f * param->mFlyTurnRate, mRoll);
    bobFade(1.0f - (f32)mTimer / (f32)mReturnTime);
    if (mReturnTime < mTimer) {
        initLand();
    }
}

// 0xA064
void dInsMusBee_c::stateLand() {
    if (isHouseFly() == true) {
        mSnd.holdSound(SE_hae);
    } else {
        mSnd.holdSound(SE_mitsubachi_fly);
    }
    dInsectUtil::easeMove(&mPos, mPrevTargetPos, mTargetPos, mLandTime, mTimer);
    mBaseY = mPos.y;
    f32 rate = 1.0f / mLandTime;
    mVec3_c up = mAxis.getUp();
    mVec3_c tilt(0.0f, 1.0f, 0.0f);
    tilt.rotX(DEG_TO_ANGLE(25));
    lerpTo(&up, tilt, rate);
    mAxis.setUp(up);
    if (mLandTime <= mTimer) {
        if (isActiveTime() == true) {
            initRest();
        } else {
            initSleep();
        }
    }
}

// 0xA1DC
void dInsMusBee_c::stateRest() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    if (!isHouseFly()) {
        hover();
        if (mTargetPos.xzDistTo(mPos) < 0.5f) {
            initRest();
        }
    }
    if (mRestTime <= mTimer) {
        int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
        int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
        mFlyTimeLimit = cM::rndRange<u16>(min, max);
        if (isHouseFly() == true) {
            initFly();
        } else {
            initTakeOff();
        }
    }
    mFlyTimer = 0;
}

// 0xA2D4
void dInsMusBee_c::stateSleep() {
    if (mScare.isScared() == true) {
        if (isHouseFly() == true) {
            initFly();
        } else {
            initTakeOff();
        }
    }
    mFlyTimer = 0;
}

// 0xA338
void dInsMusBee_c::stateTakeOff() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mSpeed.y *= param->mTakeOffBrake;
    mSpeed.y += param->mTakeOffLift;
    mPos.y += mSpeed.y;
    mBaseY = mPos.y;
    f32 rate = 2.0f / mTakeOffTime;
    mVec3_c up = mAxis.getUp();
    mVec3_c straight(0.0f, 1.0f, 0.0f);
    lerpTo(&up, straight, rate);
    mAxis.setUp(up);
    if (mTakeOffTime <= mTimer) {
        initFly();
    }
}

// 0xA464
bool dInsMusBee_c::isHouseFly() {
    return mType == INSECT_FLY;
}

// 0xA478
bool dInsMusBee_c::isFlyTimeOver() {
    return mFlyTimeLimit <= mFlyTimer;
}

// 0xA498
void dInsMusBee_c::fly(f32 accel, f32 brake, f32 turnRate, mAng roll) {
    flyToTarget(0.0f, accel, brake, turnRate);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    up.rotX(roll);
    up.rotY(mAxis.getAngleY());
    mAxis.setUp(up);
}

// 0xA544
void dInsMusBee_c::hover() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    up.rotX(DEG_TO_ANGLE(25));
    dInsectUtil::flyTo(&mPos, &mAxis, mTargetPos, up, param->mHoverAccel, param->mHoverTurnRate);
}

// 0xA5CC
void dInsMusBee_c::bob() {
    updateBaseY();
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    s16 next = mBobAngle.mAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle.mAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle.mAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mBobAngle.sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0xA6D0
void dInsMusBee_c::bobFade(f32 rate) {
    rate = clamp01(rate);
    updateBaseY();
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    s16 next = mBobAngle.mAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle.mAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle.mAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mBobHeight *= rate;
    mPos.y = mBaseY + mBobHeight * mBobAngle.sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0xA824
void dInsMusBee_c::updateBaseY() {
    f32 flyY = getFlyY();
    if (flyY < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = nw4r::ut::Max(mBaseY, flyY);
    } else {
        mBaseY += 0.05f;
        mBaseY = nw4r::ut::Min(mBaseY, flyY);
    }
}

// 0xA8A8
f32 dInsMusBee_c::getFlyY() {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    if (mStateId == STATE_RETURN) {
        return 3.0f + mTargetPos.y;
    }
    return param->mOffset.y + getGroundY();
}

// 0xA914
mAng dInsMusBee_c::getFlyAngle() {
    mAng range(DEG_TO_ANGLE(120));
    s16 angle = mAngle.y.mAngle;
    return angle + (s16)cM::rndCenterF(range);
}

// 0xA978
f32 dInsMusBee_c::getFlyDist() const {
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0xA9A8
void dInsMusBee_c::getFlyTarget(mVec3_c *target) {
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusBeeParam_c *param = getBeeParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}

// 0xAAA8
bool dInsMusBee_c::isAwayFromHome() {
    mVec3_c home;
    getLocatorPos(&home);
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType); // sic: the dragonfly table
    home.x += param->mRestOffset.x;
    home.y += param->mRestOffset.y;
    home.z += param->mRestOffset.z;
    if (10.0f <= home.xzDistTo(mPos)) {
        return true;
    }
    return false;
}

// 0xAB58
bool dInsMusBee_c::isFacingTarget() {
    if (isHouseFly() == true) {
        return true;
    }
    mVec3_c front = mAxis.getFront();
    mVec3_c diff = mTargetPos - mPos;
    return 0.59f <= front.dot(diff);
}
