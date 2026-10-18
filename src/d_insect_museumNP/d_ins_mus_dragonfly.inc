// dInsMusDragonfly_c: turns, darts to a random point and hovers; lands on its resting spot after a while.
namespace dInsectMuseum {

enum { DRAGONFLY_TYPE_NUM = 4 };

dInsMusDragonflyParam_c sDragonflyParam[DRAGONFLY_TYPE_NUM] = {
    {{0.0f, 45.0f, 12.0f}, 5.0f, 3.0f, 0.5f, 120.0f, 50.0f, 0.98f, 0.002f, 1.2f, 5.0f, 15.0f, 10.0f, 12.0f, 5.0f,
     7.0f, 50.0f, 400, {0.0f, 34.0f, 0.0f}, 2}, // red_dragonfly
    {{0.0f, 45.0f, 12.0f}, 5.0f, 3.0f, 0.5f, 150.0f, 50.0f, 0.98f, 0.003f, 1.2f, 5.0f, 15.0f, 10.0f, 15.0f, 4.0f,
     7.0f, 85.0f, 400, {0.0f, 0.0f, 0.0f}, 2}, // darner_dragonfly
    {{57.0f, 45.0f, -54.0f}, 5.0f, 3.0f, 0.5f, 200.0f, 50.0f, 0.97f, 0.003f, 1.2f, 5.0f, 15.0f, 10.0f, 15.0f, 5.0f,
     10.0f, 60.0f, 400, {57.0f, 19.0f, -54.0f}, 2}, // banded_dragonfly
    {{0.0f, 55.0f, 12.0f}, 3.5f, 3.0f, 0.5f, 200.0f, 80.0f, 0.97f, 0.003f, 1.2f, 5.0f, 15.0f, 10.0f, 15.0f, 7.0f,
     12.0f, 60.0f, 400, {0.0f, 33.0f, 0.0f}, 21}, // giant_petaltail_dragonfly
};

// 0x5EC0
const dInsMusDragonflyParam_c *getDragonflyParam(int type) {
    const int types[DRAGONFLY_TYPE_NUM] = {
        INSECT_RED_DRAGONFLY, INSECT_DARNER_DRAGONFLY, INSECT_BANDED_DRAGONFLY, INSECT_GIANT_PETALTAIL_DRAGONFLY,
    };
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sDragonflyParam[i];
        }
    }
    return &sDragonflyParam[0];
}

} // namespace dInsectMuseum

// 0x5F34
dInsMusDragonfly_c::dInsMusDragonfly_c()
    : mState(NULL), mStateId(STATE_TURN), mPrevStateId(STATE_TURN), mBaseY(0.0f), mHoverRate(0.0f), mHoverSpeed(0), mHoverPhase(0),
      _230(0), mFlyTime(0), mLandTime(0), mRestTime(0), mFlyTimer(0), mFlyTimeLimit(0), mTargetAngle(0) {}

// 0x5FCC
dInsMusDragonfly_c::~dInsMusDragonfly_c() {}

// 0x6024
BOOL dInsMusDragonfly_c::create(nw4r::g3d::ResFile &res) {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimeLimit = cM::rndRange<u16>(min, max);
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    return TRUE;
}

// 0x60BC
BOOL dInsMusDragonfly_c::draw() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    dInsMusBase_c::draw();
    f32 alpha = 0.5f * (mAlpha / 255.0f);
    mAng angle = getAngleY(); // inline getter needed to match
    dShadow::requestRound(&mPos, 0, &angle, param->mShadowSizeX, 1.0f, alpha, param->mShadowSizeZ, 1.0f);
    return TRUE;
}

// 0x6168
void dInsMusDragonfly_c::init() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    if (isActiveTime() == true) {
        mPos.x = mHomePos.x + param->mFlyOffset.x;
        mPos.y = mHomePos.y + param->mFlyOffset.y;
        mPos.z = mHomePos.z + param->mFlyOffset.z;
    } else {
        mPos.x = mHomePos.x + param->mRestOffset.x;
        mPos.y = mHomePos.y + param->mRestOffset.y;
        mPos.z = mHomePos.z + param->mRestOffset.z;
    }
    mScareRadius = param->mScareRadius;
    mShadowSize = 0.0f;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSizeZ;
}

// 0x6240
void dInsMusDragonfly_c::initState() {
    if (isActiveTime() == true) {
        initTurn();
    } else {
        initRest();
    }
}

