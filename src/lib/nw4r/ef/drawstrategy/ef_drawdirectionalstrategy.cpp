#include <nw4r/ef.h>
#include <nw4r/NW4RAssert.hpp>
#include <nw4r/math.h>

namespace nw4r {
namespace ef {

DrawDirectionalStrategy::DrawDirectionalStrategy() {}

static u8 directional_tex0_u8[] ATTRIBUTE_ALIGN(32) = {0x00, 0x01, 0x00, 0x00,
                                                       0x01, 0x00, 0x01, 0x01};

static void DrawQuad(const math::MTX34& m, const math::_VEC3* p,
                     bool texCoord) {
    math::VEC3 p0;
    math::VEC3 p1;
    math::VEC3 p2;
    math::VEC3 p3;

    math::VEC3Transform(&p0, &m, static_cast<const math::VEC3*>(&p[0]));
    math::VEC3Transform(&p1, &m, static_cast<const math::VEC3*>(&p[1]));
    math::VEC3Transform(&p2, &m, static_cast<const math::VEC3*>(&p[2]));
    math::VEC3Transform(&p3, &m, static_cast<const math::VEC3*>(&p[3]));

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GXPosition(p0);
        if (texCoord) {
            GXTexCoord1x8(0);
        }

        GXPosition(p1);
        if (texCoord) {
            GXTexCoord1x8(1);
        }

        GXPosition(p2);
        if (texCoord) {
            GXTexCoord1x8(2);
        }

        GXPosition(p3);
        if (texCoord) {
            GXTexCoord1x8(3);
        }
    }
    GXEnd();
}

static void DrawQuad(const math::VEC3& p, const math::VEC3& d0,
                     const math::VEC3& d1, bool texCoord) {
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GXPosition(p - d0);
        if (texCoord) {
            GXTexCoord1x8(0);
        }

        GXPosition(p - d1);
        if (texCoord) {
            GXTexCoord1x8(1);
        }

        GXPosition(p + d0);
        if (texCoord) {
            GXTexCoord1x8(2);
        }

        GXPosition(p + d1);
        if (texCoord) {
            GXTexCoord1x8(3);
        }
    }
    GXEnd();
}

static inline math::MTX34 CalcRotate(Particle* pp, u8 typeAxis) {
    math::VEC3 particleRotate;
    pp->Draw_GetRotate(&particleRotate);

    f32 sx = 0.0f;
    f32 cx = 1.0f;
    f32 sy = 0.0f;
    f32 cy = 1.0f;
    f32 sz = 0.0f;
    f32 cz = 1.0f;

    switch (typeAxis) {
    case AXIS_X: {
        if (particleRotate.x != 0.0f) {
            PSSinCosRad(&sx, &cx, -particleRotate.x);
        }

        // clang-format off
        return math::MTX34(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f,   cx,  -sx, 0.0f,
            0.0f,   sx,   cx, 0.0f);
        // clang-format on
    }

    case AXIS_Y: {
        if (particleRotate.y != 0.0f) {
            PSSinCosRad(&sy, &cy, -particleRotate.y);
        }

        // clang-format off
        return math::MTX34(
              cy, 0.0f,   sy, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
             -sy, 0.0f,   cy, 0.0f);
        // clang-format on
    }

    case AXIS_Z: {
        if (particleRotate.z != 0.0f) {
            PSSinCosRad(&sz, &cz, -particleRotate.z);
        }

        // clang-format off
        return math::MTX34(
              cz,  -sz, 0.0f, 0.0f,
              sz,   cz, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f);
        // clang-format on
    }

    case 3:
    default: {
        if (particleRotate.x != 0.0f) {
            PSSinCosRad(&sx, &cx, -particleRotate.x);
        }

        if (particleRotate.y != 0.0f) {
            PSSinCosRad(&sy, &cy, -particleRotate.y);
        }

        if (particleRotate.z != 0.0f) {
            PSSinCosRad(&sz, &cz, -particleRotate.z);
        }

        f32 sx_m_sy = sx * sy;
        f32 cx_m_sy = cx * sy;

        // clang-format off
        return math::MTX34(
            cy * cz,  sx_m_sy * cz - cx * sz,  cx_m_sy * cz + sx * sz,  0.0f,
            cy * sz,  sx_m_sy * sz + cx * cz,  cx_m_sy * sz - sx * cz,  0.0f,
                -sy,                 cy * sx,                 cx * cy,  0.0f);
        // clang-format on
    }
    }
}

