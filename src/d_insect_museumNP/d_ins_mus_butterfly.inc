// dInsMusButterfly_c: flies around near its flower and now and then lands on it to rest.
namespace dInsectMuseum {

enum { BUTTERFLY_TYPE_NUM = 9 };

dInsMusButterflyParam_c sButterflyParam[BUTTERFLY_TYPE_NUM] = {
    {{0.0f, 30.0f, 0.0f}, 0.1f, 0.01f, 0.9f, 0.02f, 20.0f, 10.0f, 12.0f, 2.0f, 1.0f, 0.9f, 1.4f, 0.08f, 0.08f, 0.0f, 1.5f,
     20.0f, 10.0f, 10.0f, 5.0f, 6.0f, 5.0f, 140.0f, 120, 1.0f, {1.0f, 15.0f, -1.0f}, 0}, // common_butterfly
    {{0.0f, 30.0f, 0.0f}, 0.1f, 0.01f, 0.9f, 0.02f, 20.0f, 10.0f, 12.0f, 2.0f, 1.0f, 0.9f, 1.4f, 0.08f, 0.08f, 0.0f, 1.5f,
     20.0f, 10.0f, 10.0f, 5.0f, 6.0f, 5.0f, 140.0f, 120, 1.0f, {0.0f, 14.5f, 2.5f}, 0}, // yellow_butterfly
    {{0.0f, 30.0f, 0.0f}, 0.125f, 0.0125f, 0.9f, 0.03f, 30.0f, 20.0f, 8.0f, 3.0f, 2.0f, 1.0f, 1.6f, 0.17f, 0.17f, 0.0f,
     1.6f, 20.0f, 10.0f, 10.0f, 5.0f, 8.5f, 8.0f, 140.0f, 120, 1.0f, {3.0f, 12.0f, 2.0f}, 0}, // tiger_butterfly
    {{0.0f, 30.0f, 0.0f}, 0.125f, 0.0125f, 0.9f, 0.03f, 30.0f, 20.0f, 8.0f, 3.0f, 2.0f, 1.0f, 1.6f, 0.17f, 0.17f, 0.0f,
     1.6f, 20.0f, 10.0f, 10.0f, 5.0f, 8.5f, 8.0f, 140.0f, 120, 1.0f, {0.0f, 15.5f, 0.0f}, 0}, // peacock_butterfly
    {{32.0f, 50.0f, 0.0f}, 0.2f, 0.02f, 0.9f, 0.02f, 20.0f, 10.0f, 4.0f, 4.0f, 2.0f, 0.8f, 1.7f, 0.1f, 0.1f, 0.0f, 1.7f,
     20.0f, 10.0f, 10.0f, 5.0f, 8.0f, 7.0f, 96.0f, 120, 1.0f, {23.0f, 50.0f, 10.0f}, 21}, // monarch_butterfly
    {{0.0f, 20.0f, 0.0f}, 0.5f, 0.025f, 0.95f, 0.04f, 30.0f, 20.0f, 4.0f, 4.0f, 3.0f, 1.0f, 1.6f, 0.13f, 0.13f, 0.0f, 1.5f,
     20.0f, 10.0f, 10.0f, 5.0f, 7.5f, 7.0f, 140.0f, 120, 1.0f, {0.0f, 15.5f, -1.5f}, 0}, // emperor_butterfly
    {{0.0f, 30.0f, 0.0f}, 0.6f, 0.06f, 0.9f, 0.07f, 20.0f, 10.0f, 4.0f, 4.0f, 3.0f, 0.9f, 1.5f, 0.18f, 0.18f, 0.0f, 1.5f,
     20.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f, 150.0f, 120, 1.0f, {7.5f, 14.0f, -2.5f}, 0}, // agrias_butterfly
    {{12.0f, 30.0f, 1.0f}, 0.25f, 0.025f, 0.9f, 0.05f, 30.0f, 20.0f, 10.0f, 4.0f, 3.0f, 1.2f, 1.8f, 0.15f, 0.15f, 0.0f,
     1.8f, 20.0f, 10.0f, 10.0f, 5.0f, 9.0f, 7.0f, 96.0f, 120, 1.0f, {10.0f, 23.0f, -7.0f}, 9}, // raja_brooke
    {{0.0f, 60.0f, 0.0f}, 0.3f, 0.03f, 0.9f, 0.03f, 50.0f, 30.0f, 3.0f, 6.0f, 4.0f, 0.7f, 1.4f, 0.3f, 0.3f, 0.0f, 1.4f,
     50.0f, 30.0f, 10.0f, 5.0f, 15.0f, 13.0f, 96.0f, 120, 0.8f, {0.0f, 34.0f, 0.0f}, 15}, // birdwing_butterfly
};

// 0x42A4
const dInsMusButterflyParam_c *getButterflyParam(int type) {
    const int types[BUTTERFLY_TYPE_NUM] = {
        INSECT_COMMON_BUTTERFLY, INSECT_YELLOW_BUTTERFLY, INSECT_TIGER_BUTTERFLY,
        INSECT_PEACOCK_BUTTERFLY, INSECT_MONARCH_BUTTERFLY, INSECT_EMPEROR_BUTTERFLY,
        INSECT_AGRIAS_BUTTERFLY, INSECT_RAJA_BROOKE, INSECT_BIRDWING_BUTTERFLY,
    };
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sButterflyParam[i];
        }
    }
    return &sButterflyParam[0];
}

} // namespace dInsectMuseum

