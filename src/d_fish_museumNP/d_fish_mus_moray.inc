// Its string literal sends it to the section of the last data definition, dFishMusObjLotus_c::sBaseNodeID, so it
// ends up among the frog's functions (as does dFishMusOctopus_c::create).
// 0xD154
void dFishMusMoray_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    dBgUtil::getNodePos(&mHomePos, "rock08", -1, -1);
    mPos = mHomePos;
    mPos.y = mpData->mSwimY;
    mAngle.y = 0x2000;
    mLoopStart = 0.0f;
    mLoopEnd = 120.0f;
    mAnmState = ANM_HIDDEN;
}

// 0xFEE4
dFishMusMoray_c::dFishMusMoray_c() {}

// 0xFF20
dFishMusMoray_c::~dFishMusMoray_c() {}

// 0xFF78
BOOL dFishMusMoray_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.m3d::smdl_c::create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0x10058
BOOL dFishMusMoray_c::execute() {
    mPos = mHomePos;
    mPos.y = mpData->mSwimY;
    checkPlayer();
    playAnm();
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x10118
BOOL dFishMusMoray_c::draw() {
    dFishMusBase_c::draw();
    return TRUE;
}

// 0x1013C
BOOL dFishMusMoray_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    getAquarium()->removeHitFish(this);
    return TRUE;
}

// 0x101A0
void dFishMusMoray_c::checkPlayer() {
    mVec3_c playerPos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c front = mPos;
    front.x += 40.0f;
    front.z = mTankMax.z;
    if (playerPos.xzDistTo(front) < 0.333333f * (2.0f * (mTankMax.z - mTankMin.z))) {
        if (mAnmState == ANM_HIDDEN) {
            mAnmState = ANM_COME_OUT;
        }
    } else {
        if (mAnmState == ANM_OUT) {
            mAnmState = ANM_GO_IN;
        }
    }
}

// 0x10298
void dFishMusMoray_c::playAnm() {
    if (mAnmState == ANM_HIDDEN) {
        mLoopStart = 0.0f;
        mLoopEnd = 120.0f;
    } else if (mAnmState == ANM_COME_OUT) {
        mLoopStart = 180.0f;
        mLoopEnd = 180.0f;
    } else if (mAnmState == ANM_OUT) {
        mLoopStart = 180.0f;
        mLoopEnd = 370.0f;
    } else if (mAnmState == ANM_GO_IN) {
        mLoopStart = 0.0f;
        mLoopEnd = 430.0f;
    }
    if (mFrame > mLoopEnd) {
        f32 over;
        if (mFrame > 430.0f) {
            over = mAnm.getFrame();
        } else {
            over = mAnm.getFrame() - mLoopEnd;
        }
        mFrame = mLoopStart + over;
        if (mAnmState == ANM_COME_OUT) {
            mAnmState = ANM_OUT;
        } else if (mAnmState == ANM_GO_IN) {
            mAnmState = ANM_HIDDEN;
        }
    }
    mAnm.setFrame(mFrame);
    mFrame += 1.0f;
}
