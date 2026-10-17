// 0xB75C
dFishMusFlatfish_c::dFishMusFlatfish_c() {}

// 0xB798
dFishMusFlatfish_c::~dFishMusFlatfish_c() {}

// 0xB7F0
BOOL dFishMusFlatfish_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.m3d::smdl_c::create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0xB8E0
BOOL dFishMusFlatfish_c::execute() {
    (this->*mState)();
    calcMove();
    calcAnmRate();
    mTimer--;
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xB95C
void dFishMusFlatfish_c::init() {
    initMove();
    setTankArea();
    mState = NULL;
    mSpeed = 0.0f;
    mAlpha = 0xFF;
    setRandomPos();
    initWait();
}

// 0xB9DC
void dFishMusFlatfish_c::initWait() {
    setWaitTimer();
    mState = &dFishMusFlatfish_c::stateWait;
}

// 0xBA24
void dFishMusFlatfish_c::stateWait() {
    if (mAnm.getFrame() < 1.5f && mSpeed < 0.1f) {
        mAnm.setFrame(0.0f);
        mAnmRate = 0.0f;
    }
    if (mPos.y > mTankMin.y + mpData->mSwimYMin) {
        mTargetAngleX = DEG_TO_ANGLE(1);
        turnX(20, 3.0f, 1.0f);
    } else {
        mAngle.x = 0;
    }
    sLib::addCalc0(&mSpeed, 0.95f, 0.01f);
    if (mTimer < 0) {
        setSwimTimer();
        initSwim();
    }
}

// 0xBB10
void dFishMusFlatfish_c::initSwim() {
    mSpeed = mpData->mSpeed;
    mTargetPos = mPos;
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 60.0f + 30.0f * rnd);
    setTargetOfs();
    mTargetPos.x += mTargetOfs.x;
    mTargetPos.z += mTargetOfs.z;
    mTargetPos.y = mTankMin.y;
    mState = &dFishMusFlatfish_c::stateSwim;
}

// 0xBBDC
void dFishMusFlatfish_c::stateSwim() {
    if (mTimer < 0) {
        initWait();
    } else if (mTargetPos.distTo(mPos) < 10.0f || mSpeed < 0.05f) {
        initSwim();
    }
    mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
    turnX(10, 0.1f, 0.05f);
    turnY(20, 2.0f, 1.0f);
    sLib::addCalc0(&mSpeed, 0.95f, 0.01f);
}

// 0xBCBC
void dFishMusFlatfish_c::calcAnmRate() {
    if (0.05f < mSpeed && mSpeed < 0.4f) {
        sLib::addCalc2(&mAnmRate, 1.0f, 0.95f, 0.05f);
    } else if (mSpeed < 0.05f) {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.1f, 0.1f);
        fadeAlpha(4, 0xFF);
    } else {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateFast, 0.5f, 0.23f);
        fadeAlpha(-8, 0);
    }
    mAnm.setRate(mAnmRate);
}

// 0xBD8C
void dFishMusFlatfish_c::addPos(const mVec3_c &push) {
    mVec3_c v = push;
    v.y = 0.0f;
    dFishMusBase_c::addPos(v);
    mMdl.shiftTrail(mPush);
}

// 0xBDE4
void dFishMusFlatfish_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mHitFishIdx != other->mFishIdx || mHitFrames > 13) {
        const dFishMusData_c *otherData = &dFishMuseum::sFishData[other->mFishIdx];
        setHitInfo(other);
        setEscapeAngleY(other, otherData);
        setEscapeTarget(otherData);
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0xBE90
bool dFishMusFlatfish_c::isCornered(dFishMusBase_c *other) {
    return true;
}

// Its string literal sends it to the section of the last data definition, dFishMuseum::sEelBendRate, so it ends
// up among the eel's functions.
// 0xA398
void dFishMusFlatfish_c::fadeAlpha(int step, int target) {
    int matID = m3d::getMatID(mMdl.getResMdl(), "m0");
    if ((step > 0 && mAlpha < target) || (step < 0 && mAlpha > target)) {
        mAlpha += step;
    }
    if ((step > 0 && mAlpha > target) || (step < 0 && mAlpha < target)) {
        mAlpha = target;
    }
    GXColor color = {0, 0, 0, (u8)mAlpha};
    mMdl.setTevKColor(matID, GX_KCOLOR1, color, false);
}
