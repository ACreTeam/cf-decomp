// dInsFldGrasshopper_c: the hopping insects. See
// include/game/game/d_ins_fld_grasshopper.hpp.

const dInsFldGrasshopper_c::StateFunc dInsFldGrasshopper_c::sExecute[] = {
    &dInsFldGrasshopper_c::executeWait,
    &dInsFldGrasshopper_c::executeTurn,
    &dInsFldGrasshopper_c::executeJump,
    &dInsFldGrasshopper_c::executeEscapeTurn,
    &dInsFldGrasshopper_c::executeEscapeJump,
    &dInsFldGrasshopper_c::executeLeave,
};

const dInsFldGrasshopper_c::StateFunc dInsFldGrasshopper_c::sInit[] = {
    &dInsFldGrasshopper_c::initWait,
    &dInsFldGrasshopper_c::initTurn,
    &dInsFldGrasshopper_c::initJump,
    &dInsFldGrasshopper_c::initEscapeTurn,
    &dInsFldGrasshopper_c::initEscapeJump,
    &dInsFldGrasshopper_c::onRelease,
};

// .data AB8
dInsFldGrasshopper_c::param_c dInsFldGrasshopper_c::sParam[TYPE_NUM] = {
    {0.0f, 0.0f,  12.0f, 5.0f,  1.0f,   0.0f,  1.2f,  60.0f, 45.0f, 0.9f,
     1.0f, -0.2f, 1.0f,  30,    0,      60.0f, 45.0f, 0.93f, 2.0f,  -0.08f,
     1.0f, 40,    0,     0.92f, -0.13f, 5.0f,  5.0f,  80.0f, 100,   0},
    {0.0f, 0.0f,  12.0f, 5.0f,  1.0f,   0.0f,  1.5f,  60.0f, 45.0f, 0.9f,
     1.0f, -0.2f, 1.0f,  30,    0,      60.0f, 45.0f, 0.93f, 2.0f,  -0.08f,
     1.0f, 40,    0,     0.92f, -0.13f, 6.0f,  4.0f,  80.0f, 70,    0},
    {0.0f, 0.0f,  12.0f, 5.0f,  1.0f,  1.0f,  1.5f,  30.0f, 15.0f, 0.9f,
     1.0f, -0.1f, 1.0f,  30,    0,     60.0f, 30.0f, 0.93f, 2.0f,  -0.08f,
     1.0f, 40,    0,     0.93f, -0.1f, 3.5f,  4.0f,  80.0f, 30,    100},
    {0.0f, 0.0f,  12.0f, 5.0f,  1.0f,  1.0f,  1.2f,  30.0f, 15.0f, 0.9f,
     1.0f, -0.1f, 1.0f,  30,    0,     60.0f, 30.0f, 0.93f, 2.0f,  -0.08f,
     1.0f, 40,    0,     0.93f, -0.1f, 4.0f,  4.0f,  80.0f, 30,    100},
    {0.0f, 0.0f,   12.0f, 5.0f,  1.0f,    1.0f,   1.2f,  60.0f, 30.0f, 0.9f,
     1.0f, -0.15f, 1.0f,  30,    0,       120.0f, 60.0f, 0.93f, 2.0f,  -0.08f,
     1.0f, 40,     0,     0.92f, -0.135f, 5.0f,   4.0f,  80.0f, 30,    100},
};

// .bss 8
mAng dInsFldGrasshopper_c::sJumpAngleRange(0x2AAB);

// .text C5A8
int dInsFldGrasshopper_c::getParamIdx(int type) {
    const int types[] = {
        INSECT_LONG_LOCUST,  INSECT_MIGRATORY_LOCUST, INSECT_CRICKET,
        INSECT_BELL_CRICKET, INSECT_GRASSHOPPER,
    };
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text C618
const dInsFldGrasshopper_c::param_c *
dInsFldGrasshopper_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text C648
int dInsFldGrasshopper_c::getChirpSound(int type) {
    const int sounds[] = {SE_shouryoubatta, SE_tonosamabatta, SE_koorogi,
                          SE_suzumushi, SE_kirigirisu};
    return sounds[getParamIdx(type)];
}

// .text C6A0
dInsFldGrasshopper_c::dInsFldGrasshopper_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mHasJumped(0), mWaitTime(0),
      mJumpTime(0), mEscapeTime(0), mTargetAngle(0) {
}

// .text C6FC
dInsFldGrasshopper_c::~dInsFldGrasshopper_c() {
}

// .text C754
int dInsFldGrasshopper_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text C778
void dInsFldGrasshopper_c::calcMtx() {
    setMtxFromAxis();
}

// .text C77C
void dInsFldGrasshopper_c::onBecomeOwner() {
    const param_c *param = getKindParam(mType);
    if (mPos.y < dBGCF::getGroundY(&mPos, TRUE)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = param->mOfsY + dBGCF::getGroundY(&pos, FALSE);
        mPos = pos;
        mOldPos = mPos;
        mVelocity.set(0.0f, 0.0f, 0.0f);
    }
}

// .text C8A8
void dInsFldGrasshopper_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPosX(mHomePos.x);
            data->setPosZ(mHomePos.z);
            data->setReq(mState);
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
        }
    }
}

