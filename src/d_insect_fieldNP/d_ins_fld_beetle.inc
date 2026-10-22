// dInsFldBeetle_c: the insects on tree trunks. See
// include/game/game/d_ins_fld_beetle.hpp.

const dInsFldBeetle_c::StateFunc dInsFldBeetle_c::sExecute[] = {
    &dInsFldBeetle_c::executeWait,   &dInsFldBeetle_c::executeWalk,
    &dInsFldBeetle_c::executeEscape, &dInsFldBeetle_c::executeHide,
    &dInsFldBeetle_c::executeLeave,
};

const dInsFldBeetle_c::StateFunc dInsFldBeetle_c::sInit[] = {
    &dInsFldBeetle_c::initWait,   &dInsFldBeetle_c::initWalk,
    &dInsFldBeetle_c::initEscape, &dInsFldBeetle_c::initHide,
    &dInsFldBeetle_c::onRelease,
};

const int dInsFldBeetle_c::sSounds[] = {
    SE_mushi_out_habataki,   SE_mushi_out_fall,       SE_mushi_fly_out_midium,
    SE_mushi_fly_out_midium, SE_mushi_fly_out_midium, SE_mushi_fly_out_midium,
    SE_mushi_fly_out_midium, SE_mushi_fly_out_midium, SE_mushi_fly_out_midium,
    SE_mushi_fly_out_midium, SE_mushi_fly_out_midium, SE_mushi_fly_out_midium,
    SE_mushi_fly_out_big,    SE_mushi_fly_out_big,    SE_mushi_fly_out_big,
    SE_mushi_fly_out_big,
};

// .data 588
dInsFldBeetle_c::param_c dInsFldBeetle_c::sParam[TYPE_NUM] = {
    {0.0f,  22.0f, 12.0f, 5.0f, 1.0f, 3.0f,  1.0f,  1.0f,   10.0f,  0.8f,
     0.05f, 1.0f,  0.0f,  5.0f, 3.0f, 12.0f, 10.0f, 120.0f, 240.0f, 80},
    {0.0f, 22.0f, 10.0f, 10.0f, 5.0f, 2.0f, 1.0f, 0.3f,   10.0f,  0.3f,
     0.0f, 5.0f,  0.0f,  5.0f,  3.0f, 4.0f, 8.0f, 100.0f, 240.0f, 200},
    {0.0f,  18.0f, 10.0f, 5.0f, 1.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.03f, 1.0f,  0.0f,  5.0f, 3.0f, 5.0f, 7.0f, 60.0f, 240.0f, 200},
    {0.0f,  18.0f, 10.0f, 5.0f, 2.0f, 3.0f, 1.0f, 0.6f,  10.0f,  0.6f,
     0.03f, 1.0f,  0.0f,  5.0f, 3.0f, 5.0f, 7.0f, 90.0f, 240.0f, 80},
    {0.0f,  18.0f, 10.0f, 5.0f, 1.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.04f, 1.0f,  0.0f,  5.0f, 3.0f, 4.5f, 8.0f, 70.0f, 240.0f, 150},
    {0.0f,  18.0f, 10.0f, 5.0f, 1.0f, 3.0f, 2.0f, 0.5f,  10.0f,  0.6f,
     0.04f, 1.0f,  0.0f,  5.0f, 3.0f, 6.5f, 8.0f, 60.0f, 240.0f, 200},
    {0.0f,  18.0f, 10.0f, 5.0f, 1.0f, 3.0f, 2.0f, 0.5f,  10.0f,  0.6f,
     0.04f, 1.0f,  0.0f,  5.0f, 3.0f, 6.5f, 8.0f, 90.0f, 240.0f, 150},
    {0.0f,  18.0f, 10.0f, 5.0f, 3.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.5f,
     0.02f, 1.0f,  0.0f,  5.0f, 3.0f, 7.0f, 8.0f, 90.0f, 240.0f, 100},
    {0.0f,  18.0f, 10.0f, 5.0f, 2.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.03f, 1.0f,  0.0f,  5.0f, 3.0f, 6.0f, 8.0f, 90.0f, 240.0f, 100},
    {0.0f,  18.0f, 8.0f, 5.0f, 1.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 7.5f, 8.0f, 90.0f, 240.0f, 100},
    {0.0f,  18.0f, 8.0f, 5.0f, 1.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 7.0f, 8.0f, 90.0f, 240.0f, 80},
    {0.0f,  18.0f, 10.0f, 5.0f, 1.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f,  5.0f, 3.0f, 7.0f, 8.0f, 90.0f, 240.0f, 150},
    {0.0f,  18.0f, 8.0f, 5.0f, 2.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 8.0f, 9.0f, 90.0f, 240.0f, 100},
    {0.0f,  18.0f, 8.0f, 5.0f, 2.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 7.5f, 9.0f, 90.0f, 240.0f, 100},
    {0.0f,  18.0f, 8.0f, 5.0f, 2.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 8.0f, 9.0f, 90.0f, 240.0f, 80},
    {0.0f,  18.0f, 8.5f, 5.0f, 2.0f, 3.0f, 1.0f, 0.5f,  10.0f,  0.6f,
     0.02f, 1.0f,  0.0f, 5.0f, 3.0f, 8.0f, 9.0f, 90.0f, 240.0f, 120},
};

