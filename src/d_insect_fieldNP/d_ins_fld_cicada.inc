// dInsFldCicada_c: the cicadas and the lantern fly. See
// include/game/game/d_ins_fld_cicada.hpp.

const dInsFldCicada_c::StateFunc dInsFldCicada_c::sExecute[] = {
    &dInsFldCicada_c::executeWait,
    &dInsFldCicada_c::executeAlert,
    &dInsFldCicada_c::executeEscape,
    &dInsFldCicada_c::executeLeave,
};

const dInsFldCicada_c::StateFunc dInsFldCicada_c::sInit[] = {
    &dInsFldCicada_c::initWait,
    &dInsFldCicada_c::initAlert,
    &dInsFldCicada_c::initEscape,
    &dInsFldCicada_c::onRelease,
};

// .data E60
dInsFldCicada_c::param_c dInsFldCicada_c::sParam[TYPE_NUM] = {
    {0.0f, 18.0f, 12.0f, 1.0f, 0.6f, 0.03f, 1.0f, 0.0f, 5.0f, 3.0f, 5.0f, 8.0f,
     60.0f, 240.0f, 40, 200},
    {0.0f, 18.0f, 12.0f, 1.0f, 0.6f, 0.03f, 1.0f, 0.0f, 5.0f, 3.0f, 5.0f, 8.0f,
     60.0f, 240.0f, 40, 200},
    {0.0f, 18.0f, 12.0f, 1.0f, 0.6f, 0.03f, 1.0f, 0.0f, 5.0f, 3.0f, 5.0f, 8.0f,
     60.0f, 240.0f, 40, 200},
    {0.0f, 18.0f, 12.0f, 1.0f, 0.6f, 0.03f, 1.0f, 0.0f, 5.0f, 3.0f, 5.0f, 8.0f,
     60.0f, 240.0f, 40, 200},
    {0.0f, 18.0f, 12.0f, 1.0f, 0.7f, 0.03f, 1.0f, 0.0f, 5.0f, 3.0f, 7.5f, 9.0f,
     120.0f, 240.0f, 30, 80},
};

// .text F9B0
int dInsFldCicada_c::getParamIdx(int type) {
    const int types[] = {
        INSECT_BROWN_CICADA,   INSECT_ROBUST_CICADA, INSECT_WALKER_CICADA,
        INSECT_EVENING_CICADA, INSECT_LANTERN_FLY,
    };
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text FA20
const dInsFldCicada_c::param_c *dInsFldCicada_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text FA50
int dInsFldCicada_c::getSingSound(int type) {
    const int sounds[] = {SE_aburazemi, SE_minminzemi, SE_tsukutsukuhoushi,
                          SE_higurashi, -1}; // -1: the lantern fly is silent
    return sounds[getParamIdx(type)];
}

// .text FAA8
dInsFldCicada_c::dInsFldCicada_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mLastState(STATE_WAIT),
      mSlerp(0.0f), mRotFrom(1.0f, 0.0f, 0.0f, 0.0f),
      mRotTo(1.0f, 0.0f, 0.0f, 0.0f), mRot(1.0f, 0.0f, 0.0f, 0.0f) {
}

// .text FB74
dInsFldCicada_c::~dInsFldCicada_c() {
}

// .text FBCC
BOOL dInsFldCicada_c::create(const nw4r::g3d::ResFile &res) {
    dInsFldBase_c::create(res);
    return TRUE;
}

// .text FBF0
int dInsFldCicada_c::draw() {
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

// .text FD48
void dInsFldCicada_c::calcMtx() {
    mMtx_c mtx;
    mMtx_c rot;
    fn_80444FE4(rot, &mRot);
    dActor_c::makeMtx(&mtx, &mPos, 0);
    mtx.concat(rot);
    mMdl.setLocalMtx(&mtx);
}

// .text FDB0
void dInsFldCicada_c::onBecomeOwner() {
}

// .text FDB4
void dInsFldCicada_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setReq(mState);
        }
    }
}

// .text FE34
void dInsFldCicada_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x = mInitPos.x + param->mOfsX;
    mPos.y = param->mOfsY + getGroundY();
    mPos.z = mInitPos.z + param->mOfsZ;
}

// .text FEA4
void dInsFldCicada_c::initState() {
    setState(STATE_WAIT);
}

// .text FEAC
void dInsFldCicada_c::executeState() {
    const param_c *param = getKindParam(mType);
    mBgCheck = FALSE;
    mLastState = mState;
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

// .text FF68
void dInsFldCicada_c::initWait() {
    const param_c *param = getKindParam(mType);
    f32 frame = mAnmChr.getFrame();
    if (isLanternFly() == true && mLastState == STATE_ALERT) {
        frame = 710.0f;
    }
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWaitAnmRate);
    setAnmFrame(frame);
    getRestRot(&mRotFrom);
    mRot = mRotFrom;
}

