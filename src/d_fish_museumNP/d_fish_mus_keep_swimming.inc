// 0xD7A4
dFishMusKeepSwimming_c::dFishMusKeepSwimming_c() {}

// 0xD7E0
dFishMusKeepSwimming_c::~dFishMusKeepSwimming_c() {}

// 0xD838
BOOL dFishMusKeepSwimming_c::create(nw4r::g3d::ResFile res) {
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

// 0xD938
BOOL dFishMusKeepSwimming_c::execute() {
    (this->*mState)();
    calcMove();
    mAnm.setRate(mpData->mAnmRateFast);
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xD9F4
void dFishMusKeepSwimming_c::init() {
    initMove();
    setTankArea();
    mLeftX = mTankMin.x + 95.0f;
    mEscaped = false;
    mTurnXScale = 20;
    mTurnXMaxStep = 0.2f;
    mTurnYMaxStep = 2.0f;
    mBrakeStep = 0.002f;
    _45C = 60;
    if (mFishIdx == FISH_SAME) {
        mRightX = mTankMax.x - 95.0f;
        mCornerOfsX = 30.0f;
        mHeightOfs = 15.0f;
        mBrakeDist = 130.0f;
        mAccelRatio = 0.2f;
        mRollDeg = 60.0f;
        mRollBackStep = 0.3f;
        mRollDist = 100;
    } else if (mFishIdx == FISH_SHUMOKU) {
        mRightX = mTankMax.x - 95.0f;
        mCornerOfsX = 70.0f;
        mHeightOfs = 22.5f;
        mBrakeDist = 100.0f;
        mAccelRatio = 0.1f;
        mRollDeg = 70.0f;
        mRollBackStep = 0.5f;
        mRollDist = 80;
    } else {
        mRightX = mTankMax.x - 200.0f;
        mLeftX = mTankMin.x + 200.0f;
        mPos.x = (mTankMin.x + mTankMax.x) / 2.0f;
        if (mFishIdx == FISH_MAGURO) {
            mHeightOfs = 10.0f;
            mTurnXMaxStep = 0.7f;
            mBrakeDist = 100.0f;
            mBrakeStep = 0.008f;
            mAccelRatio = 0.2f;
            mTurnYMaxStep = 4.0f;
        } else if (mFishIdx == FISH_KAJIKI) {
            mHeightOfs = 20.0f;
            mTurnXScale = 30;
            mTurnXMaxStep = 0.5f;
            mBrakeDist = 80.0f;
            mBrakeStep = 0.006f;
            mAccelRatio = 0.25f;
            mTurnYMaxStep = 3.0f;
        } else if (mFishIdx == FISH_AJI) {
            mRightX = mTankMax.x - 300.0f;
            mLeftX = mTankMin.x + 300.0f;
            mHeightOfs = 15.0f;
            mTurnXScale = 30;
            mTurnXMaxStep = 0.5f;
            mBrakeDist = 60.0f;
            mBrakeStep = 0.003f;
            mAccelRatio = 0.25f;
            mTurnYMaxStep = 3.0f;
            _45C = 60;
        }
    }
    mPoint0Pos.set(mRightX, mpData->mSwimY - mHeightOfs, mTankMin.z + 23.0f);
    mPoint2Pos.set(mRightX - 30.0f, mpData->mSwimY + mHeightOfs, mTankMax.z - 23.0f);
    mPoint5Pos.set(mLeftX + 30.0f, mpData->mSwimY + mHeightOfs, mTankMin.z + 23.0f);
    mPoint3Pos.set(mLeftX, mpData->mSwimY - mHeightOfs, mTankMax.z - 23.0f);
    mLap = 0;
    initSwim();
    mState = &dFishMusKeepSwimming_c::stateSwim;
}

// 0xDCC0
void dFishMusKeepSwimming_c::setTargetRightLow() {
    if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
        mNextPoint = 2;
        mTargetPos = mPoint2Pos;
    } else {
        mNextPoint = 0;
        mTargetPos = mPoint0Pos;
    }
    mTargetPos.x = mRightX;
    mTargetPos.y = mpData->mSwimY - mHeightOfs;
}

// 0xDD70
void dFishMusKeepSwimming_c::setTargetLeftLow() {
    if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
        mNextPoint = 3;
        mTargetPos = mPoint3Pos;
    } else {
        mNextPoint = 5;
        mTargetPos = mPoint5Pos;
    }
    mTargetPos.x = mLeftX;
    mTargetPos.y = mpData->mSwimY - mHeightOfs;
}

