#include <nw4r/ef.h>
#include <nw4r/NW4RAssert.hpp>
#include <nw4r/math.h>

namespace nw4r {
namespace ef {

static inline bool texEnable(int flags) {
    return (flags & 1) != 0;
}

static inline bool billboardEnable(int flags) {
    return (flags & 2) != 0;
}

static inline bool reverseOrder(int flags) {
    return (flags & 4) != 0;
}

static inline void DrawPolygon(const math::VEC3 &p0, const math::VEC3 &p1,
                               f32 texCoord, int flags) {
    GXPosition(p0);
    if (texEnable(flags)) {
        GXTexCoord2f32(1.0f, texCoord);
    }

    GXPosition(p1);
    if (texEnable(flags)) {
        GXTexCoord2f32(0.0f, texCoord);
    }
}

DrawSmoothStripeStrategy::DrawSmoothStripeStrategy() {
}

math::MTX34 DrawSmoothStripeStrategy::CalcParticleMtx(
    const AheadContextStripe &context, int flags, Particle *pp,
    const math::VEC3 &yAxis, const math::VEC3 &pos) {

    math::VEC3 xAxis;
    math::VEC3 zAxis;

    if (!billboardEnable(flags)) {
        math::VEC3Cross(&xAxis, &yAxis, &pp->mPrevAxis);
    } else {
        math::VEC3Cross(&xAxis, &yAxis, &context.mScreenAxisZ);
    }

    if (!Normalize(&xAxis)) {
        xAxis = context.mEmitterAxisX;
    }

    math::VEC3Cross(&zAxis, &xAxis, &yAxis);
    Normalize(&zAxis);

    pp->mPrevAxis = zAxis;

    // clang-format off
    return math::MTX34(
        xAxis.x, yAxis.x, zAxis.x, pos.x,
        xAxis.y, yAxis.y, zAxis.y, pos.y,
        xAxis.z, yAxis.z, zAxis.z, pos.z);
    // clang-format on
}

math::MTX34 DrawSmoothStripeStrategy::CalcLocalMtx(Particle *pp, f32 pivotX) {
    math::VEC3 particleRotate;
    pp->Draw_GetRotate(&particleRotate);

    f32 sx = pp->Draw_GetSizeX();

    f32 cs;
    f32 sn;
    PSSinCosRad(&sn, &cs, particleRotate.y);

    cs *= sx;
    sn *= sx;

    // clang-format off
    return math::MTX34(
          cs, 0.0f,  -sn, pivotX - cs * pivotX,
        0.0f, 1.0f, 0.0f, 0.0f,
          sn, 0.0f,   cs, -sn * pivotX);
    // clang-format on
}

void DrawSmoothStripeStrategy::CalcVertex(
    Vertex *dst, AheadContextStripe *context, int flags, Particle *pp,
    CalcAheadFunc calcAhead, const math::VEC3 &pos, const math::VEC3 &p0,
    const math::VEC3 &p1, f32 pivotX, f32 texCoord) {

    math::VEC3 yAxis;
    math::MTX34 mtx;

    calcAhead(&yAxis, context, pp);

    math::MTX34 mtxParticle = CalcParticleMtx(*context, flags, pp, yAxis, pos);
    math::MTX34 mtxLocal = CalcLocalMtx(pp, pivotX);
    math::MTX34Mult(&mtx, &mtxParticle, &mtxLocal);

    math::VEC3Transform(&dst->mVertex0, &mtx, &p0);
    math::VEC3Transform(&dst->mVertex1, &mtx, &p1);
    dst->mTexCoord = texCoord;
}

void DrawSmoothStripeStrategy::CalcVertexTube(VertexTube *dst,
                                              AheadContextStripe *context,
                                              int flags, Particle *pp,
                                              CalcAheadFunc calcAhead, f32 px,
                                              f32 pz, f32 texCoord) {
    math::VEC3 yAxis;
    calcAhead(&yAxis, context, pp);

    math::MTX34 mtxParticle =
        CalcParticleMtx(*context, flags, pp, yAxis, pp->mParameter.mPosition);

    math::VEC3 particleRotate;
    pp->Draw_GetRotate(&particleRotate);

    f32 sx = pp->Draw_GetSizeX();
    f32 sz = pp->Draw_GetSizeY();

    f32 sn, cs;
    PSSinCosRad(&sn, &cs, particleRotate.y);

    // clang-format off
    math::MTX34 mtxLocal(
        cs * sx, 0.0f, -sn * sz, px - cs * sx * px + sn * sz * pz,
           0.0f, 1.0f,     0.0f, 0.0f,
        sn * sx, 0.0f,  cs * sz, pz - sn * sx * px - cs * sz * pz);
    // clang-format on

    math::MTX34 mtx;
    math::MTX34Mult(&mtx, &mtxParticle, &mtxLocal);

    dst->mCenter = math::VEC3(mtx._03, mtx._13, mtx._23);
    math::VEC3TransformNormal(&dst->mX, &mtx, &GetXUnitVec());
    math::VEC3TransformNormal(&dst->mZ, &mtx, &GetZUnitVec());
    dst->mTexCoord = texCoord;
}

math::VEC3 DrawSmoothStripeStrategy::GetCoeff_BSpline(f32 t) {
    f32 t2 = t * t;

    return math::VEC3(0.5f * t2 - 1.0f * t + 0.5f, -1.0f * t2 + 1.0f * t + 0.5f,
                      0.5f * t2 + 0.0f * t + 0.0f);
}

void DrawSmoothStripeStrategy::DrawSection_BSpline(int divSection,
                                                   const Vertex &q0,
                                                   const Vertex &q1,
                                                   const Vertex &q2,
                                                   int flags) {
    f32 rDivSection = math::FInv(divSection);

    for (int i = 0; i < divSection; ++i) {
        f32 t = i * rDivSection;
        math::VEC3 coeff = GetCoeff_BSpline(t);
        Vertex p(q0, q1, q2, coeff);
        DrawPolygon(p.mVertex0, p.mVertex1, p.mTexCoord, flags);
    }
}

void DrawSmoothStripeStrategy::DrawSectionEnd_BSpline(const Vertex &q0,
                                                      const Vertex &q1,
                                                      const Vertex &q2,
                                                      int flags) {
    math::VEC3 coeff = GetCoeff_BSpline(1.0f);
    Vertex p(q0, q1, q2, coeff);
    DrawPolygon(p.mVertex0, p.mVertex1, p.mTexCoord, flags);
}

void DrawSmoothStripeStrategy::DrawSectionTube_BSpline(
    int divSection, int divTube, const VertexTube &q0, const VertexTube &q1,
    const VertexTube &q2, int flags, const Trigonometric *trigonometric) {

    f32 rDivSection = math::FInv(divSection);
    f32 rDivTube = math::FInv(divTube);

    math::VEC3 coeff = GetCoeff_BSpline(0.0f);
    VertexTube p0;
    VertexTube p1(q0, q1, q2, coeff);

    for (int i = 1; i <= divSection; ++i) {
        p0 = p1;

        f32 t = i * rDivSection;
        coeff = GetCoeff_BSpline(t);
        p1 = VertexTube(q0, q1, q2, coeff);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0,
                static_cast<u16>(divTube * 2 + 2));

        if (reverseOrder(flags)) {
            for (int i = 0; i <= divTube; ++i) {
                f32 sn = trigonometric[i].mSin;
                f32 cs = trigonometric[i].mCos;

                GXPosition(p0.mX * cs + p0.mZ * sn + p0.mCenter);
                if (texEnable(flags)) {
                    GXTexCoord2f32(rDivTube * i, p0.mTexCoord);
                }

                GXPosition(p1.mX * cs + p1.mZ * sn + p1.mCenter);
                if (texEnable(flags)) {
                    GXTexCoord2f32(rDivTube * i, p1.mTexCoord);
                }
            }
        } else {
            for (int i = 0; i <= divTube; ++i) {
                f32 sn = trigonometric[i].mSin;
                f32 cs = trigonometric[i].mCos;

                GXPosition(p1.mX * cs + p1.mZ * sn + p1.mCenter);
                if (texEnable(flags)) {
                    GXTexCoord2f32(rDivTube * i, p1.mTexCoord);
                }

                GXPosition(p0.mX * cs + p0.mZ * sn + p0.mCenter);
                if (texEnable(flags)) {
                    GXTexCoord2f32(rDivTube * i, p0.mTexCoord);
                }
            }
        }

        GXEnd();
    }
}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectNone(
    AheadContextStripe *aheadContext, int divSection, int flags,
    const math::VEC3 &v0, const math::VEC3 &v1) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    int num = GetNumDrawParticle(pm);
    f32 pivotX = 0.01f * ed.pivotX;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0,
            static_cast<u16>((num * divSection + 1) * 2));
    {
        f32 texCoordScale;

        switch (GetStripeTexmapType(ed)) {
        default:
        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
            texCoordScale = math::FInv(num - 1);
            break;
        }

        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
            texCoordScale = 1.0f;
            break;
        }
        }

        int count = 0;
        int step = 1;

        if (drawOrder != 0) {
            count = num - 1;
            step = -1;
        }

        Vertex point[3];
        int i0 = 0;
        int i1 = 0;
        int i2 = 0;

        Particle *pp = getFirst(pm);
        CalcVertex(point + 0, aheadContext, flags, pp, calcAhead,
                   pp->mParameter.mPosition, v0, v1, pivotX,
                   texCoordScale * count);
        count += step;

        for (pp = getNext(pm, pp); pp != NULL;
             pp = getNext(pm, pp), count += step) {
            i0 = i1;
            i1 = i2;
            i2 = (i2 + 1) % 3;
            CalcVertex(point + i2, aheadContext, flags, pp, calcAhead,
                       pp->mParameter.mPosition, v0, v1, pivotX,
                       texCoordScale * count);
            DrawSection_BSpline(divSection, point[i0], point[i1], point[i2],
                                flags);
        }

        i0 = i1;
        i1 = i2;
        DrawSection_BSpline(divSection, point[i0], point[i1], point[i2], flags);
        DrawSectionEnd_BSpline(point[i0], point[i1], point[i2], flags);
    }
    GXEnd();
}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectRing(
    AheadContextStripe *aheadContext, int divSection, int flags,
    const math::VEC3 &v0, const math::VEC3 &v1) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    int num = GetNumDrawParticle(pm);
    f32 pivotX = 0.01f * ed.pivotX;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0,
            static_cast<u16>((num * divSection + 1) * 2));
    {
        f32 texCoordScale;

        switch (GetStripeTexmapType(ed)) {
        default:
        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
            texCoordScale = 0.5f / num;
            break;
        }

        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
            texCoordScale = 0.5f;
            break;
        }
        }

        int count = -1;
        int step = 2;

        if (drawOrder != 0) {
            count = 2 * num + 1;
            step = -2;
        }

        Vertex point[5];
        math::VEC3 q0;
        math::VEC3 q1;

        Particle *pp = getFirst(pm);
        CalcVertex(point + 3, aheadContext, flags, pp, calcAhead,
                   pp->mParameter.mPosition, v0, v1, pivotX,
                   texCoordScale * count);
        pp = getNext(pm, pp);
        count += step;

        CalcVertex(point + 4, aheadContext, flags, pp, calcAhead,
                   pp->mParameter.mPosition, v0, v1, pivotX,
                   texCoordScale * count);
        pp = getNext(pm, pp);
        count += step;

        int i0;
        int i1 = 3;
        int i2 = 4;

        for (; pp != NULL; pp = getNext(pm, pp), count += step) {
            i0 = i1;
            i1 = i2;
            i2 = (i2 + 1) % 3;
            CalcVertex(point + i2, aheadContext, flags, pp, calcAhead,
                       pp->mParameter.mPosition, v0, v1, pivotX,
                       texCoordScale * count);
            DrawSection_BSpline(divSection, point[i0], point[i1], point[i2],
                                flags);
        }

        i0 = i1;
        i1 = i2;
        i2 = 3;
        point[i2].mTexCoord = texCoordScale * count;
        DrawSection_BSpline(divSection, point[i0], point[i1], point[i2], flags);
        count += step;

        i0 = i1;
        i1 = i2;
        i2 = 4;
        point[i2].mTexCoord = texCoordScale * count;
        DrawSection_BSpline(divSection, point[i0], point[i1], point[i2], flags);
        DrawSectionEnd_BSpline(point[i0], point[i1], point[i2], flags);
    }
    GXEnd();
}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectEmitter(
    AheadContextStripe *aheadContext, int divSection, int flags,
    const math::VEC3 &v0, const math::VEC3 &v1) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * ed.pivotX;

    Particle *youngest =
        GetYoungestDrawParticle(aheadContext->mCommon.mParticleManager);
    math::VEC3 toEmitterCenter =
        aheadContext->mCommon.mEmitterCenter - youngest->mParameter.mPosition;

    int num = GetNumDrawParticle(pm);
    int numSection = num + 1;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0,
            static_cast<u16>((numSection * divSection + 1) * 2));
    {
        f32 texCoordScale;

        switch (GetStripeTexmapType(ed)) {
        default:
        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
            texCoordScale = math::FInv(num);
            break;
        }

        case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
            texCoordScale = 1.0f;
            break;
        }
        }

        int count = 0;
        int step = 1;

        if (drawOrder != 0) {
            count = num;
            step = -1;
        }

        Vertex point[3];
        int i0 = 0;
        int i1 = 0;
        int i2 = 0;
        math::VEC3 q0;
        math::VEC3 q1;

        Particle *pp = getFirst(pm);
        CalcVertex(point + 0, aheadContext, flags, pp, calcAhead,
                   pp->mParameter.mPosition, v0, v1, pivotX,
                   texCoordScale * count);
        count += step;

        if (drawOrder == 0) {
            i2 = 1;
            point[1] = point[0];
            point[1].mTexCoord = texCoordScale * count;
            point[0].mVertex0 += toEmitterCenter;
            point[0].mVertex1 += toEmitterCenter;
            count += step;
            DrawSection_BSpline(divSection, point[i0], point[i1], point[i2],
                                flags);
        }

        for (pp = getNext(pm, pp); pp != NULL;
             pp = getNext(pm, pp), count += step) {
            i0 = i1;
            i1 = i2;
            i2 = (i2 + 1) % 3;
            CalcVertex(point + i2, aheadContext, flags, pp, calcAhead,
                       pp->mParameter.mPosition, v0, v1, pivotX,
                       texCoordScale * count);
            DrawSection_BSpline(divSection, point[i0], point[i1], point[i2],
                                flags);
        }

        if (drawOrder != 0) {
            i0 = i1;
            i1 = i2;
            i2 = (i2 + 1) % 3;
            point[i2] = Vertex(point[i1].mVertex0 + toEmitterCenter,
                               point[i1].mVertex1 + toEmitterCenter,
                               texCoordScale * count);
            DrawSection_BSpline(divSection, point[i0], point[i1], point[i2],
                                flags);
        }

        i0 = i1;
        i1 = i2;
        DrawSection_BSpline(divSection, point[i0], point[i1], point[i2], flags);
        DrawSectionEnd_BSpline(point[i0], point[i1], point[i2], flags);
    }
    GXEnd();
}

