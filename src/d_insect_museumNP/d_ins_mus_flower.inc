// dInsMusFlower_c: the mantises rear up at a nearby player; the snail and the ladybug walk around their flower.
namespace dInsectMuseum {

enum { FLOWER_TYPE_NUM = 4 };

f32 sFlowerTurnAnmRate = 2.0f;

dInsMusFlowerParam_c sFlowerParam[FLOWER_TYPE_NUM] = {
    {{0.0f, 12.0f, 0.0f}, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 1.0f, 1.0f, 5.0f, 60.0f, 200},     // mantis
    {{5.0f, 12.0f, -4.0f}, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 1.0f, 1.0f, 4.0f, 120.0f, 200},   // orchid mantis
    {{-1.8f, 10.0f, 2.4f}, 3.0f, 1.0f, 2.5f, 2.5f, 0.005f, 0.02f, -0.007f, 5.0f, 1.0f, 1.0f, 5.0f, 60.0f, 200}, // snail
    {{2.0f, 12.0f, 3.0f}, 3.0f, 1.0f, 2.5f, 2.5f, 0.01f, 0.3f, -0.007f, 5.0f, 1.0f, 0.4f, 3.0f, 80.0f, 100},    // ladybug
};

// 0x8284
const dInsMusFlowerParam_c *getFlowerParam(int type) {
    const int types[FLOWER_TYPE_NUM] = {INSECT_MANTIS, INSECT_ORCHID_MANTIS, INSECT_SNAIL, INSECT_LADYBUG};
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return &sFlowerParam[i];
        }
    }
    return &sFlowerParam[0];
}

} // namespace dInsectMuseum

// 0x82F8
dInsMusFlower_c::dInsMusFlower_c()
    : mState(NULL), mUnitCenter(0.0f, 0.0f, 0.0f), mRestTime(0), mWaitTime(0), mWalkTime(0), _22E(150), _230(182),
      mTargetAngle(0) {}

// 0x8388
dInsMusFlower_c::~dInsMusFlower_c() {}

// 0x83E0
BOOL dInsMusFlower_c::draw() {
    dInsMusBase_c::draw();
    mTargetPos.y = mPos.y;
    return TRUE;
}

// 0x8418
void dInsMusFlower_c::calcMtx() {
    setAxisMtx();
}

// 0x841C
void dInsMusFlower_c::init() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mPos.x = mHomePos.x + param->mOffset.x;
    mPos.y = mHomePos.y + param->mOffset.y;
    mPos.z = mHomePos.z + param->mOffset.z;
    dFdBase_c::snapToUnitCenter(mUnitCenter, mHomePos);
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = 5.0f;
    mAngle.y = cM::rndRange<s16>(-0x7F4A, 0x7F4A);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
}

// 0x84E8
void dInsMusFlower_c::initState() {
    if (isMantis() == true) {
        initRest();
    } else if (isSnail() == true) {
        if (isActiveTime() == true) {
            initWait();
        } else {
            initRest();
            setAnmFrame(SNAIL_HIDE_END_FRAME);
        }
    } else {
        initWait();
    }
}

// 0x8578
void dInsMusFlower_c::executeState() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    (this->*mState)();
}

// 0x85B8
void dInsMusFlower_c::initRest() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mTimer = 0;
    if (isSnail() == true) {
        setAnmChr(mAnmChrStop, 0.0f);
        setAnmVis(mAnmVisStop);
    } else {
        setAnmChr(mAnmChrWait, 60.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusFlower_c::stateRest;
    mRestTime = 120;
}

// 0x8688
void dInsMusFlower_c::initWait() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(frame);
    mState = &dInsMusFlower_c::stateWait;
    int waitMax = dInsectUtil::secToFrame(param->mWaitTimeMax);
    int waitMin = dInsectUtil::secToFrame(param->mWaitTimeMin);
    mWaitTime = cM::rndRange<u16>(waitMin, waitMax);
    if (!isActiveTime()) {
        mWaitTime *= 5;
    }
    if (isLadybug() == true) {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(DEG_TO_ANGLE(45));
        mAxis.setUp(up);
    } else if (isSnail() == true) {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(0x1C72);
        up.rotY(-DEG_TO_ANGLE(45));
        mAxis.setUp(up);
    } else {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(0xE39);
        mAxis.setUp(up);
    }
}

// 0x8878
void dInsMusFlower_c::initWalk() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mWalkAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusFlower_c::stateWalk;
    getWalkTarget(&mTargetPos);
    mWalkTime = (int)(mTargetPos.xzDistTo(mPos) / param->mWalkSpeed) + 1;
}

// 0x897C
void dInsMusFlower_c::initTurn() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mTimer = 0;
    if (isSnail() == true) {
        setAnmChr(mAnmChrStop, 0.0f);
        setAnmVis(mAnmVisStop);
    } else {
        setAnmChr(mAnmChrWait, 0.0f);
        setAnmVis(mAnmVisWait);
    }
    setAnmRate(sFlowerTurnAnmRate);
    setAnmFrame(0.0f);
    mState = &dInsMusFlower_c::stateTurn;
    mTargetAngle = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
}

// 0x8A58
void dInsMusFlower_c::initThreat() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusFlower_c::stateThreat;
}

