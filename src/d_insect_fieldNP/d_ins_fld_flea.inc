// dInsFldFlea_c: the flea. See include/game/game/d_ins_fld_flea.hpp.

const dInsFldFlea_c::StateFunc dInsFldFlea_c::sExecute[] = {
    &dInsFldFlea_c::executeWait, &dInsFldFlea_c::executeJump};

const dInsFldFlea_c::StateFunc dInsFldFlea_c::sInit[] = {
    &dInsFldFlea_c::initWait, &dInsFldFlea_c::onRelease};

dInsFldFlea_c::param_c dInsFldFlea_c::sParam[] = {
    {0.0f, 12.0f, 20.0f, 0.8f, 1.5f, -0.15f, 0.0f, 5.0f},
};

// .bss 0x40: the flea (getFlea / addFlea).
dInsFldBase_c *dInsectField::sFlea;

// 0x1E478
int dInsFldFlea_c::getParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_FLEA:
        idx = 0;
        break;
    }
    return idx;
}

// 0x1E48C
const dInsFldFlea_c::param_c *dInsFldFlea_c::getFleaParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x1E4BC
dInsFldFlea_c::dInsFldFlea_c() : mState(STATE_WAIT) {
}

// 0x1E500
dInsFldFlea_c::~dInsFldFlea_c() {
}

// 0x1E558
BOOL dInsFldFlea_c::create(const nw4r::g3d::ResFile &res) {
    bool ok = dInsFldBase_c::create(res);
    dInsectField::sFlea = this;
    return ok;
}

// 0x1E598
BOOL dInsFldFlea_c::doDelete() {
    bool ok = dInsFldBase_c::doDelete();
    dInsectField::sFlea = NULL;
    return ok;
}

// 0x1E5D0
void dInsFldFlea_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x1E608
void dInsFldFlea_c::setGroundPos() {
    const param_c *param = getFleaParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
}

// 0x1E678
void dInsFldFlea_c::initState() {
    setState(STATE_JUMP);
    mActive = false;
}

// 0x1E6B0
void dInsFldFlea_c::executeState() {
    (this->*sExecute[mState])();
}

// 0x1E6E8
void dInsFldFlea_c::initWait() {
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
}

// 0x1E734
void dInsFldFlea_c::onRelease() {
    mState = STATE_JUMP;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    const param_c *param = getFleaParam(mType);
    mVelocity.set(0.0f, param->mJumpSpeedY, param->mJumpSpeed);
    mVelocity.rotY(mAngle.y);
    mSound.startSound(SE_mushi_out_fall);
    mShadowSize = 1.0f;
}

// 0x1E7D4
void dInsFldFlea_c::executeWait() {
}

// 0x1E7D8
void dInsFldFlea_c::executeJump() {
    const param_c *param = getFleaParam(mType);
    mVelocity.y += param->mGravity;
    mPos += mVelocity;
    if (isUnderGround() == TRUE) {
        mPos.y = getGroundY();
        setState(STATE_JUMP);
    }
    if (isUnderWater() == TRUE) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.5f, 0.5f, 0.5f);
        dEffect::create("af_hny_watercolumn_S", &mPos, &mAngle, &scale);
        mActive = false;
        requestDelete();
    }
    mAngle.y = cM::atan2s(mVelocity.x, mVelocity.z);
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}
