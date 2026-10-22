// dInsFldHornet_c: the hornets. See include/game/game/d_ins_fld_hornet.hpp.

const dInsFldHornet_c::StateFunc dInsFldHornet_c::sExecute[] = {
    &dInsFldHornet_c::executeAppear, &dInsFldHornet_c::executeWait,
    &dInsFldHornet_c::executeChase,  &dInsFldHornet_c::executeEscape,
    &dInsFldHornet_c::executeSting,  &dInsFldHornet_c::executeRelease,
};

const dInsFldHornet_c::StateFunc dInsFldHornet_c::sInit[] = {
    &dInsFldHornet_c::initAppear, &dInsFldHornet_c::initWait,
    &dInsFldHornet_c::initChase,  &dInsFldHornet_c::initEscape,
    &dInsFldHornet_c::initSting,  &dInsFldHornet_c::onRelease,
};

const int dInsFldHornet_c::sTypes[] = {INSECT_BEE};

dInsFldHornet_c::param_c dInsFldHornet_c::sParam[] = {
    {0.0f, 8.0f, 0.0f, 5.0f, 0.3f, 0.94f, 0.01f, 20.0f, 1.2f, 2.5f, 5.0f, 2.5f,
     7.0f, 1.5f, 0.03f, 1.0f, 10.0f, 5.0f},
};

u8 dInsFldHornet_c::sBeeNum;

// 0x16340
int dInsFldHornet_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x16360
const dInsFldHornet_c::param_c *dInsFldHornet_c::getHornetParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x16390
dInsFldHornet_c::dInsFldHornet_c()
    : mState(STATE_APPEAR), mTurnDir(false), mFinished(false),
      mTurnDirSet(false), mCenter(0.0f, 0.0f, 0.0f), mRadius(0.0f),
      mCircleAngle(0), _45A(0), mChaseFrames(0), mObjcTimer(0), mWallTimer(0) {
}

// 0x1646C
dInsFldHornet_c::~dInsFldHornet_c() {
}

// 0x16524
bool dInsFldHornet_c::isBeesOut() {
    return sBeeNum != 0;
}

// 0x1653C
BOOL dInsFldHornet_c::create(const nw4r::g3d::ResFile &res) {
    sBeeNum++;
    dInsFldBase_c::create(res);
    return TRUE;
}

// 0x16570
int dInsFldHornet_c::draw() {
    if (mState == STATE_RELEASE) {
        if (!mActive) {
            return FALSE;
        }
        calcMtx();
        mMdl.calc(false);
        mMdl.entry();
    }
    if (!isZero(mShadowSize)) {
        f32 alpha = mAlpha / 255.0f * sShadowAlphaScale;
        mAng ang = 0;
        dShadow::requestRound(&mPos, 0, &ang, mShadowSize, 1.0f, alpha, 0.0f,
                              1.0f);
    }
    return TRUE;
}

// 0x16678
BOOL dInsFldHornet_c::doDelete() {
    dInsFldBase_c::doDelete();
    sBeeNum--;
    if (sBeeNum == 0) {
        fgMngProc_clearBusy();
    }
    return TRUE;
}

// 0x166B8
void dInsFldHornet_c::calcMtx() {
    setMtxFromAxis();
}

// 0x166BC (its trailing dead blr is dtk's fn_116_166F8)
void dInsFldHornet_c::onHold() {
    if (mFinished) {
        return;
    }
    if (mState == STATE_RELEASE) {
        return;
    }
    mFinished = true;
    dEffect::create("af_bug_bee_finish", &mPos, &mAngle, NULL);
}

// 0x166FC
void dInsFldHornet_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x16734
void dInsFldHornet_c::setGroundPos() {
    const param_c *param = getHornetParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
}

// 0x167A4
void dInsFldHornet_c::initState() {
    if (mSpawnKind != SPAWN_PLAYER) {
        setState(STATE_APPEAR);
    } else {
        setState(STATE_RELEASE);
    }
}

// 0x167C0
void dInsFldHornet_c::executeState() {
    getHornetParam(mType);
    (this->*sExecute[mState])();
}

// 0x16810
void dInsFldHornet_c::initAppear() {
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mPos.y = getGroundY();
    _45A = 120;
    mAngle.x = mAxis.getAngleX();
    mAngle.y = mAxis.getAngleY();
    mAngle.z = mAxis.getAngleZ();
    dEffect::create("af_bug_bee_start", &mPos, &mAngle, NULL);
}

// 0x168D0
void dInsFldHornet_c::initWait() {
    mFrame = 0;
}

