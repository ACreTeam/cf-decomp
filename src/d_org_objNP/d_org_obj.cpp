#include <game/game/d_org_obj.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>

// Debug parameters (RTTI "dOrgObj_hostIO_c"): the models' position, rotation
// and scale when calc is given no matrix.
class dOrgObj_hostIO_c {
  public:
    dOrgObj_hostIO_c() {
        _4 = -1;
        _8 = -1;
        mPos.x = 0.0f;
        mPos.y = 0.0f;
        mPos.z = 0.0f;
        mScale = 1.0f;
        mRotX = mRotY = 0;
        _1C = 0x100;
        _24 = 0;
    }
    virtual ~dOrgObj_hostIO_c();

    /* 0x04 */ s16 _4;
    /* 0x08 */ int _8;
    /* 0x0C */ mVec3_c mPos;
    /* 0x18 */ s16 mRotX;
    /* 0x1A */ s16 mRotY;
    /* 0x1C */ s16 _1C;
    /* 0x20 */ f32 mScale;
    /* 0x24 */ u8 _24;
}; // size 0x28

namespace dOrgObj {

// .text:0xCC
void *create() {
    return new dOrgObj_c;
}

fProf::fBaseProfile_c g_profile_ORG_OBJ = {&create, fProf::ORG_OBJ,
                                           fProf::DRAW_ORDER::ORG_OBJ};

} // namespace dOrgObj

dOrgObj_c *dOrgObj_c::sInstance;

namespace dOrgObj {

static dOrgObj_hostIO_c l_HIO;

} // namespace dOrgObj

// .text:0x1E0
int dOrgObj_c::create() {
    initHeap();
    if (loadRes()) {
        sInstance = this;
        createCloth();
        createUmb();
        createCap();
        createCat();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x250
int dOrgObj_c::execute() {
    executeCloth();
    executeUmb();
    executeCap();
    executeCat();
    return SUCCEEDED;
}

// .text:0x298
int dOrgObj_c::draw() {
    return SUCCEEDED;
}

// .text:0x2A0
int dOrgObj_c::doDelete() {
    if (releaseRes()) {
        removeCloth();
        removeUmb();
        removeCap();
        removeCat();
        deleteHeap();
        sInstance = NULL;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x314
void dOrgObj_c::replaceTexture(void *tex, void *pltt) {
    mLoader.replaceTexture(tex, pltt, 0, 0, 0x20, 0x80);
}

// .text:0x32C
void dOrgObj_c::setLight(m3d::bmdl_c *mdl) {
    static int envCamRef = 1;
    int lightRef = dBookLight_c::sLightObjIdx;
    mdl->setLightSetIdxAll(1);
    mdl->setEnvMapRefAll(envCamRef, lightRef);
}

// .text:0x384
BOOL dOrgObj_c::loadRes() {
    return mLoader.loadItem(dItem::Item(0x372), dHeap::localHeap_p);
}

// .text:0x3CC
bool dOrgObj_c::releaseRes() {
    return mLoader.release();
}

// .text:0x3D4
void dOrgObj_c::initHeap() {
    if (m_frmHeap_p == NULL) {
        m_frmHeap_p =
            mHeap::createFrmHeap(0x7800, dHeap::localHeap_p,
                                 "dOrgObj_c::m_frmHeap_p : オリジナルOBJヒープ",
                                 0x20, mHeap::OPT_NONE);
        mAllocator.attach(m_frmHeap_p, 0x20);
    }
}

// .text:0x43C
void dOrgObj_c::deleteHeap() {
    if (m_frmHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = NULL;
    }
}

// .text:0x480
void dOrgObj_c::bindCloth() {
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(0));
    res.Release();
    res.Bind(nw4r::g3d::ResFile(mLoader.getData()));
    res.Bind();
}

// .text:0x4E8
BOOL dOrgObj_c::createCloth() {
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(0));
    bindCloth();
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mCloth.mMdl.create(mdl, &mAllocator, 0x30C, 1, NULL);
    mCloth.mAnmSrt.create(mdl, res.GetResAnmTexSrt(0), &mAllocator, NULL, 1);
    mCloth.mAnmClr.create(mdl, res.GetResAnmClr(0), &mAllocator, NULL, 1);
    mCloth.mAnm = -1;
    setClothAnm(-1);
    setLight(&mCloth.mMdl);
    return TRUE;
}

// .text:0x5E4
BOOL dOrgObj_c::executeCloth() {
    if (mCloth.mAnm >= 0) {
        mCloth.mAnmSrt.play();
        mCloth.mAnmClr.play();
    } else {
        mCloth.mAnmSrt.setFrame(0.0f, 0);
        mCloth.mAnmClr.setFrame(0.0f, 0);
    }
    return TRUE;
}

// .text:0x670
void dOrgObj_c::calcCloth(const mMtx_c *mtx) {
    mCloth.mMdl.setLocalMtx(mtx);
    mCloth.mMdl.calc(false);
}

// .text:0x6AC
BOOL dOrgObj_c::drawCloth() {
    if (isDraw(DRAW_CLOTH)) {
        mCloth.mMdl.entry();
    }
    return TRUE;
}

// .text:0x6E8
BOOL dOrgObj_c::removeCloth() {
    mCloth.mMdl.remove();
    if (mCloth.mAnmSrt.IsBound()) {
        mCloth.mAnmSrt.remove();
    }
    if (mCloth.mAnmClr.IsBound()) {
        mCloth.mAnmClr.remove();
    }
    return TRUE;
}

// .text:0x770
void dOrgObj_c::setClothAnm(int anm) {
    if (anm >= 0) {
        nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(0));
        mCloth.mAnmSrt.setAnm(mCloth.mMdl,
                              res.GetResAnmTexSrt(anm < 0 ? 0 : (u32)anm % 4),
                              0, m3d::FORWARD_ONCE);
        mCloth.mMdl.setAnm(mCloth.mAnmSrt);
        mCloth.mAnmClr.setAnm(mCloth.mMdl, res.GetResAnmClr(0), 0,
                              m3d::FORWARD_LOOP);
        mCloth.mMdl.setAnm(mCloth.mAnmClr);
    } else {
        mCloth.mAnmSrt.releaseAnm(0);
        mCloth.mMdl.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_TEXSRT);
        mCloth.mAnmClr.releaseAnm(0);
        mCloth.mMdl.removeAnm(nw4r::g3d::ScnMdlSimple::ANMOBJTYPE_MATCLR);
    }
    mCloth.mAnm = anm;
}

