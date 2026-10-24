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

DrawStripeStrategy::DrawStripeStrategy() {
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

inline math::MTX34 DrawStripeStrategy::CalcParticleMtx(
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

inline math::MTX34 DrawStripeStrategy::CalcLocalMtx(Particle *pp, f32 pivotX) {
    math::VEC3 particleRotate;
    pp->Draw_GetRotate(&particleRotate);

    f32 sx = pp->Draw_GetSizeX();

    f32 cs;
    f32 sn;
    PSSinCosRad(&sn, &cs, particleRotate.y);

    sn *= sx;
    cs *= sx;

    // clang-format off
    return math::MTX34(
          cs, 0.0f,  -sn, pivotX - cs * pivotX,
        0.0f, 1.0f, 0.0f, 0.0f,
          sn, 0.0f,   cs, -sn * pivotX);
    // clang-format on
}

inline void DrawStripeStrategy::DrawParticle(
    AheadContextStripe *aheadContext, int flags, Particle *pp,
    CalcAheadFunc calcAhead, const math::VEC3 &pos, const math::VEC3 &p0,
    const math::VEC3 &p1, f32 pivotX, f32 texCoord) {

    math::VEC3 yAxis;
    math::MTX34 mtx;

    if (pp == NULL) {
        pp = GetYoungestDrawParticle(aheadContext->mCommon.mParticleManager);
    }

    calcAhead(&yAxis, aheadContext, pp);

    math::MTX34 mtxParticle =
        CalcParticleMtx(*aheadContext, flags, pp, yAxis, pos);
    math::MTX34 mtxLocal = CalcLocalMtx(pp, pivotX);
    math::MTX34Mult(&mtx, &mtxParticle, &mtxLocal);

    math::VEC3 q0;
    math::VEC3 q1;
    math::VEC3Transform(&q0, &mtx, &p0);
    math::VEC3Transform(&q1, &mtx, &p1);

    DrawPolygon(q0, q1, texCoord, flags);
}

void DrawStripeStrategy::DrawStripe(AheadContextStripe *aheadContext, int flags,
                                    const math::VEC3 &p0,
                                    const math::VEC3 &p1) {
    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    int num = GetNumDrawParticle(pm);
    f32 pivotX = 0.01f * ed.pivotX;

    bool isRing = (ed.typeOption2 & EmitterDrawSetting::STRIPE_CONNECT__MASK) ==
                  EmitterDrawSetting::STRIPE_CONNECT_RING;
    bool isOrigin =
        (ed.typeOption2 & EmitterDrawSetting::STRIPE_CONNECT__MASK) ==
        EmitterDrawSetting::STRIPE_CONNECT_EMITTER;

    if (isRing || isOrigin) {
        num += 1;
    }

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>(num * 2));
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

        int particleNum;
        int particleNumStep;

        switch (ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER) {
        case 0: {
            particleNum = 0;
            particleNumStep = 1;
            break;
        }

        case 1:
        default: {
            particleNum = num - 1;
            particleNumStep = -1;
            break;
        }
        }

        if (isOrigin && drawOrder == 0) {
            DrawParticle(aheadContext, flags, NULL, calcAhead,
                         aheadContext->mCommon.mEmitterCenter, p0, p1, pivotX,
                         texCoordScale * particleNum);
            particleNum += particleNumStep;
        }

        for (Particle *pp = getFirst(pm); pp != NULL;
             pp = getNext(pm, pp), particleNum += particleNumStep) {
            DrawParticle(aheadContext, flags, pp, calcAhead,
                         pp->mParameter.mPosition, p0, p1, pivotX,
                         texCoordScale * particleNum);
        }

        if (isRing) {
            Particle *first = getFirst(pm);
            DrawParticle(aheadContext, flags, first, calcAhead,
                         first->mParameter.mPosition, p0, p1, pivotX,
                         texCoordScale * particleNum);
        }

        if (isOrigin && drawOrder != 0) {
            DrawParticle(aheadContext, flags, NULL, calcAhead,
                         aheadContext->mCommon.mEmitterCenter, p0, p1, pivotX,
                         texCoordScale * particleNum);
        }
    }
    GXEnd();
}