// 0x168DC
void dInsFldHornet_c::initChase() {
    const param_c *param = getHornetParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getTargetPos(&mHomePos);
    mAngle.y = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (mWallTimer >= WALL_TURN_TIME) {
        mVec3_c pos = mPos;
        pos += mAxis.getFront() * 32.0f;
        if (dInsectUtil::isStructure(pos) == 1) {
            mVec3_c ofs(0.0f, 0.0f, 200.0f);
            s16 turn = cM::rndRange<s16>(DEG_TO_ANGLE(120), DEG_TO_ANGLE(140));
            mAng angle =
                mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
            angle += turn;
            ofs.rotY(angle);
            mHomePos += ofs;
            mAngle.y = cLib::targetAngleY(mPos, mHomePos);
        }
    }
    mChaseFrames = (int)((mHomePos - mPos).xzLen() / param->mApproachSpeed) + 1;
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x16AEC
void dInsFldHornet_c::initEscape() {
    getHornetParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getTargetPos(&mHomePos);
    mTurnDir = getTurnDir();
}

// 0x16B5C
void dInsFldHornet_c::initSting() {
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mPlayerCheck.getPlayerPos(&mCenter, dPlayerMgr::PLAYER_INDEX_CURRENT);
    mVec3_c diff = mCenter - mPos;
    mRadius = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mCircleAngle = cLib::targetAngleY(mCenter, mPos);
}

// 0x16BF4
void dInsFldHornet_c::onRelease() {
    const param_c *param = getHornetParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x16C9C
void dInsFldHornet_c::executeAppear() {
    setState(STATE_WAIT);
    mSound.holdSound(SE_hachi_fly);
}

// 0x16CD8
void dInsFldHornet_c::executeWait() {
    if (mFrame == WAIT_TIME) {
        dBgm::l_mgr.mEvent.play14();
        setState(STATE_CHASE);
    }
    mSound.holdSound(SE_hachi_fly);
}

// 0x16D34
void dInsFldHornet_c::executeChase() {
    const param_c *param = getHornetParam(mType);
    mObjcTimer++;
    int objcDelay = dInsectUtil::secToFrame(param->mObjcDelay);
    u16 timer = mObjcTimer;
    if (timer > (u16)objcDelay) {
        timer = objcDelay;
    }
    mObjcTimer = timer;
    mVec3_c playerPos;
    mPlayerCheck.getPlayerPos(&playerPos, dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (mPlayerCheck.isValid(dPlayerMgr::PLAYER_INDEX_CURRENT) == true) {
        if (dPlayerMgr::isSafeFromInsects() == TRUE) {
            setState(STATE_ESCAPE);
        } else if (!mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT,
                                        param->mNearRadius)) {
            flyToHome(0.0f, param->mFlyAccel, param->mFlyDrag, param->mFlyTurn);
            if (mChaseFrames <= mFrame) {
                setState(STATE_CHASE);
            }
        } else {
            if (!mTurnDirSet) {
                mTurnDirSet = true;
                mTurnDir = getTurnDir();
            }
            const param_c *circle = getHornetParam(mType);
            mVec3_c center;
            mPlayerCheck.getPlayerPos(&center,
                                      dPlayerMgr::PLAYER_INDEX_CURRENT);
            if (20.0f + center.y < mPos.y) {
                mPos.y -= 0.3f;
            }
            mRadius = 20.0f;
            mVec3_c old = mPos;
            if (mTurnDir == true) {
                int step = 182.0f * circle->mCircleSpeed;
                mCircleAngle += step;
            } else {
                int step = 182.0f * circle->mCircleSpeed;
                mCircleAngle -= step;
            }
            mPos.x = center.x + mRadius * mAng(mCircleAngle).sin();
            mPos.z = center.z + mRadius * mAng(mCircleAngle).cos();
            mAngle.y = cLib::targetAngleY(old, mPos);
            if (playerPos.y < mPos.y && mPos.y - playerPos.y < 60.0f &&
                (u32)dPlayerMgr::requestBeeSting(NULL) == TRUE) {
                setState(STATE_STING);
            }
        }
    }
    if (20.0f + mPos.y < 40.0f + playerPos.y) {
        mPos.y += param->mRiseSpeed;
        mPos.y = mPos.y > 40.0f + playerPos.y ? 40.0f + playerPos.y : mPos.y;
    } else {
        f32 height = getChaseHeight();
        if (height < mPos.y) {
            mPos.y -= 1.2f;
            mPos.y = mPos.y < height ? height : mPos.y;
        } else if (mPos.y < height) {
            mPos.y += 0.3f;
            mPos.y = mPos.y > height ? height : mPos.y;
        }
    }
    if ((u16)objcDelay <= mObjcTimer) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
    mAngle.x = mAxis.getAngleX();
    mAngle.y = mAxis.getAngleY();
    mAngle.z = mAxis.getAngleZ();
    mGroupEffect.create("af_bug_bee_group", &mPos, &mAngle, NULL, 1);
    mTailEffect.create("af_bug_bee_tail01", &mPos, &mAngle, NULL, 1);
    mSound.holdSound(SE_hachi_fly);
    if (isSceneChanging() == TRUE) {
        dEffect::create("af_bug_bee_finish", &mPos, &mAngle, NULL);
        requestDelete();
    }
    if (isWallHit() == true) {
        mWallTimer++;
    } else {
        mWallTimer = 0;
    }
}

// 0x17160
void dInsFldHornet_c::executeEscape() {
    const param_c *param = getHornetParam(mType);
    if (!dPlayerMgr::isSafeFromInsects()) {
        setState(STATE_CHASE);
    }
    if (mTurnDir == true) {
        int turn = 364.0f * param->mEscapeTurn;
        mAngle.y.mAngle += turn;
    } else {
        int turn = 364.0f * param->mEscapeTurn;
        mAngle.y.mAngle -= turn;
    }
    mVec3_c move(0.0f, 0.0f, param->mEscapeSpeed);
    move.rotY(mAngle.y);
    mPos += move;
    registObjc(&mPos, mBaseRadius, TRUE);
    mGroupEffect.create("af_bug_bee_group", &mPos, &mAngle, NULL, 1);
    mTailEffect.create("af_bug_bee_tail01", &mPos, &mAngle, NULL, 1);
    mSound.holdSound(SE_hachi_fly);
    if (isSceneChanging() == TRUE) {
        dEffect::create("af_bug_bee_finish", &mPos, &mAngle, NULL);
        requestDelete();
    }
}

// 0x17304
void dInsFldHornet_c::executeSting() {
    const param_c *param = getHornetParam(mType);
    mVec3_c playerPos;
    mPlayerCheck.getPlayerPos(&playerPos, dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (20.0f + playerPos.y < mPos.y) {
        mPos.y -= 0.3f;
    }
    mRadius *= 0.999f;
    mVec3_c old = mPos;
    if (mTurnDir == true) {
        int step = 182.0f * param->mCircleSpeed;
        mCircleAngle += step;
    } else {
        int step = 182.0f * param->mCircleSpeed;
        mCircleAngle -= step;
    }
    mPos.x = mCenter.x + mRadius * mAng(mCircleAngle).sin();
    mPos.z = mCenter.z + mRadius * mAng(mCircleAngle).cos();
    mAngle.y = cLib::targetAngleY(old, mPos);
    mGroupEffect.create("af_bug_bee_group", &mPos, &mAngle, NULL, 1);
    mTailEffect.create("af_bug_bee_tail01", &mPos, &mAngle, NULL, 1);
    if (mFrame >= STING_LEAVE_TIME) {
        mVec3_c pos;
        mPlayerCheck.getPlayerPos(&pos, dPlayerMgr::PLAYER_INDEX_CURRENT);
        s16 ang = cLib::targetAngleY(pos, mPos);
        if (ang >= 0x78E4 ||
            ang <= -0x78E4 || // ~170 degrees: behind the player
            mFrame >= STING_END_TIME) {
            mGroupEffect.kill();
            mTailEffect.kill();
            dEffect::create("af_bug_bee_finish", &mPos, &mAngle, NULL);
            requestDelete();
        }
    }
    mSound.holdSound(SE_hachi_fly);
}

// 0x1754C
void dInsFldHornet_c::executeRelease() {
    const param_c *param = getHornetParam(mType);
    moveForward(param->mReleaseSpeed, param->mReleaseAccelY);
}

// 0x17590
f32 dInsFldHornet_c::getChaseHeight() {
    getHornetParam(mType);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != 0) {
        return 40.0f + chk.getWaterY();
    }
    return 40.0f + getGroundY();
}

// 0x1760C
mAng dInsFldHornet_c::getAngleToPlayer() {
    return mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
}

// 0x17618
f32 dInsFldHornet_c::getTargetDist() {
    return 16.0f;
}

// 0x17624
void dInsFldHornet_c::getTargetPos(mVec3_c *out) {
    mAng ang = getAngleToPlayer();
    f32 dist = getTargetDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    getHornetParam(mType);
    mVec3_c from = mPos;
    mVec3_c to = target;
    u8 steps = (int)(dist / 32.0f) + 1;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, FALSE, TRUE,
                                       -500.0f, 5.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 1, FALSE);
    }
    *out = pos;
}

// 0x17798
void dInsFldHornet_c::flyToHome(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
}

// 0x1779C
bool dInsFldHornet_c::getTurnDir() {
    mVec3_c diff = mPos - mCenter;
    f32 vx = mVelocity.x;
    f32 vz = mVelocity.z;
    vz = __fabsf(vz);
    vx = __fabsf(vx);
    if (diff.x < 0.0f) {
        if (diff.z < 0.0f) {
            if (vx < vz) {
                return true;
            } else {
                return false;
            }
        } else if (vx < vz) {
            return false;
        } else {
            return true;
        }
    } else if (diff.z < 0.0f) {
        if (vx < vz) {
            return false;
        } else {
            return true;
        }
    }
    return vx < vz;
}
