// The shutter fader (dShutterFader_c). .text 801603A0..80160AE4, .ctors 80465750, .data 804F06A0..804F0710,
// .bss 805F2F20..805FAF38, .sdata 8074B2A8..8074B2B8, .sbss 8074E820..8074E828, .sdata2 80750E48..80750E78.
// See include/game/game/d_shutter_fader.hpp.
#include <game/game/d_shutter_fader.hpp>
#include <lib/egg/gfxe/eggScreen.h>
#include <lib/nw4r/g3d/res/g3d_resfile.h>
#include <nw4r/math.h>
#include <revolution/gx.h>
#include <private/gx.h>
#include <revolution/mtx.h>
#include <string.h>

#define SHUTTER_TEX_SIZE 0x80
#define SHUTTER_BLADE_NUM 8

static u8 sTexture[SHUTTER_TEX_SIZE * SHUTTER_TEX_SIZE * 2] ALIGN(32); // 805F2F20: IA8

// 801603A0
dShutterFader_c::dShutterFader_c() : mWipeFader_c(mColor(0), mFaderBase_c::HIDDEN) {}

// 801603EC
void dShutterFader_c::loadTexture(void *brres) {
    nw4r::g3d::ResFile res(brres);
    res.Init();
    res.Bind(res);
    memcpy(sTexture, res.GetResTex(0).GetTexData(), sizeof(sTexture));
}

// 80160460
void dShutterFader_c::init() {
    setTexture(sTexture, SHUTTER_TEX_SIZE, SHUTTER_TEX_SIZE);
}

// 80160480
void dShutterFader_c::calcMtx() {
    PSMTXIdentity(mTexMtx);
}

static mColor sLightColor(0x46, 0x40, 0x3E, 0xFF); // 8074E820: the texture's bright parts
static mColor sDarkColor(0x0E, 0x14, 0x12, 0xFF);  // 8074E824: its dark parts

// 80160488
void dShutterFader_c::draw() {
    if (mProgress == 0) {
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
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumChans(0);

    GXTexObj texObj;
    GXInitTexObj(&texObj, mpTextureData, (u16)mTexWidth, (u16)mTexHeight, GX_TF_IA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXLoadTexObj(&texObj, GX_TEXMAP0);

    Mtx texMtx;
    PSMTXCopy(mTexMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    __GXSetIndirectMask(0);

    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, sDarkColor);
    GXSetTevColor(GX_TEVREG1, sLightColor);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZMode(GX_FALSE, GX_NEVER, GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);

    for (int i = 0; i < SHUTTER_BLADE_NUM; i++) {
        f32 s = nw4r::math::SinIdx(i * 0x2000 + 0x1000);
        f32 c = nw4r::math::CosIdx(i * 0x2000 + 0x1000);
        f32 x = -(255 - mProgress) * (1.0f + nw4r::math::FSqrt(2.0f)) - 4.0f;
        f32 y = (255 - mProgress) - 4.0f;
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(c * (512.0f + x) - s * (512.0f + y), -(s * (512.0f + x) + c * (512.0f + y)), 0.0f);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(c * x - s * (512.0f + y), -(s * x + c * (512.0f + y)), -1.0f);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition3f32(c * x - s * y, -(s * x + c * y), 0.0f);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(c * (512.0f + x) - s * y, -(s * (512.0f + x) + c * y), 1.0f);
        GXTexCoord2f32(0.0f, 1.0f);
        GXEnd();
    }
}
