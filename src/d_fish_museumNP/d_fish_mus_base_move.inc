// 0x65E4
dFishMusBaseMove_c::dFishMusBaseMove_c() {}

// 0x6620
dFishMusBaseMove_c::~dFishMusBaseMove_c() {}

// 0x6678
BOOL dFishMusBaseMove_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnmRate();
    setBottomHeight();
    mTimer--;
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    if (mFishIdx == FISH_ANKOU) {
        mCenterOffset.set(0.0f, 3.0f, data.mCenterOffsetZ);
    } else {
        mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    }
    mMdl.mTrailStep = data.mTrailStep;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x6764
void dFishMusBaseMove_c::initWait() {
    setWaitTimer();
    mState = &dFishMusBaseMove_c::stateWait;
}

// 0x67AC
void dFishMusBaseMove_c::initSwim() {
    setSwimTimer();
    if (mFishIdx == FISH_NAPOLEON) {
        mTurnX.mScale = 30;
        mTurnX.mMaxStep = 0.1f;
        mTurnX.mMinStep = 0.05f;
        mTurnY.mScale = 30;
        mTurnY.mMaxStep = 0.5f;
        mTurnY.mMinStep = 0.2f;
    } else {
        mTurnX.mScale = 20;
        mTurnX.mMaxStep = 0.2f;
        mTurnX.mMinStep = 0.1f;
        mTurnY.mScale = 20;
        mTurnY.mMaxStep = 2.0f;
        mTurnY.mMinStep = 1.0f;
    }
    mTargetPos = mPos;
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 60.0f + 30.0f * rnd);
    if (!isBottomFish()) {
        setTargetAngleX();
    }
    setTargetOfs();
    mTargetPos += mTargetOfs;
    if (mFishIdx == FISH_NAPOLEON && (mPos.x < 230.0f || 760.0f < mPos.x)) {
        mTargetPos.x = cM::rndRange(250.0f, 740.0f);
    }
    resetAccel();
    mState = &dFishMusBaseMove_c::stateSwim;
}

// 0x6938
void dFishMusBaseMove_c::stateSwim() {
    accelerate(mpData->mSpeed, mAccelStep, 0.95f);
    turn();
    if (mPos.xzDistTo(mTargetPos) < 15.0f || mTimer < 0) {
        if (cM::rndRange(0.0f, 100.0f) < mSwimThroughRate) {
            mState = &dFishMusBaseMove_c::stateSwimThrough;
        } else {
            mState = &dFishMusBaseMove_c::stateStop;
        }
        mSwimThroughRate = 60.0f;
        if (isFishKind(FISH_NAPOLEON) || isFishKind(FISH_MINOKASAGO)) {
            mSwimThroughRate = 30.0f;
        } else if (isFishKind(FISH_AYU)) {
            mSwimThroughRate = 80.0f;
        }
    }
}

// 0x6A58
void dFishMusBaseMove_c::setBottomHeight() {
    if (mFishIdx == FISH_ANKOU || mFishIdx == FISH_SHIIRA) {
        mPos.y = mpData->mSwimY;
    }
}

// 0x6A7C
void dFishMusBaseMove_c::addPos(const mVec3_c &push) {
    mVec3_c v = push;
    if (isBottomFish()) {
        v.y = 0.0f;
    }
    dFishMusBase_c::addPos(v);
    mMdl.shiftTrail(mPush);
}

// 0x6AEC
void dFishMusBaseMove_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mHitFishIdx != other->mFishIdx || mHitFrames > 13) {
        hitFish(other);
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0x6B64
void dFishMusBaseMove_c::hitFish(dFishMusBase_c *other) {
    const dFishMusData_c *otherData = &dFishMuseum::sFishData[other->mFishIdx];
    setHitInfo(other);
    if (otherData->mAquariumIdx == dAquarium_c::IDX_RIVER) {
        hitFishRiver(other, otherData);
    } else if (otherData->mAquariumIdx == dAquarium_c::IDX_POND) {
        hitFishPond(other, otherData);
    } else if (otherData->mAquariumIdx == dAquarium_c::IDX_SEA) {
        hitFishSea(other, otherData);
    }
}

// 0x6C0C
void dFishMusBaseMove_c::hitFishRiver(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    if (mHitApproach >= 0.0f && canEscape()) {
        escape(other, otherData);
    } else if (mHitFishIdx == FISH_UNAGI) {
        setEscapeAngleY(other, otherData);
        setEscapeTarget(otherData);
        mHitFrames = -50;
        if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
            startSwim();
        }
    } else if (mHitFishIdx == FISH_ITOU || mHitFishIdx == FISH_SAKE) {
        mHitFrames = 0;
        setEscapeAngleX(otherData, mAng(2002));
        if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
            startSwim();
        }
    }
}

