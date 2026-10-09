// The m3d library. Based on the Skyward Sword decompilation (zeldaret/ss), m/m3d/*.cpp
// (m3d, m_scnleaf, m_calc_ratio, m_bmdl, m_smdl, m_mdl, m_banm, m_fanm, m_anmchr, m_anmvis, m_anmshp,
// m_anmmatclr, m_anmtexpat, m_anmtexsrt, m_proc).
//
// In City Folk all of this is ONE translation unit, built with -sym on (see notes/m3d.txt):
// - every function shares one .sdata2 float pool (0.0f, 1.0f, -1.0f and the int->float double),
// - the weak inline functions come once, after the last function (EGG::FogManager's dtor, then the
//   m_3d.hpp ones: banm_c::play, calcRatio_c's dtor, the anm*_c getTypes and child dtors),
// - the vtables come from one end-of-TU pass in reverse class-completion order.
// With -sym on every source FILE gets its own .text section, so the parts cannot be separate files that
// this one #includes: the code has to be in this file for the functions to stay in target order.
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_video.hpp>
#include <lib/egg/gfxe/eggStateGX.h>
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <game/mLib/m_mtx.hpp>
#include <nw4r/ut.h>
#include <game/mLib/m_heap.hpp>

// ---- m3d (SS m/m3d/m3d.cpp)

// City Folk keeps a single light manager and a single fog manager instead of SS's arrays.


namespace m3d {

namespace internal {

mAllocator_c *l_allocator_p;
nw4r::g3d::ScnRoot *l_scnRoot_p;
EGG::LightManager *l_lightMgr_p;
EGG::FogManager *l_fogMgr_p;

} // namespace internal

bool create(EGG::Heap *heap, ulong maxChildren, ulong maxScnObj, ulong numLightObj, ulong numLightSet) {
    internal::l_allocator_p = new (heap, 4) mAllocator_c();
    internal::l_allocator_p->attach(heap, 0x20);

    nw4r::g3d::G3dInit(true);
    nw4r::g3d::G3DState::SetRenderModeObj(mVideo::m_video->mRenderModeObj);

    size_t size;
    internal::l_scnRoot_p = nw4r::g3d::ScnRoot::Construct(
        internal::l_allocator_p, &size, maxChildren, maxScnObj, numLightObj, numLightSet
    );
    return internal::l_scnRoot_p != nullptr;
}

bool createFogMgr(EGG::Heap *heap, int numFog) {
    internal::l_fogMgr_p = new (heap, 4) EGG::FogManager(numFog);
    if (internal::l_fogMgr_p == nullptr) {
        removeFogMgr();
        return false;
    }
    return true;
}

void removeFogMgr() {
    if (internal::l_fogMgr_p != nullptr) {
        delete internal::l_fogMgr_p;
        internal::l_fogMgr_p = nullptr;
    }
}

nw4r::g3d::ScnRoot *getScnRoot() {
    return internal::l_scnRoot_p;
}

nw4r::g3d::Camera getCamera(int idx) {
    return internal::l_scnRoot_p->GetCamera(idx);
}

nw4r::g3d::Camera getCurrentCamera() {
    return internal::l_scnRoot_p->GetCurrentCamera();
}

int getCurrentCameraID() {
    return internal::l_scnRoot_p->GetCurrentCameraID();
}

void setCurrentCamera(int idx) {
    internal::l_scnRoot_p->SetCurrentCamera(idx);
}

nw4r::g3d::LightSetting *getLightSettingP() {
    return &internal::l_scnRoot_p->GetLightSetting();
}

EGG::LightManager *getLightMgr() {
    return internal::l_lightMgr_p;
}

EGG::FogManager *getFogMgr() {
    return internal::l_fogMgr_p;
}

void calcWorld() {
    if (internal::l_lightMgr_p != nullptr) {
        internal::l_lightMgr_p->Calc(internal::l_scnRoot_p);
    }
    if (internal::l_fogMgr_p != nullptr) {
        internal::l_fogMgr_p->Calc();
    }
    internal::l_scnRoot_p->CalcWorld();
}

void calcMaterial() {
    internal::l_scnRoot_p->CalcMaterial();
}

void calcView() {
    if (internal::l_lightMgr_p != nullptr) {
        nw4r::math::MTX34 camMtx;
        getCurrentCamera().GetCameraMtx(&camMtx);
        internal::l_lightMgr_p->CalcView(camMtx, getCurrentCameraID(), internal::l_scnRoot_p);
    }
    if (internal::l_fogMgr_p != nullptr) {
        internal::l_fogMgr_p->CopyToG3D(internal::l_scnRoot_p);
    }

    internal::l_scnRoot_p->CalcView();
    internal::l_scnRoot_p->GatherDrawScnObj();
    internal::l_scnRoot_p->ZSort();
}

void drawOpa() {
    internal::l_scnRoot_p->DrawOpa();
}

void drawXlu() {
    internal::l_scnRoot_p->DrawXlu();
}

bool pushBack(nw4r::g3d::ScnObj *obj) {
    return internal::l_scnRoot_p->PushBack(obj);
}

void clear() {
    internal::l_scnRoot_p->Clear();
}

void reset() {
    nw4r::g3d::G3dReset();
    if (internal::l_lightMgr_p != nullptr) {
        EGG::StateGX::resetGXCache();
    }
}

int getMatID(nw4r::g3d::ResMdl mdl, const char *name) {
    if (mdl.IsValid()) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(name);
        if (mat.IsValid()) {
            return mat.ref().id;
        }
    }
    return -1;
}

int getNodeID(nw4r::g3d::ResMdl mdl, const char *name) {
    if (mdl.IsValid()) {
        nw4r::g3d::ResNode node = mdl.GetResNode(name);
        if (node.IsValid()) {
            return node.GetID();
        }
    }
    return -1;
}

void resetMaterial() {
    GXSetNumIndStages(0);
    for (int i = GX_TEVSTAGE0; i < GX_MAX_TEVSTAGE; i++) {
        GXSetTevDirect((GXTevStageID)i);
    }
}

} // namespace m3d

// ---- scn_leaf (SS m/m3d/m_scnleaf.cpp)

m3d::scnLeaf_c::scnLeaf_c() {
    mpScn = nullptr;
}

m3d::scnLeaf_c::~scnLeaf_c() {
    scnLeaf_c::remove();
}

void m3d::scnLeaf_c::remove() {
    if (mpScn == nullptr) {
        return;
    }
    mpScn->Destroy();
    mpScn = nullptr;
}

void m3d::scnLeaf_c::entry() {
    m3d::pushBack(mpScn);
}

void m3d::scnLeaf_c::setOption(ulong option, ulong value) {
    mpScn->SetScnObjOption(option, value);
}

