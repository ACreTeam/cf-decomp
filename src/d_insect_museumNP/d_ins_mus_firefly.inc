// dInsMusFirefly_c: roams over the pond at night, glowing, and rests on its locator by day.
namespace dInsectMuseum {

enum { FIREFLY_TYPE_NUM = 1 };

dInsMusFireflyParam_c sFireflyParam[FIREFLY_TYPE_NUM] = {
    {{0.0f, 45.0f, 0.0f}, 0.1f, 0.01f, 0.9f, 0.01f, 20.0f, 10.0f, 1.8f, 6.0f, 4.0f, 1.0f, 20.0f, 10.0f, 10.0f, 5.0f,
     0.2f, 0.2f, 0.0f, 1.0f, {-3.0f, 15.0f, -1.5f}, 7.0f, 80.0f, 70, 8}, // firefly
};

// 0xBD78
const dInsMusFireflyParam_c *getFireflyParam(int type) {
    const int types[FIREFLY_TYPE_NUM] = {INSECT_FIREFLY};
    for (int i = 0; i < FIREFLY_TYPE_NUM; i++) {
        if (type == types[i]) {
            return &sFireflyParam[i];
        }
    }
    return &sFireflyParam[0];
}

mColor sGlowColorDark(0x08, 0x48, 0x10, 0xFF);
mColor sGlowColorBright(0x28, 0xA0, 0x50, 0xFF);

} // namespace dInsectMuseum

// 0xBDAC
dInsMusFirefly_c::dInsMusFirefly_c()
    : mState(NULL), mBobAngle(0), mBobHeight(0.0f), mBaseY(0.0f), _22C(0.0f), mFlyTime(0), mReturnTime(0),
      mLandTime(0), mRestTime(0), mFlyTimeLimit(0), mFlyTimer(0) {}

// 0xBE80
dInsMusFirefly_c::~dInsMusFirefly_c() {}

// 0xBF18
BOOL dInsMusFirefly_c::create(nw4r::g3d::ResFile &res) {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimeLimit = cM::rndRange<u16>(min, max);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mGlow.init();
    return TRUE;
}

// 0xBFC4
BOOL dInsMusFirefly_c::draw() {
    dInsMusBase_c::draw();
    mGlow.draw(&mPos);
    return TRUE;
}

// 0xC000
void dInsMusFirefly_c::calcMtx() {
    setAxisMtx();
}

// 0xC004
void dInsMusFirefly_c::init() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
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
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0xC0E0
void dInsMusFirefly_c::initState() {
    if (isActiveTime() == true) {
        initFly();
    } else {
        initRest();
    }
}

// 0xC128
void dInsMusFirefly_c::executeState() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    (this->*mState)();
    mFlyTimer++;
    int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
    mFlyTimer = mFlyTimer > (u16)max ? max : mFlyTimer;
    mVec3_c pos(0.0f, 1.0f, -4.0f);
    mMdl.getNodeWorldMtxMultVec(mMdl.getResMdl().GetResNode(0).GetID(), pos, pos);
    mEffect.create("af_ins_hotaru_glow", &pos, &mAngle, NULL, 1);
}

// 0xC224
void dInsMusFirefly_c::initFly() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusFirefly_c::stateFly;
    getFlyTarget(&mTargetPos);
    mFlyTime = (int)(mTargetPos.xzDistTo(mPos) / param->mFlySpeed) + 1;
    mStateId = STATE_FLY;
    mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
}

// 0xC360
void dInsMusFirefly_c::initReturn() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusFirefly_c::stateReturn;
    sLocatorMgr.setPond()->getPos(&mTargetPos, "ike_hotaru00");
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mReturnTime = (int)(0.9f * (mPos.distTo(mTargetPos) / param->mFlySpeed)) + 1;
    mStateId = STATE_RETURN;
}

// 0xC4A0
void dInsMusFirefly_c::initLand() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    mState = &dInsMusFirefly_c::stateLand;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    sLocatorMgr.setPond()->getPos(&mTargetPos, "ike_hotaru00");
    mTargetPos.x += param->mRestOffset.x;
    mTargetPos.y += param->mRestOffset.y;
    mTargetPos.z += param->mRestOffset.z;
    mPrevTargetPos = mPos;
    mAngle.y = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mLandTime = (int)(mPrevTargetPos.distTo(mTargetPos) / param->mFlySpeed) + LAND_EXTRA_TIME;
    mStateId = STATE_LAND;
}

// 0xC5FC
void dInsMusFirefly_c::initRest() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mTimer = 0;
    mState = &dInsMusFirefly_c::stateRest;
    setAnmChr(mAnmChrWait, 30.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    mRestTime = cM::rndRange<u16>(min, max);
    mStateId = STATE_REST;
    mAxis.setUp(mVec3_c(0.0f, 0.0f, 1.0f));
}

// 0xC6F0
void dInsMusFirefly_c::initTakeOff() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mTimer = 0;
    mState = &dInsMusFirefly_c::stateTakeOff;
    setAnmChr(mAnmChrFly, 30.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mTakeOffAnmRate);
    setAnmFrame(0.0f);
    mTakeOffTime = TAKE_OFF_TIME;
    mBobAngle = 0;
    mStateId = STATE_TAKE_OFF;
}

// 0xC7A8
void dInsMusFirefly_c::stateFly() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    flyToTarget(0.0f, param->mFlyAccel, param->mFlyBrake, param->mFlyTurnRate);
    bob();
    if (mFlyTime < mTimer) {
        initFly(); // sic
        if (!isFlyTimeOver()) {
            initFly();
        } else {
            initReturn();
        }
    }
}