// 0x4350
dInsMusButterfly_c::dInsMusButterfly_c()
    : mState(NULL), mStateId(STATE_FLY), mBobAngle(0), mBobHeight(0.0f), mBaseY(0.0f), mFlyTime(0), mApproachTime(0),
      mLandTime(0), mRestTime(0), mTakeOffTime(0), mFlyTimeLimit(0), mFlyTimer(0), mFlapAngle(0) {}

// 0x43E4
dInsMusButterfly_c::~dInsMusButterfly_c() {}

// 0x443C
BOOL dInsMusButterfly_c::create(nw4r::g3d::ResFile &res) {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimeLimit = cM::rndRange<u16>(min, max);
    f32 frameMax = mAnmChr.mFrameMax;
    setAnmFrame(cM::rndRange(0.0f, frameMax));
    return TRUE;
}

// 0x44DC
BOOL dInsMusButterfly_c::draw() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    setScale(param->mScale);
    dInsMusBase_c::draw();
    mAngle.y = mAxis.getAngleY();
    f32 alpha = 0.5f * (mAlpha / 255.0f);
    mAng angle = mAngle.y;
    dShadow::requestRound(&mPos, 0, &angle, param->mShadowSizeX, 1.0f, alpha, param->mShadowSizeZ, 1.0f);
    return TRUE;
}

// 0x45A0
void dInsMusButterfly_c::calcMtx() {
    mMtx_c mtx;
    mMtx_c rotMtx;
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mVec3_c pos = mPos;
    mVec3_c side = mAxis.getSide();
    if (mStateId == STATE_FLY) {
        mFlapAngle += mAng::fromDegree(param->mBobSpeed) / 4;
    } else {
        mFlapAngle = 0.95f * mFlapAngle;
    }
    pos += side * mAng(mFlapAngle).sin() * 1.2f;
    dActor_c::makeMtx(&mtx, &pos, 0);
    rotMtx = getAxisMtx();
    PSMTXConcat(mtx, rotMtx, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0x4790
void dInsMusButterfly_c::init() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    if (isActiveTime() == true) {
        mPos.x = mHomePos.x + param->mFlyOffset.x;
        mPos.y = mHomePos.y + param->mFlyOffset.y;
        mPos.z = mHomePos.z + param->mFlyOffset.z;
    } else {
        mPos.x = mHomePos.x + param->mRestOffset.x;
        mPos.y = mHomePos.y + param->mRestOffset.y;
        mPos.z = mHomePos.z + param->mRestOffset.z;
    }
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mShadowSize = 0.0f;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = 1.5f * param->mShadowSizeX;
}

// 0x48A0
void dInsMusButterfly_c::initState() {
    if (isActiveTime() == true) {
        initFly();
    } else {
        initRest();
    }
}

// 0x48E8
void dInsMusButterfly_c::executeState() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    (this->*mState)();
    mFlyTimer++;
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimer = mFlyTimer > (u16)max ? max : mFlyTimer;
}

// 0x4960
void dInsMusButterfly_c::initFly() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mTimer = 0;
    mState = &dInsMusButterfly_c::stateFly;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    mPrevTargetPos = mPos;
    getFlyTarget(&mTargetPos);
    mFlyTime = (int)(mTargetPos.xzDistTo(mPos) / param->mFlySpeed) + 1;
    mStateId = STATE_FLY;
}

// 0x4A88
void dInsMusButterfly_c::initApproach() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusButterfly_c::stateApproach;
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mApproachTime = (int)(0.8f * (mPrevTargetPos.distTo(mTargetPos) / param->mFlySpeed)) + 1;
    mStateId = STATE_APPROACH;
}

// 0x4BD0
void dInsMusButterfly_c::initLand() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    mState = &dInsMusButterfly_c::stateLand;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLandAnmRate);
    setAnmFrame(frame);
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mLandTime = (int)(mPrevTargetPos.distTo(mTargetPos) / param->mFlySpeed) + LAND_EXTRA_TIME;
    mStateId = STATE_LAND;
}

// 0x4D0C
void dInsMusButterfly_c::initRest() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mTimer = 0;
    mState = &dInsMusButterfly_c::stateRest;
    setAnmChr(mAnmChrWait, 60.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    mRestTime = cM::rndRange<u16>(min, max);
    mBobAngle = 0;
    mStateId = STATE_REST;
}