// 0xDE20
void dFishMusKeepSwimming_c::setTargetRightHigh() {
    if (mNextPoint == 2) {
        mTargetPos = mPoint0Pos;
    } else {
        mTargetPos = mPoint2Pos;
    }
    mTargetPos.x = mRightX - mCornerOfsX;
    mTargetPos.y = mpData->mSwimY + mHeightOfs;
}

// 0xDE88
void dFishMusKeepSwimming_c::setTargetLeftHigh() {
    if (mNextPoint == 3) {
        mTargetPos = mPoint5Pos;
    } else {
        mTargetPos = mPoint3Pos;
    }
    mTargetPos.x = mLeftX + mCornerOfsX;
    mTargetPos.y = mpData->mSwimY + mHeightOfs;
}

// 0xDEF0
void dFishMusKeepSwimming_c::initSwim() {
    int lap = mLap % 4;
    if (lap == 0) {
        setTargetRightLow();
        mLap = 0;
    } else if (lap == 1) {
        setTargetRightHigh();
    } else if (lap == 2) {
        setTargetLeftLow();
    } else {
        setTargetLeftHigh();
    }
    mLap++;
}

// 0xDF74
void dFishMusKeepSwimming_c::brake() {
    if (mSpeed > 1.0f) {
        sLib::addCalc0(&mSpeed, 0.99f, mBrakeStep);
    }
}

// 0xDFA0
void dFishMusKeepSwimming_c::setTargetRoll() {
    if (mNextPoint == 0 || mNextPoint == 3) {
        mTargetAngleZ = mAng::fromDegree(mRollDeg);
    } else {
        mTargetAngleZ = mAng::fromDegree(-mRollDeg);
    }
}

// 0xE008
void dFishMusKeepSwimming_c::roll() {
    if (mAngle.z.mAngle > mAng::fromDegree(mRollDeg)) {
        mAngle.z = mAng::fromDegree(mRollDeg);
    } else if (mAngle.z.mAngle < mAng::fromDegree(-mRollDeg)) {
        mAngle.z = mAng::fromDegree(-mRollDeg);
    } else if (nw4r::math::FAbs(mTargetPos.y - (mpData->mSwimY - mHeightOfs)) < 0.1f &&
               mPos.xzDistTo(mTargetPos) < mRollDist) {
        sLib::chase(&mRollStep, 0.6f, 0.05f);
        setTargetRoll();
        turnZ(20, mRollStep, 0.5f);
    } else if (nw4r::math::FAbs(mTargetPos.y - (mpData->mSwimY + mHeightOfs)) < 0.1f) {
        sLib::chase(&mRollStep, 0.6f, 0.05f);
        setTargetRoll();
        turnZ(20, mRollStep, 0.5f);
    } else if (nw4r::math::FAbs(mTargetPos.y - (mpData->mSwimY - mHeightOfs)) < 0.1f) {
        mTargetAngleZ = mAng::fromDegree(0.0f);
        turnZ(10, mRollBackStep, 0.1f);
        mRollStep = 0.0f;
    }
}

// 0xE1F8
void dFishMusKeepSwimming_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < 12.0f) {
        if (mEscaped) {
            mEscaped = false;
            if (isNearLeft() || (!isNearRight() && isFacingRight())) {
                mLap = 4;
            } else if (isNearRight() || (!isNearLeft() && !isFacingLeft())) {
                mLap = 2;
            }
        }
        initSwim();
    } else {
        if (nw4r::math::FAbs(mTargetPos.y - (mpData->mSwimY - mHeightOfs)) < 0.1f) {
            if (mPos.xzDistTo(mTargetPos) < mBrakeDist) {
                brake();
                resetAccel();
            } else {
                accelerate(mpData->mSpeed, 0.001f, mAccelRatio);
            }
        }
        mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
        turnX(mTurnXScale, mTurnXMaxStep, 0.1f);
        turnY(20, mTurnYMaxStep, 1.0f);
        if (isFishKind(FISH_SAME) || isFishKind(FISH_SHUMOKU)) {
            roll();
        }
    }
}

