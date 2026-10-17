// Defined here, not with sBaseMtx: both its .bss slot and the placement of dFishMusMoray_c::init depend on it.
int dFishMusObjLotus_c::sBaseNodeID;

// 0xCB04
dFishMusFrog_c::dFishMusFrog_c() {}

// 0xCB50
dFishMusFrog_c::~dFishMusFrog_c() {}

// 0xCBC4
BOOL dFishMusFrog_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.m3d::smdl_c::create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    mMtx_c mtx = dFishMusObjLotus_c::sBaseMtx;
    mMdl.setLocalMtx(&mtx);
    init();
    return TRUE;
}

// 0xCCF0
BOOL dFishMusFrog_c::execute() {
    mSound.setPos(&mPos);
    (this->*mState)();
    mMtx_c mtx = dFishMusObjLotus_c::sBaseMtx;
    mMdl.setLocalMtx(&mtx);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0xCDD0
BOOL dFishMusFrog_c::draw() {
    mMdl.entry();
    return TRUE;
}

// 0xCE00
BOOL dFishMusFrog_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    return TRUE;
}

const f32 dFishMuseum::sCroakFrames[2] = {20.0f, 160.0f};

// 0xCE54
void dFishMusFrog_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    mPos.x = mTankMin.x + 13.0f * (mTankMax.x - mTankMin.x) / 16.0f;
    mPos.y = mTankMax.y;
    mPos.z = (mTankMin.z + mTankMax.z) / 2.0f;
    mFrame = mpData->mAnmRateFast;
    mCroakEndFrame = 20.0f;
    initWait();
}

// 0xCF08
void dFishMusFrog_c::initWait() {
    setWaitTimer();
    mState = &dFishMusFrog_c::stateWait;
}

// 0xCF50
void dFishMusFrog_c::stateWait() {
    mAnm.setFrame(mFrame);
    if (--mTimer < 0) {
        initCroak();
    }
}

// 0xCF9C
void dFishMusFrog_c::initCroak() {
    setCroakCount();
    mSound.startSound(SE_kaeru_Trg);
    mState = &dFishMusFrog_c::stateCroak;
}

// 0xCFF0
void dFishMusFrog_c::stateCroak() {
    if (isCroakDone()) {
        if (__fabsf(mCroakEndFrame - 160.0f) < 0.1f) {
            mCroakEndFrame = 20.0f;
            mFrame = mAnm.getFrame();
        } else {
            mCroakEndFrame += 20.0f;
        }
        if (mTimer > 1) {
            mSound.startSound(SE_kaeru_Trg);
        }
        if (--mTimer == 0) {
            initWait();
        }
    }
    mAnm.setFrame(mFrame);
    mFrame += mpData->mAnmRateFast;
}

// 0xD0C4
void dFishMusFrog_c::setWaitTimer() {
    mTimer = cM::rndRange(mpData->mWaitTimeMin, mpData->mWaitTimeMax);
}

// 0xD100
void dFishMusFrog_c::setCroakCount() {
    mTimer = cM::rndRange(mpData->mSwimTimeMin, mpData->mSwimTimeMax);
}

// 0xD13C
bool dFishMusFrog_c::isCroakDone() const {
    return mFrame > mCroakEndFrame;
}
