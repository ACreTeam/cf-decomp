// dInsFldButterfly_c: the butterflies. See
// include/game/game/d_ins_fld_butterfly.hpp.

const dInsFldButterfly_c::StateFunc dInsFldButterfly_c::sExecute[] = {
    &dInsFldButterfly_c::executeFly,     &dInsFldButterfly_c::executeApproach,
    &dInsFldButterfly_c::executeLand,    &dInsFldButterfly_c::executeRest,
    &dInsFldButterfly_c::executeTakeOff, &dInsFldButterfly_c::executeEscape,
    &dInsFldButterfly_c::executeLeave,
};

const dInsFldButterfly_c::StateFunc dInsFldButterfly_c::sInit[] = {
    &dInsFldButterfly_c::initFly,     &dInsFldButterfly_c::initApproach,
    &dInsFldButterfly_c::initLand,    &dInsFldButterfly_c::initRest,
    &dInsFldButterfly_c::initTakeOff, &dInsFldButterfly_c::initEscape,
    &dInsFldButterfly_c::onRelease,
};

const int dInsFldButterfly_c::sSounds[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, SE_mushi_out_habataki};

// .data 8
dInsFldButterfly_c::param_c dInsFldButterfly_c::sParam[PARAM_NUM] = {
    {0.0f,  30.0f, 12.0f, 0.2f,  0.02f, 0.9f,  0.03f, 20.0f, 10.0f, 12.0f,
     2.0f,  1.0f,  1.2f,  1.5f,  3.0f,  3.0f,  0.05f, 0.05f, 0.0f,  1.3f,
     0.2f,  0.02f, 0.9f,  0.07f, 40.0f, 20.0f, 10.0f, 2.0f,  2.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 1.8f,  6.0f,  5.0f,  80.0f, 150},
    {0.0f,  30.0f, 12.0f, 0.2f,  0.02f, 0.9f,  0.03f, 20.0f, 10.0f, 12.0f,
     2.0f,  1.0f,  1.2f,  1.5f,  3.0f,  3.0f,  0.05f, 0.05f, 0.0f,  1.3f,
     0.2f,  0.02f, 0.9f,  0.07f, 40.0f, 20.0f, 10.0f, 2.0f,  2.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 1.8f,  6.0f,  5.0f,  80.0f, 150},
    {0.0f,  30.0f, 12.0f, 0.25f, 0.025f, 0.9f,  0.02f, 30.0f,  20.0f, 12.0f,
     4.0f,  3.0f,  1.2f,  1.5f,  3.0f,   3.0f,  0.08f, 0.08f,  0.0f,  1.5f,
     0.5f,  0.05f, 0.9f,  0.04f, 40.0f,  20.0f, 12.0f, 4.0f,   3.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 1.8f,   8.0f,  7.0f,  100.0f, 120},
    {0.0f,  30.0f, 12.0f, 0.25f, 0.025f, 0.9f,  0.02f, 30.0f,  20.0f, 12.0f,
     4.0f,  3.0f,  1.2f,  1.5f,  3.0f,   3.0f,  0.08f, 0.08f,  0.0f,  1.5f,
     0.5f,  0.05f, 0.9f,  0.04f, 40.0f,  20.0f, 12.0f, 4.0f,   3.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 1.8f,   8.0f,  7.0f,  100.0f, 120},
    {0.0f,  30.0f, 12.0f, 0.2f,  0.02f, 0.9f,  0.02f, 20.0f, 10.0f, 5.0f,
     4.0f,  2.0f,  0.9f,  1.5f,  3.0f,  3.0f,  0.15f, 0.15f, 0.0f,  1.3f,
     0.2f,  0.02f, 0.9f,  0.03f, 40.0f, 20.0f, 5.0f,  4.0f,  2.0f,  1.0f,
     20.0f, 10.0f, 0.5f,  0.03f, 1.4f,  7.0f,  5.0f,  80.0f, 120},
    {0.0f,  20.0f, 12.0f, 0.4f,  0.02f, 0.95f, 0.03f, 30.0f,  10.0f, 5.0f,
     4.0f,  2.0f,  1.0f,  1.5f,  10.0f, 15.0f, 0.05f, 0.05f,  0.0f,  1.3f,
     1.2f,  0.12f, 0.9f,  0.05f, 50.0f, 30.0f, 5.0f,  2.0f,   2.0f,  1.4f,
     20.0f, 10.0f, 0.9f,  0.05f, 1.6f,  9.0f,  6.0f,  100.0f, 80},
    {0.0f,  30.0f, 12.0f, 0.8f,  0.08f, 0.9f,  0.07f, 50.0f,  30.0f, 5.0f,
     6.0f,  5.0f,  1.2f,  1.5f,  3.0f,  3.0f,  0.1f,  0.1f,   0.0f,  1.3f,
     1.5f,  0.15f, 0.9f,  0.1f,  70.0f, 50.0f, 5.0f,  6.0f,   5.0f,  1.2f,
     20.0f, 10.0f, 0.7f,  0.04f, 2.0f,  9.0f,  5.0f,  100.0f, 120},
    {0.0f,  30.0f, 12.0f, 0.5f,  0.05f, 0.9f,  0.04f, 30.0f,  20.0f, 12.0f,
     4.0f,  3.0f,  1.2f,  1.5f,  3.0f,  3.0f,  0.08f, 0.08f,  0.0f,  1.5f,
     0.7f,  0.07f, 0.9f,  0.07f, 40.0f, 20.0f, 12.0f, 4.0f,   3.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 2.2f,  8.0f,  8.0f,  150.0f, 120},
    {0.0f,  60.0f, 12.0f, 0.4f,  0.04f, 0.9f,  0.03f, 50.0f,  30.0f, 3.0f,
     6.0f,  4.0f,  1.2f,  1.8f,  3.0f,  3.0f,  0.3f,  0.3f,   0.0f,  1.8f,
     0.8f,  0.08f, 0.9f,  0.06f, 60.0f, 40.0f, 3.0f,  6.0f,   4.0f,  1.5f,
     20.0f, 10.0f, 0.7f,  0.04f, 2.2f,  13.0f, 12.0f, 220.0f, 80},
};