// 0xE3E0
bool dFishMusKeepSwimming_c::isFacingRight() {
    return mAngle.y.mAngle >= 0 && mAngle.y.mAngle < 0x7FFF;
}

// 0xE400
bool dFishMusKeepSwimming_c::isFacingLeft() {
    return !isFacingRight();
}

// 0xE428
bool dFishMusKeepSwimming_c::isNearRight() {
    return mPoint0Pos.x - 150.0f < mPos.x;
}

// 0xE44C
bool dFishMusKeepSwimming_c::isNearLeft() {
    return mPos.x < mPoint5Pos.x + 150.0f;
}

// 0xE470
void dFishMusKeepSwimming_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0xE4A8
void dFishMusKeepSwimming_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mHitFishIdx != other->mFishIdx || mHitFrames > 60) {
        hitFish(other);
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0xE520
void dFishMusKeepSwimming_c::hitFish(dFishMusBase_c *other) {
    const dFishMusData_c *otherData = &dFishMuseum::sFishData[other->mFishIdx];
    setHitInfo(other);
    if (isFishKind(FISH_AJI)) {
        hitFishAji(other, otherData);
    } else if (isFishKind(FISH_MAGURO) || isFishKind(FISH_KAJIKI)) {
        hitFishBig(other, otherData);
    }
}

// 0xE5B4
void dFishMusKeepSwimming_c::hitFishAji(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    mEscaped = true;
    setEscapeAngleY(other, otherData);
    setEscapeHeight(otherData);
    setEscapeTarget(otherData);
    clampTarget(18.0f, 10.0f);
}

// 0xE624
void dFishMusKeepSwimming_c::hitFishBig(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    if (mHitFishIdx == FISH_HARISEN || mHitFishIdx == FISH_EI || mHitFishIdx == FISH_SHUMOKU ||
        mHitFishIdx == FISH_SAME || (mFishIdx == FISH_MAGURO && mHitFishIdx == FISH_KAJIKI)) {
        mEscaped = true;
        setEscapeAngleY(other, otherData);
        setEscapeHeight(otherData);
        setEscapeTargetFar();
        clampTarget(35.0f, 14.0f);
    }
}

// 0xE6D0
bool dFishMusKeepSwimming_c::isCornered(dFishMusBase_c *other) {
    if (isFishKind(FISH_AJI)) {
        return mHitFishIdx == FISH_MAGURO || mHitFishIdx == FISH_KAJIKI || mHitFishIdx == FISH_EI ||
               mHitFishIdx == FISH_SHUMOKU || mHitFishIdx == FISH_SAME;
    }
    if (isFishKind(FISH_MAGURO) || isFishKind(FISH_KAJIKI)) {
        return (mHitFishIdx == FISH_HARISEN || mHitFishIdx == FISH_EI) ||
               ((mHitFishIdx == FISH_SHUMOKU || mHitFishIdx == FISH_SAME) && mHitVelDot > 0.0f);
    }
    return false;
}

// 0xE788
void dFishMusKeepSwimming_c::setEscapeHeight(const dFishMusData_c *otherData) {
    f32 tankHeight = mTankMax.y - mTankMin.y;
    f32 low = mTankMin.y + 0.25f * tankHeight;
    f32 high = mTankMin.y + 0.75f * tankHeight;
    if (low >= mPos.y || (high >= mPos.y && mPos.y >= mHitOtherCenter.y)) {
        mTargetPos.y += 32.0f;
    } else if (high < mPos.y || (low < mPos.y && mPos.y < mHitOtherCenter.y)) {
        mTargetPos.y -= 32.0f;
    }
    if (mTargetPos.y < mpData->mSwimYMin) {
        mTargetPos.y = mpData->mSwimYMin;
    }
    mTurnXScale = 10;
    mTurnXMaxStep = 1.0f;
    mTurnX.mMinStep = 0.5f;
}

// 0xE858
void dFishMusKeepSwimming_c::setEscapeTargetFar() {
    mTargetPos = mPos;
    mTargetOfs.set(0.0f, 0.0f, 5.0f * mpData->mHitLength);
    mTargetOfs.rotY(mEscapeAngleY);
    mTargetPos += mTargetOfs;
}
