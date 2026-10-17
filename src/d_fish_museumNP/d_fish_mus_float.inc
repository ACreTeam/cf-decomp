// 0xBE98
dFishMusFloat_c::dFishMusFloat_c() {}

// 0xBED4
dFishMusFloat_c::~dFishMusFloat_c() {}

// 0xBF2C
BOOL dFishMusFloat_c::create(nw4r::g3d::ResFile res) {
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

// 0xC01C
BOOL dFishMusFloat_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnmFrame();
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, data.mMdlOffsetZ, 0.0f);
    mCenterOffset.set(0.0f, data.mCenterOffsetZ, 0.0f);
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xC0C8
BOOL dFishMusFloat_c::draw() {
    mMdl.entry();
    return TRUE;
}

// 0xC0F8
void dFishMusFloat_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    mAngle.x = 0;
    mAngle.y = 0x4000;
    mPos.y = mpData->mSwimY;
    mAnmFrame = 0.0f;
    if (mFishIdx == FISH_KURIONE) {
        mSwimSpeed = 0.05f;
        mStrokeFrames = 30.0f;
        mSwayDeg = 30.0f;
        mTurnMaxStep = 0.6f;
        mTurnMinStep = 0.3f;
        mPos.x -= 5.0f;
    } else if (mFishIdx == FISH_KURAGE) {
        mSwimSpeed = 0.07f;
        mStrokeFrames = 80.0f;
        mSwayDeg = 45.0f;
        mTurnMaxStep = 0.2f;
        mTurnMinStep = 0.1f;
        mPos.x += 5.0f;
    } else if (mFishIdx == FISH_TATSU) {
        mSwimSpeed = 0.06f;
        mStrokeFrames = 40.0f;
        mSwayDeg = 15.0f;
        mTurnMaxStep = 0.2f;
        mTurnMinStep = 0.1f;
    }
    initDrift();
}

// 0xC1F0
void dFishMusFloat_c::initDrift() {
    setWaitTimer();
    mSpeed = -0.02f;
    mAnmRate = mpData->mAnmRateSlow;
    if (mFishIdx == FISH_KURIONE) {
        mSway = 0;
    } else if (mFishIdx == FISH_KURAGE) {
        mSway = cM::rndRange<long>(mAng::fromDegree(mSwayDeg), mAng::fromDegree(2.0f * mSwayDeg));
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mSway = -mSway;
        }
    } else if (mFishIdx == FISH_TATSU) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mSway = mAng::fromDegree(15.0f);
        } else {
            mSway = mAng::fromDegree(-15.0f);
        }
        mSwayWait = 15;
        mSwayPause = false;
    }
    mState = &dFishMusFloat_c::stateDrift;
}

// 0xC35C
void dFishMusFloat_c::stateDrift() {
    if (mTimer == 0) {
        initSwim();
        return;
    }
    if (isFishKind(FISH_KURIONE) || isFishKind(FISH_KURAGE)) {
        mTargetAngleX = mSway;
        turnX(mTurnMaxStep, mTurnMinStep);
        if (mAnmFrame > mStrokeFrames) {
            if (mPos.y < mpData->mSwimYMin) {
                initSwimLong();
            }
            mTimer--;
            mAnmFrame -= mStrokeFrames;
        }
    } else if (isFishKind(FISH_TATSU)) {
        sway();
    }
    if (mAnmFrame > mStrokeFrames) {
        mAnmFrame -= mStrokeFrames;
    }
}