bool m3d::scnLeaf_c::getOption(ulong option, ulong *value) const {
    return mpScn->GetScnObjOption(option, value);
}

void m3d::scnLeaf_c::setScale(float x, float y, float z) {
    mpScn->SetScale(x, y, z);
}

void m3d::scnLeaf_c::setScale(const nw4r::math::VEC3 &scale) {
    mpScn->SetScale(scale);
}

void m3d::scnLeaf_c::setLocalMtx(const nw4r::math::MTX34 *mtx) {
    mpScn->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, mtx);
}

void m3d::scnLeaf_c::getLocalMtx(nw4r::math::MTX34 *mtx) const {
    mpScn->GetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, mtx);
}

void m3d::scnLeaf_c::getViewMtx(nw4r::math::MTX34 *mtx) const {
    mpScn->GetMtx(nw4r::g3d::ScnObj::MTX_VIEW, mtx);
}

void m3d::scnLeaf_c::calc(bool keepEnabledAfter) {
    setOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_WORLD, 0);
    mpScn->G3dProc(nw4r::g3d::G3dObj::G3DPROC_CALC_WORLD, 0, nullptr);
    if (!keepEnabledAfter) {
        setOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_WORLD, 1);
    }
}

void m3d::scnLeaf_c::calcVtx(bool keepEnabledAfter) {
    setOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_VTX, 0);
    mpScn->G3dProc(nw4r::g3d::G3dObj::G3DPROC_CALC_VTX, 0, nullptr);
    if (!keepEnabledAfter) {
        setOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_VTX, 1);
    }
}

void m3d::scnLeaf_c::setPriorityDraw(int prioOpa, int prioXlu) {
    mpScn->SetPriorityDrawOpa(prioOpa);
    mpScn->SetPriorityDrawXlu(prioXlu);
}

// ---- calc_ratio (SS m/m3d/m_calc_ratio.cpp)

m3d::calcRatio_c::calcRatio_c() {
    mWeight = 0.0f;
    mIsActive = false;
}

void m3d::calcRatio_c::remove() {
    mIsActive = false;
    reset();
}

void m3d::calcRatio_c::reset() {
    mWeight = 0;
    mT = 1;

    mScaleFrom = 0;
    mScaleTo = 1;
    mInterpolateT = 1;
}

void m3d::calcRatio_c::offUpdate() {
    mIsActive = true;
    mIsBlending = false;
}

void m3d::calcRatio_c::set(float duration) {
    if (duration == 0) {
        reset();
    } else {
        mWeight = 1;
        mT = 0;
        mScaleFrom = 1;
        mScaleTo = 0;
        mInterpolateT = 0;

        mIsBlending = true;
        mTimeStep = 1.0f / duration;
    }
}

void m3d::calcRatio_c::calc() {
    if (mWeight == 0.0f) {
        return;
    }
    mT += mTimeStep;
    if (mT >= 1.0f) {
        reset();
    } else {
        mIsBlending = true;

        float prevWeight = mWeight;
        mWeight -= mWeight * mT * mT;

        float divRes = mWeight / prevWeight;
        float subRes = 1.0f - mWeight;

        // [Doing this twice is required for matching]
        float inv1 = nw4r::math::FInv(divRes + subRes);
        float inv2 = nw4r::math::FInv(divRes + subRes);

        mScaleFrom = divRes * inv1;
        mScaleTo = subRes * inv1;
        mInterpolateT = subRes * inv2;
    }
}

bool m3d::calcRatio_c::isEnd() const {
    return mWeight == 0;
}

// ---- bmdl (SS m/m3d/m_bmdl.cpp)

m3d::bmdl_c::~bmdl_c() {
    remove();
}

void m3d::bmdl_c::setDrawMode(nw4r::g3d::ResMdlDrawMode mode) {
    nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn)->SetDrawMode(mode);
}

bool m3d::bmdl_c::getNodeWorldMtx(ulong idx, nw4r::math::MTX34 *mtx) const {
    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    return scnMdl->GetScnMtxPos(mtx, nw4r::g3d::ScnObj::MTX_WORLD, idx);
}

bool m3d::bmdl_c::getNodeWorldMtxMultVecZero(ulong idx, nw4r::math::VEC3 &vec) const {
    mMtx_c mtx;

    if (!getNodeWorldMtx(idx, &mtx)) {
        return false;
    }

    mtx.multVecZero(vec);
    return true;
}

bool m3d::bmdl_c::getNodeWorldMtxMultVec(ulong idx, const nw4r::math::VEC3 &in, nw4r::math::VEC3 &out) const {
    nw4r::math::MTX34 mtx;
    if (!getNodeWorldMtx(idx, &mtx)) {
        return false;
    }
    fn_803911A4(mtx, in, out);
    return true;
}

void m3d::bmdl_c::setAnm(m3d::banm_c &anm) {
    if (anm.getType() == banm_c::TYPE_ANM_SHP) {
        nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
        scnMdl->SetAnmObj(anm.getObj(), nw4r::g3d::ScnMdl::ANMOBJTYPE_NOT_SPECIFIED);
    } else {
        nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
        if (anm.getType() == banm_c::TYPE_ANM_CHR) {
            mpAnm = &anm;
        }

        scnMdl->SetAnmObj(anm.getObj(), nw4r::g3d::ScnMdl::ANMOBJTYPE_NOT_SPECIFIED);
    }
}

void m3d::bmdl_c::play() {
    if (mpAnm == nullptr) {
        return;
    }

    mpAnm->play();
}

nw4r::g3d::ResMdl m3d::bmdl_c::getResMdl() const {
    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    return scnMdl->GetResMdl();
}

nw4r::g3d::ResMat m3d::bmdl_c::getResMat(size_t idx) const {
    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    nw4r::g3d::ResMdl mdl = scnMdl->GetResMdl();
    return mdl.GetResMat(idx);
}

void m3d::bmdl_c::removeAnm(nw4r::g3d::ScnMdlSimple::AnmObjType objType) {
    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    if (objType == nw4r::g3d::ScnMdl::ANMOBJTYPE_CHR) {
        mpAnm = nullptr;
    }

    scnMdl->RemoveAnmObj(objType);
}

void m3d::bmdl_c::setTevColor(ulong idx, _GXTevRegID regID, _GXColor color, bool markDirty) {
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, idx);
        nw4r::g3d::ResMatTevColor tevColor = cma.GetResMatTevColor(markDirty);

        tevColor.GXSetTevColor(regID, color);
        tevColor.DCStore(false);
    } else {
        nw4r::g3d::ResMat resMat = getResMat(idx);
        nw4r::g3d::ResMatTevColor tevColor = resMat.GetResMatTevColor();

        tevColor.GXSetTevColor(regID, color);
        tevColor.DCStore(false);
    }
}