// 0x6D94
void dFishMusBaseMove_c::hitFishPond(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    if (mHitFishIdx == FISH_WAKASAGI) {
        return;
    }
    if (mHitApproach >= 0.0f) {
        escape(other, otherData);
        return;
    }
    if ((mHitFishIdx == FISH_BLACK && mHitOtherCenter.y < 35.0f) || mHitFishIdx == FISH_KING ||
        mHitFishIdx == FISH_GA || mHitFishIdx == FISH_PIRARUKU) {
        mHitFrames = 0;
        escapeUnder(otherData);
    } else if (mHitFishIdx == FISH_BLACK && mHitOtherCenter.y > 35.0f) {
        mHitFrames = 0;
        setEscapeAngleX(otherData, mAng(2002));
    }
    if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
        startSwim();
    }
}

// 0x6ECC
void dFishMusBaseMove_c::hitFishSea(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    if (mHitFishIdx == FISH_HARISEN || mHitFishIdx == FISH_MAGURO || mHitFishIdx == FISH_KAJIKI ||
        mHitFishIdx == FISH_SHUMOKU || mHitFishIdx == FISH_SAME) {
        escapeFast(other, otherData);
    } else if (mHitFishIdx == FISH_UTSUBO) {
        mHitFrames = 0;
        setEscapeAngleX(otherData, mAng(2002));
        setEscapeTarget(otherData);
        if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
            startSwim();
        }
    } else if ((mHitApproach >= 0.0f && canEscape()) || mHitFishIdx == FISH_EI) {
        escape(other, otherData);
    }
}

// 0x701C
bool dFishMusBaseMove_c::canEscape() {
    if (isLurkFish()) {
        if (mHitFishIdx == FISH_DONKO || mHitFishIdx == FISH_DOJOU || mHitFishIdx == FISH_NAMAZU ||
            mHitFishIdx == FISH_RAIGYO || mHitFishIdx == FISH_ANKOU || mHitFishIdx == FISH_SHIIRA) {
            return true;
        }
        return false;
    }
    return true;
}

// 0x7098
bool dFishMusBaseMove_c::isCornered(dFishMusBase_c *other) {
    if (mHitFishIdx == FISH_UNAGI || mHitFishIdx == FISH_SHUMOKU || mHitFishIdx == FISH_SAME) {
        return true;
    }
    bool ret = false;
    if (other->isNearWall() && mHitVelDot >= 0.0f) {
        ret = true;
    }
    return ret;
}

// 0x711C
void dFishMusBaseMove_c::escapeUnder(const dFishMusData_c *otherData) {
    f32 height = mpData->mHitHeight + 2.0f * otherData->mHitHeight;
    f32 dist;
    if (mPos.y >= mHitOtherCenter.y) {
        dist = nw4r::math::FAbs(mPos.y - mHitOtherCenter.y);
    } else {
        dist = otherData->mHitHeight + nw4r::math::FAbs(mPos.y - mHitOtherCenter.y);
    }
    mTargetAngleX = -(s16)(2002.0f * (height - dist) / height);
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 1.0f;
    mTurnX.mMinStep = 0.5f;
}

// 0x71B0
void dFishMusBaseMove_c::escape(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    setEscapeAngleY(other, otherData);
    if (!isLurkFish()) {
        setEscapeAngleX(otherData, mAng(2002));
    }
    setEscapeTarget(otherData);
    if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
        startSwim();
    }
}

// 0x727C
void dFishMusBaseMove_c::escapeFast(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    setEscapeAngleY(other, otherData);
    if (mHitFishIdx == FISH_HARISEN || mHitFishIdx == FISH_MAGURO || mHitFishIdx == FISH_KAJIKI) {
        setEscapeAngleX(otherData, mAng(0xEEF));
        mSpeed = 1.2f * mpData->mSpeed;
    } else {
        setEscapeAngleX(otherData, mAng(0x160B));
        mSpeed = 1.5f * mpData->mSpeed;
    }
    setEscapeTarget(otherData);
    if (mState == &dFishMusBaseMove_c::stateStop || mState == &dFishMusBaseMove_c::stateWait) {
        startSwim();
    }
}

