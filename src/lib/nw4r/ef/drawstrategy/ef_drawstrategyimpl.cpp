#include <nw4r/ef.h>

#include <revolution/gx.h>

#include <cmath>

namespace nw4r {
namespace ef {

const math::VEC3 DrawStrategyImpl::mXUnitVec(1.0f, 0.0f, 0.0f);
const math::VEC3 DrawStrategyImpl::mYUnitVec(0.0f, 1.0f, 0.0f);
const math::VEC3 DrawStrategyImpl::mZUnitVec(0.0f, 0.0f, 1.0f);
const math::VEC3 DrawStrategyImpl::mZeroVec(0.0f, 0.0f, 0.0f);

// clang-format off
const math::MTX34 DrawStrategyImpl::mIdentityMtx(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f);
// clang-format on

/******************************************************************************
 *
 * DrawStrategyImpl
 *
 ******************************************************************************/
DrawStrategyImpl::DrawStrategyImpl() {}

void DrawStrategyImpl::_InitTexture(const EmitterDrawSetting &ed) {
    mNumTexmap = 0;

    mTexmapMap[TEX_LAYER_1] = -1;
    mTexmapMap[TEX_LAYER_2] = -1;
    mTexmapMap[TEX_LAYER_IND] = -1;

    if (ed.mFlags & EmitterDrawSetting::FLAG_TEX1_ENABLE) {
        mTexmapMap[TEX_LAYER_1] = 0;
        ++mNumTexmap;
    }

    if (ed.mFlags & EmitterDrawSetting::FLAG_TEX2_ENABLE) {
        mTexmapMap[TEX_LAYER_2] = mNumTexmap;
        ++mNumTexmap;
    }

    if (ed.mFlags & EmitterDrawSetting::FLAG_TEXIND_ENABLE) {
        mTexmapMap[TEX_LAYER_IND] = mNumTexmap;
        ++mNumTexmap;
    }

    GXSetNumTexGens(mNumTexmap);
}

void DrawStrategyImpl::Initialize(ParticleManager *pm,
                                  const EmitterDrawSetting &ed,
                                  const DrawInfo &drawInfo) {
    _InitTexture(ed);
    _InitTev(ed, drawInfo);
    _InitColor(pm, ed, drawInfo);
}

void DrawStrategyImpl::_InitTev(const EmitterDrawSetting &ed,
                                const DrawInfo &drawInfo) {
    GXSetClipMode(ed.mFlags & EmitterDrawSetting::FLAG_CLIP_DISABLE
                      ? GX_CLIP_DISABLE
                      : GX_CLIP_ENABLE);

    GXSetNumTevStages(ed.mNumTevs);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);

