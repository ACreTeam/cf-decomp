#include <nw4r/lyt.h>
#include <nw4r/ut.h>

#include <cstring>

/******************************************************************************
 *
 * Private structures
 *
 ******************************************************************************/
namespace nw4r {
namespace lyt {
namespace res {

/******************************************************************************
 * AnimationTarget
 ******************************************************************************/
struct AnimationTarget {
    enum AnimCurve {
        ANIMCURVE_NONE,
        ANIMCURVE_STEP,
        ANIMCURVE_HERMITE,

        ANIMCURVE_MAX
    };

    u8 id;                     // at 0x0
    u8 target;                 // at 0x1
    u8 curveType;              // at 0x2
    u8 PADDING_0x3;            // at 0x3
    u16 keyNum;                // at 0x4
    u8 PADDING_0x6[0x8 - 0x6]; // at 0x6
    u32 keysOffset;            // at 0x8
};

/******************************************************************************
 * StepKey
 ******************************************************************************/
struct StepKey {
    f32 frame;       // at 0x0
    u16 value;       // at 0x4
    u16 PADDING_0x6; // at 0x6
};
/******************************************************************************
 * HermiteKey
 ******************************************************************************/
struct HermiteKey {
    f32 frame; // at 0x0
    f32 value; // at 0x4
    f32 slope; // at 0x8
};

} // namespace res
} // namespace lyt
} // namespace nw4r

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
namespace {

using namespace nw4r;
using namespace nw4r::lyt;

inline bool RIsSame(f32 a, f32 b, f32 tolerance) {
    f32 c = a - b;
    return -tolerance < c && c < tolerance;
}

u16 GetStepCurveValue(f32 frame, const res::StepKey* pKeys, u32 numKey) {
    if (numKey == 1 || frame <= pKeys[0].frame) {
        return pKeys[0].value;
    }

    if (frame >= pKeys[numKey - 1].frame) {
        return pKeys[numKey - 1].value;
    }

    int left = 0;
    int right = numKey - 1;

    while (left != right - 1 && left != right) {
        int center = (left + right) / 2;
        const res::StepKey& rCenterKey = pKeys[center];

        if (frame < rCenterKey.frame) {
            right = center;
        } else {
            left = center;
        }
    }

    if (RIsSame(frame, pKeys[right].frame, 0.001f)) {
        return pKeys[right].value;
    }

    return pKeys[left].value;
}

f32 GetHermiteCurveValue(f32 frame, const res::HermiteKey* pKeys, u32 numKey) {

    if (numKey == 1 || frame <= pKeys[0].frame) {
        return pKeys[0].value;
    }

    if (frame >= pKeys[numKey - 1].frame) {
        return pKeys[numKey - 1].value;
    }

    int left = 0;
    int right = numKey - 1;

    while (left != right - 1 && left != right) {
        int center = (left + right) / 2;

        if (frame <= pKeys[center].frame) {
            right = center;
        } else {
            left = center;
        }
    }

    const res::HermiteKey& rLeftKey = pKeys[left];
    const res::HermiteKey& rRightKey = pKeys[right];

    if (RIsSame(frame, rRightKey.frame, 0.001f)) {
        if (right < numKey - 1 && rRightKey.frame == pKeys[right + 1].frame) {
            return pKeys[right + 1].value;
        }

        return rRightKey.value;
    }

    f32 t1 = frame - rLeftKey.frame;
    f32 t2 = 1.0f / (rRightKey.frame - rLeftKey.frame);

    f32 v0 = rLeftKey.value;
    f32 v1 = rRightKey.value;

    f32 s0 = rLeftKey.slope;
    f32 s1 = rRightKey.slope;

    f32 t1_t1_t2 = t1 * t1 * t2;
    f32 t1_t1_t2_t2 = t1_t1_t2 * t2;
    f32 t1_t1_t1_t2_t2 = t1 * t1_t1_t2_t2;
    f32 t1_t1_t1_t2_t2_t2 = t1_t1_t1_t2_t2 * t2;

    return v0 * (2.0f * t1_t1_t1_t2_t2_t2 - 3.0f * t1_t1_t2_t2 + 1.0f) +
           v1 * (-2.0f * t1_t1_t1_t2_t2_t2 + 3.0f * t1_t1_t2_t2) +
           s0 * (t1_t1_t1_t2_t2 - 2.0f * t1_t1_t2 + t1) +
           s1 * (t1_t1_t1_t2_t2 - t1_t1_t2);
}

void AnimatePaneSRT(Pane* pPane, const res::AnimationInfo* pAnimInfo,
                    const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        const res::HermiteKey* keys = detail::ConvertOffsToPtr<res::HermiteKey>(
            pAnimTarget, pAnimTarget->keysOffset);

        pPane->SetSRTElement(
            pAnimTarget->target,
            GetHermiteCurveValue(frame, keys, pAnimTarget->keyNum));
    }
}

