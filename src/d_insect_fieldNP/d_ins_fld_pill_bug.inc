// dInsFldPillBug_c: the pill bug and the centipede. See
// include/game/game/d_ins_fld_pill_bug.hpp.

const dInsFldPillBug_c::StateFunc dInsFldPillBug_c::sExecute[] = {
    &dInsFldPillBug_c::executeHide,    &dInsFldPillBug_c::executeJumpOut,
    &dInsFldPillBug_c::executeWalk,    &dInsFldPillBug_c::executeCurl,
    &dInsFldPillBug_c::executeRelease,
};

const dInsFldPillBug_c::StateFunc dInsFldPillBug_c::sInit[] = {
    &dInsFldPillBug_c::initHide,  &dInsFldPillBug_c::initJumpOut,
    &dInsFldPillBug_c::initWalk,  &dInsFldPillBug_c::initCurl,
    &dInsFldPillBug_c::onRelease,
};

dInsFldPillBug_c::param_c dInsFldPillBug_c::sParam[] = {
    {0.0f, 0.0f,  0.0f,  15,   2.0f, -0.15f, 0.5f, 20.0f, 10.0f, 0.15f, 0.003f,
     0.9f, 0.01f, 60.0f, 0.4f, 0.3f, 1.0f,   3.0f, 3.0f,  64.0f, 50.0f},
    {0.0f, 0.0f,  0.0f,  15,   2.0f, -0.15f, 0.5f, 20.0f, 10.0f,  0.1f, 0.01f,
     0.9f, 0.02f, 60.0f, 2.0f, 0.3f, 3.0f,   3.0f, 3.0f,  100.0f, 70.0f},
};

