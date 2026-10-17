// 0x11F64
dFishMusRay_c::dFishMusRay_c()
    : mWallPush(0.0f, 0.0f, 0.0f), mLastWallPush(0.0f, 0.0f, -1.0f), mCurSwimAngle(0),
      mSwimAngle(cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX)) {}

// 0x11FE0
dFishMusRay_c::~dFishMusRay_c() {}

// 0x12038
BOOL dFishMusRay_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 4, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

// 0x12138
BOOL dFishMusRay_c::execute() {
    swim();
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

// 0x121EC
void dFishMusRay_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    setTankArea();
    mSpeed = mpData->mSpeed;
    mAnmRate = mpData->mAnmRateFast;
    mPos.x = mTankMin.x + 0.5f * (mTankMax.x - mTankMin.x);
    mPos.z = mTankMin.z + 0.5f * (mTankMax.z - mTankMin.z);
    mAngle.y = 0x4000;
    mTargetAngleX = -cM::rndRange<long>(PITCH_MIN, PITCH_MAX);
    mTargetAngleY = mSwimAngle + mAngle.y;
    mTargetAngleZ = 0;
}

// 0x122B0
void dFishMusRay_c::roll() {
    turnZ(40, 0.1f, 0.06f);
    if (mTargetAngleZ == mAngle.z) {
        mTargetAngleZ = 0;
    }
}

// 0x12308
void dFishMusRay_c::swim() {
    if (!mWallPush.isZero()) {
        turnAtWall();
    }
    checkHeight();
    turnX(50, 0.1f, 0.05f);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngleY, 50, 80, 18);
    roll();
}

// 0x123B4
void dFishMusRay_c::turnAtWall() {
    if (mTargetAngleY == mAngle.y && abs(mCurSwimAngle) == WALL_ANGLE) {
        if (mCurSwimAngle > 0) {
            mSwimAngle = cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
            mTargetAngleY = mSwimAngle - 0x4000;
        } else {
            mSwimAngle = -cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
            mTargetAngleY = mSwimAngle + 0x4000;
        }
        mCurSwimAngle = mSwimAngle;
        mLastWallPush.z = -mLastWallPush.z;
        return;
    }
    f32 margin = 0.25f * (mTankMax.x - mTankMin.x);
    if ((!isSwimmingRight() && mPos.x < mTankMin.x + margin) || (isSwimmingRight() && mTankMax.x - margin < mPos.x)) {
        if (mWallPush.z > 0.0f) {
            turnAtSideWallPlusZ();
        } else {
            turnAtSideWall();
        }
        mCurSwimAngle = mSwimAngle;
    } else {
        if (mWallPush.z > 0.0f) {
            if (isSwimmingRight()) {
                mSwimAngle = -cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
                mTargetAngleY = mSwimAngle + 0x4000;
            } else {
                mSwimAngle = cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
                mTargetAngleY = mSwimAngle - 0x4000;
            }
        } else {
            if (isSwimmingRight()) {
                mSwimAngle = cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
                mTargetAngleY = mSwimAngle + 0x4000;
            } else {
                mSwimAngle = -cM::rndRange<long>(SWIM_ANGLE_MIN, SWIM_ANGLE_MAX);
                mTargetAngleY = mSwimAngle - 0x4000;
            }
        }
        mCurSwimAngle = mSwimAngle;
    }
}

// 0x1263C
void dFishMusRay_c::checkHeight() {
    if (mPos.y < mpData->mSwimYMin) {
        mTargetAngleX = -cM::rndRange<long>(PITCH_MIN, PITCH_MAX);
    } else if (mPos.y > mpData->mSwimYMax) {
        mTargetAngleX = cM::rndRange<long>(PITCH_MIN, PITCH_MAX);
    }
}

// 0x126AC
bool dFishMusRay_c::isSwimmingRight() {
    mVec3_c right(1.0f, 0.0f, 0.0f);
    if (right.dot(mVelocity) >= 0.0f) {
        return true;
    }
    return false;
}

// 0x1270C
void dFishMusRay_c::addPos(const mVec3_c &push) {
    if (abs(mCurSwimAngle) != WALL_ANGLE && nw4r::math::FAbs(push.x) > 0.0f) {
        if (push.z > 0.0f) {
            turnAtSideWallPlusZ();
        } else if (isZero(push.z)) {
            turnAtSideWall();
        }
        mCurSwimAngle = mSwimAngle;
    } else if (mTargetAngleZ == 0 &&
               ((push.z < 0.0f && mLastWallPush.z > 0.0f) || (push.z > 0.0f && mLastWallPush.z < 0.0f))) {
        mWallPush = push;
        mLastWallPush = push;
    } else {
        mWallPush = mVec3_c::Zero;
    }
    dFishMusBase_c::addPos(push);
    mMdl.shiftTrail(mPush);
}

// 0x128DC
void dFishMusRay_c::onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push) {
    if (!push.isZero()) {
        f32 tankHeight = mTankMax.y - mTankMin.y;
        if (tankHeight - obj->getHeight() < 5.0f) {
            if (abs(mCurSwimAngle) != WALL_ANGLE) {
                turnAtSideWall();
                mCurSwimAngle = mSwimAngle;
            }
        } else if (mTargetAngleX > 0) {
            mTargetAngleX = -cM::rndRange<long>(PITCH_MIN, PITCH_MAX);
        }
    }
    dFishMusBase_c::onHitObj(obj, push);
}
