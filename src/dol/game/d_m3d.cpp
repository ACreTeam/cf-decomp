// Game-side m3d extensions: m3d::mdlEx_c (a model with per-part animation slots and blending) and
// m3d::anmChrPart_c. City Folk only. .text 8000AD84..8000BCA8, .data 8049DF10..8049E010,
// .sdata 80749768..80749798, .sdata2 8074FD60..8074FD68.
// mdlEx_c::mdlCallback_c is a variant of m3d::mdl_c::mdlCallback_c (m_3d.cpp) that blends per slot.
// See include/game/game/d_m3d.hpp and notes/d_m3d.txt.
#include <game/game/d_m3d.hpp>
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>

m3d::mdlEx_c::mdlCallback_c::mdlCallback_c() :
    mNodeCount(0), mAnmNum(0), mpNodeResults(nullptr), mpAnm(nullptr), mpCalcRatio(nullptr), mpNodeAnmIdx(nullptr),
    mpCallback(nullptr) {}

m3d::mdlEx_c::mdlCallback_c::~mdlCallback_c() {}

void m3d::mdlEx_c::mdlCallback_c::ExecCallbackA(nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    u16 nodeID = cw->GetNodeID();
    nw4r::g3d::ChrAnmResult *result = &mpNodeResults[nodeID];
    calcRatio_c *calcRatio = getCalcRatio(nodeID);
    getAnmResult(anmRes, nodeID);

    if (result->flags != 0 && !calcRatio->isEnd()) {
        u32 flags = anmRes->flags;
        float scaleFrom = calcRatio->getScaleFrom();
        float scaleTo = calcRatio->getScaleTo();
        float slerpParam = calcRatio->getSlerpParam();

        float sx = result->s.x * scaleFrom;
        float sy = result->s.y * scaleFrom;
        float sz = result->s.z * scaleFrom;
        if ((flags & nw4r::g3d::ChrAnmResult::FLAG_SCALE_ONE) == 0) {
            sx += anmRes->s.x * scaleTo;
            sy += anmRes->s.y * scaleTo;
            sz += anmRes->s.z * scaleTo;
        } else {
            sx += scaleTo;
            sy += scaleTo;
            sz += scaleTo;
        }
        anmRes->s.x = sx;
        anmRes->s.y = sy;
        anmRes->s.z = sz;

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
    }

    *result = *anmRes;
    result->flags = 1;

    if (mpCallback != nullptr) {
        mpCallback->timingA(nodeID, anmRes, resMdl);
    }
}

void m3d::mdlEx_c::mdlCallback_c::ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    u16 nodeID = cw->GetNodeID();
    if (mpCallback != nullptr) {
        mpCallback->timingB(nodeID, manip, resMdl);
    }

    u32 nodeCount = resMdl.GetResNodeNumEntries();
    cw->SetNodeID((nodeID + 1) % nodeCount);
}

void m3d::mdlEx_c::mdlCallback_c::ExecCallbackC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw) {
    if (mpCallback != nullptr) {
        mpCallback->timingC(mtx, resMdl);
    }
}

bool m3d::mdlEx_c::mdlCallback_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong anmNum, size_t *pSize) {
    size_t size = 0;
    if (pSize == nullptr) {
        pSize = &size;
    }

    mAnmNum = anmNum;
    mNodeCount = resMdl.GetResNodeNumEntries();

    size_t resultSize = mNodeCount * sizeof(nw4r::g3d::ChrAnmResult);
    mpNodeResults = (nw4r::g3d::ChrAnmResult *) MEMAllocFromAllocator(allocator, resultSize);
    if (mpNodeResults == nullptr) {
        return false;
    }
    *pSize = nw4r::ut::RoundUp(resultSize + nw4r::ut::RoundUp(*pSize, 0x20), 4);

    size_t idxSize = nw4r::ut::RoundUp(mNodeCount, 0x20);
    mpNodeAnmIdx = (u8 *) MEMAllocFromAllocator(allocator, idxSize);
    if (mpNodeAnmIdx == nullptr) {
        return false;
    }
    *pSize = nw4r::ut::RoundUp(idxSize + nw4r::ut::RoundUp(*pSize, 0x20), 4);

    size_t anmSize = mAnmNum * sizeof(anmChr_c *);
    mpAnm = (anmChr_c **) MEMAllocFromAllocator(allocator, anmSize);
    if (mpAnm == nullptr) {
        return false;
    }
    *pSize = nw4r::ut::RoundUp(anmSize + nw4r::ut::RoundUp(*pSize, 0x20), 4);

    size_t ratioSize = anmNum * sizeof(calcRatio_c);
    mpCalcRatio = (calcRatio_c *) MEMAllocFromAllocator(allocator, ratioSize);
    if (mpCalcRatio == nullptr) {
        return false;
    }
    *pSize = nw4r::ut::RoundUp(ratioSize + nw4r::ut::RoundUp(*pSize, 0x20), 4);

    nw4r::g3d::ChrAnmResult *curr = mpNodeResults;
    ulong i;
    for (i = 0; i < mNodeCount; i++) {
        mpNodeAnmIdx[i] = 0;
        curr->flags = 0;
        curr->s.x = 1.0f;
        curr->s.y = 1.0f;
        curr->s.z = 1.0f;
        PSMTXIdentity(curr->rt.m);
        curr++;
    }

    for (i = 0; i < anmNum; i++) {
        mpAnm[i] = nullptr;
        mpCalcRatio[i].reset();
    }

    return true;
}

void m3d::mdlEx_c::mdlCallback_c::setAnm(int idx, anmChr_c *anm, float blendFrame) {
    mpAnm[idx] = anm;
    mpCalcRatio[idx].set(blendFrame);
}

