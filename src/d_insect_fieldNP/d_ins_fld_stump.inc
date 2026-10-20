// dInsFldStump_c: the violin beetle and the longhorn beetle. See
// include/game/game/d_ins_fld_stump.hpp.

const dInsFldStump_c::StateFunc dInsFldStump_c::sExecute[] = {
    &dInsFldStump_c::executeWait,
    &dInsFldStump_c::executeWalk,
    &dInsFldStump_c::executeFlyAway,
    &dInsFldStump_c::executeRelease,
};

const dInsFldStump_c::StateFunc dInsFldStump_c::sInit[] = {
    &dInsFldStump_c::initWait,
    &dInsFldStump_c::initWalk,
    &dInsFldStump_c::initFlyAway,
    &dInsFldStump_c::onRelease,
};

dInsFldStump_c::param_c dInsFldStump_c::sParam[] = {
    {0.0f, 16.5f, 0.0f, 10.0f, 5.0f, 5.0f, 3.0f, 0.01f, 0.4f, 0.5f, 0.6f, 0.03f,
     1.0f, 4.0f, 5.0f, 100.0f, 200},
    {0.0f, 16.0f, 0.0f, 10.0f, 5.0f, 5.0f, 3.0f, 0.01f, 0.4f, 0.5f, 0.6f, 0.03f,
     1.0f, 4.0f, 4.0f, 100.0f, 200},
};

// 0x2421C
int dInsFldStump_c::getParamIdx(int type) {
    const int types[] = {INSECT_VIOLIN_BEETLE, INSECT_LONGHORN_BEETLE};
    for (int i = 0; i < ARRAY_SIZE(types); i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x24250
const dInsFldStump_c::param_c *dInsFldStump_c::getStumpParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x24280
bool dInsFldStump_c::isStump(const dItem::Item *item, const mVec3_c *pos) {
    switch (item->mId) {
    case dItem::FG_STUMP:
    case dItem::FG_CEDAR_STUMP:
    case dItem::FG_PALM_STUMP:
        return true;
    }
    return false;
}

// 0x242B8
dInsFldStump_c::dInsFldStump_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mWaitFrames(0),
      mWalkFrames(0), _324(0), mUnitCenter(0.0f, 0.0f, 0.0f), mSwayY(0),
      mSwayZ(0), mSwayDir(false), mRot(1.0f, 0.0f, 0.0f, 0.0f) {
}

// 0x2434C
dInsFldStump_c::~dInsFldStump_c() {
}

// 0x243A4
BOOL dInsFldStump_c::create(const nw4r::g3d::ResFile &res) {
    const param_c *param = getStumpParam(mType);
    dInsFldBase_c::create(res);
    mScare.setLimit(param->mScareLimit);
    dFdBase_c::snapToUnitCenter(mUnitCenter, mInitPos);
    mInitPos.y = getGroundY();
    return TRUE;
}

// 0x24420
int dInsFldStump_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x24444
void dInsFldStump_c::calcMtx() {
    mMtx_c mtx, rotY, rotX, rotZ, sway;
    rotX.XrotS(mAngle.x);
    rotY.YrotS(mAngle.y);
    rotZ.ZrotS(mAngle.z);
    fn_80444FE4(sway, &mRot);
    dActor_c::makeMtx(&mtx, &mPos, 0);
    PSMTXConcat(mtx, rotY, mtx);
    PSMTXConcat(mtx, rotX, mtx);
    PSMTXConcat(mtx, rotZ, mtx);
    PSMTXConcat(mtx, sway, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0x24518
void dInsFldStump_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == 1) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL && state == STATE_FLY_AWAY) {
            data->setReq(mState);
        }
    }
}

// 0x245AC
void dInsFldStump_c::setGroundPos() {
    const param_c *param = getStumpParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
}

// 0x2461C
void dInsFldStump_c::initState() {
    setState(STATE_WAIT);
}

// 0x24624
void dInsFldStump_c::executeState() {
    getStumpParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// 0x24698
void dInsFldStump_c::initWait() {
    const param_c *param = getStumpParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitMax);
    int min = dInsectUtil::secToFrame(param->mWaitMin);
    mWaitFrames = cM::rndRange<u16>(min, max);
}

// 0x2475C
void dInsFldStump_c::initWalk() {
    const param_c *param = getStumpParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    mWalkFrames = cM::rndRange<int>(60, 180);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mVelocity.set(0.0f, 0.0f, param->mWalkSpeed);
    mVelocity.rotY(mAngle.y);
    getWalkTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mWalkFrames = (int)(dist / param->mWalkSpeed) + 1;
}

// 0x24884
void dInsFldStump_c::initFlyAway() {
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mAngle.y = getFlyAngle();
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mPos.y += 2.0f;
    mSound.startSound(SE_mushi_fly_out_midium);
}

// 0x24934
void dInsFldStump_c::onRelease() {
    const param_c *param = getStumpParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mSound.startSound(SE_mushi_fly_out_midium);
}

// 0x249CC
void dInsFldStump_c::executeWait() {
    registObjc(&mPos, mBaseRadius, TRUE);
}

