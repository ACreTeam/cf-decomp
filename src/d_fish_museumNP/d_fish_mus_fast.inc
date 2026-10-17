// 0xA9CC
dFishMusFast_c::dFishMusFast_c() {}

// 0xAA08
dFishMusFast_c::~dFishMusFast_c() {}

// 0xAA60
BOOL dFishMusFast_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    int nodeNum = 3;
    if (mFishIdx == FISH_PIKE) {
        nodeNum = 2;
    }
    mMdl.create(mdl, &mAllocator, nodeNum, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0xAB70
BOOL dFishMusFast_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnmRate();
    mTimer--;
    mPointTimer--;
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xAC3C
void dFishMusFast_c::init() {
    initPoints();
    if (mFishIdx == FISH_PIKE || mFishIdx == FISH_ITOU) {
        mNextPoint = 0;
        mPoint = 5;
        mArriveDist = 19.0f;
        mPos.x = mTankMin.x + 55.0f;
        mWaitRate = 10.0f;
        mPointTimer = 400;
        setSwimTimer();
        addOfs(mPoint5Pos, 4.0f, 0.0f, 4.0f);
        mPoint4Pos.x += 5.0f;
        addOfs(mPoint3Pos, 4.0f, 0.0f, -4.0f);
        addOfs(mPoint0Pos, -4.0f, 0.0f, 4.0f);
        mPoint1Pos.x -= 5.0f;
        addOfs(mPoint2Pos, -4.0f, 0.0f, -4.0f);
    } else {
        mNextPoint = 3;
        mPoint = 2;
        mArriveDist = 14.0f;
        mPos.x = mTankMax.x - 55.0f;
        mWaitRate = 0.0f;
        mPointTimer = 230;
    }
    if (mFishIdx == FISH_SAKE) {
        addOfs(mPoint5Pos, -4.0f, 0.0f, -1.0f);
        mPoint4Pos.x -= 5.0f;
        addOfs(mPoint3Pos, -4.0f, 0.0f, 1.0f);
        addOfs(mPoint0Pos, 4.0f, 0.0f, -1.0f);
        mPoint1Pos.x += 5.0f;
        addOfs(mPoint2Pos, 4.0f, 0.0f, 1.0f);
    }
    mPos.z = cM::rndRange(mTankMin.z, mTankMax.z);
    goToPoint(mNextPoint);
    mState = &dFishMusFast_c::stateSwim;
}

// 0xAE4C
void dFishMusFast_c::initWait() {
    setWaitTimer();
    mState = &dFishMusFast_c::stateWait;
}

// 0xAE94
void dFishMusFast_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.95f, 0.005f);
    if (mTimer < 0) {
        setSwimTimer();
        resetAccel();
        mState = &dFishMusFast_c::stateSwim;
    }
}

// 0xAF0C
void dFishMusFast_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < mArriveDist || mPointTimer < 0) {
        // Always true in the original (|| instead of &&), so every fish gets 400; the compares are in the target.
        if (mFishIdx != FISH_PIKE || mFishIdx != FISH_ITOU) {
            mPointTimer = 400;
        } else {
            mPointTimer = 230;
        }
        goToPoint(mNextPoint);
    } else if (mTimer < 0 && 1.0f - mMdl.mNodeRot[1].w < 0.005f && cM::rndRange(0.0f, 100.0f) < mWaitRate) {
        initWait();
    } else {
        accelerate(mpData->mSpeed, 0.0005f, 0.98f);
        mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
        turnX(95, 0.2f, 0.1f);
        turnY(20, 2.0f, 1.0f);
    }
}

// 0xB064
void dFishMusFast_c::goToPoint5() {
    mTargetPos.x = mPoint5Pos.x;
    mTargetPos.z = mPoint5Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 4) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mNextPoint = 0;
        } else {
            mNextPoint = 2;
        }
        setRandomHeight();
    } else {
        mNextPoint = 4;
    }
    mPoint = 5;
}

// 0xB108
void dFishMusFast_c::goToPoint4() {
    mTargetPos.x = mPoint4Pos.x;
    mTargetPos.z = mPoint4Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 5) {
        mNextPoint = 3;
    } else {
        mNextPoint = 5;
    }
    mPoint = 4;
}

