// dInsFldDivingBeetle_c: the diving beetle. See
// include/game/game/d_ins_fld_diving_beetle.hpp.

const dInsFldDivingBeetle_c::StateFunc dInsFldDivingBeetle_c::sExecute[] = {
    &dInsFldDivingBeetle_c::executeWait,    &dInsFldDivingBeetle_c::executeSwim,
    &dInsFldDivingBeetle_c::executeSurface, &dInsFldDivingBeetle_c::executeDive,
    &dInsFldDivingBeetle_c::executeRelease,
};

const dInsFldDivingBeetle_c::StateFunc dInsFldDivingBeetle_c::sInit[] = {
    &dInsFldDivingBeetle_c::initWait,    &dInsFldDivingBeetle_c::initSwim,
    &dInsFldDivingBeetle_c::initSurface, &dInsFldDivingBeetle_c::initDive,
    &dInsFldDivingBeetle_c::onRelease,
};

const int dInsFldDivingBeetle_c::sTypes[] = {INSECT_DIVING_BEETLE};

dInsFldDivingBeetle_c::param_c dInsFldDivingBeetle_c::sParam[] = {
    {0.0f, -3.0f, 0.0f, 3.0f, 0.0f, 120.0f, 40.0f, 1.5f, 0.5f, 0.7f, 10, 0.98f,
     0.4f, 0.02f, 1.0f, 0.0f, 8.0f},
};

// 0x18E6C
int dInsFldDivingBeetle_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x18E8C
const dInsFldDivingBeetle_c::param_c *
dInsFldDivingBeetle_c::getDivingBeetleParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x18EBC
dInsFldDivingBeetle_c::dInsFldDivingBeetle_c()
    : mState(STATE_WAIT), mPrevState(STATE_WAIT), mStartPos(0.0f, 0.0f, 0.0f),
      mWaitFrames(0), mMoveFrames(0), mVisAlpha(0) {
}

// 0x18F24
dInsFldDivingBeetle_c::~dInsFldDivingBeetle_c() {
}

// 0x18F7C
BOOL dInsFldDivingBeetle_c::create(const nw4r::g3d::ResFile &res) {
    dInsFldBase_c::create(res);
    mStartPos = mPos;
    mHomePos = mPos;
    mAcch.mHitFlags |= dBGCF::HIT_UNDER_WATER;
    return TRUE;
}

// 0x18FDC
int dInsFldDivingBeetle_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x19000
void dInsFldDivingBeetle_c::onBecomeOwner() {
    mAcch.mHitFlags |= dBGCF::HIT_UNDER_WATER;
    const param_c *param = getDivingBeetleParam(mType);
    if (mPos.y < dBGCF::getGroundY(&mPos, TRUE)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = param->mOffsetY + getWaterY();
        mPos = pos;
        mOldPos = pos;
        mVelocity.set(0.0f, 0.0f, 0.0f);
    }
}

// 0x19130
void dInsFldDivingBeetle_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == 1) {
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

// 0x192C0
void dInsFldDivingBeetle_c::setGroundPos() {
    mAcch.mHitFlags |= dBGCF::HIT_UNDER_WATER;
    const param_c *param = getDivingBeetleParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getWaterY();
    mPos.z += param->mOffsetZ;
}

// 0x1933C
void dInsFldDivingBeetle_c::initState() {
    setState(STATE_WAIT);
}

// 0x19344
void dInsFldDivingBeetle_c::executeState() {
    getDivingBeetleParam(mType);
    mAcch.mHitFlags |= dBGCF::HIT_UNDER_WATER;
    mPrevState = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
    mBgRadius = 32.0f;
}

// 0x193D0
void dInsFldDivingBeetle_c::initWait() {
    const param_c *param = getDivingBeetleParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.1f);
    setAnmFrame(frame);
    int max = dInsectUtil::secToFrame(param->mWaitMax);
    int min = dInsectUtil::secToFrame(param->mWaitMin);
    mWaitFrames = cM::rndRange<u16>(min, max);
}

// 0x19490
void dInsFldDivingBeetle_c::initSwim() {
    const param_c *param = getDivingBeetleParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.5f);
    setAnmFrame(frame);
    getMoveTarget(&mHomePos);
    mStartPos = mPos;
    mMoveFrames = SWIM_TIME;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mVec3_c diff = mHomePos - mStartPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mVelocity.set(0.0f, 0.0f,
                  dist / (param->mMoveDecay / (1.0f - param->mMoveDecay)));
    mVelocity.rotY(mAngle.y);
}

// 0x195D0
void dInsFldDivingBeetle_c::initSurface() {
    const param_c *param = getDivingBeetleParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.5f);
    setAnmFrame(frame);
    getMoveTarget(&mHomePos);
    mStartPos = mPos;
    mMoveFrames = SWIM_TIME;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mVec3_c diff = mHomePos - mStartPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mVelocity.set(0.0f, 0.0f,
                  dist / (param->mMoveDecay / (1.0f - param->mMoveDecay)));
    mVelocity.rotY(mAngle.y);
}

// 0x19710
void dInsFldDivingBeetle_c::initDive() {
    const param_c *param = getDivingBeetleParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.5f);
    setAnmFrame(frame);
    getMoveTarget(&mHomePos);
    mStartPos = mPos;
    mMoveFrames = SWIM_TIME;
    mAngle.y = cLib::targetAngleY(mStartPos, mHomePos);
    mVec3_c diff = mHomePos - mStartPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    mVelocity.set(0.0f, 0.0f,
                  dist / (param->mMoveDecay / (1.0f - param->mMoveDecay)));
    mVelocity.rotY(mAngle.y);
}

