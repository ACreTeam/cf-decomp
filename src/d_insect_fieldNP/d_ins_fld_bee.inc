// dInsFldBee_c: the honeybee and the fly. See
// include/game/game/d_ins_fld_bee.hpp.

const dInsFldBee_c::StateFunc dInsFldBee_c::sExecute[] = {
    &dInsFldBee_c::executeFly,   &dInsFldBee_c::executeApproach,
    &dInsFldBee_c::executeLand,  &dInsFldBee_c::executeRest,
    &dInsFldBee_c::executeLeave,
};

const dInsFldBee_c::StateFunc dInsFldBee_c::sInit[] = {
    &dInsFldBee_c::initFly,   &dInsFldBee_c::initApproach,
    &dInsFldBee_c::initLand,  &dInsFldBee_c::initRest,
    &dInsFldBee_c::onRelease,
};

// .data 11B8
dInsFldBee_c::param_c dInsFldBee_c::sParam[TYPE_NUM] = {
    {0.0f,  30.0f, 0.0f, 20.0f, 10.0f, 0.2f,  0.03f, 0.9f,   0.03f,
     2.0f,  3.0f,  2.0f, 1.0f,  1.0f,  30.0f, 0.45f, 0.045f, 0.9f,
     0.05f, 1.5f,  1.5f, 1.0f,  30.0f, 0.2f,  0.02f, 0.9f,   0.08f,
     5.0f,  3.0f,  2.5f, 2.5f,  0.01f, 0.02f, 10.0f, 3.0f,   1.0f,
     0.6f,  0.03f, 1.0f, 3.0f,  3.0f,  50.0f, 200},
    {0.0f,  45.0f, 0.0f, 20.0f, 10.0f, 3.0f, 0.3f, 0.9f, 0.3f,   12.0f, 1.0f,
     0.5f,  1.0f,  1.0f, 30.0f, 5.0f,  0.5f, 0.9f, 0.5f, 1.0f,   1.5f,  1.0f,
     30.0f, 2.0f,  0.2f, 0.9f,  0.3f,  5.0f, 2.0f, 0.0f, 0.0f,   0.0f,  0.0f,
     15.0f, 10.0f, 1.0f, 1.0f,  0.03f, 1.0f, 3.0f, 2.0f, 100.0f, 100},
};

// .text 146DC
bool dInsFldBee_c::isBeeFood(const dItem::Item *item, const mVec3_c *pos) {
    if (item->isFlower() == true && !dInsectUtil::isExcludedFlower(pos)) {
        return true;
    }
    return false;
}

// .text 1472C
bool dInsFldBee_c::isFlyFood(const dItem::Item *item, const mVec3_c *pos) {
    if ((item->isRafflesia() == true || item->isDust() == true) &&
        !dInsectUtil::isExcludedFlower(pos)) {
        return true;
    }
    if (dItem::Item(dItem::ITEM_IDX_SPOILED_TURNIPS) == *item) {
        return true;
    }
    return false;
}

// .text 147D0
int dInsFldBee_c::getParamIdx(int type) {
    const int types[] = {INSECT_HONEYBEE, INSECT_FLY};
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text 14804
const dInsFldBee_c::param_c *dInsFldBee_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text 14834
dInsFldBee_c::dInsFldBee_c()
    : mState(STATE_FLY), mBaseY(0.0f), mStartPos(0.0f, 0.0f, 0.0f), mFlutter(0),
      mFlutterAmp(0.0f), mSearchDelay(0), mSearchTimer(0), mFlyTime(0),
      mApproachTime(0), mLandTime(0), mRestTime(0), mPitchPhase(0), mPitch(0),
      mFlyCenter(0.0f, 0.0f, 0.0f) {
}

// .text 148E0
dInsFldBee_c::~dInsFldBee_c() {
}

// .text 14948
BOOL dInsFldBee_c::create(const nw4r::g3d::ResFile &res) {
    const param_c *param = getKindParam(mType);
    dInsFldBase_c::create(res);
    if (isFly() == true) {
        mFoodSearch.mCheck = isFlyFood;
        mFoodSearch.search(mPos, 96.0f);
    } else {
        mFoodSearch.mCheck = isBeeFood;
    }
    mBaseY = mPos.y;
    mSearchDelay = dInsectUtil::secToFrame(param->mSearchDelayMax);
    mVec3_c front(0.0f, 0.0f, 1.0f);
    front.rotY(mAngle.y);
    mAxis.setFront(front);
    return TRUE;
}

// .text 14A34
int dInsFldBee_c::execute() {
    const param_c *param = getKindParam(mType);
    if (isFly() != true) {
        mFoodSearch.search(mPos, 48.0f);
    }
    mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS;
    dInsFldBase_c::execute();
    mSearchTimer++;
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    mSearchTimer = mSearchTimer > (u16)max ? max : mSearchTimer;
    return TRUE;
}

// .text 14AD4
int dInsFldBee_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text 14AF8
void dInsFldBee_c::calcMtx() {
    setMtxFromAxis();
}

// .text 14AFC
void dInsFldBee_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// .text 14B34
void dInsFldBee_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x += param->mOfsX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOfsZ;
    mFlyCenter = mInitPos;
}