// .text AB8C
const dInsFldBeetle_c::param_c *dInsFldBeetle_c::getKindParam(int type) {
    const int types[] = {
        INSECT_OAK_SILK_MOTH,    INSECT_WALKING_STICK,   INSECT_DRONE_BEETLE,
        INSECT_SCARAB_BEETLE,    INSECT_JEWEL_BEETLE,    INSECT_MIYAMA_STAG,
        INSECT_SAW_STAG,         INSECT_GIANT_STAG,      INSECT_RAINBOW_STAG,
        INSECT_CYCLOMMATUS_STAG, INSECT_GOLDEN_STAG,     INSECT_DYNASTID_BEETLE,
        INSECT_ATLAS_BEETLE,     INSECT_ELEPHANT_BEETLE, INSECT_HERCULES_BEETLE,
        INSECT_GOLIATH_BEETLE,
    };
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return &sParam[i];
        }
    }
    return &sParam[0];
}

// .text AC80
int dInsFldBeetle_c::getEscapeSound(int type) {
    const int types[] = {
        INSECT_OAK_SILK_MOTH,    INSECT_WALKING_STICK,   INSECT_DRONE_BEETLE,
        INSECT_SCARAB_BEETLE,    INSECT_JEWEL_BEETLE,    INSECT_MIYAMA_STAG,
        INSECT_SAW_STAG,         INSECT_GIANT_STAG,      INSECT_RAINBOW_STAG,
        INSECT_CYCLOMMATUS_STAG, INSECT_GOLDEN_STAG,     INSECT_DYNASTID_BEETLE,
        INSECT_ATLAS_BEETLE,     INSECT_ELEPHANT_BEETLE, INSECT_HERCULES_BEETLE,
        INSECT_GOLIATH_BEETLE,
    };
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return sSounds[i];
        }
    }
    return SE_mushi_out_habataki;
}

// .text AD70
dInsFldBeetle_c::dInsFldBeetle_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mWobbleDir(0), mWobbleY(0),
      mWobbleZ(0), mSlerp(0.0f), mWaitTime(0), mWalkTime(0),
      mRotFrom(1.0f, 0.0f, 0.0f, 0.0f), mRotTo(1.0f, 0.0f, 0.0f, 0.0f),
      mRot(1.0f, 0.0f, 0.0f, 0.0f), mWobble(1.0f, 0.0f, 0.0f, 0.0f) {
}

// .text AE74
dInsFldBeetle_c::~dInsFldBeetle_c() {
}

// .text AECC
int dInsFldBeetle_c::draw() {
    getKindParam(mType);
    if (mState != STATE_LEAVE) {
        dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
        if (camera != NULL) {
            mVec3_c pos;
            dFdBase_c::snapToUnitCenter(pos, mPos);
            if (camera->isPastForeground(&pos, 32.0f)) {
                return TRUE;
            }
        }
    }
    if (!mActive) {
        return TRUE;
    }
    calcMtx();
    mMdl.calc(false);
    mMdl.entry();
    if (!isZero(mShadowSize)) {
        f32 alpha = mAlpha / 255.0f * sShadowAlphaScale;
        mAng angle(0);
        dShadow::requestRound(&mPos, 0, &angle, mShadowSize, 1.0f, alpha, 0.0f,
                              1.0f);
    }
    return TRUE;
}

