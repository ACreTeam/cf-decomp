// 0x7E28
dFishMusCommonMove_c::dFishMusCommonMove_c() {}

// 0x7E64
dFishMusCommonMove_c::~dFishMusCommonMove_c() {}

// 0x7EBC
void dFishMusCommonMove_c::initSwim() {}

// 0x7EC0
void dFishMusCommonMove_c::initWait() {}

// 0x7EC4
void dFishMusCommonMove_c::initMove() {
    dFishMusData_c *data = &dFishMuseum::sFishData[mFishIdx];
    mNearWall = true;
    mTurnBackRate = 75.0f;
    mpData = data;
    mAnmRate = mpData->mAnmRateFast;
    mPos.y = mpData->mSwimY;
    mSpeed = mpData->mSpeed;
}

// 0x7F0C
void dFishMusCommonMove_c::initNextAction() {
    if (1.0f - mMdl.mNodeRot[1].w > 0.1f) {
        initSwim();
    } else {
        initWait();
    }
}

// 0x7F50
void dFishMusCommonMove_c::resetAccel() {
    mAccel = 0.0f;
}

// 0x7F60
void dFishMusCommonMove_c::accelerate(f32 maxSpeed, f32 accelStep, f32 ratio) {
    if (mSpeed < maxSpeed) {
        sLib::chase(&mAccel, mpData->mSpeed, accelStep);
        sLib::addCalc(&mSpeed, mpData->mSpeed, ratio, mAccel, accelStep);
    }
}

// 0x7FE8
void dFishMusCommonMove_c::setRandomPos() {
    mVec3_c center = mTankMin;
    center.x = 0.5f * (mTankMin.x + mTankMax.x);
    center.z = 0.5f * (mTankMin.z + mTankMax.z);
    mPos.x = cM::rndRange(center.x - 300.0f, center.x + 300.0f);
    mPos.z = cM::rndRange(center.z - 25.0f, center.z + 25.0f);
}

// 0x8088
void dFishMusCommonMove_c::setWaitTimer() {
    mTimer = cM::rndRange(mpData->mWaitTimeMin, mpData->mWaitTimeMax);
}

// 0x80C4
void dFishMusCommonMove_c::setSwimTimer() {
    mTimer = cM::rndRange(mpData->mSwimTimeMin, mpData->mSwimTimeMax);
}

// 0x8100
void dFishMusCommonMove_c::setTargetAngleX() {
    f32 maxPitch = mpData->mPitchDeg;
    if (mPos.y < mpData->mSwimYMin) {
        mTargetAngleX = cM::rndRange<long>(mAng::fromDegree(-maxPitch), mAng::fromDegree(-1.0f));
    } else if (mPos.y > mpData->mSwimYMax) {
        mTargetAngleX = cM::rndRange<long>(mAng::fromDegree(1.0f), mAng::fromDegree(maxPitch));
    } else {
        mTargetAngleX = cM::rndRange<long>(mAng::fromDegree(-maxPitch), mAng::fromDegree(maxPitch));
    }
}

// 0x8234
void dFishMusCommonMove_c::setTargetOfs() {
    if (mNearWall && cM::rndRange(0.0f, 100.0f) < mTurnBackRate) {
        mTargetYaw = cM::rndRange<long>(0x6AAB, 0x7F4A);
    } else {
        mTargetYaw = cM::rndRange<long>(0, mpData->mTurnAngle >> 1);
    }
    if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
        mTargetOfs.rotY(mAngle.y + mTargetYaw);
    } else {
        mTargetOfs.rotY(mAngle.y - mTargetYaw);
    }
}

// 0x8310
void dFishMusCommonMove_c::turnX(s16 scale, f32 maxStep, f32 minStep) {
    sLib::addCalcAngle(&mAngle.x.mAngle, mTargetAngleX, scale, mAng::fromDegree(maxStep), mAng::fromDegree(minStep));
}

// 0x836C
void dFishMusCommonMove_c::turnY(s16 scale, f32 maxStep, f32 minStep) {
    mTargetAngleY = cLib::targetAngleY(mPos, mTargetPos);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngleY, scale, mAng::fromDegree(maxStep), mAng::fromDegree(minStep));
}

// 0x841C
void dFishMusCommonMove_c::turnZ(s16 scale, f32 maxStep, f32 minStep) {
    sLib::addCalcAngle(&mAngle.z.mAngle, mTargetAngleZ, scale, mAng::fromDegree(maxStep), mAng::fromDegree(minStep));
}

// 0x8478
void dFishMusCommonMove_c::calcMove() {
    mVelocity.set(0.0f, 0.0f, mSpeed);
    if (!isBottomFish()) {
        mVelocity.rotX(mAngle.x);
    }
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    mHeadPos = mPos;
    if (mFishIdx == FISH_HARISEN || mFishIdx == FISH_SHIIRA) {
        mHeadPos.z = mPos.z;
    } else {
        mHeadPos.z = mpData->mHitLength / 2.0f + (mPos.z + mMdlOffset.z);
    }
}