void m3d::bmdl_c::setTevColorAll(_GXTevRegID regID, _GXColor color, bool markDirty) {
    nw4r::g3d::ResMdl resMdl = getResMdl();
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, i);
            nw4r::g3d::ResMatTevColor tevColor = cma.GetResMatTevColor(markDirty);

            tevColor.GXSetTevColor(regID, color);
            tevColor.DCStore(false);
        }
    } else {
        for (size_t i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ResMat resMat = resMdl.GetResMat(i);
            nw4r::g3d::ResMatTevColor tevColor = resMat.GetResMatTevColor();

            tevColor.GXSetTevColor(regID, color);
            tevColor.DCStore(false);
        }
    }
}

void m3d::bmdl_c::setTevKColor(ulong idx, _GXTevKColorID colID, _GXColor color, bool markDirty) {
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, idx);
        nw4r::g3d::ResMatTevColor tevColor = cma.GetResMatTevColor(markDirty);

        tevColor.GXSetTevKColor(colID, color);
        tevColor.DCStore(false);
    } else {
        nw4r::g3d::ResMat resMat = getResMat(idx);
        nw4r::g3d::ResMatTevColor tevColor = resMat.GetResMatTevColor();

        tevColor.GXSetTevKColor(colID, color);
        tevColor.DCStore(false);
    }
}

void m3d::bmdl_c::setTevKColorAll(_GXTevKColorID colID, _GXColor color, bool markDirty) {
    nw4r::g3d::ResMdl resMdl = getResMdl();
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, i);
            nw4r::g3d::ResMatTevColor tevColor = cma.GetResMatTevColor(markDirty);

            tevColor.GXSetTevKColor(colID, color);
            tevColor.DCStore(false);
        }
    } else {
        for (size_t i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ResMat resMat = resMdl.GetResMat(i);
            nw4r::g3d::ResMatTevColor tevColor = resMat.GetResMatTevColor();

            tevColor.GXSetTevKColor(colID, color);
            tevColor.DCStore(false);
        }
    }
}

void m3d::bmdl_c::setLightSetIdxAll(int idx) {
    nw4r::g3d::ResMdl resMdl = getResMdl();
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, i);
            nw4r::g3d::ResMatMisc misc = cma.GetResMatMisc();
            misc.SetLightSetIdx(idx);
        }
    } else {
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ResMatMisc misc = resMdl.GetResMat(i).GetResMatMisc();
            misc.SetLightSetIdx(idx);
        }
    }
}

void m3d::bmdl_c::setEnvMapRefAll(int camRef, int lightRef) {
    nw4r::g3d::ResMdl resMdl = getResMdl();
    nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mpScn);
    if (scnMdl != nullptr) {
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ScnMdl::CopiedMatAccess cma(scnMdl, i);
            nw4r::g3d::ResTexSrt texSrt = cma.GetResTexSrtEx();
            for (ulong j = 0; j < 8; j++) {
                ulong mapMode;
                int curCamRef;
                int curLightRef;
                texSrt.GetMapMode(j, &mapMode, &curCamRef, &curLightRef);
                if (mapMode == 3 || mapMode == 1 || mapMode == 4) {
                    texSrt.SetMapMode(j, mapMode, camRef, lightRef);
                }
            }
        }
    } else {
        nw4r::g3d::ScnMdlSimple *simple = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
        for (ulong i = 0; i < resMdl.GetResMatNumEntries(); i++) {
            nw4r::g3d::ResTexSrt texSrt = resMdl.GetResMat(i).GetResTexSrt();
            for (ulong j = 0; j < 8; j++) {
                ulong mapMode;
                int curCamRef;
                int curLightRef;
                texSrt.GetMapMode(j, &mapMode, &curCamRef, &curLightRef);
                if (mapMode == 3 || mapMode == 1 || mapMode == 4) {
                    texSrt.SetMapMode(j, mapMode, camRef, lightRef);
                }
            }
        }
    }
}

void m3d::bmdl_c::remove() {
    mpAnm = nullptr;
    scnLeaf_c::remove();
}

// ---- smdl (SS m/m3d/m_smdl.cpp)

m3d::smdl_c::smdl_c() {}

m3d::smdl_c::~smdl_c() {}

bool m3d::smdl_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    if (bufferOption != 0) {
        mpScn = nw4r::g3d::ScnMdl::Construct(allocator, objSize, resMdl, bufferOption, viewCount);
    } else {
        mpScn = nw4r::g3d::ScnMdlSimple::Construct(allocator, objSize, resMdl, viewCount);
    }

    if (mpScn == nullptr) {
        return false;
    }

    mpScn->SetPriorityDrawOpa(127);
    mpScn->SetPriorityDrawXlu(127);

    return true;
}

// ---- mdl (SS m/m3d/m_mdl.cpp)

m3d::mdl_c::mdlCallback_c::mdlCallback_c() :
    mNodeCount(0), mpNodeResults(nullptr), mpCallback(nullptr), mpAllocator(nullptr) {}

m3d::mdl_c::mdlCallback_c::~mdlCallback_c() {}

