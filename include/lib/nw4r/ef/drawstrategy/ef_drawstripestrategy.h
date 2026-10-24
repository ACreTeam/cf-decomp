#ifndef NW4R_EF_DRAW_STRIPE_STRATEGY_H
#define NW4R_EF_DRAW_STRIPE_STRATEGY_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>

namespace nw4r {
namespace ef {

class DrawStripeStrategy : public DrawStrategyImpl {
public:
    DrawStripeStrategy();

    virtual void Draw(const DrawInfo &drawInfo, ParticleManager *pm); // at 0xC

  protected:
    struct Trigonometric {
        f32 mSin; // at 0x0
        f32 mCos; // at 0x4
    };

    struct AheadContextStripe : public AheadContext {
        AheadContextStripe(const math::MTX34 &viewMtx, ParticleManager *pm);

        math::VEC3 mEmitterAxisX;            // at 0xBC
        math::VEC3 mScreenAxisZ;             // at 0xC8
        const Trigonometric *mTrigonometric; // at 0xD4
    };

    typedef void (*CalcAheadFunc)(math::VEC3 *yAxis,
                                  AheadContextStripe *context,
                                  Particle *particle);

    struct VertexTube {
        math::VEC3 mCenter; // at 0x0
        math::VEC3 mX;      // at 0xC
        math::VEC3 mZ;      // at 0x18
        f32 mTexCoord;      // at 0x24
    };

  private:
    void InitGraphics(const DrawInfo &drawInfo, ParticleManager *pm);

    void DrawStripe(AheadContextStripe *aheadContext, int flags,
                    const math::VEC3 &p0, const math::VEC3 &p1);

    inline void DrawParticle(AheadContextStripe *aheadContext, int flags,
                             Particle *pp, CalcAheadFunc calcAhead,
                             const math::VEC3 &pos, const math::VEC3 &p0,
                             const math::VEC3 &p1, f32 pivotX, f32 texCoord);

    void DrawStripeTube(AheadContextStripe *aheadContext, int flags,
                        bool xfflush);
    void DrawStripeTube_ConnectNone(AheadContextStripe *aheadContext,
                                    int flags);
    void DrawStripeTube_ConnectRing(AheadContextStripe *aheadContext,
                                    int flags);
    void DrawStripeTube_ConnectEmitter(AheadContextStripe *aheadContext,
                                       int flags);

    void CalcVertexTube(VertexTube *dst, AheadContextStripe *context, int flags,
                        Particle *pp, CalcAheadFunc calcAhead, f32 px, f32 pz,
                        f32 texCoord);

    void DrawTubeSegment(const VertexTube &param0, const VertexTube &param1,
                         int flags, int numDivide,
                         const Trigonometric *trigonometric);

    inline math::MTX34 CalcParticleMtx(const AheadContextStripe &context,
                                       int flags, Particle *pp,
                                       const math::VEC3 &yAxis,
                                       const math::VEC3 &pos);

    inline math::MTX34 CalcLocalMtx(Particle *pp, f32 pivotX);

    math::VEC3 GetInitialPrevAxis(const EmitterDrawSetting &ed,
                                  const AheadContextStripe &aheadContext);

    EmitterDrawSetting::StripeConnect
    GetStripeConnect(const EmitterDrawSetting &ed) const {
        return static_cast<EmitterDrawSetting::StripeConnect>(
            ed.typeOption2 & EmitterDrawSetting::STRIPE_CONNECT__MASK);
    }

    EmitterDrawSetting::StripeInitialPrevAxis
    GetStripeInitialPrevAxis(const EmitterDrawSetting &ed) const {
        return static_cast<EmitterDrawSetting::StripeInitialPrevAxis>(
            ed.typeOption2 &
            EmitterDrawSetting::STRIPE_INITIAL_PREV_AXIS__MASK);
    }

    EmitterDrawSetting::StripeTexmapType
    GetStripeTexmapType(const EmitterDrawSetting &ed) const {
        return static_cast<EmitterDrawSetting::StripeTexmapType>(
            ed.typeOption2 & EmitterDrawSetting::STRIPE_TEXMAP_TYPE__MASK);
    }

    int GetDrawOrder(const EmitterDrawSetting &ed) const {
        return ed.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;
    }

    int GetTubeDivide(const EmitterDrawSetting &ed) const {
        return ed.typeOption0;
    }

    virtual CalcAheadFunc GetCalcAheadFunc(ParticleManager *pm); // at 0x18

    static void CalcAhead_Particle_Stripe(math::VEC3 *yAxis,
                                          AheadContextStripe *context,
                                          Particle *particle);
    static void CalcAhead_ParticleBoth_Stripe(math::VEC3 *yAxis,
                                              AheadContextStripe *context,
                                              Particle *particle);
    static void CalcAhead_ParticleBoth_Ring(math::VEC3 *yAxis,
                                            AheadContextStripe *context,
                                            Particle *particle);
    static void CalcAhead_ParticleBoth_Origin(math::VEC3 *yAxis,
                                              AheadContextStripe *context,
                                              Particle *particle);
};

} // namespace ef
} // namespace nw4r

#endif
