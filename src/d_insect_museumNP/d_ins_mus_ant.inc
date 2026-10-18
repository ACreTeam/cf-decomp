// dInsMusAnt_c: the ant line of the museum's insect rooms.

namespace dInsectMuseum {

enum { ANT_TYPE_NUM = 1 };

dInsMusAntParam_c sAntParam[ANT_TYPE_NUM] = {
    {{0.0f, 0.0f, 0.0f}},
};

f32 sAntAnmRate = 1.0f; // unused

// 0xF180
int getAntParamIdx(int type) {
    int idx = 0;
    switch (type) {
    case INSECT_ANT:
        idx = 0;
        break;
    }
    return idx;
}

// 0xF194
const dInsMusAntParam_c *getAntParam(int type) {
    return &sAntParam[getAntParamIdx(type)];
}

} // namespace dInsectMuseum

// 0xF1C4
dInsMusAnt_c::dInsMusAnt_c() : mState(NULL) {}

// 0xF280
dInsMusAnt_c::~dInsMusAnt_c() {}

// 0xF2F4
void dInsMusAnt_c::init() {
    mHomePos.set(162.0f, 28.0f, 416.0f);
    mPos.set(162.0f, 28.0f, 416.0f);
    mAngle.set(0, 0, 0);
}

// 0xF334
void dInsMusAnt_c::initState() {
    initWait();
}

// 0xF338
void dInsMusAnt_c::executeState() {
    const dInsMusAntParam_c *param = getAntParam(mType);
    mCheckBg = false;
    (this->*mState)();
    mAnmTexPat.play();
    mAnmTexSrt.play();
}

// 0xF3A8
void dInsMusAnt_c::initWait() {
    mTimer = 0;
    mState = &dInsMusAnt_c::stateWait;
}

// 0xF3D0
void dInsMusAnt_c::stateWait() {
    static const f32 anmRate = 1.0f;
    mAnmTexPat.setRate(anmRate, 0);
    mAnmTexSrt.setRate(anmRate, 0);
}

// 0xF424
void dInsMusAnt_c::createModel(nw4r::g3d::ResFile &res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::ANM_TEXPAT | nw4r::g3d::ScnMdl::ANM_TEXSRT, 1, NULL);
    nw4r::g3d::ResAnmTexPat texPat = res.GetResAnmTexPat(0);
    nw4r::g3d::ResAnmTexSrt texSrt = res.GetResAnmTexSrt(0);
    mAnmTexPat.create(mdl, texPat, &mAllocator, NULL, 1);
    mAnmTexSrt.create(mdl, texSrt, &mAllocator, NULL, 1);
    mMdl.setAnm(mAnmTexPat);
    mMdl.setAnm(mAnmTexSrt);
}

// 0xF530
BOOL dInsMusAnt_c::doDelete() {
    mMdl.remove();
    mAnmTexPat.remove();
    mAnmTexSrt.remove();
    return TRUE;
}
