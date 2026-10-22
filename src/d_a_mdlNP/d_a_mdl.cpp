// A generic model actor (REL d_a_mdlNP). See include/game/game/d_a_mdl.hpp.
#include <game/game/d_a_mdl.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_heap.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/wpad.h>

namespace dAMdl {
// .text:0x5C
BASE_PROFILE(A_MDL, dAcMdl_c);
} // namespace dAMdl

// The number of presses of the A button so far.
static u8 sPressCount;

// .text:0x13C
int dAcMdl_c::create() {
    if (!createFrmHeap()) {
        return NOT_READY;
    }
    if (mpRes == nullptr) {
        mpRes = mLoader.request("Sample/cat.brres", 0, mpFrmHeap);
        if (mpRes == nullptr) {
            return NOT_READY;
        }
    }
    mAllocator.attach(mpFrmHeap, 0x20);
    setupModel();
    mHeap::adjustFrmHeap(mpFrmHeap);
    return SUCCEEDED;
}

// .text:0x1D4
int dAcMdl_c::doDelete() {
    destroyFrmHeap();
    return SUCCEEDED;
}

// .text:0x1F8
int dAcMdl_c::execute() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        switch (sPressCount & 3) {
        case 0:
            mAnmChrPart.setAnm(mMdl, mRes.GetResAnmChr("walk"),
                               m3d::FORWARD_LOOP);
            mMdl.setPartAnm(1, &mAnmChrPart, 256.0f);
            mMdl.setPartNode(1, "jnt_Rfoot1_npc");
            mMdl.setPartNode(1, "jnt_Larm1_npc");
            break;
        case 1:
            mAnmChrPart.setAnm(mMdl, mRes.GetResAnmChr("run"),
                               m3d::FORWARD_LOOP);
            mMdl.setPartAnm(1, &mAnmChrPart, 256.0f);
            break;
        case 2:
            mMdl.removePartAnm(1, 256.0f);
            break;
        }
        sPressCount++;
    }
    mMdl.play();
    mAnmTexPat.play();
    mMdl.calc(false);
    return SUCCEEDED;
}

// .text:0x380
int dAcMdl_c::draw() {
    mMdl.entry();
    return SUCCEEDED;
}

// .text:0x3B0
BOOL dAcMdl_c::createFrmHeap() {
    if (mpFrmHeap != nullptr) {
        return TRUE;
    }
    mpFrmHeap =
        mHeap::createFrmHeap(0x222E0, EGG::Heap::getCurrentHeap(),
                             "dAcMdl_c::::m_heap", 0x20, mHeap::OPT_NONE);
    return TRUE;
}

// .text:0x418
void dAcMdl_c::destroyFrmHeap() {
    if (mpFrmHeap != nullptr) {
        mMdl.remove();
        mAnmChr.remove();
        mAnmChrPart.remove();
        mAnmTexPat.remove();
        mHeap::destroyFrmHeap(mpFrmHeap);
        mpFrmHeap = nullptr;
    }
}

// .text:0x4A8
void dAcMdl_c::setupModel() {
    mRes = nw4r::g3d::ResFile(mpRes);
    mRes.Init();
    mRes.Bind();
    nw4r::g3d::ResMdl resMdl = mRes.GetResMdl(0);
    mMdl.create(resMdl, &mAllocator, 2, 0, 1, nullptr);
    mAnmChr.create(resMdl, mRes.GetResAnmChr("wait"), &mAllocator, nullptr);
    mAnmChr.mPlayMode = m3d::FORWARD_LOOP;
    mMdl.setAnm(mAnmChr);
    mAnmChrPart.create(resMdl, mRes.GetResAnmChr("walk"), &mAllocator, nullptr);
    mMdl.setPartAnm(1, &mAnmChrPart, 0.0f);
    mAnmTexPat.create(resMdl, mRes.GetResAnmTexPat("cat_0"), &mAllocator,
                      nullptr, 1);
    mMdl.setAnm(mAnmTexPat);
    setupMtx();
}

// .text:0x628
void dAcMdl_c::setupMtx() {
    mMtx_c mtx;
    makeMtx(&mtx, &mPos, mAngle.y.mAngle);
    mtx.scaleM(0.01f, 0.01f, 0.01f);
    mMdl.setLocalMtx(&mtx);
}

// .text:0x69C
dAcMdl_c::~dAcMdl_c() {
}
