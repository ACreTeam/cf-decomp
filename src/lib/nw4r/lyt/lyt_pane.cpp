#include <nw4r/lyt.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>

#include <revolution/gx.h>
#include <revolution/mtx.h>

#include <cstring>

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
namespace {

using namespace nw4r;
using namespace nw4r::lyt;

void ReverseYAxis(math::MTX34* pMtx) {
    pMtx->m[0][1] = -pMtx->m[0][1];
    pMtx->m[1][1] = -pMtx->m[1][1];
    pMtx->m[2][1] = -pMtx->m[2][1];
}

} // namespace

namespace nw4r {
namespace lyt {

NW4R_UT_RTTI_DEF_BASE(Pane);

namespace detail {

/******************************************************************************
 *
 * PaneBase
 *
 ******************************************************************************/
PaneBase::PaneBase() {}

PaneBase::~PaneBase() {}

} // namespace detail

/******************************************************************************
 *
 * Pane
 *
 ******************************************************************************/
Pane::Pane(const res::Pane* pBlock) {
    Init();

    mBasePosition = pBlock->basePosition;

    SetName(pBlock->name);
    SetUserData(pBlock->userData);

    mTranslate = pBlock->translate;
    mRotate = pBlock->rotate;
    mScale = pBlock->scale;
    mSize = pBlock->size;

    mAlpha = pBlock->alpha;
    mGlbAlpha = mAlpha;
    mFlag = pBlock->flag;
}

void Pane::Init() {
    mpParent = NULL;
    mpMaterial = NULL;
    mbUserAllocated = false;
    mpExtUserDataList = NULL;
}

Pane::~Pane() {
    for (PaneList::Iterator it = mChildList.GetBeginIter();
         it != mChildList.GetEndIter();) {
        PaneList::Iterator currIt = it++;
        mChildList.Erase(currIt);

        if (!currIt->IsUserAllocated()) {
            Layout::DeleteObj(&*currIt);
        }
    }

    UnbindAnimationSelf(NULL);

    if (mpMaterial != NULL && !mpMaterial->IsUserAllocated()) {
        Layout::DeleteObj(mpMaterial);
    }
}

void Pane::SetName(const char* pName) {
    std::strncpy(mName, pName, NW4R_LYT_RES_NAME_LEN);
    mName[NW4R_LYT_RES_NAME_LEN] = '\0';
}

void Pane::SetUserData(const char* pUserData) {
    std::strncpy(mUserData, pUserData, NW4R_LYT_PANE_USERDATA_LEN);
    mUserData[NW4R_LYT_PANE_USERDATA_LEN] = '\0';
}

void Pane::AppendChild(Pane* pChild) {
    InsertChild(mChildList.GetEndIter(), pChild);
}

void Pane::InsertChild(PaneList::Iterator next, Pane* pChild) {
    mChildList.Insert(next, pChild);
    pChild->mpParent = this;
}

void Pane::RemoveChild(Pane* pChild) {
    mChildList.Erase(pChild);
    pChild->mpParent = NULL;
}

ut::Rect Pane::GetPaneRect() const {
    ut::Rect ret;
    math::VEC2 basePt = GetVtxPos();

    ret.left = basePt.x;
    ret.top = basePt.y;
    ret.right = basePt.x + mSize.width;
    ret.bottom = basePt.y - mSize.height;

    return ret;
}

ut::Rect Pane::GetPaneRect(const DrawInfo& drawInfo) const {
#pragma unused(drawInfo)

    return GetPaneRect();
}

ut::Color Pane::GetVtxColor(u32 idx) const {
#pragma unused(idx)

    return ut::Color::WHITE;
}

void Pane::SetVtxColor(u32 idx, ut::Color color) {
#pragma unused(idx)
#pragma unused(color)
}

u8 Pane::GetColorElement(ulong idx) const {
    switch (idx) {
    case ANIMTARGET_PANE_COLOR_ALPHA:
        return mAlpha;
    default:
        return GetVtxColorElement(idx);
    }
}

void Pane::SetColorElement(ulong idx, u8 value) {
    switch (idx) {
    case ANIMTARGET_PANE_COLOR_ALPHA:
        mAlpha = value;
        break;
    default:
        SetVtxColorElement(idx, value);
        break;
    }
}

u8 Pane::GetVtxColorElement(u32 idx) const {
#pragma unused(idx)

    return 0xFF;
}

void Pane::SetVtxColorElement(u32 idx, u8 value) {
#pragma unused(idx)
#pragma unused(value)
}

Pane* Pane::FindPaneByName(const char* findName, bool bRecursive) {
    if (detail::EqualsResName(mName, findName)) {
        return this;
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter();
             it != mChildList.GetEndIter(); it++) {
            Pane* pPane = it->FindPaneByName(findName, bRecursive);

            if (pPane != NULL) {
                return pPane;
            }
        }
    }

