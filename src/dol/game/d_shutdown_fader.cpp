// The shutdown fader (dShutdownFader_c). .text 8015FDF0..801603A0, .data 804F0658..804F06A0,
// .sdata 8074B2A0..8074B2A8, .sbss 8074E818..8074E820, .sdata2 80750E20..80750E48.
// See include/game/game/d_shutdown_fader.hpp.
#include <game/game/d_shutdown_fader.hpp>
#include <lib/egg/gfxe/eggScreen.h>
#include <lib/nw4r/g3d/g3d_state.h>
#include <revolution/gx.h>
#include <private/gx.h>
#include <revolution/mtx.h>
#include <revolution/sc.h>

// 8015FDF0
dShutdownFader_c::dShutdownFader_c(mColor col, mFaderBase_c::EStatus status) : mFaderBase_c(col, status) {
    if (SCGetAspectRatio() == SC_ASPECT_RATIO_16x9) {
        mWidthScale = 1.368421f;
    } else {
        mWidthScale = 1.0f;
    }
}

// 8015FE74
void dShutdownFader_c::setStatus(EStatus status) {
    if (status == OPAQUE) {
        mStatus = OPAQUE;
        mFaderColor.a = 255;
    } else if (status == HIDDEN) {
        mStatus = HIDDEN;
        mFaderColor.a = 0;
    }
}

// 8015FEAC
int dShutdownFader_c::calc() {
    int res = mFaderBase_c::calc();

    u16 currFrame = mCurrFrame;
    u16 endFrame = mFrameCount;
    if (currFrame > mFrameCount) {
        currFrame = mFrameCount;
    }

    switch (mStatus) {
    case HIDDEN:
        mFaderColor.a = 0;
        break;
    case OPAQUE:
    case FADE_OUT:
        mFaderColor.a = 255;
        break;
    case FADE_IN:
        mFaderColor.a = 255 - (currFrame * 255 / endFrame);
        break;
    }
    return res;
}

// 8015FF48
void dShutdownFader_c::draw() {
    f32 halfWidth = 304.0f * mWidthScale;
    if (mFaderColor.a == 0) {
        return;
    }

    EGG::Screen screen;
    f32 halfW = EGG::Screen::sTVModeInfo[EGG::Screen::sTVMode].width / 2.0f;
    f32 halfH = EGG::Screen::sTVModeInfo[EGG::Screen::sTVMode].height / 2.0f;
    screen.mProjType = EGG::Frustum::PROJ_ORTHO;
    screen.ResetOrthographic(halfH, -halfH, -halfW, halfW, 0.0f, 1.0f);
    screen.SetProjectionGX();

    Mtx posMtx;
    PSMTXIdentity(posMtx);
    GXLoadPosMtxImm(posMtx, 0);
    GXSetCurrentMtx(0);

    GXClearVtxDesc();
    GXInvalidateVtxCache();

    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    GXSetNumChans(1);
    GXSetChanMatColor(GX_COLOR0A0, mFaderColor);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    GXSetNumTexGens(0);
    GXSetNumIndStages(0);
    __GXSetIndirectMask(0);

    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    if (mFaderColor.a == 255) {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
    } else {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    }

    GXSetColorUpdate(1);
    GXSetAlphaUpdate(1);
    GXSetZMode(0, GX_NEVER, 0);
    GXSetCullMode(GX_CULL_BACK);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-halfWidth, 228.0f, 0.0f);
    GXPosition3f32(halfWidth, 228.0f, 0.0f);
    GXPosition3f32(halfWidth, -228.0f, 0.0f);
    GXPosition3f32(-halfWidth, -228.0f, 0.0f);
    GXEnd();
}

// 801601BC
dShutdownFader_c::~dShutdownFader_c() {}

namespace dShutdownFader {

static dShutdownFader_c *sFader; // 8074E818
static u16 sInFrame;             // 8074E81C

// 801601FC
BOOL startFadeOut(u16 outFrame, u16 inFrame) {
    if (!isHidden()) {
        return FALSE;
    }
    sFader->setFrame(outFrame);
    sInFrame = inFrame;
    return sFader->fadeOut();
}

// 80160268
BOOL isHidden() {
    return sFader->getStatus() == mFaderBase_c::HIDDEN;
}

// 801602A4
void calc() {
    sFader->calc();
    if (sFader->getStatus() == mFaderBase_c::OPAQUE) {
        sFader->setFrame(sInFrame);
        sFader->fadeIn();
    }
}

// 80160310
void draw() {
    nw4r::g3d::G3DState::Invalidate();
    sFader->draw();
}

// 80160348
BOOL create() {
    sFader = new dShutdownFader_c(mColor(0), mFaderBase_c::HIDDEN);
    if (sFader == NULL) {
        return FALSE;
    }
    return TRUE;
}

} // namespace dShutdownFader
