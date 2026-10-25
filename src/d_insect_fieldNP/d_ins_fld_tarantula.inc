// dInsFldTarantula_c: the tarantula and the scorpion. See
// include/game/game/d_ins_fld_tarantula.hpp.

const dInsFldTarantula_c::StateFunc dInsFldTarantula_c::sExecute[] = {
    &dInsFldTarantula_c::executeWait,     &dInsFldTarantula_c::executeWalk,
    &dInsFldTarantula_c::executeTurn,     &dInsFldTarantula_c::executeThreat,
    &dInsFldTarantula_c::executeTurnAway, &dInsFldTarantula_c::executeAttack,
    &dInsFldTarantula_c::executeStop,     &dInsFldTarantula_c::executeEscape,
    &dInsFldTarantula_c::executeRelease,
};

const dInsFldTarantula_c::StateFunc dInsFldTarantula_c::sInit[] = {
    &dInsFldTarantula_c::initWait,     &dInsFldTarantula_c::initWalk,
    &dInsFldTarantula_c::initTurn,     &dInsFldTarantula_c::initThreat,
    &dInsFldTarantula_c::initTurnAway, &dInsFldTarantula_c::initAttack,
    &dInsFldTarantula_c::initStop,     &dInsFldTarantula_c::initEscape,
    &dInsFldTarantula_c::onRelease,
};

f32 dInsFldTarantula_c::sAttackJump[2] = {1.6f, 0.0f};

dInsFldTarantula_c::param_c dInsFldTarantula_c::sParam[] = {
    {0.0f,  0.0f,  0.0f,  6.0f,   1.0f,  0.7f,  4.0f,  1.0f,  0.3f,
     0.03f, 0.9f,  0.02f, 1.1f,   1.3f,  0.65f, 0.5f,  0.15f, 2.2f,
     60.0f, 1.0f,  0.1f,  0.9f,   0.07f, 1.9f,  0.05f, 2.0f,  -0.15f,
     2.5f,  10.0f, 6.0f,  120.0f, 40,    80},
    {0.0f,  0.0f,  0.0f,  6.0f,   1.0f,  0.7f,  4.0f,  1.0f,  0.3f,
     0.03f, 0.9f,  0.02f, 1.3f,   1.3f,  0.65f, 0.5f,  0.15f, 3.5f,
     60.0f, 1.0f,  0.1f,  0.9f,   0.07f, 3.0f,  0.05f, 2.0f,  -0.15f,
     3.0f,  10.0f, 7.0f,  120.0f, 40,    80},
};

