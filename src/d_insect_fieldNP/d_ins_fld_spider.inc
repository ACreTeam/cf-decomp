// dInsFldSpider_c: the spider and the bagworm. See
// include/game/game/d_ins_fld_spider.hpp.

const dInsFldSpider_c::StateFunc dInsFldSpider_c::sExecute[] = {
    &dInsFldSpider_c::executeHide, &dInsFldSpider_c::executeDrop,
    &dInsFldSpider_c::executeHang, &dInsFldSpider_c::executeClimb,
    &dInsFldSpider_c::executeFall, &dInsFldSpider_c::executeRelease,
};

const dInsFldSpider_c::StateFunc dInsFldSpider_c::sInit[] = {
    &dInsFldSpider_c::initHide, &dInsFldSpider_c::initDrop,
    &dInsFldSpider_c::initHang, &dInsFldSpider_c::initClimb,
    &dInsFldSpider_c::initFall, &dInsFldSpider_c::onRelease,
};

dInsFldSpider_c::param_c dInsFldSpider_c::sParam[] = {
    {23.0f, 40.0f, 0.0f, 40, 10.0f, 8.0f, 1.0f, -0.08f, 0.7f, 2.0f, 0.0f, 4.0f},
    {23.0f, 40.0f, 0.0f, 40, 10.0f, 8.0f, 1.0f, -0.15f, 0.2f, 1.0f, 0.0f, 4.0f},
};

// 0x215F4
int dInsFldSpider_c::getParamIdx(int type) {
    const int types[] = {INSECT_SPIDER, INSECT_BAGWORM};
    for (int i = 0; i < ARRAY_SIZE(types); i++) {
        if (type == types[i]) {
            return i;
        }
    }
    return 0;
}

// 0x21628
const dInsFldSpider_c::param_c *dInsFldSpider_c::getSpiderParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x21658
bool dInsFldSpider_c::isHangTree(const dItem::Item *item, const mVec3_c *pos) {
    dItem::FgInfo *info = item->getFgInfo();
    if (info == NULL) {
        return false;
    }
    if (info->mTreeStage > 3 && !item->isPalmTree()) {
        switch (item->mId) {
        case dItem::FG_TREE_FTR:
        case dItem::FG_TREE_BEES:
        case dItem::FG_TREE_BELLS:
        case dItem::FG_MONEY_TREE:
        case dItem::FG_CEDAR_FTR:
        case dItem::FG_CEDAR_BEES:
        case dItem::FG_CEDAR_BELLS:
        case dItem::FG_CEDAR_LIGHTS:
            break;
        default:
            return true;
        }
    }
    return false;
}

// 0x21704
dInsFldSpider_c::dInsFldSpider_c()
    : mState(STATE_HIDE), mSearch(isHangTree), mHangPos(0.0f, 0.0f, 0.0f),
      mShaken(false), mHangFrames(0) {
}

// 0x2177C
dInsFldSpider_c::~dInsFldSpider_c() {
}

// 0x217E4
int dInsFldSpider_c::execute() {
    mSearch.search(mInitPos, 1.0f);
    dInsFldBase_c::execute();
    return TRUE;
}

// 0x2182C
int dInsFldSpider_c::draw() {
    if (mState != STATE_HIDE) {
        dInsFldBase_c::draw();
    }
    return TRUE;
}

// 0x2185C
void dInsFldSpider_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x21894
void dInsFldSpider_c::setGroundPos() {
    const param_c *param = getSpiderParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
    mHangPos = mPos;
}

// 0x21914
void dInsFldSpider_c::initState() {
    setState(STATE_HIDE);
}

// 0x2191C
void dInsFldSpider_c::executeState() {
    getSpiderParam(mType);
    (this->*sExecute[mState])();
    if (!isHeld() && mState != STATE_FALL && mState != STATE_RELEASE &&
        !isOnTree()) {
        setState(STATE_FALL);
    }
}

// 0x219AC
void dInsFldSpider_c::initHide() {
    getSpiderParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.0f);
    setAnmFrame(0.0f);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mShaken = false;
}

// 0x21A3C
void dInsFldSpider_c::initDrop() {
    const param_c *param = getSpiderParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(0.0f);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    f32 x = mInitPos.x + getDropOffset();
    mHangPos.y = mPos.y;
    mPos.x = x;
    mHangPos.x = x;
    mHangPos.z = mPos.z;
}

// 0x21AFC
void dInsFldSpider_c::initHang() {
    const param_c *param = getSpiderParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(HANG_START_FRAME);
    int max = dInsectUtil::secToFrame(param->mHangTimeMax);
    int min = dInsectUtil::secToFrame(param->mHangTimeMin);
    mHangFrames = cM::rndRange<u16>(min, max);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
}

// 0x21BB4
void dInsFldSpider_c::initClimb() {
    const param_c *param = getSpiderParam(mType);
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(param->mAnmRate);
    setAnmFrame(ASCEND_START_FRAME);
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
}

