// 0xA484
dFishMusElegance_c::dFishMusElegance_c() {}

// 0xA4C0
dFishMusElegance_c::~dFishMusElegance_c() {}

// 0xA518
BOOL dFishMusElegance_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    int nodeNum = 3;
    if (mFishIdx == FISH_ANGEL) {
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

// 0xA628
BOOL dFishMusElegance_c::execute() {
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

// 0xA6E8
void dFishMusElegance_c::initWait() {
    setWaitTimer();
    mState = &dFishMusSmall_c::stateWait;
}

// 0xA730
void dFishMusElegance_c::initSwim() {
    setSwimTimer();
    mTimer = cM::rndRange(80, 150);
    mTargetPos = mPos;
    mAccelerating = true;
    resetAccel();
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 16.0f + 16.0f * rnd);
    setTargetAngleX();
    setTargetOfs();
    mTargetPos += mTargetOfs;
    mState = &dFishMusElegance_c::stateSwim;
}

// 0xA818
void dFishMusElegance_c::stateSwim() {
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
        turnY(15, 2.0f, 1.0f);
        if (mAccelerating) {
            accelerate(mpData->mSpeed, 0.01f, 0.1f);
            if (mSpeed > mpData->mSpeed) {
                mAccelerating = false;
            }
        } else {
            sLib::addCalc0(&mSpeed, 0.98f, 0.005f);
            if (mSpeed < 0.001f) {
                initNextAction();
            }
        }
    }
}

// 0xA994
void dFishMusElegance_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}
