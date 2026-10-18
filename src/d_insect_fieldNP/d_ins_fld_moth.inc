// dInsFldMoth_c: the moth. See include/game/game/d_ins_fld_moth.hpp.

const dInsFldMoth_c::StateFunc dInsFldMoth_c::sExecute[] = {
    &dInsFldMoth_c::executeFly,    &dInsFldMoth_c::executeGlide,
    &dInsFldMoth_c::executeEscape, &dInsFldMoth_c::executeAway,
    &dInsFldMoth_c::executeLeave,
};

const dInsFldMoth_c::StateFunc dInsFldMoth_c::sInit[] = {
    &dInsFldMoth_c::initFly,    &dInsFldMoth_c::initGlide,
    &dInsFldMoth_c::initEscape, &dInsFldMoth_c::initAway,
    &dInsFldMoth_c::onRelease,
};

// .data 1150
dInsFldMoth_c::param_c dInsFldMoth_c::sParam[TYPE_NUM] = {
    {0.0f,   40.0f, 0.0f,  0.5f, 0.05f, 0.9f, 0.07f, 1.0f, 0.8f,
     0.08f,  0.9f,  0.05f, 1.2f, 10.0f, 5.0f, 6.6f,  7.0f, 5.0f,
     128.0f, 0.7f,  0.04f, 1.0f, 6.5f,  5.0f, 50.0f, 400},
};

// .bss C
int dInsectField::sLightHouseFound;

// .bss 10
int dInsectField::sLightHouse;

// .text 13050
int dInsFldMoth_c::getParamIdx(int type) {
    const int types[] = {INSECT_MOTH};
    for (int i = 0; i < TYPE_NUM; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// .text 13070
const dInsFldMoth_c::param_c *dInsFldMoth_c::getKindParam(int type) {
    return &sParam[getParamIdx(type)];
}

// .text 130A0
dInsFldMoth_c::dInsFldMoth_c()
    : mState(STATE_FLY), mAtHouse(0), mHouse(-1), mLight(LIGHT_HOUSE),
      mBaseY(0.0f), mFlutterAmp(0.0f), mFlutter(0), mFlyTime(0), mEscapeTime(0),
      mSway(0) {
}

// .text 13114
dInsFldMoth_c::~dInsFldMoth_c() {
}

// .text 1316C
void dInsFldMoth_c::calcMtx() {
    const param_c *param = getKindParam(mType);
    mVec3_c pos = mPos;
    mVec3_c side = mAxis.getSide();
    mSway += mAng::fromDegree(param->mFlutterSpeed) / 4;
    pos += side * mAng(mSway).sin() * 5.0f;
    mMtx_c mtx;
    mMtx_c rot;
    mMtx_c tilt;
    nw4r::math::MTX34 axisMtx;
    dActor_c::makeMtx(&mtx, &pos, 0);
    dInsectUtil::getMtx(axisMtx.mtx, mAxis);
    rot = axisMtx;
    mtx.concat(rot);
    tilt.XrotS(-0xFA5); // ~22 degrees
    mtx.concat(tilt);
    mMdl.setLocalMtx(&mtx);
}

// .text 1332C
BOOL dInsFldMoth_c::create(const nw4r::g3d::ResFile &res) {
    if (dInsectField::sLightHouseFound == TRUE) {
        mAtHouse = TRUE;
        mHouse = dInsectField::sLightHouse;
        dInsectField::sLightHouseFound = FALSE;
    }
    dInsFldBase_c::create(res);
    mBaseY = mPos.y;
    mLight = getLight();
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    return TRUE;
}

// .text 133BC
int dInsFldMoth_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// .text 133E0
void dInsFldMoth_c::setState(int state) {
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

// .text 13570
void dInsFldMoth_c::setGroundPos() {
    const param_c *param = getKindParam(mType);
    mPos.x += param->mOfsX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOfsZ;
}

// .text 135E0
void dInsFldMoth_c::initState() {
    setState(STATE_FLY);
}

// .text 135E8
void dInsFldMoth_c::executeState() {
    getKindParam(mType);
    mBgCheck = TRUE;
    (this->*sExecute[mState])();
    if (mState != STATE_ESCAPE && mState != STATE_LEAVE &&
        mPlayerCheck.isDisturbing(mScareRadius) == true) {
        mScare.mValue = (int)mScare.mLimit * 3;
    }
    if (isLightOff() == true) {
        if (mState != STATE_ESCAPE && mState != STATE_LEAVE) {
            mScare.mValue = (int)mScare.mLimit * 3;
            setState(STATE_ESCAPE);
        }
    } else if (isTooFar() == true) {
        if (mState != STATE_AWAY && mState != STATE_LEAVE) {
            setState(STATE_AWAY);
        }
    }
}

// .text 13744
void dInsFldMoth_c::initFly() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
    setFlyTarget();
}

// .text 137E4
void dInsFldMoth_c::initGlide() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mFlyAnmRate);
    setAnmFrame(frame);
}