static inline math::MTX34 CalcLocalTransform(u8 face, f32 px, f32 py, f32 sx,
                                             f32 sy, f32 vs,
                                             const math::MTX34& rot) {
    switch (face) {
    case 0: {
        f32 sxpx = sx * px;
        f32 syvs = sy * vs;
        f32 exp0 = sy * (py + vs - 1.0f);

        // clang-format off
        return math::MTX34(
            rot._00 * sx, rot._01 * syvs, rot._02 * sx, -rot._00 * sxpx - rot._01 * exp0 + px,
            rot._10 * sx, rot._11 * syvs, rot._12 * sx, -rot._10 * sxpx - rot._11 * exp0 + py,
            rot._20 * sx, rot._21 * syvs, rot._22 * sx, -rot._20 * sxpx - rot._21 * exp0);
        // clang-format on
    }

    case 1:
    default: {
        f32 sxpx = sx * px;
        f32 sypy = sy * py;

        // clang-format off
        return math::MTX34(
            rot._00 * sx, rot._02 * sy, rot._01 * sx, -rot._00 * sxpx - rot._02 * sypy + px,
            rot._10 * sx, rot._12 * sy, rot._11 * sx, -rot._10 * sxpx - rot._12 * sypy,
            rot._20 * sx, rot._22 * sy, rot._21 * sx, -rot._20 * sxpx - rot._22 * sypy - py);
        // clang-format on
    }
    }
}

void DrawDirectionalStrategy::Draw(const DrawInfo& drawInfo,
                                   ParticleManager* pm) {
    InitGraphics(drawInfo, pm);

    const EmitterDrawSetting& ed = *pm->mResource->GetEmitterDrawSetting();

    switch (ed.typeOption2 & EmitterDrawSetting::DIRECTIONAL_PIVOT__MASK) {
    default:
    case EmitterDrawSetting::DIRECTIONAL_PIVOT_NOP: {
        DrawDirectional(drawInfo, pm);
        break;
    }

    case EmitterDrawSetting::DIRECTIONAL_PIVOT_BILLBOARD: {
        DrawDirectionalBillboard(drawInfo, pm);
        break;
    }
    }
}