void DrawSmoothStripeStrategy::DrawSmoothStripe(
    AheadContextStripe *aheadContext, int divSection, int flags,
    const math::VEC3 &v0, const math::VEC3 &v1) {
    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    switch (GetStripeConnect(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_CONNECT_NONE: {
        DrawSmoothStripe_BSpline_ConnectNone(aheadContext, divSection, flags,
                                             v0, v1);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_RING: {
        DrawSmoothStripe_BSpline_ConnectRing(aheadContext, divSection, flags,
                                             v0, v1);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_EMITTER: {
        DrawSmoothStripe_BSpline_ConnectEmitter(aheadContext, divSection, flags,
                                                v0, v1);
        break;
    }
    }
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectNone(
    AheadContextStripe *aheadContext, int divSection, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    int num = GetNumDrawParticle(pm);
    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    f32 texCoordScale;

    switch (GetStripeTexmapType(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
        texCoordScale = math::FInv(num - 1);
        break;
    }

    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
        texCoordScale = 1.0f;
        break;
    }
    }

    int count = 0;
    int step = 1;

    if (drawOrder != 0) {
        count = num - 1;
        step = -1;
    }

    VertexTube point[3];
    int i0 = 0;
    int i1 = 0;
    int i2 = 0;

    Particle *pp = getFirst(pm);
    CalcVertexTube(point + 0, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    count += step;

    for (pp = getNext(pm, pp); pp != NULL;
         pp = getNext(pm, pp), count += step) {
        i0 = i1;
        i1 = i2;
        i2 = (i2 + 1) % 3;
        CalcVertexTube(point + i2, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0],
                                point[i1], point[i2], flags,
                                aheadContext->mTrigonometric);
    }

    i0 = i1;
    i1 = i2;
    DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0], point[i1],
                            point[i2], flags, aheadContext->mTrigonometric);
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectRing(
    AheadContextStripe *aheadContext, int divSection, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    int num = GetNumDrawParticle(pm);
    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    f32 texCoordScale;

    switch (GetStripeTexmapType(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
        texCoordScale = 0.5f / num;
        break;
    }

    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
        texCoordScale = 0.5f;
        break;
    }
    }

    int count = -1;
    int step = 2;

    if (drawOrder != 0) {
        count = 2 * num + 1;
        step = -2;
    }

    VertexTube point[5];

    Particle *pp = getFirst(pm);
    CalcVertexTube(point + 3, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    pp = getNext(pm, pp);
    count += step;

    CalcVertexTube(point + 4, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    pp = getNext(pm, pp);
    count += step;

    int i0;
    int i1 = 3;
    int i2 = 4;

    for (; pp != NULL; pp = getNext(pm, pp), count += step) {
        i0 = i1;
        i1 = i2;
        i2 = (i2 + 1) % 3;
        CalcVertexTube(point + i2, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0],
                                point[i1], point[i2], flags,
                                aheadContext->mTrigonometric);
    }

    i0 = i1;
    i1 = i2;
    i2 = 3;
    point[i2].mTexCoord = texCoordScale * count;
    DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0], point[i1],
                            point[i2], flags, aheadContext->mTrigonometric);
    count += step;

    i0 = i1;
    i1 = i2;
    i2 = 4;
    point[i2].mTexCoord = texCoordScale * count;
    DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0], point[i1],
                            point[i2], flags, aheadContext->mTrigonometric);
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectEmitter(
    AheadContextStripe *aheadContext, int divSection, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    Particle *youngest =
        GetYoungestDrawParticle(aheadContext->mCommon.mParticleManager);
    math::VEC3 toEmitterCenter =
        aheadContext->mCommon.mEmitterCenter - youngest->mParameter.mPosition;

    int num = GetNumDrawParticle(pm);
    int numSection = num + 1;

    f32 texCoordScale;

    switch (GetStripeTexmapType(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_STRETCH: {
        texCoordScale = math::FInv(num);
        break;
    }

    case EmitterDrawSetting::STRIPE_TEXMAP_TYPE_REPEAT: {
        texCoordScale = 1.0f;
        break;
    }
    }

    int count = 0;
    int step = 1;

    if (drawOrder != 0) {
        count = num;
        step = -1;
    }

    VertexTube point[3];
    int i0 = 0;
    int i1 = 0;
    int i2 = 0;

    Particle *pp = getFirst(pm);
    CalcVertexTube(point + 0, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    count += step;

    if (drawOrder == 0) {
        i2 = 1;
        point[1] = VertexTube(point[0].mCenter, point[0].mX, point[0].mZ,
                              texCoordScale * count);
        point[0].mCenter += toEmitterCenter;
        count += step;
        DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0],
                                point[i1], point[i2], flags,
                                aheadContext->mTrigonometric);
    }

    for (pp = getNext(pm, pp); pp != NULL;
         pp = getNext(pm, pp), count += step) {
        i0 = i1;
        i1 = i2;
        i2 = (i2 + 1) % 3;
        CalcVertexTube(point + i2, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0],
                                point[i1], point[i2], flags,
                                aheadContext->mTrigonometric);
    }

    if (drawOrder != 0) {
        i0 = i1;
        i1 = i2;
        i2 = (i2 + 1) % 3;
        point[i2] =
            VertexTube(point[i1].mCenter + toEmitterCenter, point[i1].mX,
                       point[i1].mZ, texCoordScale * count);
        DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0],
                                point[i1], point[i2], flags,
                                aheadContext->mTrigonometric);
    }

    i0 = i1;
    i1 = i2;
    DrawSectionTube_BSpline(divSection, GetTubeDivide(ed), point[i0], point[i1],
                            point[i2], flags, aheadContext->mTrigonometric);
}