void m3d::mdl_c::mdlCallback_c::ExecCallbackA(nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    u16 nodeID = cw->GetNodeID();
    nw4r::g3d::ChrAnmResult *result = &mpNodeResults[nodeID];

    if (mCalcRatio.isActive() && !mCalcRatio.isEnd()) {
        if (!mCalcRatio.isBlending()) {
            *anmRes = *result;
        } else {
            float slerpParam = mCalcRatio.getSlerpParam();
            float scaleFrom = mCalcRatio.getScaleFrom();
            float scaleTo = mCalcRatio.getScaleTo();

            u32 flags = anmRes->flags;

            if ((flags & nw4r::g3d::ChrAnmResult::FLAG_SCALE_ONE) == 0) {
                anmRes->s.x = anmRes->s.x * scaleTo + result->s.x * scaleFrom;
                anmRes->s.y = anmRes->s.y * scaleTo + result->s.y * scaleFrom;
                anmRes->s.z = anmRes->s.z * scaleTo + result->s.z * scaleFrom;
            } else {
                anmRes->s.x = scaleTo + result->s.x * scaleFrom;
                anmRes->s.y = scaleTo + result->s.y * scaleFrom;
                anmRes->s.z = scaleTo + result->s.z * scaleFrom;
            }

            Quaternion p, q;
            C_QUATMtx(&p, result->rt.m);
            if ((flags & nw4r::g3d::ChrAnmResult::FLAG_ROT_ZERO) == 0) {
                C_QUATMtx(&q, anmRes->rt.m);
            } else {
                q.x = 0;
                q.y = 0;
                q.z = 0;
                q.w = 1;
            }

            C_QUATSlerp(&p, &q, &p, slerpParam);

            nw4r::math::VEC3 cpy;
            cpy.x = anmRes->rt.m[0][3];
            cpy.y = anmRes->rt.m[1][3];
            cpy.z = anmRes->rt.m[2][3];
            PSMTXQuat(anmRes->rt.m, &p);
            anmRes->rt.m[0][3] = cpy.x;
            anmRes->rt.m[1][3] = cpy.y;
            anmRes->rt.m[2][3] = cpy.z;

            if ((flags & nw4r::g3d::ChrAnmResult::FLAG_TRANS_ZERO) == 0) {
                anmRes->rt.m[0][3] = cpy.x * scaleTo;
                anmRes->rt.m[1][3] = cpy.y * scaleTo;
                anmRes->rt.m[2][3] = cpy.z * scaleTo;
            }

            anmRes->rt.m[0][3] += result->rt.m[0][3] * scaleFrom;
            anmRes->rt.m[1][3] += result->rt.m[1][3] * scaleFrom;
            anmRes->rt.m[2][3] += result->rt.m[2][3] * scaleFrom;

            anmRes->flags &= ~(nw4r::g3d::ChrAnmResult::FLAG_ROT_RAW_FMT |
                               nw4r::g3d::ChrAnmResult::FLAG_TRANS_ZERO |
                               nw4r::g3d::ChrAnmResult::FLAG_ROT_ZERO |
                               nw4r::g3d::ChrAnmResult::FLAG_SCALE_ONE);

            *result = *anmRes;
        }
    } else {
        *result = *anmRes;
    }

    if (mpCallback != nullptr) {
        mpCallback->timingA(nodeID, anmRes, resMdl);
    }
}

void m3d::mdl_c::mdlCallback_c::ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    u16 nodeID = cw->GetNodeID();
    if (mpCallback != nullptr) {
        mpCallback->timingB(nodeID, manip, resMdl);
    }

    u32 nodeCount = resMdl.GetResNodeNumEntries();
    cw->SetNodeID((nodeID + 1) % nodeCount);
}

void m3d::mdl_c::mdlCallback_c::ExecCallbackC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    if (mpCallback != nullptr) {
        mpCallback->timingC(mtx, resMdl);
    }
    mCalcRatio.offUpdate();
}

bool m3d::mdl_c::mdlCallback_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, size_t *pSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size = 0;
    if (pSize == nullptr) {
        pSize = &size;
    }

    mNodeCount = resMdl.GetResNodeNumEntries();
    size_t resultSize = mNodeCount * sizeof(nw4r::g3d::ChrAnmResult);

    mpNodeResults = (nw4r::g3d::ChrAnmResult *) MEMAllocFromAllocator(allocator, resultSize);
    if (mpNodeResults == nullptr) {
        return false;
    }

    *pSize = nw4r::ut::RoundUp(resultSize + nw4r::ut::RoundUp(*pSize, 4), 4);

    nw4r::g3d::ChrAnmResult *curr = mpNodeResults;
    for (int i = 0; i < mNodeCount; i++) {
        curr->s.x = 1.0f;
        curr->s.y = 1.0f;
        curr->s.z = 1.0f;
        PSMTXIdentity(curr->rt.m);
        curr++;
    }

    mpAllocator = allocator;
    return true;
}

void m3d::mdl_c::mdlCallback_c::remove() {
    mCalcRatio.remove();
    if (mpNodeResults != nullptr) {
        mpAllocator->free(mpNodeResults);
    }
    mpNodeResults = nullptr;
    mpAllocator = nullptr;
}

void m3d::mdl_c::mdlCallback_c::setBlendFrame(float blendFrame) {
    mCalcRatio.set(blendFrame);
}

void m3d::mdl_c::mdlCallback_c::calcBlend() {
    if (!mCalcRatio.isEnd()) {
        mCalcRatio.calc();
    }
}

m3d::mdl_c::mdl_c() {}
m3d::mdl_c::~mdl_c() {}

bool m3d::mdl_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *pSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    if (!smdl_c::create(resMdl, allocator, bufferOption, viewCount, pSize)) {
        return false;
    }

    size_t mdlSize = 0;
    if (pSize != nullptr) {
        mdlSize = *pSize;
    }

    if (!mCallback.create(resMdl, allocator, pSize)) {
        remove();
        return false;
    }

    if (pSize != nullptr) {
        *pSize += mdlSize;
    }

    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    scnMdl->SetScnMdlCallback((nw4r::g3d::ICalcWorldCallback *) &mCallback);
    scnMdl->EnableScnMdlCallbackTiming(nw4r::g3d::ScnObj::CALLBACK_TIMING_ALL);

    setCallback(nullptr);
    return true;
}

void m3d::mdl_c::remove() {
    mCallback.remove();
    m3d::smdl_c::remove();
}

void m3d::mdl_c::setAnm(m3d::banm_c &anm) {
    setAnm(anm, 0.0f);
}

void m3d::mdl_c::play() {
    m3d::bmdl_c::play();
    mCallback.calcBlend();
}

void m3d::mdl_c::setAnm(m3d::banm_c &anm, float blendFrame) {
    if (anm.getType() == m3d::banm_c::TYPE_ANM_CHR) {
        mCallback.setBlendFrame(blendFrame);
    }

    bmdl_c::setAnm(anm);
}

void m3d::mdl_c::setCallback(m3d::mdl_c::callback_c *callback) {
    mCallback.mpCallback = callback;
}

// ---- banm (SS m/m3d/m_banm.cpp)

m3d::banm_c::~banm_c() {
    banm_c::remove();
}

void m3d::banm_c::remove() {
    if (mpObj != nullptr) {
        mpObj->DetachFromParent();
        mpObj->Destroy();
        mpObj = nullptr;

        if (mpHeap != nullptr) {
            mHeap::destroyFrmHeap(mpHeap);
            mpHeap = nullptr;
        }
    }

}

bool m3d::banm_c::createAllocator(mAllocator_c *allocator, size_t *size) {
    size_t aligned = nw4r::ut::RoundUp(mHeap::frmHeapCost(*size, 0x20), 0x20) - mHeap::frmHeapCost(0, 0x20);
    *size = nw4r::ut::RoundUp(mHeap::frmHeapCost(aligned, 0x20), 0x20);

    mpHeap = mHeap::createFrmHeap(aligned, allocator->mpHeap, "アニメ切り替え用アロケータ(m3d::banm_c::m_heap)", 0x20, mHeap::OPT_NONE);
    mAllocator.attach(mpHeap, 0x20);
    return true;
}