// .text 8268
int dInsFldButterfly_c::getParamIdx(int type) {
    const int types[] = {
        INSECT_COMMON_BUTTERFLY,   INSECT_YELLOW_BUTTERFLY,
        INSECT_TIGER_BUTTERFLY,    INSECT_PEACOCK_BUTTERFLY,
        INSECT_MONARCH_BUTTERFLY,  INSECT_EMPEROR_BUTTERFLY,
        INSECT_AGRIAS_BUTTERFLY,   INSECT_RAJA_BROOKE,
        INSECT_BIRDWING_BUTTERFLY,
    };
    for (int i = 0; i < PARAM_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text 8354
const dInsFldButterfly_c::param_c *dInsFldButterfly_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text 8384
int dInsFldButterfly_c::getLeaveSound(int type) {
    return sSounds[getParamIdx(type)];
}

// .text 83B4
bool dInsFldButterfly_c::isLandingFlower(const dItem::Item *item,
                                         const mVec3_c *pos) {
    if (item->isFlower() == true && !dInsectUtil::isExcludedFlower(pos)) {
        return true;
    }
    return false;
}

// .text 8404
dInsFldButterfly_c::dInsFldButterfly_c()
    : mState(STATE_FLY), mPrevState(STATE_FLY), mFlowerSearch(isLandingFlower),
      mFlutter(0), mFlutterAmp(0.0f), mBaseY(0.0f), mStartPos(0.0f, 0.0f, 0.0f),
      mFlyTime(0), mApproachTime(0), mLandTime(0), mRestTime(0),
      mTakeOffTime(0), mEscapeTime(0), mSearchDelay(0), mSearchTimer(0),
      mSway(0) {
}

// .text 84A8
dInsFldButterfly_c::~dInsFldButterfly_c() {
}

// .text 8510
BOOL dInsFldButterfly_c::create(const nw4r::g3d::ResFile &res) {
    const param_c *param = getKindParam(mType);
    mSearchDelay = getSearchDelay();
    dInsFldBase_c::create(res);
    mBaseY = mPos.y;
    mOldPos = mPos;
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (isBirdwing() == true) {
        mBgRadius = 1.5f * param->mBgRadius;
    }
    return TRUE;
}

// .text 85D4
int dInsFldButterfly_c::execute() {
    const param_c *param = getKindParam(mType);
    mFlowerSearch.search(mPos, 48.0f);
    mBgCheck = TRUE;
    dInsFldBase_c::execute();
    mSearchTimer++;
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    mSearchTimer = mSearchTimer > (u16)max ? max : mSearchTimer;
    return TRUE;
}

// .text 8664
int dInsFldButterfly_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text 8688
void dInsFldButterfly_c::calcMtx() {
    const param_c *param = getKindParam(mType);
    mVec3_c pos = mPos;
    mVec3_c side = mAxis.getSide();
    if (mState == STATE_FLY || mState == STATE_LEAVE) {
        mSway.mAngle += mAng::fromDegree(param->mFlutterSpeed) / 5;
    } else {
        mSway = 0.95f * mSway;
    }
    pos += side * mSway.sin() * 2.0f;
    mMtx_c mtx;
    mMtx_c rot;
    nw4r::math::MTX34 axisMtx;
    dActor_c::makeMtx(&mtx, &pos, 0);
    dInsectUtil::getMtx(axisMtx.mtx, mAxis);
    rot = axisMtx;
    mtx.concat(rot);
    mMdl.setLocalMtx(&mtx);
}

// .text 8890
void dInsFldButterfly_c::onBecomeOwner() {
    const param_c *param = getKindParam(mType);
    if (mPos.y < dBGCF::getGroundY(&mPos, TRUE)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = param->mHeight + dBGCF::getGroundY(&pos, FALSE);
        mPos = pos;
        mOldPos = mPos;
    }
}

// .text 89A8
void dInsFldButterfly_c::setState(int state) {
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

// .text 8B38
void dInsFldButterfly_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x += param->mOfsX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOfsZ;
}

// .text 8BA8
void dInsFldButterfly_c::initState() {
    setState(STATE_FLY);
}

// .text 8BB0
void dInsFldButterfly_c::executeState() {
    getKindParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    checkState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// .text 8C24
void dInsFldButterfly_c::initFly() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    if (mPrevState == STATE_REST) {
        setAnmChr(mAnmChrWait, 60.0f);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
    }
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    setFlyTarget();
    mVelocity.y = 0.0f;
}

// .text 8CF4
void dInsFldButterfly_c::initApproach() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    getFlowerPos(&mHomePos);
    mStartPos = mPos;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mApproachTime =
        (int)(0.8f * (mStartPos.distTo(mHomePos) / param->mSpeed)) + 1;
}

// .text 8DF8
void dInsFldButterfly_c::initLand() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mLandAnmRate);
    setAnmFrame(frame);
    getFlowerPos(&mHomePos);
    mStartPos = mPos;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mLandTime =
        (int)(mStartPos.distTo(mHomePos) / param->mSpeed) + LAND_EXTRA_TIME;
}

