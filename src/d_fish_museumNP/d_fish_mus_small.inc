// 0x13854
dFishMusSmall_c::dFishMusSmall_c() {}

// 0x13890
dFishMusSmall_c::~dFishMusSmall_c() {}

// 0x138E8
BOOL dFishMusSmall_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    int nodeNum = 2;
    if (mFishIdx == FISH_WAKASAGI || mFishIdx == FISH_KUMANOMI) {
        nodeNum = 3;
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

// 0x13A00
BOOL dFishMusSmall_c::execute() {
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

// 0x13AC0
void dFishMusSmall_c::init() {
    mState = NULL;
    initMove();
    initWait();
}

// 0x13B1C
void dFishMusSmall_c::initWait() {
    setWaitTimer();
    mState = &dFishMusSmall_c::stateWait;
}

// 0x13B64
void dFishMusSmall_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.98f, 0.015f);
    if (mTimer < 0) {
        initSwim();
    }
}

// 0x13BC4
void dFishMusSmall_c::initSwim() {
    setSwimTimer();
    mTimer = cM::rndRange(80, 150);
    mTargetPos = mPos;
    mSpeed = mpData->mSpeed;
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 16.0f + 16.0f * rnd);
    setTargetAngleX();
    setTargetOfs();
    mTargetPos += mTargetOfs;
    mState = &dFishMusSmall_c::stateSwim;
}

// 0x13CA8
void dFishMusSmall_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < 6.0f) {
        initNextAction();
    } else if (mTimer < 0) {
        if (cM::rndRange(0.0f, 100.0f) < 40.0f || 1.0f - mMdl.mNodeRot[1].w > 0.1f) {
            initSwim();
        } else {
            initWait();
        }
    } else {
        turnX(20, 0.2f, 0.1f);
        turnY(8, 8.0f, 1.0f);
        if (mFishIdx == FISH_NANYOU) {
            sLib::addCalc0(&mSpeed, 0.2f, 0.015f);
        } else {
            sLib::addCalc0(&mSpeed, 0.98f, 0.008f);
        }
    }
}

// 0x13DE8
void dFishMusSmall_c::calcAnmRate() {
    int fishIdx = mFishIdx;
    if (mSpeed > 0.1f) {
        if (fishIdx == FISH_KINGYO || fishIdx == FISH_DEMEKIN || fishIdx == FISH_NANYOU) {
            sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.5f, 0.5f);
        } else if (fishIdx == FISH_KUMANOMI || fishIdx == FISH_ANGEL) {
            sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.35f, 0.1f);
        } else {
            sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.4f, 0.2f);
        }
    } else if (fishIdx == FISH_KUMANOMI || fishIdx == FISH_MEDAKA) {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.2f, 0.02f);
    } else {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.1f, 0.05f);
    }
    mAnm.setRate(mAnmRate);
}

// 0x13EF4
void dFishMusSmall_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}
