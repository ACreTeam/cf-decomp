// EGG::Graphics. .text 80445D18..80445F80. See include/lib/egg/gfx/eggDrawHelper.h.
#include <lib/egg/gfx/eggDrawHelper.h>
#include <revolution/GX.h>

namespace EGG {

// 80445D18: GX state for untextured, vertex-coloured drawing: one channel lit from the vertex colour, alpha
// blending, a single pass-colour TEV stage, position + colour vertices.
void Graphics::setupGX() {
    GXColor mat = {0xFF, 0xFF, 0xFF, 0xFF};
    GXColor amb = {0x00, 0x00, 0x00, 0xFF};
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, amb);
    GXSetChanMatColor(GX_COLOR0A0, mat);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetCoPlanar(GX_FALSE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumTevStages(1);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevDirect(GX_TEVSTAGE4);
    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevDirect(GX_TEVSTAGE6);
    GXSetTevDirect(GX_TEVSTAGE7);
    GXSetTevDirect(GX_TEVSTAGE8);
    GXSetTevDirect(GX_TEVSTAGE9);
    GXSetTevDirect(GX_TEVSTAGE10);
    GXSetTevDirect(GX_TEVSTAGE11);
    GXSetTevDirect(GX_TEVSTAGE12);
    GXSetTevDirect(GX_TEVSTAGE13);
    GXSetTevDirect(GX_TEVSTAGE14);
    GXSetTevDirect(GX_TEVSTAGE15);
    GXSetNumIndStages(0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

// 8074FC80
Color Graphics::mColor;

} // namespace EGG