// 0x249DC
void dInsFldStump_c::executeWalk() {
    const param_c *param = getStumpParam(mType);
    dInsectUtil::moveTo(&mPos, &mAngle.y, mHomePos, param->mWalkSpeed,
                        param->mWalkTurn);
    registObjc(&mPos, mBaseRadius, TRUE);
    if (mSwayDir == true) {
        mSwayY += SWAY_SPEED;
        mSwayZ -= SWAY_SPEED;
        if (mSwayY >= SWAY_MAX) {
            mSwayDir = false;
        }
    } else {
        mSwayY -= SWAY_SPEED;
        mSwayZ += SWAY_SPEED;
        if (mSwayY <= -SWAY_MAX) {
            mSwayDir = true;
        }
    }
    EGG::Vector3f axisZ(0.0f, 0.0f, 1.0f);
    f32 radZ = mSwayZ * mAng::AngleToRadianCoefficient;
    mQuat_c swayZ(axisZ, radZ);
    EGG::Vector3f axisY(0.0f, 1.0f, 0.0f);
    f32 radY = mSwayY * mAng::AngleToRadianCoefficient;
    mQuat_c swayY(axisY, radY);
    mRot = swayZ * swayY;
}

// 0x24BDC
void dInsFldStump_c::executeFlyAway() {
    const param_c *param = getStumpParam(mType);
    moveForward(param->mFlySpeed, param->mFlyAccelY);
}

// 0x24C20
void dInsFldStump_c::executeRelease() {
    const param_c *param = getStumpParam(mType);
    moveForward(param->mFlySpeed, param->mFlyAccelY);
}

// 0x24C64
mAng dInsFldStump_c::getFlyAngle() {
    int player = mPlayerCheck.getNearest();
    s16 away = mPlayerCheck.getAngleTo(player).mAngle - DEG_TO_ANGLE(180);
    s16 npcAway = away;
    if (mNpcCheck.isNear(mScareRadius) == true) {
        npcAway = mNpcCheck.getAngleTo().mAngle - DEG_TO_ANGLE(180);
    }
    return (away + npcAway) >> 1;
}

// 0x24CFC
mAng dInsFldStump_c::getWalkAngle() {
    s16 ang = mAngle.y;
    int range = DEG_TO_ANGLE(120);
    return ang + (int)cM::rndCenterF(range);
}

// 0x24D60
f32 dInsFldStump_c::getWalkDist() const {
    const param_c *param = getStumpParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x24D90
void dInsFldStump_c::getWalkTarget(mVec3_c *out) {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        return;
    }
    mAng ang = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    target.set(0.0f, 0.0f, 0.0f);
    getStumpPos(&target);
    pos = target;
    *out = pos;
}

// 0x24E80
void dInsFldStump_c::getStumpPos(mVec3_c *out) {
    f32 dist = cM::rndRange(0.0f, 3.0f);
    mAng ang = cM::rndRange<s16>(0, -DEG_TO_ANGLE(180));
    mVec3_c ofs(0.0f, 0.0f, dist);
    ofs.rotY(ang);
    out->x = mInitPos.x + ofs.x;
    out->z = mInitPos.z + ofs.z;
}

// 0x24F30
bool dInsFldStump_c::isSomeoneNear() {
    int player = mPlayerCheck.getNearest();
    bool playerNear = mPlayerCheck.isNear(player, 16.0f);
    bool npcNear = mNpcCheck.isNear(16.0f);
    if (playerNear == true || npcNear == true) {
        return true;
    }
    return false;
}

// 0x24FA8
BOOL dInsFldStump_c::isOnStump() {
    dInsectUtil::itemSearch_c search(isStump);
    search.search(mInitPos, 1.0f);
    return search.isFound();
}

// 0x25010
void dInsFldStump_c::syncState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        if (mState == STATE_WAIT) {
            if (mWaitFrames <= mFrame) {
                if (0.5f < cM::rnd()) {
                    mState = STATE_WAIT;
                } else {
                    mState = STATE_WALK;
                }
            }
        } else if (mState == STATE_WALK && mWalkFrames <= mFrame) {
            if (0.5f < cM::rnd()) {
                mState = STATE_WAIT;
            } else {
                mState = STATE_WALK;
            }
        }
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            int state = data->getReq();
            if (state == STATE_FLY_AWAY) {
                u32 req = data->getReq();
                if (req < STATE_NUM_NET) {
                    mState = req;
                }
            }
        }
    } else {
        if (mState == STATE_WAIT) {
            if (mWaitFrames <= mFrame) {
                if (0.5f < cM::rnd()) {
                    mState = STATE_WAIT;
                } else {
                    mState = STATE_WALK;
                }
            } else if (mScare.isScared() == true) {
                mState = STATE_FLY_AWAY;
            }
        } else if (mState == STATE_WALK) {
            if (mWalkFrames <= mFrame) {
                if (0.5f < cM::rnd()) {
                    mState = STATE_WAIT;
                } else {
                    mState = STATE_WALK;
                }
            } else if (mScare.isScared() == true) {
                mState = STATE_FLY_AWAY;
            }
        }
        if (mState != STATE_FLY_AWAY && mState != STATE_RELEASE) {
            if (isSomeoneNear() == true) {
                mState = STATE_FLY_AWAY;
            } else if (!isOnStump()) {
                mState = STATE_FLY_AWAY;
            }
        }
        if (mState != STATE_FLY_AWAY && mState != STATE_RELEASE &&
            mPlayerCheck.isDisturbing(mScareRadius) == true) {
            mScare.mValue = (int)(3.0f * (int)mScare.mLimit);
        }
    }
}
