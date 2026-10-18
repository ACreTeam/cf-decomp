// dInsMusStump_c: the longhorn and violin beetles on the museum's tree stumps.

namespace dInsectMuseum {

enum { STUMP_TYPE_NUM = 2 };

dInsMusStumpParam_c sStumpParam[STUMP_TYPE_NUM] = {
    {{9.0f, 1.0f, 3.0f}, 20.0f, 5.0f, 0.02f, 0.002f, 0.9f, 0.02f, 1.5f, 20.0f, 10.0f, 8.0f, 2.0f, 1.2f, 4.0f, 64.0f,
     100, 19}, // longhorn beetle
    {{-7.0f, 1.0f, -6.0f}, 20.0f, 5.0f, 0.02f, 0.002f, 0.9f, 0.01f, 2.0f, 20.0f, 10.0f, 8.0f, 2.0f, 1.2f, 4.0f, 64.0f,
     100, 18}, // violin beetle
};

// 0x13DD4
int getStumpParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_LONGHORN_BEETLE:
        idx = 0;
        break;
    case INSECT_VIOLIN_BEETLE:
        idx = 1;
        break;
    }
    return idx;
}

// 0x13E04
const dInsMusStumpParam_c *getStumpParam(int type) {
    return &sStumpParam[getStumpParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x13E34
dInsMusStump_c::dInsMusStump_c()
    : mState(NULL), mWaitTime(0), mMoveTime(0), mSleepTime(0), mWalkTime(0), mWalkTimer(0) {}

// 0x13EA4
dInsMusStump_c::~dInsMusStump_c() {}

// 0x13EFC
BOOL dInsMusStump_c::draw() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    setScale(param->mScale);
    dInsMusBase_c::draw();
    mAng angle(0);
    dShadow::requestRound(&mPos, dShadow::DRAW_QUAD, &angle, param->mShadowSize, 1.0f, 0.5f, 0.0f, 1.0f);
    return TRUE;
}

// 0x13F8C
void dInsMusStump_c::calcMtx() {
    setAxisMtx();
}

// 0x13F90
void dInsMusStump_c::init() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mShadowSize = param->mShadowSize;
    mBgRadius = param->mShadowSize;
}

// 0x14038
void dInsMusStump_c::initState() {
    initWait();
}

// 0x1403C
void dInsMusStump_c::executeState() {
    (this->*mState)();
}

// 0x14064
void dInsMusStump_c::initWait() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    mTimer = 0;
    mState = &dInsMusStump_c::stateWait;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
}

// 0x14144
void dInsMusStump_c::initWalk() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusStump_c::stateWalk;
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mMoveTime = (int)(mTargetPos.xzDistTo(mPrevTargetPos) / param->mWalkSpeed) + 1;
}

// 0x14260
void dInsMusStump_c::initSleep() {
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    mState = &dInsMusStump_c::stateSleep;
    mSleepTime = dInsectUtil::secToFrame(10.0f);
}

// 0x14318
void dInsMusStump_c::stateWait() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    if (isActiveTime() == true) {
        if (mWaitTime <= mTimer) {
            initWalk();
            int max = dInsectUtil::secToFrame(param->mWalkTimeMax);
            int min = dInsectUtil::secToFrame(param->mWalkTimeMin);
            mWalkTime = cM::rndRange<u16>(min, max);
        }
    } else if (mScare.isScared() == true) {
        initWalk();
        int max = dInsectUtil::secToFrame(3.0f);
        int min = dInsectUtil::secToFrame(2.0f);
        mWalkTime = cM::rndRange<u16>(min, max);
    }
}

// 0x143E8
void dInsMusStump_c::stateWalk() {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    flyToTarget(0.0f, param->mWalkAccel, param->mWalkBrake, param->mWalkTurnRate);
    if (mMoveTime <= mTimer) {
        initWalk();
    } else if (mWalkTime <= mWalkTimer) {
        mWalkTimer = 0;
        if (isActiveTime() == true) {
            initWait();
        } else {
            initSleep();
        }
    }
    mWalkTimer++;
}

// 0x1449C
void dInsMusStump_c::stateSleep() {
    if (mSleepTime <= mTimer) {
        initWait();
    }
}

// 0x144B4
mAng dInsMusStump_c::getWalkAngle() {
    return mAngle.y.mAngle + (s16)cM::rndCenterF(0x2AAB);
}

// 0x14500
f32 dInsMusStump_c::getWalkDist() const {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x14530
void dInsMusStump_c::getWalkTarget(mVec3_c *target) {
    const dInsMusStumpParam_c *param = getStumpParam(mType);
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}