void m3d::mdlEx_c::mdlCallback_c::calcBlend() {
    for (ulong i = 0; i < mAnmNum; i++) {
        if (!mpCalcRatio[i].isEnd()) {
            mpCalcRatio[i].calc();
            if (i != 0 && mpCalcRatio[i].isEnd() && mpAnm[i] == nullptr) {
                changeNodeAnmIdx(i, 0);
            }
        }
    }
}

void m3d::mdlEx_c::mdlCallback_c::setPartAnm(int idx, anmChr_c *anm, float blendFrame) {
    mpAnm[idx] = anm;
    if (blendFrame == 0.0f) {
        mpCalcRatio[idx].reset();
    } else {
        mpCalcRatio[idx].set(blendFrame);
    }
}

void m3d::mdlEx_c::mdlCallback_c::removePartAnm(int idx, float blendFrame) {
    mpAnm[idx] = nullptr;
    u8 newIdx = 0;
    if (blendFrame == 0.0f) {
        mpCalcRatio[idx].reset();
    } else {
        mpCalcRatio[idx].set(blendFrame);
        newIdx = idx | 0x80;
    }
    changeNodeAnmIdx(idx, newIdx);
}

void m3d::mdlEx_c::mdlCallback_c::playAnm() {
    for (ulong i = 0; i < mAnmNum; i++) {
        if (mpAnm[i] != nullptr) {
            mpAnm[i]->play();
        }
    }
}

m3d::calcRatio_c *m3d::mdlEx_c::mdlCallback_c::getCalcRatio(ulong nodeId) {
    return &mpCalcRatio[getNodeAnmIdx(nodeId)];
}

void m3d::mdlEx_c::mdlCallback_c::getAnmResult(nw4r::g3d::ChrAnmResult *anmRes, ulong nodeId) {
    u8 anmIdx = mpNodeAnmIdx[nodeId];
    u8 idx = anmIdx & 0x7F;
    if (idx == 0 || (anmIdx & 0x80)) {
        return;
    }

    nw4r::g3d::AnmObjChr *anmChr = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::AnmObjChr>(mpAnm[idx]->getObj());
    nw4r::g3d::ChrAnmResult buf;
    *anmRes = *anmChr->GetResult(&buf, nodeId);
}

void m3d::mdlEx_c::mdlCallback_c::changeNodeAnmIdx(u8 from, u8 to) {
    for (ulong i = 0; i < mNodeCount; i++) {
        if (getNodeAnmIdx(i) == from) {
            mpNodeAnmIdx[i] = to;
        }
    }
}

m3d::mdlEx_c::mdlEx_c() {}

m3d::mdlEx_c::~mdlEx_c() {}

bool m3d::mdlEx_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *pSize) {
    return create(resMdl, allocator, 1, bufferOption, viewCount, pSize);
}

bool m3d::mdlEx_c::create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong anmNum, ulong bufferOption, int viewCount, size_t *pSize) {
    if (!smdl_c::create(resMdl, allocator, bufferOption, viewCount, pSize)) {
        return false;
    }

    if (!mCallback.create(resMdl, allocator, anmNum, pSize)) {
        remove();
        return false;
    }

    nw4r::g3d::ScnMdlSimple *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    scnMdl->SetScnMdlCallback(&mCallback);
    scnMdl->EnableScnMdlCallbackTiming(nw4r::g3d::ScnObj::CALLBACK_TIMING_ALL);

    setCallback(nullptr);
    return true;
}

void m3d::mdlEx_c::setAnm(m3d::banm_c &anm) {
    setAnm(anm, 0.0f);
}

void m3d::mdlEx_c::play() {
    mCallback.playAnm();
    mCallback.calcBlend();
}

void m3d::mdlEx_c::calcBlend() {
    mCallback.calcBlend();
}

void m3d::mdlEx_c::setAnm(m3d::banm_c &anm, float blendFrame) {
    if (anm.getType() == m3d::banm_c::TYPE_ANM_CHR) {
        mCallback.setAnm(0, static_cast<m3d::anmChr_c *>(&anm), blendFrame);
    }

    bmdl_c::setAnm(anm);
}

bool m3d::mdlEx_c::setPartAnm(int idx, m3d::anmChr_c *anm, float blendFrame) {
    mCallback.setPartAnm(idx, anm, blendFrame);
    return true;
}

void m3d::mdlEx_c::setPartNode(u8 idx, const char *nodeName) {
    nw4r::g3d::ResNode node = getResMdl().GetResNode(nodeName);
    setPartNode(idx, node.GetID());
}

void m3d::mdlEx_c::setPartNode(u8 idx, ulong nodeId) {
    nw4r::g3d::ResNode node = getResMdl().GetResNode(nodeId);
    nw4r::g3d::ResNode next = node.GetNextSibling();
    while (!next.IsValid()) {
        node = node.GetParentNode();
        if (!node.IsValid()) {
            break;
        }
        next = node.GetNextSibling();
    }

    ulong end;
    if (next.IsValid()) {
        end = next.GetID();
    } else {
        end = getResMdl().GetResNodeNumEntries();
    }

    for (; nodeId < end; nodeId++) {
        setNodeAnmIdx(idx, nodeId);
    }
}

void m3d::mdlEx_c::setNodeAnmIdx(u8 idx, ulong nodeId) {
    mCallback.mpNodeAnmIdx[nodeId] = idx;
}

void m3d::mdlEx_c::removePartAnm(int idx, float blendFrame) {
    mCallback.removePartAnm(idx, blendFrame);
}

void m3d::mdlEx_c::setCallback(callback_c *callback) {
    mCallback.mpCallback = callback;
}

void m3d::anmChrPart_c::setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, m3d::playMode_e playMode) {
    setAnmAfter(mdl, anmChr, playMode);
}
