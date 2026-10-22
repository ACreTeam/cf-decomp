// EGG::StateGX. .text 80449314..80449D3C
#include <lib/egg/core/eggSystem.h>
#include <lib/egg/gfxe/eggStateGX.h>
#include <nw4r/g3d.h>
#include <revolution/gx.h>

namespace {

const GXColor WHITE = {255, 255, 255, 255};

} // namespace

namespace EGG {

static GXTexObj sNullTexObj;

StateGX::GXCache StateGX::s_cacheGX;
u16 StateGX::s_widthFb;
u16 StateGX::s_heightEfb;
GXPixelFmt StateGX::s_pixFormat;
GXZFmt16 StateGX::s_zFmt16;
GXPixelFmt StateGX::s_pixFormatCurrent;
GXZFmt16 StateGX::s_zFmt16Current;
GXColor StateGX::s_clearEfb;
u16 StateGX::s_cacheFlags;
int StateGX::s_field_CE4;

u16 StateGX::s_flag = FLAG_ENABLE_CACHE;
nw4r::g3d::tmem::TMemLayout StateGX::s_tmem_layout = nw4r::g3d::tmem::TMEM_LAYOUT_1;

static u8 sNullTexData[16] ALIGN(32);

void StateGX::initialize(unsigned short width, unsigned short height, GXColor clearColor, GXPixelFmt pixFmt) {
    s_widthFb = width;
    s_heightEfb = height;
    s_clearEfb = clearColor;
    s_pixFormat = pixFmt;

    if (BaseSystem::mConfigData->getDisplay() != NULL) {
        BaseSystem::mConfigData->getDisplay()->setClearColor(clearColor);
    }

    GXInitTexObj(&sNullTexObj, sNullTexData, 4, 4, GX_TF_IA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&sNullTexObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);

    s_flag &= ~FLAG_ENABLE_CACHE;

    GXInvalidateTexAll();
    frameInit();

    s_tmem_layout = nw4r::g3d::tmem::TMEM_LAYOUT_1;
    nw4r::g3d::tmem::SetTMemLayout(s_tmem_layout);
}

void StateGX::frameInit() {
    s_field_CE4 = 0;
    setPixelFormatGX(s_pixFormat, s_zFmt16);
    GXInvalidateVtxCache();
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetCoPlanar(GX_FALSE);
    GXSetClipMode(GX_CLIP_ENABLE);

    resetGX();
    s_cacheFlags = 0;

    if (BaseSystem::mConfigData->getDisplay() != NULL) {
        s_clearEfb = BaseSystem::mConfigData->getDisplay()->getClearColor();
    }

    nw4r::g3d::G3dReset();

    if (s_flag & FLAG_ENABLE_TMEM) {
        nw4r::g3d::tmem::SetTMemLayout(s_tmem_layout);
    }
}

void StateGX::resetGX() {
    if (!(s_flag & FLAG_0x01)) {
        resetVtx();
        resetIndirect();
        resetColorChannel();
        resetTexture();
        resetTexGen();
        resetTev();
        resetPE();
    }
    resetGXCache();
}

void StateGX::resetVtx() {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX2, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX3, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX4, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX5, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX6, GX_TEX_ST, GX_S16, 7);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX7, GX_TEX_ST, GX_S16, 7);
}

void StateGX::resetColorChannel() {
    GXSetNumChans(1);

    GXSetChanAmbColor(GX_COLOR0A0, WHITE);
    GXSetChanAmbColor(GX_COLOR1A1, WHITE);
    GXSetChanMatColor(GX_COLOR0A0, WHITE);
    GXSetChanMatColor(GX_COLOR1A1, WHITE);

    GXSetCullMode(GX_CULL_BACK);

    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}

static const f32 identityMtx[3][4] = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
};

static const f32 offsetMtx[2][3] = {
    {0.5f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f},
};

