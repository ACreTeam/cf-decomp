#include <game/game/d_a_rmobj_grace.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_face_tex_mng.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_shadow.hpp>
#include <game/mLib/m_heap.hpp>

// GracieGrace's mannequin. Functions are in target .text order.

namespace dARmobjGrace {
// The mannequin's place (the shadow and the pointer box).
static mVec3_c l_pos(384.0f, 28.0f, 24.0f);

// .text 0xCC
BASE_PROFILE(A_RMOBJ_GRACE, daRmobjGrace_c);
} // namespace dARmobjGrace

// .text 0x4DC
int daRmobjGrace_c::create() {
    setDemoID(0);
    dOutfit_c *outfit = dSaveData_c::getExtra()->mOutfit.getShown();
    createMdlHeap();
    if (loadRes()) {
        bindBodyTex();
        if (dItem::isRealItemId(outfit->mEquip.mHat.mId)) {
            nw4r::g3d::ResFile res(mHatRes.getData());
            mHatMdl.create(res.GetResMdl(0), &mAllocator, 0, 1, NULL);
        }
        if (dItem::isRealItemId(outfit->mEquip.mAcc.mId)) {
            nw4r::g3d::ResFile res(mAccRes.getData());
            mAccMdl.create(res.GetResMdl(0), &mAllocator, 0, 1, NULL);
        }
        mBodyModel.setAnmChr(ANM_POSE, m3d::FORWARD_ONCE, 0.0f);
        mMtx_c mtx;
        PSMTXTrans(mtx, 384.0f, 28.0f, 20.0f);
        mBodyModel.mMdl.setLocalMtx(&mtx);
        mBodyModel.mMdl.calc(false);
        int node = m3d::getNodeID(mBodyModel.mMdl.getResMdl(), "head");
        if (node != -1) {
            mMtx_c headMtx;
            mBodyModel.mMdl.getNodeWorldMtx(node, &headMtx);
            if (mHeadModel.isCreated()) {
                mHeadModel.mMdl.setLocalMtx(&headMtx);
            }
            if (mCapHeadModel.isCreated()) {
                mCapHeadModel.mMdl.setLocalMtx(&headMtx);
            }
            if (mBaldHeadModel.isCreated()) {
                mBaldHeadModel.mMdl.setLocalMtx(&headMtx);
            }
            if (mHatMdl.getScn() != NULL) {
                mHatMdl.setLocalMtx(&headMtx);
            }
            if (mAccMdl.getScn() != NULL) {
                mAccMdl.setLocalMtx(&headMtx);
            }
        }
        mDpdBox.set(dARmobjGrace::l_pos, 64.0f, 28.8f, 48.0f, 0,
                    dObjc::HIT_RMOBJ, this, 0);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x770
int daRmobjGrace_c::execute() {
    mDpdBox.regist();
    return SUCCEEDED;
}

// .text 0x798
int daRmobjGrace_c::draw() {
    mAng angle(0);
    dShadow::requestRound(&dARmobjGrace::l_pos, 3, &angle, 16.0f, 1.0f, 1.0f,
                          0.0f, 1.0f);
    if (mBodyModel.isCreated()) {
        mBodyModel.entry();
    }
    if (mHeadModel.isCreated()) {
        mHeadModel.entry();
    }
    if (mCapHeadModel.isCreated()) {
        mCapHeadModel.entry();
    }
    if (mBaldHeadModel.isCreated()) {
        mBaldHeadModel.entry();
    }
    if (mHatMdl.getScn() != NULL) {
        mHatMdl.entry();
    }
    if (mAccMdl.getScn() != NULL) {
        mAccMdl.entry();
    }
    return SUCCEEDED;
}

// .text 0x88C
int daRmobjGrace_c::doDelete() {
    bool ok = true;
    ok &= mBodyModel.remove();
    ok &= mHeadModel.remove();
    ok &= mCapHeadModel.remove();
    ok &= mBaldHeadModel.remove();
    ok &= mShirtRes.release();
    ok &= mHatRes.release();
    ok &= mAccRes.release();
    if (mHatMdl.getScn() != NULL) {
        mHatMdl.remove();
    }
    if (mAccMdl.getScn() != NULL) {
        mAccMdl.remove();
    }
    if (ok) {
        deleteMdlHeap();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x9B4
BOOL daRmobjGrace_c::loadRes() {
    dOutfit_c *saved = &dSaveData_c::getExtra()->mOutfit;
    BOOL male = saved->isValid() ? saved->isMale() : FALSE;
    dOutfit_c *outfit = saved->getShown();
    bool ok = true;
    BOOL bald = FALSE;
    BOOL cap = FALSE;
    BOOL mask = FALSE;
    dItem::Item shirt = outfit->mEquip.mShirt;
    if (dItem::isRealItemId(shirt.mId)) {
        ok &= mShirtRes.loadItem(shirt, dHeap::localHeap_p);
    }
    dItem::Item hat = outfit->mEquip.mHat;
    if (dItem::isRealItemId(hat.mId)) {
        ok &= mHatRes.loadItem(hat, dHeap::localHeap_p);
        const dItem::BITM *bitm =
            dItem::infoBank_c::get()->getBITM(outfit->mEquip.mHat);
        if (bitm != NULL) {
            switch (bitm->getHideBone()) {
            case dItem::HIDE_BONE_CAP:
                cap = TRUE;
                break;
            case dItem::HIDE_BONE_FULL_MASK:
                mask = TRUE;
                break;
            case 3: // the other hats that hide the hair (unnamed hide-bone
                    // values)
            case 4:
            case 5:
            case 6:
                bald = TRUE;
                break;
            }
        }
    }
    dItem::Item acc = outfit->mEquip.mAcc;
    if (dItem::isRealItemId(acc.mId)) {
        ok &= mAccRes.loadItem(acc, dHeap::localHeap_p);
    }
    ok &= mBodyModel.create(male ? "g_mnqn_body_boy" : "g_mnqn_body_girl", 0);
    if (cap) {
        ok &= mCapHeadModel.create(
            male ? "g_mnqn_head_boy_b" : "g_mnqn_head_girl_b", 0);
    } else if (bald) {
        ok &= mBaldHeadModel.create("g_mnqn_head_hage", 0);
    } else if (!mask) {
        ok &= mHeadModel.create(
            male ? "g_mnqn_head_boy_a" : "g_mnqn_head_girl_a", 0);
    }
    return ok;
}

// .text 0xC7C
void daRmobjGrace_c::createMdlHeap() {
    if (m_frmHeap_p == NULL) {
        m_frmHeap_p = mHeap::createFrmHeap(
            0x7800, dHeap::localHeap_p,
            "daRmobjGrace_c::m_frmHeap_p : グレースマネキンヒープ", 0x20,
            mHeap::OPT_NONE);
        mAllocator.attach(m_frmHeap_p, 0x20);
    }
}

// .text 0xCE4
void daRmobjGrace_c::deleteMdlHeap() {
    if (m_frmHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = NULL;
    }
}

// .text 0xD28
// The shoes' texture and palette are numbered from the shoe colour:
// dHmnFaceTexMng_c files 0x2C.. (0x22.. without shoes) and palettes 0x52..
// (0x48..), into palette buffer 2 of type 3.
void daRmobjGrace_c::bindBodyTex() {
    dOutfit_c *outfit = &dSaveData_c::getRaw()->mExtra.mOutfit;
    nw4r::g3d::ResFile(mBodyModel.getData()).Release();
    nw4r::g3d::ResFile(mBodyModel.getData())
        .Bind(nw4r::g3d::ResFile(mShirtRes.getData()));
    nw4r::g3d::ResFile(mBodyModel.getData())
        .Bind(dHmnFaceTexMng_c::getResFile(
            outfit->mShoeColor + (!outfit->hasShoes() ? 0x22 : 0x2C)));
    int pltt = outfit->mShoeColor + (!outfit->hasShoes() ? 0x48 : 0x52);
    mPalette.setType(3);
    mPalette.setPltt(pltt, 2);
    nw4r::g3d::ResFile(mBodyModel.getData()).Bind(mPalette.getResFile(2));
    nw4r::g3d::ResFile(mBodyModel.getData()).Bind();
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