// 0xC434
void dFishMusFloat_c::sway() {
    if (mSwayPause) {
        if (--mSwayWait == 0) {
            mSwayWait = 15;
            mSwayPause = false;
        }
        return;
    }
    if (mSway > 0) {
        mAngle.x += mAng::fromDegree(0.3f);
        if (mAngle.x >= mAng::fromDegree(15.0f)) {
            mSwayPause = true;
            mTimer--;
            mSway = mAng::fromDegree(-15.0f);
        }
    } else {
        mAngle.x -= mAng::fromDegree(0.3f);
        if (mAngle.x <= mAng::fromDegree(-15.0f)) {
            mSwayPause = true;
            mTimer--;
            mSway = mAng::fromDegree(15.0f);
        }
    }
    if (mAng::fromDegree(-5.0f) <= mAngle.x && mAngle.x <= mAng::fromDegree(5.0f) && mPos.y < mpData->mSwimYMin) {
        initSwimLong();
    }
}

// 0xC5F0
void dFishMusFloat_c::initDriftLong() {
    initDrift();
    mTimer *= 2;
    mState = &dFishMusFloat_c::stateDriftLong;
}

// 0xC644
void dFishMusFloat_c::stateDriftLong() {
    stateDrift();
}

// 0xC648
void dFishMusFloat_c::initSwim() {
    mAnmRate = mpData->mAnmRateFast;
    mSpeed = mSwimSpeed;
    setSwimTimer();
    setSwayAngle();
    setTargetAngleY();
    mState = &dFishMusFloat_c::stateSwim;
}

// 0xC6B4
void dFishMusFloat_c::stateSwim() {
    if (mTimer == 0) {
        initDrift();
        return;
    }
    mTargetAngleX = mPrevSway - mSway;
    turnX(0.5f, 0.2f);
    turnY();
    if (mAnmFrame > mStrokeFrames) {
        if (mPos.y > mpData->mSwimYMax) {
            initDriftLong();
        }
        mTimer--;
        mAnmFrame -= mStrokeFrames;
    }
}

// 0xC764
void dFishMusFloat_c::initSwimLong() {
    initSwim();
    mTimer *= 2;
    mState = &dFishMusFloat_c::stateSwimLong;
}

// 0xC7B8
void dFishMusFloat_c::stateSwimLong() {
    stateSwim();
}

// 0xC7BC
void dFishMusFloat_c::setWaitTimer() {
    mTimer = cM::rndRange(mpData->mWaitTimeMin, mpData->mWaitTimeMax);
}

// 0xC7F8
void dFishMusFloat_c::setSwimTimer() {
    mTimer = cM::rndRange(mpData->mSwimTimeMin, mpData->mSwimTimeMax);
}

// 0xC834
void dFishMusFloat_c::setSwayAngle() {
    mPrevSway = mSway;
    mSway = cM::rndRange<long>(mAng::fromDegree(-mSwayDeg), mAng::fromDegree(mSwayDeg));
}

// 0xC8A4
void dFishMusFloat_c::setTargetAngleY() {
    if (cM::rndRange(0.0f, 100.0f) < 20.0f) {
        mYawStep = cM::rndRange<long>(mAng::fromDegree(0.0f), mAng::fromDegree(10.0f));
        mTargetAngleY += mYawStep;
    } else {
        mTargetAngleY = mAngle.y;
    }
}

// 0xC950
void dFishMusFloat_c::turnX(f32 maxStep, f32 minStep) {
    sLib::addCalcAngle(&mAngle.x.mAngle, mTargetAngleX, 40, mAng::fromDegree(maxStep), mAng::fromDegree(minStep));
}

// 0xC9AC
void dFishMusFloat_c::turnY() {
    mYawStep = mTargetAngleY;
    sLib::addCalcAngle(&mAngle.y.mAngle, mYawStep, 40, mAng::fromDegree(0.1f), mAng::fromDegree(0.05f));
}

// 0xCA1C
void dFishMusFloat_c::calcMove() {
    mVelocity.set(0.0f, mSpeed, 0.0f);
    mVelocity.rotX(mAngle.x);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
}

// 0xCAB4
void dFishMusFloat_c::calcAnmFrame() {
    mAnmFrame += mAnmRate;
    mAnm.setFrame(mAnmFrame);
}

// 0xCACC
void dFishMusFloat_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}