// 0xC83C
void dInsMusFirefly_c::stateReturn() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    flyToTarget(0.0f, param->mFlyAccel, param->mFlyBrake, 2.0f * param->mFlyTurnRate);
    bobFade(1.0f - (f32)mTimer / (f32)mReturnTime / 2.0f);
    if (mReturnTime < mTimer) {
        initLand();
    }
}

// 0xC904
void dInsMusFirefly_c::stateLand() {
    dInsectUtil::easeMove(&mPos, mPrevTargetPos, mTargetPos, mLandTime, mTimer);
    mBaseY = mPos.y;
    f32 rate = 5.0f / mLandTime;
    mVec3_c up = mAxis.getUp();
    mVec3_c front = mAxis.getFront();
    mVec3_c restUp(0.0f, 0.0f, 1.0f);
    mVec3_c restFront(0.0f, 1.0f, 0.0f);
    lerpTo(&front, restFront, rate);
    mAxis.setFront(front);
    lerpTo(&up, restUp, rate);
    mAxis.setUp(up);
    if (mLandTime <= mTimer) {
        initRest();
    }
}

// 0xCA8C
void dInsMusFirefly_c::stateRest() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    bool takeOff = false;
    if (isActiveTime() == true) {
        takeOff = mRestTime <= mTimer;
    }
    mAxis.setUp(mVec3_c(0.0f, 0.0f, 1.0f));
    mAxis.setIdentity();
    mAxis.rotateZ(DEG_TO_ANGLE(180));
    mAxis.rotateX(DEG_TO_ANGLE(90));
    if (takeOff == true) {
        initTakeOff();
        int min = dInsectUtil::secToFrame(param->mFlyTimeMin);
        int max = dInsectUtil::secToFrame(param->mFlyTimeMax);
        mFlyTimeLimit = cM::rndRange<u16>(min, max);
    }
    mFlyTimer = 0;
}

// 0xCB98
void dInsMusFirefly_c::stateTakeOff() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mSpeed.y *= param->mTakeOffBrake;
    mSpeed.y += param->mTakeOffLift;
    mPos.y += mSpeed.y;
    mBaseY = mPos.y;
    f32 rate = (f32)mTimer / (f32)mTakeOffTime;
    mVec3_c up = mAxis.getUp();
    mVec3_c flyUp(0.0f, 1.0f, 0.0f);
    lerpTo(&up, flyUp, rate);
    mAxis.setUp(up);
    if (mTakeOffTime <= mTimer) {
        initFly();
    }
}

// 0xCCD0
void dInsMusFirefly_c::bob() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    updateBaseY();
    s16 next = mBobAngle.mAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle.mAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle.mAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mBobAngle.sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0xCDD8
void dInsMusFirefly_c::bobFade(f32 rate) {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    f32 scale = clamp01(rate);
    updateBaseY();
    s16 next = mBobAngle.mAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle.mAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle.mAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mBobHeight *= scale;
    mPos.y = mBaseY + mBobHeight * mBobAngle.sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0xCF40
mAng dInsMusFirefly_c::getFlyAngle() const {
    mAng range(DEG_TO_ANGLE(120));
    s16 angle = mAngle.y.mAngle;
    return angle + (s16)cM::rndCenterF(range);
}

// 0xCFA4
f32 dInsMusFirefly_c::getFlyDist() const {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0xCFD4
void dInsMusFirefly_c::getFlyTarget(mVec3_c *target) {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}

// 0xD0E4
bool dInsMusFirefly_c::isFlyTimeOver() const {
    return mFlyTimeLimit <= mFlyTimer;
}

// 0xD104
void dInsMusFirefly_c::updateBaseY() {
    f32 flyY = getFlyY();
    if (flyY < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = nw4r::ut::Max(mBaseY, flyY);
    } else {
        mBaseY += 0.05f;
        mBaseY = nw4r::ut::Min(mBaseY, flyY);
    }
}

// 0xD188
f32 dInsMusFirefly_c::getFlyY() {
    const dInsMusFireflyParam_c *param = getFireflyParam(mType);
    if (mStateId == STATE_RETURN) {
        return 3.0f + mTargetPos.y;
    }
    return param->mOffset.y + getGroundY();
}

// 0xD1F4
dInsMusFirefly_c::glow_c::glow_c() : mPhase(0), mSpeed(0), mAlpha(0xFF) {}

// 0xD20C
dInsMusFirefly_c::glow_c::~glow_c() {}

// 0xD24C
void dInsMusFirefly_c::glow_c::init() {
    mPhase = mAng::fromDegree(cM::rndRange(-180.0f, 179.0f));
    mSpeed = mAng::fromDegree(cM::rndRange(2.5f, 5.0f));
}

// 0xD2E0
void dInsMusFirefly_c::glow_c::draw(const mVec3_c *pos) {
    dEffectLight_c *light = dEffectLight_c::sInstance;
    if (light != NULL) {
        int prev = mPhase;
        mPhase += mSpeed;
        if (mPhase * prev <= 0) {
            mSpeed = mAng::fromDegree(cM::rndRange(2.5f, 5.0f));
        }
        f32 t = 0.5f + 0.5f * nw4r::math::SinIdx(mPhase);
        GXColor dark = sGlowColorDark;
        GXColor bright = sGlowColorBright;
        mColor color;
        color.lerp(dark, bright, t);
        static const f32 sizeMin = 44.8f;
        static const f32 sizeMax = 51.2f;
        f32 size = sizeMin + (sizeMax - sizeMin) * t;
        size *= mAlpha / 255.0f;
        light->addGlow(&color, pos, size);
    }
}
