// dInsFldMoleCricket_c: the mole cricket. See
// include/game/game/d_ins_fld_mole_cricket.hpp.

const dInsFldMoleCricket_c::StateFunc dInsFldMoleCricket_c::sExecute[] = {
    &dInsFldMoleCricket_c::executeHide,
    &dInsFldMoleCricket_c::executeJumpOut,
    &dInsFldMoleCricket_c::executeLand,
    &dInsFldMoleCricket_c::executeWalk,
    &dInsFldMoleCricket_c::executeBurrow,
    &dInsFldMoleCricket_c::executeFade,
    &dInsFldMoleCricket_c::executeRelease,
};

const dInsFldMoleCricket_c::StateFunc dInsFldMoleCricket_c::sInit[] = {
    &dInsFldMoleCricket_c::initHide,   &dInsFldMoleCricket_c::initJumpOut,
    &dInsFldMoleCricket_c::initLand,   &dInsFldMoleCricket_c::initWalk,
    &dInsFldMoleCricket_c::initBurrow, &dInsFldMoleCricket_c::initFade,
    &dInsFldMoleCricket_c::onRelease,
};

const int dInsFldMoleCricket_c::sTypes[] = {INSECT_MOLE_CRICKET};

dInsFldMoleCricket_c::param_c dInsFldMoleCricket_c::sParam[] = {
    {0.0f, 0.0f,  0.0f, 100.0f, 25,    0.4f, 2.5f, -0.15f, 1.0f, 150.0f, 100.0f,
     0.5f, 0.05f, 0.9f, 0.07f,  60.0f, 1.6f, 0.3f, 1.5f,   5.0f, 5.0f},
};

// 0x1A184
int dInsFldMoleCricket_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x1A1A4
const dInsFldMoleCricket_c::param_c *
dInsFldMoleCricket_c::getMoleCricketParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x1A1D4
dInsFldMoleCricket_c::dInsFldMoleCricket_c()
    : mState(STATE_HIDE), mPrevState(STATE_HIDE), mStateCheck(STATE_HIDE),
      mDug(false), mLandFrames(0), mMoveFrames(0), mWalkTimer(0), mDigTimer(0) {
}

// 0x1A234
dInsFldMoleCricket_c::~dInsFldMoleCricket_c() {
}

// 0x1A28C
int dInsFldMoleCricket_c::draw() {
    if (mState != STATE_HIDE) {
        if (!mActive) {
            return FALSE;
        }
        calcMtx();
        mMdl.calc(false);
        mMdl.entry();
        if (!isZero(mShadowSize)) {
            f32 alpha = mAlpha / 255.0f * sShadowAlphaScale;
            mAng ang = 0;
            dShadow::requestRound(&mPos, 0, &ang, mShadowSize, 1.0f, alpha,
                                  0.0f, 1.0f);
        }
    }
    return TRUE;
}

// 0x1A394
void dInsFldMoleCricket_c::calcMtx() {
    setMtxFromAxis();
}

// 0x1A398
void dInsFldMoleCricket_c::onBecomeOwner() {
    if (mState == STATE_HIDE) {
        return;
    }
    if (mState == STATE_BURROW) {
        return;
    }
    dInsFldBase_c::onBecomeOwner();
}

// 0x1A3B4
void dInsFldMoleCricket_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == 1) {
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

// 0x1A544
void dInsFldMoleCricket_c::setGroundPos() {
    const param_c *param = getMoleCricketParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
    mHomePos = mPos;
}

// 0x1A5C4
void dInsFldMoleCricket_c::initState() {
    setState(STATE_HIDE);
}

// 0x1A5CC
void dInsFldMoleCricket_c::executeState() {
    getMoleCricketParam(mType);
    mPrevState = mState;
    mStateCheck = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mStateCheck != state) {
        setState(mState);
    }
}

// 0x1A644
void dInsFldMoleCricket_c::initHide() {
    mFrame = 0;
    mDigTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
}