    int i;
    for (i = 0; i < ed.mNumTevs; i++) {
        GXSetTevDirect(static_cast<GXTevStageID>(i));

        GXSetTevColorIn(static_cast<GXTevStageID>(i),
                        static_cast<GXTevColorArg>(ed.mTevColor[i].mA),
                        static_cast<GXTevColorArg>(ed.mTevColor[i].mB),
                        static_cast<GXTevColorArg>(ed.mTevColor[i].mC),
                        static_cast<GXTevColorArg>(ed.mTevColor[i].mD));

        GXSetTevAlphaIn(static_cast<GXTevStageID>(i),
                        static_cast<GXTevAlphaArg>(ed.mTevAlpha[i].mA),
                        static_cast<GXTevAlphaArg>(ed.mTevAlpha[i].mB),
                        static_cast<GXTevAlphaArg>(ed.mTevAlpha[i].mC),
                        static_cast<GXTevAlphaArg>(ed.mTevAlpha[i].mD));

        GXSetTevColorOp(static_cast<GXTevStageID>(i),
                        static_cast<GXTevOp>(ed.mTevColorOp[i].mOp),
                        static_cast<GXTevBias>(ed.mTevColorOp[i].mBias),
                        static_cast<GXTevScale>(ed.mTevColorOp[i].mScale),
                        static_cast<GXBool>(ed.mTevColorOp[i].mClamp),
                        static_cast<GXTevRegID>(ed.mTevColorOp[i].mOutReg));

        GXSetTevAlphaOp(static_cast<GXTevStageID>(i),
                        static_cast<GXTevOp>(ed.mTevAlphaOp[i].mOp),
                        static_cast<GXTevBias>(ed.mTevAlphaOp[i].mBias),
                        static_cast<GXTevScale>(ed.mTevAlphaOp[i].mScale),
                        static_cast<GXBool>(ed.mTevAlphaOp[i].mClamp),
                        static_cast<GXTevRegID>(ed.mTevAlphaOp[i].mOutReg));

        GXSetTevKColorSel(static_cast<GXTevStageID>(i),
                          static_cast<GXTevKColorSel>(ed.mTevKColorSel[i]));

        GXSetTevKAlphaSel(static_cast<GXTevStageID>(i),
                          static_cast<GXTevKAlphaSel>(ed.mTevKAlphaSel[i]));

        GXSetTevSwapMode(static_cast<GXTevStageID>(i), GX_TEV_SWAP0,
                         GX_TEV_SWAP0);

        if (ed.mTevTexture[i] == 0) {
            GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD0,
                          GX_TEXMAP0, GX_COLOR0A0);
        } else if (ed.mTevTexture[i] == 1) {
            GXSetTevOrder(static_cast<GXTevStageID>(i),
                          static_cast<GXTexCoordID>(mTexmapMap[TEX_LAYER_2]),
                          static_cast<GXTexMapID>(mTexmapMap[TEX_LAYER_2]),
                          GX_COLOR0A0);
        } else {
            GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD_NULL,
                          GX_TEXMAP_NULL, GX_COLOR0A0);
        }
    }

    if (ed.mFlags & EmitterDrawSetting::FLAG_TEXIND_ENABLE) {
        GXSetNumIndStages(1);

        GXSetIndTexOrder(GX_INDTEXSTAGE0,
                         static_cast<GXTexCoordID>(mTexmapMap[TEX_LAYER_IND]),
                         static_cast<GXTexMapID>(mTexmapMap[TEX_LAYER_IND]));

        GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
        GXSetIndTexMtx(GX_ITM_0, ed.mIndTexOffsetMtx, ed.mIndTexScaleExp);

        u8 mask = 1;
        for (i = 0; i < ed.mNumTevs; i++, mask <<= 1) {
            if ((ed.mIndirectTargetStage & mask) != 0) {
                GXSetTevIndirect(static_cast<GXTevStageID>(i), GX_INDTEXSTAGE0,
                                 GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                                 GX_ITW_OFF, FALSE, FALSE, GX_ITBA_OFF);
            }
        }
    } else {
        GXSetNumIndStages(0);
    }

    GXSetZCompLoc(ed.mFlags & EmitterDrawSetting::FLAG_ZCOMP_BEFORE_TEX
                      ? GX_TRUE
                      : GX_FALSE);

    GXSetCullMode(GX_CULL_NONE);
    GXSetCoPlanar(FALSE);

    GXSetBlendMode(static_cast<GXBlendMode>(ed.mBlendMode.mType),
                   static_cast<GXBlendFactor>(ed.mBlendMode.mSrcFactor),
                   static_cast<GXBlendFactor>(ed.mBlendMode.mDstFactor),
                   static_cast<GXLogicOp>(ed.mBlendMode.mOp));

    GXSetZMode(
        ed.mFlags & EmitterDrawSetting::FLAG_ZCOMP_ENABLE_TEST ? GX_TRUE
                                                               : GX_FALSE,
        static_cast<GXCompare>(ed.mZCompareFunc),
        ed.mFlags & EmitterDrawSetting::FLAG_ZCOMP_ENABLE_UPDATE ? GX_TRUE
                                                                 : GX_FALSE);

    if (ed.mFlags & EmitterDrawSetting::FLAG_FOG_ENABLE) {
        GXFogType type;
        f32 startz;
        f32 endz;
        f32 nearz;
        f32 farz;
        GXColor color;

        drawInfo.GetFog(&type, &startz, &endz, &nearz, &farz, &color);
        GXSetFog(type, startz, endz, nearz, farz, color);
    } else {
        GXColor color = {0, 0, 0, 0};
        GXSetFog(GX_FOG_NONE, 0.0f, 100.0f, 0.0f, 100.0f, color);
    }
}

