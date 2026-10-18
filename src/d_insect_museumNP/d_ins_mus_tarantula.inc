// dInsMusTarantula_c: the tarantula and the scorpion of the museum's insect rooms.

namespace dInsectMuseum {

enum { TARANTULA_TYPE_NUM = 2 };

f32 sTarantulaCircleSpeed[2] = {0.3f, 0.0f};

dInsMusTarantulaParam_c sTarantulaParam[TARANTULA_TYPE_NUM] = {
    {{-32.0f, 1.0f, 0.0f}, 6.0f, 1.0f, 4.0f, 1.0f, 0.2f, 0.02f, 0.9f, 0.02f, 2.5f, 2.0f, 2.0f, 1.0f, 2.3f, 0.8f, 0.95f,
     0.2f, 2.0f, 10.0f, 60.0f, 100, 14}, // tarantula
    {{-64.0f, 1.0f, 0.0f}, 6.0f, 1.0f, 4.0f, 1.0f, 0.15f, 0.015f, 0.9f, 0.02f, 2.5f, 2.0f, 2.0f, 1.0f, 2.3f, 0.8f, 0.95f,
     0.2f, 2.0f, 10.0f, 60.0f, 100, 14}, // scorpion
};

// 0x12138
int getTarantulaParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_TARANTULA:
        idx = 0;
        break;
    case INSECT_SCORPION:
        idx = 1;
        break;
    }
    return idx;
}

// 0x12168
const dInsMusTarantulaParam_c *getTarantulaParam(int type) {
    return &sTarantulaParam[getTarantulaParamIdx(type)];
}

dInsMusTarantula_c::Pose_e sTarantulaPose;
dInsMusTarantula_c::Pose_e sScorpionPose;

} // namespace dInsectMuseum

// 0x12198
dInsMusTarantula_c::dInsMusTarantula_c()
    : mState(NULL), mWaitTime(0), mWalkTime(0), mCircleTime(0), mStandTime(0), mRunSpeed(0.0f), mBackAngle(0),
      mCircleAngle(0), mCircleRadius(0.0f), mCircleCenter(0.0f, 0.0f, 0.0f) {}

// 0x12228
dInsMusTarantula_c::~dInsMusTarantula_c() {}

// 0x12280
BOOL dInsMusTarantula_c::draw() {
    dInsMusBase_c::draw();
    return TRUE;
}

// 0x122A4
void dInsMusTarantula_c::calcMtx() {
    setAxisMtx();
}

// 0x122A8
void dInsMusTarantula_c::init() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y = param->mOffset.y + getGroundY();
    mPos.z += param->mOffset.z;
    if (isTarantula() == true) {
        mCircleAngle = -DEG_TO_ANGLE(180);
    }
    mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS;
    mShadowSize = param->mShadowSize;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x12388
void dInsMusTarantula_c::initState() {
    if (isPairDonated() == true) {
        initCircle();
    } else {
        initWait();
    }
}

// 0x123D0
void dInsMusTarantula_c::executeState() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    (this->*mState)();
    if (isPairDonated() == true && !isTarantula() && sScorpionPose != sTarantulaPose) {
        sScorpionPose = sTarantulaPose;
        switch (sScorpionPose) {
        case POSE_JUMP:
            initJump();
            break;
        case POSE_RUN:
            initRun();
            break;
        case POSE_CIRCLE:
            initCircle();
            break;
        case POSE_STAND:
            initStand();
            break;
        }
    }
}

// 0x124A0
void dInsMusTarantula_c::initWait() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateWait;
    setAnmChr(mAnmChrStop, 20.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
    mStateId = STATE_WAIT;
}

// 0x12574
void dInsMusTarantula_c::initWalk() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateWalk;
    if (mStateId == STATE_WAIT) {
        setAnmChr(mAnmChrWait, 20.0f);
        setAnmVis(mAnmVisWait);
    } else {
        f32 frame = mAnmChr.getFrame();
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
        setAnmFrame(frame);
    }
    setAnmRate(param->mWalkAnmRate);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWalkTime = cM::rndRange<u16>(min, max);
    getWalkTarget(&mTargetPos);
    mStateId = STATE_WALK;
}