// 0x856C
bool dFishMusCommonMove_c::isLurkFish() {
    return isLurkKind() || isSeaBottomKind();
}

// 0x85D4
bool dFishMusCommonMove_c::isBottomFish() {
    return isRiverBottomKind() || isSeaBottomKind();
}

// 0x861C
void dFishMusCommonMove_c::setHitInfo(dFishMusBase_c *other) {
    mHitFishIdx = other->mFishIdx;
    other->getCenterPos(mHitOtherCenter);
    getCenterPos(mHitCenter);
    mHitDir = mHitOtherCenter - mHitCenter;
    mHitOtherVel = other->mVelocity;
    mHitApproach = mVelocity.dot(mHitDir);
    mHitVelDot = mHitOtherVel.dot(mVelocity);
}

// 0x87E4
void dFishMusCommonMove_c::setEscapeAngleY(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    mHitFrames = 0;
    mVec3_c side(1.0f, 0.0f, 0.0f);
    side.rotY(mAngle.y);
    f32 dist = mHitDir.dot(side);
    int angle;
    if (isCornered(other)) {
        angle = cM::rndRange<long>(0x471C, 0x71C7);
    } else {
        angle = 16384.0f * (1.0f - nw4r::math::FAbs(dist) / (mpData->mHitLength + otherData->mHitLength));
    }
    if (dist > 0.0f) {
        mEscapeAngleY = mAngle.y.mAngle - angle;
    } else {
        mEscapeAngleY = angle + mAngle.y.mAngle;
    }
    if (!isLurkFish() && mFishIdx != FISH_NAPOLEON) {
        if (mpData->mAquariumIdx == dAquarium_c::IDX_RIVER) {
            mTurnY.mScale = 15;
            mTurnY.mMaxStep = 4.0f;
            mTurnY.mMinStep = 2.0f;
        } else {
            mTurnY.mScale = 10;
            mTurnY.mMaxStep = 5.0f;
            mTurnY.mMinStep = 3.0f;
        }
    }
}

// 0x898C
bool dFishMusCommonMove_c::isCornered(dFishMusBase_c *other) {
    bool ret = false;
    if (other->isNearWall() && mHitVelDot >= 0.0f) {
        ret = true;
    }
    return ret;
}

// 0x89F0
void dFishMusCommonMove_c::setEscapeAngleX(const dFishMusData_c *otherData, mAng maxAngle) {
    f32 dy = mPos.y - mHitOtherCenter.y;
    f32 height = mpData->mHitHeight + otherData->mHitHeight;
    s16 angle = (height - nw4r::math::FAbs(dy)) * maxAngle / height;
    f32 tankHeight = mTankMax.y - mTankMin.y;
    f32 low = mTankMin.y + 0.25f * tankHeight;
    f32 high = mTankMin.y + 0.75f * tankHeight;
    if (low >= mPos.y || (high >= mPos.y && mPos.y >= mHitOtherCenter.y)) {
        mTargetAngleX = -angle;
    } else if (high < mPos.y || (low < mPos.y && mPos.y < mHitOtherCenter.y)) {
        mTargetAngleX = angle;
    }
    if (mFishIdx != FISH_NAPOLEON) {
        mTurnX.mScale = 10;
        mTurnX.mMaxStep = 1.0f;
        mTurnX.mMinStep = 0.5f;
    }
}

// 0x8AF8
void dFishMusCommonMove_c::setEscapeTarget(const dFishMusData_c *otherData) {
    mTargetPos = mPos;
    mTargetOfs.set(0.0f, 0.0f, 3.0f * otherData->mHitLength);
    mTargetOfs.rotY(mEscapeAngleY);
    mTargetPos += mTargetOfs;
}

// 0x8BA0
void dFishMusCommonMove_c::clampTarget(f32 marginX, f32 marginZ) {
    if (mTargetPos.x < mTankMin.x + marginX) {
        mTargetPos.x = mTankMin.x + marginX;
    } else if (mTargetPos.x > mTankMax.x - marginX) {
        mTargetPos.x = mTankMax.x - marginX;
    }
    if (mTargetPos.z < mTankMin.z + marginZ) {
        mTargetPos.z = mTankMin.z + marginZ;
    } else if (mTargetPos.z > mTankMax.z - marginZ) {
        mTargetPos.z = mTankMax.z - marginZ;
    }
}

// 0x8C04
bool dFishMusCommonMove_c::isHitByDangerous(dFishMusBase_c *other) {
    if (mHitFishIdx == FISH_PIRANIA || mHitFishIdx == FISH_HARISEN) {
        if (static_cast<dFishMusBaseMove_c *>(other)->mMode == dFishMusBaseMove_c::MODE_DANGEROUS) {
            return true;
        }
        return false;
    }
    return false;
}

// 0x8C3C
s16 dFishMuseum::degToAng(f32 deg) {
    return deg * mAng::DegreeToAngleCoefficient;
}