// .text CA38
void dInsFldGrasshopper_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x += param->mOfsX;
    mPos.y = param->mOfsY + getGroundY();
    mPos.z += param->mOfsZ;
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// .text CAC4
void dInsFldGrasshopper_c::initState() {
    setState(STATE_WAIT);
}

// .text CACC
void dInsFldGrasshopper_c::executeState() {
    getKindParam(mType);
    mOldPos = mPos;
    mPrevState = mState;
    (this->*sExecute[mState])();
    checkState();
    mBgCheck = FALSE;
    if (dNet::isSessionActive() == true) {
        if (dNet::isSceneOwner() == TRUE) {
            mAcch.checkSteps(mShadowSize, &mPos, &mOldPos, mAngle.y, 0,
                             dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                                 dBGCF::CHECK_SET_POS);
        }
    } else {
        mAcch.checkSteps(mShadowSize, &mPos, &mOldPos, mAngle.y, 0,
                         dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                             dBGCF::CHECK_SET_POS);
    }
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
    }
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// .text CBE8
void dInsFldGrasshopper_c::initWait() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    if (isChirper() == true) {
        setAnmChr(mAnmChrStop, 0.0f);
        setAnmVis(mAnmVisStop);
        setAnmRate(param->mChirpAnmRate);
        setAnmFrame(0.0f);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
        setAnmRate(0.0f);
        setAnmFrame(0.0f);
    }
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mPos.y = getGroundY();
}

// .text CCFC
void dInsFldGrasshopper_c::initTurn() {
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.2f);
    setAnmFrame(0.0f);
    getJumpTarget(&mHomePos);
    mTargetAngle = cLib::targetAngleY(mPos, mHomePos);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mPos.y = getGroundY();
}

// .text CDA4
void dInsFldGrasshopper_c::initJump() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mJumpAnmRate);
    setAnmFrame(0.0f);
    mJumpTime = dInsectUtil::getDecayFrames(param->mJumpDecay);
    f32 dist = (mHomePos - mPos).xzLen();
    f32 speedY = getJumpSpeedY(param->mJumpGravity, mJumpTime);
    f32 speed = getJumpSpeed(dist, param->mJumpDecay);
    mAngle.y = cLib::targetAngleY(mPos, mHomePos);
    mVelocity.set(0.0f, speedY, speed);
    mVelocity.rotY(mAngle.y);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    const mVec3_c normal = getGroundNormal(&mPos);
    mAxis.setUp(normal);
}

// .text CF10
void dInsFldGrasshopper_c::initEscapeTurn() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mTurnAnmRate);
    setAnmFrame(0.0f);
    getJumpTarget(&mHomePos);
    mTargetAngle = cLib::targetAngleY(mPos, mHomePos);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mPos.y = getGroundY();
}

// .text CFCC
void dInsFldGrasshopper_c::initEscapeJump() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    mEscapeTime = dInsectUtil::getDecayFrames(param->mEscapeDecay);
    f32 dist = (mHomePos - mPos).xzLen();
    f32 speedY = getJumpSpeedY(param->mEscapeGravity, mEscapeTime);
    f32 speed = getJumpSpeed(dist, param->mEscapeDecay);
    mAngle.y = cLib::targetAngleY(mPos, mHomePos);
    mVelocity.set(0.0f, speedY, speed);
    mVelocity.rotY(mAngle.y);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    const mVec3_c normal = getGroundNormal(&mPos);
    mAxis.setUp(normal);
}

