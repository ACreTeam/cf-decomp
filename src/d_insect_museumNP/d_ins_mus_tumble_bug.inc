// dInsMusTumbleBug_c: the dung beetle of the museum's insect rooms, pushing its ball around.

namespace dInsectMuseum {

enum { TUMBLE_BUG_TYPE_NUM = 1 };

dInsMusTumbleBugParam_c sTumbleBugParam[TUMBLE_BUG_TYPE_NUM] = {
    {{60.0f, 1.5f, -25.0f}, 5.0f, 3.0f, 8.0f, 2.0f, 20.0f, 10.0f, 0.08f, 0.003f, 0.96f, 0.5f, 0.8f, 4.0f, 4},
};

// 0xF598
const dInsMusTumbleBugParam_c *getTumbleBugParam(int type) {
    const int types[] = {INSECT_DUNG_BEETLE};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sTumbleBugParam[i];
        }
    }
    return &sTumbleBugParam[0];
}

} // namespace dInsectMuseum

// 0xF5C0
dInsMusTumbleBug_c::dInsMusTumbleBug_c()
    : mState(NULL), mPushSpeed(0.0f), mWaitTime(0), mMoveTime(0), mPushTime(0), mPushTimer(0) {}

// 0xF640
dInsMusTumbleBug_c::~dInsMusTumbleBug_c() {}

// 0xF6A8
BOOL dInsMusTumbleBug_c::create(nw4r::g3d::ResFile &res) {
    dInsMusBase_c::create(res);
    mBall.create(nw4r::g3d::ResFile(dInsectMus_c::sInstance->mBanks[dInsectMus_c::BANK_BALL].getData()),
                 &mAllocator);
    mAngle.set(0, 0, 0);
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mVec3_c ballPos = mPos;
    mVec3_c offset(0.0f, 0.0f, BALL_DIST);
    offset.rotY(mAngle.y);
    ballPos += offset;
    mBall.setPos(ballPos);
    mBgRadius = 15.0f;
    return TRUE;
}

// 0xF7AC
BOOL dInsMusTumbleBug_c::draw() {
    mAngle.y += DEG_TO_ANGLE(180);
    dInsMusBase_c::draw();
    mAngle.y -= DEG_TO_ANGLE(180);
    mBall.draw();
    return TRUE;
}

// 0xF800
BOOL dInsMusTumbleBug_c::doDelete() {
    dInsMusBase_c::doDelete();
    mBall.remove();
    return TRUE;
}

// 0xF838
void dInsMusTumbleBug_c::init() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y = param->mOffset.y + getGroundY();
    mPos.z += param->mOffset.z;
    mShadowSize = param->mShadowSize;
}

// 0xF8B0
void dInsMusTumbleBug_c::initState() {
    initWait();
}

// 0xF8B4
void dInsMusTumbleBug_c::executeState() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    (this->*mState)();
    mBall.calc();
}

// 0xF8FC
void dInsMusTumbleBug_c::initWait() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    mTimer = 0;
    mState = &dInsMusTumbleBug_c::stateWait;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
    mPushTimer = 0;
}

// 0xF9E8
void dInsMusTumbleBug_c::initPush() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    mTimer = 0;
    mState = &dInsMusTumbleBug_c::statePush;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    setWalkTarget();
    int max = dInsectUtil::secToFrame(param->mPushTimeMax);
    int min = dInsectUtil::secToFrame(param->mPushTimeMin);
    mPushTime = cM::rndRange<u16>(min, max);
}

// 0xFAC8
void dInsMusTumbleBug_c::stateWait() {
    mAngle.x = TILT_ANGLE;
    if (mWaitTime <= mTimer) {
        initPush();
    }
}