void StateGX::resetIndirect() {
    GXSetNumIndStages(0);

    for (u32 i = 0; i < 3; i++) {
        GXSetIndTexMtx(static_cast<GXIndTexMtxID>(i + GX_ITM_0), offsetMtx, 1);
    }

    for (u32 i = 0; i < GX_MAX_INDTEXSTAGE; i++) {
        GXSetIndTexCoordScale(static_cast<GXIndTexStageID>(i), GX_ITS_1, GX_ITS_1);
    }
}

void StateGX::resetTexture() {
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP0);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP1);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP2);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP3);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP4);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP5);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP6);
    GXLoadTexObj(&sNullTexObj, GX_TEXMAP7);
}

void StateGX::resetTexGen() {
    GXSetNumTexGens(1);

    for (u32 i = 0; i < 10; i++) {
        GXLoadTexMtxImm(identityMtx, GX_TEXMTX0 + i * 3, GX_MTX3x4);
    }

    for (u32 i = 0; i < GX_MAX_TEXCOORD; i++) {
        GXSetTexCoordGen2(static_cast<GXTexCoordID>(i), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        GXSetTexCoordScaleManually(static_cast<GXTexCoordID>(i), GX_FALSE, 0, 0);
        GXSetTexCoordBias(static_cast<GXTexCoordID>(i), GX_FALSE, GX_FALSE);
    }
}

void StateGX::resetTev() {
    GXSetNumTevStages(1);

    for (u32 i = 0; i < GX_MAX_TEVSTAGE; i++) {
        GXSetTevDirect(static_cast<GXTevStageID>(i));
        GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(static_cast<GXTevStageID>(i), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(static_cast<GXTevStageID>(i), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(static_cast<GXTevStageID>(i), GX_TEV_SWAP0, GX_TEV_SWAP0);
    }

    for (u32 i = 0; i < GX_MAX_KCOLOR; i++) {
        GXSetTevKColor(static_cast<GXTevKColorID>(i), WHITE);
    }

    for (u32 i = 0; i < GX_MAX_TEVREG; i++) {
        GXSetTevColor(static_cast<GXTevRegID>(i), WHITE);
    }

    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
}

void StateGX::resetPE() {
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    GXSetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.1f, 1.0f, WHITE);
    GXSetFogRangeAdj(GX_FALSE, 0, NULL);
    GXSetZCompLoc(GX_TRUE);
    GXSetDstAlpha(GX_FALSE, 0);
}

void StateGX::resetGXCache() {
    s_cacheGX.flags = 0;

    GXSetColorUpdate(GX_TRUE);
    s_cacheGX.colorUpdate = true;
    GXSetAlphaUpdate(GX_TRUE);
    s_cacheGX.alphaUpdate = true;
    GXSetDither(GX_TRUE);
    s_cacheGX.dither = true;
    GXSetDstAlpha(GX_FALSE, 0);
    s_cacheGX.dstAlphaEnable = false;
    s_cacheGX.dstAlpha = 0;
}

void StateGX::setPixelFormatGX(GXPixelFmt pixFormat, GXZFmt16 zFmt16) {
    if (pixFormat != s_pixFormatCurrent || zFmt16 != s_zFmt16Current) {
        GXSetPixelFmt(pixFormat, zFmt16);
        GXSetColorUpdate(s_cacheGX.colorUpdate);
        GXSetAlphaUpdate(s_cacheGX.alphaUpdate);
        s_pixFormatCurrent = pixFormat;
        s_zFmt16Current = zFmt16;
    }
}

void StateGX::GXSetProjectionv_(const f32 *proj) {
    GXSetProjectionv(proj);
    s_cacheFlags |= CACHE_PROJECTION;
}

void StateGX::GXSetViewport_(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ) {
    GXSetViewport(left, top, width, height, nearZ, farZ);
    s_cacheFlags |= CACHE_VIEWPORT;
}

void StateGX::GXSetScissor_(u32 left, u32 top, u32 width, u32 height) {
    GXSetScissor(left, top, width, height);
    s_cacheFlags |= CACHE_SCISSOR;
}

void StateGX::GXSetScissorBoxOffset_(int x, int y) {
    GXSetScissorBoxOffset(x, y);
    s_cacheFlags |= CACHE_SCISSOR_BOX;
}

} // namespace EGG