    return NULL;
}

Material* Pane::FindMaterialByName(const char* findName, bool bRecursive) {
    if (mpMaterial != NULL &&
        detail::EqualsMaterialName(mpMaterial->GetName(), findName)) {

        return mpMaterial;
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter();
             it != mChildList.GetEndIter(); it++) {
            Material* pMat = it->FindMaterialByName(findName, bRecursive);

            if (pMat != NULL) {
                return pMat;
            }
        }
    }

    return NULL;
}

void Pane::CalculateMtx(const DrawInfo& rInfo) {
    if (!IsVisible() && !rInfo.IsInvisiblePaneCalculateMtx()) {
        return;
    }

    math::MTX34 mtx1, mtx2;
    math::MTX34 rotateMtx;

    math::VEC2 scale = mScale;
    if (rInfo.IsLocationAdjust() && IsLocationAdjust()) {
        scale.x *= rInfo.GetLocationAdjustScale().x;
        scale.y *= rInfo.GetLocationAdjustScale().y;
    }

    PSMTXScale(mtx2, scale.x, scale.y, 1.0f);

    PSMTXRotRad(rotateMtx, 'x', NW4R_MATH_DEG_TO_RAD(mRotate.x));
    PSMTXConcat(rotateMtx, mtx2, mtx1);

    PSMTXRotRad(rotateMtx, 'y', NW4R_MATH_DEG_TO_RAD(mRotate.y));
    PSMTXConcat(rotateMtx, mtx1, mtx2);

    PSMTXRotRad(rotateMtx, 'z', NW4R_MATH_DEG_TO_RAD(mRotate.z));
    PSMTXConcat(rotateMtx, mtx2, mtx1);

    PSMTXTransApply(mtx1, mMtx, mTranslate.x, mTranslate.y, mTranslate.z);

    if (mpParent != NULL) {
        math::MTX34Mult(&mGlbMtx, &mpParent->mGlbMtx, &mMtx);
    } else if (rInfo.IsMultipleViewMtxOnDraw()) {
        mGlbMtx = mMtx;
    } else {
        math::MTX34Mult(&mGlbMtx, &rInfo.GetViewMtx(), &mMtx);
    }

    if (rInfo.IsInfluencedAlpha() && mpParent != NULL) {
        mGlbAlpha = static_cast<u8>(mAlpha * rInfo.GetGlobalAlpha());
    } else {
        mGlbAlpha = mAlpha;
    }

    f32 glbAlpha = rInfo.GetGlobalAlpha();
    bool influenced = rInfo.IsInfluencedAlpha();
    bool modifyInfo = IsInfluencedAlpha() && mAlpha != 255;

    if (modifyInfo) {
        DrawInfo& rMtInfo = const_cast<DrawInfo&>(rInfo);
        rMtInfo.SetGlobalAlpha(glbAlpha * mAlpha * (1.0f / 255.0f));
        rMtInfo.SetInfluencedAlpha(true);
    }

    CalculateMtxChild(rInfo);

    if (modifyInfo) {
        DrawInfo& rMtInfo = const_cast<DrawInfo&>(rInfo);
        rMtInfo.SetGlobalAlpha(glbAlpha);
        rMtInfo.SetInfluencedAlpha(influenced);
    }
}

void Pane::CalculateMtxChild(const DrawInfo& rInfo) {
    NW4R_UT_LINKLIST_FOREACH (it, mChildList, { it->CalculateMtx(rInfo); })
}

void Pane::Draw(const DrawInfo& rInfo) {
    if (!IsVisible()) {
        return;
    }

    DrawSelf(rInfo);
    NW4R_UT_LINKLIST_FOREACH (it, mChildList, { it->Draw(rInfo); })
}

void Pane::DrawSelf(const DrawInfo& rInfo) {
#pragma unused(rInfo)
    // Debug draw stripped out
}

void Pane::Animate(u32 option) {
    AnimateSelf(option);

    if (IsVisible() || !(option & ANIMOPTION_SKIP_INVISIBLE)) {
        NW4R_UT_LINKLIST_FOREACH (it, mChildList, { it->Animate(option); })
    }
}

void Pane::AnimateSelf(u32 option) {
    NW4R_UT_LINKLIST_FOREACH (it, mAnimList, {
        if (!it->IsEnable()) {
            continue;
        }

        AnimTransform* pAnimTrans = it->GetAnimTransform();
        pAnimTrans->Animate(it->GetIndex(), this);
    })

    if (IsVisible() || !(option & ANIMOPTION_SKIP_INVISIBLE)) {
        if (mpMaterial != NULL) {
            mpMaterial->Animate();
        }
    }
}

