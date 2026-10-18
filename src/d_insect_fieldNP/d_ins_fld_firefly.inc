// dInsFldFirefly_c: the firefly. See include/game/game/d_ins_fld_firefly.hpp.

const dInsFldFirefly_c::StateFunc dInsFldFirefly_c::sExecute[] = {
    &dInsFldFirefly_c::executeFly, &dInsFldFirefly_c::executeRelease};

const dInsFldFirefly_c::StateFunc dInsFldFirefly_c::sInit[] = {
    &dInsFldFirefly_c::initFly, &dInsFldFirefly_c::onRelease};

const int dInsFldFirefly_c::sTypes[] = {INSECT_FIREFLY};

dInsFldFirefly_c::param_c dInsFldFirefly_c::sParam[] = {
    {0.0f, 40.0f, 0.0f, 20.0f, 15.0f, 0.1f, 0.8f, 1.8f, 4.0f,  4.0f,
     0.2f, 0.8f,  1.5f, 0.8f,  0.05f, 1.0f, 7.0f, 4.0f, 80.0f, 70},
};

// The glow's two colors (.bss 0x2C / 0x3C, constructed in __sinit).
static mColor l_glowColorA(8, 72, 16, 255);
static mColor l_glowColorB(40, 160, 80, 255);

// 0x1D24C
const dInsFldFirefly_c::param_c *dInsFldFirefly_c::getFireflyParam(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return &sParam[i];
        }
    }
    return &sParam[0];
}

// 0x1D280
dInsFldFirefly_c::dInsFldFirefly_c()
    : mState(STATE_FLY), mBobHeight(0.0f), mBaseY(0.0f), _334(0.0f),
      mMoveFrames(0), mBobPhase(0) {
}

// 0x1D328
dInsFldFirefly_c::~dInsFldFirefly_c() {
}

// 0x1D3C0
BOOL dInsFldFirefly_c::create(const nw4r::g3d::ResFile &res) {
    dInsFldBase_c::create(res);
    mBaseY = mPos.y;
    mGlow.init();
    return TRUE;
}

// 0x1D400
int dInsFldFirefly_c::draw() {
    mGlow.draw(&mPos);
    f32 alpha = mAlpha / 255.0f * sShadowAlphaScale;
    mAng ang = 0;
    dShadow::requestRound(&mPos, 0, &ang, mShadowSize, 1.0f, alpha, 0.0f, 1.0f);
    return TRUE;
}

// 0x1D4A0
void dInsFldFirefly_c::onBecomeOwner() {
    const param_c *param = getFireflyParam(mType);
    if (mPos.y < dBGCF::getGroundY(&mPos, 1)) {
        mVec3_c pos;
        pos.x = getCatchData()->getPosX() * 4;
        pos.z = getCatchData()->getPosZ() * 4;
        pos.y = param->mHeight + dBGCF::getGroundY(&pos, 0);
        mPos = pos;
        mOldPos = mPos;
    }
}