// 0x1A6B4
void dInsFldMoleCricket_c::initJumpOut() {
    const param_c *param = getMoleCricketParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, param->mJumpSpeedY, param->mJumpSpeed);
    int player = mPlayerCheck.getNearest();
    dInsectCatchData_c *data;
    if (dNet::isSessionActive() == true && (data = getCatchData()) != NULL) {
        if (dNet::isSceneOwner() == 1) {
            data->setPlayer(player);
        } else {
            player = data->getPlayer();
        }
    }
    mAngle.y = mPlayerCheck.getAngleTo(player).mAngle - DEG_TO_ANGLE(180);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mVelocity.rotY(mAngle.y);
}

// 0x1A7F4
void dInsFldMoleCricket_c::initLand() {
    const param_c *param = getMoleCricketParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mLandFrames = dInsectUtil::secToFrame(param->mLandTime);
}

// 0x1A894
void dInsFldMoleCricket_c::initWalk() {
    const param_c *param = getMoleCricketParam(mType);
    if (mPrevState != STATE_WALK) {
        mWalkTimer = 0;
    }
    mState = STATE_WALK;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(0.0f);
    getWalkTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    f32 frames = dist / param->mWalkSpeed;
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mMoveFrames = (int)frames + 1;
}

// 0x1A994
void dInsFldMoleCricket_c::initBurrow() {
    getMoleCricketParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mAngle.x = mAxis.getAngleX();
    mAngle.y = mAxis.getAngleY();
    mAngle.z = mAxis.getAngleZ();
    dEffect::create("af_ins_okera_hide", &mPos, &mAngle, NULL);
}

// 0x1AA58
void dInsFldMoleCricket_c::initFade() {
    const param_c *param = getMoleCricketParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(0.0f);
}

// 0x1AADC
void dInsFldMoleCricket_c::onRelease() {
    const param_c *param = getMoleCricketParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x1AB84
void dInsFldMoleCricket_c::executeHide() {
    int me = dNet::getMyNo();
    if (mPlayerCheck.isValid(me) == true &&
        mPlayerCheck.isNear(me, getMoleCricketParam(mType)->mChirpRadius) ==
            true) {
        mSound.holdSound(SE_okera_chika);
    }
    mVec3_c pos = mInitPos;
    bool dug =
        mPlayerCheck.isDigging(pos, dPlayerMgr::PLAYER_INDEX_CURRENT) == TRUE;
    if (dug) {
        mDug = true;
    }
    if (mDug == true) {
        mDigTimer++;
    }
}

// 0x1AC54
void dInsFldMoleCricket_c::executeJumpOut() {
    getMoleCricketParam(mType);
    mSound.holdSound(SE_okera_chijyou);
    mVelocity.y += -0.15f;
    mPos += mVelocity;
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    if (isUnderGround() == true) {
        mVelocity.set(0.0f, 0.0f, 0.0f);
    } else {
        mFrame = 0;
        mAngle.y = cM::atan2s(mVelocity.x, mVelocity.z);
    }
    if (isUnderWater() == true) {
        if (!wasUnderWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == 1) {
                mActive = false;
                requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
                alive = false;
#endif
            }
        } else {
            mActive = false;
            requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
            alive = false;
#endif
        }
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1ADE8
void dInsFldMoleCricket_c::executeLand() {
    getMoleCricketParam(mType);
    mSound.holdSound(SE_okera_chijyou);
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    mVelocity.y += -0.15f;
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        f32 y = getGroundY();
        if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
            mPos.y = y;
        } else {
            mPos.y = y;
        }
    }
    if (isUnderWater() == true) {
        if (!wasUnderWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == 1) {
                mActive = false;
                requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
                alive = false;
#endif
            }
        } else {
            mActive = false;
            requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
            alive = false;
#endif
        }
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1AF90
void dInsFldMoleCricket_c::executeWalk() {
    const param_c *param = getMoleCricketParam(mType);
    mSound.holdSound(SE_okera_chijyou);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        f32 y = getGroundY();
        if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
            if (y - mPos.y < 30.0f) {
                mPos.y = y;
            }
        } else {
            mPos.y = y;
        }
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    bool alive = true;
#else
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    mWalkTimer++;
    if (isUnderWater() == true) {
        if (!wasUnderWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == 1) {
                mActive = false;
                requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
                alive = false;
#endif
            }
        } else {
            mActive = false;
            requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
            alive = false;
#endif
        }
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (alive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1B148
void dInsFldMoleCricket_c::executeBurrow() {
    mPos.y -= 0.1f;
    mBgCheck = false;
    if (mFrame <= DIG_TILT_TIME) {
        mAxis.rotateX(DEG_TO_ANGLE(2));
    }
    f32 y = getGroundY() - 16.0f;
    mShadowSize *= 0.95f;
    if (mPos.y <= y) {
        requestDelete();
    }
}

// 0x1B1F0
void dInsFldMoleCricket_c::executeFade() {
    const param_c *param = getMoleCricketParam(mType);
    mSound.holdSound(SE_okera_chijyou);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        f32 y = getGroundY();
        if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
            if (y - mPos.y < 50.0f) {
                mPos.y = y;
            }
        } else {
            mPos.y = y;
        }
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
    if (isUnderWater() == true) {
        if (!wasUnderWater()) {
            mSound.startSound(SE_MushiFallWater);
            dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
            mPos.y = chk.getWaterY();
            dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
            dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                            checkPtclLanding, 0);
        }
        if (dNet::isSessionActive() == true) {
            if (dNet::isSceneOwner() == 1) {
                mActive = false;
                requestDelete();
            }
        } else {
            mActive = false;
            requestDelete();
        }
    }
}