// .text D138
void dInsFldGrasshopper_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    mHomePos.set(0.0f, 0.0f, 15.0f);
    mHomePos.rotY(mAngle.y);
    mHomePos += mPos;
    f32 dist = (mHomePos - mPos).xzLen();
    f32 speedY = 0.0f;
    if (mHasJumped == true) {
        speedY = getJumpSpeedY(param->mLeaveGravity, JUMP_TIME);
    }
    f32 speed = getJumpSpeed(dist, param->mLeaveDecay);
    mAngle.y = cLib::targetAngleY(mPos, mHomePos);
    mVelocity.set(0.0f, speedY, speed);
    mVelocity.rotY(mAngle.y);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mHasJumped = true;
}

// .text D2F4
void dInsFldGrasshopper_c::executeWait() {
    const param_c *param = getKindParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderWater() == true) {
        if (!isOldPosInWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == TRUE) {
                mActive = FALSE;
                requestDelete();
            }
        } else {
            mActive = FALSE;
            requestDelete();
        }
    } else if (isChirper() == true) {
        if (mScare.isScared() == true) {
            setAnmRate(0.0f);
        } else {
            mSound.holdSound(getChirpSound(mType));
            setAnmRate(param->mChirpAnmRate);
        }
    }
}

// .text D460
void dInsFldGrasshopper_c::executeTurn() {
    const param_c *param = getKindParam(mType);
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1, step, step);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mAxis.setUp(getGroundNormal(&mPos));
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderWater() == true) {
        if (!isOldPosInWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == TRUE) {
                mActive = FALSE;
                requestDelete();
            }
        } else {
            mActive = FALSE;
            requestDelete();
        }
    }
}

// .text D5D4
void dInsFldGrasshopper_c::executeJump() {
    const param_c *param = getKindParam(mType);
    mVelocity.x *= param->mJumpDecay;
    mVelocity.z *= param->mJumpDecay;
    mVelocity.y += param->mJumpGravity;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mAxis.setUp(getGroundNormal(&mPos));
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderWater() == true) {
        if (!isOldPosInWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == TRUE) {
                mActive = FALSE;
                requestDelete();
            }
        } else {
            mActive = FALSE;
            requestDelete();
        }
    } else if (!isChirper()) {
        mSound.holdSound(getChirpSound(mType));
    }
}

// .text D790
void dInsFldGrasshopper_c::executeEscapeTurn() {
    const param_c *param = getKindParam(mType);
    s16 prev = mAngle.y;
    s16 step = mAng::fromDegree(param->mEscapeTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1, step, step);
    mAxis.rotateY((s16)(mAngle.y.mAngle - prev));
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderWater() == true) {
        if (!isOldPosInWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == TRUE) {
                mActive = FALSE;
                requestDelete();
            }
        } else {
            mActive = FALSE;
            requestDelete();
        }
    }
}

// .text D8F0
void dInsFldGrasshopper_c::executeEscapeJump() {
    const param_c *param = getKindParam(mType);
    mVelocity.x *= param->mEscapeDecay;
    mVelocity.z *= param->mEscapeDecay;
    mVelocity.y += param->mEscapeGravity;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mAxis.setUp(getGroundNormal(&mPos));
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderWater() == true) {
        if (!isOldPosInWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == TRUE) {
                mActive = FALSE;
                requestDelete();
            }
        } else {
            mActive = FALSE;
            requestDelete();
        }
    } else if (!isChirper()) {
        mSound.holdSound(getChirpSound(mType));
    }
}

// .text DAAC
void dInsFldGrasshopper_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    mVelocity.y += param->mEscapeGravity;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mAxis.setUp(getGroundNormal(&mPos));
        setState(STATE_LEAVE);
    }
    fadeOut();
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = FALSE;
        requestDelete();
    }
    if (mAlpha == 0) {
        requestDelete();
    }
}

// .text DC00
mAng dInsFldGrasshopper_c::getJumpAngle() const {
    s16 angle = mAngle.y.mAngle;
    if (mState == STATE_ESCAPE_TURN) {
        s16 player = mPlayerCheck.getAngleTo(mPlayerCheck.getNearest()).mAngle +
                     INSECT_FIELD_FLEE_ANGLE;
        s16 npc = player;
        if (mNpcCheck.isNear(mScareRadius) == true) {
            npc = mNpcCheck.getAngleTo().mAngle + INSECT_FIELD_FLEE_ANGLE;
        }
        angle = (player + npc) >> 1;
    }
    return angle + (int)cM::rndCenterF(sJumpAngleRange.mAngle);
}

