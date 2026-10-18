// The balloon actor (daBalloon_c, profile BALLOON). Included at the end of
// d_a_balloon.cpp.

namespace dABalloon {
BASE_PROFILE(BALLOON, daBalloon_c);
} // namespace dABalloon

// Stripped-function witness: three named floats at .rodata 0x20..0x2C that
// nothing in the REL references (0.1 and 15.0 are also separate pool literals
// later, so they are not shared literals). Only a function the link strips can
// have kept them; its body, and the values' names and meaning, are unknown.
const f32 *dABalloon_retainParams() {
    static const f32 l_unknown[3] = {0.1f, 15.0f, 20.0f};
    return l_unknown;
}

static u8 l_balloonBuf[sizeof(daBalloon_c)];

daBalloon_c *daBalloon_c::s_instance;

// .text:0x6BC
daBalloon_c::daBalloon_c()
    : mState(0), mSwayXAngle(0), mSwayZAngle(0), mBobAngle(0), mFallSpeed(0.0f),
      mPivot(0.0f, 0.0f, 0.0f), mEndX(0.0f), mRaccoIdx(0),
      mLeftNum(BALLOON_MAX), mStringPos(0.0f, 0.0f, 0.0f),
      mStringTargetAngle(0), _474(0) {
    mSpeedF = 0.0f;
    mAccelY = 0.0f;
    mLastPos.set(0.0f, 0.0f, 0.0f);
    mSpeed.set(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < BALLOON_MAX; i++) {
        mBalloonPos[i].set(0.0f, 0.0f, 0.0f);
        mBalloonTarget[i].set(0.0f, 0.0f, 0.0f);
        mHitPos[i].set(0.0f, 0.0f, 0.0f);
        mBalloonAngle[i].set(0, 0, 0);
        mBalloonTargetAngle[i] = 0;
        mLeft[i] = 1;
        mStringAngle[i] = 0;
    }
    mSwayPhase[0] = 0;
    mSwayPhase[1] = 0x1C72;
    mSwayPhase[2] = 0x38E4;
    mGold = dBalloonMng_c::s_instance->mGold;
    if (mGold) {
        mBalloonNum = BALLOON_MAX;
    } else {
        mBalloonNum = 1;
    }
}

// .text:0x994
daBalloon_c::~daBalloon_c() {
}

// .text:0xB04
int daBalloon_c::create() {
    s_instance = this;
    dBalloonMng_c *mng = dBalloonMng_c::s_instance;
    nw4r::g3d::ResFile file(nullptr);
    if (mGold) {
        file =
            nw4r::g3d::ResFile(dSTR::getBrresBank()->getBrres("obj_balloonC"));
    } else {
        file =
            nw4r::g3d::ResFile(dSTR::getBrresBank()->getBrres("obj_balloon"));
    }
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    bool created = true;
    for (int i = 0; i < mBalloonNum; i++) {
        created =
            created & mMdl[i].create(mdl, &mng->mAllocator, 0, 1, nullptr);
    }
    if (mGold) {
        initThree();
    } else {
        mAnmClr.create(mdl, file.GetResAnmClr(0), &mng->mAllocator, nullptr, 1);
        mAnmClr.setFrame(mng->mColor & 7, 0);
        mMdl[0].setAnm(mAnmClr);
        initOne();
    }
    static const GXColor lineColor = {0xA0, 0xA0, 0xC0, 0xFF};
    GXColor color = lineColor;
    mLines.create(dHeap::localHeap_p, BALLOON_MAX);
    mLines.mMatColor = color;
    dFltg::add(this);
    return SUCCEEDED;
}

// .text:0xCE8
int daBalloon_c::execute() {
    mSnd.setPos(&mPos);
    mSnd.holdSound(SE_sky_FUUSEN_fly);
    executeState();
    calcMove();
    calcMtx();
    if (isRouteEnd()) {
        deleteRequest();
    }
    return SUCCEEDED;
}

