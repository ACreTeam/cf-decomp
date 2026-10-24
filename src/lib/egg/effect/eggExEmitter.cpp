#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>

namespace EGG {

ExEmitter::ExEmitter() : mFlags(0) {
    reset();
}

ExEmitter::~ExEmitter() {
}

bool ExEmitter::Initialize(nw4r::ef::Effect *pParent,
                           nw4r::ef::EmitterResource *pResource,
                           u8 drawWeight) {
    reset();
    return nw4r::ef::Emitter::Initialize(pParent, pResource, drawWeight);
}

nw4r::ef::Emitter *
ExEmitter::CreateEmitter(nw4r::ef::EmitterResource *pResource,
                         nw4r::ef::EmitterInheritSetting *pSetting,
                         nw4r::ef::Particle *pParticle, u16 calcRemain,
                         nw4r::math::VEC3 *position) {
    if ((mFlags & 1) == 0) {
        return nw4r::ef::Emitter::CreateEmitter(pResource, pSetting, pParticle,
                                                calcRemain, position);
    }
    return 0;
}


void ExEmitter::disableChildEmitter() {
    mFlags |= 1;
}

void ExEmitter::reset() {
    mFlags = 0;
}

} // namespace EGG