// .text B024
void dInsFldBeetle_c::calcMtx() {
    mMtx_c mtx;
    mMtx_c rot;
    ((EGG::Matrix34f *)(nw4r::math::MTX34 *)(rot))->makeQ(mRot);
    dActor_c::makeMtx(&mtx, &mPos, 0);
    mtx.concat(rot);
    mMtx_c wobble;
    ((EGG::Matrix34f *)(nw4r::math::MTX34 *)(wobble))->makeQ(mWobble);
    mtx.concat(wobble);
    mMdl.setLocalMtx(&mtx);
}

// .text B0A8
void dInsFldBeetle_c::onBecomeOwner() {
}

// .text B0AC
void dInsFldBeetle_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE &&
        (state == STATE_WAIT || state == STATE_ESCAPE)) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setReq(mState);
        }
    }
}

// .text B148
void dInsFldBeetle_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x = mInitPos.x + param->mOfsX;
    mPos.y = param->mOfsY + getGroundY();
    mPos.z = mInitPos.z + param->mOfsZ;
}

// .text B1B8
void dInsFldBeetle_c::initState() {
    setState(STATE_WAIT);
}

// .text B1C0
void dInsFldBeetle_c::executeState() {
    const param_c *param = getKindParam(mType);
    mBgCheck = FALSE;
    mPrevState = mState;
    (this->*sExecute[mState])();
    checkState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
    if (mState == STATE_ESCAPE || mState == STATE_LEAVE) {
        mBgCheck = TRUE;
        mShadowSize = param->mShadowSize;
    } else {
        mShadowSize = 0.0f;
    }
}

// .text B27C
void dInsFldBeetle_c::initWait() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    if (isWalkingStick() == true) {
        setAnmChr(mAnmChrFly, 0.0f);
        setAnmVis(mAnmVisFly);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(0.05f);
    setAnmFrame(frame);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    mWaitTime = cM::rndRange<u16>(min, max);
    getRestRot(&mRotFrom);
    mRot = mRotFrom;
}

// .text B3A0
void dInsFldBeetle_c::initWalk() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    if (isWalkingStick() == true) {
        setAnmChr(mAnmChrFly, 0.0f);
        setAnmVis(mAnmVisFly);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    mWalkTime = cM::rndRange<u16>(min, max);
}

// .text B494
void dInsFldBeetle_c::initEscape() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mAngle.y = getEscapeAngle();
    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    mRotTo.setAxisRotation(up, mAngle.y.radian());
    if (isWalkingStick() == true) {
        mAlpha = 0;
    }
    int sound = getEscapeSound(mType);
    if (sound > 0) {
        mSound.startSound(sound);
    }
}

// .text B5C0
void dInsFldBeetle_c::initHide() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(frame);
}

// .text B658
void dInsFldBeetle_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    mRot.set(1.0f, 0.0f, 0.0f, 0.0f);
    mRot.setAxisRotation(up, mAngle.y.radian());
    mVelocity.set(0.0f, 0.0f, 0.0f);
    int sound = getEscapeSound(mType);
    if (sound > 0) {
        mSound.startSound(sound);
    }
}

// .text B774
void dInsFldBeetle_c::executeWait() {
    const param_c *param = getKindParam(mType);
    if (isWalkingStick() == true) {
        mAlpha = getHideAlpha();
    }
    mVec3_c pos = mPos;
    pos.x += param->mObjcOfsX;
    pos.y += param->mObjcOfsY;
    pos.z += param->mObjcOfsZ;
    registObjc(&pos, mBaseRadius, TRUE);
    if (mWaitTime <= mFrame) {
        mState = STATE_WALK;
    }
}