// .text:0x87C
void dOrgObj_c::bindUmb() {
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(1));
    res.Release();
    res.Bind(nw4r::g3d::ResFile(mLoader.getData()));
    res.Bind();
}

// .text:0x8E4
BOOL dOrgObj_c::createUmb() {
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(1));
    bindUmb();
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mUmb.mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mUmb.mAnmChr.create(mdl, res.GetResAnmChr(0), &mAllocator, NULL);
    mUmb.mMdl.setAnm(mUmb.mAnmChr);
    setLight(&mUmb.mMdl);
    return TRUE;
}

// .text:0x9B0
BOOL dOrgObj_c::executeUmb() {
    return TRUE;
}

// .text:0x9B8
void dOrgObj_c::calcUmb(const mMtx_c *mtx) {
    mUmb.mMdl.setLocalMtx(mtx);
    mUmb.mMdl.calc(false);
}

// .text:0x9F4
BOOL dOrgObj_c::drawUmb() {
    if (isDraw(DRAW_UMB)) {
        mUmb.mMdl.entry();
    }
    return TRUE;
}

// .text:0xA30
BOOL dOrgObj_c::removeUmb() {
    mUmb.mMdl.remove();
    if (mUmb.mAnmChr.IsBound()) {
        mUmb.mAnmChr.remove();
    }
    return TRUE;
}

// .text:0xA94
void dOrgObj_c::bindCap() {
    nw4r::g3d::ResFile cap(dFTR::alwaysBank_c::get()->getFile(2));
    cap.Release();
    cap.Bind(nw4r::g3d::ResFile(mLoader.getData()));
    cap.Bind();
    nw4r::g3d::ResFile stand(dFTR::alwaysBank_c::get()->getFile(3));
    stand.Release();
    stand.Bind(nw4r::g3d::ResFile(mLoader.getData()));
    stand.Bind();
}

