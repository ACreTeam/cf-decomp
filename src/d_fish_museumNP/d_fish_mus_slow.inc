// 0x12A28
dFishMusSlow_c::dFishMusSlow_c() {}

// 0x12A64
dFishMusSlow_c::~dFishMusSlow_c() {}

// 0x12ABC
BOOL dFishMusSlow_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    int nodeNum = 2;
    if (mFishIdx == FISH_AROWANA) {
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

// 0x12BCC
BOOL dFishMusSlow_c::execute() {
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

// 0x12C8C
void dFishMusSlow_c::init() {
    initPoints();
    mAngle.y = 0x4000;
    mWaitRate = 40.0f;
    mVec3_c center = (mTankMin + mTankMax) / 2.0f;
    mWayPoint0.set(mTankMax.x - 55.0f, mHeight, mTankMin.z + 12.0f);
    mWayPoint1.set(center.x + 90.0f, mHeight, center.z - 20.0f);
    mWayPoint2.set(mTankMax.x - 25.0f, mHeight, center.z);
    mWayPoint3.set(center.x + 90.0f, mHeight, center.z + 20.0f);
    mWayPoint4.set(mTankMax.x - 55.0f, mHeight, mTankMax.z - 12.0f);
    mWayPoint5.set(mTankMin.x + 55.0f, mHeight, mTankMax.z - 12.0f);
    mWayPoint6.set(center.x - 90.0f, mHeight, center.z + 20.0f);
    mWayPoint7.set(mTankMin.x + 25.0f, mHeight, center.z);
    mWayPoint8.set(center.x - 90.0f, mHeight, center.z - 20.0f);
    mWayPoint9.set(mTankMin.x + 55.0f, mHeight, mTankMin.z + 12.0f);
    setSwimTimer();
    if (mFishIdx == FISH_AROWANA) {
        mArriveDist = 10.0f;
        mTurnY.mScale = 15;
        mTurnY.mMaxStep = 0.8f;
        mTurnY.mMinStep = 0.4f;
        mNextPoint = 0;
        mPoint = 9;
        mPos = center;
        mPos.z = mTankMin.z + 10.0f;
        addOfs(mWayPoint0, -10.0f, 0.0f, -1.0f);
        mWayPoint2.x -= 10.0f;
        addOfs(mWayPoint4, -10.0f, 0.0f, 1.0f);
        addOfs(mWayPoint5, 10.0f, 0.0f, 1.0f);
        mWayPoint7.x += 10.0f;
        addOfs(mWayPoint9, 10.0f, 0.0f, -1.0f);
    } else if (mFishIdx == FISH_GA) {
        mArriveDist = 3.0f;
        mTurnY.mScale = 96;
        mTurnY.mMaxStep = 0.6f;
        mTurnY.mMinStep = 0.35f;
        mNextPoint = 4;
        mPoint = 5;
        mPos.set(mTankMin.x + 55.0f, mHeight, mTankMax.z - 12.0f);
        addOfs(mWayPoint0, -25.0f, 0.0f, 9.0f);
        addOfs(mWayPoint4, -9.0f, 0.0f, -16.0f);
        addOfs(mWayPoint5, 26.0f, 0.0f, -13.0f);
        addOfs(mWayPoint9, 17.0f, 0.0f, 11.0f);
    } else if (mFishIdx == FISH_PIRARUKU) {
        mArriveDist = 10.0f;
        mTurnY.mScale = 95;
        mTurnY.mMaxStep = 0.3f;
        mTurnY.mMinStep = 0.1f;
        mNextPoint = 0;
        mPoint = 9;
        mWaitRate = 90.0f;
        addOfs(mWayPoint0, -7.0f, 0.0f, -2.0f);
        addOfs(mWayPoint1, -3.0f, 0.0f, -4.0f);
        mWayPoint2.x += 5.0f;
        addOfs(mWayPoint3, -5.0f, 0.0f, 4.0f);
        addOfs(mWayPoint4, -10.0f, 0.0f, 2.0f);
        addOfs(mWayPoint5, 2.0f, 0.0f, -1.5f);
        addOfs(mWayPoint6, 0.0f, 0.0f, -4.0f);
        addOfs(mWayPoint7, -6.0f, 0.0f, -6.0f);
        addOfs(mWayPoint8, 2.0f, 0.0f, -6.0f);
        addOfs(mWayPoint9, 6.0f, 0.0f, -5.0f);
        mPos = mWayPoint9;
    }
    swimToPoint(mNextPoint);
    mHitFrames = 250;
    mState = &dFishMusSlow_c::stateSwim;
}

// 0x1310C
void dFishMusSlow_c::initWait() {
    setWaitTimer();
    mState = &dFishMusSlow_c::stateWait;
}

// 0x13154
void dFishMusSlow_c::stateWait() {
    sLib::addCalc0(&mSpeed, 0.03f, 0.001f);
    if (mTimer < 0) {
        setSwimTimer();
        mState = &dFishMusSlow_c::stateSwim;
    }
}

// 0x131C4
void dFishMusSlow_c::stateSwim() {
    if (mPos.xzDistTo(mTargetPos) < mArriveDist) {
        swimToPoint(mNextPoint);
    } else if (mTimer < 0 && 1.0f - mMdl.mNodeRot[1].w < 0.005f && cM::rndRange(0.0f, 100.0f) < mWaitRate) {
        initWait();
    } else {
        if (mSpeed < mpData->mSpeed) {
            mSpeed += 0.001f;
        }
        mTargetAngleX = cLib::targetAngleX(mTargetPos, mPos);
        turnX(95, 0.2f, 0.1f);
        turnY(mTurnX.mScale, mTurnY.mMaxStep, mTurnY.mMinStep); // sic: mTurnX.mScale
    }
}

// 0x132F0
void dFishMusSlow_c::swimToPoint(u8 point) {
    if (point == 0) {
        swimToPoint0();
    } else if (point == 1) {
        swimToPoint1();
    } else if (point == 2) {
        swimToPoint2();
    } else if (point == 3) {
        swimToPoint3();
    } else if (point == 4) {
        swimToPoint4();
    } else if (point == 5) {
        swimToPoint5();
    } else if (point == 6) {
        swimToPoint6();
    } else if (point == 7) {
        swimToPoint7();
    } else if (point == 8) {
        swimToPoint8();
    } else if (point == 9) {
        swimToPoint9();
    }
}

// 0x1336C
void dFishMusSlow_c::swimToPoint0() {
    mTargetPos = mWayPoint0;
    if (mPoint == 9) {
        if (mFishIdx == FISH_AROWANA) {
            setRandomHeight();
            mNextPoint = 2;
        } else {
            mNextPoint = 1;
        }
    } else {
        mNextPoint = 9;
    }
    mPoint = 0;
}

// 0x133F0
void dFishMusSlow_c::swimToPoint1() {
    mTargetPos = mWayPoint1;
    if (mPoint == 0) {
        mNextPoint = 2;
    } else {
        mNextPoint = 0;
    }
    mPoint = 1;
}

// 0x13434
void dFishMusSlow_c::swimToPoint2() {
    mTargetPos = mWayPoint2;
    if (mPoint == 0) {
        mNextPoint = 4;
    } else if (mPoint == 1) {
        mNextPoint = 3;
    } else {
        mNextPoint = 1;
    }
    mPoint = 2;
}

// 0x1348C
void dFishMusSlow_c::swimToPoint3() {
    mTargetPos = mWayPoint3;
    if (mPoint == 2) {
        mNextPoint = 4;
    } else {
        mNextPoint = 2;
    }
    mPoint = 3;
}

// 0x134D0
void dFishMusSlow_c::swimToPoint4() {
    mTargetPos = mWayPoint4;
    if (mPoint == 2 || mPoint == 3) {
        mNextPoint = 5;
    } else {
        mNextPoint = 0;
    }
    mPoint = 4;
}

// 0x1351C
void dFishMusSlow_c::swimToPoint5() {
    mTargetPos = mWayPoint5;
    if (mPoint == 4) {
        if (mFishIdx == FISH_AROWANA) {
            setRandomHeight();
            mNextPoint = 7;
        } else {
            mNextPoint = 6;
        }
    } else {
        mNextPoint = 4;
    }
    mPoint = 5;
}

// 0x135A0
void dFishMusSlow_c::swimToPoint6() {
    mTargetPos = mWayPoint6;
    if (mPoint == 5) {
        mNextPoint = 7;
    } else {
        mNextPoint = 5;
    }
    mPoint = 6;
}

// 0x135E4
void dFishMusSlow_c::swimToPoint7() {
    mTargetPos = mWayPoint7;
    if (mPoint == 5) {
        mNextPoint = 9;
    } else if (mPoint == 6) {
        mNextPoint = 8;
    } else {
        mNextPoint = 6;
    }
    mPoint = 7;
}

// 0x1363C
void dFishMusSlow_c::swimToPoint8() {
    mTargetPos = mWayPoint8;
    if (mPoint == 7) {
        mNextPoint = 9;
    } else {
        mNextPoint = 7;
    }
    mPoint = 8;
}

// 0x13680
void dFishMusSlow_c::swimToPoint9() {
    mTargetPos = mWayPoint9;
    if (mPoint == 7 || mPoint == 8) {
        mNextPoint = 0;
    } else {
        mNextPoint = 5;
    }
    mPoint = 9;
}

// 0x136CC
void dFishMusSlow_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0x13704
void dFishMusSlow_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    if (mFishIdx == FISH_AROWANA) {
        mHitFishIdx = other->mFishIdx;
        if (isHitByDangerous(other) && mHitFrames > 150) {
            escape();
        }
    }
    dFishMusBase_c::onHitFish(other, push);
}

// 0x13788
void dFishMusSlow_c::escape() {
    mHitFrames = 0;
    swimToPoint(mNextPoint);
    mTargetOfs.set(0.0f, 0.0f, 4.0f * mpData->mHitLength);
    s16 angle = -cM::rndRange<long>(0x18E4, 0x2000);
    mTargetOfs.rotY(angle + mAngle.y);
    mTargetPos = mPos + mTargetOfs;
    clampTarget(10.0f, 10.0f);
}
