// dInsMusPillBug_c: the pill bug and the centipede walk around; the pill bug rolls up when scared.
namespace dInsectMuseum {

enum { PILL_BUG_TYPE_NUM = 2 };

dInsMusPillBugParam_c sPillBugParam[PILL_BUG_TYPE_NUM] = {
    {{-16.0f, 2.0f, 0.0f}, 20.0f, 10.0f, 0.02f, 0.002f, 0.9f, 0.01f, 0.2f, 3.0f, 100.0f, 300.0f, 3},  // pill bug
    {{25.0f, -6.0f, 0.0f}, 20.0f, 10.0f, 0.05f, 0.005f, 0.9f, 0.01f, 1.2f, 3.0f, 100.0f, 70.0f, 13}, // centipede
};

// 0xE6C4
const dInsMusPillBugParam_c *getPillBugParam(int type) {
    const int types[PILL_BUG_TYPE_NUM] = {INSECT_PILL_BUG, INSECT_CENTIPEDE};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sPillBugParam[i];
        }
    }
    return &sPillBugParam[0];
}

} // namespace dInsectMuseum

// 0xE728
dInsMusPillBug_c::dInsMusPillBug_c() : mState(NULL), mMoveTime(0) {}

// 0xE7C0
dInsMusPillBug_c::~dInsMusPillBug_c() {}

// 0xE828
BOOL dInsMusPillBug_c::draw() {
    if (isPillBug() == true) {
        dInsMusBase_c::draw();
    } else {
        s16 angleY = mAxis.getAngleY();
        mShadowSize = 0.0f;
        dInsMusBase_c::draw();
        mAng angle(angleY);
        dShadow::requestRound(&mPos, 0, &angle, 4.0f, 1.0f, 1.0f, 10.0f, 1.0f);
    }
    return TRUE;
}

// 0xE8C8
BOOL dInsMusPillBug_c::doDelete() {
    dInsMusBase_c::doDelete();
    if (!isPillBug()) {
        mAnmTexSrt.remove();
    }
    return TRUE;
}

// 0xE91C
void dInsMusPillBug_c::calcMtx() {
    setAxisMtx();
}

// 0xE920
void dInsMusPillBug_c::createModel(nw4r::g3d::ResFile &res) {
    if (isPillBug() == true) {
        dInsMusBase_c::createModel(res);
    } else {
        nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
        mMdl.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR | nw4r::g3d::ScnMdl::ANM_TEXSRT, 1, NULL);
        mAnmChrWait = res.GetResAnmChr("wait");
        if (mAnmChrWait.IsValid()) {
            mAnmChr.create(mdl, mAnmChrWait, &mAllocator, NULL);
        }
        mResAnmTexSrt = res.GetResAnmTexSrt("wait");
        if (mResAnmTexSrt.IsValid()) {
            mAnmTexSrt.create(mdl, mResAnmTexSrt, &mAllocator, NULL, 1);
        }
        mMdl.setAnm(mAnmTexSrt);
        mMdl.setAnm(mAnmChr);
        mCreated = true;
    }
}

// 0xEA5C
void dInsMusPillBug_c::init() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mShadowSize = param->mShadowSize;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
    mBgFlags = dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0xEB2C
void dInsMusPillBug_c::initState() {
    initWalk();
}

// 0xEB30
void dInsMusPillBug_c::executeState() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    if (!isPillBug()) {
        mAnmTexSrt.play();
    }
    (this->*mState)();
}

// 0xEB94
void dInsMusPillBug_c::initWalk() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusPillBug_c::stateWalk;
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mVec3_c diff = mTargetPos - mPrevTargetPos;
    f32 dist = diff.xzLen();
    mMoveTime = (int)(dist / param->mWalkSpeed) + 1;
}

// 0xECB0
void dInsMusPillBug_c::initRoll() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusPillBug_c::stateRoll;
}

// 0xED48
void dInsMusPillBug_c::initUnroll() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(ROLL_END_FRAME);
    mState = &dInsMusPillBug_c::stateUnroll;
}

// 0xEDE0
void dInsMusPillBug_c::stateWalk() {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    flyToTarget(0.0f, param->mWalkAccel, param->mWalkBrake, param->mWalkTurnRate);
    if (mMoveTime <= mTimer) {
        initWalk();
    } else if (isPillBug() == true && mScare.isScared() == true) {
        initRoll();
    }
}

// 0xEE74
void dInsMusPillBug_c::stateRoll() {
    if (ROLL_END_FRAME <= mAnmChr.getFrame()) {
        if (!mScare.isScared()) {
            initUnroll();
        } else {
            setAnmRate(0.0f);
            setAnmFrame(ROLL_END_FRAME);
        }
    }
}

// 0xEEF8
void dInsMusPillBug_c::stateUnroll() {
    if (UNROLL_END_FRAME <= mAnmChr.getFrame()) {
        initWalk();
    }
}

// 0xEF44
mAng dInsMusPillBug_c::getWalkAngle() {
    return mAngle.y.mAngle + (int)cM::rndCenterF(0x2AAB);
}

// 0xEF90
f32 dInsMusPillBug_c::getWalkDist() const {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    return cM::rndRange(param->mWalkDistMin, param->mWalkDistMax);
}

// 0xEFC0
void dInsMusPillBug_c::getWalkTarget(mVec3_c *target) {
    const dInsMusPillBugParam_c *param = getPillBugParam(mType);
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}

// 0xF0D0
bool dInsMusPillBug_c::isPillBug() {
    return mType == INSECT_PILL_BUG;
}

// 0xF0E4
void dInsMusPillBug_c::setAnmRate(f32 rate) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setRate(rate);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setRate(rate);
    }
    if (mAnmTexSrt.IsBound() == true) {
        mAnmTexSrt.setRate(rate / 2.0f, 0);
    }
}