void DrawStripeStrategy::CalcVertexTube(VertexTube *dst,
                                        AheadContextStripe *context, int flags,
                                        Particle *pp, CalcAheadFunc calcAhead,
                                        f32 px, f32 pz, f32 texCoord) {
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

void DrawStripeStrategy::DrawTubeSegment(const VertexTube &param0,
                                         const VertexTube &param1, int flags,
                                         int numDivide,
                                         const Trigonometric *trigonometric) {
    f32 scale = math::FInv(numDivide);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>(numDivide * 2 + 2));

    if (reverseOrder(flags)) {
        for (int i = 0; i <= numDivide; ++i) {
            f32 sn = trigonometric[i].mSin;
            f32 cs = trigonometric[i].mCos;

            GXPosition(param0.mX * cs + param0.mZ * sn + param0.mCenter);
            if (texEnable(flags)) {
                GXTexCoord2f32(scale * i, param0.mTexCoord);
            }

            GXPosition(param1.mX * cs + param1.mZ * sn + param1.mCenter);
            if (texEnable(flags)) {
                GXTexCoord2f32(scale * i, param1.mTexCoord);
            }
        }
    } else {
        for (int i = 0; i <= numDivide; ++i) {
            f32 sn = trigonometric[i].mSin;
            f32 cs = trigonometric[i].mCos;

            GXPosition(param1.mX * cs + param1.mZ * sn + param1.mCenter);
            if (texEnable(flags)) {
                GXTexCoord2f32(scale * i, param1.mTexCoord);
            }

            GXPosition(param0.mX * cs + param0.mZ * sn + param0.mCenter);
            if (texEnable(flags)) {
                GXTexCoord2f32(scale * i, param0.mTexCoord);
            }
        }
    }

    GXEnd();
}