// .text B834
void dInsFldBeetle_c::executeWalk() {
    const param_c *param = getKindParam(mType);
    mVec3_c pos = mPos;
    pos.x += param->mObjcOfsX;
    pos.y += param->mObjcOfsY;
    pos.z += param->mObjcOfsZ;
    registObjc(&pos, mBaseRadius, TRUE);
    if (isWalkingStick() == true) {
        mAlpha = getHideAlpha();
    }
    if (mWalkTime <= mFrame) {
        mState = STATE_WAIT;
    }
    if (!isOakSilkMoth() && !isWalkingStick()) {
        wobble();
    }
}

// .text B910
void dInsFldBeetle_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    if (isWalkingStick() == true) {
        requestDelete();
    } else {
        moveForward(param->mSpeed, param->mAccelY);
        mRotFrom.slerpTo(mRotTo, mSlerp, mRot);
        mSlerp += 0.08f;
        mSlerp = mSlerp > 1.0f ? 1.0f : mSlerp;
    }
}

// .text B9BC
void dInsFldBeetle_c::executeHide() {
    int alpha = mAlpha - 21;
    mAlpha = alpha < 0 ? 0 : alpha;
}

// .text B9D4
void dInsFldBeetle_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    if (isWalkingStick() == true) {
        mVec3_c move(0.0f, 0.0f, 0.1f * param->mSpeed);
        move.rotY(mAngle.y);
        mVelocity *= 0.9f;
        mVelocity += move;
        mVelocity.y += -0.15f;
        mPos += mVelocity;
        if (isUnderGround() == true) {
            mVelocity.y = 0.0f;
            mPos.y = getGroundY();
        }
        fadeOut();
        if (mAlpha == 0) {
            requestDelete();
        }
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
    } else {
        mAngle.x = mAng::fromDegree(param->mLeaveAngleX);
        EGG::Vector3f up(0.0f, 1.0f, 0.0f);
        EGG::Vector3f side(1.0f, 0.0f, 0.0f);
        mQuat_c rotY;
        rotY.setAxisRotation(up, mAngle.y.radian());
        mQuat_c rotX;
        rotX.setAxisRotation(side, mAngle.x.radian());
        mRot = rotY * rotX;
        f32 vy = mVelocity.y;
        mVelocity.set(0.0f, 0.0f, param->mSpeed);
        mVelocity.rotY(mAngle.y);
        mVelocity.y = vy + param->mAccelY;
        mOldPos = mPos;
        mPos += mVelocity;
        mAcch.checkSteps(10.0f, &mPos, &mOldPos, mAngle.y, 0,
                         dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                             dBGCF::CHECK_SET_POS);
        fadeOut();
        if (mAlpha == 0) {
            requestDelete();
        }
    }
}

// .text BD64
void dInsFldBeetle_c::getRestRot(mQuat_c *out) {
    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    EGG::Vector3f side(1.0f, 0.0f, 0.0f);
    mQuat_c rotY(1.0f, 0.0f, 0.0f, 0.0f);
    mQuat_c rotX(1.0f, 0.0f, 0.0f, 0.0f);
    rotY.setAxisRotation(up,
                         DEG_TO_ANGLE(-180) * mAng::AngleToRadianCoefficient);
    rotX.setAxisRotation(side,
                         DEG_TO_ANGLE(90) * mAng::AngleToRadianCoefficient);
    *out = rotX * rotY;
}

// .text BE7C
mAng dInsFldBeetle_c::getEscapeAngle() {
    int player = mPlayerCheck.getNearest();
    f32 playerDist = mPlayerCheck.getDistance(player);
    f32 npcDist = mNpcCheck.getDistance();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    if (playerDist < npcDist) {
        mPlayerCheck.getPlayerPos(&pos, player);
    } else {
        mNpcCheck.getNpcPos(&pos);
    }
    if (pos.x - mPos.x > 0.0f) {
        return DEG_TO_ANGLE(-45);
    }
    return DEG_TO_ANGLE(45);
}

// .text BF44
bool dInsFldBeetle_c::isOakSilkMoth() {
    return mType == INSECT_OAK_SILK_MOTH;
}

// .text BF58
bool dInsFldBeetle_c::isWalkingStick() {
    return mType == INSECT_WALKING_STICK;
}