// 0x12690
void dInsMusTarantula_c::initTurnToPlayer() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateTurnToPlayer;
    setAnmChr(mAnmChrWait, 20.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mTurnAnmRate);
    setAnmFrame(0.0f);
    mAngle.y = mAxis.getAngleY();
    mStateId = STATE_TURN_TO_PLAYER;
}

// 0x12744
void dInsMusTarantula_c::initThreat() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateThreat;
    setAnmChr(mAnmChrStop, 30.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(param->mThreatAnmRate);
    setAnmFrame(0.0f);
    mStateId = STATE_THREAT;
}

// 0x127E8
void dInsMusTarantula_c::initTurnBack() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateTurnBack;
    setAnmChr(mAnmChrWait, 30.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mTurnAnmRate);
    setAnmFrame(0.0f);
    mStateId = STATE_TURN_BACK;
    mBackAngle = mAngle.y.mAngle - DEG_TO_ANGLE(180);
}

// 0x12898
void dInsMusTarantula_c::initJump() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    sTarantulaPose = POSE_JUMP;
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateJump;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mSpeed.set(0.0f, param->mJumpSpeedY, param->mJumpSpeedZ);
    mSpeed.rotY(mAngle.y.mAngle - DEG_TO_ANGLE(180));
    mAxis.rotateX(DEG_TO_ANGLE(10));
}

// 0x12984
void dInsMusTarantula_c::initRun() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    sTarantulaPose = POSE_RUN;
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateRun;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    static const f32 runAnmRate = 5.0f;
    setAnmRate(runAnmRate);
    setAnmFrame(0.0f);
    mRunSpeed = 0.0f;
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x12A4C
void dInsMusTarantula_c::initCircle() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateCircle;
    setAnmChr(mAnmChrWait, 20.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(3.0f);
    setAnmFrame(0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    sTarantulaPose = POSE_CIRCLE;
    mCircleTime = CIRCLE_TIME;
}

// 0x12B14
void dInsMusTarantula_c::initStand() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    sTarantulaPose = POSE_STAND;
    mTimer = 0;
    mState = &dInsMusTarantula_c::stateStand;
    setAnmChr(mAnmChrStop, 10.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mStandTime = STAND_TIME;
}

// 0x12BC0
void dInsMusTarantula_c::stateWait() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mSpeed *= 0.7f;
    mPos += mSpeed;
    if (mScare.isScared() == true) {
        initThreat();
    }
    mPos.y = param->mOffset.y + getGroundY();
    if (mWaitTime <= mTimer) {
        if (0.5f < cM::rnd()) {
            initWait();
        } else {
            initWalk();
        }
    }
}

// 0x12CB0
void dInsMusTarantula_c::stateWalk() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    flyToTarget(0.0f, param->mWalkSpeed, param->mWalkBrake, param->mWalkTurnRate);
    mPos.y = param->mOffset.y + getGroundY();
    if (mWalkTime <= mTimer) {
        if (0.5f < cM::rnd()) {
            initWait();
        } else {
            initWalk();
        }
    } else if (mScare.isScared() == true) {
        initThreat();
    }
    holdRunSound();
}

// 0x12D9C
void dInsMusTarantula_c::stateThreat() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    s16 diff = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle - mAngle.y.mAngle;
    if ((s16)nw4r::math::FAbs(diff) > DEG_TO_ANGLE(30)) {
        initTurnToPlayer();
    } else if (!mScare.isScared()) {
        initTurnBack();
    }
    if (!isTarantula()) {
        mSnd.holdSound(SE_ScorpionIkaku);
    }
}

// 0x12E68
void dInsMusTarantula_c::stateTurnToPlayer() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mAng target = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
    sLib::addCalcAngle(&mAngle.y.mAngle, target, 20, mAng::fromDegree(param->mTurnSpeed), DEG_TO_ANGLE(1));
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (target.mAngle == mAngle.y.mAngle) {
        initThreat();
    } else if (!mScare.isScared()) {
        initTurnBack();
    }
    holdRunSound();
}