static inline void setCullModeSafe(GXCullMode cullmode, bool xfflush,
                                   int flags) {
    GXSetCullMode(cullmode);

    if (xfflush) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);

        for (int i = 0; i < 8; ++i) {
            GXPosition3f32(0.0f, 0.0f, 0.0f);
            if (texEnable(flags)) {
                GXTexCoord2f32(0.0f, 0.0f);
            }
        }

        GXEnd();
    }
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube(
    AheadContextStripe *aheadContext, int divSection, int flags, bool xfflush) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    int divTube = GetTubeDivide(ed);
    if (divTube < 3) {
        return;
    }

    Trigonometric *trigonometric = static_cast<Trigonometric *>(
        __alloca(sizeof(Trigonometric) * (divTube + 1)));
    if (trigonometric == NULL) {
        return;
    }

    const f32 arc = 360.0f / divTube;

    trigonometric[0].mCos = 1.0f;
    trigonometric[0].mSin = 0.0f;

    for (int i = 1; i < divTube; ++i) {
        math::SinCosDeg(&trigonometric[i].mSin, &trigonometric[i].mCos,
                        arc * i);
    }

    trigonometric[divTube].mCos = 1.0f;
    trigonometric[divTube].mSin = 0.0f;

    aheadContext->mTrigonometric = trigonometric;

    switch (GetStripeConnect(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_CONNECT_NONE: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectNone(aheadContext, divSection,
                                                 flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectNone(aheadContext, divSection,
                                                 flags);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_RING: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectRing(aheadContext, divSection,
                                                 flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectRing(aheadContext, divSection,
                                                 flags);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_EMITTER: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectEmitter(aheadContext, divSection,
                                                    flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawSmoothStripeTube_BSpline_ConnectEmitter(aheadContext, divSection,
                                                    flags);
        break;
    }
    }
}

void DrawSmoothStripeStrategy::Draw(const DrawInfo &drawInfo,
                                    ParticleManager *pm) {
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    int num = GetNumDrawParticle(pm);
    if (num == 0) {
        return;
    }

    math::MTX34 cameraMtx;
    drawInfo.GetViewMtxCooked(&cameraMtx);

    AheadContextStripe aheadContext(cameraMtx, pm);

    {
        math::VEC3 prevAxis = GetInitialPrevAxis(ed, aheadContext);

        for (Particle *pp = GetYoungestParticle(pm); pp != NULL;
             pp = GetElderParticle(pm, pp)) {

            if (pp->mPrevAxis.x <= 1.0f) {
                break;
            }

            pp->mPrevAxis = prevAxis;
        }
    }

    int flags =
        (GetDrawOrder(ed) != 0 ? 4 : 0) |
        (ed.typeOption == EmitterDrawSetting::ASSIST_ST_BILLBOARD ? 2 : 0);
    int drawOrder = GetDrawOrder(ed);

    math::MTX34 mtxVM;
    {
        drawInfo.GetViewMtxCooked(&mtxVM);
        math::MTX34Mult(&mtxVM, &mtxVM,
                        &aheadContext.mCommon.mParticleManagerMtx);

        if (billboardEnable(flags)) {
            math::MTX34 mtxVMInv;

            if (math::MTX34Inv(&mtxVMInv, &mtxVM)) {
                aheadContext.mScreenAxisZ =
                    math::VEC3(mtxVMInv._02, mtxVMInv._12, mtxVMInv._22);
            } else {
                aheadContext.mScreenAxisZ = math::VEC3(0.0f, 0.0f, 1.0f);
            }
        }
    }

    int connectType = GetStripeConnect(ed);

    if ((connectType == EmitterDrawSetting::STRIPE_CONNECT_RING && num < 3) ||
        (connectType == EmitterDrawSetting::STRIPE_CONNECT_NONE && num < 2)) {

        CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);
        GetFirstDrawParticleFunc getFirst =
            GetGetFirstDrawParticleFunc(drawOrder);
        GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

        for (Particle *pp = getFirst(pm); pp != NULL; pp = getNext(pm, pp)) {
            math::VEC3 yAxis;
            calcAhead(&yAxis, &aheadContext, pp);
            CalcParticleMtx(aheadContext, flags, pp, yAxis,
                            pp->mParameter.mPosition);
        }

        return;
    }

    InitGraphics(drawInfo, pm);
    GXLoadPosMtxImm(mtxVM, GX_PNMTX0);

    flags |= mNumTexmap > 0 ? 1 : 0;

    Particle *pp = GetGetFirstDrawParticleFunc(GetDrawOrder(ed))(pm);
    SetupGP(pp, ed, drawInfo, true, false);

    int divSection = GetDivSection(ed);
    if (divSection == 0) {
        divSection = 1;
    }

    static const math::VEC3 v0(1.0f, 0.0f, 0.0f);
    static const math::VEC3 v1(-1.0f, 0.0f, 0.0f);
    static const math::VEC3 v2(0.0f, 0.0f, 1.0f);
    static const math::VEC3 v3(0.0f, 0.0f, -1.0f);

    switch (ed.typeOption) {
    default: {
        DrawSmoothStripe(&aheadContext, divSection, flags, v0, v1);

        if (ed.typeOption == EmitterDrawSetting::ASSIST_ST_CROSS) {
            DrawSmoothStripe(&aheadContext, divSection, flags, v2, v3);
        }
        break;
    }

    case EmitterDrawSetting::ASSIST_ST_TUBE: {
        bool xfflush = false;

        if ((ed.mFlags & EmitterDrawSetting::FLAG_CLIP_DISABLE) ||
            !pp->mParticleManager->mManagerEM->mManagerEF->mManagerES
                 ->mXFFlushSafe) {
            xfflush = true;
        }

        DrawSmoothStripeTube(&aheadContext, divSection, flags, xfflush);
        break;
    }
    }
}

void DrawSmoothStripeStrategy::InitGraphics(const DrawInfo &drawInfo,
                                            ParticleManager *pm) {
    NW4RAssertPointerNonnull(pm);

    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    Initialize(pm, ed, drawInfo);

    GXEnableTexOffsets(GX_TEXCOORD0, GX_TRUE, GX_TRUE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    if (mNumTexmap > 0) {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    }

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GXSetCurrentMtx(GX_PNMTX0);
}

math::VEC3 DrawSmoothStripeStrategy::GetInitialPrevAxis(
    const EmitterDrawSetting &ed, const AheadContextStripe &aheadContext) {
    math::VEC3 prevAxis;

    if (ed.typeDir == EmitterDrawSetting::AHEAD_CMN_NODESIGN2) {
        const math::MTX34 *mInv = &aheadContext.mCommon.mParticleManagerMtxInv;

        switch (GetStripeInitialPrevAxis(ed)) {
        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_X: {
            prevAxis = math::VEC3(mInv->_00, mInv->_10, mInv->_20);
            break;
        }

        default:
        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_Y: {
            prevAxis = math::VEC3(mInv->_01, mInv->_11, mInv->_21);
            break;
        }

        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_Z: {
            prevAxis = math::VEC3(mInv->_02, mInv->_12, mInv->_22);
            break;
        }

        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_XYZ: {
            prevAxis = math::VEC3(mInv->_00, mInv->_10, mInv->_20) +
                       math::VEC3(mInv->_01, mInv->_11, mInv->_21) +
                       math::VEC3(mInv->_02, mInv->_12, mInv->_22);
            break;
        }
        }
    } else {
        switch (GetStripeInitialPrevAxis(ed)) {
        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_X: {
            math::VEC3TransformNormal(
                &prevAxis, &aheadContext.mCommon.mEmitterMtx, &GetXUnitVec());
            math::VEC3TransformNormal(
                &prevAxis, &aheadContext.mCommon.mParticleManagerMtxInv,
                &prevAxis);
            break;
        }

        default:
        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_Y: {
            prevAxis = aheadContext.mCommon.mEmitterAxisY;
            return prevAxis;
        }

        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_AXIS_Z: {
            math::VEC3TransformNormal(
                &prevAxis, &aheadContext.mCommon.mEmitterMtx, &GetZUnitVec());
            math::VEC3TransformNormal(
                &prevAxis, &aheadContext.mCommon.mParticleManagerMtxInv,
                &prevAxis);
            break;
        }

        case EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS_EMITTER_XYZ: {
            math::VEC3 vec(1.0f, 1.0f, 1.0f);
            math::VEC3TransformNormal(&prevAxis,
                                      &aheadContext.mCommon.mEmitterMtx, &vec);
            math::VEC3TransformNormal(
                &prevAxis, &aheadContext.mCommon.mParticleManagerMtxInv,
                &prevAxis);
            break;
        }
        }
    }

    if (!Normalize(&prevAxis)) {
        prevAxis = aheadContext.mCommon.mEmitterAxisY;
    }

    return prevAxis;
}

DrawSmoothStripeStrategy::CalcAheadFunc
DrawSmoothStripeStrategy::GetCalcAheadFunc(ParticleManager *pm) {
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    switch (ed.typeDir) {
    case EmitterDrawSetting::AHEAD_BB_SPEED: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_Speed);
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_CENTER: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_EmitterCenter);
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_DESIGN: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_EmitterDesign);
    }

    case EmitterDrawSetting::AHEAD_BB_PARTICLE: {
        return CalcAhead_Particle_Stripe;
    }

    case EmitterDrawSetting::AHEAD_CMN_NODESIGN:
    case EmitterDrawSetting::AHEAD_CMN_NODESIGN2: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_NoDesign);
    }

    case EmitterDrawSetting::AHEAD_DIR_PARTICLE_BOTH: {
        switch (ed.typeOption2 & EmitterDrawSetting::STRIPE_CONNECT__MASK) {
        case EmitterDrawSetting::STRIPE_CONNECT_RING: {
            return CalcAhead_ParticleBoth_Ring;
        }

        case EmitterDrawSetting::STRIPE_CONNECT_EMITTER: {
            return CalcAhead_ParticleBoth_Origin;
        }

        default: {
            return CalcAhead_ParticleBoth_Stripe;
        }
        }
    }

    default: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_Speed);
    }
    }
}

