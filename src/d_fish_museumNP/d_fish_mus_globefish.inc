// 0xD2E4
dFishMusGlobefish_c::dFishMusGlobefish_c() {}

// 0xD320
dFishMusGlobefish_c::~dFishMusGlobefish_c() {}

// 0xD378
BOOL dFishMusGlobefish_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 2, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    init();
    return TRUE;
}

// 0xD458
BOOL dFishMusGlobefish_c::execute() {
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

// 0xD514
BOOL dFishMusGlobefish_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    return TRUE;
}

// 0xD568
void dFishMusGlobefish_c::init() {
    initMove();
    setTankArea();
    mPoint0Pos.set(110.0f + mTankMax.x, mpData->mSwimY, mTankMin.z - 4.0f);
    mPoint5Pos.set(mTankMin.x - 110.0f, mpData->mSwimY, mTankMin.z - 4.0f);
    mCrossCount = cM::rndRange(0, 2);
    switch (mCrossCount) {
    case 0:
        mPos = mPoint5Pos;
        mAngle.y = -0x4000;
        break;
    case 1:
        mPos = mPoint0Pos;
        mAngle.y = 0x4000;
        break;
    default:
        mPos = mPoint5Pos;
        break;
    }
    initSwim();
    mState = &dFishMusGlobefish_c::stateSwim;
}

// 0xD6A4
void dFishMusGlobefish_c::initSwim() {
    if (mCrossCount % 2 == 0) {
        mTargetPos = mPoint0Pos;
    } else {
        mTargetPos = mPoint5Pos;
    }
    mAngle.y += 0x8000;
    mCrossCount++;
}

// 0xD70C
void dFishMusGlobefish_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < 12.0f) {
        initSwim();
    } else {
        turnY(20, 3.0f, 1.0f);
    }
}