bool m3d::banm_c::IsBound() const {
    if (mpObj == nullptr) {
        return false;
    }
    return mpObj->TestAnmFlag(nw4r::g3d::AnmObj::FLAG_ANM_BOUND);
}

float m3d::banm_c::getFrame() const {
    return mpObj->GetFrame();
}

void m3d::banm_c::setFrameOnly(float frame) {
    mpObj->SetFrame(frame);
}

float m3d::banm_c::getRate() const {
    return mpObj->GetUpdateRate();
}

void m3d::banm_c::setRate(float frame) {
    mpObj->SetUpdateRate(frame);
}

// ---- fanm (SS m/m3d/m_fanm.cpp)

m3d::fanm_c::fanm_c() {
    mFrameMax = 0;
    mCurrFrame = 0;
    mPlayMode = FORWARD_LOOP;
}

m3d::fanm_c::~fanm_c() {}

void m3d::fanm_c::play() {
    float frame = mpObj->GetFrame();
    float updateRate = mpObj->GetUpdateRate();

    bool updateRateNegative = updateRate < 0.0f;
    if (updateRateNegative) {
        updateRate *= -1.0f;
    }

    mCurrFrame = frame;

    if (updateRateNegative || (mPlayMode & MASK_FORWARD) != 0) {
        // Reverse animation
        if (frame >= updateRate) {
            frame -= updateRate;
        } else {
            if ((mPlayMode & MASK_LOOP) == 0) {
                frame += mFrameMax - updateRate;
            } else {
                frame = mFrameStart;
            }
        }
    } else {
        // Forward animation
        frame += updateRate;
        if ((mPlayMode & MASK_LOOP) == 0) {
            if (frame >= mFrameMax) {
                frame -= mFrameMax;
            }
        } else {
            float endFrame = mFrameMax - 1.0f;
            if (frame >= endFrame) {
                frame = endFrame;
            }
        }
    }

    mpObj->SetFrame(frame);
}

void m3d::fanm_c::set(float duration, m3d::playMode_e playMode, float updateRate, float startFrame) {
    float frame = startFrame;
    if (startFrame < 0.0f) {
        if (playMode == FORWARD_ONCE) {
            frame = 0.0f;
        } else {
            frame = duration - 1.0f;
        }
    }

    mFrameMax = duration;
    mFrameStart = 0.0f;

    mpObj->SetFrame(frame);
    mpObj->SetUpdateRate(updateRate);

    mPlayMode = playMode;
    mCurrFrame = frame;
}

float m3d::fanm_c::getFrame() const {
    return mpObj->GetFrame();
}

void m3d::fanm_c::setFrame(float f) {
    mpObj->SetFrame(f);
    mCurrFrame = f;
}

float m3d::fanm_c::getRate() const {
    return mpObj->GetUpdateRate();
}

void m3d::fanm_c::setRate(float rate) {
    mpObj->SetUpdateRate(rate);
}

bool m3d::fanm_c::isStop() const {
    float frame = mpObj->GetFrame();
    float updateRate = mpObj->GetUpdateRate();

    // Only the "once" modes should ever be able to stop
    if (updateRate < 0.0f || mPlayMode == REVERSE_ONCE) {
        return frame <= mFrameStart;
    } else if (mPlayMode == FORWARD_ONCE) {
        return frame >= mFrameMax - 1.0f;
    }

    return false;
}

bool m3d::fanm_c::checkFrame(float f) const {
    float objFrame = mpObj->GetFrame();
    if (mCurrFrame == objFrame) {
        return objFrame == f;
    }

    float updateRate = mpObj->GetUpdateRate();

    if (updateRate < 0.0f || (mPlayMode & MASK_FORWARD) != 0) {
        // Reverse animation
        if (mCurrFrame > objFrame) {
            if (mCurrFrame > f && objFrame <= f) {
                return true;
            }
        } else {
            // mCurrFrame < objFrame
            if (f < mCurrFrame || f >= objFrame) {
                return true;
            }
        }
    } else {
        // Forward animation
        if (mCurrFrame < objFrame) {
            if (mCurrFrame < f && objFrame >= f) {
                return true;
            }
        } else {
            // mCurrFrame > objFrame
            if (f > mCurrFrame || f <= objFrame) {
                return true;
            }
        }
    }

    return false;
}

// ---- anm_chr (SS m/m3d/m_anmchr.cpp)

bool m3d::anmChr_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr anmChr, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    nw4r::g3d::AnmObjChrRes::Construct(nullptr, objSize, anmChr, mdl, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjChrRes::Construct(&mAllocator, &size, anmChr, mdl, false);
    mpObj->Bind(mdl);

    setFrmCtrlDefault(anmChr, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmChr_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, m3d::playMode_e playMode) {
    nw4r::g3d::ScnMdlSimple::AnmObjType anmType = (nw4r::g3d::ScnMdlSimple::AnmObjType) getType();
    mdl.removeAnm(anmType);
    setAnmAfter(mdl, anmChr, playMode);
}

void m3d::anmChr_c::setAnmAfter(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, m3d::playMode_e playMode) {
    nw4r::g3d::G3dObj *n = mpObj->GetParent();

    nw4r::g3d::AnmObjChrNode *chrNode;
    int nodeIdx;
    float weight;
    if (n != nullptr) {
        chrNode = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjChrNode>(n);

        nodeIdx = 0;
        while (nodeIdx < chrNode->Size()) {
            if (chrNode->GetChild(nodeIdx) != mpObj) {
                nodeIdx++;
            } else {
                break;
            }
        }

        weight = chrNode->GetWeight(nodeIdx);
        chrNode->Detach(nodeIdx);
    }

    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);

    size_t size;
    mpObj = nw4r::g3d::AnmObjChrRes::Construct(&mAllocator, &size, anmChr, mdl.getResMdl(), false);
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmChr, playMode);
    if (n == nullptr) {
        return;
    }

    // Newly constructed object is at the last position
    nw4r::g3d::AnmObjChrRes *chrRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjChrRes>(mpObj);
    chrNode->SetWeight(nodeIdx, weight);
    chrNode->Attach(nodeIdx, chrRes);
}

void m3d::anmChr_c::setFrmCtrlDefault(nw4r::g3d::ResAnmChr &anmChr, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmChr.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmChr.GetNumFrame(), playMode, 1.0f, -1.0f);
}

// ---- anm_vis (SS m/m3d/m_anmvis.cpp)

