// dInsMusCicada_c: sings on its trunk; the lantern fly plays its flee animation when scared.
namespace dInsectMuseum {

enum { CICADA_TYPE_NUM = 5 };

dInsMusCicadaParam_c sCicadaParam[CICADA_TYPE_NUM] = {
    {{3.0f, 11.0f, 9.0f}, 1.0f, 0.0f, 60.0f, 400},  // brown_cicada
    {{-3.0f, 20.0f, 6.0f}, 1.0f, 0.0f, 65.0f, 400}, // robust_cicada
    {{-5.0f, 15.0f, 8.0f}, 1.0f, 0.0f, 100.0f, 400}, // walker_cicada
    {{4.0f, 10.0f, 9.0f}, 1.0f, 0.0f, 95.0f, 400},  // evening_cicada
    {{0.0f, 8.0f, 11.0f}, 0.0f, 0.0f, 85.0f, 120},  // lantern_fly
};

// 0x585C
int getCicadaParamIdx(int type) {
    const int types[CICADA_TYPE_NUM] = {
        INSECT_BROWN_CICADA, INSECT_ROBUST_CICADA, INSECT_WALKER_CICADA, INSECT_EVENING_CICADA, INSECT_LANTERN_FLY,
    };
    static const int typeNum = ARRAY_SIZE(types);
    for (int i = 0; i < typeNum; i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x58CC
const dInsMusCicadaParam_c *getCicadaParam(int type) {
    return &sCicadaParam[getCicadaParamIdx(type)];
}

// 0x58FC
int getCicadaSound(int type) {
    const int sounds[CICADA_TYPE_NUM] = {
        SE_aburazemi, SE_minminzemi, SE_tsukutsukuhoushi, SE_higurashi, -1,
    };
    return sounds[getCicadaParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x5954
dInsMusCicada_c::dInsMusCicada_c() : mState(NULL) {}

// 0x59AC
dInsMusCicada_c::~dInsMusCicada_c() {}

// 0x5A04
void dInsMusCicada_c::calcMtx() {
    setAxisMtx();
}

// 0x5A08
void dInsMusCicada_c::init() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y += param->mOffset.y;
    mPos.z += param->mOffset.z;
    mAxis.setIdentity();
    mAxis.rotateY(DEG_TO_ANGLE(180));
    mAxis.rotateX(mAng(-0x2AAB));
    mScareRadius = param->mScareRadius;
    mShadowSize = param->mShadowSize;
    mScare.setLimit(param->mScareLimit);
    mBgRadius = param->mShadowSize;
}

// 0x5AC8
void dInsMusCicada_c::initState() {
    initWait();
}

// 0x5ACC
void dInsMusCicada_c::executeState() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    (this->*mState)();
}

// 0x5B0C
void dInsMusCicada_c::initWait() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    mTimer = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(frame);
    mState = &dInsMusCicada_c::stateWait;
}

// 0x5BC0
void dInsMusCicada_c::initFlee() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(0.0f);
    mState = &dInsMusCicada_c::stateFlee;
}

// 0x5C58
void dInsMusCicada_c::initReturn() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(RETURN_START_FRAME);
    mState = &dInsMusCicada_c::stateReturn;
}

// 0x5CF0
void dInsMusCicada_c::stateWait() {
    const dInsMusCicadaParam_c *param = getCicadaParam(mType);
    if (!mScare.isScared()) {
        if (isLanternFly() == true) {
            setAnmRate(0.0f);
        } else if (mSnd.isSounding() == TRUE) {
            setAnmRate(param->mAnmRate);
        } else {
            setAnmRate(0.0f);
        }
        if (isActiveTime() == true) {
            int sound = getCicadaSound(mType);
            if (sound > 0) {
                mSnd.holdSound(sound);
            }
        }
    } else {
        if (isLanternFly() == true) {
            initFlee();
        } else {
            setAnmRate(0.0f);
        }
    }
}

// 0x5DEC
void dInsMusCicada_c::stateFlee() {
    if (FLEE_END_FRAME <= mAnmChr.getFrame()) {
        setAnmFrame(FLEE_END_FRAME);
        if (!mScare.isScared()) {
            initReturn();
        }
    }
}

// 0x5E54
void dInsMusCicada_c::stateReturn() {
    if (RETURN_END_FRAME <= mAnmChr.getFrame()) {
        setAnmFrame(RETURN_END_FRAME);
        initWait();
    }
}

// 0x5EAC
bool dInsMusCicada_c::isLanternFly() {
    return mType == INSECT_LANTERN_FLY;
}
