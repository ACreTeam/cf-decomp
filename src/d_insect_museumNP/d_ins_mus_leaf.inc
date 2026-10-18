// dInsMusLeaf_c: the walking leaf of the museum's insect rooms.

namespace dInsectMuseum {

enum { LEAF_TYPE_NUM = 1 };

dInsMusLeafParam_c sLeafParam[LEAF_TYPE_NUM] = {
    {{-40.0f, -13.0f, -15.0f}, 20.0f, 5.0f, 8.0f, 2.0f, 20.0f, 10.0f, 0.03f, 0.003f, 0.9f, 0.005f, 1.2f, 7.0f, 9.0f,
     64.0f, 100, 17},
};

// 0x14640
int getLeafParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_WALKING_LEAF:
        idx = 0;
        break;
    }
    return idx;
}

// 0x14654
const dInsMusLeafParam_c *getLeafParam(int type) {
    return &sLeafParam[getLeafParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x14684
dInsMusLeaf_c::dInsMusLeaf_c()
    : mState(NULL), mWaitTime(0), mMoveTime(0), mSleepTime(0), mWalkTime(0), mWalkTimer(0) {}

// 0x146F4
dInsMusLeaf_c::~dInsMusLeaf_c() {}

// 0x1474C
BOOL dInsMusLeaf_c::draw() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    dInsMusBase_c::draw();
    mAngle.y = mAxis.getAngleY();
    f32 alpha = 0.5f * (mAlpha / 255.0f);
    mAng angle = mAngle.y;
    dShadow::requestRound(&mPos, 0, &angle, param->mShadowSizeX, 1.0f, alpha, param->mShadowSizeZ, 1.0f);
    return TRUE;
}

// 0x14804
void dInsMusLeaf_c::calcMtx() {
    setAxisMtx();
}

// 0x14808
void dInsMusLeaf_c::init() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS;
    mShadowSize = 0.0f;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSizeZ;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x148D0
void dInsMusLeaf_c::initState() {
    initWait();
}

// 0x148D4
void dInsMusLeaf_c::executeState() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    (this->*mState)();
}

// 0x14914
void dInsMusLeaf_c::initWait() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    mTimer = 0;
    mState = &dInsMusLeaf_c::stateWait;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
}

// 0x149F4
void dInsMusLeaf_c::initWalk() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(frame);
    mState = &dInsMusLeaf_c::stateWalk;
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mMoveTime = (int)(mTargetPos.xzDistTo(mPrevTargetPos) / param->mWalkSpeed) + 1;
}

// 0x14B14
void dInsMusLeaf_c::initSleep() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    mState = &dInsMusLeaf_c::stateSleep;
    mSleepTime = dInsectUtil::secToFrame(10.0f);
}

// 0x14BD4
void dInsMusLeaf_c::stateWait() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
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

// 0x14CA4
void dInsMusLeaf_c::stateWalk() {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
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

// 0x14D58
void dInsMusLeaf_c::stateSleep() {
    if (mSleepTime <= mTimer) {
        initWait();
    }
}

// 0x14D70
mAng dInsMusLeaf_c::getWalkAngle() {
    return mAngle.y.mAngle + (s16)cM::rndCenterF(0x2AAB);
}

// 0x14DBC
f32 dInsMusLeaf_c::getWalkDist() const {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x14DEC
void dInsMusLeaf_c::getWalkTarget(mVec3_c *target) {
    const dInsMusLeafParam_c *param = getLeafParam(mType);
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