// 0x12F6C
void dInsMusTarantula_c::stateTurnBack() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    sLib::addCalcAngle(&mAngle.y.mAngle, mBackAngle, 20, mAng::fromDegree(param->mTurnSpeed), DEG_TO_ANGLE(1));
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (mBackAngle.mAngle == mAngle.y.mAngle) {
        initWalk();
    } else if (mScare.isScared() == true) {
        initTurnToPlayer();
    }
    holdRunSound();
}

// 0x1304C
void dInsMusTarantula_c::stateJump() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mSpeed.y += -0.164f;
    mSpeed.x *= param->mJumpBrake;
    mSpeed.z *= param->mJumpBrake;
    mPos += mSpeed;
    if (isUnderGround() == true) {
        mPos.y = param->mOffset.y + getGroundY();
        if (isTarantula() == true) {
            initCircle();
        }
    }
    if (mTimer == JUMP_SOUND_TIME && isTarantula() == true) {
        mSnd.startSound(SE_InsectBattle_InsectCrush);
    }
}

// 0x13144
void dInsMusTarantula_c::stateRun() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mAngle.x -= DEG_TO_ANGLE(1);
    mRunSpeed += param->mRunAccel;
    mRunSpeed = mRunSpeed > param->mRunSpeedMax ? param->mRunSpeedMax : mRunSpeed;
    mSpeed.set(0.0f, 0.0f, mRunSpeed);
    mSpeed.rotY(mAngle.y);
    mPos += mSpeed;
    if (mTimer >= RUN_TIME && isTarantula() == true) {
        initJump();
    }
    holdRunSound();
}

// 0x13254
void dInsMusTarantula_c::stateCircle() {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mCircleRadius = 16.0f;
    mCircleCenter.set(370.0f, 0.0f, 610.0f);
    mCircleCenter.y = getGroundY();
    mCircleAngle += mAng::fromDegree(sTarantulaCircleSpeed[0]);
    mVec3_c offset(0.0f, 0.0f, mCircleRadius);
    offset.rotY(mCircleAngle);
    mPos = mCircleCenter + offset;
    mPos.y = param->mOffset.y + getGroundY();
    mAngle.y = cLib::targetAngleY(mPos, mCircleCenter);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (mCircleTime < mTimer && isTarantula() == true) {
        initStand();
    }
    holdRunSound();
}

// 0x133DC
void dInsMusTarantula_c::stateStand() {
    if (mStandTime <= mTimer && isTarantula() == true) {
        if (0.7f < cM::rnd()) {
            initRun();
        } else if (0.5f < cM::rnd()) {
            initCircle();
        } else {
            initStand();
        }
    }
    if (!isTarantula()) {
        mSnd.holdSound(SE_ScorpionIkaku);
    }
}

// 0x13484
mAng dInsMusTarantula_c::getWalkAngle() {
    s16 angle = mAngle.y.mAngle;
    return angle + (int)cM::rndCenterF(0x38E4);
}

// 0x134CC
void dInsMusTarantula_c::getWalkTarget(mVec3_c *target) {
    const dInsMusTarantulaParam_c *param = getTarantulaParam(mType);
    mAng angle = getWalkAngle();
    f32 dist = mWalkTime * param->mWalkDist;
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    result = pos; // dead store, needed to match
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}

// 0x13600
bool dInsMusTarantula_c::isTarantula() {
    return mType == INSECT_TARANTULA;
}

// 0x13614
bool dInsMusTarantula_c::isPairDonated() {
    dItem::Item tarantula(dItem::ITEM_IDX_COMMON_BUTTERFLY, INSECT_TARANTULA, FALSE);
    dItem::Item scorpion(dItem::ITEM_IDX_COMMON_BUTTERFLY, INSECT_SCORPION, FALSE);
    dMuseum_c &museum = dSaveData_c::getTown()->mMuseum;
    u32 hasTarantula = museum.isDonated(tarantula);
    u32 hasScorpion = museum.isDonated(scorpion);
    if (hasTarantula == TRUE && hasScorpion == TRUE) {
        return true;
    }
    return false;
}