// 0x6288
void dInsMusDragonfly_c::executeState() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    (this->*mState)();
    mFlyTimer++;
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimer = mFlyTimer > (u16)max ? max : mFlyTimer;
}

// 0x6300
void dInsMusDragonfly_c::initTurn() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mPrevStateId = mStateId;
    mStateId = STATE_TURN;
    mTimer = 0;
    mState = &dInsMusDragonfly_c::stateTurn;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    getFlyTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mTargetAngle = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mBaseY = mPos.y;
    mHoverRate = 0.0f;
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mHoverSpeed = 0;
}

// 0x6424
void dInsMusDragonfly_c::initFly() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mPrevStateId = mStateId;
    mStateId = STATE_FLY;
    mTimer = 0;
    mState = &dInsMusDragonfly_c::stateFly;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    mBaseY = mPos.y;
    if (isBig() == true) {
        mFlyTime = FLY_TIME_BIG;
    } else {
        mFlyTime = FLY_TIME;
    }
    mAngle.y = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    f32 len = mTargetPos.xzDistTo(mPrevTargetPos);
    mSpeed.set(0.0f, 0.0f, len / (param->mBrake / (1.0f - param->mBrake)));
    mSpeed.rotY(mAngle.y);
    mSpeed.y = -param->mGravity * (mFlyTime / 2);
    if (getFlyY() < mPos.y) {
        mSpeed.y *= 1.5f;
    } else {
        mSpeed.y *= 0.8f;
    }
}

// 0x65FC
void dInsMusDragonfly_c::initTurnHome() {
    mPrevStateId = mStateId;
    mStateId = STATE_TURN_HOME;
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    mState = &dInsMusDragonfly_c::stateTurnHome;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mTargetAngle = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mBaseY = mPos.y;
    mHoverRate = 0.0f;
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mHoverSpeed = 0;
}

// 0x6754
void dInsMusDragonfly_c::initLand() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mPrevStateId = mStateId;
    mStateId = STATE_LAND;
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    mState = &dInsMusDragonfly_c::stateLand;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    getLocatorPos(&mTargetPos);
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mAngle.y = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mLandTime = (int)(mPrevTargetPos.distTo(mTargetPos) / 0.3f) + 1;
    mBaseY = mPos.y;
}

// 0x68B4
void dInsMusDragonfly_c::initRest() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mPrevStateId = mStateId;
    mStateId = STATE_REST;
    mTimer = 0;
    mState = &dInsMusDragonfly_c::stateRest;
    setAnmChr(mAnmChrWait, 30.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mRestTime = dInsectUtil::secToFrame(param->mRestTime);
    mPrevTargetPos = mPos;
    mBaseY = mPos.y;
}

// 0x6994
void dInsMusDragonfly_c::initTakeOff() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mPrevStateId = mStateId;
    mStateId = STATE_TAKE_OFF;
    mTimer = 0;
    mState = &dInsMusDragonfly_c::stateTakeOff;
    setAnmChr(mAnmChrFly, 30.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAnmRate);
    setAnmFrame(0.0f);
    mPrevTargetPos = mPos;
    mTargetPos = mPos;
    mTargetPos.y += param->mTakeOffHeight;
    mBaseY = mPos.y;
    mHoverRate = 0.0f;
    if (isDarner() == true) {
        dEffect::create("af_ins_amenbo_hamon", &mPos, &mAngle, NULL);
    }
}

// 0x6AB0
void dInsMusDragonfly_c::stateTurn() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    updateHover();
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 15, mAng::fromDegree(param->mTurnSpeed), DEG_TO_ANGLE(1));
    if (mAngle.y.mAngle == mTargetAngle) {
        initFly();
    }
}

// 0x6B40
void dInsMusDragonfly_c::stateFly() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mSpeed.x *= param->mBrake;
    mSpeed.z *= param->mBrake;
    mSpeed.y += param->mGravity;
    mPos += mSpeed;
    mBaseY = mPos.y;
    if (mFlyTime <= mTimer) {
        if (isFlyTimeOver() == true && isAwayFromHome() == true && !isFarFromHome()) {
            if (isActiveTime() == true) {
                initTurnHome();
            } else if (mScare.isScared() == true) {
                initTurn();
            } else {
                initTurnHome();
            }
        } else {
            initTurn();
        }
    }
}