// 0x1B3A4
void dInsFldMoleCricket_c::executeRelease() {
    const param_c *param = getMoleCricketParam(mType);
    setAnmRate(param->mWalkAnmRate);
    mVelocity.y += param->mGravity;
    mVelocity.set(0.0f, mVelocity.y, param->mReleaseSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x1B528
mAng dInsFldMoleCricket_c::getWalkAngle() {
    int player = mPlayerCheck.getNearest();
    return mPlayerCheck.getAngleTo(player).mAngle + INSECT_FIELD_FLEE_ANGLE;
}

// 0x1B574
f32 dInsFldMoleCricket_c::getWalkDist() const {
    const param_c *param = getMoleCricketParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x1B5A4
void dInsFldMoleCricket_c::getWalkTarget(mVec3_c *out) {
    const param_c *param = getMoleCricketParam(mType);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        return;
    }
    mAng ang = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    u8 steps = (int)(param->mWalkDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = target;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, TRUE, TRUE,
                                       -30.0f, 5.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 0, TRUE);
    }
    *out = pos;
}

// 0x1B70C
void dInsFldMoleCricket_c::walk(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    mVec3_c up = chk.mFloor.mNormal;
    up.normalize();
    mAxis.setUp(up);
}

// 0x1B77C
bool dInsFldMoleCricket_c::canDig() {
    return dField::getInfo(FD_ID_CURRENT)->getDigType(mPos) == BG_DIG_SOIL;
}

// 0x1B7BC
void dInsFldMoleCricket_c::setWalkTarget() {
    const param_c *param = getMoleCricketParam(mType);
    mFrame = 0;
    getWalkTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mMoveFrames = (int)(dist / param->mWalkSpeed) + 1;
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == 1) {
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

// 0x1B994
void dInsFldMoleCricket_c::syncState() {
    const param_c *param = getMoleCricketParam(mType);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
            u32 req = data->getReq();
            if (req < STATE_NUM_NET) {
                mState = req;
            }
        }
    } else if (mState == STATE_HIDE) {
        if (param->mDigFrames <= mDigTimer) {
            mState = STATE_JUMP_OUT;
        }
    } else if (mState == STATE_JUMP_OUT) {
        if (isUnderGround() == true) {
            mPos.y = getGroundY();
            mState = STATE_LAND;
        }
    } else if (mState == STATE_LAND) {
        if (mLandFrames <= mFrame) {
            mState = STATE_WALK;
        }
    } else if (mState == STATE_WALK) {
        if (mMoveFrames <= mFrame) {
            setWalkTarget();
        } else if ((u16)dInsectUtil::secToFrame(param->mWalkTime) <=
                   mWalkTimer) {
            if (canDig() == true) {
                mState = STATE_BURROW;
            } else {
                mState = STATE_FADE;
            }
        }
    }
}

// 0x1BB50
bool dInsFldMoleCricket_c::wasUnderWater() {
    dBGCF::groundChk_c chk(&mOldPos, dBGCF::LAYER_TOP, 1, 0);
    if (chk.isUnderWater(mOldPos.y) == true) {
        return true;
    }
    return false;
}
