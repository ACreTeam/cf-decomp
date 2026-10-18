// dInsFldDragonfly_c: the dragonflies. See
// include/game/game/d_ins_fld_dragonfly.hpp.

const dInsFldDragonfly_c::StateFunc dInsFldDragonfly_c::sExecute[] = {
    &dInsFldDragonfly_c::executeTurn,   &dInsFldDragonfly_c::executeTurnPerch,
    &dInsFldDragonfly_c::executeFly,    &dInsFldDragonfly_c::executeApproach,
    &dInsFldDragonfly_c::executeRest,   &dInsFldDragonfly_c::executeTakeOff,
    &dInsFldDragonfly_c::executeEscape, &dInsFldDragonfly_c::executeEscapeTurn,
    &dInsFldDragonfly_c::executeLeave,
};

const dInsFldDragonfly_c::StateFunc dInsFldDragonfly_c::sInit[] = {
    &dInsFldDragonfly_c::initTurn,   &dInsFldDragonfly_c::initTurnPerch,
    &dInsFldDragonfly_c::initFly,    &dInsFldDragonfly_c::initApproach,
    &dInsFldDragonfly_c::initRest,   &dInsFldDragonfly_c::initTakeOff,
    &dInsFldDragonfly_c::initEscape, &dInsFldDragonfly_c::initEscapeTurn,
    &dInsFldDragonfly_c::onRelease,
};

// .data FA0
dInsFldDragonfly_c::param_c dInsFldDragonfly_c::sParam[PARAM_NUM] = {
    {0.0f,  45.0f, 12.0f,  5.0f,  3.0f,  0.5f,   120.0f, 20.0f, 0.98f, 0.003f,
     150,   0,     120.0f, 80.0f, 0.98f, 0.003f, 150,    0,     5.0f,  5.0f,
     10.0f, 5.0f,  1.8f,   0.04f, 1.0f,  5.0f,   4.0f,   80.0f, 100},
    {0.0f,  45.0f, 12.0f,  5.0f,  3.0f,  0.5f,   200.0f, 50.0f, 0.98f, 0.003f,
     150,   0,     100.0f, 75.0f, 0.98f, 0.003f, 150,    0,     5.0f,  5.0f,
     10.0f, 5.0f,  1.8f,   0.04f, 1.0f,  6.0f,   5.0f,   80.0f, 100},
    {0.0f,  45.0f, 12.0f,  3.5f,   3.0f,  0.5f,   200.0f, 120.0f, 0.97f, 0.003f,
     100,   0,     200.0f, 150.0f, 0.96f, 0.003f, 70,     0,      5.0f,  5.0f,
     10.0f, 5.0f,  2.0f,   0.04f,  1.0f,  8.0f,   5.0f,   120.0f, 100},
    {0.0f,  60.0f, 12.0f,  3.5f,   3.0f,  0.5f,   200.0f, 50.0f,  0.97f, 0.003f,
     100,   0,     200.0f, 150.0f, 0.96f, 0.003f, 70,     0,      20.0f, 10.0f,
     10.0f, 5.0f,  2.0f,   0.04f,  1.0f,  11.0f,  10.0f,  200.0f, 100},
};

// .text 10C4C
bool dInsFldDragonfly_c::isPerchItem(const dItem::Item *item,
                                     const mVec3_c *pos) {
    if ((item->isStone() == true || item->isMoneyRock()) &&
        !dInsectUtil::isExcludedFlower(pos)) {
        return true;
    }
    return false;
}

