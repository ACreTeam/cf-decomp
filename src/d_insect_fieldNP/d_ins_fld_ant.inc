// dInsFldAnt_c: the ant. See include/game/game/d_ins_fld_ant.hpp.

const dInsFldAnt_c::StateFunc dInsFldAnt_c::sExecute[] = {
    &dInsFldAnt_c::executeWait, &dInsFldAnt_c::executeFade,
    &dInsFldAnt_c::executeRelease};

const dInsFldAnt_c::StateFunc dInsFldAnt_c::sInit[] = {
    &dInsFldAnt_c::initWait, &dInsFldAnt_c::initFade, &dInsFldAnt_c::onRelease};

const int dInsFldAnt_c::sTypes[] = {INSECT_ANT};

dInsFldAnt_c::param_c dInsFldAnt_c::sParam[] = {
    {0.0f, 0.0f, 0.0f, -0.15f, 0.3f, 1.0f, 0.0f, 6.0f, 15},
};

int dInsFldAnt_c::sAntNum;

// 0x17864
int dInsFldAnt_c::getParamIdx(int type) {
    for (int i = 0; i < ARRAY_SIZE(sTypes); i++) {
        if (type == sTypes[i]) {
            return i;
        }
    }
    return 0;
}

// 0x17884
const dInsFldAnt_c::param_c *dInsFldAnt_c::getAntParam(int type) {
    return &sParam[getParamIdx(type)];
}

// 0x178B4
dInsFldAnt_c::dInsFldAnt_c() : mState(STATE_WAIT), mAnmTexSrtCreated(false) {
}

// 0x17934
dInsFldAnt_c::~dInsFldAnt_c() {
}

// 0x1799C
BOOL dInsFldAnt_c::create(const nw4r::g3d::ResFile &res) {
    const param_c *param = getAntParam(mType);
    sAntNum++;
    initParam();
    createAntMdl(res);
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    if (mSpawnKind != SPAWN_PLAYER) {
        mAnmTexSrtWait = res.GetResAnmTexSrt("wait");
        mAnmTexSrt.create(mdl, mAnmTexSrtWait, &mAllocator, NULL, 1);
    }
    mAnmTexSrtCreated = true;
    if (dInsectField_c::getInstance()->getLifecycleState() !=
        fBase_c::DELETING) {
        setGroundPos();
        initState();
    }
    mAngle.x = 0;
    mAngle.y = 0;
    mAngle.z = 0;
    mShadowSize = param->mShadowSize;
    return TRUE;
}

// 0x17ACC
int dInsFldAnt_c::draw() {
    dInsFldBase_c::draw();
    return TRUE;
}

// 0x17AF0
BOOL dInsFldAnt_c::doDelete() {
    dInsFldBase_c::doDelete();
    if (mAnmTexSrtCreated == true && mSpawnKind != SPAWN_PLAYER) {
        mAnmTexSrt.remove();
    }
    if (!dInsectInfo::isAntsAttracted()) {
        fgMngProc_updateLitterFlags();
    }
    sAntNum--;
    return TRUE;
}

// 0x17B6C
void dInsFldAnt_c::createAntMdl(const nw4r::g3d::ResFile &res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    if (mSpawnKind != SPAWN_PLAYER) {
        mMdl.create(mdl, &mAllocator,
                    nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR |
                        nw4r::g3d::ScnMdl::ANM_VIS |
                        nw4r::g3d::ScnMdl::ANM_TEXSRT,
                    1, NULL);
    } else {
        mMdl.create(mdl, &mAllocator,
                    nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR |
                        nw4r::g3d::ScnMdl::ANM_VIS,
                    1, NULL);
    }
    mAnmChrWait = res.GetResAnmChr("wait");
    mAnmChrFly = res.GetResAnmChr("fly");
    if (mAnmChrWait.IsValid()) {
        mAnmChr.create(mdl, mAnmChrWait, &mAllocator, NULL);
    }
    mAnmVisWait = res.GetResAnmVis("wait");
    mAnmVisFly = res.GetResAnmVis("fly");
    if (mAnmVisWait.IsValid()) {
        mAnmVis.create(mdl, mAnmVisWait, &mAllocator, NULL);
    }
    mMdlCreated = true;
}

// 0x17CC0
void dInsFldAnt_c::setState(int state) {
    mState = state;
    (this->*sInit[state])();
}

