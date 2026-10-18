// dInsMusStrider_c: the pond skater waits on the pond, then glides to a random point.
namespace dInsectMuseum {

enum { STRIDER_TYPE_NUM = 1 };

dInsMusStriderParam_c sStriderParam[STRIDER_TYPE_NUM] = {
    {{-25.0f, 0.0f, -20.0f}, 1.0f, 0.0f, 0.25f, 120.0f, 40.0f, 0.0f, 6}, // pond_skater
};

// 0xB708
int getStriderParamIdx(int type) {
    int idx = 0;
    static const int typeNum = STRIDER_TYPE_NUM;
    for (int i = 0; i < typeNum; i++) {
        if (type == INSECT_POND_SKATER) {
            idx = i;
            break;
        }
    }
    return idx;
}

// 0xB71C
const dInsMusStriderParam_c *getStriderParam(int type) {
    return &sStriderParam[getStriderParamIdx(type)];
}

} // namespace dInsectMuseum

// 0xB74C
dInsMusStrider_c::dInsMusStrider_c() : mState(NULL), mRipple(true), mWaitTime(0), mMoveTime(0) {}

// 0xB7B8
dInsMusStrider_c::~dInsMusStrider_c() {}

// 0xB810
void dInsMusStrider_c::init() {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mShadowSize = param->mShadowSize;
    mBgRadius = param->mShadowSize;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
}

// 0xB890
void dInsMusStrider_c::initState() {
    initWait();
}

// 0xB894
void dInsMusStrider_c::executeState() {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    (this->*mState)();
}

// 0xB8D4
void dInsMusStrider_c::initWait() {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusStrider_c::stateWait;
    mWaitTime = cM::rndRange<s16>(0, 180);
    if (!isActiveTime()) {
        mWaitTime *= 5.0f;
    }
}

// 0xB9C4
void dInsMusStrider_c::initMove() {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusStrider_c::stateMove;
    getMoveTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mMoveTime = (int)(mTargetPos.xzDistTo(mPrevTargetPos) / param->mMoveSpeed) + 1 + GLIDE_START_FRAME;
    mAngle.y = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mRipple = true;
}

// 0xBAE8
void dInsMusStrider_c::stateWait() {
    if (mWaitTime <= mTimer) {
        initMove();
    }
}

// 0xBB00
void dInsMusStrider_c::stateMove() {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    if (GLIDE_START_FRAME <= mAnmChr.getFrame()) {
        if (mRipple == true) {
            dEffect::create("af_ins_amenbo_hamon", &mPos, &mAngle, NULL);
            mRipple = false;
        }
        dInsectUtil::easeMoveXZ(&mPos, mPrevTargetPos, mTargetPos, (u16)(mMoveTime - GLIDE_START_FRAME), mTimer - GLIDE_START_FRAME);
        if (mMoveTime <= mTimer) {
            initWait();
        }
    }
    mPos.y = getGroundY();
    if (MOVE_END_FRAME <= mAnmChr.getFrame()) {
        setAnmFrame(MOVE_END_FRAME);
        setAnmRate(0.0f);
    }
}

// 0xBBFC
mAng dInsMusStrider_c::getMoveAngle() {
    return mAng(mAngle.y.mAngle + (int)cM::rndCenterF(0x2AAB));
}

// 0xBC48
f32 dInsMusStrider_c::getMoveDist() const {
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    return cM::rndRange<f32>(param->mMoveDistMin, param->mMoveDistMax);
}

// 0xBC78
void dInsMusStrider_c::getMoveTarget(mVec3_c *target) {
    mAng angle = getMoveAngle();
    f32 dist = getMoveDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusStriderParam_c *param = getStriderParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}