// .text 10CC8
int dInsFldDragonfly_c::getParamIdx(int type) {
    const int types[] = {
        INSECT_RED_DRAGONFLY,
        INSECT_DARNER_DRAGONFLY,
        INSECT_BANDED_DRAGONFLY,
        INSECT_GIANT_PETALTAIL_DRAGONFLY,
    };
    for (int i = 0; i < PARAM_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text 10D24
const dInsFldDragonfly_c::param_c *dInsFldDragonfly_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text 10D54
dInsFldDragonfly_c::dInsFldDragonfly_c()
    : mState(STATE_TURN), mPrevState(STATE_TURN), mPerchSearch(isPerchItem),
      mStartPos(0.0f, 0.0f, 0.0f), mBaseY(0.0f), mHoverAmp(0.0f),
      mHoverPhase(0), _356(0), mFlyTime(0), mEscapeTime(0), mApproachTime(0),
      mRestTime(0), mTakeOffTime(0), mSearchTimer(0), mSearchDelay(0),
      mTargetAngle(0) {
}

// .text 10E00
dInsFldDragonfly_c::~dInsFldDragonfly_c() {
}

// .text 10E74
BOOL dInsFldDragonfly_c::create(const nw4r::g3d::ResFile &res) {
    mSearchDelay = getSearchDelay();
    dInsFldBase_c::create(res);
    return TRUE;
}

// .text 10EC0
int dInsFldDragonfly_c::execute() {
    const param_c *param = getKindParam(mType);
    mPerchSearch.search(mPos, 128.0f);
    mVec3_c pos = mPos;
    mSiteSearch.search(pos, 128.0f);
    dInsFldBase_c::execute();
    mSearchTimer++;
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    mSearchTimer = mSearchTimer > (u16)max ? max : mSearchTimer;
    return TRUE;
}

// .text 10F78
int dInsFldDragonfly_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text 10F9C
void dInsFldDragonfly_c::onBecomeOwner() {
    const param_c *param = getKindParam(mType);
    if (mPos.y < dBGCF::getGroundY(&mPos, TRUE)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = param->mHeight + dBGCF::getGroundY(&pos, FALSE);
        mPos = pos;
        mOldPos = mPos;
        mVelocity.x = 0.0f;
        mVelocity.z = 0.0f;
    }
}

// .text 110C4
void dInsFldDragonfly_c::setState(int state) {
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

// .text 11254
void dInsFldDragonfly_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x += param->mOfsX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOfsZ;
}

// .text 112C4
void dInsFldDragonfly_c::initState() {
    setState(STATE_TURN);
}

// .text 112CC
void dInsFldDragonfly_c::executeState() {
    getKindParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    checkState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// .text 11340
void dInsFldDragonfly_c::initTurn() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getFlyTarget(&mHomePos);
    mStartPos = mPos;
    mTargetAngle = cLib::targetAngleY(mPos, mHomePos);
    mBaseY = mPos.y;
    mHoverAmp = 0.0f;
    mVelocity.set(0.0f, 0.0f, 0.0f);
}

// .text 113F0
void dInsFldDragonfly_c::initTurnPerch() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getPerchPos(&mHomePos);
    mStartPos = mPos;
    mTargetAngle = cLib::targetAngleY(mPos, mHomePos);
    mBaseY = mPos.y;
    mHoverAmp = 0.0f;
    mVelocity.set(0.0f, 0.0f, 0.0f);
}

// .text 114A0
void dInsFldDragonfly_c::initFly() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mBaseY = mPos.y;
    mFlyTime = param->mFlyTime;
    mFlyTime = (int)(0.8f * mFlyTime) + 1;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    f32 dist = (mHomePos - mStartPos).xzLen();
    mVelocity.set(0.0f, 0.0f,
                  dist / (param->mFlyDecay / (1.0f - param->mFlyDecay)));
    mVelocity.rotY(mAngle.y);
    mVelocity.y = -param->mFlyGravity * (mFlyTime / 2);
    if (getBaseHeight() < mPos.y) {
        mVelocity.y *= 1.5f;
    } else {
        mVelocity.y *= 0.8f;
    }
}

// .text 11634
void dInsFldDragonfly_c::initApproach() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getPerchPos(&mHomePos);
    mStartPos = mPos;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mApproachTime = (int)(mStartPos.distTo(mHomePos) / 0.5f) + 1;
}

// .text 116F0
void dInsFldDragonfly_c::initRest() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 60.0f);
    setAnmVis(mAnmVisWait);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    mRestTime = cM::rndRange<u16>(min, max);
    mBaseY = mPos.y;
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            if (data->getPerch() == PERCH_ITEM) {
                mVec3_c ofs;
                mVec3_c pos = mHomePos;
                dItem::Item *item = dInsectUtil::getItem(&pos);
                dInsectField::getItemOffset(&ofs, item);
                mHomePos.y = ofs.y + dBGCF::getGroundY(&mHomePos, FALSE);
            } else {
                f32 radius = mSiteSearch.getRadius();
                f32 y = dBGCF::getGroundY(&mHomePos, FALSE);
                mHomePos.y = y + radius;
            }
        }
    }
}