// 0x17CF8
void dInsFldAnt_c::setGroundPos() {
    const param_c *param = getAntParam(mType);
    mPos.x += param->mOffsetX;
    mPos.y = param->mOffsetY + getGroundY();
    mPos.z += param->mOffsetZ;
}

// 0x17D68
void dInsFldAnt_c::initState() {
    setState(STATE_WAIT);
}

// 0x17D70
void dInsFldAnt_c::executeState() {
    (this->*sExecute[mState])();
    if (mSpawnKind != SPAWN_PLAYER) {
        mAnmTexSrt.play();
    }
}

// 0x17DD4
void dInsFldAnt_c::initWait() {
    mFrame = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmTexSrt(mAnmTexSrtWait);
}

// 0x17E2C
void dInsFldAnt_c::initFade() {
    mFrame = 0;
    f32 frame = mAnmChr.getFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmTexSrt(mAnmTexSrtWait);
    setAnmFrame(frame);
}

// 0x17EB0
void dInsFldAnt_c::onRelease() {
    const param_c *param = getAntParam(mType);
    mState = STATE_RELEASE;
    mFrame = 0;
    setAnmChr(mAnmChrFly, 0.0f);
    setAnmVis(mAnmVisFly);
    setAnmRate(param->mReleaseAnmRate);
    setAnmFrame(0.0f);
}

// 0x17F3C
void dInsFldAnt_c::executeWait() {
    registObjc(&mPos, 8.0f, TRUE);
    if (!isOnBadTurnip() && !isOnCandy()) {
        setState(STATE_FADE);
    }
}

// 0x17FA4
void dInsFldAnt_c::executeFade() {
    const param_c *param = getAntParam(mType);
    int alpha = mAlpha;
    alpha -= param->mFadeSpeed;
    mAlpha = alpha < 0 ? 0 : alpha;
    setMatAlpha(mAlpha);
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x1800C
void dInsFldAnt_c::executeRelease() {
    const param_c *param = getAntParam(mType);
    mVelocity.y += param->mGravity;
    mVelocity.set(0.0f, mVelocity.y, param->mSpeed);
    mVelocity.rotY(mAngle.y);
    mPos += mVelocity;
    if (isUnderGround() == true) {
        mPos.y = getGroundY();
        mVelocity.y = 0.0f;
    }
    if (isUnderWater() == true) {
        mSound.startSound(SE_MushiFallWater);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
        mPos.y = chk.getWaterY();
        mVec3_c scale(0.5f, 0.5f, 0.5f);
        dEffect::create("af_hny_watercolumn_S", &mPos, &mAngle, &scale);
        mActive = false;
        requestDelete();
    }
    fadeOut();
    if (mAlpha == 0) {
        requestDelete();
    }
}

// 0x18168
void dInsFldAnt_c::setAnmTexSrt(const nw4r::g3d::ResAnmTexSrt &anm) {
    if (mSpawnKind != SPAWN_PLAYER) {
        mAnmTexSrt.setAnm(mMdl, anm, 0, m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmTexSrt);
    }
}

// 0x181D4
BOOL dInsFldAnt_c::isOnBadTurnip() const {
    mVec3_c pos = mPos;
    dItem::Item *item = dInsectUtil::getItem(&pos);
    if (item != NULL && item->getKind() == dItem::KIND_BAD_KABU) {
        return TRUE;
    }
    return FALSE;
}

// 0x18230
BOOL dInsFldAnt_c::isOnCandy() const {
    mVec3_c pos = mPos;
    dItem::Item *item = dInsectUtil::getItem(&pos);
    if (item != NULL && item->getKind() == dItem::KIND_CANDY) {
        return TRUE;
    }
    return FALSE;
}

// 0x1828C
void dInsFldAnt_c::setMatAlpha(u8 alpha) {
    mMdl.setDrawMode(nw4r::g3d::RESMDL_DRAWMODE_SORT_XLU_Z);
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("m0");
    if (mat.IsValid()) {
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn());
        nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mat.ptr()->id);
        if (access.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = access.GetResMatTevColor();
            tev.GXGetTevKColor(GX_KCOLOR1, &color);
            color.a = alpha;
            tev.GXSetTevKColor(GX_KCOLOR1, color);
            tev.DCStore(false);
        }
    }
}
