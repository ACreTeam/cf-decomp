#include <nw4r/ef.h>

#include <cmath>

namespace nw4r {
namespace ef {

void EmitterFormLine::Emission(Emitter *pEmitter, ParticleManager *pManager,
                               int count, u32 flags, f32 *pParams, u16 life,
                               f32 lifeRnd, const math::MTX34 *pSpace) {

    if (count < 1) {
        return;
    }

    for (int i = 0; i < count; i++) {
        f32 fpos;

        if (!(flags & EmitterDesc::EMIT_FLAG_17)) {
            fpos = pEmitter->mRandom.RandFloat();
        } else if (count > 1) {
            fpos = static_cast<f32>(i) / static_cast<f32>(count - 1);
        } else {
            fpos = 0.0f;
        }

        if (flags & EmitterDesc::EMIT_FLAG_26) {
            fpos -= 0.5f;
        }

        fpos *= pParams[0];

        f32 sx, cx, sy, cy, sz, cz;
        PSSinCosRad(&sx, &cx, pParams[1]);
        PSSinCosRad(&sy, &cy, pParams[2]);
        PSSinCosRad(&sz, &cz, pParams[3]);

        math::VEC3 pos;
        pos.x = (cx * cz * sy + sx * sz) * fpos;
        pos.y = (-cz * sx + cx * sy * sz) * fpos;
        pos.z = (cx * cy) * fpos;

        math::VEC3 normal;
        normal.x = 0.0f;
        normal.y = 1.0f;
        normal.z = 0.0f;

        math::VEC3 fromYAxis(pos.x, 0.0f, pos.z);

        math::VEC3 vel;
        CalcVelocity(&vel, pEmitter, pos, normal, pos, fromYAxis);

        // clang-format off
        pManager->CreateParticle(
            CalcLife(life, lifeRnd, pEmitter),
            pos,
            vel,
            pSpace,
            1.0f + pEmitter->mParameter.mVelMomentumRandom *
                (1.0f / 100.0f) * pEmitter->mRandom.RandFloat(),
            &pEmitter->mInheritSetting,
            pEmitter->mpReferenceParticle,
            pEmitter->mCalcRemain);
        // clang-format on
    }
}

} // namespace ef
} // namespace nw4r