// .text 8EF0
void dInsFldButterfly_c::initRest() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrStop, 60.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    mRestTime = cM::rndRange<u16>(min, max);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        mVec3_c ofs;
        mVec3_c pos = mHomePos;
        dItem::Item *item = dInsectUtil::getItem(&pos);
        if (item != NULL) {
            dInsectField::getItemOffset(&ofs, item);
            mHomePos.y = ofs.y + dBGCF::getGroundY(&mHomePos, FALSE);
        }
    }
}

// .text 9000
void dInsFldButterfly_c::initTakeOff() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 10.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mTakeOffAnmRate);
    setAnmFrame(0.0f);
    mFlutter = 0;
    mTakeOffTime = TAKE_OFF_TIME;
}

// .text 9094
void dInsFldButterfly_c::initEscape() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    if (mPrevState == STATE_REST) {
        setAnmChr(mAnmChrWait, 60.0f);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
    }
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mEscapeAnmRate);
    setAnmFrame(frame);
    setEscapeTarget();
    mVelocity.y = 0.0f;
}

// .text 9164
void dInsFldButterfly_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
    int sound = getLeaveSound(mType);
    if (sound > 0) {
        mSound.startSound(sound);
    }
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// .text 9228
void dInsFldButterfly_c::executeFly() {
    const param_c *param = getKindParam(mType);
    fly(0.0f, param->mFlyAccel, param->mFlyDrag, param->mFlyTurn);
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mFlyTime <= mFrame) {
        setFlyTarget();
    }
}

