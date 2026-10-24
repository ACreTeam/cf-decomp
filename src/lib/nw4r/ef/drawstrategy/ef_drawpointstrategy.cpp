#include <nw4r/ef.h>
#include <nw4r/NW4RAssert.hpp>
#include <nw4r/math.h>

#include <revolution/gx.h>

#include <algorithm>
#include <limits>

namespace nw4r {
namespace ef {

static void DrawPoint(const math::VEC3& rPos, bool texCoord);

DrawPointStrategy::DrawPointStrategy() {}

void DrawPointStrategy::Draw(const DrawInfo &drawInfo, ParticleManager *pm) {
    InitGraphics(drawInfo, pm);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    math::MTX34 mtxM;
    pm->CalcGlobalMtx(&mtxM);

    math::MTX34 mtxVM;
    drawInfo.GetViewMtxCooked(&mtxVM);
    math::MTX34Mult(&mtxVM, &mtxVM, &mtxM);
    GXLoadPosMtxImm(mtxVM, GX_PNMTX0);

    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    u8 prevPointSize = 0;
    bool first = true;

    for (Particle *pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        f32 sx = pp->Draw_GetSizeX();
        if (sx < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        u8 pointSize = static_cast<u8>(6.0f * std::min(42.5f, sx));
        if (pointSize == 0) {
            continue;
        }

        if (prevPointSize != pointSize) {
            prevPointSize = pointSize;
            GXSetPointSize(pointSize, GX_TO_ONE);

            SetupGP(pp, ed, drawInfo, first, true);
        } else {
            SetupGP(pp, ed, drawInfo, first, false);
        }

        first = false;
        DrawPoint(pp->mParameter.mPosition, mNumTexmap > 0);
    }
}

static void DrawPoint(const math::VEC3& rPos, bool texCoord) {
    GXBegin(GX_POINTS, GX_VTXFMT0, 1);
    {
        GXPosition(rPos);

        if (texCoord) {
            GXTexCoord2f32(0.0f, 0.0f);
        }
    }
    GXEnd();
}

void DrawPointStrategy::InitGraphics(const DrawInfo& rInfo,
                                     ParticleManager* pManager) {
    NW4RAssertPointerNonnull(pManager);

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    Initialize(pManager, rSetting, rInfo);

    GXEnableTexOffsets(GX_TEXCOORD0, TRUE, TRUE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    if (mNumTexmap > 0) {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    }

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GXSetCurrentMtx(GX_PNMTX0);
}

} // namespace ef
} // namespace nw4r
