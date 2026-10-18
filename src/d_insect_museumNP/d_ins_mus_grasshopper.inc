// dInsMusGrasshopper_c: waits (the crickets chirp), turns to a random point and jumps to it.
namespace dInsectMuseum {

enum { GRASSHOPPER_TYPE_NUM = 5 };

dInsMusGrasshopperParam_c sGrasshopperParam[GRASSHOPPER_TYPE_NUM] = {
    {{40.0f, 0.0f, 20.0f}, 10.0f, 5.0f, 1.0f, 1.0f, 1.8f, 60.0f, 45.0f, 0.9f, -0.2f, 1.0f, 0.8f, 3.0f, 5.0f, 80.0f, 0, 5}, // long_locust
    {{3.0f, 0.0f, 20.0f}, 10.0f, 5.0f, 1.0f, 1.0f, 1.3f, 60.0f, 45.0f, 0.9f, -0.2f, 1.0f, 0.8f, 5.0f, 7.0f, 80.0f, 0, 5}, // migratory_locust
    {{20.0f, 0.0f, -40.0f}, 10.0f, 5.0f, 1.0f, 1.0f, 1.3f, 30.0f, 15.0f, 0.9f, -0.1f, 1.0f, 1.0f, 4.0f, 5.0f, 80.0f, 400, 16}, // cricket
    {{40.0f, 0.0f, -40.0f}, 10.0f, 5.0f, 1.0f, 1.0f, 1.3f, 30.0f, 15.0f, 0.9f, -0.1f, 1.0f, 1.0f, 4.0f, 5.0f, 80.0f, 400, 16}, // bell_cricket
    {{-74.0f, 0.0f, -15.0f}, 10.0f, 5.0f, 1.0f, 1.0f, 1.3f, 60.0f, 30.0f, 0.9f, -0.15f, 1.0f, 0.8f, 4.0f, 5.0f, 80.0f, 400, 17}, // grasshopper
};

// 0x74C4
int getGrasshopperParamIdx(int type) {
    const int types[GRASSHOPPER_TYPE_NUM] = {
        INSECT_LONG_LOCUST, INSECT_MIGRATORY_LOCUST, INSECT_CRICKET, INSECT_BELL_CRICKET, INSECT_GRASSHOPPER,
    };
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x7534
const dInsMusGrasshopperParam_c *getGrasshopperParam(int type) {
    return &sGrasshopperParam[getGrasshopperParamIdx(type)];
}

// 0x7564
u32 getGrasshopperSound(int type) {
    const u32 sounds[GRASSHOPPER_TYPE_NUM] = {
        SE_shouryoubatta, SE_tonosamabatta, SE_koorogi, SE_suzumushi, SE_kirigirisu,
    };
    return sounds[getGrasshopperParamIdx(type)];
}

static mAng sJumpAngleRange(0x2AAB);

} // namespace dInsectMuseum

// 0x75BC
dInsMusGrasshopper_c::dInsMusGrasshopper_c() : mState(NULL), mWaitTime(0), mJumpTime(0), mTargetAngle(0) {}

// 0x7624
dInsMusGrasshopper_c::~dInsMusGrasshopper_c() {}

// 0x767C
BOOL dInsMusGrasshopper_c::draw() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    setScale(param->mScale);
    dInsMusBase_c::draw();
    mAngle.y = mAxis.getAngleY();
    f32 alpha = 0.5f * (mAlpha / 255.0f);
    mAng angle = mAngle.y;
    dShadow::requestRound(&mPos, 0, &angle, param->mShadowSizeX, 1.0f, alpha, param->mShadowSizeZ, 1.0f);
    return TRUE;
}

// 0x7740
void dInsMusGrasshopper_c::calcMtx() {
    setAxisMtx();
}

// 0x7744
void dInsMusGrasshopper_c::init() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y = getGroundY() + param->mOffset.y;
    mPos.z += param->mOffset.z;
    mShadowSize = 0.0f;
    mScareRadius = param->mScareRadius;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSizeZ;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
}

// 0x77EC
void dInsMusGrasshopper_c::initState() {
    initTurn();
}

// 0x77F0
void dInsMusGrasshopper_c::executeState() {
    mVec3_c oldPos = mPos;
    (this->*mState)();
    mCheckBg = false;
    mAcch.check(mBgRadius, &mPos, &oldPos, mAngle.y, 0, mBgFlags, TRUE);
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
    }
}