DrawSmoothStripeStrategy::AheadContextStripe::AheadContextStripe(
    const math::MTX34 &viewMtx, ParticleManager *pm)
    : AheadContext(viewMtx, pm) {

    math::VEC3 xAxis(mCommon.mEmitterMtx._00, mCommon.mEmitterMtx._10,
                     mCommon.mEmitterMtx._20);
    math::VEC3TransformNormal(&mEmitterAxisX, &mCommon.mParticleManagerMtxInv,
                              &xAxis);
}

void DrawSmoothStripeStrategy::CalcAhead_Particle_Stripe(
    math::VEC3 *yAxis, AheadContextStripe *context, Particle *particle) {
    Particle *youngerParticle =
        GetYoungerDrawParticle(context->mCommon.mParticleManager, particle);

    if (youngerParticle != NULL) {
        math::VEC3Sub(yAxis, &particle->mParameter.mPosition,
                      &youngerParticle->mParameter.mPosition);
    } else {
        math::VEC3Sub(yAxis, &particle->mParameter.mPosition,
                      &context->mCommon.mEmitterCenter);
    }

    if (!Normalize(yAxis)) {
        *yAxis = context->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Stripe(
    math::VEC3 *yAxis, AheadContextStripe *context, Particle *particle) {

    Particle *elder =
        GetElderDrawParticle(context->mCommon.mParticleManager, particle);
    Particle *younger =
        GetYoungerDrawParticle(context->mCommon.mParticleManager, particle);

    math::VEC3 elderPos(0.0f, 0.0f, 0.0f);

    if (elder != NULL) {
        math::VEC3Sub(&elderPos, &elder->mParameter.mPosition,
                      &particle->mParameter.mPosition);

        if (!Normalize(&elderPos)) {
            elderPos = math::VEC3(0.0f, 0.0f, 0.0f);
        }
    }

    math::VEC3 youngerPos(0.0f, 0.0f, 0.0f);

    if (younger != NULL) {
        math::VEC3Sub(&youngerPos, &younger->mParameter.mPosition,
                      &particle->mParameter.mPosition);

        if (!Normalize(&youngerPos)) {
            youngerPos = math::VEC3(0.0f, 0.0f, 0.0f);
        }
    }

    math::VEC3Sub(yAxis, &elderPos, &youngerPos);

    if (!Normalize(yAxis)) {
        *yAxis = context->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Ring(
    math::VEC3 *yAxis, AheadContextStripe *context, Particle *particle) {

    Particle *elder =
        GetElderDrawParticle(context->mCommon.mParticleManager, particle);
    if (elder == NULL) {
        elder = GetYoungestDrawParticle(context->mCommon.mParticleManager);
    }

    Particle *younger =
        GetYoungerDrawParticle(context->mCommon.mParticleManager, particle);
    if (younger == NULL) {
        younger = GetOldestDrawParticle(context->mCommon.mParticleManager);
    }

    math::VEC3 elderPos;
    math::VEC3Sub(&elderPos, &elder->mParameter.mPosition,
                  &particle->mParameter.mPosition);

    if (!Normalize(&elderPos)) {
        elderPos = math::VEC3(0.0f, 0.0f, 0.0f);
    }

    math::VEC3 youngerPos;
    math::VEC3Sub(&youngerPos, &younger->mParameter.mPosition,
                  &particle->mParameter.mPosition);

    if (!Normalize(&youngerPos)) {
        youngerPos = math::VEC3(0.0f, 0.0f, 0.0f);
    }

    math::VEC3Sub(yAxis, &elderPos, &youngerPos);

    if (!Normalize(yAxis)) {
        *yAxis = context->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Origin(
    math::VEC3 *yAxis, AheadContextStripe *context, Particle *particle) {

    Particle *elder =
        GetElderDrawParticle(context->mCommon.mParticleManager, particle);
    Particle *younger =
        GetYoungerDrawParticle(context->mCommon.mParticleManager, particle);

    math::VEC3 elderPos(0.0f, 0.0f, 0.0f);

    if (elder != NULL) {
        math::VEC3Sub(&elderPos, &elder->mParameter.mPosition,
                      &particle->mParameter.mPosition);
        Normalize(&elderPos);
    }

    math::VEC3 youngerPos;

    if (younger != NULL) {
        math::VEC3Sub(&youngerPos, &younger->mParameter.mPosition,
                      &particle->mParameter.mPosition);
    } else {
        math::VEC3Sub(&youngerPos, &context->mCommon.mEmitterCenter,
                      &particle->mParameter.mPosition);
    }

    Normalize(&youngerPos);

    math::VEC3Sub(yAxis, &elderPos, &youngerPos);

    if (!Normalize(yAxis)) {
        *yAxis = context->mCommon.mEmitterAxisY;
    }
}

} // namespace ef
} // namespace nw4r
