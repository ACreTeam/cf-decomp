// 0x10568
dFishMusPiranha_c::dFishMusPiranha_c() {}

// 0x105AC
dFishMusPiranha_c::~dFishMusPiranha_c() {}

// 0x10614
BOOL dFishMusPiranha_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 3, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0x10714
BOOL dFishMusPiranha_c::execute() {
    mSnd.setPos(&mPos);
    checkPlayer();
    (this->*mState)();
    calcMove();
    calcAnm();
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

// 0x107EC
BOOL dFishMusPiranha_c::draw() {
    dFishMusBase_c::draw();
    return TRUE;
}

// 0x10810
void dFishMusPiranha_c::init() {
    initMove();
    setTankArea();
    mState = NULL;
    mAccelStep = 0.01f;
    mSwimThroughRate = 60.0f;
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.2f;
    mTurnX.mMinStep = 0.1f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    mHitFrames = 10;
    mFrame = 30.0f;
    mLoopStart = 30.0f;
    mLoopEnd = 60.0f;
    mMode = MODE_NORMAL;
    dFishMusCommonMove_c::initNextAction();
}

// 0x108D4
void dFishMusPiranha_c::initWait() {
    setWaitTimer();
    mState = &dFishMusBaseMove_c::stateWait;
}

// 0x1091C
void dFishMusPiranha_c::stateSwim() {
    accelerate(mpData->mSpeed, mAccelStep, 50.0f);
    turn();
    if (mPos.xzDistTo(mTargetPos) < 15.0f || mTimer < 0) {
        if (cM::rndRange(0.0f, 100.0f) < mSwimThroughRate) {
            mState = &dFishMusBaseMove_c::stateSwimThrough;
        } else {
            mState = &dFishMusBaseMove_c::stateStop;
        }
        mSwimThroughRate = 60.0f;
    }
}

// 0x10A0C
void dFishMusPiranha_c::initSwim() {
    setSwimTimer();
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 0.2f;
    mTurnX.mMinStep = 0.1f;
    mTurnY.mScale = 20;
    mTurnY.mMaxStep = 2.0f;
    mTurnY.mMinStep = 1.0f;
    mTargetPos = mPos;
    f32 rnd = cM::rnd();
    mTargetOfs.set(0.0f, 0.0f, 60.0f + 30.0f * rnd);
    setTargetAngleX();
    setTargetOfs();
    mTargetPos += mTargetOfs;
    mSpeed = mpData->mSpeed;
    resetAccel();
    mState = &dFishMusPiranha_c::stateSwim;
}

// 0x10B20
void dFishMusPiranha_c::checkPlayer() {
    if (isPlayerNear() && mMode == MODE_NORMAL) {
        mMode = MODE_ATTACK;
        mState = &dFishMusPiranha_c::stateAttack;
    }
}

// 0x10B84
void dFishMusPiranha_c::stateAttack() {
    mTurnY.mMaxStep = 10.0f; // overwritten below; both stores are in the target
    mTurnX.mScale = 20;
    mTurnX.mMaxStep = 1.0f;
    mTurnX.mMinStep = 0.5f;
    mTurnY.mScale = 10;
    mTurnY.mMaxStep = 5.0f;
    mVec3_c playerPos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mTargetPos = playerPos;
    mSpeed = 2.0f * mpData->mSpeed;
    mTargetPos.y += 30.0f;
    if (mTargetPos.y > mpData->mSwimYMax) {
        mTargetPos.y = mpData->mSwimYMax;
    }
    mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
    turn();
}

// 0x10C60
void dFishMusPiranha_c::startBounce() {
    mSnd.startSound(SE_PiranhaHitSuisou);
    f32 speed = mpData->mSpeed / 3.0f;
    mPrevPos = mPos;
    mSpeed = -speed;
    mTargetAngleX = cLib::targetAngleX(mPos, mTargetPos);
}

// 0x10CD8
void dFishMusPiranha_c::stateBounce() {
    sLib::addCalc0(&mSpeed, 0.1f, cM::rndRange(0.6, 0.8));
    mVec3_c move = mPos - mPrevPos;
    dFishMusBase_c::addPos(move);
    mMdl.shiftTrail(mPush);
    mPrevPos = mPos;
    if (nw4r::math::FAbs(mSpeed) < 0.01f) {
        mTimer = 0;
        if (mMode == MODE_ATTACK) {
            mState = &dFishMusPiranha_c::stateAttackWait;
        } else {
            initWait();
        }
    }
}

// 0x10DC8
void dFishMusPiranha_c::stateAttackWait() {
    if (mTimer < 0) {
        mState = &dFishMusPiranha_c::stateAttack;
    }
}

// 0x10DF4
void dFishMusPiranha_c::correctPos() {
    f32 cosY = mAng(mAngle.y.mAngle - 0x8000).cos();
    const dFishMusData_c *data = mpData;
    f32 marginX, marginZ;
    data->getHalfSizeXZ(cosY, marginX, marginZ);
    f32 minX = mTankMin.x + marginX;
    f32 maxX = mTankMax.x - marginX;
    f32 minZ = mTankMin.z + marginZ;
    f32 maxZ = mTankMax.z - marginZ;
    f32 minY = mTankMin.y;
    f32 maxY = mTankMax.y - 0.5f * data->mHitHeight;
    mVec3_c push(0.0f, 0.0f, 0.0f);
    mVec3_c center;
    getCenterPos(center);
    if (center.x < minX) {
        push.x = minX - center.x;
    } else if (center.x > maxX) {
        push.x = maxX - center.x;
    }
    if (center.z < minZ) {
        push.z = minZ - center.z;
    } else if (center.z > maxZ) {
        push.z = maxZ - center.z;
    }
    if (center.y < minY) {
        push.y = minY - center.y;
    } else if (center.y > maxY) {
        push.y = maxY - center.y;
    }
    dFishMusBase_c::addPos(push);
    mPrevPos += push;
    mMdl.shiftTrail(mPush);
}

// 0x11064
BOOL dFishMusPiranha_c::isPlayerNear() {
    mVec3_c playerPos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (playerPos.xzDistTo(mPos) < 2.0f * (mTankMax.z - mTankMin.z) / 3.0f) {
        return TRUE;
    }
    return FALSE;
}

// 0x1110C
BOOL dFishMusPiranha_c::isFacingPlayer(const mVec3_c &push) {
    mVec3_c playerPos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c dir = playerPos - mPos;
    dir.normalize();
    mVec3_c normal(0.0f, 0.0f, 0.0f);
    if (!isZero(push.x)) {
        normal.set(0.0f, 0.0f, 1.0f);
    }
    if (!isZero(push.z)) {
        normal.x += 1.0f;
        normal.normalize();
    }
    if (std::fabs(normal.x * dir.x + normal.y * dir.y + normal.z * dir.z) < 0.342f ||
        (!isZero(push.x) && !isZero(push.z))) {
        return TRUE;
    }
    return FALSE;
}

// 0x11298
BOOL dFishMusPiranha_c::isPushedToPlayer(const mVec3_c &push) {
    mVec3_c playerPos = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c dir = playerPos - mPos;
    if (push.x * dir.x + push.y * dir.y + push.z * dir.z >= 0.0f) {
        return TRUE;
    }
    return FALSE;
}

// 0x11348
void dFishMusPiranha_c::calcAnm() {
    mFrame += mFrameRate;
    if (mFrame > mLoopEnd) {
        if (mLoopEnd < 40.0f) {
            mFrame -= mLoopEnd;
        } else {
            mFrame = mLoopStart + mAnm.getFrame();
        }
    }
    if (mMode == MODE_ATTACK) {
        mLoopStart = 0.0f;
        mLoopEnd = 30.0f;
        mFrameRate = 3.2f * mpData->mAnmRateFast;
    } else {
        mLoopStart = 30.0f;
        mLoopEnd = 60.0f;
        if (mSpeed < 0.05f || isNearWall()) {
            mFrameRate = mpData->mAnmRateSlow;
        } else {
            mFrameRate = mpData->mAnmRateFast;
        }
    }
    if ((4.0f - 0.5f * mFrameRate < mFrame && mFrame < 4.0f + 0.5f * mFrameRate) ||
        (18.0f - 0.5f * mFrameRate < mFrame && mFrame < 18.0f + 0.5f * mFrameRate)) {
        mSnd.startSound(SE_PiranhaTeeth);
    }
    mAnm.setFrame(mFrame);
}

// 0x114B8
void dFishMusPiranha_c::addPos(const mVec3_c &push) {
    if (!push.isZero()) {
        if (mState == &dFishMusPiranha_c::stateBounce) {
            correctPos();
        } else if (mMode == MODE_ATTACK && isPlayerNear() && !isPushedToPlayer(push)) {
            if (isFacingPlayer(push)) {
                startBounce();
                mState = &dFishMusPiranha_c::stateBounce;
            }
        } else if (mMode == MODE_ATTACK && !isPlayerNear() && !isPushedToPlayer(push)) {
            mMode = MODE_NORMAL;
            startBounce();
            mState = &dFishMusPiranha_c::stateBounce;
        }
    }
    if (mState != &dFishMusPiranha_c::stateBounce) {
        dFishMusBase_c::addPos(push);
        mMdl.shiftTrail(mPush);
    }
}

// 0x11648
void dFishMusPiranha_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mMode != MODE_ATTACK) {
        dFishMusBaseMove_c::onHitFish(other, push);
    } else {
        dFishMusBase_c::onHitFish(other, push);
    }
}