bool m3d::anmVis_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmVis anmVis, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    nw4r::g3d::AnmObjVisRes::Construct(nullptr, objSize, anmVis, mdl);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjVisRes::Construct(&mAllocator, &size, anmVis, mdl);
    mpObj->Bind(mdl);
    setFrmCtrlDefault(anmVis, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmVis_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmVis anmVis, m3d::playMode_e playMode) {
    nw4r::g3d::ScnMdlSimple::AnmObjType anmType = (nw4r::g3d::ScnMdlSimple::AnmObjType) getType();
    mdl.removeAnm(anmType);
    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);

    size_t size;
    mpObj = nw4r::g3d::AnmObjVisRes::Construct(&mAllocator, &size, anmVis, mdl.getResMdl());
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmVis, playMode);
}

void m3d::anmVis_c::setFrmCtrlDefault(nw4r::g3d::ResAnmVis &anmVis, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmVis.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmVis.GetNumFrame(), playMode, 1.0f, -1.0f);
}

// ---- anm_shp (SS m/m3d/m_anmshp.cpp)

bool m3d::anmShp_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmShp anmShp, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    nw4r::g3d::AnmObjShpRes::Construct(nullptr, objSize, anmShp, mdl, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjShpRes::Construct(&mAllocator, &size, anmShp, mdl, false);
    mpObj->Bind(mdl);
    setFrmCtrlDefault(anmShp, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmShp_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmShp anmShp, m3d::playMode_e playMode) {
    nw4r::g3d::ScnMdlSimple::AnmObjType anmType = (nw4r::g3d::ScnMdlSimple::AnmObjType) getType();
    mdl.removeAnm(anmType);
    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);

    size_t size;
    mpObj = nw4r::g3d::AnmObjShpRes::Construct(&mAllocator, &size, anmShp, mdl.getResMdl(), false);
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmShp, playMode);
}

void m3d::anmShp_c::setFrmCtrlDefault(nw4r::g3d::ResAnmShp &anmShp, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmShp.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmShp.GetNumFrame(), playMode, 1.0f, -1.0f);
}

// ---- anm_mat_clr (SS m/m3d/m_anmmatclr.cpp)

size_t m3d::anmMatClr_c::child_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjMatClrRes::Construct(nullptr, &size, anmClr, mdl, false);
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmMatClr_c::child_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmClr, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjMatClrRes::Construct(&mAllocator, nullptr, anmClr, mdl, false);
    mpObj->Bind(mdl);
    setFrmCtrlDefault(anmClr, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmMatClr_c::child_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmClr anmClr, m3d::playMode_e playMode) {
    releaseAnm();
    mpObj = nw4r::g3d::AnmObjMatClrRes::Construct(&mAllocator, nullptr, anmClr, mdl.getResMdl(), false);
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmClr, playMode);
}

void m3d::anmMatClr_c::child_c::releaseAnm() {
    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);
}

void m3d::anmMatClr_c::child_c::setFrmCtrlDefault(nw4r::g3d::ResAnmClr &anmClr, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmClr.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmClr.GetNumFrame(), playMode, 1.0f, -1.0f);
}

size_t m3d::anmMatClr_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, long count, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjMatClrOverride::Construct(nullptr, &size, mdl, count);
    size += nw4r::ut::RoundUp(count * sizeof(child_c), 0x20);
    size += nw4r::ut::RoundUp(child_c::heapCost(mdl, anmClr, true), 0x20) * count;
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmMatClr_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, mAllocator_c *allocator, size_t *objSize, long count) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size = 0;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmClr, count, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjMatClrOverride::Construct(&mAllocator, nullptr, mdl, count);
    mpChildren = (m3d::anmMatClr_c::child_c *) MEMAllocFromAllocator(&mAllocator, nw4r::ut::RoundUp(count * sizeof(child_c), 0x20));

    nw4r::g3d::AnmObjMatClrOverride *matClrOverride = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrOverride>(mpObj);

    child_c *child = &mpChildren[0];
    for (int i = 0; i < count; i++) {
        new(child) child_c();
        if (!child->create(mdl, anmClr, &mAllocator, nullptr)) {
            mHeap::destroyFrmHeap(mpHeap);
            return false;
        }
        nw4r::g3d::AnmObjMatClrRes *clrRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrRes>(child->getObj());
        matClrOverride->Attach(i, clrRes);
        if (i > 0) {
            child->releaseAnm();
        }
        child++;
    }
    return true;
}

m3d::anmMatClr_c::~anmMatClr_c() {
    anmMatClr_c::remove();
}

void m3d::anmMatClr_c::remove() {
    nw4r::g3d::AnmObjMatClrOverride *matClr = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrOverride>(mpObj);
    if (matClr != nullptr && mpChildren != nullptr) {
        int count = matClr->Size();
        for (int i = 0; i < count; i++) {
            mpChildren[i].remove();
        }
        mpChildren = nullptr;
    }
    banm_c::remove();
}

void m3d::anmMatClr_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmClr clr, long idx, m3d::playMode_e playMode) {
    nw4r::g3d::AnmObjMatClrOverride *matClr = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrOverride>(mpObj);
    matClr->Detach(idx);
    mpChildren[idx].setAnm(mdl, clr, playMode);
    nw4r::g3d::AnmObjMatClrRes *clrRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrRes>(mpChildren[idx].getObj());
    matClr->Attach(idx, clrRes);
}

void m3d::anmMatClr_c::releaseAnm(long idx) {
    nw4r::g3d::AnmObjMatClrOverride *matClr = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrOverride>(mpObj);
    mpChildren[idx].releaseAnm();
}

void m3d::anmMatClr_c::play() {
    nw4r::g3d::AnmObjMatClrOverride *matClr = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjMatClrOverride>(mpObj);
    int count = matClr->Size();
    for (int i = 0; i < count; i++) {
        play(i);
    }
}

void m3d::anmMatClr_c::play(long idx) {
    if (mpChildren[idx].IsBound()) {
        mpChildren[idx].play();
    }
}

float m3d::anmMatClr_c::getFrame(long idx) const {
    return mpChildren[idx].getFrame();
}

void m3d::anmMatClr_c::setFrame(float frame, long idx) {
    mpChildren[idx].setFrame(frame);
}

float m3d::anmMatClr_c::getRate(long idx) const {
    return mpChildren[idx].getRate();
}

void m3d::anmMatClr_c::setRate(float rate, long idx) {
    mpChildren[idx].setRate(rate);
}

bool m3d::anmMatClr_c::isStop(long idx) const {
    return mpChildren[idx].isStop();
}

bool m3d::anmMatClr_c::checkFrame(float frame, long idx) const {
    return mpChildren[idx].checkFrame(frame);
}

void m3d::anmMatClr_c::setPlayMode(m3d::playMode_e playMode, long idx) {
    mpChildren[idx].mPlayMode = playMode;
}