void DrawStripeStrategy::DrawStripeTube_ConnectNone(
    AheadContextStripe *aheadContext, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    int num = GetNumDrawParticle(pm);
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

    VertexTube point[2];

    Particle *pp = getFirst(pm);
    CalcVertexTube(point + 1, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    count += step;

    for (pp = getNext(pm, pp); pp != NULL;
         pp = getNext(pm, pp), count += step) {
        point[0] = point[1];
        CalcVertexTube(point + 1, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawTubeSegment(point[0], point[1], flags, GetTubeDivide(ed),
                        aheadContext->mTrigonometric);
    }
}

void DrawStripeStrategy::DrawStripeTube_ConnectRing(
    AheadContextStripe *aheadContext, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    int num = GetNumDrawParticle(pm);
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

    VertexTube startPoint;
    VertexTube point[2];

    Particle *pp = getFirst(pm);
    CalcVertexTube(&startPoint, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    point[1] = startPoint;
    count += step;

    for (pp = getNext(pm, pp); pp != NULL;
         pp = getNext(pm, pp), count += step) {
        point[0] = point[1];
        CalcVertexTube(point + 1, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawTubeSegment(point[0], point[1], flags, GetTubeDivide(ed),
                        aheadContext->mTrigonometric);
    }

    startPoint.mTexCoord = texCoordScale * count;
    DrawTubeSegment(point[1], startPoint, flags, GetTubeDivide(ed),
                    aheadContext->mTrigonometric);
}

void DrawStripeStrategy::DrawStripeTube_ConnectEmitter(
    AheadContextStripe *aheadContext, int flags) {

    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    CalcAheadFunc calcAhead = GetCalcAheadFunc(pm);

    int drawOrder = GetDrawOrder(ed);
    GetFirstDrawParticleFunc getFirst = GetGetFirstDrawParticleFunc(drawOrder);
    GetNextDrawParticleFunc getNext = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * ed.pivotX;
    f32 pivotZ = 0.01f * ed.pivotY;

    int num = GetNumDrawParticle(pm);
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

    VertexTube point[2];

    Particle *pp = getFirst(pm);
    CalcVertexTube(point + 0, aheadContext, flags, pp, calcAhead, pivotX,
                   pivotZ, texCoordScale * count);
    count += step;

    point[1] = point[0];
    point[1].mTexCoord = texCoordScale * count;

    {
        Particle *youngest = GetYoungestDrawParticle(pm);
        math::VEC3 toEmitterCenter = aheadContext->mCommon.mEmitterCenter -
                                     youngest->mParameter.mPosition;
        point[0].mCenter += toEmitterCenter;
    }

    count += step;
    DrawTubeSegment(point[0], point[1], flags, GetTubeDivide(ed),
                    aheadContext->mTrigonometric);

    for (pp = getNext(pm, pp); pp != NULL;
         pp = getNext(pm, pp), count += step) {
        point[0] = point[1];
        CalcVertexTube(point + 1, aheadContext, flags, pp, calcAhead, pivotX,
                       pivotZ, texCoordScale * count);
        DrawTubeSegment(point[0], point[1], flags, GetTubeDivide(ed),
                        aheadContext->mTrigonometric);
    }
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

void DrawStripeStrategy::DrawStripeTube(AheadContextStripe *aheadContext,
                                        int flags, bool xfflush) {
    ParticleManager *pm = aheadContext->mCommon.mParticleManager;
    const EmitterDrawSetting &ed = *pm->mResource->GetEmitterDrawSetting();

    int numDivide = GetTubeDivide(ed);
    if (numDivide < 3) {
        return;
    }

    Trigonometric *trigonometric = static_cast<Trigonometric *>(
        __alloca(sizeof(Trigonometric) * (numDivide + 1)));
    if (trigonometric == NULL) {
        return;
    }

    const f32 arc = 360.0f / numDivide;

    trigonometric[0].mCos = 1.0f;
    trigonometric[0].mSin = 0.0f;

    for (int i = 1; i < numDivide; ++i) {
        math::SinCosDeg(&trigonometric[i].mSin, &trigonometric[i].mCos,
                        arc * i);
    }

    trigonometric[numDivide].mCos = 1.0f;
    trigonometric[numDivide].mSin = 0.0f;

    aheadContext->mTrigonometric = trigonometric;

    switch (GetStripeConnect(ed)) {
    default:
    case EmitterDrawSetting::STRIPE_CONNECT_NONE: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawStripeTube_ConnectNone(aheadContext, flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawStripeTube_ConnectNone(aheadContext, flags);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_RING: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawStripeTube_ConnectRing(aheadContext, flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawStripeTube_ConnectRing(aheadContext, flags);
        break;
    }

    case EmitterDrawSetting::STRIPE_CONNECT_EMITTER: {
        setCullModeSafe(GX_CULL_FRONT, xfflush, flags);
        DrawStripeTube_ConnectEmitter(aheadContext, flags);
        setCullModeSafe(GX_CULL_BACK, xfflush, flags);
        DrawStripeTube_ConnectEmitter(aheadContext, flags);
        break;
    }
    }
}

void DrawStripeStrategy::Draw(const DrawInfo &drawInfo, ParticleManager *pm) {
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

    int connectType = ed.typeOption2 & EmitterDrawSetting::STRIPE_CONNECT__MASK;

    if ((connectType == EmitterDrawSetting::STRIPE_CONNECT_RING && num < 3) ||
        (connectType == EmitterDrawSetting::STRIPE_CONNECT_NONE && num < 2)) {

        int drawOrder = ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;
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

    Particle *pp = GetGetFirstDrawParticleFunc(
        ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER)(pm);
    SetupGP(pp, ed, drawInfo, true, false);

    switch (ed.typeOption) {
    default: {
        DrawStripe(&aheadContext, flags, math::VEC3(1.0f, 0.0f, 0.0f),
                   math::VEC3(-1.0f, 0.0f, 0.0f));

        if (ed.typeOption == EmitterDrawSetting::ASSIST_ST_CROSS) {
            DrawStripe(&aheadContext, flags, math::VEC3(0.0f, 0.0f, -1.0f),
                       math::VEC3(0.0f, 0.0f, 1.0f));
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

        DrawStripeTube(&aheadContext, flags, xfflush);
        break;
    }
    }
}

void DrawStripeStrategy::InitGraphics(const DrawInfo &drawInfo,
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

math::VEC3
DrawStripeStrategy::GetInitialPrevAxis(const EmitterDrawSetting &ed,
                                       const AheadContextStripe &aheadContext) {
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

DrawStripeStrategy::CalcAheadFunc
DrawStripeStrategy::GetCalcAheadFunc(ParticleManager *pm) {
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

DrawStripeStrategy::AheadContextStripe::AheadContextStripe(
    const math::MTX34 &viewMtx, ParticleManager *pm)
    : AheadContext(viewMtx, pm), mTrigonometric(NULL) {

    math::VEC3 xAxis(mCommon.mEmitterMtx._00, mCommon.mEmitterMtx._10,
                     mCommon.mEmitterMtx._20);
    math::VEC3TransformNormal(&mEmitterAxisX, &mCommon.mParticleManagerMtxInv,
                              &xAxis);
}

void DrawStripeStrategy::CalcAhead_Particle_Stripe(math::VEC3 *yAxis,
                                                   AheadContextStripe *context,
                                                   Particle *particle) {
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Stripe(
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Ring(
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Origin(
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
