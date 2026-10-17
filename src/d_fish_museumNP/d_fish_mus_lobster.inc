// 0xF630
dFishMusLobster_c::dFishMusLobster_c() {}

// 0xF66C
dFishMusLobster_c::~dFishMusLobster_c() {}

// 0xF6C4
BOOL dFishMusLobster_c::create(nw4r::g3d::ResFile res) {
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

// 0xF7B4
BOOL dFishMusLobster_c::execute() {
    (this->*mState)();
    walk();
    calcAnmRate();
    mTimer--;
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xF85C
void dFishMusLobster_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    mAnmRate = mpData->mAnmRateSlow;
    mPos.y = mpData->mSwimY;
    mWalkTarget.set(0.0f, mpData->mSwimY, 0.0f);
    mSpeed = mpData->mSpeed;
    setRandomPos();
    if (mFishIdx == FISH_ZARIGANI) {
        mWalkMin.set(mTankMin.x + 5.0f, mpData->mSwimY, mTankMin.z + 5.0f);
        mWalkMax.set(mTankMin.x + 72.0f, mpData->mSwimY, mTankMax.z - 5.0f);
        mPos.x = cM::rndRange(mWalkMin.x, mWalkMax.x);
        mPos.z = cM::rndRange(mWalkMin.z, mWalkMax.z);
    } else if (mFishIdx == FISH_LOBSTER) {
        mWalkMin.set(mTankMin.x + 50.0f, mpData->mSwimY, mTankMin.z + 50.0f);
        mWalkMax.set(mTankMax.x - 50.0f, mpData->mSwimY, mTankMax.z - 50.0f);
    }
    initWait();
    mState = &dFishMusLobster_c::stateWait;
}

// 0xF9D8
void dFishMusLobster_c::initWait() {
    setWaitTimer();
    mState = &dFishMusLobster_c::stateWait;
}

// 0xFA20
void dFishMusLobster_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.98f, 0.001f);
    if (mTimer < 0) {
        initSwim();
    }
}

// 0xFA80
void dFishMusLobster_c::initSwim() {
    setSwimTimer();
    mWalkTarget.x = cM::rndRange(mWalkMin.x, mWalkMax.x);
    mWalkTarget.z = cM::rndRange(mWalkMin.z, mWalkMax.z);
    resetAccel();
    mState = &dFishMusLobster_c::stateWalk;
}

// 0xFAF0
void dFishMusLobster_c::stateWalk() {
    turn();
    accelerate(mpData->mSpeed, 0.01f, 0.1f);
    if (mPos.xzDistTo(mWalkTarget) < 8.0f || --mTimer < 0) {
        if (cM::rndRange(0, 100) < 50) {
            initWait();
        } else {
            initSwim();
        }
    }
}

// 0xFBC8
void dFishMusLobster_c::turn() {
    mWalkAngleY = cLib::targetAngleY(mPos, mWalkTarget);
    sLib::addCalcAngle(&mAngle.y.mAngle, mWalkAngleY, 30, mAng::fromDegree(0.8f), mAng::fromDegree(0.3f));
}

// 0xFC50
void dFishMusLobster_c::walk() {
    mVelocity.set(0.0f, 0.0f, mSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    mPos.y = mpData->mSwimY;
}

// 0xFCE0
void dFishMusLobster_c::calcAnmRate() {
    if (mSpeed > 0.07f) {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.98f, 0.03f);
    } else {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.98f, 0.005f);
    }
    mAnm.setRate(mAnmRate);
}

// 0xFD60
void dFishMusLobster_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0xFD98
void dFishMusLobster_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (other->mFishIdx == FISH_UTSUBO || other->mFishIdx == FISH_ANKOU || other->mFishIdx == FISH_SHIIRA) {
        if (mHitFishIdx != other->mFishIdx || mHitFrames > 500) {
            setHitInfo(other);
            mHitFrames = 0;
            escape();
        }
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0xFE38
void dFishMusLobster_c::escape() {
    mWalkTarget = mPos + (mPos - mWalkTarget);
    if (mWalkTarget.x < mWalkMin.x) {
        mWalkTarget.x = mWalkMin.x;
    } else if (mWalkMax.x < mWalkTarget.x) {
        mWalkTarget.x = mWalkMax.x;
    } else if (mWalkTarget.z < mWalkMin.z) {
        mWalkTarget.z = mWalkMin.z;
    } else if (mWalkMax.z < mWalkTarget.z) {
        mWalkTarget.z = mWalkMax.z;
    }
}