// 0x8AF0
void dInsMusFlower_c::stateRest() {
    f32 frame = mAnmChr.getFrame();
    if (isMantis() == true) {
        if (!isActiveTime()) {
            if (isAnmStopped() == true) {
                if (mTimer == MANTIS_PAUSE_TIME) {
                    setAnmRate(1.0f);
                    mTimer = 0;
                }
            } else if (mTimer == MANTIS_SWAY_TIME) {
                setAnmRate(0.0f);
                mTimer = 0;
            }
        }
        if (mScare.isScared() == true) {
            initTurn();
        }
    } else {
        if (isActiveTime() == true) {
            if (!mScare.isScared()) {
                setAnmRate(-0.25f);
                if (frame <= 1.0f) {
                    initWait();
                }
            } else {
                setAnmRate(1.0f);
            }
        }
        if (SNAIL_HIDE_END_FRAME < frame) {
            setAnmRate(0.0f);
            setAnmFrame(SNAIL_HIDE_END_FRAME);
        }
    }
}

// 0x8C64
void dInsMusFlower_c::stateWait() {
    if (mWaitTime < mTimer) {
        initWalk();
    } else if (isSnail() == true && mScare.isScared() == true) {
        initRest();
    }
}

// 0x8CC8
void dInsMusFlower_c::stateWalk() {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    if (isLadybug() == true) {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(DEG_TO_ANGLE(45));
        mAxis.setUp(up);
    } else if (isSnail() == true) {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(0x1C72);
        up.rotY(-DEG_TO_ANGLE(45));
        mAxis.setUp(up);
    } else {
        mVec3_c up(0.0f, 1.0f, 0.0f);
        up.rotX(0xE39);
        mAxis.setUp(up);
    }
    mVec3_c front = mAxis.getFront();
    mVec3_c dir = mTargetPos - mPos;
    dir.y = 0.0f;
    dir.normalize();
    mVec3_c turn = dir - front;
    static const f32 turnRate = 0.02f;
    front += turn * turnRate;
    front.normalize();
    mAxis.setFront(front);
    mVec3_c move = front;
    move *= param->mWalkSpeed;
    mPos += move;
    if (mWalkTime < mTimer) {
        if (0.5f < cM::rnd()) {
            initWalk();
        } else {
            initWait();
        }
    } else if (isSnail() == true && mScare.isScared() == true) {
        initRest();
    }
}

// 0x8F30
void dInsMusFlower_c::stateTurn() {
    setAnmRate(sFlowerTurnAnmRate);
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    s16 step = mAng::fromDegree(param->mTurnSpeed);
    s16 rest = sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle.mAngle, 1, step, 0);
    mAxis.setIdentity();
    mAxis.rotateY(mAngle.y);
    if (rest == 0) {
        initThreat();
    } else if (!mScare.isScared()) {
        setAnmFrame(0.0f);
        initRest();
    }
}

// 0x9004
void dInsMusFlower_c::stateThreat() {
    if (mScare.isScared() == true) {
        const dInsMusFlowerParam_c *param = getFlowerParam(mType);
        if ((u16)dInsectUtil::secToFrame(param->mThreatTime) <= mTimer) {
            mAng angle = mPlayerCheck.getAngleTo(dPlayerMgr::PLAYER_INDEX_CURRENT);
            mAng diff = angle - mAngle.y;
            s16 absDiff = __fabsf(diff.mAngle);
            if (absDiff > 0xE39) {
                setAnmRate(-1.0f);
                if (mAnmChr.getFrame() <= 0.0f) {
                    initTurn();
                }
            }
        }
    } else if (isMantis() == true) {
        initRest();
    } else {
        initWalk();
    }
}

// 0x9114
bool dInsMusFlower_c::isMantis() {
    if (mType == INSECT_MANTIS) {
        return true;
    }
    return mType == INSECT_ORCHID_MANTIS;
}

// 0x9138
bool dInsMusFlower_c::isSnail() {
    return mType == INSECT_SNAIL;
}

// 0x914C
bool dInsMusFlower_c::isLadybug() {
    return mType == INSECT_LADYBUG;
}

// 0x9160
mAng dInsMusFlower_c::getWalkAngle() {
    mAng range(DEG_TO_ANGLE(120));
    s16 angle = mAngle.y.mAngle;
    return angle + (s16)cM::rndCenterF(range);
}

// 0x91C4
f32 dInsMusFlower_c::getWalkDist() const {
    const dInsMusFlowerParam_c *param = getFlowerParam(mType);
    return cM::rndRange<f32>(param->mWalkDistMin, param->mWalkDistMax);
}

// 0x91F4
void dInsMusFlower_c::getWalkTarget(mVec3_c *target) {
    mAng angle = getWalkAngle();
    f32 dist = getWalkDist();
    mVec3_c result(0.0f, 0.0f, 0.0f);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    if (isLadybug() == true) {
        dInsectMus_c::sInstance->mAreaMgr.mAreas[AREA_LADYBUG].clampTarget(&pos, &mPos, &pos);
        result = pos;
    } else if (isSnail() == true) {
        dInsectMus_c::sInstance->mAreaMgr.mAreas[AREA_SNAIL].clampTarget(&pos, &mPos, &pos);
        result = pos;
    }
    *target = result;
}