// 0x2250C
int dInsFldTarantula_c::getParamIdx(int type) {
    const int types[] = {INSECT_TARANTULA, INSECT_SCORPION};
    for (int i = 0; i < ARRAY_SIZE(types); i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x22540
const dInsFldTarantula_c::param_c *
dInsFldTarantula_c::getTarantulaParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x22570
dInsFldTarantula_c::dInsFldTarantula_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mWaitFrames(0),
      mWalkFrames(0), mEscapeFrames(0), mEscapeTimer(0), mWalkTimer(0),
      mWalkTime(0), mLanded(false) {
}

// 0x225D4
dInsFldTarantula_c::~dInsFldTarantula_c() {
}

// 0x2262C
int dInsFldTarantula_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x22650
void dInsFldTarantula_c::calcMtx() {
    setMtxFromAxis();
}

// 0x22654
void dInsFldTarantula_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x2268C
void dInsFldTarantula_c::setGroundPos() {
    const param_c *param = getTarantulaParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x22718
void dInsFldTarantula_c::initState() {
    setState(STATE_WAIT);
}

// 0x22720
void dInsFldTarantula_c::executeState() {
    getTarantulaParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    if (mState != STATE_RELEASE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.mValue = (int)(3.0f * (int)mScare.mLimit);
    }
}

// 0x227F4
void dInsFldTarantula_c::initWait() {
    const param_c *param = getTarantulaParam(mType);
    f32 frame = 0.0f;
    if (mPrevState == STATE_WAIT || mPrevState == STATE_WALK) {
        frame = mAnmChr.getFrame();
    }
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitMax);
    int min = dInsectUtil::secToFrame(param->mWaitMin);
    mWaitFrames = cM::rndRange<u16>(min, max);
}

// 0x228CC
void dInsFldTarantula_c::initWalk() {
    const param_c *param = getTarantulaParam(mType);
    mFrame = 0;
    mWalkTimer = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(0.0f);
    setWalkTarget();
    int max = dInsectUtil::secToFrame(param->mWalkTimeMax);
    int min = dInsectUtil::secToFrame(param->mWalkTimeMin);
    mWalkTime = cM::rndRange<u16>(min, max);
}

// 0x22980
void dInsFldTarantula_c::initTurn() {
    getTarantulaParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(2.0f);
    setAnmFrame(0.0f);
}

// 0x229FC
void dInsFldTarantula_c::initThreat() {
    getTarantulaParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    if (!isTarantula()) {
        mSound.startSound(SE_ScorpionIkaku);
    }
}

// 0x22A94
void dInsFldTarantula_c::initTurnAway() {
    getTarantulaParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
}

// 0x22B10
void dInsFldTarantula_c::initAttack() {
    const param_c *param = getTarantulaParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mAttackAnmRate);
    setAnmFrame(0.0f);
    if (isTarantula() == true && mPrevState != STATE_STOP) {
        mSound.startSound(SE_TarantulaJump);
        mVelocity.set(0.0f, sAttackJump[0], 0.0f);
    }
}

// 0x22BD4
void dInsFldTarantula_c::initStop() {
    mFrame = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mSavedScare = (int)mScare.mValue;
}

// 0x22C78
void dInsFldTarantula_c::initEscape() {
    const param_c *param = getTarantulaParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    mEscapeTimer = 0;
    setEscapeTarget();
}

// 0x22D10
void dInsFldTarantula_c::onRelease() {
    const param_c *param = getTarantulaParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x22DC8
void dInsFldTarantula_c::executeWait() {
    const param_c *param = getTarantulaParam(mType);
    mVelocity *= param->mWaitDecay;
    mVelocity.y += param->mGravity;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mScare.isEscaping() == true) {
        if (mPlayerCheck.isNetPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT) ==
            TRUE) {
            setState(STATE_ATTACK);
        } else {
            setState(STATE_ESCAPE);
        }
    } else if (mScare.isScared() == true) {
        if (dPlayerMgr::isUsingNet(dPlayerMgr::PLAYER_INDEX_CURRENT) == TRUE) {
            setState(STATE_TURN);
        } else {
            setState(STATE_ESCAPE);
        }
    } else if (mWaitFrames <= mFrame) {
        if (!dInsectUtil::isColumn(mPos)) {
            if (0.5f < cM::rnd()) {
                setState(STATE_WAIT);
            } else {
                setState(STATE_WALK);
            }
        } else {
            setState(STATE_WALK);
        }
    }
}

