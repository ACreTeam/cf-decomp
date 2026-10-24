#include <nw4r/ef.h>
#include <nw4r/NW4RAssert.hpp>

#include <limits>

namespace nw4r {
namespace ef {

DrawBillboardStrategy::DrawBillboardStrategy() {}

static u8 billboard_tex0_u8[] ATTRIBUTE_ALIGN(32) = {0x00, 0x01, 0x00, 0x00,
                                                     0x01, 0x00, 0x01, 0x01};

static inline bool texEnable(int flags) {
    return (flags & 1) != 0;
}

inline void DrawBillboardStrategy::DispPolygon(const math::VEC3 &p,
                                               const math::VEC3 &d0,
                                               const math::VEC3 &d1,
                                               int flags) {
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GXPosition(p - d0);
        if (texEnable(flags)) {
            GXTexCoord1x8(0);
        }

        GXPosition(p - d1);
        if (texEnable(flags)) {
            GXTexCoord1x8(1);
        }

        GXPosition(p + d0);
        if (texEnable(flags)) {
            GXTexCoord1x8(2);
        }

        GXPosition(p + d1);
        if (texEnable(flags)) {
            GXTexCoord1x8(3);
        }
    }
    GXEnd();
}

inline void DrawBillboardStrategy::DispParticle_Normal(
    Particle *particle, const register math::MTX34 *vm, register f32 vx,
    register f32 vy, register f32 rc, register f32 rs, register f32 sx,
    register f32 sy, const register math::VEC2 *pivot, int flags) {

    math::VEC3 particleRotate;
    particle->Draw_GetRotate(&particleRotate);

    if (particleRotate.z != 0.0f) {
        math::VEC3 pos;
        math::VEC3 p0;
        math::VEC3 d0;
        math::VEC3 d1;

        math::VEC3Transform(&pos, vm, &particle->mParameter.mPosition);

        f32 px = pivot->x;
        f32 py = pivot->y;

        f32 vxrc = vx * rc;
        f32 vxrs = vx * rs;
        f32 vyrc = vy * rc;
        f32 vyrs = vy * rs;

        f32 cr;
        f32 sr;
        PSSinCosRad(&sr, &cr, -particleRotate.z);

        f32 crsx = cr * sx;
        f32 srsx = sr * sx;
        f32 crsy = cr * sy;
        f32 srsy = sr * sy;

        f32 exp0 = px - crsx * px - srsy * py;
        f32 exp1 = py + srsx * px - crsy * py;

        p0.x = vxrc * exp0 + vyrs * exp1 + pos.x;
        p0.y = vxrs * exp0 - vyrc * exp1 + pos.y;
        p0.z = pos.z;

        f32 vxrccrsx = vxrc * crsx;
        f32 vxrcsrsy = vxrc * srsy;
        f32 vyrssrsx = vyrs * srsx;
        f32 vyrscrsy = vyrs * crsy;
        f32 vxrscrsx = vxrs * crsx;
        f32 vxrssrsy = vxrs * srsy;
        f32 vyrcsrsx = vyrc * srsx;
        f32 vyrccrsy = vyrc * crsy;

        d0.x = vxrccrsx - vxrcsrsy - vyrssrsx - vyrscrsy;
        d0.y = vxrscrsx - vxrssrsy + vyrcsrsx + vyrccrsy;
        d0.z = 0.0f;

        d1.x = vxrccrsx + vxrcsrsy - vyrssrsx + vyrscrsy;
        d1.y = vxrscrsx + vxrssrsy + vyrcsrsx - vyrccrsy;
        d1.z = 0.0f;

        DispPolygon(p0, d0, d1, flags);
    } else {
        math::VEC3 pos[4];

        register f32 *ptclpos = &particle->mParameter.mPosition.x;
        register f32 *ptr = &pos[0].x;

        register __vec2x32float__ exp;
        register __vec2x32float__ vxrcsx_vxrssx;
        register __vec2x32float__ vyrssy_mvyrcsy;
        register __vec2x32float__ r;
        register __vec2x32float__ vxrc_vxrs;
        register __vec2x32float__ vyrs_mvyrc;
        register __vec2x32float__ pxpy;
        register __vec2x32float__ rsrc;
        register __vec2x32float__ sxsy;
        register __vec2x32float__ p0;
        register __vec2x32float__ d1;
        register __vec2x32float__ d0;
        register __vec2x32float__ posxy;
        register __vec2x32float__ posz;

        // clang-format off
        asm {
            psq_l      fp0, 0(ptclpos), 0, 0
            psq_l      fp2, 0(vm), 0, 0
            psq_l      fp1, 8(ptclpos), 1, 0
            ps_mul     fp4, fp2, fp0
            psq_l      fp3, 8(vm), 0, 0
            ps_madd    fp5, fp3, fp1, fp4
            psq_l      fp8, 16(vm), 0, 0
            ps_sum0    fp6, fp5, fp6, fp5
            psq_l      fp9, 24(vm), 0, 0
            ps_mul     fp10, fp8, fp0
            ps_madd    fp11, fp9, fp1, fp10
            psq_l      fp2, 32(vm), 0, 0
            ps_sum0    fp12, fp11, fp12, fp11
            psq_l      fp3, 40(vm), 0, 0
            ps_mul     fp4, fp2, fp0
            ps_merge00 posxy, fp6, fp12
            ps_madd    fp5, fp3, fp1, fp4
            ps_sum0    posz, fp5, fp6, fp5

            psq_l      pxpy, 0(pivot), 0, 0
            ps_merge00 rsrc, rs, rc
            ps_merge00 sxsy, sx, sy
            ps_muls0   vxrc_vxrs, rsrc, vx
            ps_merge10 vxrc_vxrs, vxrc_vxrs, vxrc_vxrs
            ps_muls0   vyrs_mvyrc, rsrc, vy
            ps_neg     r, vyrs_mvyrc
            ps_merge01 vyrs_mvyrc, vyrs_mvyrc, r
            ps_nmsub   exp, sxsy, pxpy, pxpy
            ps_madds0  r, vxrc_vxrs, exp, posxy
            ps_madds1  p0, vyrs_mvyrc, exp, r

            psq_st     posz, 8(ptr), 1, 0
            psq_st     posz, 20(ptr), 1, 0
            psq_st     posz, 32(ptr), 1, 0
            psq_st     posz, 44(ptr), 1, 0

            ps_muls0   vxrcsx_vxrssx, vxrc_vxrs, sxsy
            ps_muls1   vyrssy_mvyrcsy, vyrs_mvyrc, sxsy
            ps_add     d1, vxrcsx_vxrssx, vyrssy_mvyrcsy
            ps_neg     vyrssy_mvyrcsy, vyrssy_mvyrcsy
            ps_add     d0, vxrcsx_vxrssx, vyrssy_mvyrcsy

            ps_sub     r, p0, d0
            psq_st     r, 0(ptr), 0, 0
            ps_sub     r, p0, d1
            psq_st     r, 12(ptr), 0, 0
            ps_add     r, p0, d0
            psq_st     r, 24(ptr), 0, 0
            ps_add     r, p0, d1
            psq_st     r, 36(ptr), 0, 0
        }
        // clang-format on

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        {
            GXPosition(pos[0]);
            if (texEnable(flags)) {
                GXTexCoord1x8(0);
            }

            GXPosition(pos[1]);
            if (texEnable(flags)) {
                GXTexCoord1x8(1);
            }

            GXPosition(pos[2]);
            if (texEnable(flags)) {
                GXTexCoord1x8(2);
            }

            GXPosition(pos[3]);
            if (texEnable(flags)) {
                GXTexCoord1x8(3);
            }
        }
        GXEnd();
    }
}

inline void DrawBillboardStrategy::DispParticle_YBillboard(
    Particle *particle, const math::MTX34 &vm, f32 vx, f32 vy, f32 rc, f32 rs,
    f32 sx, f32 sy, const math::VEC2 &pivot, int flags) {

    math::VEC3 particleRotate;
    particle->Draw_GetRotate(&particleRotate);

    math::VEC3 pos;
    math::VEC3Transform(&pos, &vm, &particle->mParameter.mPosition);

    f32 vyrc = vy * rc;
    f32 vyrs = vy * rs;
    f32 px = pivot.x;
    f32 py = pivot.y;

    math::VEC3 p0;
    math::VEC3 d0;
    math::VEC3 d1;

    if (particleRotate.z != 0.0f) {
        f32 sr;
        f32 cr;
        PSSinCosRad(&sr, &cr, -particleRotate.z);

        f32 crsx = cr * sx;
        f32 srsx = sr * sx;
        f32 crsy = cr * sy;
        f32 srsy = sr * sy;

        f32 exp0 = -py - srsx * px + crsy * py;

        p0.x = vx * (px - crsx * px - srsy * py) + pos.x;
        p0.y = vyrc * exp0 + pos.y;
        p0.z = vyrs * exp0 + pos.z;

        d0.x = vx * (crsx - srsy);
        d0.y = vyrc * (srsx + crsy);
        d0.z = vyrs * (srsx + crsy);

        d1.x = vx * (crsx + srsy);
        d1.y = vyrc * (srsx - crsy);
        d1.z = vyrs * (srsx - crsy);
    } else {
        p0.x = vx * (px - px * sx) + pos.x;
        p0.y = vyrc * (py * sy - py) + pos.y;
        p0.z = vyrs * (py * sy - py) + pos.z;

        d0.x = vx * sx;
        d0.y = vyrc * sy;
        d0.z = vyrs * sy;

        d1.x = vx * sx;
        d1.y = -vyrc * sy;
        d1.z = -vyrs * sy;
    }

    DispPolygon(p0, d0, d1, flags);
}

inline void DrawBillboardStrategy::DispParticle_Directional(
    Particle *particle, const math::MTX34 &vm, f32 vx, f32 vy, f32 rc, f32 rs,
    f32 sx, f32 sy, f32 vs, const math::VEC2 &pivot, int flags) {

    math::VEC3 pos;
    math::VEC3Transform(&pos, &vm, &particle->mParameter.mPosition);

    f32 px = pivot.x;
    f32 py = pivot.y;

    f32 vxrc = vx * rc;
    f32 vxrs = vx * rs;
    f32 vyrc = vy * rc;
    f32 vyrs = vy * rs;

    f32 exp0 = px - sx * px;
    f32 exp1 = (vs + py - 1.0f) * sy - py;

    math::VEC3 p0(vxrc * exp0 + vyrs * exp1 + pos.x,
                  vxrs * exp0 - vyrc * exp1 + pos.y, pos.z);

    f32 vxrcsx = vxrc * sx;
    f32 vxrssx = vxrs * sx;
    f32 vyrssyvs = vyrs * sy * vs;
    f32 vyrcsyvs = vyrc * sy * vs;

    math::VEC3 d0(vxrcsx - vyrssyvs, vxrssx + vyrcsyvs, 0.0f);
    math::VEC3 d1(vxrcsx + vyrssyvs, vxrssx - vyrcsyvs, 0.0f);

    DispPolygon(p0, d0, d1, flags);
}

void DrawBillboardStrategy::Draw(const DrawInfo &drawInfo,
                                 ParticleManager *pm) {
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    switch (ed.typeOption) {
    case EmitterDrawSetting::ASSIST_BB_NORMAL:
    case EmitterDrawSetting::ASSIST_BB_NOROLL: {
        DrawNormalBillboard(drawInfo, pm);
        break;
    }

    case EmitterDrawSetting::ASSIST_BB_Y: {
        DrawYBillboard(drawInfo, pm);
        break;
    }

    case EmitterDrawSetting::ASSIST_BB_DIRECTIONAL: {
        DrawDirectionalBillboard(drawInfo, pm);
        break;
    }

    default: {
        break;
    }
    }
}

void DrawBillboardStrategy::DrawNormalBillboard(const DrawInfo &drawInfo,
                                                ParticleManager *pm) {
    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();
    InitGraphics(drawInfo, pm);

    const int flags = mNumTexmap > 0 ? 1 : 0;

    math::VEC2 pivot;
    pivot.x = ed.pivotX / 100.0f;
    pivot.y = ed.pivotY / 100.0f;

    math::MTX34 emMtx;
    pm->CalcGlobalMtx(&emMtx);
    math::MTX34Mult(&emMtx, &cameraMtx, &emMtx);

    f32 vx = MTXColLen(&emMtx, 0);
    f32 vy = MTXColLen(&emMtx, 1);

    f32 rc;
    f32 rs;
    f32 mag = math::FSqrt(cameraMtx._01 * cameraMtx._01 +
                          cameraMtx._11 * cameraMtx._11);

    if (ed.typeOption == EmitterDrawSetting::ASSIST_BB_NOROLL || mag == 0.0f) {
        rs = 0.0f;
        rc = 1.0f;
    } else {
        f32 denom = math::FInv(mag);
        rs = -cameraMtx._01 * denom;
        rc = cameraMtx._11 * denom;
    }

    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    bool first = true;

    for (Particle *pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        f32 sx = pp->Draw_GetSizeX();
        if (sx < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        f32 sy = pp->Draw_GetSizeY();
        if (sy < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        SetupGP(pp, ed, drawInfo, first, false);
        first = false;

        DispParticle_Normal(pp, &emMtx, vx, vy, rc, rs, sx, sy, &pivot, flags);
    }
}

void DrawBillboardStrategy::DrawYBillboard(const DrawInfo &drawInfo,
                                           ParticleManager *pm) {
    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();
    InitGraphics(drawInfo, pm);

    const int flags = mNumTexmap > 0 ? 1 : 0;

    math::VEC2 pivot(ed.pivotX / 100.0f, ed.pivotY / 100.0f);

    math::MTX34 emMtx;
    pm->CalcGlobalMtx(&emMtx);
    math::MTX34Mult(&emMtx, &cameraMtx, &emMtx);

    f32 vx = MTXColLen(&emMtx, 0);
    f32 vy = MTXColLen(&emMtx, 1);

    f32 rc;
    f32 rs;
    f32 mag = math::FSqrt(cameraMtx._11 * cameraMtx._11 +
                          cameraMtx._21 * cameraMtx._21);

    if (mag == 0.0f) {
        rs = 1.0f;
        rc = 0.0f;
    } else {
        f32 denom = math::FInv(mag);
        rs = cameraMtx._21 * denom;
        rc = cameraMtx._11 * denom;
    }

    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    bool first = true;

    for (Particle *pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        if (pp->GetLifeStatus() != ReferencedObject::NW4R_EF_LS_ACTIVE) {
            continue;
        }

        f32 sx = pp->Draw_GetSizeX();
        if (sx < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        f32 sy = pp->Draw_GetSizeY();
        if (sy < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        SetupGP(pp, ed, drawInfo, first, false);
        first = false;

        DispParticle_YBillboard(pp, emMtx, vx, vy, rc, rs, sx, sy, pivot,
                                flags);
    }
}

void DrawBillboardStrategy::DrawDirectionalBillboard(const DrawInfo &drawInfo,
                                                     ParticleManager *pm) {
    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();
    InitGraphics(drawInfo, pm);

    const int flags = mNumTexmap > 0 ? 1 : 0;

    math::VEC2 pivot;
    pivot.x = ed.pivotX / 100.0f;
    pivot.y = ed.pivotY / 100.0f;

    math::MTX34 emMtx;
    pm->CalcGlobalMtx(&emMtx);
    math::MTX34Mult(&emMtx, &cameraMtx, &emMtx);

    f32 vx = MTXColLen(&emMtx, 0);
    f32 vy = MTXColLen(&emMtx, 1);

    AheadContext aheadContext(cameraMtx, pm);
    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    bool first = true;

    for (Particle *pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        if (pp->GetLifeStatus() != ReferencedObject::NW4R_EF_LS_ACTIVE) {
            continue;
        }

        f32 sx = pp->Draw_GetSizeX();
        if (sx < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        f32 sy = pp->Draw_GetSizeY();
        if (sy < std::numeric_limits<f32>::epsilon()) {
            continue;
        }

        SetupGP(pp, ed, drawInfo, first, false);
        first = false;

        math::VEC3 yAxis;
        calcAhead(&yAxis, &aheadContext, pp);
        math::VEC3TransformNormal(&yAxis, &emMtx, &yAxis);

        f32 rc;
        f32 rs;
        f32 mag = math::FSqrt(yAxis.x * yAxis.x + yAxis.y * yAxis.y);

        if (mag == 0.0f) {
            rs = 0.0f;
            rc = 1.0f;
        } else {
            f32 denom = math::FInv(mag);
            rs = -yAxis.x * denom;
            rc = yAxis.y * denom;
        }

        f32 vs = 1.0f;

        if (ed.typeOption0) {
            math::VEC3 v;
            pp->GetMoveDir(&v);
            vs += 0.5f * math::FSqrt(math::VEC3Dot(&v, &v)) / sy;
        }

        DispParticle_Directional(pp, emMtx, vx, vy, rc, rs, sx, sy, vs, pivot,
                                 flags);
    }
}

void DrawBillboardStrategy::InitGraphics(const DrawInfo &drawInfo,
                                         ParticleManager *pm) {
    NW4RAssertPointerNonnull(pm);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    Initialize(pm, ed, drawInfo);

    GXEnableTexOffsets(GX_TEXCOORD0, GX_TRUE, GX_TRUE);

    GXSetArray(GX_VA_TEX0, billboard_tex0_u8, 2);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    if (mNumTexmap > 0) {
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    }

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);

    math::MTX34 vm;
    math::MTX34Identity(&vm);
    GXLoadPosMtxImm(vm, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

DrawStrategyImpl::CalcAheadFunc
DrawBillboardStrategy::GetCalcAheadFunc(ParticleManager *pm) {
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    switch (ed.typeDir) {
    case EmitterDrawSetting::AHEAD_BB_SPEED: {
        return CalcAhead_Speed;
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_CENTER: {
        return CalcAhead_EmitterCenter;
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_DESIGN: {
        return CalcAhead_EmitterDesign;
    }

    case EmitterDrawSetting::AHEAD_BB_PARTICLE: {
        return CalcAhead_Particle;
    }

    case EmitterDrawSetting::AHEAD_BB_PARTICLE_BOTH: {
        return CalcAhead_ParticleBoth;
    }

    default: {
        return CalcAhead_Speed;
    }
    }
}

} // namespace ef
} // namespace nw4r