// 0x19850
void dInsFldDivingBeetle_c::onRelease() {
    const param_c *param = getDivingBeetleParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mSound.startSound(SE_mushi_fly_out_midium);
    mShadowSize = 6.0f;
}

// 0x198F4
void dInsFldDivingBeetle_c::executeWait() {
    if (mVisAlpha == 255) {
        registObjc(&mPos, mBaseRadius, TRUE);
    } else {
        registObjc(&mPos, mBaseRadius, FALSE);
    }
}

// 0x19920
void dInsFldDivingBeetle_c::executeSwim() {
    swim();
    registObjc(&mPos, mBaseRadius, FALSE);
}

// 0x19960
void dInsFldDivingBeetle_c::executeSurface() {
    const param_c *param = getDivingBeetleParam(mType);
    mVisAlpha += 255 / param->mFadeFrames;
    mVisAlpha = mVisAlpha > 255 ? 255 : mVisAlpha;
    setMatAlpha(mVisAlpha);
    if (mVisAlpha >= CATCHABLE_ALPHA) {
        registObjc(&mPos, mBaseRadius, TRUE);
    } else {
        registObjc(&mPos, mBaseRadius, FALSE);
    }
    swim();
}

// 0x19A08
void dInsFldDivingBeetle_c::executeDive() {
    const param_c *param = getDivingBeetleParam(mType);
    mVisAlpha -= 255 / param->mFadeFrames;
    mVisAlpha = mVisAlpha < 0 ? 0 : mVisAlpha;
    setMatAlpha(mVisAlpha);
    if (mVisAlpha >= CATCHABLE_ALPHA) {
        registObjc(&mPos, mBaseRadius, TRUE);
    } else {
        registObjc(&mPos, mBaseRadius, FALSE);
    }
    swim();
}

// 0x19AA8
void dInsFldDivingBeetle_c::executeRelease() {
    const param_c *param = getDivingBeetleParam(mType);
    moveForward(param->mReleaseSpeed, param->mReleaseAccelY);
}

// 0x19AEC
mAng dInsFldDivingBeetle_c::getMoveAngle() {
    return mAngle.y.mAngle + (int)cM::rndCenterF(10923.0f);
}

// 0x19B38
f32 dInsFldDivingBeetle_c::getMoveDist() const {
    const param_c *param = getDivingBeetleParam(mType);
    return cM::rndRange(param->mMoveDistMin, param->mMoveDistMax);
}

// 0x19B68
void dInsFldDivingBeetle_c::getMoveTarget(mVec3_c *out) {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        return;
    }
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

// 0x19CFC
f32 dInsFldDivingBeetle_c::getWaterY() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
    if (chk.mWater != 0) {
        return chk.getWaterY();
    }
    return 0.0f;
}

// 0x19D50
void dInsFldDivingBeetle_c::setMatAlpha(u8 alpha) {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("m0");
    if (mat.IsValid()) {
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn());
        nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mat.ptr()->id);
        if (access.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = access.GetResMatTevColor();
            tev.GXGetTevColor(GX_TEVREG0, &color);
            color.a = alpha;
            tev.GXSetTevColor(GX_TEVREG0, color);
            tev.DCStore(false);
        }
    }
}

// 0x19E6C
void dInsFldDivingBeetle_c::swim() {
    const param_c *param = getDivingBeetleParam(mType);
    f32 t = (f32)mFrame / mMoveFrames;
    t = cLib::easeOut(t > 1.0f ? 1.0f : t, 2.0f);
    mVelocity *= param->mMoveDecay;
    mPos += mVelocity;
    setAnmRate(param->mSwimAnmRateStart -
               t * (param->mSwimAnmRateStart - param->mSwimAnmRateEnd));
    mAngle.y = cLib::targetAngleY(mPos, mHomePos);
}

// 0x19F88
bool dInsFldDivingBeetle_c::isBlockedWater(const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, dBGCF::LAYER_WATER, 0, 0);
    if (chk.mWater != 0 && chk.mUnitAttr == BG_ATTR_SEA) {
        return true;
    }
    return false;
}

// 0x19FDC
void dInsFldDivingBeetle_c::syncState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dInsectCatchData_c *data = getCatchData();
        if (data != NULL) {
            mHomePos.x = data->getPosX() * 4;
            mHomePos.z = data->getPosZ() * 4;
            u32 req = data->getReq();
            if (req < STATE_NUM_NET) {
                mState = req;
            }
        }
    } else if (mState == STATE_WAIT) {
        if (mWaitFrames <= mFrame) {
            if (mVisAlpha == 255) {
                mState = STATE_DIVE;
            } else {
                const param_c *param = getDivingBeetleParam(mType);
                if (cM::rnd() <= param->mSwimChance) {
                    mState = STATE_SURFACE;
                } else {
                    mState = STATE_SWIM;
                }
            }
        }
    } else if (mState == STATE_SWIM) {
        if (mMoveFrames <= mFrame) {
            mState = STATE_WAIT;
        }
    } else if (mState == STATE_SURFACE) {
        if (mMoveFrames <= mFrame) {
            mState = STATE_WAIT;
        }
    } else if (mState == STATE_DIVE && mMoveFrames <= mFrame) {
        mState = STATE_WAIT;
    }
}
