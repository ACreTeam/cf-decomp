namespace dFishMuseum {

f32 sEelBendRate = 0.5f;

} // namespace dFishMuseum

// 0x96D0
dFishMusEel_c::dFishMusEel_c() {}

// 0x972C
dFishMusEel_c::~dFishMusEel_c() {}

// 0x97A0
BOOL dFishMusEel_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, NODE_NUM, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdl.setAnm(mAnm);
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    mMdl.mBendRate = dFishMuseum::sEelBendRate;
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0x98F4
BOOL dFishMusEel_c::execute() {
    (this->*mState)();
    mTimer--;
    calcMove();
    calcMtx();
    calcAnmRate();
    const dFishMusData_c &data = dFishMuseum::sFishData[mFishIdx];
    mMdlOffset.set(0.0f, 0.0f, data.mMdlOffsetZ);
    mCenterOffset.set(0.0f, 0.0f, data.mCenterOffsetZ);
    mMdl.mTrailStep = data.mTrailStep;
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x99B4
BOOL dFishMusEel_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    getAquarium()->removeFish(this);
    getAquarium()->removeHitFish(this);
    return TRUE;
}

// 0x9A28
void dFishMusEel_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    mSpeed = mpData->mSpeed;
    mAnmRate = 0.0f;
    mTargetPos.y = mpData->mSwimY;
    initWait();
    mMdl.calc(false);
}

// 0x9AA0
void dFishMusEel_c::initWait() {
    setWaitTimer();
    mState = &dFishMusEel_c::stateWait;
}

// 0x9AE8
void dFishMusEel_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.025f, 0.01f);
    if (mTimer < 0) {
        initSwim();
    }
}

// 0x9B3C
void dFishMusEel_c::initSwim() {
    setSwimTimer();
    mTargetPos = mPos;
    f32 rnd = cM::rnd();
    mSpeed = cM::rndRange(mpData->mSpeed - 0.2f, mpData->mSpeed);
    mVec3_c ofs(0.0f, 0.0f, 23.0f + 23.0f * rnd);
    s16 yaw = mAng::fromDegree(cM::rndRange(-160.0f, 160.0f));
    ofs.rotY(yaw + mAngle.y.mAngle);
    mTargetPos += ofs;
    mState = &dFishMusEel_c::stateSwim;
}

// 0x9C64
void dFishMusEel_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < 6.0f) {
        if (cM::rndRange(0.0f, 100.0f) < 20.0f || isBent()) {
            initSwim();
        } else {
            initWait();
        }
    } else if (mTimer < 0) {
        if (isBent()) {
            initSwim();
        } else {
            initWait();
        }
    } else {
        turn();
    }
}

// 0x9D4C
void dFishMusEel_c::setWaitTimer() {
    mTimer = cM::rndRange(mpData->mWaitTimeMin, mpData->mWaitTimeMax);
}

// 0x9D88
void dFishMusEel_c::setSwimTimer() {
    mTimer = cM::rndRange(mpData->mSwimTimeMin, mpData->mSwimTimeMax);
}

// 0x9DC4
void dFishMusEel_c::turn() {
    mTargetAngleY = cLib::targetAngleY(mPos, mTargetPos);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngleY, 8, mAng::fromDegree(4.0f), mAng::fromDegree(1.0f));
}

// 0x9E4C
void dFishMusEel_c::calcMove() {
    mVelocity.set(0.0f, 0.0f, mSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
}

// 0x9ED0
void dFishMusEel_c::calcAnmRate() {
    mAnmRate = mpData->mAnmRateSlow + 0.6666667f * mSpeed * (mpData->mAnmRateFast - mpData->mAnmRateSlow);
    mAnm.setRate(mAnmRate);
}

// 0x9F04
BOOL dFishMusEel_c::isBent() {
    for (int i = 0; i < NODE_NUM; i++) {
        if (1.0f - mMdl.mNodeRot[i].w > 0.03f) {
            return TRUE;
        }
    }
    return FALSE;
}

// 0x9FC8
void dFishMusEel_c::addPos(const mVec3_c &) {
    mVec3_c push(0.0f, 0.0f, 0.0f);
    calcCorners();
    mVec3_c pushes[CORNER_NUM];
    for (int i = 0; i < CORNER_NUM; i++) {
        pushes[i] = getWallPush(mCorners[i]);
    }
    push = getMaxPush(pushes[0], pushes[1], pushes[2], pushes[3]);
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0xA12C
void dFishMusEel_c::calcCorners() {
    for (int i = 0; i < CORNER_NUM; i++) {
        nw4r::math::MTX34 &mtx = mMdl.mNodeMtx[i * 2];
        mCorners[i].set(mtx._03, mtx._13, mtx._23);
    }
}

// 0xA194
mVec3_c dFishMusEel_c::getWallPush(mVec3_c pos) {
    f32 margin = mpData->mHitLength;
    f32 minX = mTankMin.x + margin;
    f32 maxX = mTankMax.x - margin;
    f32 minZ = mTankMin.z + margin;
    f32 maxZ = mTankMax.z - margin;
    mVec3_c push(0.0f, 0.0f, 0.0f);
    if (pos.x < minX) {
        push.x = minX - pos.x;
    } else if (pos.x > maxX) {
        push.x = maxX - pos.x;
    }
    if (pos.z < minZ) {
        push.z = minZ - pos.z;
    } else if (pos.z > maxZ) {
        push.z = maxZ - pos.z;
    }
    return push;
}

// 0xA224
mVec3_c dFishMusEel_c::getMaxPush(mVec3_c push0, mVec3_c push1, mVec3_c push2, mVec3_c push3) {
    mVec3_c push01 = getMaxAbs(push0, push1);
    mVec3_c push23 = getMaxAbs(push2, push3);
    return getMaxAbs(push01, push23);
}

// 0xA338
mVec3_c dFishMusEel_c::getMaxAbs(mVec3_c a, mVec3_c b) {
    mVec3_c ret(0.0f, 0.0f, 0.0f);
    if (nw4r::math::FAbs(a.x) > nw4r::math::FAbs(b.x)) {
        ret.x = a.x;
    } else {
        ret.x = b.x;
    }
    if (nw4r::math::FAbs(a.z) > nw4r::math::FAbs(b.z)) {
        ret.z = a.z;
    } else {
        ret.z = b.z;
    }
    return ret;
}