// .text 11830
void dInsFldDragonfly_c::initTakeOff() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 60.0f);
    setAnmVis(mAnmVisFly);
    getFlyTarget(&mHomePos);
    mHomePos.y = 10.0f + mPos.y;
    mStartPos = mPos;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mTakeOffTime = (int)(mStartPos.distTo(mHomePos) / 0.5f) + 1;
}

// .text 11904
void dInsFldDragonfly_c::initEscape() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mBaseY = mPos.y;
    mEscapeTime = param->mEscapeTime;
    mEscapeTime = (int)(0.7f * mEscapeTime) + 1;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    f32 dist = (mHomePos - mStartPos).xzLen();
    mVelocity.set(0.0f, 0.0f,
                  dist / (param->mEscapeDecay / (1.0f - param->mEscapeDecay)));
    mVelocity.rotY(mAngle.y);
    mVelocity.y = -param->mEscapeGravity * (mEscapeTime / 2);
    if (getBaseHeight() < mPos.y) {
        mVelocity.y *= 1.5f;
    } else {
        mVelocity.y *= 0.8f;
    }
}

// .text 11A98
void dInsFldDragonfly_c::initEscapeTurn() {
    getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    getFlyTarget(&mHomePos);
    mStartPos = mPos;
    mTargetAngle = cLib::targetAngleY(mStartPos, mHomePos);
    mVelocity.set(0.0f, 0.0f, 0.0f);
}

// .text 11B3C
void dInsFldDragonfly_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
}

// .text 11BC8
void dInsFldDragonfly_c::executeTurn() {
    const param_c *param = getKindParam(mType);
    hover();
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1, step, step);
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11C54
void dInsFldDragonfly_c::executeTurnPerch() {
    const param_c *param = getKindParam(mType);
    hover();
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1, step, step);
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11CE0
void dInsFldDragonfly_c::executeFly() {
    const param_c *param = getKindParam(mType);
    if (mFrame < mFlyTime) {
        mVelocity.x *= param->mFlyDecay;
        mVelocity.z *= param->mFlyDecay;
        mVelocity.y += param->mFlyGravity;
        mPos += mVelocity;
        mBaseY = mPos.y;
    }
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11D8C
void dInsFldDragonfly_c::executeApproach() {
    getKindParam(mType);
    cLib::addCalcPos2(&mPos, mHomePos, 0.1f, 0.8f);
    mBaseY = mPos.y;
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11DF4
void dInsFldDragonfly_c::executeRest() {
    getKindParam(mType);
    if (mRestTime <= mFrame) {
        mSearchDelay = getSearchDelay();
    }
    mBaseY = mPos.y;
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mScare.isScared() == true) {
        mSearchDelay = getSearchDelay();
    }
    mSearchTimer = 0;
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        cLib::addCalcPos2(&mPos, mHomePos, 0.1f, 2.0f);
    }
}

// .text 11EB4
void dInsFldDragonfly_c::executeTakeOff() {
    getKindParam(mType);
    cLib::addCalcPos2(&mPos, mHomePos, 0.05f, 0.5f);
    mBaseY = mPos.y;
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11F1C
void dInsFldDragonfly_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    if (mFrame < mEscapeTime) {
        mVelocity.x *= param->mEscapeDecay;
        mVelocity.z *= param->mEscapeDecay;
        mVelocity.y += param->mEscapeGravity;
        mPos += mVelocity;
        mBaseY = mPos.y;
    }
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 11FC8
void dInsFldDragonfly_c::executeEscapeTurn() {
    const param_c *param = getKindParam(mType);
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 1, step, step);
    registObjc(&mPos, mBaseRadius, TRUE);
}

// .text 12040
void dInsFldDragonfly_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mLeaveSpeed, param->mLeaveAccelY);
}

// .text 12084
void dInsFldDragonfly_c::hover() {
    const param_c *param = getKindParam(mType);
    mHoverAmp += 0.01f;
    mHoverAmp = mHoverAmp > 1.0f ? 1.0f : mHoverAmp;
    s16 step =
        DEG_TO_ANGLE(180) / (u16)dInsectUtil::secToFrame(param->mHoverPeriod);
    mHoverPhase.mAngle += step * 2;
    f32 hover = param->mHoverAmp * mHoverPhase.sin();
    mPos.y = mBaseY + mHoverAmp * hover;
    f32 height = getBaseHeight();
    if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.5f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
}

