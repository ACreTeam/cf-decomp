// dInsMusMosquito_c: the mosquito of the museum's insect rooms, circling and biting the player.

namespace dInsectMuseum {

enum { MOSQUITO_TYPE_NUM = 1 };

dInsMusMosquitoParam_c sMosquitoParam[MOSQUITO_TYPE_NUM] = {
    {{-64.0f, 25.0f, 0.0f}, 30.0f, 20.0f, 0.1f, 3.0f, 2.0f, 2.0f, 1.0f, 1.0f, 30.0f, 14.0f, 0.25f, 2.0f, 0.0f},
};

// 0x10CA0
int getMosquitoParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_MOSQUITO:
        idx = 0;
        break;
    }
    return idx;
}

// 0x10CB4
const dInsMusMosquitoParam_c *getMosquitoParam(int type) {
    return &sMosquitoParam[getMosquitoParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x10CE4
dInsMusMosquito_c::dInsMusMosquito_c()
    : mState(NULL), mBaseY(0.0f), mBobAngle(0), mBobHeight(0.0f), mFlyTime(0), _22A(0), _22C(0), mBitten(false),
      mCircleAngle(0), mPlayerPos(0.0f, 0.0f, 0.0f), mCircleRadius(0.0f), mRollAngle(0), mClockwise(false) {}

// 0x10D80
dInsMusMosquito_c::~dInsMusMosquito_c() {}

// 0x10DD8
BOOL dInsMusMosquito_c::draw() {
    dInsMusBase_c::draw();
    return TRUE;
}

// 0x10DFC
void dInsMusMosquito_c::init() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y = param->mOffset.y + getGroundY();
    mPos.z += param->mOffset.z;
    mBaseY = mPos.y;
    mScareRadius = 0.0f;
    mBgRadius = 1.0f;
}

// 0x10E88
void dInsMusMosquito_c::initState() {
    initFly();
}

// 0x10E8C
void dInsMusMosquito_c::executeState() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    (this->*mState)();
}

// 0x10ECC
void dInsMusMosquito_c::initFly() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    mTimer = 0;
    mState = &dInsMusMosquito_c::stateFly;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    getFlyTarget(&mTargetPos);
    mFlyTime = mTargetPos.xzDistTo(mPos) / param->mSpeed;
}

// 0x10FBC
void dInsMusMosquito_c::initFollow() {
    mTimer = 0;
    mState = &dInsMusMosquito_c::stateFollow;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mSpeed.z = getMosquitoParam(mType)->mSpeed;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
}

// 0x11064
void dInsMusMosquito_c::initCircle() {
    mTimer = 0;
    mState = &dInsMusMosquito_c::stateCircle;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mCircleAngle = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle - DEG_TO_ANGLE(180);
    mPlayerCheck.getPlayerPos(&mPlayerPos, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mCircleRadius = mPlayerPos.xzDistTo(mPos);
    mClockwise = isCircleClockwise();
}

// 0x11154
void dInsMusMosquito_c::stateFly() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    mSnd.holdSound(SE_mosqueto);
    dInsectUtil::moveTo(&mPos, &mAngle.y, mTargetPos, param->mSpeed, param->mTurnDeg);
    bob();
    mRollAngle += mAng::fromDegree(param->mRollSpeed);
    mAngle.x = mAng(mRollAngle).sin() * mAng::fromDegree(param->mRollMax);
    if (mFlyTime <= mTimer) {
        if (isPlayerNear() == true) {
            initFollow();
        } else {
            initFly();
        }
    }
}

// 0x11288
void dInsMusMosquito_c::stateFollow() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    mSnd.holdSound(SE_mosqueto);
    if (mPlayerCheck.isValid(dPlayerMgr::PLAYER_INDEX_CURRENT) == true) {
        mVec3_c oldPos = mPos;
        mPlayerCheck.getPlayerPos(&mTargetPos, dPlayerMgr::PLAYER_INDEX_CURRENT);
        dInsectUtil::moveTo(&mPos, &mAngle.y, mTargetPos, param->mSpeed, param->mTurnDeg);
        mSpeed = mPos - oldPos;
        mSpeed.y = 0.0f;
        mRollAngle += mAng::fromDegree(param->mRollSpeed);
        mAngle.x = mAng(mRollAngle).sin() * mAng::fromDegree(param->mRollMax);
    }
    bob();
    if (!isPlayerNear()) {
        initFly();
    } else if (isPlayerClose() == true) {
        initCircle();
    }
}

// 0x1142C
void dInsMusMosquito_c::stateCircle() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    mVec3_c oldPos = mPos;
    mSnd.holdSound(SE_mosqueto);
    if (mClockwise == true) {
        mCircleAngle += mAng::fromDegree(param->mCircleSpeed);
    } else {
        mCircleAngle -= mAng::fromDegree(param->mCircleSpeed);
    }
    mRollAngle += mAng::fromDegree(param->mRollSpeed);
    mAngle.x = mAng(mRollAngle).sin() * mAng::fromDegree(param->mRollMax);
    mVec3_c offset(0.0f, 0.0f, mCircleRadius);
    offset.rotY(mCircleAngle);
    f32 y = mPos.y;
    mPos = mPlayerPos + offset;
    mPos.y = y;
    if (!isPlayerClose()) {
        initFollow();
    }
    mAngle.y = cLib::targetAngleY(oldPos, mPos);
    bob();
    if (mTimer >= BITE_WAIT_TIME && !mBitten && !dNet::isSessionActive() && dPlayerMgr::requestInsectBite() == TRUE) {
        mBitten = true;
        initFly();
    }
}

// 0x11660
void dInsMusMosquito_c::bob() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    f32 baseY = param->mOffset.y + getGroundY();
    if (baseY < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < baseY ? baseY : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > baseY ? baseY : mBaseY;
    }
    s16 next = mBobAngle + mAng::fromDegree(param->mBobSpeed);
    if (mBobAngle > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (mBobAngle <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mAng(mBobAngle).sin();
    mBobAngle += mAng::fromDegree(param->mBobSpeed);
}

// 0x117C8
mAng dInsMusMosquito_c::getFlyAngle() {
    return cM::rndCenterF(mAng(DEG_TO_ANGLE(120)));
}

// 0x1181C
f32 dInsMusMosquito_c::getFlyDist() const {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x1184C
void dInsMusMosquito_c::getFlyTarget(mVec3_c *target) {
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    result = pos;
    *target = result;
}

// 0x11910
bool dInsMusMosquito_c::isPlayerNear() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    return mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, 320.0f) == true;
}

// 0x11960
bool dInsMusMosquito_c::isPlayerClose() {
    const dInsMusMosquitoParam_c *param = getMosquitoParam(mType);
    return mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, param->mCloseRadius) == true;
}

// 0x119B0
bool dInsMusMosquito_c::isCircleClockwise() {
    mVec3_c dir = mPos - mPlayerPos;
    f32 speedX = mSpeed.x; // copies before __fabsf needed to match
    f32 speedZ = mSpeed.z;
    speedX = __fabsf(speedX);
    speedZ = __fabsf(speedZ);
    if (dir.x < 0.0f) {
        if (dir.z < 0.0f) {
            if (speedX < speedZ) {
                return true;
            }
            return false;
        }
        if (speedX < speedZ) {
            return false;
        }
        return true;
    }
    if (dir.z < 0.0f) {
        if (speedX < speedZ) {
            return false;
        }
        return true;
    }
    return speedX < speedZ;
}
