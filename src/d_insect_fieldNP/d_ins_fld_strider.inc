// dInsFldStrider_c: the pond skater. See
// include/game/game/d_ins_fld_strider.hpp.

const dInsFldStrider_c::StateFunc dInsFldStrider_c::sExecute[] = {
    &dInsFldStrider_c::executeWait, &dInsFldStrider_c::executeMove,
    &dInsFldStrider_c::executeRelease};

const dInsFldStrider_c::StateFunc dInsFldStrider_c::sInit[] = {
    &dInsFldStrider_c::initWait, &dInsFldStrider_c::initMove,
    &dInsFldStrider_c::onRelease};

const int dInsFldStrider_c::sTypes[] = {INSECT_POND_SKATER};

dInsFldStrider_c::param_c dInsFldStrider_c::sParam[] = {
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 120.0f, 40.0f, 0.95f, 0.5f, 0.02f, 1.0f,
     0.0f, 6.0f},
};

// 0x183B4
const dInsFldStrider_c::param_c *dInsFldStrider_c::getStriderParam(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return &sParam[i];
        }
    }
    return &sParam[0];
}

// 0x183E8
dInsFldStrider_c::dInsFldStrider_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mStartPos(0.0f, 0.0f, 0.0f),
      mWaitFrames(0), mMoveFrames(0) {
}

// 0x1844C
dInsFldStrider_c::~dInsFldStrider_c() {
}

// 0x184A4
int dInsFldStrider_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x184C8
void dInsFldStrider_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == 1) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            data->setPosX(mHomePos.x);
            data->setPosZ(mHomePos.z);
            data->setReq(mState);
        }
    }
}

// 0x185F8
void dInsFldStrider_c::setGroundPos() {
    const param_c *param = getStriderParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getWaterY();
    mPos.z += param->mOffsetZ;
    mShadowSize = 0.0f;
}

// 0x18674
void dInsFldStrider_c::initState() {
    setState(STATE_WAIT);
}

// 0x1867C
void dInsFldStrider_c::executeState() {
    getStriderParam(mType);
    mAcch.mHitFlags |= dBGCF::HIT_UNDER_WATER;
    mPrevState = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// 0x186FC
void dInsFldStrider_c::initWait() {
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    mWaitFrames = cM::rndRange<int>(0, 180);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
}

// 0x18778
void dInsFldStrider_c::initMove() {
    const param_c *param = getStriderParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    getMoveTarget(&mHomePos);
    mStartPos = mPos;
    mMoveFrames = MOVE_TIME;
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mVec3_c diff = mHomePos - mStartPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    f32 speed = dist / (param->mMoveDecay / (1.0f - param->mMoveDecay));
    mAng ang = cLib::targetAngleY(mStartPos, mHomePos);
    mAngle.y = ang;
    mVelocity.set(0.0f, 0.0f, speed);
    mVelocity.rotY(ang);
}

// 0x188B0
void dInsFldStrider_c::onRelease() {
    const param_c *param = getStriderParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mShadowSize = 4.0f;
}

// 0x18948
void dInsFldStrider_c::executeWait() {
    registObjc(&mPos, mBaseRadius, TRUE);
}

// 0x18958
void dInsFldStrider_c::executeMove() {
    const param_c *param = getStriderParam(mType);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (GLIDE_START_FRAME <= mAnmChr.getFrame()) {
        if (mAnmChr.getFrame() == GLIDE_START_FRAME) {
            dEffect::create("af_ins_amenbo_hamon", &mPos, &mAngle, NULL);
        }
        mVelocity *= param->mMoveDecay;
        mPos += mVelocity;
        if (MOVE_END_FRAME <= mAnmChr.getFrame()) {
            setAnmFrame(MOVE_END_FRAME);
            setAnmRate(0.0f);
        }
    }
    mBgRadius = 32.0f;
}

// 0x18A80
void dInsFldStrider_c::executeRelease() {
    const param_c *param = getStriderParam(mType);
    moveForward(param->mReleaseSpeed, param->mReleaseAccelY);
}

// 0x18AC4
mAng dInsFldStrider_c::getMoveAngle() {
    return mAngle.y.mAngle + (int)cM::rndCenterF(10923.0f);
}

// 0x18B10
f32 dInsFldStrider_c::getMoveDist() const {
    const param_c *param = getStriderParam(mType);
    return cM::rndRange(param->mMoveDistMin, param->mMoveDistMax);
}

// 0x18B40
void dInsFldStrider_c::getMoveTarget(mVec3_c *out) {
    mAng ang = getMoveAngle();
    f32 dist = getMoveDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    mVec3_c from = mPos;
    mVec3_c to = target;
    u8 steps = (int)(dist / 32.0f) + 1;
    if (!dInsectUtil::getWaterMovePos(&pos, from, to, steps)) {
        dInsectUtil::getWaterPos(&pos, mPos);
    }
    if (isBlockedWater(&pos) == true) {
        pos = mStartPos;
        mAngle.y.mAngle += DEG_TO_ANGLE(10);
    }
    *out = pos;
}

// 0x18CB8
f32 dInsFldStrider_c::getWaterY() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
    if (chk.mWater != 0) {
        return chk.getWaterY();
    }
    return 0.0f;
}

// 0x18D0C
bool dInsFldStrider_c::isBlockedWater(const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, dBGCF::LAYER_WATER, 0, 0);
    if (chk.mWater != 0 && chk.mUnitAttr == BG_ATTR_SEA) {
        return true;
    }
    return false;
}

// 0x18D60
void dInsFldStrider_c::syncState() {
    dInsectCatchData_c *data;
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner() &&
        (data = getCatchData()) != NULL) {
        mHomePos.x = data->getPosX() * 4;
        mHomePos.z = data->getPosZ() * 4;
        mState = data->getReq();
    } else if (mState == STATE_WAIT) {
        if (mWaitFrames <= mFrame) {
            mState = STATE_MOVE;
        }
    } else if (mState == STATE_MOVE && mMoveFrames <= mFrame) {
        mState = STATE_WAIT;
    }
}
