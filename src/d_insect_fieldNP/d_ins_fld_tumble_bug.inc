// dInsFldTumbleBug_c: the dung beetle. See
// include/game/game/d_ins_fld_tumble_bug.hpp.

const dInsFldTumbleBug_c::StateFunc dInsFldTumbleBug_c::sExecute[] = {
    &dInsFldTumbleBug_c::executeWait,    &dInsFldTumbleBug_c::executeWalk,
    &dInsFldTumbleBug_c::executePush,    &dInsFldTumbleBug_c::executeEscape,
    &dInsFldTumbleBug_c::executeRelease,
};

const dInsFldTumbleBug_c::StateFunc dInsFldTumbleBug_c::sInit[] = {
    &dInsFldTumbleBug_c::initWait,  &dInsFldTumbleBug_c::initWalk,
    &dInsFldTumbleBug_c::initPush,  &dInsFldTumbleBug_c::initEscape,
    &dInsFldTumbleBug_c::onRelease,
};

const int dInsFldTumbleBug_c::sNoBall = 3;

const int dInsFldTumbleBug_c::sTypes[] = {INSECT_DUNG_BEETLE};

dInsFldTumbleBug_c::param_c dInsFldTumbleBug_c::sParam[] = {
    {0.0f,   0.0f, 5.5f,  6.0f, 1.0f, 0.85f, 8.0f, 2.0f, 30.0f, 10.0f,  0.07f,
     0.007f, 0.9f, 0.01f, 3.0f, 0.5f, 0.02f, 1.0f, 6.0f, 8.0f,  100.0f, 150},
};

u8 dInsFldTumbleBug_c::sBallTaken[daSnowMan_c::BALL_NUM];

// 0x1BBB0
int dInsFldTumbleBug_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x1BBD0
const dInsFldTumbleBug_c::param_c *
dInsFldTumbleBug_c::getTumbleBugParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x1BC00
dInsFldTumbleBug_c::dInsFldTumbleBug_c()
    : mState(STATE_WAIT), mWallNormal(0.0f, 0.0f, 0.0f), mWaitFrames(0),
      mMoveFrames(0), _334(480), _336(mAng::fromDegree(0.05f)), mPushTimer(0) {
}

// 0x1BCA0
dInsFldTumbleBug_c::~dInsFldTumbleBug_c() {
}

// 0x1BD08
BOOL dInsFldTumbleBug_c::create(const nw4r::g3d::ResFile &res) {
    dInsFldBase_c::create(res);
    findNearestBall();
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
    return TRUE;
}

// 0x1BD84
int dInsFldTumbleBug_c::draw() {
    mAxis.rotateY(DEG_TO_ANGLE(180));
    dInsFldBase_c::draw();
    mAxis.rotateY(DEG_TO_ANGLE(180));
    return TRUE;
}

// 0x1BDE4
BOOL dInsFldTumbleBug_c::doDelete() {
    sBallTaken[mBall.mIdx] = false;
    dInsFldBase_c::doDelete();
    return TRUE;
}

// 0x1BE1C
void dInsFldTumbleBug_c::calcMtx() {
    setMtxFromAxis();
}

// 0x1BE20
BOOL dInsFldTumbleBug_c::isBallTaken(int ball) {
    return sBallTaken[ball];
}