void DrawDirectionalStrategy::DrawDirectional(const DrawInfo& drawInfo,
                                              ParticleManager* pm) {
    InitGraphics(drawInfo, pm);

    const EmitterDrawSetting& ed = *pm->mResource->GetEmitterDrawSetting();

    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    AheadContext aheadContext(cameraMtx, pm);
    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    math::VEC3 refYAxis;
    math::VEC3 refZAxis;

    if (ed.typeDir == EmitterDrawSetting::AHEAD_CMN_NODESIGN2) {
        math::MTX34* mtxM = &aheadContext.mCommon.mParticleManagerMtx;
        math::MTX34* mtxMInv = &aheadContext.mCommon.mParticleManagerMtxInv;

        math::MTX34 mtxVM;
        math::MTX34Mult(&mtxVM, &cameraMtx, mtxM);
        GXLoadPosMtxImm(mtxVM, GX_PNMTX0);

        refYAxis = math::VEC3(mtxMInv->_01, mtxMInv->_11, mtxMInv->_21);
        refZAxis = math::VEC3(mtxMInv->_02, mtxMInv->_12, mtxMInv->_22);
        Normalize(&refZAxis);
    } else {
        math::MTX34* mtxM = &aheadContext.mCommon.mParticleManagerMtx;

        math::MTX34 mtxVM;
        math::MTX34Mult(&mtxVM, &cameraMtx, mtxM);
        GXLoadPosMtxImm(mtxVM, GX_PNMTX0);

        math::MTX34 mtxEM;
        pm->mManagerEM->CalcGlobalMtx(&mtxEM);

        math::MTX34* mtxMInv = &aheadContext.mCommon.mParticleManagerMtxInv;
        math::MTX34Mult(&mtxEM, mtxMInv, &mtxEM);

        refYAxis = math::VEC3(mtxEM._01, mtxEM._11, mtxEM._21);
        refZAxis = math::VEC3(mtxEM._02, mtxEM._12, mtxEM._22);
        Normalize(&refZAxis);
    }

    for (Particle* pp = GetYoungestParticle(pm);
         pp != NULL && pp->mPrevAxis.x > 1.0f; pp = GetElderParticle(pm, pp)) {
        pp->mPrevAxis = refYAxis;
    }

    f32 pvx = ed.pivotX / 100.0f;
    f32 pvy = ed.pivotY / 100.0f;

    GetFirstDrawParticleFunc getFirst =
        GetGetFirstDrawParticleFunc(ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext =
        GetGetNextDrawParticleFunc(ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    bool first = true;

    for (Particle* pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        f32 sx = pp->Draw_GetSizeX();
        if (sx < NW4R_MATH_FLT_EPSILON) {
            continue;
        }

        f32 sy = pp->Draw_GetSizeY();
        if (sy < NW4R_MATH_FLT_EPSILON) {
            continue;
        }

        f32 vs = 1.0f;

        if (ed.typeOption0) {
            math::VEC3 v;
            pp->GetMoveDir(&v);
            vs += 0.5f * math::FSqrt(math::VEC3Dot(&v, &v)) / sy;
        }

        SetupGP(pp, ed, drawInfo, first, false);
        first = false;

        math::MTX34 mtxRotate = CalcRotate(pp, ed.typeAxis);
        math::MTX34 mtxLocalTransform = CalcLocalTransform(
            ed.typeOption1, pvx, pvy, sx, sy, vs, mtxRotate);

        math::VEC3 xAxis;
        math::VEC3 yAxis;
        math::VEC3 zAxis;

        calcAhead(&yAxis, &aheadContext, pp);
        math::VEC3Cross(&zAxis, &pp->mPrevAxis, &yAxis);

        if (!Normalize(&zAxis)) {
            zAxis = refZAxis;
        }

        math::VEC3Cross(&xAxis, &yAxis, &zAxis);
        pp->mPrevAxis = xAxis;

        math::VEC3 pos = pp->mParameter.mPosition;

        // clang-format off
        math::MTX34 mtxParticle(
            xAxis.x, yAxis.x, zAxis.x, pos.x,
            xAxis.y, yAxis.y, zAxis.y, pos.y,
            xAxis.z, yAxis.z, zAxis.z, pos.z);
        // clang-format on

        math::MTX34 mtx;
        math::MTX34Mult(&mtx, &mtxParticle, &mtxLocalTransform);

        // clang-format off
        static const math::_VEC3 p[4] = {
            -1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f
        };
        // clang-format on

        DrawQuad(mtx, p, mNumTexmap > 0);

        if (ed.typeOption == EmitterDrawSetting::TYPE_CMN_CROSS) {
            // clang-format off
            static const math::_VEC3 px[4] = {
                0.0f, -1.0f,  1.0f,
                0.0f,  1.0f,  1.0f,
                0.0f,  1.0f, -1.0f,
                0.0f, -1.0f, -1.0f
            };
            // clang-format on

            DrawQuad(mtx, px, mNumTexmap > 0);
        }
    }
}

void DrawDirectionalStrategy::DrawDirectionalBillboard(const DrawInfo& drawInfo,
                                                       ParticleManager* pm) {
    InitGraphics(drawInfo, pm);

    const EmitterDrawSetting& ed = *pm->mResource->GetEmitterDrawSetting();

    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    AheadContext aheadContext(cameraMtx, pm);
    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    math::MTX34 mtxVM;
    {
        math::MTX34 mtxM;
        pm->CalcGlobalMtx(&mtxM);
        math::MTX34Mult(&mtxVM, &cameraMtx, &mtxM);
    }

    GXLoadPosMtxImm(mIdentityMtx, GX_PNMTX0);

    f32 vx = MTXColLen(&mtxVM, 0);
    f32 vy = MTXColLen(&mtxVM, 1);
    f32 vz = MTXColLen(&mtxVM, 2);

    for (Particle* pp = GetYoungestParticle(pm);
         pp != NULL && pp->mPrevAxis.x > 1.0f; pp = GetElderParticle(pm, pp)) {
        pp->mPrevAxis = mZeroVec;
    }

    f32 pvx = ed.pivotX / 100.0f;
    f32 pvy = ed.pivotY / 100.0f;

    GetFirstDrawParticleFunc getFirst =
        GetGetFirstDrawParticleFunc(ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    GetNextDrawParticleFunc getNext =
        GetGetNextDrawParticleFunc(ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER);

    bool first = true;

    for (Particle* pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
        f32 sx = pp->Draw_GetSizeX();
        if (sx < NW4R_MATH_FLT_EPSILON) {
            continue;
        }

        f32 sy = pp->Draw_GetSizeY();
        if (sy < NW4R_MATH_FLT_EPSILON) {
            continue;
        }

        f32 vs = 1.0f;

        if (ed.typeOption0) {
            math::VEC3 v;
            pp->GetMoveDir(&v);
            vs += 0.5f * math::FSqrt(math::VEC3Dot(&v, &v)) / sy;
        }

        SetupGP(pp, ed, drawInfo, first, false);
        first = false;

        math::MTX34 mtxRotate = CalcRotate(pp, ed.typeAxis);
        math::MTX34 mtxLocalTransform = CalcLocalTransform(
            ed.typeOption1, pvx, pvy, sx, sy, vs, mtxRotate);

        math::VEC3 xAxis;
        math::VEC3 yAxis;
        math::VEC3 zAxis;
        math::VEC3 dir;

        calcAhead(&dir, &aheadContext, pp);
        math::VEC3TransformNormal(&dir, &mtxVM, &dir);

        yAxis = dir;
        Normalize(&yAxis);

        xAxis = math::VEC3(dir.y, -dir.x, 0.0f);

        if (Normalize(&xAxis)) {
            zAxis = math::VEC3(-dir.x * dir.z, -dir.y * dir.z,
                               dir.x * dir.x + dir.y * dir.y);
        } else {
            math::VEC3Cross(&xAxis, &dir, &pp->mPrevAxis);

            if (!Normalize(&xAxis)) {
                xAxis = mXUnitVec;
            }

            math::VEC3Cross(&zAxis, &xAxis, &dir);
        }

        Normalize(&zAxis);
        pp->mPrevAxis = zAxis;

        math::VEC3 pos;
        math::VEC3Transform(&pos, &mtxVM, &pp->mParameter.mPosition);

        // clang-format off
        math::MTX34 mtxParticle(
            vx * xAxis.x, vy * yAxis.x, vz * zAxis.x, pos.x,
            vx * xAxis.y, vy * yAxis.y, vz * zAxis.y, pos.y,
            vx * xAxis.z, vy * yAxis.z, vz * zAxis.z, pos.z);
        // clang-format on

        math::MTX34 mtx;
        math::MTX34Mult(&mtx, &mtxParticle, &mtxLocalTransform);

        math::VEC3 vp0(mtx._03, mtx._13, mtx._23);
        math::VEC3 vd0(mtx._00 + mtx._01, mtx._10 + mtx._11, mtx._20 + mtx._21);
        math::VEC3 vd1(mtx._00 - mtx._01, mtx._10 - mtx._11, mtx._20 - mtx._21);
        DrawQuad(vp0, vd0, vd1, mNumTexmap > 0);

        if (ed.typeOption == EmitterDrawSetting::TYPE_CMN_CROSS) {
            math::VEC3 vd2(mtx._01 + mtx._02, mtx._11 + mtx._12,
                           mtx._21 + mtx._22);
            math::VEC3 vd3(mtx._01 - mtx._02, mtx._11 - mtx._12,
                           mtx._21 - mtx._22);
            DrawQuad(vp0, vd2, vd3, mNumTexmap > 0);
        }
    }
}

void DrawDirectionalStrategy::InitGraphics(const DrawInfo& drawInfo,
                                           ParticleManager* pm) {
    NW4RAssertPointerNonnull(pm);

    const EmitterDrawSetting& ed = *pm->mResource->GetEmitterDrawSetting();

    Initialize(pm, ed, drawInfo);

    GXEnableTexOffsets(GX_TEXCOORD0, GX_TRUE, GX_TRUE);

    GXSetArray(GX_VA_TEX0, directional_tex0_u8, 2);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    if (mNumTexmap > 0) {
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    }

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);

    GXSetCurrentMtx(GX_PNMTX0);
}

DrawStrategyImpl::CalcAheadFunc
DrawDirectionalStrategy::GetCalcAheadFunc(ParticleManager* pm) {
    const EmitterDrawSetting& ed = *pm->mResource->GetEmitterDrawSetting();

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

    case EmitterDrawSetting::AHEAD_CMN_NODESIGN:
    case EmitterDrawSetting::AHEAD_CMN_NODESIGN2: {
        return CalcAhead_NoDesign;
    }

    case EmitterDrawSetting::AHEAD_DIR_PARTICLE_BOTH: {
        return CalcAhead_ParticleBoth;
    }

    default: {
        return CalcAhead_Speed;
    }
    }
}

} // namespace ef
} // namespace nw4r