// .text 14BBC
void dInsFldBee_c::initState() {
    setState(STATE_FLY);
}

// .text 14BC4
void dInsFldBee_c::executeState() {
    getKindParam(mType);
    (this->*sExecute[mState])();
    if (mPlayerCheck.isDisturbing(mScareRadius) == true &&
        mState != STATE_LEAVE) {
        mScare.setMax();
    }
}

// .text 14C94
void dInsFldBee_c::initFly() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmFrame(0.0f);
    getFlyTarget(&mHomePos);
    f32 speed;
    if (mScare.isScared() == true) {
        setAnmRate(param->mEscapeAnmRate);
        speed = param->mEscapeSpeed;
    } else {
        setAnmRate(param->mFlyAnmRate);
        speed = param->mSpeed;
    }
    mFlyTime = (int)((mHomePos - mPos).xzLen() / speed) + 1;
}

// .text 14D9C
void dInsFldBee_c::initApproach() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(0.0f);
    if (isFly() == true) {
        getFoodPosFly(&mHomePos);
    } else {
        getFoodPos(&mHomePos);
    }
    mApproachTime =
        (int)((mHomePos - mPos).xzLen() / param->mApproachSpeed) + 1;
}

// .text 14E90
void dInsFldBee_c::initLand() {
    const param_c *param = getKindParam(mType);
    int max = dInsectUtil::secToFrame(param->mSearchDelayMax);
    int min = dInsectUtil::secToFrame(param->mSearchDelayMin);
    mSearchDelay = cM::rndRange<u16>(min, max);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(2.0f * param->mFlyAnmRate);
    setAnmFrame(0.0f);
    if (isFly() == true) {
        getFoodPosFly(&mHomePos);
    } else {
        getFoodPos(&mHomePos);
    }
    mStartPos = mPos;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mLandTime = (int)(mStartPos.distTo(mHomePos) / param->mApproachSpeed) + 60;
}

// .text 14FC4
void dInsFldBee_c::initRest() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mRestAnmRate);
    setAnmFrame(0.0f);
    getFlyTarget(&mHomePos);
    int max = dInsectUtil::secToFrame(param->mRestTimeMax);
    int min = dInsectUtil::secToFrame(param->mRestTimeMin);
    mRestTime = cM::rndRange<u16>(min, max);
    mVec3_c front(0.0f, 0.0f, 1.0f);
    front.rotY(mAngle.y);
    mAxis.setFront(front);
}

// .text 150B0
void dInsFldBee_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
    if (isFly() == true) {
        mSound.startSound(SE_mushi_fly_out_small);
    }
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// .text 15174
void dInsFldBee_c::executeFly() {
    const param_c *param = getKindParam(mType);
    if (isFly() == true) {
        mSound.holdSound(SE_hae);
    } else {
        mSound.holdSound(SE_mitsubachi_fly);
    }
    f32 accel;
    f32 drag;
    f32 turn;
    if (mScare.isScared() == true) {
        accel = param->mEscapeAccel;
        drag = param->mEscapeDrag;
        turn = param->mEscapeTurn;
    } else {
        accel = param->mFlyAccel;
        drag = param->mFlyDrag;
        turn = param->mFlyTurn;
    }
    mPitchPhase += mAng::fromDegree(param->mPitchSpeed);
    mPitch = mPitchPhase.sin() * mAng::fromDegree(param->mPitchAmp);
    flyPitch(accel, drag, turn, mPitch);
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mFlyTime <= mFrame) {
        if (canSearch() == true && isFoodAway() == true && !mScare.isScared()) {
            setState(STATE_APPROACH);
        } else {
            setState(STATE_FLY);
        }
    }
}