// 0x73A0
void dFishMusBaseMove_c::init() {
    mState = NULL;
    mAccelStep = 0.01f;
    mSwimThroughRate = 60.0f;
    setTankArea();
    if (isFishKind(FISH_NAPOLEON)) {
        mTurnX.mScale = 30;
        mTurnX.mMaxStep = 0.1f;
        mTurnX.mMinStep = 0.05f;
        mTurnY.mScale = 30;
        mTurnY.mMaxStep = 0.5f;
        mTurnY.mMinStep = 0.2f;
        setRandomPos();
    } else {
        mTurnX.mScale = 20;
        mTurnX.mMaxStep = 0.2f;
        mTurnX.mMinStep = 0.1f;
        mTurnY.mScale = 20;
        mTurnY.mMaxStep = 2.0f;
        mTurnY.mMinStep = 1.0f;
    }
    if (isFishKind(FISH_DONKO) || isFishKind(FISH_DOJOU) || isFishKind(FISH_NAMAZU)) {
        mPos.x = cM::rndRange(235.0f, 355.0f);
    } else if (isFishKind(FISH_NIGOI)) {
        mMdl.mBendRate = 0.2f;
    } else if (isFishKind(FISH_AYU)) {
        mSwimThroughRate = 80.0f;
    } else if (isFishKind(FISH_NAPOLEON) || isFishKind(FISH_MINOKASAGO)) {
        mAccelStep = 0.007f;
        mTurnBackRate = 70.0f;
        mSwimThroughRate = 30.0f;
    } else if (isFishKind(FISH_ANKOU)) {
        mTurnBackRate = 50.0f;
        setRandomPos();
    } else if (isFishKind(FISH_SHIIRA)) {
        mTurnBackRate = 50.0f;
        setRandomPos();
        mMdl.mBendRate = 0.03f;
    }
    mHitFrames = 10;
    mInHeightRange = true;
    initMove();
    initNextAction();
}

// 0x7558
void dFishMusBaseMove_c::initNextAction() {
    if (1.0f - mMdl.mNodeRot[1].w > 0.1f || (mFishIdx == FISH_SHIIRA && 1.0f - mMdl.mNodeRot[2].w > 0.05f)) {
        initSwim();
    } else {
        initWait();
    }
}

// 0x75BC
void dFishMusBaseMove_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.03f, 0.001f);
    if (mTimer < 0) {
        initSwim();
    }
}

// 0x761C
void dFishMusBaseMove_c::turn() {
    checkHeight();
    if (!isBottomFish()) {
        turnX(mTurnX.mScale, mTurnX.mMaxStep, mTurnX.mMinStep);
    }
    if (isLurkFish()) {
        turnY(10, 0.4f, 0.1f);
    } else {
        turnY(mTurnY.mScale, mTurnY.mMaxStep, mTurnY.mMinStep);
    }
}

// 0x76B0
void dFishMusBaseMove_c::stateSwimThrough() {
    turn();
    if (mPos.xzDistTo(mTargetPos) < 3.0f || mTimer < 0) {
        mNearWall = isNearWall();
        initSwim();
    }
}

// 0x7740
void dFishMusBaseMove_c::stateStop() {
    sLib::addCalc0(&mSpeed, 0.99f, 0.005f);
    if (mSpeed < 0.001f) {
        initNextAction();
    }
    checkHeight();
}

// 0x77A8
void dFishMusBaseMove_c::checkHeight() {
    if (mInHeightRange) {
        if (mPos.y > mpData->mSwimYMax || mpData->mSwimYMin > mPos.y) {
            mTargetAngleX = 0;
            mInHeightRange = false;
        }
    } else if (!mInHeightRange) {
        if (mpData->mSwimYMax > mPos.y && mPos.y > mpData->mSwimYMin) {
            mInHeightRange = true;
        }
    }
}

// 0x7814
void dFishMusBaseMove_c::calcAnmRate() {
    if (mSpeed < 0.05f || isNearWall()) {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.1f, 0.1f);
    } else {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.5f, 0.23f);
    }
    if (isLurkFish()) {
        if (isNearWall() || mSpeed < 0.005f) {
            mAnmRate = mpData->mAnmRateSlow;
        } else {
            mAnmRate = mpData->mAnmRateFast;
        }
    }
    mAnm.setRate(mAnmRate);
}