// 0x21C48
void dInsFldSpider_c::initFall() {
    getSpiderParam(mType);
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrStop, 0.0f);
    setAnmVis(mAnmVisStop);
    setAnmFrame(frame);
    setAnmRate(0.0f);
}

// 0x21CDC
void dInsFldSpider_c::onRelease() {
    const param_c *param = getSpiderParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
    if (!isSpider()) {
        mVelocity.set(0.0f, 0.0f, param->mReleaseSpeed);
        mVelocity.rotY(mAngle.y);
    }
    mSound.startSound(SE_mushi_out_fall);
    mShadowSize = 5.0f;
}

// 0x21DB8
void dInsFldSpider_c::executeHide() {
    const param_c *param = getSpiderParam(mType);
    if (!mShaken) {
        mFrame = 0;
    } else if (param->mShakeFrames <= mFrame) {
        setState(STATE_DROP);
    }
    mVec3_c pos = mInitPos;
    bool shaken =
        mPlayerCheck.isShaking(pos, dPlayerMgr::PLAYER_INDEX_CURRENT) == true;
    if (!shaken) {
        mVec3_c pos2 = mInitPos;
        shaken = mPlayerCheck.isShakingNear(
                     pos2, dPlayerMgr::PLAYER_INDEX_CURRENT) == true;
    }
    if (shaken) {
        mShaken = true;
    }
}

// 0x21E98
void dInsFldSpider_c::executeDrop() {
    getSpiderParam(mType);
    f32 frame = mAnmChr.getFrame();
    mHangPos.y += -1.25f;
    mHangPos.y = nw4r::ut::Max(mHangPos.y, mPos.y - 25.0f);
    registObjc(&mHangPos, mBaseRadius, TRUE);
    if (DESCEND_END_FRAME <= frame) {
        setState(STATE_HANG);
    }
}

// 0x21F54
void dInsFldSpider_c::executeHang() {
    getSpiderParam(mType);
    f32 frame = mAnmChr.getFrame();
    if (HANG_END_FRAME <= frame) {
        setAnmFrame(frame - (HANG_END_FRAME - HANG_START_FRAME));
    }
    registObjc(&mHangPos, mBaseRadius, TRUE);
    if (mHangFrames <= mFrame) {
        setState(STATE_CLIMB);
    }
}

// 0x21FE4
void dInsFldSpider_c::executeClimb() {
    f32 frame = mAnmChr.getFrame();
    if (mFrame > CLIMB_WAIT_TIME) {
        mHangPos.y += 0.89285713f;
    }
    mHangPos.y = nw4r::ut::Min(mHangPos.y, mPos.y);
    registObjc(&mHangPos, mBaseRadius, TRUE);
    if (ASCEND_END_FRAME <= frame) {
        setState(STATE_HIDE);
    }
}

// 0x22098
void dInsFldSpider_c::executeFall() {
    int alpha = mAlpha - FALL_FADE_SPEED;
    mAlpha = alpha < 0 ? 0 : alpha;
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x220BC
void dInsFldSpider_c::executeRelease() {
    if (isSpider() == true) {
        executeReleaseSpider();
    } else {
        executeReleaseBagworm();
    }
}

// 0x22104
f32 dInsFldSpider_c::getDropOffset() {
    const param_c *param = getSpiderParam(mType);
    if (mPlayerCheck.isValid(dPlayerMgr::PLAYER_INDEX_CURRENT) == true) {
        mVec3_c player(0.0f, 0.0f, 0.0f);
        mPlayerCheck.getPlayerPos(&player, dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player.x - mInitPos.x > 0.0f) {
            return -param->mOffsetX;
        }
    }
    return param->mOffsetX;
}

// 0x221A8
bool dInsFldSpider_c::isOnTree() {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, mInitPos);
    dFdBlock_c *block =
        dField::getInfo(FD_ID_CURRENT)->getBlock(blockX, blockZ);
    if (block == NULL) {
        return false;
    }
    dItem::Item *item = block->getItem(unitX, unitZ, 0);
    if (item == NULL) {
        return false;
    }
    if (isHangTree(item, &mInitPos) == true) {
        return true;
    }
    return false;
}

// 0x22254
void dInsFldSpider_c::executeReleaseSpider() {
    const param_c *param = getSpiderParam(mType);
    mVelocity.y += param->mGravity;
    mVelocity.set(0.0f, mVelocity.y, param->mReleaseSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mVelocity.y = 0.0f;
        mPos.y = getGroundY();
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x223C0
void dInsFldSpider_c::executeReleaseBagworm() {
    const param_c *param = getSpiderParam(mType);
    mVelocity.y += param->mGravity;
    mPos += mVelocity;
    fadeOut();
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 1.2f;
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        dEffect::create("af_hny_watercolumn_S", &mPos, NULL, NULL);
        dEffect::create("af_hny_mizutama_S", &mPos, NULL, NULL,
                        checkPtclLanding, 0);
        mActive = false;
        requestDelete();
    }
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x224F8
bool dInsFldSpider_c::isSpider() {
    return mType == INSECT_SPIDER;
}