// 0x1D5B8
void dInsFldFirefly_c::setState(int state) {
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

// 0x1D748
void dInsFldFirefly_c::setGroundPos() {
    const param_c *param = getFireflyParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mHeight + getGroundY();
    mPos.z += param->mOffsetZ;
}

// 0x1D7B8
void dInsFldFirefly_c::initState() {
    setState(STATE_FLY);
}

// 0x1D7C0
void dInsFldFirefly_c::executeState() {
    getFireflyParam(mType);
    mPrevState = mState;
    (this->*sExecute[mState])();
    syncState();
    int &state = mState;
    if (mPrevState != state) {
        setState(mState);
    }
}

// 0x1D834
void dInsFldFirefly_c::initFly() {
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(1.0f);
    setAnmFrame(frame);
    setFlyTarget();
}

// 0x1D8C4
void dInsFldFirefly_c::onRelease() {
    const param_c *param = getFireflyParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    mSound.startSound(SE_mushi_fly_out_small);
}

// 0x1D95C
void dInsFldFirefly_c::executeFly() {
    const param_c *param = getFireflyParam(mType);
    f32 speed;
    f32 turn;
    if (mScare.isScared() == true) {
        speed = param->mScaredSpeed;
        turn = param->mScaredTurn;
    } else {
        speed = param->mFlySpeed;
        turn = param->mFlyTurn;
    }
    dInsectUtil::moveTo(&mPos, &mAngle.y, mHomePos, speed, turn);
    bob();
    mGlowEffect.create("af_ins_hotaru_glow", &mPos, &mAngle, NULL, 1);
    registObjc(&mPos, mBaseRadius, TRUE);
}

// 0x1DA08
void dInsFldFirefly_c::executeRelease() {
    const param_c *param = getFireflyParam(mType);
    moveForward(param->mReleaseSpeed, param->mReleaseAccelY);
    mGlowEffect.create("af_ins_hotaru_glow", &mPos, &mAngle, NULL, 1);
    mGlowEffect.setColor(0xFF, 0xFF, 0xFF, mAlpha / 2,
                         EGG::Effect::RECURSIVE_0);
}

// 0x1DA98
void dInsFldFirefly_c::bob() {
    const param_c *param = getFireflyParam(mType);
    f32 height = getFlyHeight();
    if (height < mBaseY) {
        mBaseY -= 0.05f;
        mBaseY = mBaseY < height ? height : mBaseY;
    } else {
        mBaseY += 0.05f;
        mBaseY = mBaseY > height ? height : mBaseY;
    }
    s16 prev = mBobPhase;
    s16 next = prev + mAng::fromDegree(param->mBobSpeed);
    if (prev > 0 && next <= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    } else if (prev <= 0 && next >= 0) {
        mBobHeight = cM::rndRange(param->mBobHeightMin, param->mBobHeightMax);
    }
    mPos.y = mBaseY + mBobHeight * mAng(mBobPhase).sin();
    mBobPhase += mAng::fromDegree(param->mBobSpeed);
}

// 0x1DBF8
mAng dInsFldFirefly_c::getFlyAngle() {
    s16 ang = mAngle.y;
    int range = DEG_TO_ANGLE(120);
    return ang + (int)cM::rndCenterF(range);
}

// 0x1DC5C
f32 dInsFldFirefly_c::getFlyDist() const {
    const param_c *param = getFireflyParam(mType);
    return cM::rndRange(param->mFlyDistMin, param->mFlyDistMax);
}

// 0x1DC8C
void dInsFldFirefly_c::getFlyTarget(mVec3_c *out) {
    const param_c *param = getFireflyParam(mType);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        return;
    }
    mAng ang = getFlyAngle();
    f32 dist = getFlyDist();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    mVec3_c target(0.0f, 0.0f, dist);
    target.rotY(ang);
    target += mPos;
    u8 steps = (int)(param->mFlyDistMax / 32.0f) + 1;
    mVec3_c from = mPos;
    mVec3_c to = target;
    if (!dInsectUtil::getGroundMovePos(&pos, from, to, steps, TRUE, TRUE,
                                       -500.0f, 0.0f)) {
        dInsectUtil::getLandPos(&pos, mPos, 1, TRUE);
    }
    *out = pos;
}

// 0x1DDF4
f32 dInsFldFirefly_c::getFlyHeight() {
    const param_c *param = getFireflyParam(mType);
    f32 height = param->mHeight;
    if (mScare.isScared() == true) {
        height *= param->mScaredHeight;
    }
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (chk.mWater != 0) {
        return height + chk.getWaterY();
    }
    return height + getGroundY();
}

// 0x1DE98
void dInsFldFirefly_c::syncState() {
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
    } else {
        if (mState == STATE_FLY && mMoveFrames <= mFrame) {
            mState = STATE_FLY;
            setFlyTarget();
        }
        if (mPlayerCheck.isDisturbing(mScareRadius) == true &&
            mState != STATE_RELEASE) {
            mScare.mValue = (int)(3.0f * (int)mScare.mLimit);
        }
    }
}

// 0x1E004
void dInsFldFirefly_c::setFlyTarget() {
    const param_c *param = getFireflyParam(mType);
    mFrame = 0;
    getFlyTarget(&mHomePos);
    mVec3_c diff = mHomePos - mPos;
    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    f32 speed;
    if (mScare.isScared() == true) {
        speed = param->mScaredSpeed;
    } else {
        speed = param->mFlySpeed;
    }
    mMoveFrames = (int)(dist / speed) + 1;
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

// 0x1E1F8
dInsFldFirefly_c::glow_c::glow_c() : mPhase(0), mSpeed(0), mAlpha(0xFF) {
}

// 0x1E210
dInsFldFirefly_c::glow_c::~glow_c() {
}

// 0x1E250
void dInsFldFirefly_c::glow_c::init() {
    mPhase = mAng::fromDegree(cM::rndRange(-180.0f, 179.0f));
    mSpeed = mAng::fromDegree(cM::rndRange(2.5f, 5.0f));
}

// 0x1E2E4
void dInsFldFirefly_c::glow_c::draw(const mVec3_c *pos) {
    dEffectLight_c *light = dEffectLight_c::sInstance;
    if (light != NULL) {
        int prev = mPhase;
        mPhase += mSpeed;
        if (mPhase * prev <= 0) {
            mSpeed = mAng::fromDegree(cM::rndRange(2.5f, 5.0f));
        }
        f32 t = 0.5f + 0.5f * nw4r::math::SinIdx(mPhase);
        mColor colorA = l_glowColorA;
        mColor colorB = l_glowColorB;
        mColor color;
        color.lerp(colorA, colorB, t);
        f32 size = 44.8f + (51.2f - 44.8f) * t;
        size *= mAlpha / 255.0f;
        light->addGlow(&color, pos, size);
    }
}
