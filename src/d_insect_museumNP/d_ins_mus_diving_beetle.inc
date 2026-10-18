// dInsMusDivingBeetle_c: the diving beetle of the museum's pond.

namespace dInsectMuseum {

enum { DIVING_BEETLE_TYPE_NUM = 1 };

dInsMusDivingBeetleParam_c sDivingBeetleParam[DIVING_BEETLE_TYPE_NUM] = {
    {{32.0f, -2.5f, 0.0f}, 3.0f, 0.0f, 120.0f, 40.0f, 0.985f, 1.5f, 0.5f, 0.0f, 7},
};

// 0x136AC
int getDivingBeetleParamIdx(int type) {
    const int types[] = {INSECT_DIVING_BEETLE};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x136CC
const dInsMusDivingBeetleParam_c *getDivingBeetleParam(int type) {
    return &sDivingBeetleParam[getDivingBeetleParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x136FC
dInsMusDivingBeetle_c::dInsMusDivingBeetle_c() : mState(NULL), mWaitTime(0), mSwimTime(0) {}

// 0x13760
dInsMusDivingBeetle_c::~dInsMusDivingBeetle_c() {}

// 0x137B8
void dInsMusDivingBeetle_c::init() {
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    mShadowSize = param->mShadowSize;
    mBgRadius = param->mShadowSize;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
}

// 0x13838
void dInsMusDivingBeetle_c::initState() {
    initWait();
}

// 0x1383C
void dInsMusDivingBeetle_c::executeState() {
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    mCheckBg = false;
    (this->*mState)();
}

// 0x13884
void dInsMusDivingBeetle_c::initWait() {
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.1f);
    setAnmFrame(frame);
    mState = &dInsMusDivingBeetle_c::stateWait;
    int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(min, max);
    if (!isActiveTime()) {
        mWaitTime *= 5;
    }
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

// 0x13994
void dInsMusDivingBeetle_c::initSwim() {
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(frame);
    mState = &dInsMusDivingBeetle_c::stateSwim;
    getSwimTarget(&mTargetPos);
    mPrevTargetPos = mPos;
    mSwimTime = SWIM_TIME;
    mAngle.y = cLib::targetAngleY(mPrevTargetPos, mTargetPos);
    f32 dist = mTargetPos.xzDistTo(mPrevTargetPos);
    mSpeed.set(0.0f, 0.0f, dist / (param->mSwimBrake / (1.0f - param->mSwimBrake)));
    mSpeed.rotY(mAngle.y);
}

// 0x13AF8
void dInsMusDivingBeetle_c::stateWait() {
    if (mWaitTime <= mTimer) {
        initSwim();
    }
}

// 0x13B10
void dInsMusDivingBeetle_c::stateSwim() {
    swim();
    if (mSwimTime <= mTimer) {
        initWait();
    }
}

// 0x13B54
void dInsMusDivingBeetle_c::swim() {
    f32 t = (f32)mTimer / (f32)mSwimTime;
    f32 ease = cLib::easeOut(t > 1.0f ? 1.0f : t, 2.0f);
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    mSpeed.x *= param->mSwimBrake;
    mSpeed.z *= param->mSwimBrake;
    mPos += mSpeed;
    f32 anmRate = param->mSwimAnmRateStart - ease * (param->mSwimAnmRateStart - param->mSwimAnmRateEnd);
    setAnmRate(anmRate);
}

// 0x13C58
mAng dInsMusDivingBeetle_c::getSwimAngle() {
    return mAngle.y.mAngle + (s16)cM::rndCenterF(0x2AAB);
}

// 0x13CA4
f32 dInsMusDivingBeetle_c::getSwimDist() const {
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    return cM::rndRange(param->mSwimDistMin, param->mSwimDistMax);
}

// 0x13CD4
void dInsMusDivingBeetle_c::getSwimTarget(mVec3_c *target) {
    mAng angle = getSwimAngle();
    f32 dist = getSwimDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    const dInsMusDivingBeetleParam_c *param = getDivingBeetleParam(mType);
    dInsectMus_c::sInstance->mAreaMgr.mAreas[param->mArea].clampTarget(&pos, &mPos, &pos);
    result = pos;
    *target = result;
}