// .text 1387C
void dInsFldMoth_c::initEscape() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.2f * param->mFlyAnmRate);
    setAnmFrame(0.0f);
    setEscapeTarget();
}

// .text 13914
void dInsFldMoth_c::initAway() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mAngle.y = mAxis.getAngleY();
}

// .text 139B8
void dInsFldMoth_c::onRelease() {
    const param_c *param = getKindParam(mType);
    mState = STATE_LEAVE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mLeaveAnmRate);
    setAnmFrame(0.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// .text 13A70
void dInsFldMoth_c::executeFly() {
    const param_c *param = getKindParam(mType);
    flyAxis(0.0f, param->mFlyAccel, param->mFlyDrag, param->mFlyTurn);
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mFlyTime <= mFrame) {
        setFlyTarget();
    }
    if (mScare.isScared() == true) {
        setState(STATE_ESCAPE);
    }
}

// .text 13B10
void dInsFldMoth_c::executeGlide() {
    getKindParam(mType);
    mVelocity *= 0.95f;
    mPos += mVelocity;
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mFrame >= GLIDE_TIME) {
        setState(STATE_FLY);
    }
    if (mScare.isScared() == true) {
        setState(STATE_ESCAPE);
    }
}

// .text 13BE0
void dInsFldMoth_c::executeEscape() {
    const param_c *param = getKindParam(mType);
    flyAxis(0.0f, param->mEscapeAccel, param->mEscapeDrag, param->mEscapeTurn);
    flutter();
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mEscapeTime <= mFrame) {
        setEscapeTarget();
    }
    if (!mScare.isScared()) {
        setState(STATE_FLY);
    }
}

// .text 13C80
void dInsFldMoth_c::executeAway() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mLeaveSpeed, param->mLeaveAccelY);
}

// .text 13CC4
void dInsFldMoth_c::executeLeave() {
    const param_c *param = getKindParam(mType);
    moveForward(param->mLeaveSpeed, param->mLeaveAccelY);
}

// .text 13D08
void dInsFldMoth_c::flutter() {
    const param_c *param = getKindParam(mType);
    f32 height = getBaseHeight();
    if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.05f;
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
    mPos.y = mBaseY + mFlutterAmp * mFlutter.sin();
    mFlutter += mAng::fromDegree(param->mFlutterSpeed);
}

// .text 13E68
mAng dInsFldMoth_c::getFlyAngle() const {
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

// .text 13F54
f32 dInsFldMoth_c::getFlyDist() const {
    const param_c *param = getKindParam(mType);
    f32 dist = cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
    if (mState == STATE_ESCAPE) {
        dist *= 2.0f;
    }
    return dist;
}

// .text 13FA8
void dInsFldMoth_c::getFlyTarget(mVec3_c *out) {
    const param_c *param = getKindParam(mType);
    mAng angle = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c target(0.0f, 0.0f, 0.0f);
    mVec3_c ofs(0.0f, 0.0f, dist);
    ofs.rotY(angle);
    ofs += mPos;
    u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = ofs;
    if (!dInsectUtil::getGroundMovePos(&target, from, to, steps, TRUE, TRUE,
                                       -500.0f, 0.0f)) {
        dInsectUtil::getLandPos(&target, mPos, 1, TRUE);
    }
    if (mState != STATE_ESCAPE) {
        getHomeTarget(&ofs);
        target = ofs;
    }
    *out = target;
}

// .text 14128
f32 dInsFldMoth_c::getBaseHeight() {
    const param_c *param = getKindParam(mType);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != 0) {
        return param->mHeight + chk.getWaterY();
    }
    return param->mHeight + getGroundY();
}

