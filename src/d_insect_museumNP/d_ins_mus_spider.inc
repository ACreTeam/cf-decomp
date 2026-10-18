// dInsMusSpider_c: the bagworm and the spider of the museum's insect rooms, dropping down on their thread.

namespace dInsectMuseum {

enum { SPIDER_TYPE_NUM = 2 };

dInsMusSpiderParam_c sSpiderParam[SPIDER_TYPE_NUM] = {
    {{23.0f, 30.0f, 0.0f}, 10.0f, 8.0f, 10.0f, 8.0f, 1.0f, 0.0f, 64.0f}, // bagworm
    {{-27.5f, 32.0f, 0.0f}, 10.0f, 8.0f, 10.0f, 8.0f, 1.0f, 0.0f, 64.0f}, // spider
};

// 0x11A78
int getSpiderParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_BAGWORM:
        idx = 0;
        break;
    case INSECT_SPIDER:
        idx = 1;
        break;
    }
    return idx;
}

// 0x11AA8
const dInsMusSpiderParam_c *getSpiderParam(int type) {
    return &sSpiderParam[getSpiderParamIdx(type)];
}

} // namespace dInsectMuseum

// 0x11AD8
dInsMusSpider_c::dInsMusSpider_c() : mState(NULL), mHangTime(0), mWaitTime(0) {}

// 0x11B3C
dInsMusSpider_c::~dInsMusSpider_c() {}

// 0x11B94
BOOL dInsMusSpider_c::draw() {
    if (mAnmChr.getFrame() <= 1.0f) {
        return TRUE;
    }
    dInsMusBase_c::draw();
    return TRUE;
}

// 0x11BEC
void dInsMusSpider_c::init() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    mPos.x += param->mOffset.x;
    mPos.y += param->mOffset.y;
    mPos.z += param->mOffset.z;
    mScareRadius = param->mNearRadius;
    mShadowSize = param->mShadowSize;
    mBgRadius = param->mShadowSize;
}

// 0x11C64
void dInsMusSpider_c::initState() {
    initWait();
}

// 0x11C68
void dInsMusSpider_c::executeState() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    (this->*mState)();
}

// 0x11CA8
void dInsMusSpider_c::initWait() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    mTimer = 0;
    mState = &dInsMusSpider_c::stateWait;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mAngle.set(0, 0, 0);
}

// 0x11D50
void dInsMusSpider_c::initDescend() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    mTimer = 0;
    mState = &dInsMusSpider_c::stateDescend;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(0.0f);
    mAngle.set(0, 0, 0);
}

// 0x11E04
void dInsMusSpider_c::initHang() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    mTimer = 0;
    mState = &dInsMusSpider_c::stateHang;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(HANG_START_FRAME);
    int max = dInsectUtil::secToFrame(param->mHangTimeMax);
    int min = dInsectUtil::secToFrame(param->mHangTimeMin);
    mHangTime = cM::rndRange<u16>(min, max);
    mAngle.set(0, 0, 0);
}

// 0x11ED8
void dInsMusSpider_c::initAscend() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    mTimer = 0;
    mState = &dInsMusSpider_c::stateAscend;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(ASCEND_START_FRAME);
    mAngle.set(0, 0, 0);
}

// 0x11F88
void dInsMusSpider_c::stateWait() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    if (mPlayerCheck.isNear(dPlayerMgr::PLAYER_INDEX_CURRENT, param->mNearRadius) == true) {
        if (mWaitTime < mTimer) {
            initDescend();
        }
    } else {
        mTimer = 0;
    }
}

// 0x11FF8
void dInsMusSpider_c::stateDescend() {
    if (DESCEND_END_FRAME <= mAnmChr.getFrame()) {
        initHang();
    }
}

// 0x12044
void dInsMusSpider_c::stateHang() {
    f32 frame = mAnmChr.getFrame();
    if (HANG_END_FRAME <= frame) {
        setAnmFrame(frame - (HANG_END_FRAME - HANG_START_FRAME));
    }
    if (mHangTime <= mTimer) {
        initAscend();
    }
}

// 0x120B4
void dInsMusSpider_c::stateAscend() {
    const dInsMusSpiderParam_c *param = getSpiderParam(mType);
    if (ASCEND_END_FRAME <= mAnmChr.getFrame()) {
        int max = dInsectUtil::secToFrame(param->mWaitTimeMax);
        int min = dInsectUtil::secToFrame(param->mWaitTimeMin);
        mWaitTime = cM::rndRange<u16>(min, max);
        initWait();
    }
}
