// EGG::Screen. .text 804485FC..80449314
#include <lib/egg/gfxe/eggScreen.h>
#include <lib/egg/gfxe/eggStateGX.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>
#include <revolution/SC.h>

namespace EGG {

Screen::TVMode Screen::sTVMode = TV_MODE_4_3;

Screen::TVModeInfo Screen::sTVModeInfo[TV_MODE_MAX] = {
    {608, 456, nw4r::math::VEC2(640.0f / 608.0f, 1.0f)},
    {832, 456, nw4r::math::VEC2(640.0f / 832.0f, 1.0f)},
    {640, 456, nw4r::math::VEC2(1.0f, 1.0f)},
};

Screen *Screen::spRoot = NULL;
Screen::ChangeTVModeFunc Screen::sChangeTVModeFunc = NULL;
void *Screen::spChangeTVModeFuncInfo = NULL;

nw4r::math::VEC2 Screen::sCanvasScale(1.0f, 1.0f);
nw4r::math::VEC2 Screen::sCanvasOffset(0.0f, 0.0f);

void Screen::Initialize(const unsigned short *maxX, const unsigned short *maxY, Screen *pUserRoot) {
    sTVModeInfo[TV_MODE_4_3].width = maxX[0];
    sTVModeInfo[TV_MODE_4_3].height = maxY[0];
    sTVModeInfo[TV_MODE_16_9].width = maxX[1];
    sTVModeInfo[TV_MODE_16_9].height = maxY[1];
    sTVModeInfo[TV_MODE_EFB].width = StateGX::getEfbWidth();
    sTVModeInfo[TV_MODE_EFB].height = StateGX::getEfbHeight();

    for (int i = 0; i < TV_MODE_MAX; i++) {
        sTVModeInfo[i].ratios.x = static_cast<f32>(StateGX::getEfbWidth()) / static_cast<f32>(sTVModeInfo[i].width);
        sTVModeInfo[i].ratios.y = static_cast<f32>(StateGX::getEfbHeight()) / static_cast<f32>(sTVModeInfo[i].height);
    }

    static Screen root;
    root.mProjType = PROJ_ORTHO;
    root.SetCanvasMode(CANVAS_LU);
    root.mNearZ = 0.0f;
    root.mFarZ = 1.0f;

    pUserRoot = pUserRoot == NULL ? &root : pUserRoot;

    spRoot = pUserRoot;
    pUserRoot->mParent = NULL;

    SetTVMode(sTVMode);
}

Screen::Screen()
    : Frustum(PROJ_PERSP, nw4r::math::VEC2(sTVModeInfo[sTVMode].width, sTVModeInfo[sTVMode].height), 10.0f,
              100000.0f, CANVAS_LU),
      mParent(NULL) {
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;

    SetParent(NULL);
}

void Screen::SetProjectionGX() const {
    if (!(mFlags & FLAG_PROJ_ABSOLUTE)) {
        f32 sx, sy, ox, oy;
        GetGlobalScaleOffset(&sx, &sy, &ox, &oy);

        SetGlobalScaleOffset(sCanvasScale.x, sCanvasScale.y, sCanvasOffset.x, sCanvasOffset.y);

        Frustum::SetProjectionGX();

        SetGlobalScaleOffset(sx, sy, ox, oy);
    } else {
        Frustum::SetProjectionGX();
    }

    const DataEfb &efb = GetDataEfb();

    StateGX::GXSetViewport_(efb.vp.x, efb.vp.y, efb.vp.width, efb.vp.height, efb.vp.near, efb.vp.far);
    StateGX::GXSetScissor_(efb.vp.x, efb.vp.y, efb.vp.width, efb.vp.height);
    StateGX::GXSetScissorBoxOffset_(efb.ox, efb.oy);
}

void Screen::CopyToG3D(nw4r::g3d::Camera camera) const {
    if (!(mFlags & FLAG_PROJ_ABSOLUTE)) {
        f32 sy, sx, ox, oy;
        GetGlobalScaleOffset(&sx, &sy, &ox, &oy);

        SetGlobalScaleOffset(sCanvasScale.x, sCanvasScale.y, sCanvasOffset.x, sCanvasOffset.y);

        Frustum::CopyToG3D(camera);

        SetGlobalScaleOffset(sx, sy, ox, oy);
    } else {
        Frustum::CopyToG3D(camera);
    }

    const DataEfb &efb = GetDataEfb();

    f32 width, height;
    height = efb.vp.height;
    width = efb.vp.width;

    f32 x, y;
    y = efb.vp.y;
    x = efb.vp.x;

    f32 near, far;
    far = efb.vp.far;
    near = efb.vp.near;

    camera.SetViewport(x, y, width, height);
    camera.SetViewportZRange(near, far);

    camera.SetScissor(efb.vp.x, efb.vp.y, efb.vp.width, efb.vp.height);
    camera.SetScissorBoxOffset(efb.ox, efb.oy);
}

void Screen::GetPosSizeInEfb() const {
    f32 *pX = &mDataEfb.vp.x;
    f32 *pY = &mDataEfb.vp.y;

    TVMode tvMode = !(mFlags & FLAG_FORCE_STD_ASPECT) ? (!(mFlags & FLAG_USE_EFB_RATIO) ? sTVMode : TV_MODE_EFB) : TV_MODE_4_3;
    const TVModeInfo &info = sTVModeInfo[tvMode];

    if (mParent == NULL) {
        *pX = mPosition.x * sCanvasScale.x;
        *pY = mPosition.y * sCanvasScale.y;
    } else {
        mParent->GetPosInEfb(mPosition.x, mPosition.y, pX, pY);
    }

    mDataEfb.oy = 0;
    mDataEfb.ox = 0;

    f32 x = *pX;
    if (x < 0.0f) {
        s32 ix = static_cast<s32>(x);
        s32 lx = -ix;
        mDataEfb.ox = lx - (lx & 1);
        *pX = x - static_cast<f32>(ix);
    }

    f32 y = *pY;
    if (y < 0.0f) {
        s32 iy = static_cast<s32>(y);
        s32 ly = -iy;
        mDataEfb.oy = ly - (ly & 1);
        *pY = y - static_cast<f32>(iy);
    }

    if (mFlags & FLAG_ALIGN_EFB_POS) {
        *pX -= static_cast<s32>(*pX) & 1;
        *pY -= static_cast<s32>(*pY) & 1;
    }

    mDataEfb.vp.width = info.ratios.x * mSize.x;
    mDataEfb.vp.height = info.ratios.y * mSize.y;

    if (mFlags & FLAG_ALIGN_EFB_SIZE) {
        mDataEfb.vp.width -= static_cast<s32>(mDataEfb.vp.width) & 3;
        mDataEfb.vp.height -= static_cast<s32>(mDataEfb.vp.height) & 3;
    }

    mDataEfb.vp.width = static_cast<s32>(mDataEfb.vp.width);
    mDataEfb.vp.height = static_cast<s32>(mDataEfb.vp.height);
}

const Screen::DataEfb &Screen::GetDataEfb() const {
    if (IsChangeEfb()) {
        GetPosSizeInEfb();

        mDataEfb.vp.near = 0.0f;
        mDataEfb.vp.far = 1.0f;

        mFlags &= ~FLAG_DIRTY;
    }

    return mDataEfb;
}

bool Screen::IsChangeEfb() const {
    if (mFlags & FLAG_DIRTY) {
        return true;
    }

    if (mParent != NULL) {
        return mParent->IsChangeEfb();
    }

    return false;
}

void Screen::CalcMatrixForDrawQuad(nw4r::math::MTX34 *pMtx, f32 x, f32 y, f32 width, f32 height) const {
    PSMTXScale(*pMtx, width, height, 1.0f);

    pMtx->_03 = x;

    if (mCanvasMode == CANVAS_CC) {
        y = y - height;
    }

    pMtx->_13 = y;
    pMtx->_23 = 0.0f;
}

void Screen::GetGlobalPos(f32 *pX, f32 *pY) const {
    const Screen *parent = mParent;

    if (parent != NULL) {
        f32 gx, gy;
        parent->GetGlobalPos(&gx, &gy);

        parent->ConvertToCanvasLU(mPosition.x, mPosition.y, pX, pY);

        *pX += gx;
        *pY += gy;
    } else {
        *pX = mPosition.x;
        *pY = mPosition.y;
    }
}

void Screen::GetPosInEfb(f32 x, f32 y, f32 *pX, f32 *pY) const {
    GetGlobalPos(pX, pY);

    f32 lx, ly;
    ConvertToCanvasLU(x, y, &lx, &ly);

    TVMode tvMode = !(mFlags & FLAG_FORCE_STD_ASPECT) ? (!(mFlags & FLAG_USE_EFB_RATIO) ? sTVMode : TV_MODE_EFB) : TV_MODE_4_3;
    const TVModeInfo &info = sTVModeInfo[tvMode];

    *pX = info.ratios.x * (*pX + lx);
    *pY = info.ratios.y * (*pY + ly);
}

void Screen::SetTVMode(TVMode tvMode) {
    sTVMode = tvMode;

    if (spRoot != NULL) {
        spRoot->SetSizeX(GetSizeXMax());
        spRoot->SetSizeY(GetSizeYMax());
    }

    if (sChangeTVModeFunc != NULL) {
        sChangeTVModeFunc(spChangeTVModeFuncInfo);
    }
}

void Screen::SetTVModeDefault() {
    SetTVMode(SCGetAspectRatio() == SC_ASPECT_STD ? TV_MODE_4_3 : TV_MODE_16_9);
}

} // namespace EGG
