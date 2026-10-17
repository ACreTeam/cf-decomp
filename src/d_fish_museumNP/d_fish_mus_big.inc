// 0x7A84
dFishMusBig_c::dFishMusBig_c() {}

// 0x7AC0
dFishMusBig_c::~dFishMusBig_c() {}

// 0x7B18
void dFishMusBig_c::initPoints() {
    initMove();
    setTankArea();
    mHeight = mpData->mSwimY;
    mPoint5Pos.set(mTankMin.x + 55.0f, mHeight, mTankMin.z + 12.0f);
    mPoint4Pos.set(mTankMin.x + 25.0f, mHeight, (mTankMin.z + mTankMax.z) / 2.0f);
    mPoint3Pos.set(mTankMin.x + 55.0f, mHeight, mTankMax.z - 12.0f);
    mPoint0Pos.set(mTankMax.x - 55.0f, mHeight, mTankMin.z + 12.0f);
    mPoint1Pos.set(mTankMax.x - 25.0f, mHeight, (mTankMin.z + mTankMax.z) / 2.0f);
    mPoint2Pos.set(mTankMax.x - 55.0f, mHeight, mTankMax.z - 12.0f);
}

// 0x7BF0
void dFishMusBig_c::goToPoint(u8 point) {
    if (point == 5) {
        goToPoint5();
    } else if (point == 4) {
        goToPoint4();
    } else if (point == 3) {
        goToPoint3();
    } else if (point == 0) {
        goToPoint0();
    } else if (point == 1) {
        goToPoint1();
    } else if (point == 2) {
        goToPoint2();
    }
}

// 0x7C84
void dFishMusBig_c::goToPoint5() {}

// 0x7C88
void dFishMusBig_c::goToPoint4() {}

// 0x7C8C
void dFishMusBig_c::goToPoint3() {}

// 0x7C90
void dFishMusBig_c::goToPoint0() {}

// 0x7C94
void dFishMusBig_c::goToPoint1() {}

// 0x7C98
void dFishMusBig_c::goToPoint2() {}

// 0x7C9C
void dFishMusBig_c::setRandomHeight() {
    if (isFishKind(FISH_BLACK)) {
        if (cM::rndRange(0.0f, 100.0f) < 50.0f) {
            mHeight = mpData->mSwimYMax;
        } else {
            mHeight = mpData->mSwimYMin;
        }
    } else if (isFishKind(FISH_ITOU) || isFishKind(FISH_SAKE) || isFishKind(FISH_KING) ||
               isFishKind(FISH_AROWANA) || isFishKind(FISH_DORADO)) {
        mHeight = cM::rndRange(mpData->mSwimYMin, mpData->mSwimYMax);
    }
}

// 0x7D54
void dFishMusBig_c::addOfs(mVec3_c &pos, f32 x, f32 y, f32 z) {
    pos.x += x;
    pos.y += y;
    pos.z += z;
}

// 0x7D7C
void dFishMusBig_c::calcAnmRate() {
    if (mSpeed < 0.05f) {
        sLib::addCalc2(&mAnmRate, mpData->mAnmRateSlow, 0.1f, 0.1f);
    } else {
        mAnmRate = mpData->mAnmRateFast;
    }
    mAnm.setRate(mAnmRate);
}

// 0x7DF0
void dFishMusBig_c::addPos(const mVec3_c &push) {
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}