// .text 121AC
bool dInsFldDragonfly_c::isBig() const {
    if (mType == INSECT_BANDED_DRAGONFLY) {
        return true;
    }
    return mType == INSECT_GIANT_PETALTAIL_DRAGONFLY;
}

// .text 121D0
bool dInsFldDragonfly_c::isPerchFound() {
    bool item = mPerchSearch.isFound();
    bool site = mSiteSearch.isFound();
    if (item == true || site == true) {
        return true;
    }
    return false;
}

// .text 12230
bool dInsFldDragonfly_c::canSearch() {
    return mSearchDelay <= mSearchTimer;
}

// .text 12250
bool dInsFldDragonfly_c::isPerchBelow() {
    if (isPerchFound() == true) {
        f32 y;
        if (mPerchSearch.isFound() == true) {
            mVec3_c pos;
            mPerchSearch.getPos(&pos);
            mVec3_c ofs;
            dItem::Item item = mPerchSearch.getItem();
            dInsectField::getItemOffset(&ofs, &item);
            y = pos.y + ofs.y;
        } else {
            mVec3_c pos = mSiteSearch.getPos();
            pos.y += mSiteSearch.getRadius();
            y = pos.y;
        }
        if (y <= mPos.y) {
            return true;
        }
    }
    return false;
}

// .text 12304
mAng dInsFldDragonfly_c::getFlyAngle() const {
    int angle;
    if (mState == STATE_ESCAPE_TURN) {
        s16 player = mPlayerCheck.getAngleTo(mPlayerCheck.getNearest()).mAngle +
                     INSECT_FIELD_FLEE_ANGLE;
        s16 npc = player;
        if (mNpcCheck.isNear(mScareRadius) == true) {
            npc = mNpcCheck.getAngleTo().mAngle - DEG_TO_ANGLE(180);
        }
        angle = (player + npc) >> 1;
    } else {
        angle = mAngle.y.mAngle;
    }
    if (isBig() == true) {
        s16 offset = cM::rndCenterF(0xE39);
        angle += offset;
    } else {
        s16 offset = cM::rndCenterF(0x2AAB);
        angle += offset;
    }
    return angle;
}

// .text 12408
f32 dInsFldDragonfly_c::getFlyDist() const {
    const param_c *param = getKindParam(mType);
    if (mState == STATE_ESCAPE_TURN) {
        return cM::rndRange(param->mEscapeDistMin, param->mEscapeDistMax);
    }
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// .text 12460
void dInsFldDragonfly_c::getFlyTarget(mVec3_c *out) {
    if (dNet::isSessionActive() != true || dNet::isSceneOwner()) {
        mAng angle = getFlyAngle();
        f32 dist = getFlyDist();
        mVec3_c target(0.0f, 0.0f, 0.0f);
        mVec3_c ofs(0.0f, 0.0f, dist);
        ofs.rotY(angle);
        ofs += mPos;
        const param_c *param = getKindParam(mType);
        u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
        mAng tryAngle = angle;
        int stepValue = DEG_TO_ANGLE(30);
        int &step = stepValue;
        if (cM::rnd() < 0.5f) {
            step = -step;
        }
        mAng turn = (u16)step;
        BOOL found;
        int i = 0;
        do {
            tryAngle += turn;
            ofs.set(0.0f, 0.0f, dist);
            ofs.rotY(tryAngle);
            ofs += mPos;
            mVec3_c from = mPos;
            mVec3_c to = ofs;
            found = dInsectUtil::getGroundMovePos(&target, from, to, steps,
                                                  TRUE, TRUE, -500.0f, -15.0f);
            if ((u32)found == TRUE) {
                break;
            }
            i++;
        } while (i < 5);
        if (!found) {
            target.set(0.0f, 0.0f, param->mFlyDistMin);
            target.rotY(angle.mAngle - DEG_TO_ANGLE(180));
            target += mPos;
            u8 fallbackSteps = (u8)((int)(param->mFlyDistMax / 32.0f) + 1);
            mVec3_c from = mPos;
            mVec3_c to = ofs;
            if (!dInsectUtil::getGroundMovePos(&target, from, to, fallbackSteps,
                                               TRUE, TRUE, -500.0f, -15.0f)) {
                dInsectUtil::getLandPos(&target, mPos, 1, TRUE);
            }
        }
        *out = target;
    }
}

// .text 12758
static inline dItem::Item *getPerchItem(const mVec3_c &pos) {
    return dInsectUtil::getItem(&pos);
}
void dInsFldDragonfly_c::getPerchPos(mVec3_c *out) {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            mVec3_c pos = mHomePos;
            pos.x = data->getPosX() * 4;
            pos.z = data->getPosZ() * 4;
            if (data->getPerch() == PERCH_ITEM) {
                dItem::Item *item = getPerchItem(mVec3_c(mHomePos));
                mVec3_c ofs;
                dInsectField::getItemOffset(&ofs, item);
                pos.y = ofs.y + dBGCF::getGroundY(&pos, FALSE);
            } else {
                f32 radius = mSiteSearch.getRadius();
                f32 y = dBGCF::getGroundY(&pos, FALSE);
                pos.y = y + radius;
            }
            *out = pos;
        }
    } else if (mPerchSearch.isFound() == true) {
        mPerchSearch.getPos(out);
        mVec3_c ofs;
        dItem::Item item = mPerchSearch.getItem();
        dInsectField::getItemOffset(&ofs, &item);
        *out += ofs;
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPerch(PERCH_ITEM);
        }
    } else {
        mVec3_c pos = mSiteSearch.getPos();
        pos.y += mSiteSearch.getRadius();
        *out = pos;
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPerch(PERCH_SITE);
        }
    }
}