// .text BF6C
bool dInsFldBeetle_c::isDisturbed() {
    return mPlayerCheck.isDisturbing(mScareRadius);
}

// .text BF78
u8 dInsFldBeetle_c::getHideAlpha() {
    return 255.0f * (1.0f - mScare.getRate());
}

// .text BFC0
void dInsFldBeetle_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            if (data->getReq() < STATE_NUM_MAX) {
                int state = data->getReq();
                if (state == STATE_ESCAPE) {
                    mState = state;
                }
                if (state == STATE_WAIT && mState != STATE_WALK) {
                    mState = state;
                }
            }
            return;
        }
    }

    if (mState == STATE_WAIT) {
        if (mScare.isScared() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_WALK) {
        if (mScare.isScared() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_HIDE) {
        if (mAlpha == 0) {
            mState = STATE_ESCAPE;
        }
    }

    if (!isHeld()) {
        if (isWalkingStick() == true) {
            if (mState != STATE_HIDE && mState != STATE_ESCAPE &&
                mState != STATE_LEAVE && isDisturbed() == true) {
                mState = STATE_HIDE;
            }
        } else {
            if (mState != STATE_ESCAPE && mState != STATE_LEAVE &&
                isDisturbed() == true) {
                mState = STATE_ESCAPE;
            }
        }
    }
}

// Speculative stand-in. The REL's constant pool creates 30.0f here (before
// calcScare's 360.0f and 0.5f); its first users are MoleCricket's and PillBug's
// executeWalk, nothing compiled in between uses it: a stripped function made
// it.
f32 dInsFldBeetle_strippedScareRange() {
    return 30.0f;
}

// .text C134
void dInsFldBeetle_c::calcScare() {
    const param_c *param = getKindParam(mType);
    int player = mPlayerCheck.getFastest(mScareRadius);
    s16 min = mAng::fromDegree(360.0f - param->mScareAngle / 2.0f);
    s16 max = mAng::fromDegree(param->mScareAngle / 2.0f);
    if (player < INSECT_FIELD_PLAYER_NONE) {
        s16 angle = mPlayerCheck.getAngleTo(player);
        if (min <= angle && angle <= max) {
            f32 rate = 1.0f;
            if (mPlayerCheck.isNear(player, mScareRadius / 2.0f) == true) {
                rate = 3.0f;
            }
            mScare.add(mPlayerCheck.getPlayerSpeed(player), rate);
        } else {
            mScare.add(0.0f, 1.0f);
        }
    } else if (mNpcCheck.isNear(mScareRadius) == true) {
        s16 angle = mNpcCheck.getAngleTo();
        if (min <= angle && angle <= max) {
            // Discarded near-rate predicate, likely residue of a stripped debug
            // check.
            (void)(mNpcCheck.isNear(mScareRadius / 2.0f) == true ? 3.0f : 1.0f);
            mScare.add(mNpcCheck.getNpcSpeed(), 1.0f);
        } else {
            mScare.add(0.0f, 1.0f);
        }
    } else {
        mScare.add(0.0f, 1.0f);
    }
}

// .text C2F0
void dInsFldBeetle_c::wobble() {
    if (mWobbleDir == true) {
        mWobbleY.mAngle += ANGLE_PER_DEGREE / 2;
        mWobbleZ.mAngle -= ANGLE_PER_DEGREE / 2;
        if (mWobbleY.mAngle >= WOBBLE_MAX) {
            mWobbleDir = false;
        }
    } else {
        mWobbleY.mAngle -= ANGLE_PER_DEGREE / 2;
        mWobbleZ.mAngle += ANGLE_PER_DEGREE / 2;
        if (mWobbleY.mAngle <= -WOBBLE_MAX) {
            mWobbleDir = true;
        }
    }
    EGG::Vector3f axisZ(0.0f, 0.0f, 1.0f);
    mQuat_c rotZ(axisZ, mWobbleZ.radian());
    EGG::Vector3f axisY(0.0f, 1.0f, 0.0f);
    mQuat_c rotY(axisY, mWobbleY.radian());
    mWobble = rotZ * rotY;
}