void Pane::BindAnimation(AnimTransform* pAnimTrans, bool recursive,
                         bool bDisable) {
    pAnimTrans->Bind(this, recursive, bDisable);
}

void Pane::UnbindAnimation(AnimTransform* pAnimTrans, bool recursive) {
    UnbindAnimationSelf(pAnimTrans);

    if (recursive) {
        NW4R_UT_LINKLIST_FOREACH (it, mChildList, 
            { it->UnbindAnimation(pAnimTrans, recursive); })
    }
}

void Pane::UnbindAllAnimation(bool recursive) {
    UnbindAnimation(NULL, recursive);
}

void Pane::UnbindAnimationSelf(AnimTransform* pAnimTrans) {
    if (mpMaterial != NULL) {
        mpMaterial->UnbindAnimation(pAnimTrans);
    }

    detail::UnbindAnimationLink(&mAnimList, pAnimTrans);
}

void Pane::AddAnimationLink(AnimationLink* pAnimationLink) {
    mAnimList.PushBack(pAnimationLink);
}

AnimationLink* Pane::FindAnimationLinkSelf(AnimTransform* pAnimTrans) {
    return detail::FindAnimationLink(&mAnimList, pAnimTrans);
}

AnimationLink* Pane::FindAnimationLinkSelf(const AnimResource& animRes) {
    return detail::FindAnimationLink(&mAnimList, animRes);
}

void Pane::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable,
                              bool bRecursive) {
    AnimationLink* pAnimLink = FindAnimationLinkSelf(pAnimTrans);
    if (pAnimLink != NULL) {
        pAnimLink->SetEnable(bEnable);
    }

    u8 materialNum = GetMaterialNum();
    for (u8 i = 0; i < materialNum; i++) {
        GetMaterial(i)->SetAnimationEnable(pAnimTrans, bEnable);
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter();
             it != mChildList.GetEndIter(); it++) {
            it->SetAnimationEnable(pAnimTrans, bEnable, bRecursive);
        }
    }
}

void Pane::SetAnimationEnable(const AnimResource& animRes, bool bEnable,
                              bool bRecursive) {
    AnimationLink* pAnimLink = FindAnimationLinkSelf(animRes);
    if (pAnimLink != NULL) {
        pAnimLink->SetEnable(bEnable);
    }

    u8 materialNum = GetMaterialNum();
    for (u8 i = 0; i < materialNum; i++) {
        GetMaterial(i)->SetAnimationEnable(animRes, bEnable);
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter();
             it != mChildList.GetEndIter(); it++) {
            it->SetAnimationEnable(animRes, bEnable, bRecursive);
        }
    }
}

void Pane::LoadMtx(const DrawInfo& drawInfo) {
    math::MTX34 mtx;
    math::MTX34* mtxPtr;

    if (drawInfo.IsMultipleViewMtxOnDraw()) {
        PSMTXConcat(drawInfo.GetViewMtx(), mGlbMtx, mtx);
        mtxPtr = &mtx;
    } else {
        mtxPtr = &mGlbMtx;
    }

    GXLoadPosMtxImm(*mtxPtr, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

math::VEC2 Pane::GetVtxPos() const {
    math::VEC2 basePt(0.0f, 0.0f);

    switch (mBasePosition % HORIZONTALPOSITION_MAX) {
    default:
    case HORIZONTALPOSITION_LEFT: {
        basePt.x = 0.0f;
        break;
    }

    case HORIZONTALPOSITION_CENTER: {
        basePt.x = -mSize.width / 2;
        break;
    }

    case HORIZONTALPOSITION_RIGHT: {
        basePt.x = -mSize.width;
        break;
    }
    }

    switch (mBasePosition / HORIZONTALPOSITION_MAX) {
    default:
    case VERTICALPOSITION_TOP: {
        basePt.y = 0.0f;
        break;
    }

    case VERTICALPOSITION_CENTER: {
        basePt.y = mSize.height / 2;
        break;
    }

    case VERTICALPOSITION_BOTTOM: {
        basePt.y = mSize.height;
        break;
    }
    }

    return basePt;
}

u8 Pane::GetMaterialNum() const {
    return mpMaterial != NULL;
}

Material* Pane::GetMaterial() const {
    return mpMaterial;
}

Material* Pane::GetMaterial(ulong idx) const {
    if (idx == 0) {
        return GetMaterial();
    }

    return NULL;
}

} // namespace lyt
} // namespace nw4r