// .text DCE8
f32 dInsFldGrasshopper_c::getJumpDist() {
    const param_c *param = getKindParam(mType);
    f32 min;
    f32 max;
    if (mState == STATE_ESCAPE_TURN) {
        min = param->mEscapeDistMin;
        max = param->mEscapeDistMax;
    } else {
        min = param->mJumpDistMin;
        max = param->mJumpDistMax;
    }
    return cM::rndRange(min, max);
}

// .text DD3C
void dInsFldGrasshopper_c::getJumpTarget(mVec3_c *out) {
    if (dNet::isSessionActive() != true || dNet::isSceneOwner()) {
        f32 dist = getJumpDist();
        mAng angle = getJumpAngle();
        mVec3_c target(0.0f, 0.0f, 0.0f);
        mVec3_c ofs(0.0f, 0.0f, dist);
        ofs.rotY(angle);
        ofs += mPos;
        u8 steps = (int)(dist / 32.0f) + 1;
        f32 minDy = -10.0f;
        target = ofs;
        BOOL noSea = TRUE;
        if (mState == STATE_ESCAPE_TURN) {
            noSea = FALSE;
            minDy = -30.0f;
        }
        mVec3_c from = mPos;
        mVec3_c to = ofs;
        if (!dInsectUtil::getGroundMovePos(&target, from, to, steps, TRUE,
                                           noSea, minDy, 10.0f)) {
            dInsectUtil::getLandPos(&target, mPos, 0, TRUE);
        }
        *out = target;
    }
}

// .text DEDC
bool dInsFldGrasshopper_c::isChirper() {
    bool bellCricket = mType == INSECT_BELL_CRICKET;
    bool grasshopper = mType == INSECT_GRASSHOPPER;
    if (mType == INSECT_CRICKET || bellCricket == true || grasshopper == true) {
        return true;
    }
    return false;
}

// .text DF20
mVec3_c dInsFldGrasshopper_c::getGroundNormal(const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, dBGCF::LAYER_TOP, 0, 0);
    return chk.mFloor.mNormal;
}

// .text DF78
void dInsFldGrasshopper_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            u32 state = data->getReq();
            if (state < STATE_NUM_MAX) {
                mState = state;
                mHomePos.x = data->getPosX() * 4;
                mHomePos.z = data->getPosZ() * 4;
            }
            return;
        }
    }

    if (mState == STATE_WAIT) {
        if (mWaitTime <= mFrame) {
            if (isChirper() == true) {
                if (mScare.isEscaping() == true) {
                    mState = STATE_ESCAPE_TURN;
                } else {
                    mState = STATE_TURN;
                }
            } else if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_TURN;
            }
        }
    } else if (mState == STATE_TURN) {
        if (mAngle.y.mAngle == mTargetAngle) {
            mState = STATE_JUMP;
        }
    } else if (mState == STATE_JUMP) {
        if (mJumpTime <= mFrame && isUnderGround() == true) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_WAIT;
            }
        }
    } else if (mState == STATE_ESCAPE_TURN) {
        if (mAngle.y.mAngle == mTargetAngle) {
            mState = STATE_ESCAPE_JUMP;
        }
    } else if (mState == STATE_ESCAPE_JUMP) {
        if (mEscapeTime <= mFrame && isUnderGround() == true) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_TURN;
            }
        }
    }

    if (mState != STATE_ESCAPE_JUMP && mState != STATE_ESCAPE_TURN &&
        mState != STATE_LEAVE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mState = STATE_ESCAPE_TURN;
        mScare.setMax();
    }
}

// .text E230
f32 dInsFldGrasshopper_c::getJumpSpeedY(f32 gravity, u16 frames) {
    return -gravity * (frames / 2);
}

// .text E268
f32 dInsFldGrasshopper_c::getJumpSpeed(f32 dist, f32 decay) {
    return dist / (decay / (1.0f - decay));
}

// .text E280
bool dInsFldGrasshopper_c::isOldPosInWater() {
    dBGCF::groundChk_c chk(&mOldPos, dBGCF::LAYER_TOP, 1, 0);
    if (chk.isUnderWater(mOldPos.y) == TRUE) {
        return true;
    }
    return false;
}