// 0x6C54
void dInsMusDragonfly_c::stateTurnHome() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    updateHover();
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 15, mAng::fromDegree(param->mTurnSpeed), DEG_TO_ANGLE(1));
    if (mAngle.y.mAngle == mTargetAngle) {
        initLand();
    }
}

// 0x6CE4
void dInsMusDragonfly_c::stateLand() {
    dInsectUtil::easeMove(&mPos, mPrevTargetPos, mTargetPos, mLandTime, mTimer);
    mBaseY = mPos.y;
    if (mLandTime <= mTimer) {
        initRest();
    }
}

// 0x6D44
void dInsMusDragonfly_c::stateRest() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
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

// 0x6DF8
void dInsMusDragonfly_c::stateTakeOff() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    static const s16 takeOffTime = 50;
    dInsectUtil::easeMove(&mPos, mPrevTargetPos, mTargetPos, takeOffTime, mTimer);
    mBaseY = mPos.y;
    if (mTimer >= takeOffTime) {
        initTurn();
    }
}

// 0x6E5C
void dInsMusDragonfly_c::updateHover() {
    mHoverRate += 0.02f;
    mHoverRate = nw4r::ut::Min(mHoverRate, 1.0f);
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mHoverSpeed += 10;
    u16 period = dInsectUtil::secToFrame(param->mHoverPeriod);
    s16 step = DEG_TO_ANGLE(180) / period;
    mHoverSpeed = nw4r::ut::Min<mAng>(mHoverSpeed, mAng(step * 2));
    mHoverPhase += mHoverSpeed;
    f32 flyY = param->mFlyOffset.y + getGroundY();
    if (flyY < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = nw4r::ut::Max(mBaseY, flyY);
    } else {
        mBaseY += 0.1f;
        mBaseY = nw4r::ut::Min(mBaseY, flyY);
    }
    mPos.y = mBaseY + mHoverRate * (param->mHoverHeight * mHoverPhase.sin());
}

// 0x6FC0
mAng dInsMusDragonfly_c::getFlyAngle() const {
    s16 angle = mAngle.y.mAngle;
    int range = isBig() == true ? 0xE39 : 0x2AAB;
    if (isPetaltail() == true && mPrevStateId == STATE_TAKE_OFF) {
        angle = -DEG_TO_ANGLE(90);
    }
    return mAng(angle + (int)cM::rndCenterF(range));
}

// 0x7068
f32 dInsMusDragonfly_c::getFlyDist() const {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    return cM::rndRange<f32>(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x7098
void dInsMusDragonfly_c::getFlyTarget(mVec3_c *target) {
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    if (param->mFlyDistMin <= result.xzDistTo(mPos)) {
        mVec3_c dir = result - mPos;
        dir.y = 0.0f;
        dir.normalize();
        dir *= param->mFlyDistMin;
        result = mPos + dir;
    }
    *target = result;
}

// 0x7258
bool dInsMusDragonfly_c::isBig() const {
    if (mType == INSECT_BANDED_DRAGONFLY) {
        return true;
    }
    return isPetaltail() == true;
}

// 0x7298
bool dInsMusDragonfly_c::isDarner() const {
    return mType == INSECT_DARNER_DRAGONFLY;
}

// 0x72AC
bool dInsMusDragonfly_c::isPetaltail() const {
    return mType == INSECT_GIANT_PETALTAIL_DRAGONFLY;
}

// 0x72C0
bool dInsMusDragonfly_c::isFlyTimeOver() {
    return mFlyTimeLimit <= mFlyTimer;
}

// 0x72E0
bool dInsMusDragonfly_c::isAwayFromHome() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mVec3_c home;
    getLocatorPos(&home);
    home.x += param->mRestOffset.x;
    home.y += param->mRestOffset.y;
    home.z += param->mRestOffset.z;
    if (32.0f <= home.xzDistTo(mPos)) {
        return true;
    }
    return false;
}

// 0x73A0
bool dInsMusDragonfly_c::isFarFromHome() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    mVec3_c home;
    getLocatorPos(&home);
    home.x += param->mRestOffset.x;
    home.y += param->mRestOffset.y;
    home.z += param->mRestOffset.z;
    if (80.0f <= home.xzDistTo(mPos)) {
        return true;
    }
    return false;
}

// 0x7460
f32 dInsMusDragonfly_c::getFlyY() {
    const dInsMusDragonflyParam_c *param = getDragonflyParam(mType);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0); // unused, constructed in the target
    return param->mFlyOffset.y + getGroundY();
}