#define CHECK_USE_COLOR(input, use1_1, use1_2, use2_1, use2_2)                 \
    switch (input) {                                                           \
    case ColorInput::TEVCOLOR1_1: {                                            \
        use1_1 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_2: {                                            \
        use1_2 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_MULT: {                                         \
        use1_1 = true;                                                         \
        use1_2 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_1: {                                            \
        use2_1 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_2: {                                            \
        use2_2 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_MULT: {                                         \
        use2_1 = true;                                                         \
        use2_2 = true;                                                         \
        break;                                                                 \
    }                                                                          \
    }

void DrawStrategyImpl::_InitColor(ParticleManager *pm,
                                  const EmitterDrawSetting &ed,
                                  const DrawInfo &drawInfo) {
#pragma unused(pm)

    mPrevARef0 = -1;
    mPrevARef1 = -1;

    GXSetNumChans(1);

    if (!drawInfo.IsLightEnable()) {
        GXSetChanCtrl(GX_COLOR0A0, FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_NONE, GX_AF_NONE);
    } else {
        if (ed.mColorInput.mRasColor == ColorInput::RASCOLOR_LIGHTING) {
            GXSetChanCtrl(GX_COLOR0, TRUE, GX_SRC_REG, GX_SRC_REG,
                          drawInfo.GetLightMask(), GX_DF_NONE,
                          drawInfo.IsSpotLight() ? GX_AF_SPOT : GX_AF_NONE);
        } else {
            GXSetChanCtrl(GX_COLOR0, FALSE, GX_SRC_REG, GX_SRC_REG,
                          GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }

        if (ed.mAlphaInput.mRasColor == ColorInput::RASCOLOR_LIGHTING) {
            GXSetChanCtrl(GX_ALPHA0, TRUE, GX_SRC_REG, GX_SRC_REG,
                          drawInfo.GetLightAlphaMask(), GX_DF_NONE,
                          drawInfo.IsSpotLight() ? GX_AF_SPOT : GX_AF_NONE);
        } else {
            GXSetChanCtrl(GX_ALPHA0, FALSE, GX_SRC_REG, GX_SRC_REG,
                          GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
    }

    GXSetChanCtrl(GX_COLOR1A1, FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);

    GXSetChanMatColor(GX_COLOR0A0, drawInfo.GetChanMatColor());
    GXSetChanAmbColor(GX_COLOR0A0, drawInfo.GetChanAmbColor());

    mUseColor1_1 = false;
    mUseColor1_2 = false;
    mUseColor2_1 = false;
    mUseColor2_2 = false;
    mUseAlpha1_1 = false;
    mUseAlpha1_2 = false;
    mUseAlpha2_1 = false;
    mUseAlpha2_2 = false;

    int i;
    for (i = 0; i < 3; i++) {
        CHECK_USE_COLOR(ed.mColorInput.mTevColor[i], mUseColor1_1, mUseColor1_2,
                        mUseColor2_1, mUseColor2_2);
        CHECK_USE_COLOR(ed.mAlphaInput.mTevColor[i], mUseAlpha1_1, mUseAlpha1_2,
                        mUseAlpha2_1, mUseAlpha2_2);
    }

    for (i = 0; i < 4; i++) {
        CHECK_USE_COLOR(ed.mColorInput.mTevKColor[i], mUseColor1_1,
                        mUseColor1_2, mUseColor2_1, mUseColor2_2);
        CHECK_USE_COLOR(ed.mAlphaInput.mTevKColor[i], mUseAlpha1_1,
                        mUseAlpha1_2, mUseAlpha2_1, mUseAlpha2_2);
    }
}

#undef CHECK_USE_COLOR

bool DrawStrategyImpl::_SetupACmp(Particle *pp, const EmitterDrawSetting &ed,
                                  bool first) {
    if (first || mPrevARef0 != pp->mParameter.mACmpRef0 ||
        mPrevARef0 != pp->mParameter.mACmpRef0) {

        mPrevARef0 = pp->mParameter.mACmpRef0;
        mPrevARef1 = pp->mParameter.mACmpRef0;

        GXSetAlphaCompare(
            static_cast<GXCompare>(ed.mACmpComp0), pp->mParameter.mACmpRef0,
            static_cast<GXAlphaOp>(ed.mACmpOp),
            static_cast<GXCompare>(ed.mACmpComp1), pp->mParameter.mACmpRef1);

        return true;
    }

    return false;
}

void DrawStrategyImpl::SetupGP(Particle *pp, const EmitterDrawSetting &ed,
                               const DrawInfo &drawInfo, bool first,
                               bool xfdirty) {
#pragma unused(xfdirty)

    bool changed;
    bool XFbug;

    changed = _SetupACmp(pp, ed, first);
    XFbug = changed;

    changed = _SetupTevReg(pp, ed, first);
    XFbug = XFbug || changed;

    changed = _SetupTexture(pp, ed, drawInfo, first);
    XFbug = XFbug && !changed;

    if ((ed.mFlags & EmitterDrawSetting::FLAG_CLIP_DISABLE) ||
        !pp->mParticleManager->mManagerEM->mManagerEF->mManagerES
             ->mXFFlushSafe) {
        return;
    }

    if (XFbug) {
        struct VtxDescData {
            GXAttrType type;     // at 0x0
            GXCompCnt compCnt;   // at 0x4
            GXCompType compType; // at 0x8
        } mVtxDescPos, mVtxDescNrm, mVtxDescTex0;

        u8 frac;

        GXGetVtxDesc(GX_VA_POS, &mVtxDescPos.type);
        GXGetVtxDesc(GX_VA_NRM, &mVtxDescNrm.type);
        GXGetVtxDesc(GX_VA_TEX0, &mVtxDescTex0.type);

        GXGetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, &mVtxDescPos.compCnt,
                        &mVtxDescPos.compType, &frac);
        GXGetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, &mVtxDescNrm.compCnt,
                        &mVtxDescNrm.compType, &frac);
        GXGetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, &mVtxDescTex0.compCnt,
                        &mVtxDescTex0.compType, &frac);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
        {
            for (int i = 0; i < 8; i++) {
                if (mVtxDescPos.type != GX_NONE) {
                    switch (mVtxDescPos.type) {
                    case GX_DIRECT: {

                        switch (mVtxDescPos.compCnt) {
                        case GX_POS_XYZ: {

                            switch (mVtxDescPos.compType) {
                            case GX_U8: {
                                GXPosition3u8(0, 0, 0);
                                break;
                            }

                            case GX_S8: {
                                GXPosition3s8(0, 0, 0);
                                break;
                            }

                            case GX_U16: {
                                GXPosition3u16(0, 0, 0);
                                break;
                            }

                            case GX_S16: {
                                GXPosition3s16(0, 0, 0);
                                break;
                            }

                            case GX_F32: {
                                GXPosition3f32(0.0f, 0.0f, 0.0f);
                                break;
                            }

                            default: {
                                break;
                            }
                            }

                            break;
                        }

                        case GX_POS_XY: {

                            switch (mVtxDescPos.compType) {
                            case GX_U8: {
                                GXPosition2u8(0, 0);
                                break;
                            }

                            case GX_S8: {
                                GXPosition2s8(0, 0);
                                break;
                            }

                            case GX_U16: {
                                GXPosition2u16(0, 0);
                                break;
                            }

                            case GX_S16: {
                                GXPosition2s16(0, 0);
                                break;
                            }

                            case GX_F32: {
                                GXPosition2f32(0.0f, 0.0f);
                                break;
                            }

                            default: {
                                break;
                            }
                            }

                            break;
                        }

                        default: {
                            break;
                        }
                        }

                        break;
                    }

                    case GX_INDEX8: {
                        GXPosition1x8(0);
                        break;
                    }

                    case GX_INDEX16: {
                        GXPosition1x16(0);
                        break;
                    }

                    default: {
                        break;
                    }
                    }
                }

                if (mVtxDescNrm.type != GX_NONE) {
                    switch (mVtxDescNrm.type) {
                    case GX_DIRECT: {

                        switch (mVtxDescNrm.compCnt) {
                        case GX_NRM_XYZ: {

                            switch (mVtxDescNrm.compType) {
                            case GX_S8: {
                                GXNormal3u8(0, 0, 0);
                                break;
                            }

                            case GX_S16: {
                                GXNormal3u16(0, 0, 0);
                                break;
                            }

                            case GX_F32: {
                                GXNormal3f32(0.0f, 0.0f, 0.0f);
                                break;
                            }

                            // @bug Missing GX_U8/GX_U16
                            default: {
                                break;
                            }
                            }

                            break;
                        }

                        default: {
                            break;
                        }
                        }

                        break;
                    }

                    case GX_INDEX8: {
                        GXNormal1x8(0);
                        break;
                    }

                    case GX_INDEX16: {
                        GXNormal1x16(0);
                        break;
                    }

                    default: {
                        break;
                    }
                    }
                }

                if (mVtxDescTex0.type != GX_NONE) {
                    switch (mVtxDescTex0.type) {
                    case GX_DIRECT: {

                        switch (mVtxDescTex0.compCnt) {
                        case GX_TEX_ST: {

                            switch (mVtxDescTex0.compType) {
                            case GX_U8: {
                                GXTexCoord2u8(0, 0);
                                break;
                            }

                            case GX_S8: {
                                GXTexCoord2s8(0, 0);
                                break;
                            }

                            case GX_U16: {
                                GXTexCoord2u16(0, 0);
                                break;
                            }

                            case GX_S16: {
                                GXTexCoord2s16(0, 0);
                                break;
                            }

                            case GX_F32: {
                                GXTexCoord2f32(0.0f, 0.0f);
                                break;
                            }

                            default: {
                                break;
                            }
                            }

                            break;
                        }

                        case GX_TEX_S: {

                            switch (mVtxDescTex0.compType) {
                            case GX_U8: {
                                GXTexCoord1u8(0);
                                break;
                            }

                            case GX_S8: {
                                GXTexCoord1s8(0);
                                break;
                            }

                            case GX_U16: {
                                GXTexCoord1u16(0);
                                break;
                            }

                            case GX_S16: {
                                GXTexCoord1s16(0);
                                break;
                            }

                            case GX_F32: {
                                GXTexCoord1f32(0.0f);
                                break;
                            }

                            default: {
                                break;
                            }
                            }

                            break;
                        }

                        default: {
                            break;
                        }
                        }

                        break;
                    }

                    case GX_INDEX8: {
                        GXTexCoord1x8(0);
                        break;
                    }

                    case GX_INDEX16: {
                        GXTexCoord1x16(0);
                        break;
                    }

                    default: {
                        break;
                    }
                    }
                }
            }
        }
        GXEnd();
    }
}

#define COLOR_MODULATE(dst, a, b, c)                                           \
    dst = static_cast<u8>((a.c * b.c + 128) >> 8)

#define SELECT_COLOR(input, color, drawColor)                                  \
    switch (input) {                                                           \
    case ColorInput::TEVCOLOR1_1: {                                            \
        color.r = drawColor[0][0].r;                                           \
        color.g = drawColor[0][0].g;                                           \
        color.b = drawColor[0][0].b;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_2: {                                            \
        color.r = drawColor[0][1].r;                                           \
        color.g = drawColor[0][1].g;                                           \
        color.b = drawColor[0][1].b;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_MULT: {                                         \
        COLOR_MODULATE(color.r, drawColor[0][0], drawColor[0][1], r);          \
        COLOR_MODULATE(color.g, drawColor[0][0], drawColor[0][1], g);          \
        COLOR_MODULATE(color.b, drawColor[0][0], drawColor[0][1], b);          \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_1: {                                            \
        color.r = drawColor[1][0].r;                                           \
        color.g = drawColor[1][0].g;                                           \
        color.b = drawColor[1][0].b;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_2: {                                            \
        color.r = drawColor[1][1].r;                                           \
        color.g = drawColor[1][1].g;                                           \
        color.b = drawColor[1][1].b;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_MULT: {                                         \
        COLOR_MODULATE(color.r, drawColor[1][0], drawColor[1][1], r);          \
        COLOR_MODULATE(color.g, drawColor[1][0], drawColor[1][1], g);          \
        COLOR_MODULATE(color.b, drawColor[1][0], drawColor[1][1], b);          \
        break;                                                                 \
    }                                                                          \
    }

#define SELECT_ALPHA(input, color, drawColor)                                  \
    switch (input) {                                                           \
    case ColorInput::TEVCOLOR1_1: {                                            \
        color.a = drawColor[0][0].a;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_2: {                                            \
        color.a = drawColor[0][1].a;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR1_MULT: {                                         \
        COLOR_MODULATE(color.a, drawColor[0][0], drawColor[0][1], a);          \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_1: {                                            \
        color.a = drawColor[1][0].a;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_2: {                                            \
        color.a = drawColor[1][1].a;                                           \
        break;                                                                 \
    }                                                                          \
    case ColorInput::TEVCOLOR2_MULT: {                                         \
        COLOR_MODULATE(color.a, drawColor[1][0], drawColor[1][1], a);          \
        break;                                                                 \
    }                                                                          \
    }

#define APPLY_ALPHA_FLICK(alpha, alphaFlick)                                   \
    if (alphaFlick != 255) {                                                   \
        alpha = static_cast<u8>((alpha * alphaFlick + 128) >> 8);              \
    }

bool DrawStrategyImpl::_SetupTevReg(Particle *pp, const EmitterDrawSetting &ed,
                                    bool first) {
    bool changed = false;
    GXColor drawColor[2][2];

    if (mUseColor1_1) {
        pp->GetColorRGB(0, 0, &drawColor[0][0]);
    }

    if (mUseColor1_2) {
        pp->GetColorRGB(0, 1, &drawColor[0][1]);
    }

    if (mUseColor2_1) {
        pp->GetColorRGB(1, 0, &drawColor[1][0]);
    }

    if (mUseColor2_2) {
        pp->GetColorRGB(1, 1, &drawColor[1][1]);
    }

    u8 alphaFlick = 255;

    if (mUseAlpha1_1 || mUseAlpha1_2 || mUseAlpha2_1 || mUseAlpha2_2) {
        if (ed.mAlphaFlickType != EmitterDrawSetting::ALPHAFLICK_NONE) {
            alphaFlick = pp->GetAlphaFlick();
        }
    }

    if (mUseAlpha1_1) {
        drawColor[0][0].a = pp->GetAlpha(0, 0);
        APPLY_ALPHA_FLICK(drawColor[0][0].a, alphaFlick);
    }

    if (mUseAlpha1_2) {
        drawColor[0][1].a = pp->GetAlpha(0, 1);
        APPLY_ALPHA_FLICK(drawColor[0][1].a, alphaFlick);
    }

    if (mUseAlpha2_1) {
        drawColor[1][0].a = pp->GetAlpha(1, 0);
        APPLY_ALPHA_FLICK(drawColor[1][0].a, alphaFlick);
    }

    if (mUseAlpha2_2) {
        drawColor[1][1].a = pp->GetAlpha(1, 1);
        APPLY_ALPHA_FLICK(drawColor[1][1].a, alphaFlick);
    }

    if (pp->mParticleManager->mModifier.mLight.mType !=
        Lighting::LIGHTING_TYPE_NONE) {

        if (mUseColor1_1 || mUseColor1_2 || mUseAlpha1_1 || mUseAlpha1_2) {
            pp->mParticleManager->Draw_ModifyColor(pp, &drawColor[0][0],
                                                   &drawColor[0][1]);
        }

        if (mUseColor2_1 || mUseColor2_2 || mUseAlpha2_1 || mUseAlpha2_2) {
            pp->mParticleManager->Draw_ModifyColor(pp, &drawColor[1][0],
                                                   &drawColor[1][1]);
        }
    }

    int i;
    for (i = 0; i < 3; i++) {
        GXColor color = {0, 0, 0, 0};

        if (ed.mColorInput.mTevColor[i] == ColorInput::TEVCOLOR_NULL &&
            ed.mAlphaInput.mTevColor[i] == ColorInput::TEVCOLOR_NULL) {
            continue;
        }

        SELECT_COLOR(ed.mColorInput.mTevColor[i], color, drawColor);
        SELECT_ALPHA(ed.mAlphaInput.mTevColor[i], color, drawColor);

        if (first || mPrevColor[i].r != color.r || mPrevColor[i].g != color.g ||
            mPrevColor[i].b != color.b || mPrevColor[i].a != color.a) {

            GXSetTevColor(static_cast<GXTevRegID>(GX_TEVREG0 + i), color);
            mPrevColor[i] = color;
            changed = true;
        }
    }

    for (i = 0; i < 4; i++) {
        GXColor color = {0, 0, 0, 0};

        if (ed.mColorInput.mTevKColor[i] == ColorInput::TEVCOLOR_NULL &&
            ed.mAlphaInput.mTevKColor[i] == ColorInput::TEVCOLOR_NULL) {
            continue;
        }

        SELECT_COLOR(ed.mColorInput.mTevKColor[i], color, drawColor);
        SELECT_ALPHA(ed.mAlphaInput.mTevKColor[i], color, drawColor);

        if (first || mPrevKColor[i].r != color.r ||
            mPrevKColor[i].g != color.g || mPrevKColor[i].b != color.b ||
            mPrevKColor[i].a != color.a) {

            GXSetTevKColor(static_cast<GXTevKColorID>(GX_KCOLOR0 + i), color);
            mPrevKColor[i] = color;
            changed = true;
        }
    }

    return changed;
}

#undef APPLY_ALPHA_FLICK
#undef SELECT_ALPHA
#undef SELECT_COLOR
#undef COLOR_MODULATE

bool DrawStrategyImpl::_SetupTexture(Particle *pp, const EmitterDrawSetting &ed,
                                     const DrawInfo &drawInfo, bool first) {
    bool changed = false;

    for (int texIndex = 0; texIndex < TEX_LAYER_MAX; texIndex++) {
        if (mTexmapMap[texIndex] < 0) {
            continue;
        }

        if (pp->mParameter.mTexture[texIndex] == NULL) {
            continue;
        }

        const TextureData *texData = pp->mParameter.mTexture[texIndex];

        f32 scaleS = pp->Draw_GetTextureScaleS(texIndex);
        f32 scaleT = pp->Draw_GetTextureScaleT(texIndex);

        f32 transS = pp->Draw_GetTextureTransS(texIndex);
        f32 transT = pp->Draw_GetTextureTransT(texIndex);

        GXTexWrapMode wrapS = pp->GetTextureWrapS(texIndex);
        GXTexWrapMode wrapT = pp->GetTextureWrapT(texIndex);

        if (first || !mPrevTexture[texIndex].IsEqual(
                         texData, scaleS, scaleT, transS, transT, wrapS, wrapT,
                         pp->mParameter.mTextureScale[texIndex],
                         pp->mParameter.mTextureRotate[texIndex],
                         pp->mParameter.mTextureTranslate[texIndex])) {

            changed = true;

            mPrevTexture[texIndex].Set(
                texData, scaleS, scaleT, transS, transT, wrapS, wrapT,
                pp->mParameter.mTextureScale[texIndex],
                pp->mParameter.mTextureRotate[texIndex],
                pp->mParameter.mTextureTranslate[texIndex]);

            int format = texData->format;

            GXTexObj texObj;
            GXTlutObj tlutObj;

            switch (format) {
            case GX_TF_C4:
            case GX_TF_C8:
            case GX_TF_C14X2: {
                GXInitTlutObj(&tlutObj, texData->tlut,
                              static_cast<GXTlutFmt>(texData->tlutFormat),
                              texData->tlutEntries);

                GXLoadTlut(&tlutObj, static_cast<GXTlut>(mTexmapMap[texIndex]));

                GXInitTexObjCI(&texObj, texData->texture, texData->width,
                               texData->height, static_cast<GXCITexFmt>(format),
                               wrapS, wrapT, texData->mipmap > 1,
                               mTexmapMap[texIndex]);
                break;
            }

            case GX_TF_I4:
            case GX_TF_I8:
            case GX_TF_IA4:
            case GX_TF_IA8:
            case GX_TF_RGB565:
            case GX_TF_RGB5A3:
            case GX_TF_RGBA8:
            case GX_TF_CMPR: {
                GXInitTexObj(&texObj, texData->texture, texData->width,
                             texData->height, static_cast<GXTexFmt>(format),
                             wrapS, wrapT, texData->mipmap > 1);
                break;
            }

            default: {
                break;
            }
            }

            f32 maxLod = texData->mipmap + -1.0f;
            f32 lodBias = 0.0f;

            GXInitTexObjLOD(
                &texObj, texData->mipmap > 1 ? GX_LIN_MIP_LIN : GX_LINEAR,
                GX_LINEAR, 0.0f, maxLod, lodBias, FALSE, FALSE, GX_ANISO_1);

            GXLoadTexObj(&texObj,
                         static_cast<GXTexMapID>(mTexmapMap[texIndex]));

            math::MTX34 texmtx;
            math::MTX34 work;
            math::MTX34Identity(&texmtx);

            texmtx._03 = pp->mParameter.mTextureTranslate[texIndex].x;
            texmtx._13 = pp->mParameter.mTextureTranslate[texIndex].y;

            if (pp->mParameter.mTextureRotate[texIndex] != 0.0f) {
                f32 sr;
                f32 cr;
                PSSinCosRad(&sr, &cr, pp->mParameter.mTextureRotate[texIndex]);

                math::MTX34Identity(&work);

                work._00 = work._11 = cr;
                work._10 = sr;
                work._01 = -sr;
                work._03 = 0.5f * (1.0f - cr + sr);
                work._13 = 0.5f * (1.0f - cr - sr);

                math::MTX34Mult(&texmtx, &work, &texmtx);
            }

            if (scaleS != 1.0f || scaleT != 1.0f ||
                pp->mParameter.mTextureScale[texIndex].x != 1.0f ||
                pp->mParameter.mTextureScale[texIndex].y != 1.0f ||
                transS != 0.0f || transT != 0.0f) {

                f32 sssx = scaleS * pp->mParameter.mTextureScale[texIndex].x;
                work._00 = sssx;
                work._01 = 0.0f;
                work._02 = 0.0f;
                work._03 = transS + (0.5f * scaleS - 0.5f * sssx);

                work._10 = 0.0f;
                f32 stsy = scaleT * pp->mParameter.mTextureScale[texIndex].y;
                work._11 = stsy;
                work._12 = 0.0f;
                work._13 = transT + (0.5f * scaleT - 0.5f * stsy);

                work._20 = 0.0f;
                work._21 = 0.0f;
                work._22 = 1.0f;
                work._23 = 0.0f;

                math::MTX34Mult(&texmtx, &work, &texmtx);
            }

            if (!((ed.mFlags >> (texIndex + 7)) & 1)) {
                GXLoadTexMtxImm(texmtx, GX_TEXMTX0 + mTexmapMap[texIndex] * 3,
                                GX_MTX2x4);

                GXSetTexCoordGen(
                    static_cast<GXTexCoordID>(mTexmapMap[texIndex]),
                    GX_TG_MTX2x4, GX_TG_TEX0,
                    GX_TEXMTX0 + mTexmapMap[texIndex] * 3);
            } else {
                math::MTX34Mult(&texmtx, drawInfo.GetProjMtx(), &texmtx);

                GXLoadTexMtxImm(texmtx, GX_PTTEXMTX0 + mTexmapMap[texIndex] * 3,
                                GX_MTX3x4);

                GXSetTexCoordGen2(
                    static_cast<GXTexCoordID>(mTexmapMap[texIndex]),
                    GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE,
                    GX_PTTEXMTX0 + mTexmapMap[texIndex] * 3);
            }
        }
    }

    return changed;
}

DrawStrategyImpl::GetFirstDrawParticleFunc
DrawStrategyImpl::GetGetFirstDrawParticleFunc(int drawOrder) {

    switch (drawOrder) {
    case DRAWORDER_YOUNGERS_FIRST: {
        return GetFirstDrawParticle_YoungersFirst;
    }

    default: /* DRAWORDER_ELDERS_FIRST */ {
        return GetFirstDrawParticle_EldersFirst;
    }
    }
}

DrawStrategyImpl::GetNextDrawParticleFunc
DrawStrategyImpl::GetGetNextDrawParticleFunc(int drawOrder) {

    switch (drawOrder) {
    case DRAWORDER_YOUNGERS_FIRST: {
        return GetNextDrawParticle_YoungersFirst;
    }

    default: /* DRAWORDER_ELDERS_FIRST */ {
        return GetNextDrawParticle_EldersFirst;
    }
    }
}

Particle*
DrawStrategyImpl::GetFirstDrawParticle_EldersFirst(ParticleManager* pManager) {

    return GetOldestDrawParticle(pManager);
}

Particle* DrawStrategyImpl::GetFirstDrawParticle_YoungersFirst(
    ParticleManager* pManager) {

    return GetYoungestDrawParticle(pManager);
}

Particle*
DrawStrategyImpl::GetNextDrawParticle_EldersFirst(ParticleManager* pManager,
                                                  Particle* pParticle) {

    return GetYoungerDrawParticle(pManager, pParticle);
}

Particle*
DrawStrategyImpl::GetNextDrawParticle_YoungersFirst(ParticleManager* pManager,
                                                    Particle* pParticle) {

    return GetElderDrawParticle(pManager, pParticle);
}

/******************************************************************************
 *
 * AheadContext
 *
 ******************************************************************************/
DrawStrategyImpl::AheadContext::AheadContext(const math::MTX34& rViewMtx,
                                             ParticleManager* pManager) {

    mCommon.mViewMtx = &rViewMtx;
    mCommon.mParticleManager = pManager;

    pManager->mManagerEM->CalcGlobalMtx(&mCommon.mEmitterMtx);
    pManager->CalcGlobalMtx(&mCommon.mParticleManagerMtx);

    math::MTX34Inv(&mCommon.mParticleManagerMtxInv,
                   &mCommon.mParticleManagerMtx);

    math::VEC3 axisY(mCommon.mEmitterMtx._01, mCommon.mEmitterMtx._11,
                     mCommon.mEmitterMtx._21);
    math::VEC3TransformNormal(&axisY, &mCommon.mParticleManagerMtxInv, &axisY);

    if (!Normalize(&axisY)) {
        axisY = math::VEC3(0.0f, 1.0f, 0.0f);
    }

    mCommon.mEmitterAxisY = axisY;

    math::VEC3 center(mCommon.mEmitterMtx._03, mCommon.mEmitterMtx._13,
                      mCommon.mEmitterMtx._23);
    math::VEC3TransformCoord(&center, &mCommon.mParticleManagerMtxInv, &center);

    mCommon.mEmitterCenter = center;

    EmitterDrawSetting& rDesc = *pManager->mResource->GetEmitterDrawSetting();

    switch (rDesc.typeDir) {
    case EmitterDrawSetting::AHEAD_CMN_NODESIGN:
    case EmitterDrawSetting::AHEAD_CMN_NODESIGN2: {
        math::MTX34& rMtx = mCommon.mParticleManagerMtxInv;
        math::VEC3 worldAxisY(rMtx._01, rMtx._11, rMtx._21);

        if (!Normalize(&worldAxisY)) {
            worldAxisY = math::VEC3(0.0f, 1.0f, 0.0f);
        }

        mNoDesign.mWorldYAxis = worldAxisY;
        break;
    }

    default: {
        break;
    }
    }
}

} // namespace ef
} // namespace nw4r