// 0x1E910
int dInsFldPillBug_c::getParamIdx(int type) {
    const int types[] = {INSECT_PILL_BUG, INSECT_CENTIPEDE};
    for (int i = 0; i < ARRAY_SIZE(types); i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x1E944
const dInsFldPillBug_c::param_c *dInsFldPillBug_c::getPillBugParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x1E974
dInsFldPillBug_c::dInsFldPillBug_c()
    : mState(STATE_HIDE), mPrevState(STATE_HIDE), mStateCheck(STATE_HIDE),
      mHit(false), _328(0.0f), mOut(false), mLifeTimer(0), mMoveFrames(0),
      mHitTimer(0), mLanded(false), mHitter(INSECT_FIELD_PLAYER_NONE) {
}

// 0x1EA2C
dInsFldPillBug_c::~dInsFldPillBug_c() {
}

// 0x1EAB8
int dInsFldPillBug_c::execute() {
    const param_c *param = getPillBugParam(mType);
    dInsFldBase_c::execute();
    if (mOut == true) {
        mLifeTimer++;
        if ((u16)dInsectUtil::secToFrame(param->mLifeTime) <= mLifeTimer) {
            fadeOut();
            if (mAlpha == 0) {
                if (dNet::isSessionActive() == true) {
                    if (dNet::isSceneOwner() == 1) {
                        requestDelete();
                    }
                } else {
                    requestDelete();
                }
            }
        }
    }
    mAngle.y = mAxis.getAngleY();
    return TRUE;
}

// 0x1EB80
int dInsFldPillBug_c::draw() {
    if (isCentipede() == true) {
        mShadowSize = 0.0f;
        dInsFldBase_c::draw();
        f32 alpha = mAlpha / 255.0f * sShadowAlphaScale;
        mAng ang((u16)mAngle.y.mAngle);
        dShadow::requestRound(&mPos, 0, &ang, 4.0f, 1.0f, alpha, 10.0f, 1.0f);
    } else {
        dInsFldBase_c::draw();
    }
    return TRUE;
}

// 0x1EC3C
void dInsFldPillBug_c::calcMtx() {
    setMtxFromAxis();
}

// 0x1EC40
void dInsFldPillBug_c::onBecomeOwner() {
    if (mState == STATE_HIDE) {
        return;
    }
    dInsFldBase_c::onBecomeOwner();
}

// 0x1EC54
void dInsFldPillBug_c::setState(int state) {
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

// 0x1EDE4
void dInsFldPillBug_c::setGroundPos() {
    const param_c *param = getPillBugParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
    mHomePos = mPos;
    mBgRadius = 5.0f;
}

// 0x1EE70
void dInsFldPillBug_c::initState() {
    setState(STATE_HIDE);
}

// 0x1EE78
void dInsFldPillBug_c::executeState() {
    getPillBugParam(mType);
    mPrevState = mState;
    mStateCheck = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mStateCheck != state) {
        setState(mState);
    }
}

// 0x1EEF0
void dInsFldPillBug_c::initHide() {
    getPillBugParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mHitTimer = 0;
}

// 0x1EF54
void dInsFldPillBug_c::initJumpOut() {
    const param_c *param = getPillBugParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, param->mJumpSpeedY, 1.0f);
    const mVec3_c target = getEscapePos();
    dInsectCatchData_c *data;
    if (dNet::isSessionActive() == true) {
        if ((data = getCatchData()) != NULL) {
            if (dNet::isSceneOwner() == 1) {
                data->setPosX(target.x);
                data->setPosZ(target.z);
                mHomePos = target;
            }
            mVec3_c pos(data->getPosX() * 4, 0.0f, data->getPosZ() * 4);
            mAngle.y = cLib::targetAngleY(mPos, pos);
        }
    } else {
        mAngle.y = cLib::targetAngleY(mPos, target);
    }
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mVelocity.rotY(mAngle.y);
    mOut = true;
}

// 0x1F1AC
void dInsFldPillBug_c::initWalk() {
    const param_c *param = getPillBugParam(mType);
    if (mPrevState != STATE_WALK) {
        _328 = 0.0f;
    }
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
    mMoveFrames = (int)frames;
}

// 0x1F2A4
void dInsFldPillBug_c::initCurl() {
    getPillBugParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    _328 = 0.0f;
}

// 0x1F334
void dInsFldPillBug_c::onRelease() {
    const param_c *param = getPillBugParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mShadowSize = 0.0f;
}

// 0x1F3F4
void dInsFldPillBug_c::executeHide() {
    mBgCheck = false;
    getPillBugParam(mType);
    mVec3_c pos = mInitPos;
    bool hit = mPlayerCheck.isHittingStone(pos) == TRUE;
    if (hit && !isSurrounded()) {
        if (mHitter == INSECT_FIELD_PLAYER_NONE) {
            mVec3_c pos2 = mInitPos;
            mHitter = mPlayerCheck.getStoneHitter(pos2);
        }
        mHit = true;
    }
    if (mHit == true) {
        mHitTimer++;
    }
}

// 0x1F4C4
void dInsFldPillBug_c::executeJumpOut() {
    const param_c *param = getPillBugParam(mType);
    mBgCheck = true;
    if (mFrame <= JUMP_NO_BG_TIME) {
        mBgCheck = false;
    }
    mVelocity.y += param->mGravity;
    mPos += mVelocity;
    mScare.clear();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isUnderGround() == true) {
        mVelocity.set(0.0f, 0.0f, 0.0f);
        mPos.y = getGroundY();
        mShadowSize = 0.0f;
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
            }
        } else {
            mActive = false;
            requestDelete();
        }
    }
}

// 0x1F688
void dInsFldPillBug_c::executeWalk() {
    const param_c *param = getPillBugParam(mType);
    walk(param->mGravity, param->mWalkAccel, param->mWalkDrag,
         param->mWalkTurn);
    registObjc(&mPos, mBaseRadius, TRUE);
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
    } else if (isCentipede() == true) {
        mSound.holdSound(SE_MukadeRun);
    }
}