void AnimateVisibility(Pane* pPane, const res::AnimationInfo* pAnimInfo,
                       const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        const res::StepKey* keys = detail::ConvertOffsToPtr<res::StepKey>(
            pAnimTarget, pAnimTarget->keysOffset);

        pPane->SetVisible(
            GetStepCurveValue(frame, keys, pAnimTarget->keyNum) != 0);
    }
}

void AnimateVertexColor(Pane* pPane, const res::AnimationInfo* pAnimInfo,
                        const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        const res::HermiteKey* keys = detail::ConvertOffsToPtr<res::HermiteKey>(
            pAnimTarget, pAnimTarget->keysOffset);

        f32 value = GetHermiteCurveValue(frame, keys, pAnimTarget->keyNum);
        value += 0.5f;

        u8 u8Val;
        OSf32tou8(&value, &u8Val);

        pPane->SetColorElement(pAnimTarget->target, u8Val);
    }
}

void AnimateMaterialColor(Material* pMaterial,
                          const res::AnimationInfo* pAnimInfo,
                          const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        const res::HermiteKey* keys = detail::ConvertOffsToPtr<res::HermiteKey>(
            pAnimTarget, pAnimTarget->keysOffset);

        f32 value = GetHermiteCurveValue(frame, keys, pAnimTarget->keyNum);
        value += 0.5f;

        s16 s16Val;
        OSf32tos16(&value, &s16Val);

        s16Val = ut::Min<s16>(ut::Max<s16>(s16Val, -1024), 1023);

        pMaterial->SetColorElement(pAnimTarget->target, s16Val);
    }
}

void AnimateTextureSRT(Material* pMaterial, const res::AnimationInfo* pAnimInfo,
                       const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        if (pAnimTarget->id < pMaterial->GetTexSRTCap()) {
            const res::HermiteKey* keys =
                detail::ConvertOffsToPtr<res::HermiteKey>(
                    pAnimTarget, pAnimTarget->keysOffset);

            pMaterial->SetTexSRTElement(
                pAnimTarget->id, pAnimTarget->target,
                GetHermiteCurveValue(frame, keys, pAnimTarget->keyNum));
        }
    }
}

void AnimateTexturePattern(Material* pMaterial,
                           const res::AnimationInfo* pAnimInfo,
                           const u32* animTargetOffsets, f32 frame,
                           void** tpls) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        if (pAnimTarget->id < pMaterial->GetTextureNum()) {
            const res::StepKey* keys = detail::ConvertOffsToPtr<res::StepKey>(
                pAnimTarget, pAnimTarget->keysOffset);

            u16 fileIdx = GetStepCurveValue(frame, keys, pAnimTarget->keyNum);

            if (tpls[fileIdx] != NULL) {
                pMaterial->GetTexturePtr(pAnimTarget->id)
                    ->ReplaceImage(static_cast<TPLPalette*>(tpls[fileIdx]), 0);
            }
        }
    }
}

void AnimateIndTexSRT(Material* pMaterial, const res::AnimationInfo* pAnimInfo,
                      const u32* animTargetOffsets, f32 frame) {
    for (int i = 0; i < pAnimInfo->num; i++) {
        const res::AnimationTarget* pAnimTarget =
            detail::ConvertOffsToPtr<res::AnimationTarget>(
                pAnimInfo, animTargetOffsets[i]);

        if (pAnimTarget->id < pMaterial->GetIndTexSRTCap()) {
            const res::HermiteKey* keys =
                detail::ConvertOffsToPtr<res::HermiteKey>(
                    pAnimTarget, pAnimTarget->keysOffset);

            pMaterial->SetIndTexSRTElement(
                pAnimTarget->id, pAnimTarget->target,
                GetHermiteCurveValue(frame, keys, pAnimTarget->keyNum));
        }
    }
}

bool IsBindAnimation(Material* pMaterial, AnimTransform* pAnimTrans) {
#pragma unused(pMaterial)
#pragma unused(pAnimTrans)

    return false;
}