// 0xB14C
void dFishMusFast_c::goToPoint3() {
    mTargetPos.x = mPoint3Pos.x;
    mTargetPos.z = mPoint3Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 4) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mNextPoint = 0;
        } else {
            mNextPoint = 2;
        }
        setRandomHeight();
    } else {
        mNextPoint = 4;
    }
    mPoint = 3;
}

// 0xB1F0
void dFishMusFast_c::goToPoint0() {
    mTargetPos.x = mPoint0Pos.x;
    mTargetPos.z = mPoint0Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 1) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mNextPoint = 5;
        } else {
            mNextPoint = 3;
        }
        setRandomHeight();
    } else {
        mNextPoint = 1;
    }
    mPoint = 0;
}

// 0xB294
void dFishMusFast_c::goToPoint1() {
    mTargetPos.x = mPoint1Pos.x;
    mTargetPos.z = mPoint1Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 0) {
        mNextPoint = 2;
    } else {
        mNextPoint = 0;
    }
    mPoint = 1;
}

// 0xB2D8
void dFishMusFast_c::goToPoint2() {
    mTargetPos.x = mPoint2Pos.x;
    mTargetPos.z = mPoint2Pos.z;
    mTargetPos.y = mHeight;
    if (mPoint == 1) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mNextPoint = 5;
        } else {
            mNextPoint = 3;
        }
        setRandomHeight();
    } else {
        mNextPoint = 1;
    }
    mPoint = 2;
}

// 0xB37C
void dFishMusFast_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0xB3B4
void dFishMusFast_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mHitFishIdx != other->mFishIdx || mHitFrames > 150) {
        hitFish(other);
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0xB42C
void dFishMusFast_c::hitFish(dFishMusBase_c *other) {
    const dFishMusData_c *otherData = &dFishMuseum::sFishData[other->mFishIdx];
    setHitInfo(other);
    if (otherData->mAquariumIdx == dAquarium_c::IDX_RIVER) {
        hitFishRiver(otherData);
    } else {
        hitFishPond(other, otherData);
    }
}

// 0xB4AC
void dFishMusFast_c::hitFishRiver(const dFishMusData_c *otherData) {
    if (mHitFishIdx == FISH_ITOU || mHitFishIdx == FISH_SAKE) {
        escape(otherData);
    } else {
        escapeAhead(otherData);
    }
}

// 0xB4C4
void dFishMusFast_c::hitFishPond(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    if (mHitFishIdx == FISH_BLACK || mHitFishIdx == FISH_PIKE || mHitFishIdx == FISH_KING ||
        mHitFishIdx == FISH_AROWANA || mHitFishIdx == FISH_DORADO || isHitByDangerous(other)) {
        escape(otherData);
    }
}

// 0xB53C
void dFishMusFast_c::escape(const dFishMusData_c *otherData) {
    if (isApproaching()) {
        setEscapeYaw(otherData);
        setEscapeTarget(otherData);
        clampTarget(10.0f, 10.0f);
    }
}

// 0xB5A8
bool dFishMusFast_c::isApproaching() {
    if (mHitApproach >= 0.0f) {
        return true;
    }
    return mHitVelDot >= 0.0f;
}

// 0xB5E0
void dFishMusFast_c::setEscapeYaw(const dFishMusData_c *otherData) {
    mHitFrames = 0;
    mVec3_c side(1.0f, 0.0f, 0.0f);
    side.rotY(mAngle.y);
    f32 dist = mHitDir.dot(side);
    s16 angle = 16384.0f * (1.0f - nw4r::math::FAbs(dist) / (mpData->mHitWidth + otherData->mHitWidth));
    if (angle < 0) {
        angle = 910;
    }
    if (dist >= 0.0f) {
        mEscapeAngleY = mAngle.y.mAngle - angle;
    } else {
        mEscapeAngleY = angle + mAngle.y.mAngle;
    }
}

// 0xB6EC
void dFishMusFast_c::escapeAhead(const dFishMusData_c *otherData) {
    if (mHitApproach >= 0.0f) {
        setEscapeYaw(otherData);
        setEscapeTarget(otherData);
        clampTarget(10.0f, 10.0f);
    }
}