// .text 15364
void dInsFldBee_c::executeApproach() {
    const param_c *param = getKindParam(mType);
    if (isFly() == true) {
        mSound.holdSound(SE_hae);
    } else {
        mSound.holdSound(SE_mitsubachi_fly);
    }
    if (mFrame < mApproachTime) {
        sLib::addCalcAngle(&mPitch.mAngle, 0, mApproachTime / 2,
                           DEG_TO_ANGLE(90));
        flyPitch(param->mApproachAccel, param->mApproachDrag,
                 param->mApproachTurn, mPitch);
        flutterApproach(1.0f - (f32)mFrame / (f32)mApproachTime);
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mApproachTime <= mFrame) {
        if (mFoodSearch.isFound() == true && !mScare.isScared()) {
            setState(STATE_LAND);
        } else {
            setState(STATE_FLY);
        }
    } else {
        mVec3_c pos = mPos;
        bool taken = isUnitTaken(&pos) == TRUE;
        if (taken) {
            setState(STATE_FLY);
        }
    }
}

// .text 1550C
void dInsFldBee_c::executeLand() {
    const param_c *param = getKindParam(mType);
    if (isFly() == true) {
        mSound.holdSound(SE_hae);
    } else {
        mSound.holdSound(SE_mitsubachi_fly);
    }
    cLib::addCalcPos2(&mPos, mHomePos, 0.1f, param->mSpeed);
    mBaseY = mPos.y;
    registObjc(&mPos, mBaseRadius, TRUE);
    if (isFly() == true) {
        mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                   dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS |
                   dBGCF::CHECK_NO_COLUMNS;
    }
    if (mLandTime <= mFrame) {
        if (mScare.isScared() == true) {
            setState(STATE_FLY);
        } else {
            mVec3_c pos = mPos;
            bool free = !isUnitTaken(&pos);
            if (free) {
                setState(STATE_REST);
            } else {
                mScare.setMax();
                setState(STATE_FLY);
            }
        }
    }
}

// .text 1569C
void dInsFldBee_c::executeRest() {
    getKindParam(mType);
    if (!isFly()) {
        crawl();
        f32 dist = (mHomePos - mPos).xzLen();
        if (dist < 0.5f) {
            setState(STATE_REST);
        }
    } else {
        mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                   dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS |
                   dBGCF::CHECK_NO_COLUMNS;
    }
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mRestTime <= mFrame) {
        setState(STATE_FLY);
    } else if (mScare.isScared() == true) {
        setState(STATE_FLY);
    } else if (!isOnFood()) {
        setState(STATE_FLY);
    }
    if (mFrame % 15 == 0 &&
        dFgobjManager::isSwingPlantAt(mPos.x, mPos.z) == TRUE) {
        setState(STATE_FLY);
    }
    mSearchTimer = 0;
}

// .text 15818
void dInsFldBee_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mLeaveSpeed, param->mLeaveAccelY);
}

// .text 1585C
void dInsFldBee_c::flyPitch(f32 accel, f32 drag, f32 turn, mAng pitch) {
    fly(0.0f, accel, drag, turn);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    up.rotX(pitch);
    up.rotY(mAxis.getAngleY());
    mAxis.setUp(up);
}

// .text 15908
void dInsFldBee_c::flutter() {
    f32 height = getBaseHeight();
    if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
    const param_c *param = getKindParam(mType);
    s16 next = mFlutter.mAngle + mAng::fromDegree(param->mFlutterSpeed);
    if (mFlutter.mAngle > 0 && next <= 0) {
        mFlutterAmp =
            cM::rndRange(param->mFlutterAmpMin, param->mFlutterAmpMax);
    } else if (mFlutter.mAngle <= 0 && next >= 0) {
        mFlutterAmp =
            cM::rndRange(param->mFlutterAmpMin, param->mFlutterAmpMax);
    }
    mPos.y = mBaseY + mFlutterAmp * mFlutter.sin();
    mFlutter += mAng::fromDegree(param->mFlutterSpeed);
}

