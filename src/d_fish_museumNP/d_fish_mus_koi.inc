// 0xE904
dFishMusKoi_c::dFishMusKoi_c() : mJumpNum(0), mJumpTimer(0) {}

// 0xE94C
dFishMusKoi_c::~dFishMusKoi_c() {}

// 0xE9A4
BOOL dFishMusKoi_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 3, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0xEAA4
BOOL dFishMusKoi_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnmRate();
    mTimer--;
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xEB64
void dFishMusKoi_c::init() {
    initMove();
    mState = NULL;
    mAccelStep = 0.01f;
    setTankArea();
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.2f;
    mTurnX.mMinStep = 0.1f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    mTurnBackRate = 70.0f;
    mSwimThroughRate = 30.0f;
    mHitFrames = 10;
    initNextAction();
}

// 0xEC18
void dFishMusKoi_c::initNextAction() {
    if (1.0f - mMdl.mNodeRot[1].w > 0.1f) {
        initSwim();
    } else {
        initWait();
    }
}

// 0xEC5C
void dFishMusKoi_c::initWait() {
    setWaitTimer();
    mState = &dFishMusKoi_c::stateWait;
}

// 0xECA4
void dFishMusKoi_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.03f, 0.001f);
    if (mTimer < 0) {
        if (cM::rndRange(0, 100) < 80) {
            initRise();
        } else {
            initSwim();
        }
    }
}

// 0xED24
void dFishMusKoi_c::initRise() {
    mTargetAngleX = -0x2000;
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.4f;
    mTurnX.mMinStep = 0.3f;
    mVec3_c center = (mTankMax - mTankMin) * 0.5f + mTankMin;
    mTargetAngleY = cM::atan2s(center.x - mPos.x, center.z - mPos.z);
    mJumpNum = cM::rndRange(2, 3);
    resetAccel();
    mState = &dFishMusKoi_c::stateRise;
}

// 0xEE00
void dFishMusKoi_c::stateRise() {
    accelerate(mpData->mSpeed, 0.007f, 50.0f);
    turn();
    if (mAngle.x.mAngle < -0xB61 && mPos.y > mTankMax.y - 5.0f) {
        if (mTankMin.x + 10.0f < mPos.x && mPos.x < mTankMax.x - 10.0f && mTankMin.z + 10.0f < mPos.z &&
            mPos.z < mTankMax.z - 10.0f) {
            initJump();
        } else {
            initSwim();
            mTargetAngleX = cM::rndRange<long>(0xAAB, 0x11C7);
        }
    }
}

// 0xEEF4
void dFishMusKoi_c::initJump() {
    f32 dist = 2.0f;
    mPrevPos = mPos;
    mTargetPos = mPos;
    mTargetOfs.set(0.0f, 0.0f, dist);
    mTargetOfs.rotX(mAngle.x);
    mTargetAngleX = mAngle.x;
    mTargetOfs.rotY(mAngle.y);
    mTargetAngleY = mAngle.y;
    mTargetPos += mTargetOfs;
    mJumpTimer = 11;
    resetAccel();
    mState = &dFishMusKoi_c::stateJump;
}

// 0xEFF0
void dFishMusKoi_c::stateJump() {
    accelerate(mpData->mSpeed, 0.007f, 50.0f);
    turn();
    if (mJumpTimer <= 0) {
        initFall();
    }
    mJumpTimer--;
}

// 0xF05C
void dFishMusKoi_c::initFall() {
    mTargetPos = mPrevPos;
    mSpeed = -(0.25f * mpData->mSpeed);
    mPrevPos = mPos;
    mJumpTimer = 11;
    mState = &dFishMusKoi_c::stateFall;
}

// 0xF0D0
void dFishMusKoi_c::stateFall() {
    sLib::addCalc0(&mSpeed, 0.99f, 0.005f);
    turn();
    dFishMusBase_c::addPos(mPos - mPrevPos);
    mMdl.shiftTrail(mPush);
    mPrevPos = mPos;
    if (mJumpTimer <= 0) {
        if (--mJumpNum > 0) {
            initJump();
        } else {
            mJumpNum = 0;
            initSettle();
        }
    }
    mJumpTimer--;
}

// 0xF19C
void dFishMusKoi_c::initSettle() {
    setWaitTimer();
    mState = &dFishMusKoi_c::stateSettle;
}

// 0xF1E4
void dFishMusKoi_c::stateSettle() {
    sLib::addCalc0(&mSpeed, 0.1f, 0.02f);
    dFishMusBase_c::addPos(mPos - mPrevPos);
    mMdl.shiftTrail(mPush);
    mPrevPos = mPos;
    if (mTimer < 0) {
        initSwim();
    }
}

// 0xF284
void dFishMusKoi_c::initSwim() {
    setSwimTimer();
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.2f;
    mTurnX.mMinStep = 0.1f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    mTargetPos = mPos;
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 60.0f + 30.0f * rnd);
    setTargetAngleX();
    setTargetOfs();
    mTargetPos += mTargetOfs;
    resetAccel();
    mState = &dFishMusKoi_c::stateSwim;
}

// 0xF38C
void dFishMusKoi_c::turn() {
    turnX(mTurnX.mScale, mTurnX.mMaxStep, mTurnX.mMinStep);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngleY, mTurnY.mScale, mAng::fromDegree(mTurnY.mMaxStep),
                       mAng::fromDegree(mTurnY.mMinStep));
}

// 0xF40C
void dFishMusKoi_c::stateStop() {
    sLib::addCalc0(&mSpeed, 0.99f, 0.005f);
    if (mSpeed < 0.001f) {
        initNextAction();
    }
}

// 0xF46C
void dFishMusKoi_c::stateSwim() {
    accelerate(mpData->mSpeed, 0.007f, 50.0f);
    mTargetAngleY = cLib::targetAngleY(mPos, mTargetPos);
    turn();
    if (mPos.xzDistTo(mTargetPos) < 15.0f || mTimer < 0) {
        if (cM::rndRange(0.0f, 100.0f) < mSwimThroughRate) {
            mState = &dFishMusKoi_c::stateSwimThrough;
        } else {
            mState = &dFishMusKoi_c::stateStop;
        }
        mJumpNum = 0;
        mSwimThroughRate = 30.0f;
    }
}

// 0xF574
void dFishMusKoi_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0xF5AC
void dFishMusKoi_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mJumpNum == 0 && (mHitFishIdx != other->mFishIdx || mHitFrames > 13)) {
        hitFish(other);
    }
    dFishMusBase_c::onHitFish(other, push);
}