// 0x4DEC
void dInsMusButterfly_c::initTakeOff() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mTimer = 0;
    mState = &dInsMusButterfly_c::stateTakeOff;
    setAnmChr(mAnmChrFly, 60.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mTakeOffAnmRate);
    setAnmFrame(0.0f);
    mTakeOffTime = TAKE_OFF_TIME;
    mBobAngle = 0;
    mStateId = STATE_TAKE_OFF;
}

// 0x4EA4
void dInsMusButterfly_c::stateFly() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    flyToTarget(0.0f, param->mFlyAccel, param->mFlyBrake, param->mTurnRate);
    calcBob();
    if (mFlyTime < mTimer) {
        if (!isFlyTimeOver()) {
            initFly();
        } else if (isActiveTime() == true) {
            if (isNearRestSpot() == true) {
                initApproach();
            } else {
                initFly();
            }
        } else if (mScare.isScared() == true) {
            initFly();
        } else {
            initApproach();
        }
    }
}

// 0x4F84
void dInsMusButterfly_c::stateApproach() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    flyToTarget(0.0f, param->mFlyAccel, param->mFlyBrake, 2.0f * param->mTurnRate);
    calcBob(1.0f - (f32)mTimer / (f32)mApproachTime);
    if (mApproachTime < mTimer) {
        initLand();
    }
}

// 0x5044
void dInsMusButterfly_c::stateLand() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    dInsectUtil::easeMove(&mPos, mPrevTargetPos, mTargetPos, mLandTime, mTimer);
    mBaseY = mPos.y;
    if (mLandTime <= mTimer) {
        initRest();
    }
}

// 0x50AC
void dInsMusButterfly_c::stateRest() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    bool takeOff;
    if (isActiveTime() == true) {
        takeOff = mRestTime <= mTimer;
    } else {
        takeOff = mScare.isScared();
    }
    if (takeOff == true) {
        initTakeOff();
        int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
        int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
        mFlyTimeLimit = cM::rndRange<u16>(min, max);
    }
    mFlyTimer = 0;
}

// 0x5160
void dInsMusButterfly_c::stateTakeOff() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mSpeed.y *= param->mTakeOffBrake;
    mSpeed.y += param->mTakeOffLift;
    mPos.y += mSpeed.y;
    mBaseY = mPos.y;
    if (mTakeOffTime <= mTimer) {
        initFly();
    }
}

// 0x51D4
void dInsMusButterfly_c::calcBob() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    calcBaseY();
    s16 next = mBobAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mAng(mBobAngle).sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0x52DC
void dInsMusButterfly_c::calcBob(f32 rate) {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    f32 scale = clamp01(rate);
    calcBaseY();
    s16 next = mBobAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mBobHeight *= scale;
    mPos.y = mBaseY + mBobHeight * mAng(mBobAngle).sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0x5444
void dInsMusButterfly_c::calcBaseY() {
    f32 y = getBaseTargetY();
    if (y < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < y ? y : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > y ? y : mBaseY;
    }
}

// 0x54C8
f32 dInsMusButterfly_c::getBaseTargetY() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    if (mStateId == STATE_APPROACH) {
        return 3.0f + mTargetPos.y;
    }
    return param->mFlyOffset.y + getGroundY();
}

// 0x5534
mAng dInsMusButterfly_c::getFlyAngle() {
    mAng angle = mAngle.y;
    mAng range(DEG_TO_ANGLE(120));
    return angle.mAngle + (s16)cM::rndCenterF(range);
}

// 0x5598
f32 dInsMusButterfly_c::getFlyDist() const {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x55C8
void dInsMusButterfly_c::getFlyTarget(mVec3_c *target) {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    u8 steps = (int)(param->mFlyDistMax / 4) + 1;
    mVec3_c from = mPos;
    mVec3_c to = pos;
    if (!dInsectUtil::getGroundMovePos(&result, from, to, steps, TRUE, TRUE, -500.0f, 0.0f)) {
        dInsectUtil::getLandPos(&result, mPos, 1, TRUE);
    }
    result.y += 32.0f;
    *target = result;
}

// 0x5768
BOOL dInsMusButterfly_c::isFlyTimeOver() {
    return mFlyTimeLimit <= mFlyTimer;
}

// 0x5788
bool dInsMusButterfly_c::isNearRestSpot() {
    const dInsMusButterflyParam_c *param = getButterflyParam(mType);
    mVec3_c pos;
    getLocatorPos(&pos);
    pos.x += param->mRestOffset.x;
    pos.y += param->mRestOffset.y;
    pos.z += param->mRestOffset.z;
    f32 dist = pos.xzDistTo(mPos);
    if (16.0f <= dist && dist <= 32.0f) {
        return true;
    }
    return false;
}