// 0x788C
void dInsMusGrasshopper_c::initWait() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    mTimer = 0;
    mState = &dInsMusGrasshopper_c::stateWait;
    if (isChirping() == true) {
        setAnmChr(mAnmChrStop, 5.0f);
        setAnmVis(mAnmVisStop);
        if (isActiveTime() == true) {
            setAnmRate(param->mChirpAnmRate);
        } else {
            setAnmRate(0.0f);
        }
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
        setAnmRate(0.0f);
    }
    setAnmFrame(0.0f);
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
    if (!isActiveTime()) {
        mWaitTime *= 5.0f;
    }
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

// 0x7A18
void dInsMusGrasshopper_c::initTurn() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    mTimer = 0;
    mState = &dInsMusGrasshopper_c::stateTurn;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 60.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mTurnAnmRate);
    setAnmFrame(frame);
    getJumpTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mTargetAngle = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

// 0x7B14
void dInsMusGrasshopper_c::initJump() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    mTimer = 0;
    mState = &dInsMusGrasshopper_c::stateJump;
    setAnmChr(mAnmChrFly, 30.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mJumpAnmRate);
    setAnmFrame(0.0f);
    mJumpTime = JUMP_TIME;
    f32 dist = mTargetPos.xzDistTo(mPos);
    f32 speedY = getJumpSpeedY(param->mGravity, mJumpTime);
    f32 speedXZ = getJumpSpeedXZ(dist, param->mJumpFriction);
    mAngle.y = cLib::targetAngleY(mPos, mTargetPos);
    mSpeed.set(0.0f, speedY, speedXZ);
    mSpeed.rotY(mAngle.y);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x7C80
void dInsMusGrasshopper_c::stateWait() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    if (isChirping() == true) {
        if (mScare.isScared() == true) {
            setAnmRate(0.0f);
        } else if (isActiveTime() == true) {
            mSnd.holdSound(getGrasshopperSound(mType));
            setAnmRate(param->mChirpAnmRate);
        }
    }
    if (mWaitTime <= mTimer) {
        initTurn();
    }
}

// 0x7D38
void dInsMusGrasshopper_c::stateTurn() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle.mAngle, 1, step, step);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (mAngle.y.mAngle == mTargetAngle.mAngle) {
        initJump();
    }
}

// 0x7DD0
void dInsMusGrasshopper_c::stateJump() {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    mSpeed.x *= param->mJumpFriction;
    mSpeed.z *= param->mJumpFriction;
    mSpeed.y += param->mGravity;
    mPos += mSpeed;
    if (isUnderGround() == true) {
        mSpeed.y = 0.0f;
        initWait();
    }
    if (!isChirping()) {
        mSnd.holdSound(getGrasshopperSound(mType));
    }
}

// 0x7E9C
mAng dInsMusGrasshopper_c::getJumpAngle() const {
    s16 angle = mAngle.y.mAngle;
    return angle + (s16)cM::rndCenterF(sJumpAngleRange);
}

// 0x7F04
f32 dInsMusGrasshopper_c::getJumpDist() const {
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    return cM::rndRange<f32>(param->mJumpDistMin, param->mJumpDistMax);
}

// 0x7F34
void dInsMusGrasshopper_c::getJumpTarget(mVec3_c *target) {
    mAng angle = getJumpAngle();
    f32 dist = getJumpDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    result = pos;
    const dInsMusGrasshopperParam_c *param = getGrasshopperParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    if (param->mJumpDistMax <= result.xzDistTo(mPos)) {
        mVec3_c dir = result - mPos;
        dir.y = 0.0f;
        dir.normalize();
        dir *= param->mJumpDistMax;
        result = mPos + dir;
    }
    *target = result;
}

// 0x8100
bool dInsMusGrasshopper_c::isChirping() {
    bool bellCricket = mType == INSECT_BELL_CRICKET; // bool locals: computed before the first test
    bool grasshopper = mType == INSECT_GRASSHOPPER;
    if (mType == INSECT_CRICKET || bellCricket == true || grasshopper == true) {
        return true;
    }
    return false;
}

// 0x8144
f32 dInsMusGrasshopper_c::getJumpSpeedY(f32 gravity, u16 time) {
    return -gravity * (time / 2);
}

// 0x817C
f32 dInsMusGrasshopper_c::getJumpSpeedXZ(f32 dist, f32 friction) {
    return dist / (friction / (1.0f - friction));
}