// 0xFAE8
void dInsMusTumbleBug_c::statePush() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    mCheckBg = false;
    mVec3_c oldPos = mPos;
    mAngle.x = TILT_ANGLE;
    setAnmRate(param->mAnmRate);
    dInsectUtil::moveTo(&mPos, &mAngle.y, mTargetPos, mPushSpeed, param->mTurnDeg);
    mAcch.check(mBgRadius, &mPos, &oldPos, mAngle.y, 0, mBgFlags, TRUE);
    mVec3_c ballPos = mPos;
    mVec3_c offset(0.0f, 0.0f, BALL_DIST);
    offset.rotY(mAngle.y);
    ballPos += offset;
    mBall.roll(ballPos);
    mPushSpeed *= param->mBrake;
    if (mPushTime <= mPushTimer) {
        if (mPushSpeed <= 0.01f) {
            initWait();
        }
    } else if (mMoveTime < mTimer) {
        setWalkTarget();
    } else {
        mPushSpeed += param->mAccel;
        mPushSpeed = mPushSpeed > param->mSpeedMax ? param->mSpeedMax : mPushSpeed;
    }
    mPushTimer++;
}

// 0xFCB0
mAng dInsMusTumbleBug_c::getWalkAngle() {
    return mAngle.y.mAngle + (int)cM::rndCenterF(0x2AAB);
}

// 0xFCFC
f32 dInsMusTumbleBug_c::getWalkDist() const {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0xFD2C
void dInsMusTumbleBug_c::getWalkTarget(mVec3_c *target) {
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}

// 0xFE2C
void dInsMusTumbleBug_c::setWalkTarget() {
    const dInsMusTumbleBugParam_c *param = getTumbleBugParam(mType);
    mTimer = 0;
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mVec3_c diff = mTargetPos - mPrevTargetPos;
    f32 dist = diff.xzLen();
    mMoveTime = (int)(dist / param->mSpeedMax) + 1;
}

// 0xFED8
dInsMusTumbleBug_c::ball_c::ball_c() : mPos(0.0f, 0.0f, 0.0f), mRot(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f)) {}

// 0xFF40
dInsMusTumbleBug_c::ball_c::~ball_c() {}

// 0xFF9C
void dInsMusTumbleBug_c::ball_c::create(nw4r::g3d::ResFile res, mAllocator_c *allocator) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, allocator, 0, 1, NULL);
}

// 0xFFFC
void dInsMusTumbleBug_c::ball_c::calc() {
    mPos.y = RADIUS + dBGCF::getGroundY(&mPos, FALSE);
    mMdl.play();
    mMdl.calc(false);
}

// 0x1005C
void dInsMusTumbleBug_c::ball_c::draw() {
    mMtx_c mtx;
    mMtx_c rotMtx;
    mVec3_c pos = mPos; // unused, needed to match
    dActor_c::makeMtx(&mtx, &mPos, 0);
    ((EGG::Matrix34f *)(nw4r::math::MTX34 *)(rotMtx))->makeQ(mRot);
    PSMTXConcat(mtx, rotMtx, mtx);
    mMdl.setLocalMtx(&mtx);
    mMdl.entry();
}

// 0x100F0
void dInsMusTumbleBug_c::ball_c::remove() {
    mMdl.remove();
}

// 0x10100
void dInsMusTumbleBug_c::ball_c::setPos(const mVec3_c &pos) {
    mPos = pos;
}

// 0x1011C
void dInsMusTumbleBug_c::ball_c::roll(const mVec3_c &pos) {
    mVec3_c oldPos = mPos;
    mPos = pos;
    mVec3_c move = mPos - oldPos;
    mVec3_c up(0.0f, 1.0f, 0.0f);
    mVec3_c axis = up.cross(move);
    axis.normalize();
    f32 angle = move.xzLen() / RADIUS;
    EGG::Quatf rot(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f));
    rot.setAxisRotation(axis, angle);
    EGG::Quatf newRot = rot * mRot;
    EGG::Quatf result = newRot; // extra copy needed to match
    mRot = result;
    mRot.normalise();
}