// .text 141A8
bool dInsFldMoth_c::isLightOff() const {
    if (mHouse == -1) {
        return false;
    }
    for (u32 i = 0; i < ANIMAL_NUM; i++) {
        if (mHouse == i) {
            int x = 0;
            int z = 0;
            if (dSaveBuildingList_c::get()->getNpcHousePos(&x, &z, i)) {
                daSTR_c *house = dSTR::getAcList()->searchBuilding(
                    dSaveBuildingList_c::getNpcHouseId(i), TRUE);
                if (house != NULL && house->isWindowLit()) {
                    return false;
                }
            }
        }
    }
    return true;
}

// .text 14288
void dInsFldMoth_c::setFlyTarget() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    getFlyTarget(&mHomePos);
    mFlyTime = (int)((mHomePos - mPos).xzLen() / param->mSpeed) + 1;
}

// .text 1431C
void dInsFldMoth_c::setEscapeTarget() {
    const param_c *param = getKindParam(mType);
    mFrame = 0;
    getFlyTarget(&mHomePos);
    mEscapeTime = (int)((mHomePos - mPos).xzLen() / (2.5f * param->mSpeed)) + 1;
}

// .text 143BC
bool dInsFldMoth_c::isTooFar() {
    const param_c *param = getKindParam(mType);
    return param->mRange <= (mInitPos - mPos).xzLen();
}

// .text 14434
mVec3_c dInsFldMoth_c::getTownHallPos() const {
    int x = 0;
    int z = 0;
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    dItem::Item building((u16)BUILDING_TOWN_HALL);
    if (dSaveBuildingList_c::get()->getPos(&x, &z, building, 1)) {
        dFdBase_c::getUnitCenterPos(pos, x, z);
    }
    return pos;
}

// .text 144B8
mVec3_c dInsFldMoth_c::getGatePos() const {
    int x = 0;
    int z = 0;
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    dItem::Item building((u16)BUILDING_GATE);
    if (dSaveBuildingList_c::get()->getPos(&x, &z, building, 1)) {
        dFdBase_c::getUnitCenterPos(pos, x, z);
    }
    return pos;
}

// .text 1453C
int dInsFldMoth_c::getLight() const {
    if (mHouse >= 0) {
        return LIGHT_HOUSE;
    }
    mVec3_c townHall = getTownHallPos();
    mVec3_c gate = getGatePos();
    f32 townHallDist = (townHall - mPos).xzLen();
    f32 gateDist = (gate - mPos).xzLen();
    if (townHallDist < gateDist) {
        return LIGHT_TOWN_HALL;
    } else {
        return LIGHT_GATE;
    }
}

// .text 14608
void dInsFldMoth_c::flyAxis(f32 lift, f32 accel, f32 drag, f32 turn) {
    fly(lift, accel, drag, turn);
    mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
    mAngle.y = mAxis.getAngleY();
}

// .text 1466C
void dInsFldMoth_c::getHomeTarget(mVec3_c *out) {
    mVec3_c max;
    mVec3_c min;
    max.x = mInitPos.x + 16.0f;
    max.z = mInitPos.z + 16.0f;
    min.x = mInitPos.x - 16.0f;
    min.z = mInitPos.z - 16.0f;
    out->x = cM::rndRange(min.x, max.x);
    out->z = cM::rndRange(min.z, max.z);
}
