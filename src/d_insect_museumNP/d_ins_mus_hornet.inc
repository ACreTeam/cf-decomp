// dInsMusHornet_c: the wasp of the museum's insect rooms.

namespace dInsectMuseum {

enum { HORNET_TYPE_NUM = 1 };

dInsMusHornetParam_c sHornetParam[HORNET_TYPE_NUM] = {
    {{-64.0f, 30.0f, 0.0f}, 60.0f, 40.0f, 0.8f, 1.2f, 0.06f, 0.95f, 0.05f, 1.0f, 2.0f, 30.0f, 2.0f, 5.0f, 5.0f, 0.95f,
     5.0f, 3.0f, 6.0f, 64.0f, 100, 22},
};

// 0x10274
int getHornetParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_BEE:
        idx = 0;
        break;
    }
    return idx;
}

// 0x10288
const dInsMusHornetParam_c *getHornetParam(int type) {
    return &sHornetParam[getHornetParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x102B8
dInsMusHornet_c::dInsMusHornet_c()
    : mState(NULL), mFlyAccel(0.0f), mBaseY(0.0f), mBobHeight(0.0f), mBobAngle(0), mFlyTime(0), mWaitTime(0),
      mWobbleAngle(0), mRoll(0) {}

// 0x1033C
dInsMusHornet_c::~dInsMusHornet_c() {}

// 0x10394
BOOL dInsMusHornet_c::create(nw4r::g3d::ResFile &res) {
    dInsMusBase_c::create(res);
    mBaseY = mPos.y;
    mBgRadius = 1.5f * mShadowSize;
    return TRUE;
}

// 0x103E0
BOOL dInsMusHornet_c::draw() {
    dInsMusBase_c::draw();
    return TRUE;
}

// 0x10404
void dInsMusHornet_c::calcMtx() {
    setAxisMtx();
}

// 0x10408
void dInsMusHornet_c::init() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y = getGroundY() + param->mOffset.y;
    mPos.z += param->mOffset.z;
    mBaseY = mPos.y;
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0x104A0
void dInsMusHornet_c::initState() {
    initFly();
}

// 0x104A4
void dInsMusHornet_c::executeState() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    (this->*mState)();
}

// 0x104E4
void dInsMusHornet_c::initFly() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    mTimer = 0;
    mState = &dInsMusHornet_c::stateFly;
    changeAnm(mAnmChrWait, mAnmVisWait);
    mState = &dInsMusHornet_c::stateFly; // set again, needed to match
    getFlyTarget(&mTargetPos);
    mFlyTime = (int)((mTargetPos - mPos).xzLen() / param->mFlySpeed) + 1;
    mFlyAccel = cM::rndRange(param->mFlyAccel * param->mFlyAccelMinRate, param->mFlyAccel);
}

// 0x1061C
void dInsMusHornet_c::initWait() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    mTimer = 0;
    mState = &dInsMusHornet_c::stateWait;
    changeAnm(mAnmChrWait, mAnmVisWait);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
}

// 0x106F8
void dInsMusHornet_c::stateFly() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    mWobbleAngle += mAng::fromDegree(param->mWobbleSpeed);
    mRoll = mWobbleAngle.sin() * mAng::fromDegree(param->mRollMax);
    fly(mFlyAccel, param->mFlyBrake, param->mFlyTurnRate, mRoll);
    bob();
    if (mFlyTime < mTimer) {
        if (0.05f < cM::rnd()) {
            initFly();
        } else {
            initWait();
        }
    }
}

// 0x10828
void dInsMusHornet_c::stateWait() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    mSpeed *= param->mWaitBrake;
    mPos += mSpeed;
    bob();
    if (mWaitTime < mTimer) {
        initFly();
    }
}

// 0x108C4
void dInsMusHornet_c::fly(f32 accel, f32 brake, f32 turnRate, mAng roll) {
    flyToTarget(0.0f, accel, brake, turnRate);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    up.rotX(roll);
    up.rotY(mAxis.getAngleY());
    mAxis.setUp(up);
}

// 0x10970
void dInsMusHornet_c::bob() {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    s16 next = mBobAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mAng(mBobAngle).sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0x10A70
mAng dInsMusHornet_c::getFlyAngle() {
    s16 angle = mAngle.y;
    int range = DEG_TO_ANGLE(120); // int temp needed to match
    return angle + (s16)cM::rndCenterF(range);
}

// 0x10AD4
f32 dInsMusHornet_c::getFlyDist() const {
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x10B04
void dInsMusHornet_c::getFlyTarget(mVec3_c *target) {
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusHornetParam_c *param = getHornetParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = pos;
    if (!dInsectUtil::getGroundMovePos(&result, from, to, steps, TRUE, TRUE, -500.0f, 0.0f)) {
        dInsectUtil::getLandPos(&result, mPos, 1, TRUE);
    }
    *target = result;
}
