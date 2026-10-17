// 0x8C60
dFishMusCuttlefish_c::dFishMusCuttlefish_c() {}

// 0x8C9C
dFishMusCuttlefish_c::~dFishMusCuttlefish_c() {}

// 0x8CF4
BOOL dFishMusCuttlefish_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.m3d::smdl_c::create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0x8DE4
BOOL dFishMusCuttlefish_c::execute() {
    swim();
    calcMove();
    calcPulse();
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x8E90
void dFishMusCuttlefish_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    mPos.y = mpData->mSwimY;
    mPulsePeriod = 100.0f;
    mTurnX.mScale = 30;
    mTurnX.mMaxStep = 0.8f;
    mTurnX.mMinStep = 0.3f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    initSwim();
}

// 0x8F3C
void dFishMusCuttlefish_c::setRollAngle() {
    if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
        mRollAngle = 0x2000;
    } else {
        mRollAngle = -0x2000;
    }
}

// 0x8FA0
void dFishMusCuttlefish_c::initSwim() {
    mTurnX.mScale = 30;
    mTurnX.mMaxStep = 0.8f;
    mTurnX.mMinStep = 0.3f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    setRollAngle();
    setPulseSpeed();
    mVec3_c target;
    do {
        target.x = cM::rndRange(mTankMin.x + 20.0f, mTankMax.x - 20.0f);
        target.y = cM::rndRange(mpData->mSwimYMin, mpData->mSwimYMax);
        target.z = cM::rndRange(mTankMin.z + 20.0f, mTankMax.z - 20.0f);
    } while (mTargetPos.distTo(target) < 100.0f);
    mTargetPos = target;
}

// 0x90B0
void dFishMusCuttlefish_c::setPulseSpeed() {
    mPulseSpeed = mpData->mSpeed + cM::rndRange(-0.2f, 0.35f);
}

// 0x90FC
void dFishMusCuttlefish_c::swim() {
    if (mTargetPos.distTo(mPos) < 10.0f) {
        initSwim();
    }
    if (0.0f < mPulse && mPulse < 20.0f) {
        mSpeed = mPulseSpeed / 5.0f;
    } else if (19.5f < mPulse && mPulse < 80.0f) {
        sLib::chase(&mAccel, mPulseSpeed, 0.2f);
        sLib::addCalc(&mSpeed, mPulseSpeed, 0.2f, 0.3f, 0.1f);
        mTargetAngleZ = mRollAngle;
        turnZ(30, 0.2f, 0.1f);
    } else {
        sLib::addCalc0(&mSpeed, 0.99f, 0.03f);
        mTargetAngleZ = mRollAngle;
        turnZ(30, 0.2f, 0.1f);
    }
    mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
    turnX(30, 0.8f, 0.3f);
    turnY(mTurnY.mScale, mTurnY.mMaxStep, mTurnY.mMinStep);
}

// 0x925C
void dFishMusCuttlefish_c::calcMove() {
    mVelocity.set(0.0f, 0.0f, mSpeed);
    mVelocity.rotZ(mAngle.z);
    mVelocity.rotX(mAngle.x);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
}

// 0x9308
void dFishMusCuttlefish_c::calcPulse() {
    mPulse += mpData->mAnmRateFast;
    if (mPulse > mPulsePeriod) {
        mPulse -= mPulsePeriod;
        setPulseSpeed();
    }
}

// 0x9338
void dFishMusCuttlefish_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0x9370
void dFishMusCuttlefish_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    int otherIdx = other->mFishIdx;
    if (mHitFishIdx != otherIdx || mHitFrames > 50) {
        setHitInfo(other);
        escape(other, &dFishMuseum::sFishData[otherIdx]);
        clampTarget(18.0f, 10.0f);
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0x9424
void dFishMusCuttlefish_c::onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push) {
    if (!push.isZero()) {
        initSwim();
    }
    dFishMusBase_c::onHitObj(obj, push);
}

// 0x94BC
bool dFishMusCuttlefish_c::isCornered(dFishMusBase_c *other) {
    return true;
}

// 0x94C4
void dFishMusCuttlefish_c::setEscapeHeight(const dFishMusData_c *otherData) {
    f32 tankHeight = mTankMax.y - mTankMin.y;
    f32 low = mTankMin.y + 0.25f * tankHeight;
    f32 high = mTankMin.y + 0.75f * tankHeight;
    if (low >= mPos.y || (high >= mPos.y && mPos.y >= mHitOtherCenter.y)) {
        mTargetPos.y += 72.0f;
    } else if (high < mPos.y || (low < mPos.y && mPos.y < mHitOtherCenter.y)) {
        mTargetPos.y -= 72.0f;
    }
    mTurnX.mScale = 10;
    mTurnX.mMaxStep = 1.0f;
    mTurnX.mMinStep = 0.5f;
}

// 0x957C
void dFishMusCuttlefish_c::escape(dFishMusBase_c *other, const dFishMusData_c *otherData) {
    setEscapeAngleY(other, otherData);
    setEscapeHeight(otherData);
    mTargetPos = mPos;
    mTargetOfs.set(0.0f, 0.0f, otherData->mHitLength * cM::rndRange(10.0f, 20.0f));
    mTargetOfs.rotY(mEscapeAngleY);
    mTargetPos += mTargetOfs;
    if (mTankMin.x + 20.0f > mTargetPos.x || mTargetPos.x > mTankMax.x - 20.0f ||
        mTankMin.z + 15.0f > mTargetPos.z || mTargetPos.z > mTankMax.z - 15.0f) {
        mTargetPos.x = cM::rndRange(mTankMin.x + 20.0f, mTankMax.x - 20.0f);
        mTargetPos.z = cM::rndRange(mTankMin.z + 15.0f, mTankMax.z - 15.0f);
    }
}