// .text 129B0
f32 dInsFldDragonfly_c::getBaseHeight() {
    const param_c *param = getKindParam(mType);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        return param->mHeight + chk.getWaterY();
    }
    return param->mHeight + getGroundY();
}

// .text 12A30
void dInsFldDragonfly_c::checkState() {
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

    if (mState == STATE_TURN) {
        if (mAngle.y.mAngle == mTargetAngle) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_FLY;
            }
        }
    } else if (mState == STATE_TURN_PERCH) {
        if (mAngle.y.mAngle == mTargetAngle) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else if (isPerchFound() == true) {
                mState = STATE_APPROACH;
            } else {
                mState = STATE_ESCAPE_TURN;
            }
        }
    } else if (mState == STATE_FLY) {
        if (mFlyTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else if (canSearch() == true && isPerchFound() == true &&
                       isPerchBelow() == true) {
                mState = STATE_TURN_PERCH;
            } else {
                mState = STATE_TURN;
            }
        }
    } else if (mState == STATE_APPROACH) {
        if (mApproachTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                bool free = isCurrentUnitFree();
                if (free) {
                    mState = STATE_REST;
                } else {
                    mState = STATE_ESCAPE;
                    mScare.setMax();
                }
            }
            dItem::Item item = getItem();
            if (!item.isStone() && !item.isMoneyRock()) {
                mVec3_c site = mSiteSearch.getPos();
                int siteX = worldToUnit(site.x);
                int siteZ = worldToUnit(site.z);
                int posX = worldToUnit(mPos.x);
                int posZ = worldToUnit(mPos.z);
                if (siteX != posX || siteZ != posZ) {
                    mState = STATE_ESCAPE;
                    mScare.setMax();
                }
            }
        } else {
            bool taken = isCurrentUnitTaken();
            if (taken) {
                mState = STATE_ESCAPE;
                mScare.setMax();
            }
        }
    } else if (mState == STATE_REST) {
        if (mRestTime <= mFrame) {
            mState = STATE_TAKE_OFF;
        } else if (mScare.isScared() == true) {
            mState = STATE_ESCAPE_TURN;
        }
    } else if (mState == STATE_TAKE_OFF) {
        if (mTakeOffTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_TURN;
            }
        }
    } else if (mState == STATE_ESCAPE) {
        if (mEscapeTime <= mFrame) {
            if (mScare.isScared() == true) {
                mState = STATE_ESCAPE_TURN;
            } else {
                mState = STATE_TURN;
            }
        }
    } else if (mState == STATE_ESCAPE_TURN) {
        if (mAngle.y.mAngle == mTargetAngle) {
            mState = STATE_ESCAPE;
        }
    }

    if (mState != STATE_ESCAPE && mState != STATE_ESCAPE_TURN &&
        mState != STATE_LEAVE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.setMax();
    }
}

// .text 12FFC
u16 dInsFldDragonfly_c::getSearchDelay() const {
    const param_c *param = getKindParam(mType);
    int min = dInsectUtil::secToFrame(param->mSearchDelayMin);
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    return cM::rndRange<u16>(min, max);
}