// 0x1BE30
BOOL dInsFldTumbleBug_c::canSpawn(const mVec3_c *pos, int ball,
                                  void *awayFromPlayers) {
    if (dNet::isSessionActive() == true) {
        return FALSE;
    }
    if (!isBallTaken(ball)) {
        if (awayFromPlayers == (void *)TRUE) {
            int blockX = 0;
            int blockZ = 0;
            int unitX = 0;
            int unitZ = 0;
            dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, *pos);
            if (!dInsectField::isAwayFromPlayers(blockX, blockZ, unitX,
                                                 unitZ)) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 0x1BEF8
void dInsFldTumbleBug_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x1BF30
void dInsFldTumbleBug_c::setGroundPos() {
    getTumbleBugParam(mType);
    if (mState != STATE_RELEASE) {
        mBall.set(findNearestBall());
        if (mBall.isValid() == true) {
            mVec3_c ballPos = mBall.get()->getPos();
            mVec3_c ofs(0.0f, 0.0f, 0.75f * mBall.getRadius());
            ofs.rotY(mAngle.y.mAngle - DEG_TO_ANGLE(180));
            mPos.x = ballPos.x + ofs.x;
            mPos.y = ofs.y + getGroundY();
            mPos.z = ballPos.z + ofs.z;
        }
    }
}

// 0x1C024
void dInsFldTumbleBug_c::initState() {
    setState(STATE_WAIT);
}

// 0x1C02C
void dInsFldTumbleBug_c::executeState() {
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    mAlive = true;
#endif
    getTumbleBugParam(mType);
    if (findNearestBall() == sNoBall && mState != STATE_RELEASE) {
        requestDelete();
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
        mAlive = false;
#endif
        return;
    }
    if (mState != STATE_ESCAPE && mState != STATE_RELEASE &&
        (mBall.cannotPush() == true || mBall.isFar(&mPos) == true)) {
        setState(STATE_ESCAPE);
    }
    s16 timer = mPushTimer - 1;
    mPushTimer = timer < 0 ? 0 : timer;
    if (mPushTimer == 0) {
        mBgCheck = true;
    } else {
        mBgCheck = false;
    }
    (this->*sExecute[mState])();
}

// 0x1C11C
void dInsFldTumbleBug_c::initWait() {
    const param_c *param = getTumbleBugParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    int max = dInsectUtil::secToFrame(param->mWaitMax);
    int min = dInsectUtil::secToFrame(param->mWaitMin);
    mWaitFrames = cM::rndRange<u16>(min, max);
}

// 0x1C1C4
void dInsFldTumbleBug_c::initWalk() {
    const param_c *param = getTumbleBugParam(mType);
    mFrame = 0;
    mWalkTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(-param->mWalkAnmRate);
    setAnmFrame(0.0f);
    int max = dInsectUtil::secToFrame(param->mWalkTimeMax);
    int min = dInsectUtil::secToFrame(param->mWalkTimeMin);
    mWalkFrames = cM::rndRange<u16>(min, max);
    setWalkTarget();
}

// 0x1C27C
void dInsFldTumbleBug_c::initPush() {
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
}

// 0x1C2EC
void dInsFldTumbleBug_c::initEscape() {
    const param_c *param = getTumbleBugParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    s16 away =
        mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT).mAngle -
        DEG_TO_ANGLE(180);
    s16 npcAway = away;
    if (mNpcCheck.isNear(mScareRadius) == true) {
        npcAway = mNpcCheck.getAngleTo().mAngle - DEG_TO_ANGLE(180);
    }
    mAngle.y = (away + npcAway) >> 1;
    mSound.startSound(SE_mushi_fly_out_midium);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x1C3F4
void dInsFldTumbleBug_c::onRelease() {
    const param_c *param = getTumbleBugParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(0.0f);
    mSound.startSound(SE_mushi_fly_out_midium);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x1C4A8
void dInsFldTumbleBug_c::executeWait() {
    const param_c *param = getTumbleBugParam(mType);
    mVelocity *= param->mWaitDecay;
    mPos += mVelocity;
    if (!isHeld() && (!mBall.cannotPush() || !mBall.isFar(&mPos))) {
        daSnowMan_c *ball = mBall.get();
        mVec3_c hit(0.0f, 0.0f, 0.75f * mBall.getRadius());
        hit.rotY(mAngle.y);
        hit += mPos;
        ball->hit(&hit);
    }
#if !(VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01)
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    if (mWaitFrames <= mFrame) {
        setState(STATE_WALK);
    } else if (mScare.isScared() == true) {
        setState(STATE_ESCAPE);
    } else if (dInsectUtil::isColumn(mPos) == true) {
        setState(STATE_ESCAPE);
    }
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (mAlive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1C674
void dInsFldTumbleBug_c::executeWalk() {
    const param_c *param = getTumbleBugParam(mType);
    f32 accel = param->mWalkAccel / (mBall.getRadius() / 5.0f);
    mBall.getRadius();
    walk(0.0f, accel, param->mWalkDrag, param->mWalkTurn);
#if !(VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01)
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    if (!isHeld() && (!mBall.cannotPush() || !mBall.isFar(&mPos))) {
        daSnowMan_c *ball = mBall.get();
        mVec3_c hit(0.0f, 0.0f, 0.75f * mBall.getRadius());
        hit.rotY(mAngle.y);
        hit += mPos;
        ball->hit(&hit);
    }
    mAngle.y = mAxis.getAngleY();
    if (mWalkFrames <= mWalkTimer) {
        setState(STATE_WAIT);
    } else if (mMoveFrames <= mFrame) {
        setWalkTarget();
    } else if (mBall.getWallNormal(&mWallNormal) == true && mPushTimer == 0) {
        setState(STATE_PUSH);
        mPushTimer = PUSH_START_TIME;
    } else if (mScare.isScared() == true) {
        setState(STATE_ESCAPE);
    }
    mWalkTimer++;
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (mAlive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1C878
void dInsFldTumbleBug_c::executePush() {
    mPushTimer = PUSH_COOLDOWN_TIME;
#if !(VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01)
    registObjc(&mPos, mBaseRadius, TRUE);
#endif
    mVec3_c normal = mWallNormal;
    normal.rotY(DEG_TO_ANGLE(180));
    mVec3_c origin(0.0f, 0.0f, 0.0f);
    mAng wallAng = cLib::targetAngleY(origin, normal) - DEG_TO_ANGLE(180);
    mVec3_c ballPos(0.0f, 0.0f, 0.0f);
    mBall.getPos(&ballPos);
    mVec3_c toBall = ballPos - mPos;
    mVec3_c wallDir(0.0f, 0.0f, 1.0f);
    wallDir.rotY(wallAng);
    mAng ang = mBall.getAngleFrom(&mPos);
    if (ang.mAngle - wallAng.mAngle > 0) {
        mAngle.y.mAngle -= ANGLE_PER_DEGREE;
    } else {
        mAngle.y.mAngle += ANGLE_PER_DEGREE;
    }
    mVec3_c ofs(0.0f, 0.0f, 0.75f * mBall.getRadius());
    ofs.rotY(mAngle.y.mAngle - DEG_TO_ANGLE(180));
    mPos = ballPos + ofs;
    if (labs((s16)(ang.mAngle - wallAng.mAngle)) < ANGLE_PER_DEGREE) {
        setState(STATE_WALK);
    }
    mVec3_c front(0.0f, 0.0f, 1.0f);
    front.rotY(mAngle.y);
    mAxis.setFront(front);
#if VERSION == VERSION_RUUE01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    if (mAlive == true) {
        registObjc(&mPos, mBaseRadius, TRUE);
    }
#endif
}

// 0x1CA5C
void dInsFldTumbleBug_c::executeEscape() {
    const param_c *param = getTumbleBugParam(mType);
    moveForward(param->mEscapeSpeed, param->mEscapeAccelY);
}

// 0x1CAA0
void dInsFldTumbleBug_c::executeRelease() {
    const param_c *param = getTumbleBugParam(mType);
    moveForward(param->mEscapeSpeed, param->mEscapeAccelY);
}

// 0x1CAE4
int dInsFldTumbleBug_c::findNearestBall() {
    f32 best = 10000.0f;
    int found = sNoBall;
    for (int i = 0; i < daSnowMan_c::BALL_NUM; i++) {
        daSnowMan_c *ball = daSnowMan_c::sBall[i];
        if (ball != NULL) {
            mVec3_c pos = ball->getPos();
            mVec3_c diff = pos - mPos;
            f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
            if (dist < best) {
                best = dist;
                found = i;
            }
        }
    }
    return found;
}

// 0x1CBBC
mAng dInsFldTumbleBug_c::getWalkAngle() {
    s16 ang = mAngle.y;
    int range = DEG_TO_ANGLE(30);
    return ang + (int)cM::rndCenterF(range);
}

// 0x1CC20
f32 dInsFldTumbleBug_c::getWalkDist() const {
    const param_c *param = getTumbleBugParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x1CC50
void dInsFldTumbleBug_c::getWalkTarget(mVec3_c *out) {
    mAng ang = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    getTumbleBugParam(mType);
    mVec3_c from = mPos;
    mVec3_c to = target;
    u8 steps = (int)(dist / 32.0f) + 1;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, TRUE, TRUE,
                                       -30.0f, 5.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 0, TRUE);
    }
    *out = pos;
}

// 0x1CDC4
void dInsFldTumbleBug_c::setWalkTarget() {
    const param_c *param = getTumbleBugParam(mType);
    mFrame = 0;
    getWalkTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mMoveFrames = (int)(dist / param->mWalkSpeed) + 1;
}

// 0x1CE58
void dInsFldTumbleBug_c::walk(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
}

// 0x1CEAC
dInsFldTumbleBug_c::ball_c::ball_c() : mIdx(0) {
}

// 0x1CEB8
dInsFldTumbleBug_c::ball_c::~ball_c() {
}

// 0x1CEF8
void dInsFldTumbleBug_c::ball_c::set(int idx) {
    mIdx = idx;
}

// 0x1CF00
bool dInsFldTumbleBug_c::ball_c::isValid() {
    return daSnowMan_c::sBall[mIdx] != NULL;
}

// 0x1CF24
BOOL dInsFldTumbleBug_c::ball_c::canRoll() {
    if (isValid() == true) {
        return get()->canRoll();
    }
    return FALSE;
}

// 0x1CF6C
bool dInsFldTumbleBug_c::ball_c::isFar(const mVec3_c *pos) {
    if (isValid() == true) {
        mVec3_c ballPos = get()->getPos();
        mVec3_c diff = ballPos - *pos;
        f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
        if (1.0f + getRadius() < dist) {
            return true;
        }
    }
    return false;
}

// 0x1D030
bool dInsFldTumbleBug_c::ball_c::getWallNormal(mVec3_c *normal) {
    if (isValid() == true) {
        return get()->getWallNormal(normal);
    }
    return FALSE;
}

// 0x1D088
bool dInsFldTumbleBug_c::ball_c::cannotPush() {
    if (!isValid() || !canRoll()) {
        return true;
    }
    return false;
}

// 0x1D0D8
f32 dInsFldTumbleBug_c::ball_c::getRadius() {
    if (isValid() == true) {
        return 16.0f * get()->mScale;
    }
    return 0.0f;
}

// 0x1D130
daSnowMan_c *dInsFldTumbleBug_c::ball_c::get() {
    if (isValid() == true) {
        return daSnowMan_c::sBall[mIdx];
    }
    return NULL;
}

// 0x1D180
void dInsFldTumbleBug_c::ball_c::getPos(mVec3_c *out) {
    if (isValid() == true) {
        *out = get()->getPos();
    }
}

// 0x1D1E0
mAng dInsFldTumbleBug_c::ball_c::getAngleFrom(const mVec3_c *pos) {
    s16 ang = 0;
    if (isValid() == true) {
        mVec3_c ballPos;
        getPos(&ballPos);
        ang = cLib::targetAngleY(*pos, ballPos);
    }
    return ang;
}
