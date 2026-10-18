// dInsMusMoleCricket_c: chirps underground, jumps out when scared, walks around and digs back in.
namespace dInsectMuseum {

enum { MOLE_CRICKET_TYPE_NUM = 1 };

dInsMusMoleCricketParam_c sMoleCricketParam[MOLE_CRICKET_TYPE_NUM] = {
    {{16.0f, -10.0f, 0.0f}, 130.0f, 0.5f, 0.98f, 2.2f, -0.1f, 1.2f, 40.0f, 20.0f, 0.2f, 0.02f, 0.9f, 0.03f, 1.5f,
     10.0f, 8.0f, 80.0f, 5.0f, 6.0f, 400, 3}, // mole cricket
};

// 0xD434
const dInsMusMoleCricketParam_c *getMoleCricketParam(int type) {
    const int types[MOLE_CRICKET_TYPE_NUM] = {INSECT_MOLE_CRICKET};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sMoleCricketParam[i];
        }
    }
    return &sMoleCricketParam[0];
}

} // namespace dInsectMuseum

// 0xD468
dInsMusMoleCricket_c::dInsMusMoleCricket_c()
    : mState(NULL), mShadowSizeX(0.0f), mShadowSizeZ(0.0f), mWalkTime(0), mWalkTimer(0), mHideTime(0) {}

// 0xD4E0
dInsMusMoleCricket_c::~dInsMusMoleCricket_c() {}

// 0xD538
BOOL dInsMusMoleCricket_c::draw() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    dInsMusBase_c::draw();
    mAngle.y = mAxis.getAngleY();
    f32 alpha = 0.5f * (mAlpha / 255.0f);
    mAng angle = mAngle.y;
    dShadow::requestRound(&mPos, 0, &angle, mShadowSizeX, 1.0f, alpha, mShadowSizeZ, 1.0f);
    return TRUE;
}

// 0xD5E4
void dInsMusMoleCricket_c::calcMtx() {
    setAxisMtx();
}

// 0xD5E8
void dInsMusMoleCricket_c::init() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mShadowSize = 0.0f;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSizeZ;
    mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS;
}

// 0xD684
void dInsMusMoleCricket_c::initState() {
    initUnderground();
}

// 0xD688
void dInsMusMoleCricket_c::executeState() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mCheckBg = false;
    (this->*mState)();
}

// 0xD6D0
void dInsMusMoleCricket_c::initUnderground() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusMoleCricket_c::stateUnderground;
    mScare.clear();
    mWalkTimer = 0;
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mAxis.setIdentity();
    mShadowSizeX = 0.0f;
    mShadowSizeZ = 0.0f;
}

// 0xD798
void dInsMusMoleCricket_c::initJump() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mJumpAnmRate);
    setAnmFrame(0.0f);
    mState = &dInsMusMoleCricket_c::stateJump;
    mAngle.y = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle - DEG_TO_ANGLE(180);
    mSpeed.set(0.0f, param->mJumpSpeedY, param->mJumpSpeed);
    mSpeed.rotY(mAngle.y);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0xD898
void dInsMusMoleCricket_c::initWalk() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusMoleCricket_c::stateWalk;
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mWalkTime = (int)(mTargetPos.xzDistTo(mPrevTargetPos) / param->mWalkSpeed) + 1;
}

// 0xD9B4
void dInsMusMoleCricket_c::initDig() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusMoleCricket_c::stateDig;
    mAngle.x = mAxis.getAngleX();
    mAngle.y = mAxis.getAngleY();
    mAngle.z = mAxis.getAngleZ();
    dEffect::create("af_ins_okera_hide_room", &mPos, &mAngle, NULL);
    mShadowSizeX = 0.0f;
    mShadowSizeZ = 0.0f;
}

// 0xDAC4
void dInsMusMoleCricket_c::initHide() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusMoleCricket_c::stateHide;
    mHideTime = dInsectUtil::secToFrame(param->mHideTime);
}

// 0xDB70
void dInsMusMoleCricket_c::stateUnderground() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    if (mPlayerCheck.isValid(dPlayerMgr::PLAYER_INDEX_CURRENT) == true) {
        if (mScare.isScared() == true) {
            initJump();
        }
        if (mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, param->mChirpRadius) == true) {
            mSnd.holdSound(SE_okera_chika);
        }
    }
}

// 0xDBFC
void dInsMusMoleCricket_c::stateJump() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mSpeed.x *= param->mJumpBrake;
    mSpeed.z *= param->mJumpBrake;
    mSpeed.y += param->mGravity;
    mPos += mSpeed;
    if (mTimer > JUMP_SHADOW_TIME) {
        mShadowSizeX = param->mShadowSizeX;
        mShadowSizeZ = param->mShadowSizeZ;
    }
    if (mTimer >= JUMP_LAND_TIME && isUnderGround() == true) {
        mSpeed.set(0.0f, 0.0f, 0.0f);
        setAnmRate(0.0f);
        mPos.y = getGroundY();
        if (mTimer >= JUMP_END_TIME) {
            initWalk();
        }
    } else {
        mAngle.y = cM::atan2s(mSpeed.x, mSpeed.z);
    }
    if (mTimer == JUMP_EFFECT_TIME) {
        mVec3_c pos = mPos;
        pos.y = dBGCF::getGroundY(&mPos, FALSE);
        dEffect::create("af_ins_okera_app_room", &pos, &mAngle, NULL);
    }
    mSnd.holdSound(SE_okera_chijyou);
}

// 0xDD60
void dInsMusMoleCricket_c::stateWalk() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mCheckBg = true;
    flyToTarget(param->mGravity, param->mWalkAccel, param->mWalkBrake, param->mWalkTurnRate);
    if ((u16)dInsectUtil::secToFrame(param->mWalkTime) <= mWalkTimer) {
        if (dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].isInside(&mPos) == true) {
            initDig();
        }
    } else if (mWalkTime <= mTimer) {
        initWalk();
    }
    mWalkTimer++;
    mSnd.holdSound(SE_okera_chijyou);
}

// 0xDE38
void dInsMusMoleCricket_c::stateDig() {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mPos.y -= 0.07f;
    if (mTimer < DIG_TILT_TIME) {
        mAxis.rotateX(DEG_TO_ANGLE(1));
    }
    f32 bottomY = param->mOffset.y + getGroundY();
    mShadowSize *= 0.95f;
    if (mPos.y <= bottomY) {
        initHide();
    }
}

// 0xDEE4
void dInsMusMoleCricket_c::stateHide() {
    if (mHideTime < mTimer) {
        initUnderground();
    }
}

// 0xDEFC
mAng dInsMusMoleCricket_c::getWalkAngle() const {
    return mAngle.y.mAngle + (s16)cM::rndCenterF(0x2AAB);
}

// 0xDF48
f32 dInsMusMoleCricket_c::getWalkDist() const {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0xDF78
void dInsMusMoleCricket_c::getWalkTarget(mVec3_c *target) {
    const dInsMusMoleCricketParam_c *param = getMoleCricketParam(mType);
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}