// .text 15A64
void dInsFldBee_c::flutterApproach(f32 rate) {
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

// .text 15C1C
void dInsFldBee_c::crawl() {
    const param_c *param = getKindParam(mType);
    f32 speed = param->mRestSpeed;
    f32 turn = param->mRestTurn;
    dInsectUtil::flyTo(&mPos, &mAxis, mHomePos, mVec3_c(0.0f, 1.0f, 0.0f),
                       speed, turn);
}

// .text 15C84
bool dInsFldBee_c::canSearch() {
    return mSearchDelay <= mSearchTimer;
}

// .text 15CA4
void dInsFldBee_c::getFoodPos(mVec3_c *out) {
    mFoodSearch.getPos(out);
    mVec3_c ofs;
    dItem::Item item = mFoodSearch.getItem();
    dInsectField::getItemOffset(&ofs, &item);
    out->x += ofs.x;
    out->z += ofs.z;
    out->y = ofs.y + dBGCF::getGroundY(out, FALSE);
}

// .text 15D30
mAng dInsFldBee_c::getFlyAngle() const {
    s16 angle = mAngle.y.mAngle;
    int range = DEG_TO_ANGLE(120);
    return angle + (int)cM::rndCenterF(range);
}

// .text 15D94
f32 dInsFldBee_c::getFlyDist() const {
    const param_c *param = getKindParam(mType);
    if (mState == STATE_REST) {
        return cM::rndRange(param->mRestDistMin, param->mRestDistMax);
    }
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// .text 15DEC
void dInsFldBee_c::getFlyTarget(mVec3_c *out) {
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
    if (isFly() == true || mState == STATE_REST) {
        getAreaTarget(&ofs);
        target = ofs;
    }
    *out = target;
}

// .text 15F90
f32 dInsFldBee_c::getBaseHeight() {
    const param_c *param = getKindParam(mType);
    if (mState == STATE_APPROACH) {
        return mHomePos.y;
    }
    f32 height = param->mHeight;
    if (mScare.isScared() == true) {
        height *= param->mEscapeHeight;
    }
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != 0) {
        return height + chk.getWaterY();
    }
    return height + getGroundY();
}

// .text 16048
bool dInsFldBee_c::isFly() {
    return mType == INSECT_FLY;
}

// .text 1605C
void dInsFldBee_c::getFoodPosFly(mVec3_c *out) {
    mFoodSearch.getPos(out);
    mVec3_c ofs;
    dItem::Item item = mFoodSearch.getItem();
    dInsectField::getItemOffset(&ofs, &item);
    *out += ofs;
}

// .text 160E0
bool dInsFldBee_c::isOnFood() {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, mPos);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return false;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return false;
    }
    if (isFly() == true) {
        if (isFlyFood(item, &mInitPos) == true) {
            return true;
        }
    } else if (isBeeFood(item, &mInitPos) == true) {
        return true;
    }
    return false;
}

// .text 161D0
void dInsFldBee_c::getAreaTarget(mVec3_c *out) {
    mVec3_c max;
    mVec3_c min;
    if (isFly() == true) {
        max.x = mFlyCenter.x + 32.0f;
        max.z = mFlyCenter.z + 32.0f;
        min.x = mFlyCenter.x - 32.0f;
        min.z = mFlyCenter.z - 32.0f;
    } else {
        max.x = mFlyCenter.x + 1.6f;
        max.z = mFlyCenter.z + 1.6f;
        min.x = mFlyCenter.x - 1.6f;
        min.z = mFlyCenter.z - 1.6f;
    }
    out->x = cM::rndRange(min.x, max.x);
    out->z = cM::rndRange(min.z, max.z);
}

// .text 16294
bool dInsFldBee_c::isFoodAway() {
    if (isFly() == true) {
        mFoodSearch.search(mPos, 96.0f);
    }
    if (mFoodSearch.isFound() == true) {
        mVec3_c pos;
        mFoodSearch.getPos(&pos);
        f32 dist = (mPos - pos).xzLen();
        if (32.0f <= dist) {
            return true;
        }
    }
    return false;
}
