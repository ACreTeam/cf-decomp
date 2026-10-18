// dInsMusMoth_c: flutters around the lamp at its active time and rests on it otherwise.
namespace dInsectMuseum {

enum { MOTH_TYPE_NUM = 1 };

dInsMusMothParam_c sMothParam[MOTH_TYPE_NUM] = {
    {{0.0f, 34.0f, 0.0f}, 0.3f, 0.03f, 0.9f, 0.05f, 20.0f, 10.0f, 8.5f, 5.0f, 3.0f, 0.8f, 7.0f, 96.0f, 120, 0.8f,
     {9.0f, 8.0f, 12.0f}, 23},
};

// 0xABF4
// non-const return needed to match (draw scheduling)
dInsMusMothParam_c *getMothParam(int type) {
    const int types[MOTH_TYPE_NUM] = {INSECT_MOTH};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sMothParam[i];
        }
    }
    return &sMothParam[0];
}

} // namespace dInsectMuseum

// 0xAC28
dInsMusMoth_c::dInsMusMoth_c()
    : mState(NULL), mIsResting(FALSE), mBobAngle(0), mBobHeight(0.0f), mBaseY(0.0f), mFlyTime(0), _22E(0),
      mFlutterAngle(0) {}

// 0xACA8
dInsMusMoth_c::~dInsMusMoth_c() {}

// 0xAD00
BOOL dInsMusMoth_c::create(nw4r::g3d::ResFile &res) {
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    mBgRadius = 1.5f * mShadowSize;
    return TRUE;
}

// 0xAD4C
BOOL dInsMusMoth_c::draw() {
    setScale(getMothParam(mType)->mScale);
    dInsMusBase_c::draw();
    return TRUE;
}

// 0xAD98
void dInsMusMoth_c::calcMtx() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    mVec3_c pos = mPos;
    if (isActiveTime() == true) {
        mVec3_c side = mAxis.getSide();
        mFlutterAngle.mAngle += mAng::fromDegree(param->mBobSpeed) / 4;
        pos += side * mFlutterAngle.sin() * 5.0f;
    }
    mMtx_c mtx;
    mMtx_c rotMtx;
    mMtx_c tiltMtx;
    dActor_c::makeMtx(&mtx, &pos, 0);
    rotMtx = getAxisMtx();
    PSMTXConcat(mtx, rotMtx, mtx);
    tiltMtx.XrotS(-0xAAB);
    PSMTXConcat(mtx, tiltMtx, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0xAF68
void dInsMusMoth_c::init() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    if (isActiveTime() == true) {
        mPos.x = mHomePos.x + param->mOffset.x;
        mPos.y = mHomePos.y + param->mOffset.y;
        mPos.z = mHomePos.z + param->mOffset.z;
    } else {
        mPos.x = mHomePos.x + param->mRestOffset.x;
        mPos.y = mHomePos.y + param->mRestOffset.y;
        mPos.z = mHomePos.z + param->mRestOffset.z;
    }
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mShadowSize = param->mShadowSize;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0xB068
void dInsMusMoth_c::initState() {
    if (isActiveTime() == true) {
        initFly();
    } else {
        initRest();
    }
}

// 0xB0B0
void dInsMusMoth_c::executeState() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    (this->*mState)();
}

// 0xB0F0
void dInsMusMoth_c::initFly() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    mTimer = 0;
    mState = &dInsMusMoth_c::stateFly;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    mPrevTargetPos = mPos;
    getFlyTarget(&mTargetPos);
    mFlyTime = (int)(mTargetPos.xzDistTo(mPos) / param->mFlySpeed) + 1;
    mIsResting = FALSE;
}

// 0xB218
void dInsMusMoth_c::initRest() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    mTimer = 0;
    mState = &dInsMusMoth_c::stateRest;
    setAnmChr(mAnmChrWait, 60.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mAngle.x = DEG_TO_ANGLE(-90);
    mAngle.y = 0x199A;
    mAngle.z = DEG_TO_ANGLE(-180);
    mAxis.setIdentity();
    mAxis.rotateX(DEG_TO_ANGLE(-90));
    mAxis.rotateZ(DEG_TO_ANGLE(-144));
    mBobAngle = 0;
    mIsResting = TRUE;
}

// 0xB308
void dInsMusMoth_c::stateFly() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    fly(param->mFlyAccel, param->mFlyBrake, param->mFlyTurnRate);
    bob();
    if (mFlyTime < mTimer) {
        initFly();
    }
}

// 0xB370
void dInsMusMoth_c::stateRest() {}

// 0xB374
void dInsMusMoth_c::fly(f32 accel, f32 brake, f32 turnRate) {
    flyToTarget(0.0f, accel, brake, turnRate);
}

// 0xB394
void dInsMusMoth_c::bob() {
    updateBaseY();
    const dInsMusMothParam_c *param = getMothParam(mType);
    s16 angle = mBobAngle.mAngle;
    s16 next = angle + mAng::fromDegree(param->mBobSpeed);
    if (angle > 0 && next <= 0) {
        mBobHeight = cM::rndRange<f32>(param->mBobHeightMin, param->mBobHeightMax);
    } else if (angle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange<f32>(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mBobAngle.sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0xB498
void dInsMusMoth_c::updateBaseY() {
    f32 flyY = getFlyY();
    if (flyY < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = nw4r::ut::Max(mBaseY, flyY);
    } else {
        mBaseY += 0.05f;
        mBaseY = nw4r::ut::Min(mBaseY, flyY);
    }
}

// 0xB51C
f32 dInsMusMoth_c::getFlyY() {
    const dInsMusMothParam_c *param = getMothParam(mType);
    return param->mOffset.y + getGroundY();
}

// 0xB568
mAng dInsMusMoth_c::getFlyAngle() {
    mAng range(DEG_TO_ANGLE(120));
    s16 angle = mAngle.y.mAngle;
    return angle + (s16)cM::rndCenterF(range);
}

// 0xB5CC
f32 dInsMusMoth_c::getFlyDist() const {
    const dInsMusMothParam_c *param = getMothParam(mType);
    return cM::rndRange<f32>(param->mFlyDistMin, param->mFlyDistMax);
}

// 0xB5FC
void dInsMusMoth_c::getFlyTarget(mVec3_c *target) {
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusMothParam_c *param = getMothParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    result.y += 32.0f;
    *target = result;
}