// 0x22F8C
void dInsFldTarantula_c::executeWalk() {
    const param_c *param = getTarantulaParam(mType);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    if (isInWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.9f, 0.9f, 0.9f);
        dEffect::create("af_hny_watercolumn_M", &mPos, NULL, &scale);
        dEffect::create("af_hny_mizutama_M", &mPos, NULL, &scale,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
        alive = false;
#endif
    }
    if (mWalkFrames <= mFrame) {
        setWalkTarget();
    } else if (mScare.isEscaping() == true) {
        if (mPlayerCheck.isNetPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT) ==
            TRUE) {
            setState(STATE_ATTACK);
        } else {
            setState(STATE_ESCAPE);
        }
    } else if (mScare.isScared() == true) {
        if (mPlayerCheck.isNetPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT) ==
            TRUE) {
            setState(STATE_TURN);
        } else {
            setState(STATE_ESCAPE);
        }
    } else if (mWalkTime <= mWalkTimer) {
        if (0.5f < cM::rnd()) {
            setState(STATE_WAIT);
        } else {
            setState(STATE_WALK);
        }
    }
    mWalkTimer++;
    if (isTarantula() == true) {
        mSound.holdSound(SE_TarantulaRun);
    } else {
        mSound.holdSound(SE_ScorpionRun);
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x231D0
void dInsFldTarantula_c::executeThreat() {
    const param_c *param = getTarantulaParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c toPlayer = player - mPos;
    mVec3_c front = mAxis.getFront();
    mVelocity.x = 0.0f;
    mVelocity.y += param->mGravity;
    mVelocity.z = 0.0f;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    if (front.x * toPlayer.x + front.z * toPlayer.z < 0.86f) {
        setState(STATE_TURN);
    } else if (mScare.isEscaping() == true) {
        setState(STATE_ATTACK);
    } else if (!mScare.isScared()) {
        setState(STATE_TURN_AWAY);
    }
}

// 0x23334
void dInsFldTarantula_c::executeTurn() {
    const param_c *param = getTarantulaParam(mType);
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c dir = player - mPos;
    dir.normalize();
    mVec3_c front = mAxis.getFront();
    front += (dir - front) * param->mTurnRate;
    front.normalize();
    mAxis.setFront(front);
    alignToGround();
    mVelocity.x = 0.0f;
    mVelocity.y += param->mGravity;
    mVelocity.z = 0.0f;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (0.94f < front.dot(dir)) {
        if (mScare.isEscaping() == true) {
            setState(STATE_ATTACK);
        } else if (!mScare.isScared()) {
            setState(STATE_TURN_AWAY);
        } else {
            setState(STATE_THREAT);
        }
    }
    if (isTarantula() == true) {
        mSound.holdSound(SE_TarantulaRun);
    } else {
        mSound.holdSound(SE_ScorpionRun);
    }
}

// 0x23548
void dInsFldTarantula_c::executeTurnAway() {
    const param_c *param = getTarantulaParam(mType);
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c dir = mPos - player;
    dir.normalize();
    mVec3_c front = mAxis.getFront();
    front += (dir - front) * param->mTurnRate;
    front.normalize();
    mAxis.setFront(front);
    alignToGround();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (0.94f < front.dot(dir)) {
        setState(STATE_WALK);
    } else if (mScare.isScared() == true) {
        setState(STATE_TURN);
    }
}

// 0x236A0
void dInsFldTarantula_c::executeAttack() {
    const param_c *param = getTarantulaParam(mType);
    if (!mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, 16.0f)) {
        mPlayerCheck.getPlayerPos(&mHomePos, dPlayerMgr::PLAYER_INDEX_CURRENT);
        walk(param->mGravity, param->mAttackAccel, param->mAttackDrag,
             param->mAttackTurn);
    } else {
        mVelocity.x = 0.0f;
        mVelocity.y += param->mGravity;
        mVelocity.z = 0.0f;
        if (!dPlayerMgr::isSafeFromInsects()) {
            if (isTarantula() == true) {
                dPlayerMgr::requestFaint(FALSE);
            } else {
                dPlayerMgr::requestFaint(TRUE);
            }
        }
    }
    if (mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT,
                            param->mSafeRadius) == true &&
        dPlayerMgr::isSafeFromInsects() == true) {
        setState(STATE_STOP);
    }
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    if (!mScare.isEscaping()) {
        setState(STATE_TURN);
    }
    if (isInWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.9f, 0.9f, 0.9f);
        dEffect::create("af_hny_watercolumn_M", &mPos, NULL, &scale);
        dEffect::create("af_hny_mizutama_M", &mPos, NULL, &scale,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
        alive = false;
#endif
    }
    if (isTarantula() == true) {
        mSound.holdSound(SE_TarantulaRun);
    } else {
        mSound.holdSound(SE_ScorpionRun);
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x238F4
void dInsFldTarantula_c::executeStop() {
    const param_c *param = getTarantulaParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    mVec3_c player;
    mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c toPlayer = player - mPos;
    mVec3_c front = mAxis.getFront();
    mVelocity.x = 0.0f;
    mVelocity.y += param->mGravity;
    mVelocity.z = 0.0f;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    mScare.mValue = (int)mSavedScare;
    if (!dPlayerMgr::isSafeFromInsects()) {
        setState(STATE_ATTACK);
    }
}

// 0x23A2C
void dInsFldTarantula_c::executeEscape() {
    const param_c *param = getTarantulaParam(mType);
    walk(param->mGravity, param->mEscapeAccel, param->mEscapeDrag,
         param->mEscapeTurn);
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    mEscapeTimer++;
    if ((u16)dInsectUtil::secToFrame(param->mEscapeTime) <= mEscapeTimer) {
        fadeOut();
        if (mAlpha == 0) {
            requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
            alive = false;
#endif
        }
    } else if (mEscapeFrames <= mFrame) {
        setEscapeTarget();
    }
    if (mPlayerCheck.isNetPlayer(dPlayerMgr::PLAYER_INDEX_CURRENT) == TRUE) {
        setState(STATE_ATTACK);
    }
    if (isInWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.9f, 0.9f, 0.9f);
        dEffect::create("af_hny_watercolumn_M", &mPos, NULL, &scale);
        dEffect::create("af_hny_mizutama_M", &mPos, NULL, &scale,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
        alive = false;
#endif
    }
    if (isTarantula() == true) {
        mSound.holdSound(SE_TarantulaRun);
    } else {
        mSound.holdSound(SE_ScorpionRun);
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x23C00
void dInsFldTarantula_c::executeRelease() {
    const param_c *param = getTarantulaParam(mType);
    const mVec3_c front = mAxis.getFront();
    mHomePos = mPos + front * 100.0f;
    setAnmRate(param->mReleaseAnmRate);
    walk(param->mGravity, param->mEscapeAccel, param->mEscapeDrag,
         param->mEscapeTurn);
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
        if (!mLanded) {
            mSound.startSound(SE_mushi_out_run);
            mLanded = true;
        }
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
    if (isInWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.9f, 0.9f, 0.9f);
        dEffect::create("af_hny_watercolumn_M", &mPos, NULL, &scale);
        dEffect::create("af_hny_mizutama_M", &mPos, NULL, &scale,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
}

// 0x23DAC
mAng dInsFldTarantula_c::getMoveAngle() const {
    if (mState == STATE_ESCAPE) {
        s16 away =
            mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle +
            INSECT_FIELD_FLEE_ANGLE;
        s16 npcAway = away;
        if (mNpcCheck.isNear(mScareRadius) == true) {
            npcAway = mNpcCheck.getAngleTo().mAngle + INSECT_FIELD_FLEE_ANGLE;
        }
        return (away + npcAway) >> 1;
    }
    if (mState == STATE_ATTACK) {
        return mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
    }
    return mAngle.y.mAngle + (int)cM::rndCenterF(14564.0f);
}

// 0x23E88
void dInsFldTarantula_c::getMoveTarget(mVec3_c *out) {
    getTarantulaParam(mType);
    mAng ang = getMoveAngle();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, 50.0f);
    target.rotY(ang);
    target += mPos;
    mVec3_c from = mPos;
    mVec3_c to = target;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, 2, FALSE, TRUE, -30.0f,
                                       5.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 0, FALSE);
    }
    *out = pos;
}

// 0x23FBC
bool dInsFldTarantula_c::isTarantula() {
    return mType == INSECT_TARANTULA;
}

// 0x23FD0
bool dInsFldTarantula_c::isInWater() const {
    mVec3_c pos = mPos;
    pos.z -= 6.0f;
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 1, 0);
    if (chk.isUnderWater(mPos.y) == true) {
        return true;
    }
    return false;
}

// 0x24054
void dInsFldTarantula_c::setWalkTarget() {
    const param_c *param = getTarantulaParam(mType);
    getMoveTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    f32 speed = param->mWalkSpeed;
    mFrame = 0;
    mWalkFrames = (int)(dist / speed) + 1;
}

// 0x240E8
void dInsFldTarantula_c::setEscapeTarget() {
    const param_c *param = getTarantulaParam(mType);
    getMoveTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    f32 speed = param->mEscapeSpeed;
    mFrame = 0;
    mEscapeFrames = (int)(dist / speed) + 1;
}

// 0x2417C
void dInsFldTarantula_c::walk(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    alignToGround();
}

// 0x241B0
void dInsFldTarantula_c::alignToGround() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    mVec3_c up = chk.mFloor.mNormal;
    up.normalize();
    mAxis.setUp(up);
}
