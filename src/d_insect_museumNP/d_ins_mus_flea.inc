// dInsMusFlea_c: waits, then jumps to a random point nearby.
namespace dInsectMuseum {

enum { FLEA_TYPE_NUM = 1 };

dInsMusFleaParam_c sFleaParam[FLEA_TYPE_NUM] = {
    {{-50.0f, -13.0f, -50.0f}, 2.0f, 0.5f, 1.0f, 60.0f, 45.0f, -0.165f, 0.06f, 1.0f, 24}, // flea
};

// 0xE088
int getFleaParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_FLEA:
        idx = 0;
        break;
    }
    return idx;
}

// 0xE09C
const dInsMusFleaParam_c *getFleaParam(int type) {
    return &sFleaParam[getFleaParamIdx(type)];
}

} // namespace dInsectMuseum

// 0xE0CC
dInsMusFlea_c::dInsMusFlea_c() : mState(NULL), mWaitTime(0), mJumpTime(0) {}

// 0xE130
dInsMusFlea_c::~dInsMusFlea_c() {}

// 0xE188
void dInsMusFlea_c::init() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mShadowSize = param->mShadowSize;
    mBgRadius = 5.0f;
}

// 0xE1FC
void dInsMusFlea_c::initState() {
    initWait();
}

// 0xE200
void dInsMusFlea_c::executeState() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    (this->*mState)();
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
    }
}

// 0xE25C
void dInsMusFlea_c::initWait() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    mTimer = 0;
    mState = &dInsMusFlea_c::stateWait;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
}

// 0xE300
void dInsMusFlea_c::initJump() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    mTimer = 0;
    mState = &dInsMusFlea_c::stateJump;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    getJumpTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mVec3_c diff = mTargetPos - mPrevTargetPos;
    f32 dist = diff.xzLen();
    mJumpTime = (int)(dist / param->mJumpSpeed) + 1;
    mSpeed.set(0.0f, 0.3f + param->mJumpHeightRate * dist, 0.0f);
}

// 0xE414
void dInsMusFlea_c::stateWait() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    if (mWaitTime <= mTimer) {
        initJump();
    }
}

// 0xE45C
void dInsMusFlea_c::stateJump() {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    mCheckBg = false;
    mVec3_c oldPos = mPos;
    dInsectUtil::easeMoveXZ(&mPos, mPrevTargetPos, mTargetPos, mJumpTime, mTimer);
    mAcch.check(mBgRadius, &mPos, &oldPos, mAngle.y, 0, mBgFlags, TRUE);
    mSpeed.y += param->mGravity;
    mPos.y += mSpeed.y;
    if (isUnderGround() == true) {
        initWait();
    }
}

// 0xE530
mAng dInsMusFlea_c::getJumpAngle() {
    s16 angle = mAngle.y;
    return angle + (int)cM::rndCenterF(0x2AAB);
}

// 0xE578
f32 dInsMusFlea_c::getJumpDist() const {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    return cM::rndRange(param->mJumpDistMin, param->mJumpDistMax);
}

// 0xE5A8
void dInsMusFlea_c::getJumpTarget(mVec3_c *target) {
    const dInsMusFleaParam_c *param = getFleaParam(mType);
    mAng angle = getJumpAngle();
    f32 dist = getJumpDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    result = pos; // dead store, needed to match
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}
