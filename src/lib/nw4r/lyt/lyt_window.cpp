#include <nw4r/lyt.h>

/******************************************************************************
 *
 * Texture-flip implementation
 *
 ******************************************************************************/
namespace {

using namespace nw4r;
using namespace nw4r::lyt;

/******************************************************************************
 * TextureFlipType
 ******************************************************************************/
enum TextureFlipType {
    TEXTUREFLIP_NONE,
    TEXTUREFLIP_H,
    TEXTUREFLIP_V,
    TEXTUREFLIP_90,
    TEXTUREFLIP_180,
    TEXTUREFLIP_270,

    TEXTUREFLIP_MAX
};

/******************************************************************************
 * TextureFlipIndex
 ******************************************************************************/
enum TextureFlipIndex {
    FLIPINDEX_X,
    FLIPINDEX_Y,

    FLIPINDEX_MAX
};

/******************************************************************************
 * TextureFlipInfo
 ******************************************************************************/
struct TextureFlipInfo {
    u8 coords[VERTEXCOLOR_MAX][FLIPINDEX_MAX]; // at 0x0
    u8 idx[FLIPINDEX_MAX];                     // at 0x8
};

// @typo
const TextureFlipInfo& GetTexutreFlipInfo(u8 textureFlip) {
    static TextureFlipInfo flipInfos[TEXTUREFLIP_MAX] = {
        {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {0, 1}}, // TEXTUREFLIP_NONE
        {{{1, 0}, {0, 0}, {1, 1}, {0, 1}}, {0, 1}}, // TEXTUREFLIP_H
        {{{0, 1}, {1, 1}, {0, 0}, {1, 0}}, {0, 1}}, // TEXTUREFLIP_V
        {{{0, 1}, {0, 0}, {1, 1}, {1, 0}}, {1, 0}}, // TEXTUREFLIP_90
        {{{1, 1}, {0, 1}, {1, 0}, {0, 0}}, {0, 1}}, // TEXTUREFLIP_180
        {{{1, 0}, {1, 1}, {0, 0}, {0, 1}}, {1, 0}}, // TEXTUREFLIP_270
    };

    return flipInfos[textureFlip];
}

/******************************************************************************
 * Top-left frame
 ******************************************************************************/
void GetLTFrameSize(math::VEC2* pPoint, Size* pSize, const math::VEC2& rBase,
                    const Size& rWindowSize,
                    const WindowFrameSize& rFrameSize) {

    *pPoint = rBase;

    pSize->width = rWindowSize.width - rFrameSize.r;
    pSize->height = rFrameSize.t;
}

void GetLTTexCoord(math::VEC2* pTexCoords, const Size& rSize,
                   const Size& rTexSize, u8 flipType) {

    const TextureFlipInfo& rInfo = GetTexutreFlipInfo(flipType);

    const int ix = rInfo.idx[FLIPINDEX_X];
    const int iy = rInfo.idx[FLIPINDEX_Y];

    const math::VEC2 texSize(rTexSize.width, rTexSize.height);

    pTexCoords[VERTEXCOLOR_LT][ix] = pTexCoords[VERTEXCOLOR_LB][ix] =
        rInfo.coords[VERTEXCOLOR_LT][ix];

    pTexCoords[VERTEXCOLOR_LT][iy] = pTexCoords[VERTEXCOLOR_RT][iy] =
        rInfo.coords[VERTEXCOLOR_LT][iy];

    // clang-format off
    pTexCoords[VERTEXCOLOR_RB][ix] = pTexCoords[VERTEXCOLOR_RT][ix] =
        rInfo.coords[VERTEXCOLOR_LT][ix] + rSize.width /
        ((rInfo.coords[VERTEXCOLOR_RT][ix] - rInfo.coords[VERTEXCOLOR_LT][ix]) * texSize[ix]);
    
    pTexCoords[VERTEXCOLOR_RB][iy] = pTexCoords[VERTEXCOLOR_LB][iy] =
        rInfo.coords[VERTEXCOLOR_LT][iy] + rSize.height /
        ((rInfo.coords[VERTEXCOLOR_LB][iy] - rInfo.coords[VERTEXCOLOR_LT][iy]) * texSize[iy]);
    // clang-format on
}

/******************************************************************************
 * Top-right frame
 ******************************************************************************/
void GetRTFrameSize(math::VEC2* pPoint, Size* pSize, const math::VEC2& rBase,
                    const Size& rWindowSize,
                    const WindowFrameSize& rFrameSize) {

    *pPoint = math::VEC2(rBase.x + rWindowSize.width - rFrameSize.r, rBase.y);

    pSize->width = rFrameSize.r;
    pSize->height = rWindowSize.height - rFrameSize.b;
}

void GetRTTexCoord(math::VEC2* pTexCoords, const Size& rSize,
                   const Size& rTexSize, u8 flipType) {

    const TextureFlipInfo& rInfo = GetTexutreFlipInfo(flipType);

    const int ix = rInfo.idx[FLIPINDEX_X];
    const int iy = rInfo.idx[FLIPINDEX_Y];

    const math::VEC2 texSize(rTexSize.width, rTexSize.height);

    pTexCoords[VERTEXCOLOR_RT][ix] = pTexCoords[VERTEXCOLOR_RB][ix] =
        rInfo.coords[VERTEXCOLOR_RT][ix];

    pTexCoords[VERTEXCOLOR_RT][iy] = pTexCoords[VERTEXCOLOR_LT][iy] =
        rInfo.coords[VERTEXCOLOR_RT][iy];

    // clang-format off
    pTexCoords[VERTEXCOLOR_LB][ix] = pTexCoords[VERTEXCOLOR_LT][ix] =
        rInfo.coords[VERTEXCOLOR_RT][ix] + rSize.width /
        ((rInfo.coords[VERTEXCOLOR_LT][ix] - rInfo.coords[VERTEXCOLOR_RT][ix]) * texSize[ix]);
    
    pTexCoords[VERTEXCOLOR_LB][iy] = pTexCoords[VERTEXCOLOR_RB][iy] =
        rInfo.coords[VERTEXCOLOR_RT][iy] + rSize.height /
        ((rInfo.coords[VERTEXCOLOR_RB][iy] - rInfo.coords[VERTEXCOLOR_RT][iy]) * texSize[iy]);
    // clang-format on
}

/******************************************************************************
 * Bottom-left frame
 ******************************************************************************/
void GetLBFrameSize(math::VEC2* pPoint, Size* pSize, const math::VEC2& rBase,
                    const Size& rWindowSize,
                    const WindowFrameSize& rFrameSize) {

    *pPoint = math::VEC2(rBase.x, rBase.y - rFrameSize.t);

    pSize->width = rFrameSize.l;
    pSize->height = rWindowSize.height - rFrameSize.t;
}

void GetLBTexCoord(math::VEC2* pTexCoords, const Size& rSize,
                   const Size& rTexSize, u8 flipType) {

    const TextureFlipInfo& rInfo = GetTexutreFlipInfo(flipType);

    const int ix = rInfo.idx[FLIPINDEX_X];
    const int iy = rInfo.idx[FLIPINDEX_Y];

    const math::VEC2 texSize(rTexSize.width, rTexSize.height);

    pTexCoords[VERTEXCOLOR_LB][ix] = pTexCoords[VERTEXCOLOR_LT][ix] =
        rInfo.coords[VERTEXCOLOR_LB][ix];

    pTexCoords[VERTEXCOLOR_LB][iy] = pTexCoords[VERTEXCOLOR_RB][iy] =
        rInfo.coords[VERTEXCOLOR_LB][iy];

    // clang-format off
    pTexCoords[VERTEXCOLOR_RT][ix] = pTexCoords[VERTEXCOLOR_RB][ix] =
        rInfo.coords[VERTEXCOLOR_LB][ix] + rSize.width /
        ((rInfo.coords[VERTEXCOLOR_RB][ix] - rInfo.coords[VERTEXCOLOR_LB][ix]) * texSize[ix]);
    
    pTexCoords[VERTEXCOLOR_RT][iy] = pTexCoords[VERTEXCOLOR_LT][iy] =
        rInfo.coords[VERTEXCOLOR_LB][iy] + rSize.height / 
        ((rInfo.coords[VERTEXCOLOR_LT][iy] - rInfo.coords[VERTEXCOLOR_LB][iy]) * texSize[iy]);
    // clang-format on
}

/******************************************************************************
 * Bottom-right frame
 ******************************************************************************/
void GetRBFrameSize(math::VEC2* pPoint, Size* pSize, const math::VEC2& rBase,
                    const Size& rWindowSize,
                    const WindowFrameSize& rFrameSize) {

    *pPoint = math::VEC2(rBase.x + rFrameSize.l,
                         rBase.y - rWindowSize.height + rFrameSize.b);

    pSize->width = rWindowSize.width - rFrameSize.l;
    pSize->height = rFrameSize.b;
}

void GetRBTexCoord(math::VEC2* pTexCoords, const Size& rSize,
                   const Size& rTexSize, u8 flipType) {

    const TextureFlipInfo& rInfo = GetTexutreFlipInfo(flipType);

    const int ix = rInfo.idx[FLIPINDEX_X];
    const int iy = rInfo.idx[FLIPINDEX_Y];

    const math::VEC2 texSize(rTexSize.width, rTexSize.height);

    pTexCoords[VERTEXCOLOR_RB][ix] = pTexCoords[VERTEXCOLOR_RT][ix] =
        rInfo.coords[VERTEXCOLOR_RB][ix];

    pTexCoords[VERTEXCOLOR_RB][iy] = pTexCoords[VERTEXCOLOR_LB][iy] =
        rInfo.coords[VERTEXCOLOR_RB][iy];

    // clang-format off
    pTexCoords[VERTEXCOLOR_LT][ix] = pTexCoords[VERTEXCOLOR_LB][ix] =
        rInfo.coords[VERTEXCOLOR_RB][ix] + rSize.width /
        ((rInfo.coords[VERTEXCOLOR_LB][ix] - rInfo.coords[VERTEXCOLOR_RB][ix]) * texSize[ix]);

    pTexCoords[VERTEXCOLOR_LT][iy] = pTexCoords[VERTEXCOLOR_RT][iy] =
        rInfo.coords[VERTEXCOLOR_RB][iy] + rSize.height /
        ((rInfo.coords[VERTEXCOLOR_RT][iy] - rInfo.coords[VERTEXCOLOR_RB][iy]) * texSize[iy]);
    // clang-format on
}

} // namespace