// 0x1F840
void dInsFldPillBug_c::executeCurl() {
    registObjc(&mPos, mBaseRadius, TRUE);
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

// 0x1F93C
void dInsFldPillBug_c::executeRelease() {
    const param_c *param = getPillBugParam(mType);
    mVelocity.y += param->mGravity;
    mVelocity.set(0.0f, mVelocity.y, param->mReleaseSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
        mShadowSize = 0.0f;
        if (!mLanded) {
            mSound.startSound(SE_mushi_out_run);
            mLanded = true;
        }
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

// 0x1FACC
mAng dInsFldPillBug_c::getWalkAngle() {
    int player = mPlayerCheck.getNearest();
    return mPlayerCheck.getAngleTo(player).mAngle + INSECT_FIELD_FLEE_ANGLE;
}

// 0x1FB18
f32 dInsFldPillBug_c::getWalkDist() const {
    const param_c *param = getPillBugParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x1FB48
void dInsFldPillBug_c::getWalkTarget(mVec3_c *out) {
    const param_c *param = getPillBugParam(mType);
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

// 0x1FCB0
void dInsFldPillBug_c::walk(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    mVec3_c up = chk.mFloor.mNormal;
    up.normalize();
    mAxis.setUp(up);
}

// 0x1FD20
void dInsFldPillBug_c::syncState() {
    const param_c *param = getPillBugParam(mType);
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
        if (mHit == true && param->mHitFrames <= mHitTimer) {
            mState = STATE_JUMP_OUT;
        }
    } else if (mState == STATE_JUMP_OUT) {
        if ((u16)dInsectUtil::secToFrame(getPillBugParam(mType)->mJumpTime) <=
            mFrame) {
            mState = STATE_WALK;
        }
    } else if (mState == STATE_WALK) {
        if (mMoveFrames <= mFrame) {
            setWalkTarget();
        } else if (!isCentipede() && mScare.isScared() == true) {
            mState = STATE_CURL;
        }
    } else if (mState == STATE_CURL && !mScare.isScared()) {
        mState = STATE_WALK;
    }
}

// 0x1FED8
void dInsFldPillBug_c::setWalkTarget() {
    mFrame = 0;
    getWalkTarget(&mHomePos);
    const param_c *param = getPillBugParam(mType);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mMoveFrames = (int)(dist / param->mWalkSpeed);
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

// 0x200A8
bool dInsFldPillBug_c::isCentipede() {
    return mType == INSECT_CENTIPEDE;
}

// 0x200BC
bool dInsFldPillBug_c::isSurrounded() {
    int unitZ;
    int x;
    int z;
    u32 num = 0;
    int unitX = worldToUnit(mPos.x);
    unitZ = worldToUnit(mPos.z);
    x = unitX - 1;
    for (u32 i = 0; i < AROUND_SIZE; i++, x++) {
        z = unitZ - 1;
        for (u32 j = 0; j < AROUND_SIZE; j++, z++) {
            if ((i != 1 || j != 1) && dInsectUtil::isColumn(x, z) == true) {
                num++;
            }
        }
    }
    return num >= SURROUNDED_NUM;
}

// 0x20184
mVec3_c dInsFldPillBug_c::getEscapePos() const {
    mVec3_c player(0.0f, 0.0f, 0.0f);
    mPlayerCheck.getPlayerPos(&player, mHitter);
    mVec3_c out = player;
    int unitZ;
    int x;
    int z;
    int unitX = worldToUnit(mInitPos.x);
    unitZ = worldToUnit(mInitPos.z);
    f32 best = 0.0f;
    x = unitX - 1;
    for (u32 i = 0; i < AROUND_SIZE; i++, x++) {
        z = unitZ - 1;
        for (u32 j = 0; j < AROUND_SIZE; j++, z++) {
            if ((i != 1 || j != 1) && dInsectUtil::isColumn(x, z) == false) {
                mVec3_c pos;
                dFdBase_c::getUnitGroundPos(pos, x, z);
                mVec3_c diff = pos - player;
                f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
                if (best < dist) {
                    out = pos;
                    best = dist;
                }
            }
        }
    }
    return out;
}

// 0x202E0
bool dInsFldPillBug_c::wasUnderWater() {
    dBGCF::groundChk_c chk(&mOldPos, dBGCF::LAYER_TOP, 1, 0);
    if (chk.isUnderWater(mOldPos.y) == true) {
        return true;
    }
    return false;
}