// .text 92AC
void dInsFldButterfly_c::executeApproach() {
    const param_c *param = getKindParam(mType);
    if (mFrame < mApproachTime) {
        fly(0.0f, param->mFlyAccel, param->mFlyDrag, 2.0f * param->mFlyTurn);
        flutterApproach(1.0f - (f32)mFrame / (f32)mApproachTime);
    }
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 9380
void dInsFldButterfly_c::executeLand() {
    const param_c *param = getKindParam(mType);
    cLib::addCalcPos2(&mPos, mHomePos, 0.1f, param->mSpeed);
    mBaseY = mPos.y;
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 93E8
void dInsFldButterfly_c::executeRest() {
    registObjc(&mPos, mBaseRadius, TRUE);
    mSearchTimer = 0;
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        cLib::addCalcPos2(&mPos, mHomePos, 0.1f, 0.3f);
    }
}

// .text 945C
void dInsFldButterfly_c::executeTakeOff() {
    const param_c *param = getKindParam(mType);
    if (mFrame < mTakeOffTime) {
        mVelocity.y *= param->mTakeOffDrag;
        mVelocity.y += param->mTakeOffAccel;
        mPos.y += mVelocity.y;
        mBaseY = mPos.y;
    }
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 94DC
void dInsFldButterfly_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    fly(0.0f, param->mEscapeAccel, param->mEscapeDrag, param->mEscapeTurn);
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mEscapeTime <= mFrame) {
        setEscapeTarget();
    }
}

// .text 9560
void dInsFldButterfly_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mLeaveSpeed, param->mLeaveAccelY);
}

// .text 95A4
void dInsFldButterfly_c::flutter() {
    const param_c *param = getKindParam(mType);
    f32 height = getBaseHeight();
    if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
    mAng ampMax;
    mAng ampMin;
    mAng speed;
    if (mState == STATE_ESCAPE) {
        const f32 maxValue = param->mEscapeFlutterAmpMax;
        const f32 minValue = param->mEscapeFlutterAmpMin;
        const f32 speedValue = param->mEscapeFlutterSpeed;
        ampMax = maxValue;
        ampMin = minValue;
        speed = speedValue;
    } else {
        const f32 maxValue = param->mFlutterAmpMax;
        const f32 minValue = param->mFlutterAmpMin;
        const f32 speedValue = param->mFlutterSpeed;
        ampMax = maxValue;
        ampMin = minValue;
        speed = speedValue;
    }
    s16 next = mFlutter.mAngle + mAng::fromDegree(speed);
    if (mFlutter.mAngle > 0 && next <= 0) {
        mFlutterAmp = (s16)cM::rndRange<mAng>(ampMin, ampMax);
    } else if (mFlutter.mAngle <= 0 && next >= 0) {
        mFlutterAmp = (s16)cM::rndRange<mAng>(ampMin, ampMax);
    }
    mPos.y = mBaseY + mFlutterAmp * mFlutter.sin();
    mFlutter += mAng::fromDegree(speed);
}

