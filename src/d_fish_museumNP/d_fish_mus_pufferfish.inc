// 0x1165C
dFishMusPufferfish_c::dFishMusPufferfish_c() {}

// 0x11698
dFishMusPufferfish_c::~dFishMusPufferfish_c() {}

// 0x116F0
BOOL dFishMusPufferfish_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 3, 1, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    mAnm.setRate(0.0f);
    return TRUE;
}

// 0x11800
BOOL dFishMusPufferfish_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnm();
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

// 0x118C0
void dFishMusPufferfish_c::init() {
    mState = NULL;
    mAccelStep = 0.01f;
    mSwimThroughRate = 60.0f;
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.2f;
    mTurnX.mMinStep = 0.1f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    mFrame = 0.0f;
    mLoopStart = 0.0f;
    mLoopEnd = 20.0f;
    mPuffing = false;
    mPuffed = false;
    mPuffLoops = 0;
    mAnm.setFrame(0.0f);
    mMode = MODE_NORMAL;
    initMove();
    setTankArea();
    dFishMusCommonMove_c::initNextAction();
}

// 0x11998
void dFishMusPufferfish_c::initWait() {
    setWaitTimer();
    mState = &dFishMusBaseMove_c::stateWait;
}

// 0x119E0
void dFishMusPufferfish_c::initSwim() {
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
    mState = &dFishMusPufferfish_c::stateSwim;
}

// 0x11AE8
void dFishMusPufferfish_c::stateSwim() {
    accelerate(mpData->mSpeed, mAccelStep, 50.0f);
    turnX(mTurnX.mScale, mTurnX.mMaxStep, mTurnX.mMinStep);
    turnY(mTurnY.mScale, mTurnY.mMaxStep, mTurnY.mMinStep);
    if (mPos.xzDistTo(mTargetPos) < 15.0f || mTimer < 0) {
        if (cM::rndRange(0.0f, 100.0f) < mSwimThroughRate) {
            mState = &dFishMusBaseMove_c::stateSwimThrough;
        } else {
            mState = &dFishMusBaseMove_c::stateStop;
        }
    }
}

// 0x11BF0
void dFishMusPufferfish_c::calcAnm() {
    f32 frame = mAnm.getFrame();
    if (mPuffing && mPuffed && mPuffLoops == 1) {
        f32 ofs = frame - mLoopStart;
        mLoopStart = 60.0f;
        mLoopEnd = 80.0f;
        mPuffing = false;
        mFrame = mLoopStart + ofs;
    }
    if (mPuffing && frame > mLoopEnd && !mPuffed) {
        f32 ofs = frame - mLoopEnd;
        mLoopStart = 40.0f;
        mLoopEnd = 60.0f;
        mPuffed = true;
        mFrame = mLoopStart + ofs;
        mPuffLoops = 10;
    }
    if (!mPuffing && mPuffed && frame < mpData->mAnmRateFast) {
        mLoopStart = 0.0f;
        mLoopEnd = 20.0f;
        mFrame = frame;
        mPuffed = false;
        mMode = MODE_NORMAL;
    }
    if (frame > mLoopEnd) {
        if (mPuffLoops != 0) {
            mPuffLoops--;
        }
        mFrame = mLoopStart + (frame - mLoopEnd);
    }
    mAnm.setFrame(mFrame);
    mFrame += mpData->mAnmRateFast;
}

// 0x11D68
void dFishMusPufferfish_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0x11DA0
void dFishMusPufferfish_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (isThreat(other)) {
        int otherIdx = other->mFishIdx;
        if (mHitFishIdx != otherIdx || mHitFrames > 13) {
            setHitInfo(other);
            escape(other, &dFishMuseum::sFishData[otherIdx]);
            mTurnX.mScale = 10;
            mTurnX.mMaxStep = 5.0f;
            mTurnX.mMinStep = 3.0f;
            if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
                mState = &dFishMusPufferfish_c::stateSwim;
                mSwimThroughRate = 100.0f;
            }
        }
    }
    dFishMusBase_c::onHitFish(other, push);
    if (!mPuffing && 0.0f < mAnm.getFrame() && mAnm.getFrame() < 20.0f) {
        mLoopStart = 20.0f;
        mLoopEnd = 40.0f;
        mFrame = 20.0f;
        mPuffing = true;
        mPuffed = false;
        mMode = MODE_PUFFED;
    }
}

// 0x11F00
bool dFishMusPufferfish_c::isThreat(dFishMusBase_c *other) {
    int idx = other->mFishIdx;
    return idx == FISH_ANKOU || idx == FISH_MAGURO || idx == FISH_KAJIKI || idx == FISH_EI || idx == FISH_SHUMOKU ||
           idx == FISH_SAME || idx == FISH_SHIIRA;
}

// 0x11F2C
bool dFishMusPufferfish_c::isCornered(dFishMusBase_c *other) {
    bool ret = false;
    if (mHitApproach >= 0.0f && mHitVelDot >= 0.0f) {
        ret = true;
    }
    return ret;
}