namespace nw4r {
namespace lyt {

NW4R_UT_RTTI_DEF_DERIVED(Window, Pane);

/******************************************************************************
 *
 * Window
 *
 ******************************************************************************/
Window::Frame::~Frame() {
    Layout::DeleteObj(pMaterial);
}

Window::Window(const res::Window* pBlock, const ResBlockSet& resBlockSet)
    : Pane(pBlock) {
    const res::WindowContent* const pResContent =
        detail::ConvertOffsToPtr<res::WindowContent>(pBlock,
                                                     pBlock->contentOffset);

    u8 texCoordNum = ut::Min<u8>(pResContent->texCoordNum, GX_MAX_TEXCOORD);

    InitContent(texCoordNum);

    const u32* const matOffsTbl = detail::ConvertOffsToPtr<u32>(
        resBlockSet.pMaterialList, sizeof(res::MaterialList));

    mContentInflation = pBlock->inflation;

    for (int i = 0; i < VERTEXCOLOR_MAX; i++) {
        mContent.vtxColors[i] = pResContent->vtxCols[i];
    }

    if (texCoordNum != 0 && !mContent.texCoordAry.IsEmpty()) {
        mContent.texCoordAry.Copy(&pResContent[1], texCoordNum);
    }

    const res::Material* const pResMaterial =
        detail::ConvertOffsToPtr<res::Material>(
            resBlockSet.pMaterialList, matOffsTbl[pResContent->materialIdx]);

    mpMaterial = Layout::NewObj<Material>(pResMaterial, resBlockSet);

    mFrameNum = 0;
    mFrames = NULL;

    if (pBlock->frameNum != 0) {
        InitFrame(pBlock->frameNum);

        const u32* const frameOffsetTable = detail::ConvertOffsToPtr<u32>(
            pBlock, pBlock->frameOffsetTableOffset);

        for (int i = 0; i < mFrameNum; i++) {
            const res::WindowFrame* const pResWindowFrame =
                detail::ConvertOffsToPtr<res::WindowFrame>(pBlock,
                                                           frameOffsetTable[i]);

            mFrames[i].textureFlip = pResWindowFrame->textureFlip;

            const res::Material* const pResMaterial =
                detail::ConvertOffsToPtr<res::Material>(
                    resBlockSet.pMaterialList,
                    matOffsTbl[pResWindowFrame->materialIdx]);

            mFrames[i].pMaterial =
                Layout::NewObj<Material>(pResMaterial, resBlockSet);
        }
    }
}

void Window::InitContent(u8 texNum) {
    if (texNum != 0) {
        ReserveTexCoord(texNum);
    }
}

void Window::InitFrame(u8 frameNum) {
    mFrameNum = 0;
    mFrames = Layout::NewArray<Frame>(frameNum);

    if (mFrames != NULL) {
        mFrameNum = frameNum;
    }
}

Window::~Window() {
    Layout::DeleteArray(mFrames, mFrameNum);

    if (mpMaterial != NULL && !mpMaterial->IsUserAllocated()) {
        Layout::DeleteObj(mpMaterial);
        mpMaterial = NULL;
    }

    mContent.texCoordAry.Free();
}

void Window::ReserveTexCoord(u8 num) {
    mContent.texCoordAry.Reserve(num);
}

Material* Window::FindMaterialByName(const char* findName, bool bRecursive) {
    if (mpMaterial != NULL &&
        detail::EqualsMaterialName(mpMaterial->GetName(), findName)) {
        return mpMaterial;
    }

    for (int i = 0; i < mFrameNum; i++) {
        if (detail::EqualsMaterialName(mFrames[i].pMaterial->GetName(),
                                       findName)) {
            return mFrames[i].pMaterial;
        }
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

ut::Color Window::GetVtxColor(u32 idx) const {
    return mContent.vtxColors[idx];
}

void Window::SetVtxColor(u32 idx, ut::Color color) {
    mContent.vtxColors[idx] = color;
}

u8 Window::GetVtxColorElement(u32 idx) const {
    return detail::GetVtxColorElement(mContent.vtxColors, idx);
}

void Window::SetVtxColorElement(u32 idx, u8 value) {
    detail::SetVtxColorElement(mContent.vtxColors, idx, value);
}

void Window::DrawSelf(const DrawInfo& rInfo) {
    LoadMtx(rInfo);

    WindowFrameSize size = GetFrameSize(mFrameNum, mFrames);
    math::VEC2 base = GetVtxPos();
    DrawContent(base, size, mGlbAlpha);

    switch (mFrameNum) {
    case 1: {
        DrawFrame(base, mFrames[WINDOWFRAME_LT], size, mGlbAlpha);
        break;
    }

    case 4: {
        DrawFrame4(base, mFrames, size, mGlbAlpha);
        break;
    }

    case 8: {
        DrawFrame8(base, mFrames, size, mGlbAlpha);
        break;
    }
    }
}

void Window::AnimateSelf(u32 option) {
    Pane::AnimateSelf(option);

    if (IsVisible() || !(option & ANIMOPTION_SKIP_INVISIBLE)) {
        for (int i = 0; i < mFrameNum; i++) {
            mFrames[i].pMaterial->Animate();
        }
    }
}

void Window::UnbindAnimationSelf(AnimTransform* pAnimTrans) {
    for (int i = 0; i < mFrameNum; i++) {
        mFrames[i].pMaterial->UnbindAnimation(pAnimTrans);
    }

    Pane::UnbindAnimationSelf(pAnimTrans);
}

void Window::DrawContent(const math::VEC2& basePt,
                         const WindowFrameSize& frameSize, u8 alpha) {
    bool bUseVtxCol = mpMaterial->SetupGX(
        detail::IsModulateVertexColor(mContent.vtxColors, alpha), alpha);

    detail::SetVertexFormat(bUseVtxCol, mContent.texCoordAry.GetSize());

    detail::DrawQuad(
        math::VEC2(basePt.x + frameSize.l - mContentInflation.l,
                   basePt.y - frameSize.t + mContentInflation.t),
        Size(mSize.width - frameSize.l + mContentInflation.l - frameSize.r +
                 mContentInflation.r,
             mSize.height - frameSize.t + mContentInflation.t - frameSize.b +
                 mContentInflation.b),
        mContent.texCoordAry.GetSize(), mContent.texCoordAry.GetArray(),
        bUseVtxCol ? mContent.vtxColors : NULL, alpha);
}

void Window::DrawFrame(const math::VEC2& basePt, const Frame& frame,
                       const WindowFrameSize& frameSize, u8 alpha) {
    if (frame.pMaterial->GetTextureNum() == 0) {
        return;
    }

    const bool bUseVtxCol = frame.pMaterial->SetupGX(
        detail::IsModulateVertexColor(NULL, alpha), alpha);

    detail::SetVertexFormat(bUseVtxCol, 1);

    const Size texSize = detail::GetTextureSize(frame.pMaterial, 0);
    ut::Color vtxColors[VERTEXCOLOR_MAX];
    detail::TexCoord texCds[1];
    math::VEC2 polPt;
    Size polSize;

    GetLTFrameSize(&polPt, &polSize, basePt, mSize, frameSize);
    GetLTTexCoord(texCds[0], polSize, texSize, TEXTUREFLIP_NONE);
    detail::DrawQuad(polPt, polSize, 1, texCds,
                     bUseVtxCol ? vtxColors : NULL, alpha);

    GetRTFrameSize(&polPt, &polSize, basePt, mSize, frameSize);
    GetRTTexCoord(texCds[0], polSize, texSize, TEXTUREFLIP_H);
    detail::DrawQuad(polPt, polSize, 1, texCds,
                     bUseVtxCol ? vtxColors : NULL, alpha);

    GetRBFrameSize(&polPt, &polSize, basePt, mSize, frameSize);
    GetRBTexCoord(texCds[0], polSize, texSize, TEXTUREFLIP_180);
    detail::DrawQuad(polPt, polSize, 1, texCds,
                     bUseVtxCol ? vtxColors : NULL, alpha);

    GetLBFrameSize(&polPt, &polSize, basePt, mSize, frameSize);
    GetLBTexCoord(texCds[0], polSize, texSize, TEXTUREFLIP_V);
    detail::DrawQuad(polPt, polSize, 1, texCds,
                     bUseVtxCol ? vtxColors : NULL, alpha);
}

void Window::DrawFrame4(const math::VEC2& rBase, const Frame* pFrames,
                        const WindowFrameSize& rFrameSize, u8 alpha) {

    ut::Color vtxColors[VERTEXCOLOR_MAX];
    detail::TexCoord texCoords[1];

    math::VEC2 point;
    Size size;

    bool modVtxColor = detail::IsModulateVertexColor(NULL, alpha);

#define DRAW_FRAME(FRAME)                                                      \
    if (pFrames[WINDOWFRAME_##FRAME].pMaterial->GetTextureNum() > 0) {         \
        bool useVtxColor = pFrames[WINDOWFRAME_##FRAME].pMaterial->SetupGX(    \
            modVtxColor, alpha);                                               \
                                                                               \
        Get##FRAME##FrameSize(&point, &size, rBase, mSize, rFrameSize);        \
                                                                               \
        Get##FRAME##TexCoord(                                                  \
            texCoords[0], size,                                                \
            detail::GetTextureSize(pFrames[WINDOWFRAME_##FRAME].pMaterial, 0), \
            pFrames[WINDOWFRAME_##FRAME].textureFlip);                         \
                                                                               \
        detail::SetVertexFormat(useVtxColor, 1);                               \
        detail::DrawQuad(point, size, 1, texCoords,                            \
                         useVtxColor ? vtxColors : NULL, alpha);               \
    }

    DRAW_FRAME(LT);
    DRAW_FRAME(RT);
    DRAW_FRAME(RB);
    DRAW_FRAME(LB);

#undef DRAW_FRAME
}

void Window::DrawFrame8(const math::VEC2& rBase, const Frame* pFrames,
                        const WindowFrameSize& rFrameSize, u8 alpha) {

    ut::Color vtxColors[VERTEXCOLOR_MAX];
    detail::TexCoord texCoords[1];

    Size size;

    bool modVtxColor = detail::IsModulateVertexColor(NULL, alpha);

#define DRAW_FRAME_EX(FRAME, TEXCOORD, SIZE, POINT)                            \
    if (pFrames[WINDOWFRAME_##FRAME].pMaterial->GetTextureNum() > 0) {         \
        bool useVtxColor = pFrames[WINDOWFRAME_##FRAME].pMaterial->SetupGX(    \
            modVtxColor, alpha);                                               \
                                                                               \
        size = SIZE;                                                           \
                                                                               \
        Get##TEXCOORD##TexCoord(                                               \
            texCoords[0], size,                                                \
            detail::GetTextureSize(pFrames[WINDOWFRAME_##FRAME].pMaterial, 0), \
            pFrames[WINDOWFRAME_##FRAME].textureFlip);                         \
                                                                               \
        detail::SetVertexFormat(useVtxColor, 1);                               \
        detail::DrawQuad(POINT, size, 1, texCoords,                            \
                         useVtxColor ? vtxColors : NULL, alpha);               \
    }

#define DRAW_FRAME(FRAME, SIZE, POINT) DRAW_FRAME_EX(FRAME, FRAME, SIZE, POINT)

    // clang-format off
    DRAW_FRAME(LT,
               Size(rFrameSize.l,
                    rFrameSize.t),
               rBase);

    DRAW_FRAME_EX(T, LT,
               Size(mSize.width - rFrameSize.l - rFrameSize.r,
                    rFrameSize.t),
               math::VEC2(rBase.x + rFrameSize.l,
                          rBase.y));

    DRAW_FRAME(RT,
               Size(rFrameSize.r,
                    rFrameSize.t),
               math::VEC2(rBase.x + mSize.width - rFrameSize.r,
                          rBase.y));

    DRAW_FRAME_EX(R, RT,
               Size(rFrameSize.r,
                    mSize.height - rFrameSize.t - rFrameSize.b),
               math::VEC2(rBase.x + mSize.width - rFrameSize.r,
                          rBase.y - rFrameSize.t));

    DRAW_FRAME(RB,
               Size(rFrameSize.r,
                    rFrameSize.b),
               math::VEC2(rBase.x + mSize.width - rFrameSize.r,
                          rBase.y - mSize.height + rFrameSize.b));

    DRAW_FRAME_EX(B, RB,
               Size(mSize.width - rFrameSize.l - rFrameSize.r,
                    rFrameSize.b),
               math::VEC2(rBase.x + rFrameSize.l,
                          rBase.y - mSize.height + rFrameSize.b));

    DRAW_FRAME(LB,
               Size(rFrameSize.l,
                    rFrameSize.b),
               math::VEC2(rBase.x,
                          rBase.y - mSize.height + rFrameSize.b));

    DRAW_FRAME_EX(L, LB,
               Size(rFrameSize.l,
                    mSize.height - rFrameSize.t - rFrameSize.b),
               math::VEC2(rBase.x,
                          rBase.y - rFrameSize.t));
    // clang-format on

#undef DRAW_FRAME
#undef DRAW_FRAME_EX
}

WindowFrameSize Window::GetFrameSize(u8 frameNum, const Frame* pFrames) {
    WindowFrameSize frameSize = {0.0f, 0.0f, 0.0f, 0.0f};

    switch (frameNum) {
    case 1: {
        Size texSize =
            detail::GetTextureSize(pFrames[WINDOWFRAME_LT].pMaterial, 0);

        frameSize.l = texSize.width;
        frameSize.t = texSize.height;

        frameSize.r = texSize.width;
        frameSize.b = texSize.height;
        break;
    }

    case 4:
    case 8: {
        Size texSize =
            detail::GetTextureSize(pFrames[WINDOWFRAME_LT].pMaterial, 0);

        frameSize.l = texSize.width;
        frameSize.t = texSize.height;

        texSize = detail::GetTextureSize(pFrames[WINDOWFRAME_RB].pMaterial, 0);

        frameSize.r = texSize.width;
        frameSize.b = texSize.height;
        break;
    }
    }

    return frameSize;
}

u8 Window::GetMaterialNum() const {
    return mFrameNum + 1;
}

Material* Window::GetMaterial(ulong idx) const {
    if (idx == 0) {
        return GetContentMaterial();
    }

    return GetFrameMaterial(idx - 1);
}

Material* Window::GetFrameMaterial(ulong frameIdx) const {
    if (frameIdx >= mFrameNum) {
        return NULL;
    }

    return mFrames[frameIdx].pMaterial;
}

Material* Window::GetContentMaterial() const {
    return GetMaterial();
}

} // namespace lyt
} // namespace nw4r