static inline void requestShadow(const mVec3_c *pos) {
    mAng angle(0);
    dShadow::requestRound(pos, 0, &angle, 16.0f, 1.0f, 1.0f, 0.0f, 0.4f);
}

// .text:0xD5C
int daBalloon_c::draw() {
    if (isFar()) {
        return SUCCEEDED;
    }
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i] && mMdl[i].getScn() != nullptr) {
            mMdl[i].entry();
        }
    }
    requestShadow(&mItemPos);
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i] && mMdl[i].getScn() != nullptr) {
            mLines.addLine(&mBalloonPos[i], &mKnotPos,
                           mColor(0xFF, 0xFF, 0xFF, 0xFF));
            mLines.entry();
        }
    }
    if (mGold) {
        dItem::Item item(dItem::ITEM_IDX_GOLDEN_SLINGSHOT);
        mVec3_c scale(1.0f, 1.0f, 1.0f);
        mAng3_c ang(0, 0, 0);
        dFgobjManager::fgObjCallback(item.mId, (int)&mItemPos, (int)&scale,
                                     &ang, 0);
    } else {
        dItem::Item item(dItem::ITEM_IDX_PRESENT);
        mVec3_c scale(1.0f, 1.0f, 1.0f);
        mAng3_c ang(0, 0, 0);
        dFgobjManager::fgObjCallback(item.mId, (int)&mItemPos, (int)&scale,
                                     &ang, 0);
    }
    return SUCCEEDED;
}

// .text:0xF50
int daBalloon_c::doDelete() {
    dBalloonMng_c *mng = dBalloonMng_c::s_instance;
    m3d::smdl_c *mdl = mMdl;
    for (int i = 0; i < mBalloonNum; i++) {
        mdl->remove();
        mdl++;
    }
    dFltg::remove(this);
    mng->mSent = 0;
    mng->mGold = 0;
    mng->m_frmHeap_p->free(3);
    mLines.destroy();
    return SUCCEEDED;
}

// .text:0xFF8
void *daBalloon_c::operator new(size_t size) {
    if (size > sizeof(l_balloonBuf)) {
        return nullptr;
    }
    memset(l_balloonBuf, 0, sizeof(l_balloonBuf));
    return l_balloonBuf;
}

// .text:0x1044
void daBalloon_c::operator delete(void *mem) {
}

static const daBalloon_c::InitFunc l_initFuncs[daBalloon_c::STATE_NUM] = {
    &daBalloon_c::initWait,  &daBalloon_c::initPop,   &daBalloon_c::initGold3,
    &daBalloon_c::initGold2, &daBalloon_c::initGold1, &daBalloon_c::initGold0,
};

static const daBalloon_c::ExecFunc l_execFuncs[daBalloon_c::STATE_NUM] = {
    &daBalloon_c::executeWait,  &daBalloon_c::executePop,
    &daBalloon_c::executeGold3, &daBalloon_c::executeGold2,
    &daBalloon_c::executeGold1, &daBalloon_c::executeGold0,
};

// .text:0x1048
BOOL daBalloon_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_initFuncs[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x10B4
void daBalloon_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*l_execFuncs[mState])();
    }
}

// .text:0x10F4
BOOL daBalloon_c::initWait() {
    return TRUE;
}

// .text:0x10FC
void daBalloon_c::executeWait() {
    if (mObjc[0].mHitType) {
        setState(STATE_POP);
    }
}

// .text:0x1114
BOOL daBalloon_c::initPop() {
    return TRUE;
}