m3d::playMode_e m3d::anmMatClr_c::getPlayMode(long idx) const {
    return (playMode_e)mpChildren[idx].mPlayMode;
}

float m3d::anmMatClr_c::getFrameMax(long idx) const {
    return mpChildren[idx].mFrameMax;
}

float m3d::anmMatClr_c::getFrameStart(long idx) const {
    return mpChildren[idx].mFrameStart;
}

// ---- anm_tex_pat (SS m/m3d/m_anmtexpat.cpp)

size_t m3d::anmTexPat_c::child_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjTexPatRes::Construct(nullptr, &size, anmTexPat, mdl, false);
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmTexPat_c::child_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmTexPat, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjTexPatRes::Construct(&mAllocator, nullptr, anmTexPat, mdl, false);
    mpObj->Bind(mdl);
    setFrmCtrlDefault(anmTexPat, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmTexPat_c::child_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexPat anmTexPat, m3d::playMode_e playMode) {
    releaseAnm();
    mpObj = nw4r::g3d::AnmObjTexPatRes::Construct(&mAllocator, nullptr, anmTexPat, mdl.getResMdl(), false);
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmTexPat, playMode);
}

void m3d::anmTexPat_c::child_c::releaseAnm() {
    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);
}

void m3d::anmTexPat_c::child_c::setFrmCtrlDefault(nw4r::g3d::ResAnmTexPat &anmTexPat, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmTexPat.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmTexPat.GetNumFrame(), playMode, 1.0f, -1.0f);
}

size_t m3d::anmTexPat_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, long count, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjTexPatOverride::Construct(nullptr, &size, mdl, count);
    size += nw4r::ut::RoundUp(count * sizeof(child_c), 0x20);
    size += nw4r::ut::RoundUp(child_c::heapCost(mdl, anmTexPat, true), 0x20) * count;
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmTexPat_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator, size_t *objSize, long count) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size = 0;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmTexPat, count, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjTexPatOverride::Construct(&mAllocator, nullptr, mdl, count);
    mpChildren = (m3d::anmTexPat_c::child_c *) MEMAllocFromAllocator(&mAllocator, nw4r::ut::RoundUp(count * sizeof(child_c), 0x20));

    nw4r::g3d::AnmObjTexPatOverride *texPatOverride = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatOverride>(mpObj);

    child_c *child = &mpChildren[0];
    for (int i = 0; i < count; i++) {
        new(child) child_c();
        if (!child->create(mdl, anmTexPat, &mAllocator, nullptr)) {
            mHeap::destroyFrmHeap(mpHeap);
            return false;
        }
        nw4r::g3d::AnmObjTexPatRes *texPatRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatRes>(child->getObj());
        texPatOverride->Attach(i, texPatRes);
        if (i > 0) {
            child->releaseAnm();
        }
        child++;
    }
    return true;
}

m3d::anmTexPat_c::~anmTexPat_c() {
    anmTexPat_c::remove();
}

void m3d::anmTexPat_c::remove() {
    nw4r::g3d::AnmObjTexPatOverride *texPat = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatOverride>(mpObj);
    if (texPat != nullptr && mpChildren != nullptr) {
        int count = texPat->Size();
        for (int i = 0; i < count; i++) {
            mpChildren[i].remove();
        }
        mpChildren = nullptr;
    }
    banm_c::remove();
}

void m3d::anmTexPat_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexPat anmTexPat, long idx, m3d::playMode_e playMode) {
    nw4r::g3d::AnmObjTexPatOverride *texPat = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatOverride>(mpObj);
    texPat->Detach(idx);
    mpChildren[idx].setAnm(mdl, anmTexPat, playMode);
    nw4r::g3d::AnmObjTexPatRes *texPatRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatRes>(mpChildren[idx].getObj());
    texPat->Attach(idx, texPatRes);
}

void m3d::anmTexPat_c::releaseAnm(long idx) {
    nw4r::g3d::AnmObjTexPatOverride *texPat = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatOverride>(mpObj);
    mpChildren[idx].releaseAnm();
}

void m3d::anmTexPat_c::play() {
    nw4r::g3d::AnmObjTexPatOverride *texPat = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexPatOverride>(mpObj);
    int count = texPat->Size();
    for (int i = 0; i < count; i++) {
        play(i);
    }
}

void m3d::anmTexPat_c::play(long idx) {
    if (mpChildren[idx].IsBound()) {
        mpChildren[idx].play();
    }
}

float m3d::anmTexPat_c::getFrame(long idx) const {
    return mpChildren[idx].getFrame();
}

void m3d::anmTexPat_c::setFrame(float frame, long idx) {
    mpChildren[idx].setFrame(frame);
}

float m3d::anmTexPat_c::getRate(long idx) const {
    return mpChildren[idx].getRate();
}

void m3d::anmTexPat_c::setRate(float rate, long idx) {
    mpChildren[idx].setRate(rate);
}

bool m3d::anmTexPat_c::isStop(long idx) const {
    return mpChildren[idx].isStop();
}

bool m3d::anmTexPat_c::checkFrame(float frame, long idx) const {
    return mpChildren[idx].checkFrame(frame);
}

void m3d::anmTexPat_c::setPlayMode(m3d::playMode_e playMode, long idx) {
    mpChildren[idx].mPlayMode = playMode;
}

m3d::playMode_e m3d::anmTexPat_c::getPlayMode(long idx) const {
    return (playMode_e)mpChildren[idx].mPlayMode;
}

float m3d::anmTexPat_c::getFrameMax(long idx) const {
    return mpChildren[idx].mFrameMax;
}

float m3d::anmTexPat_c::getFrameStart(long idx) const {
    return mpChildren[idx].mFrameStart;
}

// ---- anm_tex_srt (SS m/m3d/m_anmtexsrt.cpp)

size_t m3d::anmTexSrt_c::child_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjTexSrtRes::Construct(nullptr, &size, anmTexSrt, mdl, false);
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmTexSrt_c::child_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmTexSrt, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjTexSrtRes::Construct(&mAllocator, nullptr, anmTexSrt, mdl, false);
    mpObj->Bind(mdl);
    setFrmCtrlDefault(anmTexSrt, PLAYMODE_INHERIT);
    return true;
}

void m3d::anmTexSrt_c::child_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, m3d::playMode_e playMode) {
    releaseAnm();
    mpObj = nw4r::g3d::AnmObjTexSrtRes::Construct(&mAllocator, nullptr, anmTexSrt, mdl.getResMdl(), false);
    mpObj->Bind(mdl.getResMdl());
    setFrmCtrlDefault(anmTexSrt, playMode);
}

void m3d::anmTexSrt_c::child_c::releaseAnm() {
    mpObj->Release();
    mpHeap->free(MEM_FRM_HEAP_FREE_ALL);
}

