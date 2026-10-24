#ifndef NW4R_EF_DRAW_BILLBOARD_STRATEGY_H
#define NW4R_EF_DRAW_BILLBOARD_STRATEGY_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>

namespace nw4r {
namespace ef {

class DrawBillboardStrategy : public DrawStrategyImpl {
public:
    DrawBillboardStrategy();

    virtual void Draw(const DrawInfo& rInfo,
                      ParticleManager* pManager); // at 0xC

    virtual CalcAheadFunc
    GetCalcAheadFunc(ParticleManager* pManager); // at 0x18

    void InitGraphics(const DrawInfo &drawInfo, ParticleManager *pm);

  private:
    void DrawNormalBillboard(const DrawInfo &drawInfo, ParticleManager *pm);
    void DrawYBillboard(const DrawInfo &drawInfo, ParticleManager *pm);
    void DrawDirectionalBillboard(const DrawInfo &drawInfo,
                                  ParticleManager *pm);

    inline void DispParticle_Normal(Particle *particle, const math::MTX34 *vm,
                                    f32 vx, f32 vy, f32 rc, f32 rs, f32 sx,
                                    f32 sy, const math::VEC2 *pivot, int flags);

    inline void DispParticle_YBillboard(Particle *particle,
                                        const math::MTX34 &vm, f32 vx, f32 vy,
                                        f32 rc, f32 rs, f32 sx, f32 sy,
                                        const math::VEC2 &pivot, int flags);

    inline void DispParticle_Directional(Particle *particle,
                                         const math::MTX34 &vm, f32 vx, f32 vy,
                                         f32 rc, f32 rs, f32 sx, f32 sy, f32 vs,
                                         const math::VEC2 &pivot, int flags);

    inline void DispPolygon(const math::VEC3 &p, const math::VEC3 &d0,
                            const math::VEC3 &d1, int flags);
};

} // namespace ef
} // namespace nw4r

#endif