// .text:0x111C
void daBalloon_c::executePop() {
    mSnd.startSound(SE_sky_FUUSEN_hit);
    mSnd.startSound(SE_sky_FUUSEN_hit_fall);
    mVec3_c *pos = getNodePos(&mMdl[0], "disp_main");
    pos->y += 9.5f;
    mPopEffect.createEffect("af_balloon_ex_a", pos, nullptr, nullptr);
    if (mPopColorEffect.createEffect("af_balloon_ex_b", pos, nullptr,
                                     nullptr)) {
        setPopColor(dBalloonMng_c::s_instance->mColor & 7);
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item item(dItem::ITEM_IDX_PRESENT);
    dFgobjManager::landBalloonItem(item.mId, &mItemPos);
    if (player->getShotBalloonNum() < 8) {
        player->mShotBalloonNum++;
    }
    dPlayerMgr::watchShotBalloon();
    deleteRequest();
}

// .text:0x1234
BOOL daBalloon_c::initGold3() {
    return TRUE;
}

// .text:0x123C
void daBalloon_c::executeGold3() {
    checkPop();
    calcGold3();
    if (mLeftNum == 2) {
        setState(STATE_GOLD2);
    } else if (mLeftNum == 1) {
        setState(STATE_GOLD1);
    } else if (mLeftNum == 0) {
        setState(STATE_GOLD0);
    }
}

// .text:0x12B8
void daBalloon_c::calcGold3() {
    if (mBalloonNum == BALLOON_MAX) {
        mVec3_c up[BALLOON_MAX];
        for (int i = 0; i < mBalloonNum; i++) {
            up[i].set(0.0f, 15.0f, 0.0f);
            up[i].rotZ(mStringAngle[i]);
            mBalloonPos[i] = mKnotPos + up[i];
        }
    }
}

// .text:0x139C
BOOL daBalloon_c::initGold2() {
    int left;
    int right;
    if (getLeftPair(&left, &right)) {
        mVec3_c up[BALLOON_MAX];
        mStringAngle[left] = 0xE39;
        mStringAngle[right] = -0xE39;
        // Separate indices preserve the strength-reduction update order.
        int i;
        int j;
        for (i = 0, j = 0; i < BALLOON_MAX; i++, j++) {
            up[j].set(0.0f, 15.0f, 0.0f);
            up[j].rotZ(mStringAngle[i]);
            mBalloonTarget[i] = mKnotPos + up[j];
        }
        mVec3_c hit[BALLOON_MAX];
        mBalloonTargetAngle[left] = 0xE39;
        mBalloonTargetAngle[right] = -0xE39;
        for (int i = 0; i < BALLOON_MAX; i++) {
            hit[i].set(0.0f, 10.0f, 0.0f);
            hit[i].rotZ(mBalloonTargetAngle[i]);
            mHitTarget[i] = mBalloonTarget[i] + hit[i];
            mHitTarget[i].y = 5.0f + mBalloonTarget[i].y;
        }
    }
    return TRUE;
}

// .text:0x1570
void daBalloon_c::executeGold2() {
    checkPop();
    calcGold2();
    if (mLeftNum == 1) {
        setState(STATE_GOLD1);
    } else if (mLeftNum == 0) {
        setState(STATE_GOLD0);
    }
}

// .text:0x15D4
BOOL daBalloon_c::getLeftPair(int *left, int *right) {
    int idx[2];
    int *p = idx;
    int i;
    int num;
    for (i = 0, num = 0; i < BALLOON_MAX; i++) {
        if (mLeft[i]) {
            *p++ = i;
            num++;
        }
    }
    if (num != 2) {
        return FALSE;
    }
    *left = idx[0];
    *right = idx[1];
    if (mBalloonPos[*left].x > mBalloonPos[*right].x) {
        int tmp = *left;
        *left = *right;
        *right = tmp;
    }
    return TRUE;
}

// .text:0x1688
void daBalloon_c::calcGold2() {
    int left;
    int right;
    if (getLeftPair(&left, &right)) {
        mVec3_c up[BALLOON_MAX];
        for (int i = 0; i < BALLOON_MAX; i++) {
            up[i].set(0.0f, 15.0f, 0.0f);
            up[i].rotZ(mStringAngle[i]);
            mBalloonTarget[i] = mKnotPos + up[i];
        }
        mVec3_c hit[BALLOON_MAX];
        for (int i = 0; i < BALLOON_MAX; i++) {
            hit[i].set(0.0f, 10.0f, 0.0f);
            hit[i].rotZ(mBalloonTargetAngle[i]);
            mHitTarget[i] = mBalloonTarget[i] + hit[i];
            mHitTarget[i].y = 5.0f + mBalloonTarget[i].y;
        }
        sLib::addCalcAngle(&mBalloonAngle[left].z.mAngle,
                           mBalloonTargetAngle[left], 1, 0x50, 0x4B);
        sLib::addCalcAngle(&mBalloonAngle[right].z.mAngle,
                           mBalloonTargetAngle[right], 1, 0x50, 0x4B);
        mBalloonStep[left] = (mBalloonTarget[left] - mBalloonPos[left]) * 0.1f;
        mBalloonStep[right] =
            (mBalloonTarget[right] - mBalloonPos[right]) * 0.1f;
        mBalloonPos[left] += mBalloonStep[left];
        mBalloonPos[right] += mBalloonStep[right];
        mVec3_c hitStep[BALLOON_MAX];
        hitStep[left] = (mHitTarget[left] - mHitPos[left]) * 0.1f;
        hitStep[right] = (mHitTarget[right] - mHitPos[right]) * 0.1f;
        mHitPos[left] += hitStep[left];
        mHitPos[right] += hitStep[right];
    }
}

// .text:0x1A9C
BOOL daBalloon_c::initGold1() {
    int idx;
    if (getLastOne(&idx)) {
        mPivot.set(0.0f, 5.0f, 0.0f);
        s16 &angle = mBalloonAngle[idx].z.mAngle;
        if (angle < 0) {
            angle += 0x2D8;
        } else {
            angle -= 0x2D8;
        }
        mBalloonTargetAngle[idx] = 0;
        if (mBalloonPos[idx].x > mKnotPos.x) {
            f32 half = 0.5f * nw4r::math::FAbs(mBalloonPos[idx].x - mKnotPos.x);
            mStringPos.x = mKnotPos.x + half;
        } else {
            f32 half = 0.5f * nw4r::math::FAbs(mBalloonPos[idx].x - mKnotPos.x);
            mStringPos.x = mBalloonPos[idx].x + half;
        }
        mStringPos.y = mKnotPos.y + 0.5f * (mBalloonPos[idx].y - mKnotPos.y);
        mStringPos.z = mPos.z;
        mVec3_c dir = mBalloonPos[idx] - mKnotPos;
        mStringAngle[idx] = cM::atan2s(dir.y, dir.x) - 0x4000;
        mStringTargetAngle = 0;
        mFallSpeed = -0.8f;
    }
    return TRUE;
}

// .text:0x1C08
void daBalloon_c::executeGold1() {
    checkPop();
    calcGold1();
    if (mLeftNum == 0) {
        setState(STATE_GOLD0);
    }
}

// .text:0x1C54
BOOL daBalloon_c::getLastOne(int *idx) {
    int i;
    int num;
    for (i = 0, num = 0; i < BALLOON_MAX; i++) {
        if (mLeft[i]) {
            *idx = i;
            num++;
        }
    }
    return num == 1;
}

static inline bool isZero(f32 v) {
    return std::fabs(v) <= FLT_EPSILON;
}

// .text:0x1CB0
void daBalloon_c::calcGold1() {
    int idx;
    if (getLastOne(&idx)) {
        sLib::addCalcAngle(&mBalloonAngle[idx].z.mAngle,
                           mBalloonTargetAngle[idx], 1, 0x6E, 0x50);
        sLib::addCalcAngle(&mStringAngle[idx], mStringTargetAngle, 4, 0x44C,
                           0x14);
        mVec3_c up(0.0f, 7.5f, 0.0f);
        up.rotZ(mStringAngle[idx]);
        mVec3_c top = mStringPos + up;
        mVec3_c down(0.0f, -7.5f, 0.0f);
        down.rotZ(mStringAngle[idx]);
        mVec3_c bottom = mStringPos + down;
        mBalloonPos[idx] = top;
        mKnotPos = bottom;
        mItemPos = mKnotPos;
        mItemPos.y = mKnotPos.y - 8.0f;
        if (mFallSpeed > -0.275f) {
            f32 maxY = dFltg::cFlyHeight + dBGCF::getEdgeGroundY(&mPos);
            if (!isZero(mAccelY) || mPos.y > maxY) {
                mFallSpeed = 0.0f;
            } else {
                sLib::chase(&mFallSpeed, 0.07f, 0.005f);
            }
        } else {
            sLib::addCalc0(&mFallSpeed, 0.2f, 0.025f);
        }
        mPos.y += mFallSpeed;
        mStringPos.y += mFallSpeed;
    }
}

// .text:0x1EC0
BOOL daBalloon_c::initGold0() {
    return TRUE;
}

// .text:0x1EC8
void daBalloon_c::executeGold0() {
    mSnd.startSound(SE_sky_FUUSEN_hit_fall);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item item(dItem::ITEM_IDX_GOLDEN_SLINGSHOT);
    bool landed = dFgobjManager::landBalloonItem(item.mId, &mItemPos);
    if (landed) {
        player->setFlag0(PRIVATE_FLAG0_BALLOON_GOLD_PACHINKO);
    }
    dPlayerMgr::watchShotBalloon();
    deleteRequest();
}

// .text:0x1F4C
void daBalloon_c::checkPop() {
    for (int i = 0; i < BALLOON_MAX; i++) {
        if (mLeft[i] && mObjc[i].mHitType) {
            mLeft[i] = 0;
            mLeftNum--;
            mSnd.startSound(SE_sky_FUUSEN_hit);
            mVec3_c *pos = getNodePos(&mMdl[i], "disp_main");
            pos->y += 11.0f;
            mPopEffect.createEffect("af_balloon_ex_a", pos, nullptr, nullptr);
            if (mPopColorEffect.createEffect("af_balloon_ex_b", pos, nullptr,
                                             nullptr)) {
                GXColor color = {0xDC, 0xB4, 0x3C, 0xFF};
                mPopColorEffect.setRegisterColor(color, color, 0);
            }
        }
    }
}

// .text:0x2098
void daBalloon_c::setPopColor(int color) {
    switch (color) {
    case 0: {
        GXColor c = {0xDC, 0x50, 0x46, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 1: {
        GXColor c = {0xFA, 0x96, 0x3C, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 2: {
        GXColor c = {0xFF, 0xE1, 0x50, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 3: {
        GXColor c = {0x78, 0xF0, 0x64, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 4: {
        GXColor c = {0x50, 0xB4, 0xF0, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 5: {
        GXColor c = {0x3C, 0x82, 0xFF, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 6: {
        GXColor c = {0x96, 0x50, 0xFF, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    case 7: {
        GXColor c = {0xF0, 0x64, 0xC8, 0xFF};
        mPopColorEffect.setRegisterColor(c, c, 0);
        break;
    }
    }
}

// .text:0x22F8
mVec3_c *daBalloon_c::getNodePos(m3d::bmdl_c *mdl, const char *node) {
    static mVec3_c pos;
    if (mdl->getScn() != nullptr) {
        nw4r::math::MTX34 mtx;
        int id = m3d::getNodeID(mdl->getResMdl(), node);
        if (id != -1) {
            mdl->getNodeWorldMtx(id, &mtx);
        }
        mVec3_c world;
        PSMTXMultVec(mtx, mVec3_c::Zero, world);
        dWorld::toFieldPosition(&pos, &world);
    }
    return &pos;
}

// .text:0x23C4
void daBalloon_c::initOne() {
    if (cM::rndRange(0, 100) < 50) {
        mPos.x = mBalloonPos[0].x = dFdBase_c::getRaccoParamB();
        mEndX = dFdBase_c::getRaccoParamA();
        mSpeedF = -0.1f;
    } else {
        mPos.x = mBalloonPos[0].x = dFdBase_c::getRaccoParamA();
        mEndX = dFdBase_c::getRaccoParamB();
        mSpeedF = 0.1f;
    }
    mRaccoIdx = dFltg::getOtherRaccoIdx(this);
    mPos.z = mBalloonPos[0].z =
        dFdBase_c::getRaccoDist(dFdBase_c::getOtherRaccoIdx(mRaccoIdx));
    mPos.y = dBGCF::getEdgeGroundY(&mPos);
    mPos.y += dFltg::cFlyHeight;
    mBalloonPos[0].y = mPos.y;
    mLastPos = mPos;
    mKnotPos = mPos;
    mKnotPos.y -= 15.0f;
    mHitPos[0] = mBalloonPos[0];
    mHitPos[0].y += 5.0f;
    mItemPos = mKnotPos;
    mItemPos.y -= 8.0f;
    setState(STATE_WAIT);
}

// .text:0x24EC
void daBalloon_c::initThree() {
    if (cM::rndRange(0, 100) < 50) {
        mPos.x = dFdBase_c::getRaccoParamB();
        mEndX = dFdBase_c::getRaccoParamA();
        mSpeedF = -0.1f;
    } else {
        mPos.x = dFdBase_c::getRaccoParamA();
        mEndX = dFdBase_c::getRaccoParamB();
        mSpeedF = 0.1f;
    }
    mRaccoIdx = dFltg::getOtherRaccoIdx(this);
    mPos.z = dFdBase_c::getRaccoDist(dFdBase_c::getOtherRaccoIdx(mRaccoIdx));
    mPos.y = dBGCF::getEdgeGroundY(&mPos);
    mPos.y += dFltg::cFlyHeight;
    mLastPos = mPos;
    mKnotPos = mPos;
    mKnotPos.y -= 15.0f;
    mVec3_c up[BALLOON_MAX] = {
        mVec3_c(0.0f, 15.0f, 0.0f),
        mVec3_c(0.0f, 15.0f, 0.0f),
        mVec3_c(0.0f, 15.0f, 0.0f),
    };
    mStringAngle[0] = 0x20B6;
    mStringAngle[1] = 0;
    mStringAngle[2] = -0x20B6;
    for (int i = 0; i < BALLOON_MAX; i++) {
        up[i].rotZ(mStringAngle[i]);
        mBalloonPos[i] = mKnotPos + up[i];
    }
    mBalloonAngle[0].z = 0x13E9;
    mBalloonAngle[2].z = -0x13E9;
    mVec3_c hit[BALLOON_MAX];
    for (int i = 0; i < BALLOON_MAX; i++) {
        hit[i].set(0.0f, 10.0f, 0.0f);
        hit[i].rotZ(mBalloonAngle[i].z);
    }
    for (int i = 0; i < BALLOON_MAX; i++) {
        mHitPos[i] = mBalloonPos[i] + hit[i];
        mHitPos[i].y = 3.0f + mBalloonPos[i].y;
    }
    mItemPos = mKnotPos;
    mItemPos.y -= 8.0f;
    setState(STATE_GOLD3);
}

// .text:0x2774
void daBalloon_c::calcMove() {
    mSpeed.set(mSpeedF, mAccelY, 0.0f);
    mLastPos = mPos;
    mPos += mSpeed;
    mStringPos += mSpeed;
    mKnotPos += mSpeed;
    mItemPos += mSpeed;
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i]) {
            mBalloonPos[i] += mSpeed;
            mHitPos[i] += mSpeed;
        }
    }
    calcBob();
    calcGround();
    if (mGold) {
        calcSwayGold();
    } else {
        calcSway();
    }
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i]) {
            mObjc[i].set(this, mHitPos[i], 10.0f, 16.0f, 0x4000, 0x40002000, 0,
                         0);
            mObjc[i].regist();
        }
    }
}

// .text:0x29D8
void daBalloon_c::calcSway() {
    f32 xAmp = l_HIO.mSwayXAmp;
    f32 x = xAmp * mSwayXAngle.sin();
    mSwayXAngle.mAngle += (int)(65536.0f / (60.0f * l_HIO.mSwayXPeriod));
    f32 zAmp = l_HIO.mSwayZAmp;
    f32 z = zAmp * mSwayZAngle.sin();
    mSwayZAngle.mAngle += (int)(65536.0f / (60.0f * l_HIO.mSwayZPeriod));
    mBalloonPos[0].x += x;
    mBalloonPos[0].z += z;
    mKnotPos.x += 0.7f * x;
    mKnotPos.z += 0.7f * z;
    mItemPos.x += 0.7f * x;
    mItemPos.z += 0.7f * z;
}

// .text:0x2B2C
void daBalloon_c::calcSwayGold() {
    f32 amp[BALLOON_MAX];
    f32 sway[BALLOON_MAX];
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i]) {
            amp[i] = l_HIO.mSwayAngleAmp;
            sway[i] = amp[i] * mAng(mSwayXAngle.mAngle + mSwayPhase[i]).sin();
            mStringAngle[i] += mAng::fromDegree(sway[i]);
        }
    }
    f32 x = 0.7f * sway[0];
    mKnotPos.x += x;
    mItemPos.x += x;
    mSwayXAngle.mAngle += (int)(65536.0f / (60.0f * l_HIO.mSwayXPeriod));
}

// .text:0x2C7C
void daBalloon_c::calcBob() {
    f32 amp = l_HIO.mBobAmp;
    f32 y = amp * mBobAngle.sin();
    mBobAngle.mAngle += (int)(65536.0f / (60.0f * l_HIO.mBobPeriod));
    mStringPos.y += y;
    mKnotPos.y += y;
    mItemPos.y += y;
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i]) {
            mBalloonPos[i].y += y;
            mHitPos[i].y += y;
        }
    }
}

// .text:0x2D8C
void daBalloon_c::calcGround() {
    f32 lastGround = dBGCF::getEdgeGroundY(&mLastPos);
    f32 ground = dBGCF::getEdgeGroundY(&mPos);
    mVec3_c ahead = mPos;
    if (mSpeedF < 0.0f) {
        ahead.x -= 128.0f;
    } else {
        ahead.x += 128.0f;
    }
    f32 aheadGround = dBGCF::getEdgeGroundY(&ahead);
    if (ground < lastGround) {
        mAccelY = 0.4375f * -nw4r::math::FAbs(mSpeedF);
    }
    if (ground < aheadGround) {
        mAccelY = 0.4375f * nw4r::math::FAbs(mSpeedF);
    }
    if ((mAccelY < 0.0f && mPos.y - ground < dFltg::cFlyHeight) ||
        (mAccelY > 0.0f && mPos.y - aheadGround > dFltg::cFlyHeight)) {
        mAccelY = 0.0f;
    }
}

// .text:0x2EDC
void daBalloon_c::calcMtx() {
    mMtx_c mtx[BALLOON_MAX];
    for (int i = 0; i < mBalloonNum; i++) {
        if (mLeft[i] && mMdl[i].getScn() != nullptr) {
            makeMtx(&mtx[i], &mBalloonPos[i], mAng(0));
            mMtx_c t1;
            t1.trans(mVec3_c(mPivot.x, -mPivot.y, mPivot.z));
            PSMTXConcat(mtx[i], t1, mtx[i]);
            mtx[i].ZrotM(mBalloonAngle[i].z);
            mMtx_c t2;
            t2.trans(mPivot);
            PSMTXConcat(mtx[i], t2, mtx[i]);
            mMdl[i].setLocalMtx(&mtx[i]);
            mMdl[i].calc(false);
        }
    }
}

// .text:0x3008
bool daBalloon_c::isRouteEnd() {
    if (mSpeedF < 0.0f) {
        return mPos.x < mEndX;
    }
    return mPos.x > mEndX;
}

// .text:0x304C
bool daBalloon_c::isFar() {
    mVec3_c &center = dCamera_c::sCenter;
    if (nw4r::math::FAbs(mPos.x - center.x) < lbl_80750520) {
        if ((mPos.z > center.z &&
             nw4r::math::FAbs(center.z - mPos.z) < 0.5f * lbl_80750520) ||
            (mPos.z <= center.z &&
             nw4r::math::FAbs(mPos.z - center.z) < 2.0f * lbl_80750520)) {
            return false;
        }
    }
    return true;
}