void m3d::anmTexSrt_c::child_c::setFrmCtrlDefault(nw4r::g3d::ResAnmTexSrt &anmTexSrt, m3d::playMode_e playMode) {
    if (playMode == PLAYMODE_INHERIT) {
        playMode = (anmTexSrt.GetAnmPolicy() == nw4r::g3d::ANM_POLICY_ONETIME) ? FORWARD_ONCE : FORWARD_LOOP;
    }
    fanm_c::set(anmTexSrt.GetNumFrame(), playMode, 1.0f, -1.0f);
}

size_t m3d::anmTexSrt_c::heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, long count, bool calcAligned) {
    size_t size = 0;
    nw4r::g3d::AnmObjTexSrtOverride::Construct(nullptr, &size, mdl, count);
    size += nw4r::ut::RoundUp(count * sizeof(child_c), 0x20);
    size += nw4r::ut::RoundUp(child_c::heapCost(mdl, anmTexSrt, true), 0x20) * count;
    if (calcAligned) {
        size = nw4r::ut::RoundUp(mHeap::frmHeapCost(size, 0x20), 0x20);
    }
    return size;
}

bool m3d::anmTexSrt_c::create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, mAllocator_c *allocator, size_t *objSize, long count) {
    if (allocator == nullptr) {
        allocator = m3d::internal::l_allocator_p;
    }

    size_t size = 0;
    if (objSize == nullptr) {
        objSize = &size;
    }

    *objSize = heapCost(mdl, anmTexSrt, count, false);
    if (!createAllocator(allocator, objSize)) {
        return false;
    }

    mpObj = nw4r::g3d::AnmObjTexSrtOverride::Construct(&mAllocator, nullptr, mdl, count);
    mpChildren = (m3d::anmTexSrt_c::child_c *) MEMAllocFromAllocator(&mAllocator, nw4r::ut::RoundUp(count * sizeof(child_c), 0x20));

    nw4r::g3d::AnmObjTexSrtOverride *texSrtOverride = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtOverride>(mpObj);

    child_c *child = &mpChildren[0];
    for (int i = 0; i < count; i++) {
        new(child) child_c();
        if (!child->create(mdl, anmTexSrt, &mAllocator, nullptr)) {
            mHeap::destroyFrmHeap(mpHeap);
            EGG::Heap::free(mpHeap, nullptr);
            return false;
        }
        nw4r::g3d::AnmObjTexSrtRes *texSrtRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtRes>(child->getObj());
        texSrtOverride->Attach(i, texSrtRes);
        if (i > 0) {
            child->releaseAnm();
        }
        child++;
    }
    return true;
}

m3d::anmTexSrt_c::~anmTexSrt_c() {
    anmTexSrt_c::remove();
}

void m3d::anmTexSrt_c::remove() {
    nw4r::g3d::AnmObjTexSrtOverride *texSrt = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtOverride>(mpObj);
    if (texSrt != nullptr && mpChildren != nullptr) {
        int count = texSrt->Size();
        for (int i = 0; i < count; i++) {
            mpChildren[i].remove();
        }
        mpChildren = nullptr;
    }
    banm_c::remove();
}

void m3d::anmTexSrt_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, long idx, m3d::playMode_e playMode) {
    nw4r::g3d::AnmObjTexSrtOverride *texSrt = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtOverride>(mpObj);
    texSrt->Detach(idx);
    mpChildren[idx].setAnm(mdl, anmTexSrt, playMode);
    nw4r::g3d::AnmObjTexSrtRes *texSrtRes = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtRes>(mpChildren[idx].getObj());
    texSrt->Attach(idx, texSrtRes);
}

void m3d::anmTexSrt_c::releaseAnm(long idx) {
    nw4r::g3d::AnmObjTexSrtOverride *texSrt = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtOverride>(mpObj);
    mpChildren[idx].releaseAnm();
}

void m3d::anmTexSrt_c::play() {
    nw4r::g3d::AnmObjTexSrtOverride *texSrt = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjTexSrtOverride>(mpObj);
    int count = texSrt->Size();
    for (int i = 0; i < count; i++) {
        play(i);
    }
}

void m3d::anmTexSrt_c::play(long idx) {
    if (mpChildren[idx].IsBound()) {
        mpChildren[idx].play();
    }
}

float m3d::anmTexSrt_c::getFrame(long idx) const {
    return mpChildren[idx].getFrame();
}

void m3d::anmTexSrt_c::setFrame(float frame, long idx) {
    mpChildren[idx].setFrame(frame);
}

float m3d::anmTexSrt_c::getRate(long idx) const {
    return mpChildren[idx].getRate();
}

void m3d::anmTexSrt_c::setRate(float rate, long idx) {
    mpChildren[idx].setRate(rate);
}

bool m3d::anmTexSrt_c::isStop(long idx) const {
    return mpChildren[idx].isStop();
}

bool m3d::anmTexSrt_c::checkFrame(float frame, long idx) const {
    return mpChildren[idx].checkFrame(frame);
}

void m3d::anmTexSrt_c::setPlayMode(m3d::playMode_e playMode, long idx) {
    mpChildren[idx].mPlayMode = playMode;
}

m3d::playMode_e m3d::anmTexSrt_c::getPlayMode(long idx) const {
    return (playMode_e)mpChildren[idx].mPlayMode;
}

float m3d::anmTexSrt_c::getFrameMax(long idx) const {
    return mpChildren[idx].mFrameMax;
}

float m3d::anmTexSrt_c::getFrameStart(long idx) const {
    return mpChildren[idx].mFrameStart;
}

// ---- proc (SS m/m3d/m_proc.cpp)

void m3d::proc_c_drawProc(nw4r::g3d::ScnProc *proc, bool drawOpa) {
    m3d::proc_c *m3dProc = (m3d::proc_c *) proc->GetUserData();
    if (drawOpa) {
        m3dProc->drawOpa();
    } else {
        m3dProc->drawXlu();
    }
}

bool m3d::proc_c::create(mAllocator_c *allocator, size_t *objSize) {
    if (allocator == nullptr) {
        allocator = internal::l_allocator_p;
    }

    size_t size;
    if (objSize == nullptr) {
        objSize = &size;
    }

    mpScn = nw4r::g3d::ScnProc::Construct(allocator, objSize, proc_c_drawProc, true, true, 0);
    if (mpScn == nullptr) {
        return false;
    }

    mpScn->SetPriorityDrawOpa(127);
    mpScn->SetPriorityDrawXlu(127);

    nw4r::g3d::ScnProc *p = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnProc>(mpScn);
    p->SetUserData(this);
    return true;
}