// .text 9804
void dInsFldButterfly_c::flutterApproach(f32 rate) {
    const param_c *param = getKindParam(mType);
    f32 r = rate < 0.0f ? 0.0f : (rate > 1.0f ? 1.0f : rate);
    f32 height = getBaseHeight();
    if (height < mBaseY) {
        mBaseY -= 0.12f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 5.0f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
    s16 next = mFlutter.mAngle + mAng::fromDegree(param->mFlutterSpeed);
    if (mFlutter.mAngle > 0 && next <= 0) {
        mFlutterAmp =
            cM::rndRange(param->mFlutterAmpMin, param->mFlutterAmpMax);
    } else if (mFlutter.mAngle <= 0 && next >= 0) {
        mFlutterAmp =
            cM::rndRange(param->mFlutterAmpMin, param->mFlutterAmpMax);
    }
    mFlutterAmp *= r;
    mPos.y = mBaseY + mFlutterAmp * mFlutter.sin();
    mFlutter += mAng::fromDegree(param->mFlutterSpeed);
}

// .text 99BC
mAng dInsFldButterfly_c::getFlyAngle() const {
    s16 angle;
    int range;
    if (mState == STATE_ESCAPE) {
        s16 player = mPlayerCheck.getAngleTo(mPlayerCheck.getNearest()).mAngle +
                     INSECT_FIELD_FLEE_ANGLE;
        s16 npc = player;
        if (mNpcCheck.isNear(mScareRadius) == true) {
            npc = mNpcCheck.getAngleTo().mAngle + INSECT_FIELD_FLEE_ANGLE;
        }
        angle = (player + npc) >> 1;
        range = 0;
    } else {
        angle = mAngle.y.mAngle;
        range = DEG_TO_ANGLE(120);
    }
    return angle + (int)cM::rndCenterF(range);
}

// .text 9AA8
f32 dInsFldButterfly_c::getFlyDist() {
    const param_c *param = getKindParam(mType);
    f32 dist = cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
    if (mState == STATE_ESCAPE) {
        dist = cM::rndRange(param->mEscapeDistMin, param->mEscapeDistMax);
    } else {
        dist = cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
    }
    return dist;
}

// .text 9B18
void dInsFldButterfly_c::getNextTarget(mVec3_c *out) {
    if (dNet::isSessionActive() != true || dNet::isSceneOwner()) {
        mAng angle = getFlyAngle();
        f32 dist = getFlyDist();
        mVec3_c target(0.0f, 0.0f, 0.0f);
        mVec3_c ofs(0.0f, 0.0f, dist);
        ofs.rotY(angle);
        ofs += mPos;
        const param_c *param = getKindParam(mType);
        u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
        mVec3_c from = mPos;
        mVec3_c to = ofs;
        if (!dInsectUtil::getGroundMovePos(&target, from, to, steps, TRUE, TRUE,
                                           -500.0f, 0.0f)) {
            dInsectUtil::getLandPos(&target, mPos, 1, TRUE);
        }
        *out = target;
    }
}

// .text 9C98
void dInsFldButterfly_c::getFlowerPos(mVec3_c *out) {
    mVec3_c ownerOfs;
    mVec3_c peerOfs;
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        mVec3_c pos = mHomePos;
        dItem::Item *item = dInsectUtil::getItem(&pos);
        if (item != NULL) {
            dInsectField::getItemOffset(&ownerOfs, item);
            mHomePos.y = ownerOfs.y + dBGCF::getGroundY(&mHomePos, FALSE);
        }
    } else {
        mFlowerSearch.getPos(out);
        dItem::Item item = mFlowerSearch.getItem();
        dInsectField::getItemOffset(&peerOfs, &item);
        *out += peerOfs;
    }
}

// .text 9D88
f32 dInsFldButterfly_c::getBaseHeight() {
    const param_c *param = getKindParam(mType);
    if (mState == STATE_APPROACH) {
        return mHomePos.y;
    }
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        return param->mHeight + chk.getWaterY();
    }
    return param->mHeight + getGroundY();
}

