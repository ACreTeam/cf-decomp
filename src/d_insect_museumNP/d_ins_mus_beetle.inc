// dInsMusBeetle_c: climbs up and down its trunk; the golden stag and hercules beetle walk around a stump.
namespace dInsectMuseum {

enum { BEETLE_TYPE_NUM = 16 };

dInsMusBeetleParam_c sBeetleParam[BEETLE_TYPE_NUM] = {
    {{0.0f, 11.0f, 8.0f}, 5.0f, 1.0f, 3.0f, 1.0f, 0.0f, 10.0f, -10.0f, 1.0f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // oak_silk_moth
    {{0.0f, 10.0f, 8.5f}, 5.0f, 1.0f, 3.0f, 1.0f, 0.0f, 10.0f, -10.0f, 1.0f, 25.0f, 10.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // walking_stick
    {{-2.0f, 10.0f, 13.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // drone_beetle
    {{7.0f, 3.0f, 15.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // scarab_beetle
    {{4.0f, 4.0f, 15.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // jewel_beetle
    {{0.0f, 11.0f, 10.5f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // miyama_stag
    {{0.0f, 11.0f, 10.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // saw_stag
    {{0.0f, 8.0f, 19.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // giant_stag
    {{-4.0f, 12.0f, 13.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // rainbow_stag
    {{0.0f, 11.0f, 4.5f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0}, // cyclommatus_stag
    {{6.0f, 13.0f, 4.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 7.0f, 0.0f, 0}, // golden_stag
    {{0.0f, 11.0f, 2.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0}, // dynastid_beetle
    {{1.0f, 22.0f, 4.5f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0}, // atlas_beetle
    {{1.0f, 22.0f, 5.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0}, // elephant_beetle
    {{-5.0f, 13.0f, 2.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 1.0f, 8.0f, 0.0f, 0}, // hercules_beetle
    {{0.0f, 23.0f, 7.0f}, 5.0f, 1.0f, 5.0f, 3.0f, 0.02f, 10.0f, -10.0f, 0.8f, 10.0f, 5.0f, 2.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0}, // goliath_beetle
};

// 0x2DFC
const dInsMusBeetleParam_c *getBeetleParam(int type) {
    const int types[BEETLE_TYPE_NUM] = {
        INSECT_OAK_SILK_MOTH, INSECT_WALKING_STICK, INSECT_DRONE_BEETLE, INSECT_SCARAB_BEETLE,
        INSECT_JEWEL_BEETLE, INSECT_MIYAMA_STAG, INSECT_SAW_STAG, INSECT_GIANT_STAG,
        INSECT_RAINBOW_STAG, INSECT_CYCLOMMATUS_STAG, INSECT_GOLDEN_STAG, INSECT_DYNASTID_BEETLE,
        INSECT_ATLAS_BEETLE, INSECT_ELEPHANT_BEETLE, INSECT_HERCULES_BEETLE, INSECT_GOLIATH_BEETLE,
    };
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sBeetleParam[i];
        }
    }
    return &sBeetleParam[0];
}

} // namespace dInsectMuseum

// 0x2EF0
dInsMusBeetle_c::dInsMusBeetle_c()
    : mState(NULL), mActive(true), mBaseY(0.0f), mWiggleY(0), mWiggleZ(0), mWiggleUp(false),
      mWiggleRot(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f)), mWaitTime(0), mClimbUpTime(0), mClimbDownTime(0),
      mBrakeTime(0), mWalkTime(0) {}

// 0x2FA8
dInsMusBeetle_c::~dInsMusBeetle_c() {}

// 0x3000
BOOL dInsMusBeetle_c::draw() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    setScale(param->mScale);
    dInsMusBase_c::draw();
    if (isGoldenStag() == true || isHerculesBeetle() == true) {
        mAng angle(0);
        dShadow::requestRound(&mPos, dShadow::DRAW_QUAD, &angle, param->mShadowSize, 1.0f, 0.5f, 0.0f, 1.0f);
    }
    return TRUE;
}

// 0x30B0
void dInsMusBeetle_c::calcMtx() {
    mMtx_c mtx;
    mMtx_c rotMtx;
    mtx = getAxisMtx();
    mtx.m[0][3] = mPos.x;
    mtx.m[1][3] = mPos.y;
    mtx.m[2][3] = mPos.z;
    fn_80444FE4(rotMtx, &mWiggleRot);
    PSMTXConcat(mtx, rotMtx, mtx);
    mMdl.setLocalMtx(&mtx);
}

// 0x3184
void dInsMusBeetle_c::init() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mBaseY = mPos.y;
    mAxis.setIdentity();
    if (!isGoldenStag() && !isHerculesBeetle()) {
        mAxis.rotateY(DEG_TO_ANGLE(180));
        mAxis.rotateX(mAng(-0x38E4));
    } else {
        mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
        mAxis.rotateY(mAngle.y);
    }
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0x328C
void dInsMusBeetle_c::initState() {
    if (isGoldenStag() == true || isHerculesBeetle() == true) {
        initWalk();
    } else {
        initWait();
    }
}

// 0x32E4
void dInsMusBeetle_c::executeState() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    (this->*mState)();
    if (isWalkingStick() == true) {
        mAlpha = 0xFF - (int)(255.0f * mScare.getRate());
    }
}

// 0x335C
void dInsMusBeetle_c::initWait() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    if (isOakSilkMoth() == true || isWalkingStick() == true) {
        setAnmRate(1.0f);
    } else {
        setAnmRate(0.0f);
    }
    setAnmFrame(frame);
    mState = &dInsMusBeetle_c::stateWait;
    int max;
    int min;
    if (isActiveTime() == true) {
        max = dInsectUtil::secToFrame(param->mWaitTimeMax);
        min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    } else {
        max = dInsectUtil::secToFrame(param->mRestWaitTimeMax);
        min = dInsectUtil::secToFrame(param->mRestWaitTimeMin);
    }
    mWaitTime = cM::rndRange<u16>(min, max);
}

// 0x3498
void dInsMusBeetle_c::initClimbUp() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mClimbAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusBeetle_c::stateClimbUp;
    int max;
    int min;
    if (isActiveTime() == true) {
        max = dInsectUtil::secToFrame(param->mClimbTimeMax);
        min = dInsectUtil::secToFrame(param->mClimbTimeMin);
    } else {
        max = dInsectUtil::secToFrame(param->mRestClimbTimeMax);
        min = dInsectUtil::secToFrame(param->mRestClimbTimeMin);
    }
    mClimbUpTime = cM::rndRange<u16>(min, max);
}

// 0x3598
void dInsMusBeetle_c::initClimbDown() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(-param->mClimbAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusBeetle_c::stateClimbDown;
    int max;
    int min;
    if (isActiveTime() == true) {
        max = dInsectUtil::secToFrame(param->mClimbTimeMax);
        min = dInsectUtil::secToFrame(param->mClimbTimeMin);
    } else {
        max = dInsectUtil::secToFrame(param->mRestClimbTimeMax);
        min = dInsectUtil::secToFrame(param->mRestClimbTimeMin);
    }
    mClimbDownTime = cM::rndRange<u16>(min, max);
}

// 0x369C
void dInsMusBeetle_c::stateWait() {
    if (mWaitTime <= mTimer) {
        if (0.5f < cM::rnd()) {
            initClimbUp();
        } else {
            initClimbDown();
        }
    }
}

// 0x36FC
void dInsMusBeetle_c::stateClimbUp() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mPos += mAxis.getFront() * param->mClimbSpeed;
    if (mBaseY + param->mClimbTop <= mPos.y) {
        initWait();
    } else if (mClimbUpTime <= mTimer) {
        initWait();
    }
    if (!isOakSilkMoth() && !isWalkingStick()) {
        wiggle();
    }
}

// 0x37E8
void dInsMusBeetle_c::stateClimbDown() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mPos -= mAxis.getFront() * param->mClimbSpeed;
    if (mPos.y <= mBaseY + param->mClimbBottom) {
        initWait();
    } else if (mClimbDownTime <= mTimer) {
        initWait();
    }
    if (!isOakSilkMoth() && !isWalkingStick()) {
        wiggle();
    }
}

// 0x38D4
bool dInsMusBeetle_c::isWalkingStick() {
    return mType == INSECT_WALKING_STICK;
}

// 0x38E8
bool dInsMusBeetle_c::isOakSilkMoth() {
    return mType == INSECT_OAK_SILK_MOTH;
}

// 0x38FC
bool dInsMusBeetle_c::isGoldenStag() {
    return mType == INSECT_GOLDEN_STAG;
}

// 0x3910
bool dInsMusBeetle_c::isHerculesBeetle() {
    return mType == INSECT_HERCULES_BEETLE;
}

// 0x3924
void dInsMusBeetle_c::wiggle() {
    if (mWiggleUp == true) {
        mWiggleY += WIGGLE_STEP;
        mWiggleZ -= WIGGLE_STEP;
        if (mWiggleY >= WIGGLE_MAX) {
            mWiggleUp = false;
        }
    } else {
        mWiggleY -= WIGGLE_STEP;
        mWiggleZ += WIGGLE_STEP;
        if (mWiggleY <= -WIGGLE_MAX) {
            mWiggleUp = true;
        }
    }
    EGG::Vector3f axisZ(0.0f, 0.0f, 1.0f);
    f32 angleZ = mAng(mWiggleZ).radian();
    EGG::Quatf rotZ(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f));
    rotZ.setAxisRotation(axisZ, angleZ);
    EGG::Vector3f axisY(0.0f, 1.0f, 0.0f);
    f32 angleY = mAng(mWiggleY).radian();
    EGG::Quatf rotY(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f));
    rotY.setAxisRotation(axisY, angleY);
    EGG::Quatf rot = rotZ * rotY;
    EGG::Quatf wiggleRot = rot; // extra copy needed to match
    mWiggleRot = wiggleRot;
}

// 0x3AEC
void dInsMusBeetle_c::initBrake() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 300.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mClimbAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusBeetle_c::stateBrake;
    if (isActiveTime() == true) {
        int max = dInsectUtil::secToFrame(5.0f);
        int min = dInsectUtil::secToFrame(1.0f);
        mBrakeTime = cM::rndRange<u16>(min, max);
    } else {
        int max = dInsectUtil::secToFrame(10.0f);
        int min = dInsectUtil::secToFrame(5.0f);
        mBrakeTime = cM::rndRange<u16>(min, max);
    }
    mAnmRate = param->mClimbAnmRate;
}

// 0x3C10
void dInsMusBeetle_c::initWalk() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mClimbAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusBeetle_c::stateWalk;
    setWalkTarget();
    if (isActiveTime() == true) {
        int max = dInsectUtil::secToFrame(10.0f);
        int min = dInsectUtil::secToFrame(5.0f);
        mWalkTime = cM::rndRange<u16>(min, max);
    } else {
        int max = dInsectUtil::secToFrame(3.0f);
        int min = dInsectUtil::secToFrame(1.0f);
        mWalkTime = cM::rndRange<u16>(min, max);
    }
}

// 0x3D34
void dInsMusBeetle_c::initPose() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrStop, 60.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusBeetle_c::statePose;
}

// 0x3DCC
void dInsMusBeetle_c::stateWalk() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    setAnmRate(param->mClimbAnmRate);
    static const f32 walkSpeed = 0.02f; // unused; its .rodata slot is in the target
    static const f32 walkAccel = 0.002f;
    static const f32 walkBrake = 0.9f;
    static const f32 walkTurnRate = 0.01f;
    flyToTarget(0.0f, walkAccel, walkBrake, walkTurnRate);
    if (mTargetPos.xzDistTo(mPos) <= 0.5f || mTimer % 60 == 59) {
        setWalkTarget();
    } else if (mWalkTime <= mTimer) {
        if (0.1f <= cM::rnd()) {
            initBrake();
        } else {
            initPose();
        }
    }
}

// 0x3ED0
void dInsMusBeetle_c::stateBrake() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mAnmRate *= 0.71f;
    mSpeed *= 0.5f;
    mPos += mSpeed;
    setAnmRate(mAnmRate);
    if (mTimer >= 290 && mTimer <= 340) {
        setAnmRate(0.2f);
    }
    if (mBrakeTime <= mTimer) {
        initWalk();
    }
}

// 0x3FAC
void dInsMusBeetle_c::statePose() {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    if (mTimer > POSE_STILL_TIME) {
        setAnmRate(0.0f);
    }
    if (mTimer > POSE_END_TIME) {
        initBrake();
    }
}

// 0x400C
void dInsMusBeetle_c::setWalkTarget() {
    getWalkTarget(&mTargetPos);
    mPrevTargetPos = mPos;
}

// 0x4054
void dInsMusBeetle_c::getWalkTarget(mVec3_c *target) {
    const dInsMusBeetleParam_c *param = getBeetleParam(mType);
    mAng angle = mAngle.y.mAngle + (s16)cM::rndCenterF(0x2AAB);
    f32 dist = cM::rndRange(30.0f, 60.0f);
    mVec3_c unused(0.0f, 0.0f, 0.0f); // keeps the stack layout
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    dInsectMus_c::sInstance->mAreaMgr.mAreas[STUMP_AREA].clampTarget(&pos, &mPos, &pos);
    *target = pos;
}