// .text:0xB3C
BOOL dOrgObj_c::createCap() {
    bindCap();
    nw4r::g3d::ResFile cap(dFTR::alwaysBank_c::get()->getFile(2));
    nw4r::g3d::ResMdl mdl = cap.GetResMdl(0);
    mCap.mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    nw4r::g3d::ResFile stand(dFTR::alwaysBank_c::get()->getFile(3));
    mCap.mStandMdl.create(stand.GetResMdl(0), &mAllocator, 0, 1, NULL);
    setLight(&mCap.mMdl);
    setLight(&mCap.mStandMdl);
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw() != NULL) {
        static const char *boyNode = "cap_md_boy_mdl";
        static const char *girlNode = "cap_md_girl_mdl";
        u8 gender =
            dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->mPID.player.mGender;
        nw4r::g3d::ResNode boy = mdl.GetResNode(boyNode);
        nw4r::g3d::ResNode girl = mdl.GetResNode(girlNode);
        if (gender != 0) {
            if (boy.IsValid()) {
                boy.SetVisibility(false);
            }
            if (girl.IsValid()) {
                girl.SetVisibility(true);
            }
        } else {
            if (boy.IsValid()) {
                boy.SetVisibility(true);
            }
            if (girl.IsValid()) {
                girl.SetVisibility(false);
            }
        }
    }
    return TRUE;
}

// .text:0xCAC
BOOL dOrgObj_c::executeCap() {
    return TRUE;
}

// .text:0xCB4
void dOrgObj_c::calcCap(const mMtx_c *mtx) {
    mCap.mStandMdl.setLocalMtx(mtx);
    mCap.mStandMdl.calc(false);
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(3));
    int id = m3d::getNodeID(res.GetResMdl(0), "set");
    if (id != -1) {
        mMtx_c setMtx;
        mCap.mStandMdl.getNodeWorldMtx(id, &setMtx);
        mCap.mMdl.setLocalMtx(&setMtx);
        mCap.mMdl.calc(false);
    }
}

// .text:0xD50
BOOL dOrgObj_c::drawCap() {
    if (isDraw(DRAW_CAP)) {
        mCap.mMdl.entry();
        mCap.mStandMdl.entry();
    }
    return TRUE;
}

// .text:0xDAC
BOOL dOrgObj_c::removeCap() {
    mCap.mMdl.remove();
    mCap.mStandMdl.remove();
    return TRUE;
}

// .text:0xE00
void dOrgObj_c::bindCat() {
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(4));
    res.Release();
    res.Bind(nw4r::g3d::ResFile(mLoader.getData()));
    res.Bind();
}

// .text:0xE68
BOOL dOrgObj_c::createCat() {
    bindCat();
    nw4r::g3d::ResFile res(dFTR::alwaysBank_c::get()->getFile(4));
    mCat.mMdl.create(res.GetResMdl(0), &mAllocator, 0x204, 1, NULL);
    setLight(&mCat.mMdl);
    return TRUE;
}

// .text:0xEE0
BOOL dOrgObj_c::executeCat() {
    return TRUE;
}

// .text:0xEE8
BOOL dOrgObj_c::drawCat() {
    if (isDraw(DRAW_CAT) && mCat.mMdl.getScn() != NULL) {
        mCat.mMdl.entry();
    }
    return TRUE;
}

// .text:0xF30
void dOrgObj_c::calcCat(const mMtx_c *mtx) {
    if (mCat.mMdl.getScn() != NULL) {
        mCat.mMdl.setLocalMtx(mtx);
        mCat.mMdl.calc(false);
    }
}

// .text:0xF78
BOOL dOrgObj_c::removeCat() {
    if (mCat.mMdl.getScn() != NULL) {
        mCat.mMdl.remove();
    }
    return TRUE;
}

// .text:0xFB4
void dOrgObj_c::calc(const mMtx_c *mtx) {
    if (mtx != NULL) {
        calcCloth(mtx);
        calcUmb(mtx);
        calcCap(mtx);
        calcCat(mtx);
    } else {
        dOrgObj_hostIO_c &hio = dOrgObj::l_HIO;
        mMtx_c m;
        m.trans(hio.mPos.x, hio.mPos.y, hio.mPos.z);
        m.XrotM(mAng(hio.mRotX));
        m.YrotM(mAng(hio.mRotY));
        mMtx_c scale;
        PSMTXScale(scale.m, hio.mScale, hio.mScale, hio.mScale);
        PSMTXConcat(m, scale.m, m);
        calcCloth(&m);
        calcUmb(&m);
        calcCap(&m);
        calcCat(&m);
    }
}

// .text:0x10B4
dOrgObj_hostIO_c::~dOrgObj_hostIO_c() {
}

// .text:0x10F4
dOrgObj_c::~dOrgObj_c() {
}