// .text 9E1C
u16 dInsFldButterfly_c::getSearchDelay() const {
    const param_c *param = getKindParam(mType);
    int min = dInsectUtil::secToFrame(param->mSearchDelayMin);
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    return cM::rndRange<u16>(min, max);
}

// .text 9E70
void dInsFldButterfly_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
            u32 state = data->getReq();
            if (state < STATE_NUM_MAX) {
                mState = state;
            }
        }
        return;
    }

    if (mState == STATE_FLY) {
        if (canSearch() == true && isFlowerFound() == true) {
            mState = STATE_APPROACH;
        }
        if (mScare.isScared() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_APPROACH) {
        if (mApproachTime <= mFrame) {
            if (isFlowerFound() == true) {
                mState = STATE_LAND;
            } else {
                mState = STATE_FLY;
            }
        } else {
            mVec3_c pos = mPos;
            bool taken = isUnitTaken(&pos) == TRUE;
            if (taken) {
                mState = STATE_ESCAPE;
                mScare.setMax();
            }
        }
        if (mScare.isScared() == true) {
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_ESCAPE) {
        if (!mScare.isScared()) {
            mState = STATE_FLY;
        }
    } else if (mState == STATE_LAND) {
        if (mLandTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE;
            } else {
                bool ok = false;
                mVec3_c pos = mPos;
                if (!isUnitTaken(&pos) && isOnFlower() == true) {
                    ok = true;
                }
                if (ok) {
                    mState = STATE_REST;
                } else {
                    mState = STATE_ESCAPE;
                    mScare.setMax();
                }
            }
        }
    } else if (mState == STATE_REST) {
        if (mRestTime <= mFrame) {
            mSearchDelay = getSearchDelay();
            mState = STATE_TAKE_OFF;
        }
        if (mScare.isScared() == true) {
            mSearchDelay = getSearchDelay();
            mState = STATE_ESCAPE;
        }
        if (!isOnFlower()) {
            mSearchDelay = getSearchDelay();
            mState = STATE_ESCAPE;
        }
        if (mFrame % 15 == 0 &&
            dFgobjManager::isSwingPlantAt(mPos.x, mPos.z) == TRUE) {
            mSearchDelay = getSearchDelay();
            mState = STATE_ESCAPE;
        }
    } else if (mState == STATE_TAKE_OFF) {
        if (mTakeOffTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE;
            } else {
                mState = STATE_FLY;
            }
        }
    }

    if (mState != STATE_ESCAPE && mState != STATE_LEAVE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.setMax();
    }
}

// .text A334
void dInsFldButterfly_c::setFlyTarget() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    getNextTarget(&mHomePos);
    mFlyTime = (int)((mHomePos - mPos).xzLen() / param->mSpeed) + 1;
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPosX(mHomePos.x);
            data->setPosZ(mHomePos.z);
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
            data->setReq(mState);
        }
    }
}

// .text A510
void dInsFldButterfly_c::setEscapeTarget() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    getNextTarget(&mHomePos);
    mEscapeTime = (int)((mHomePos - mPos).xzLen() / (2.5f * param->mSpeed)) + 1;
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPosX(mHomePos.x);
            data->setPosZ(mHomePos.z);
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
            data->setReq(mState);
        }
    }
}

// .text A6F8
bool dInsFldButterfly_c::isFlowerFound() {
    return mFlowerSearch.isFound();
}

// .text A700
bool dInsFldButterfly_c::canSearch() {
    return mSearchDelay <= mSearchTimer;
}

// .text A720
bool dInsFldButterfly_c::isOnFlower() const {
    mVec3_c pos = mPos;
    dItem::Item *item = dInsectUtil::getItem(&pos);
    if (item == NULL) {
        return false;
    }
    u16 id = item->mId;
    if (item->isFlower() == true || id == dItem::FG_DANDELION) {
        return true;
    }
    return false;
}

// .text A798
bool dInsFldButterfly_c::isBirdwing() {
    return mType == INSECT_BIRDWING_BUTTERFLY;
}