// .text 10050
void dInsFldCicada_c::initAlert() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    if (isLanternFly() == true) {
        setAnmRate(param->mWaitAnmRate);
        setAnmFrame(360.0f);
    } else {
        setAnmRate(0.0f);
        setAnmFrame(frame);
    }
}

// .text 10120
void dInsFldCicada_c::initEscape() {
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
    if (!isLanternFly()) {
        mSound.startSound(SE_semi_fly);
    } else {
        mSound.startSound(SE_Biwahagoromo_FlyOut);
    }
}

// .text 10248
void dInsFldCicada_c::onRelease() {
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
    mQuat_c tilt;
    EGG::Vector3f side(1.0f, 0.0f, 0.0f);
    tilt.setAxisRotation(side, mAng(0xE39).radian()); // ~20 degrees
    mRot = tilt * mRot;
    if (!isLanternFly()) {
        mSound.startSound(SE_semi_fly);
    } else {
        mSound.startSound(SE_Biwahagoromo_FlyOut);
    }
}

// .text 103EC
void dInsFldCicada_c::executeWait() {
    const param_c *param = getKindParam(mType);
    int sound = getSingSound(mType);
    if (sound > 0) {
        if (!dInsectUtil::isRaining()) {
            mSound.holdSound(sound);
        }
        if (mSound.isSounding() == TRUE) {
            setAnmRate(1.0f);
        } else {
            setAnmRate(0.0f);
        }
    }
    if (isLanternFly() == true && 718.0f < mAnmChr.getFrame()) {
        setAnmRate(0.0f);
        setAnmFrame(719.0f);
    }
    mVec3_c pos = mPos;
    pos.x += param->mObjcOfsX;
    pos.y += param->mObjcOfsY;
    pos.z += param->mObjcOfsZ;
    registObjc(&pos, mBaseRadius, TRUE);
}

// .text 10524
void dInsFldCicada_c::executeAlert() {
    const param_c *param = getKindParam(mType);
    if (isLanternFly() == true && 377.0f < mAnmChr.getFrame()) {
        setAnmRate(0.0f);
        setAnmFrame(710.0f);
    }
    mVec3_c pos = mPos;
    pos.x += param->mObjcOfsX;
    pos.y += param->mObjcOfsY;
    pos.z += param->mObjcOfsZ;
    registObjc(&pos, mBaseRadius, TRUE);
}

// .text 105FC
void dInsFldCicada_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mSpeed, param->mAccelY);
    mRotFrom.slerpTo(mRotTo, mSlerp, mRot);
    mSlerp += 0.08f;
    mSlerp = mSlerp > 1.0f ? 1.0f : mSlerp;
}

// .text 10684
void dInsFldCicada_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mSpeed, param->mAccelY);
}

// .text 106C8
void dInsFldCicada_c::getRestRot(mQuat_c *out) {
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

// .text 107E0
mAng dInsFldCicada_c::getEscapeAngle() {
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

// .text 108A8
bool dInsFldCicada_c::isLanternFly() {
    return mType == INSECT_LANTERN_FLY;
}

// .text 108BC
bool dInsFldCicada_c::isDisturbed() {
    return mPlayerCheck.isDisturbing(mScareRadius);
}

// .text 108C8
void dInsFldCicada_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            u32 state = data->getReq();
            if (state < STATE_NUM_MAX) {
                mState = state;
            }
            return;
        }
    }

    if (mState == STATE_WAIT) {
        if (isLanternFly() == true) {
            if (718.0f < mAnmChr.getFrame() && mScare.isScared() == true) {
                mState = STATE_ALERT;
            }
        } else if (mScare.isScared() == true) {
            mState = STATE_ALERT;
        }
    } else if (mState == STATE_ALERT) {
        if (isLanternFly() == true) {
            if (377.0f < mAnmChr.getFrame() &&
                (u16)dInsectUtil::secToFrame(5.0f) <= mFrame &&
                !mScare.isScared()) {
                mState = STATE_WAIT;
            }
            if (mScare.isEscaping() == true) {
                mState = STATE_ESCAPE;
            }
        } else if (mScare.isEscaping() == true) {
            mState = STATE_ESCAPE;
        } else if (!mScare.isScared()) {
            mState = STATE_WAIT;
        }
    }

    if (!isHeld() && mState != STATE_ESCAPE && mState != STATE_LEAVE &&
        isDisturbed() == true) {
        mState = STATE_ESCAPE;
    }
}

// .text 10A90
void dInsFldCicada_c::calcScare() {
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