bool IsBindAnimation(Pane* pPane, AnimTransform* pAnimTrans) {
#pragma unused(pPane)
#pragma unused(pAnimTrans)

    return false;
}

} // namespace

namespace nw4r {
namespace lyt {

/******************************************************************************
 *
 * AnimTransform
 *
 ******************************************************************************/
AnimTransform::AnimTransform() : mpRes(NULL), mFrame(0.0f) {}

AnimTransform::~AnimTransform() {}

u16 AnimTransform::GetFrameSize() const {
    return mpRes->frameSize;
}

bool AnimTransform::IsLoopData() const {
    return mpRes->loop != 0;
}

/******************************************************************************
 *
 * AnimTransformBasic
 *
 ******************************************************************************/
AnimTransformBasic::AnimTransformBasic()
    : mpFileResAry(NULL), mAnimLinkAry(NULL), mAnimLinkNum(0) {}

AnimTransformBasic::~AnimTransformBasic() {
    Layout::DeletePrimArray(mAnimLinkAry);
    Layout::DeletePrimArray(mpFileResAry);
}

void AnimTransformBasic::SetResource(const res::AnimationBlock* pRes,
                                     ResourceAccessor* pResAccessor) {
    SetResource(pRes, pResAccessor, pRes->animContNum);
}

void AnimTransformBasic::SetResource(const res::AnimationBlock* pRes,
                                     ResourceAccessor* pResAccessor,
                                     u16 animNum) {
    mpRes = pRes;
    mpFileResAry = NULL;

    if (mpRes->fileNum != 0) {
        mpFileResAry = Layout::NewArray<void*>(mpRes->fileNum);

        if (mpFileResAry != NULL) {
            const u32* fileNameOffsets =
                detail::ConvertOffsToPtr<u32>(mpRes, sizeof(*mpRes));

            for (int i = 0; i < mpRes->fileNum; i++) {
                const char* const fileName =
                    detail::GetStrTableStr(fileNameOffsets, i);

                mpFileResAry[i] = pResAccessor->GetResource(
                    ArcResourceAccessor::RES_TYPE_TEXTURE, fileName, NULL);
            }
        }
    }

    mAnimLinkAry = Layout::NewArray<AnimationLink>(animNum);
    if (mAnimLinkAry != NULL) {
        mAnimLinkNum = animNum;
    }
}

void AnimTransformBasic::Bind(Pane* pPane, bool bRecursive, bool bDisable) {
    AnimationLink* pCrAnimLink = NULL;
    const u32* const animContOffsets =
        detail::ConvertOffsToPtr<u32>(mpRes, mpRes->animContOffsetsOffset);

    for (u16 i = 0; i < mpRes->animContNum; i++) {
        const res::AnimationContent& animCont =
            *detail::ConvertOffsToPtr<res::AnimationContent>(
                mpRes, animContOffsets[i]);

        if (animCont.type == res::AnimationContent::ANIMTYPE_PANE) {
            Pane* const pFindPane =
                pPane->FindPaneByName(animCont.name, bRecursive);

            if (pFindPane != NULL && !IsBindAnimation(pFindPane, this)) {
                pCrAnimLink = Bind(pFindPane, pCrAnimLink, i, bDisable);
                if (pCrAnimLink == NULL) {
                    break;
                }
            }
        } else {
            Material* const pFindMat =
                pPane->FindMaterialByName(animCont.name, bRecursive);

            if (pFindMat != NULL && !IsBindAnimation(pFindMat, this)) {
                pCrAnimLink = Bind(pFindMat, pCrAnimLink, i, bDisable);
                if (pCrAnimLink == NULL) {
                    break;
                }
            }
        }
    }
}

void AnimTransformBasic::Bind(Material* pMaterial, bool bDisable) {
    AnimationLink* pCrAnimLink = NULL;
    const u32* const animContOffsets =
        detail::ConvertOffsToPtr<u32>(mpRes, mpRes->animContOffsetsOffset);

    for (u16 i = 0; i < mpRes->animContNum; i++) {
        const res::AnimationContent& animCont =
            *detail::ConvertOffsToPtr<res::AnimationContent>(
                mpRes, animContOffsets[i]);

        if (animCont.type == res::AnimationContent::ANIMTYPE_MATERIAL) {
            if (detail::EqualsMaterialName(pMaterial->GetName(),
                                           animCont.name) &&
                !IsBindAnimation(pMaterial, this)) {
                pCrAnimLink = Bind(pMaterial, pCrAnimLink, i, bDisable);
                if (pCrAnimLink == NULL) {
                    break;
                }
            }
        }
    }
}

void AnimTransformBasic::Animate(ulong idx, Pane* pPane) {
    ulong animContOffsets =
        detail::ConvertOffsToPtr<u32>(mpRes, mpRes->animContOffsetsOffset)[idx];

    const res::AnimationContent* pAnimCont =
        detail::ConvertOffsToPtr<res::AnimationContent>(mpRes,
                                                        animContOffsets);

    const u32* animInfoOffsets =
        detail::ConvertOffsToPtr<u32>(pAnimCont, sizeof(res::AnimationContent));

    for (int i = 0; i < pAnimCont->num; i++) {
        const res::AnimationInfo* pAnimInfo =
            detail::ConvertOffsToPtr<res::AnimationInfo>(pAnimCont,
                                                         animInfoOffsets[i]);

        const u32* animTargetOffsets = detail::ConvertOffsToPtr<u32>(
            pAnimInfo, sizeof(res::AnimationInfo));

        switch (pAnimInfo->kind) {
        case res::AnimationInfo::SIGNATURE_ANMPANESRT: {
            AnimatePaneSRT(pPane, pAnimInfo, animTargetOffsets, mFrame);
            break;
        }

        case res::AnimationInfo::SIGNATURE_ANMPANEVIS: {
            AnimateVisibility(pPane, pAnimInfo, animTargetOffsets, mFrame);
            break;
        }

        case res::AnimationInfo::SIGNATURE_ANMVTXCLR: {
            AnimateVertexColor(pPane, pAnimInfo, animTargetOffsets, mFrame);
            break;
        }
        }
    }
}

void AnimTransformBasic::Animate(ulong idx, Material* pMaterial) {
    ulong animContOffsets =
        detail::ConvertOffsToPtr<u32>(mpRes, mpRes->animContOffsetsOffset)[idx];

    const res::AnimationContent* pAnimCont =
        detail::ConvertOffsToPtr<res::AnimationContent>(mpRes,
                                                        animContOffsets);

    const u32* animInfoOffsets =
        detail::ConvertOffsToPtr<u32>(pAnimCont, sizeof(res::AnimationContent));

    for (int i = 0; i < pAnimCont->num; i++) {
        const res::AnimationInfo* pAnimInfo =
            detail::ConvertOffsToPtr<res::AnimationInfo>(pAnimCont,
                                                         animInfoOffsets[i]);

        const u32* animTargetOffsets = detail::ConvertOffsToPtr<u32>(
            pAnimInfo, sizeof(res::AnimationInfo));

        switch (pAnimInfo->kind) {
        case res::AnimationInfo::SIGNATURE_ANMMATCLR: {
            AnimateMaterialColor(pMaterial, pAnimInfo, animTargetOffsets,
                                 mFrame);
            break;
        }

        case res::AnimationInfo::SIGNATURE_ANMTEXSRT: {
            AnimateTextureSRT(pMaterial, pAnimInfo, animTargetOffsets, mFrame);
            break;
        }

        case res::AnimationInfo::SIGNATURE_ANMTEXPAT: {
            if (mpFileResAry != NULL) {
                AnimateTexturePattern(pMaterial, pAnimInfo, animTargetOffsets,
                                      mFrame, mpFileResAry);
            }
            break;
        }

        case res::AnimationInfo::SIGNATURE_ANMINDTEXSRT: {
            AnimateIndTexSRT(pMaterial, pAnimInfo, animTargetOffsets, mFrame);
            break;
        }
        }
    }
}

AnimationLink* AnimTransformBasic::FindUnbindLink(AnimationLink* pLink) const {
    if (pLink == NULL) {
        pLink = mAnimLinkAry;
    }

    for (; pLink < &mAnimLinkAry[mAnimLinkNum]; pLink++) {
        if (pLink->GetAnimTransform() == NULL) {
            return pLink;
        }
    }

    return NULL;
}

/******************************************************************************
 *
 * AnimResource
 *
 ******************************************************************************/
AnimResource::AnimResource() {
    Init();
}

void AnimResource::Set(const void* anmResBuf) {
    Init();

    const res::BinaryFileHeader* const pFileHeader =
        static_cast<const res::BinaryFileHeader*>(anmResBuf);

    if (!detail::TestFileHeader(*pFileHeader, Layout::SIGNATURE_ANIMATION)) {
        return;
    }

    if (!detail::TestFileVersion(*pFileHeader)) {
        return;
    }

    mpFileHeader = pFileHeader;

    const res::DataBlockHeader* pDataBlockHead =
        detail::ConvertOffsToPtr<res::DataBlockHeader>(
            mpFileHeader, mpFileHeader->headerSize);

    for (int i = 0; i < mpFileHeader->dataBlocks; i++) {
        switch (detail::GetSignatureInt(pDataBlockHead->kind)) {
        case 'pat1': {
            mpTagBlock =
                reinterpret_cast<const res::AnimationTagBlock*>(pDataBlockHead);
            break;
        }

        case 'pah1': {
            mpShareBlock = reinterpret_cast<const res::AnimationShareBlock*>(
                pDataBlockHead);
            break;
        }

        case 'pai1': {
            mpResBlock =
                reinterpret_cast<const res::AnimationBlock*>(pDataBlockHead);
            break;
        }
        }

        pDataBlockHead = detail::ConvertOffsToPtr<res::DataBlockHeader>(
            pDataBlockHead, pDataBlockHead->size);
    }
}

void AnimResource::Init() {
    mpFileHeader = NULL;
    mpResBlock = NULL;
    mpTagBlock = NULL;
    mpShareBlock = NULL;
}

u16 AnimResource::GetGroupNum() const {
    if (mpTagBlock != NULL) {
        return mpTagBlock->groupNum;
    }

    return 0;
}

const AnimationGroupRef* AnimResource::GetGroupArray() const {
    if (mpTagBlock != NULL) {
        return detail::ConvertOffsToPtr<AnimationGroupRef>(
            mpTagBlock, mpTagBlock->groupsOffset);
    }

    return NULL;
}

bool AnimResource::IsDescendingBind() const {
    if (mpTagBlock != NULL) {
        return detail::TestBit<u8>(mpTagBlock->flag, 0);
    }

    return false;
}

u16 AnimResource::GetAnimationShareInfoNum() const {
    if (mpShareBlock != NULL) {
        return mpShareBlock->shareNum;
    }

    return 0;
}

const AnimationShareInfo* AnimResource::GetAnimationShareInfoArray() const {
    if (mpShareBlock != NULL) {
        return detail::ConvertOffsToPtr<AnimationShareInfo>(
            mpShareBlock, mpShareBlock->animShareInfoOffset);
    }

    return NULL;
}

u16 AnimResource::CalcAnimationNum(Pane* pPane, bool bRecursive) const {
    u16 linkNum = 0;
    const u32* const animContOffsets = detail::ConvertOffsToPtr<u32>(
        mpResBlock, mpResBlock->animContOffsetsOffset);

    for (u16 i = 0; i < mpResBlock->animContNum; i++) {
        const res::AnimationContent& animCont =
            *detail::ConvertOffsToPtr<res::AnimationContent>(
                mpResBlock, animContOffsets[i]);

        if (animCont.type == res::AnimationContent::ANIMTYPE_PANE) {
            Pane* const pFindPane =
                pPane->FindPaneByName(animCont.name, bRecursive);

            if (pFindPane != NULL) {
                linkNum++;
            }
        } else {
            Material* const pFindMat =
                pPane->FindMaterialByName(animCont.name, bRecursive);

            if (pFindMat != NULL) {
                linkNum++;
            }
        }
    }

    return linkNum;
}

u16 AnimResource::CalcAnimationNum(Group* pGroup, bool bRecursive) const {
    u16 linkNum = 0;
    detail::PaneLinkList& paneList = pGroup->GetPaneList();

    for (detail::PaneLinkList::Iterator it = paneList.GetBeginIter();
         it != paneList.GetEndIter(); it++) {
        linkNum += CalcAnimationNum(it->mTarget, bRecursive);
    }

    return linkNum;
}

namespace detail {

/******************************************************************************
 *
 * AnimPaneTree
 *
 ******************************************************************************/
u16 AnimPaneTree::FindAnimContent(const res::AnimationBlock* pAnimBlock,
                                  const char* animContName, u8 animContType) {
    const u32* const animContOffsets = detail::ConvertOffsToPtr<u32>(
        pAnimBlock, pAnimBlock->animContOffsetsOffset);

    for (u16 i = 0; i < pAnimBlock->animContNum; i++) {
        const res::AnimationContent* const pAnimCont =
            detail::ConvertOffsToPtr<res::AnimationContent>(
                pAnimBlock, animContOffsets[i]);

        if (pAnimCont->type == animContType &&
            detail::EqualsMaterialName(pAnimCont->name, animContName)) {
            return i;
        }
    }

    return NOBIND;
}

void AnimPaneTree::Init() {
    mLinkNum = 0;
    mAnimPaneIdx = 0;
    mAnimMatCnt = 0;

    for (u8 i = 0; i < MATERIAL_NUM_MAX; i++) {
        mAnimMatIdxs[i] = 0;
    }
}

void AnimPaneTree::Set(Pane* pTargetPane, const AnimResource& animRes) {
    u16 linkNum = 0;
    const res::AnimationBlock* pAnimBlock = animRes.GetResourceBlock();

    u16 animContIdx =
        FindAnimContent(pAnimBlock, pTargetPane->GetName(),
                        res::AnimationContent::ANIMTYPE_PANE);
    if (animContIdx != NOBIND) {
        linkNum++;
    }

    u8 animMatCnt = pTargetPane->GetMaterialNum();
    u16 animMatIdxs[MATERIAL_NUM_MAX];

    for (u8 i = 0; i < animMatCnt; i++) {
        animMatIdxs[i] =
            FindAnimContent(pAnimBlock, pTargetPane->GetMaterial(i)->GetName(),
                            res::AnimationContent::ANIMTYPE_MATERIAL);
        if (animMatIdxs[i] != NOBIND) {
            linkNum++;
        }
    }

    if (linkNum != 0) {
        mAnimRes = animRes;
        mAnimPaneIdx = animContIdx;
        mAnimMatCnt = animMatCnt;

        for (u8 i = 0; i < animMatCnt; i++) {
            mAnimMatIdxs[i] = animMatIdxs[i];
        }

        mLinkNum = linkNum;
    }
}

AnimTransform* AnimPaneTree::Bind(Layout* pLayout, Pane* pTargetPane,
                                  ResourceAccessor* pResAccessor) const {
    AnimTransformBasic* const pAnimTrans =
        static_cast<AnimTransformBasic*>(pLayout->CreateAnimTransform());

    pAnimTrans->SetResource(mAnimRes.GetResourceBlock(), pResAccessor,
                            mLinkNum);

    AnimationLink* pCrAnimLink = NULL;

    if (mAnimPaneIdx != NOBIND) {
        pCrAnimLink =
            pAnimTrans->Bind(pTargetPane, pCrAnimLink, mAnimPaneIdx, true);
    }

    u8 animMatMax = ut::Min<u8>(mAnimMatCnt, pTargetPane->GetMaterialNum());

    for (u8 i = 0; i < animMatMax; i++) {
        if (mAnimMatIdxs[i] != NOBIND) {
            Material* const pMaterial = pTargetPane->GetMaterial(i);
            pCrAnimLink =
                pAnimTrans->Bind(pMaterial, pCrAnimLink, mAnimMatIdxs[i], true);
        }
    }

    return pAnimTrans;
}

/******************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
AnimationLink* FindAnimationLink(AnimationLinkList* pAnimList,
                                 AnimTransform* pAnimTrans) {
    for (AnimationLinkList::Iterator it = pAnimList->GetBeginIter();
         it != pAnimList->GetEndIter(); it++) {
        if (pAnimTrans == it->GetAnimTransform()) {
            return &*it;
        }
    }

    return NULL;
}

AnimationLink* FindAnimationLink(AnimationLinkList* pAnimList,
                                 const AnimResource& animRes) {
    for (AnimationLinkList::Iterator it = pAnimList->GetBeginIter();
         it != pAnimList->GetEndIter(); it++) {
        if (animRes.GetResourceBlock() ==
            it->GetAnimTransform()->GetAnimResource()) {
            return &*it;
        }
    }

    return NULL;
}

void UnbindAnimationLink(AnimationLinkList* pAnimList,
                         AnimTransform* pAnimTrans) {
    for (AnimationLinkList::Iterator it = pAnimList->GetBeginIter();
         it != pAnimList->GetEndIter();) {
        AnimationLinkList::Iterator currIt = it++;

        if (pAnimTrans == NULL || currIt->GetAnimTransform() == pAnimTrans) {
            pAnimList->Erase(currIt);
            currIt->Reset();
        }
    }
}

} // namespace detail
} // namespace lyt
} // namespace nw4r
