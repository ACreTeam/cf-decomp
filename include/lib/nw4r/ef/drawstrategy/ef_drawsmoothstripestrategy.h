#ifndef NW4R_EF_DRAW_STRATEGY_DRAW_SMOOTH_STRIPE_STRATEGY_H
#define NW4R_EF_DRAW_STRATEGY_DRAW_SMOOTH_STRIPE_STRATEGY_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>

#include <nw4r/math.h>

namespace nw4r {
namespace ef {

// Forward declarations
class Particle;

class DrawSmoothStripeStrategy : public DrawStrategyImpl {
  protected:
    struct Vertex {
        Vertex() {
        }

        Vertex(const math::VEC3 &v0, const math::VEC3 &v1, f32 texCoord)
            : mVertex0(v0), mVertex1(v1), mTexCoord(texCoord) {
        }

        Vertex(const Vertex &q0, const Vertex &q1, const Vertex &q2,
               const math::VEC3 &coeff) {
            mVertex0 = q0.mVertex0 * coeff.x + q1.mVertex0 * coeff.y +
                       q2.mVertex0 * coeff.z;
            mVertex1 = q0.mVertex1 * coeff.x + q1.mVertex1 * coeff.y +
                       q2.mVertex1 * coeff.z;
            mTexCoord = q0.mTexCoord * coeff.x + q1.mTexCoord * coeff.y +
                        q2.mTexCoord * coeff.z;
        }

        math::VEC3 mVertex0; // at 0x0
        math::VEC3 mVertex1; // at 0xC
        f32 mTexCoord;       // at 0x18
    };

    struct VertexTube {
        VertexTube() {
        }

        VertexTube(const math::VEC3 &center, const math::VEC3 &x,
                   const math::VEC3 &z, f32 texCoord)
            : mCenter(center), mX(x), mZ(z), mTexCoord(texCoord) {
        }

        VertexTube(const VertexTube &q0, const VertexTube &q1,
                   const VertexTube &q2, const math::VEC3 &coeff) {
            mCenter = q0.mCenter * coeff.x + q1.mCenter * coeff.y +
                      q2.mCenter * coeff.z;
            mX = q0.mX * coeff.x + q1.mX * coeff.y + q2.mX * coeff.z;
            mZ = q0.mZ * coeff.x + q1.mZ * coeff.y + q2.mZ * coeff.z;
            mTexCoord = q0.mTexCoord * coeff.x + q1.mTexCoord * coeff.y +
                        q2.mTexCoord * coeff.z;
        }

        math::VEC3 mCenter; // at 0x0
        math::VEC3 mX;      // at 0xC
        math::VEC3 mZ;      // at 0x18
        f32 mTexCoord;      // at 0x24
    };

    struct Trigonometric {
        f32 mCos; // at 0x0
        f32 mSin; // at 0x4
    };

  public:
    DrawSmoothStripeStrategy();

    virtual void Draw(const DrawInfo &drawInfo, ParticleManager *pm); // at 0xC

  protected:
    struct AheadContextStripe : public AheadContext {
        AheadContextStripe(const math::MTX34 &viewMtx, ParticleManager *pm);

        math::VEC3 mEmitterAxisX;      // at 0xBC
        math::VEC3 mScreenAxisZ;       // at 0xC8
        Trigonometric* mTrigonometric; // at 0xD4
    };

    typedef void (*CalcAheadFunc)(math::VEC3 *yAxis,
                                  AheadContextStripe *context,
                                  Particle *particle);

  private:
    void InitGraphics(const DrawInfo &drawInfo, ParticleManager *pm);

    void DrawSmoothStripe(AheadContextStripe *aheadContext, int divSection,
                          int flags, const math::VEC3 &v0,
                          const math::VEC3 &v1);
    void DrawSmoothStripe_BSpline_ConnectNone(AheadContextStripe *aheadContext,
                                              int divSection, int flags,
                                              const math::VEC3 &v0,
                                              const math::VEC3 &v1);
    void DrawSmoothStripe_BSpline_ConnectRing(AheadContextStripe *aheadContext,
                                              int divSection, int flags,
                                              const math::VEC3 &v0,
                                              const math::VEC3 &v1);
    void DrawSmoothStripe_BSpline_ConnectEmitter(
        AheadContextStripe *aheadContext, int divSection, int flags,
        const math::VEC3 &v0, const math::VEC3 &v1);

    void DrawSmoothStripeTube(AheadContextStripe *aheadContext, int divSection,
                              int flags, bool xfflush);
    void
    DrawSmoothStripeTube_BSpline_ConnectNone(AheadContextStripe *aheadContext,
                                             int divSection, int flags);
    void
    DrawSmoothStripeTube_BSpline_ConnectRing(AheadContextStripe *aheadContext,
                                             int divSection, int flags);
    void DrawSmoothStripeTube_BSpline_ConnectEmitter(
        AheadContextStripe *aheadContext, int divSection, int flags);

    math::VEC3 GetCoeff_BSpline(f32 t);

    void DrawSection_BSpline(int divSection, const Vertex &q0, const Vertex &q1,
                             const Vertex &q2, int flags);
    void DrawSectionEnd_BSpline(const Vertex &q0, const Vertex &q1,
                                const Vertex &q2, int flags);
    void DrawSectionTube_BSpline(int divSection, int divTube,
                                 const VertexTube &q0, const VertexTube &q1,
                                 const VertexTube &q2, int flags,
                                 const Trigonometric *trigonometric);

    math::MTX34 CalcParticleMtx(const AheadContextStripe &context, int flags,
                                Particle *pp, const math::VEC3 &yAxis,
                                const math::VEC3 &pos);

    math::MTX34 CalcLocalMtx(Particle *pp, f32 pivotX);

    void CalcVertex(Vertex *dst, AheadContextStripe *context, int flags,
                    Particle *pp, CalcAheadFunc calcAhead,
                    const math::VEC3 &pos, const math::VEC3 &p0,
                    const math::VEC3 &p1, f32 pivotX, f32 texCoord);

    void CalcVertexTube(VertexTube *dst, AheadContextStripe *context, int flags,
                        Particle *pp, CalcAheadFunc calcAhead, f32 px, f32 pz,
                        f32 texCoord);

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

    int GetDivSection(const EmitterDrawSetting &ed) const {
        return ed.typeOption1;
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
