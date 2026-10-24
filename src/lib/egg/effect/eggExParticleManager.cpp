#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>
#include <string.h>

namespace EGG {

static inline void set2(nw4r::math::_VEC2 &d, f32 x, f32 y) {
    d.x = x;
    d.y = y;
}

static inline void set3(nw4r::math::_VEC3 &d, f32 x, f32 y, f32 z) {
    d.x = x;
    d.y = y;
    d.z = z;
}

ExParticleManager::ExParticleManager() : mFlags(0) {
    reset();
}

ExParticleManager::~ExParticleManager() {
}

bool ExParticleManager::Initialize(nw4r::ef::Emitter *pParent,
                                   nw4r::ef::EmitterResource *pResource) {
    reset();
    return nw4r::ef::ParticleManager::Initialize(pParent, pResource);
}

nw4r::ef::Particle *ExParticleManager::CreateParticle(
    u16 life, nw4r::math::VEC3 pos, nw4r::math::VEC3 vel,
    const nw4r::math::MTX34 *pSpace, f32 momentum,
    const nw4r::ef::EmitterInheritSetting *pSetting,
    nw4r::ef::Particle *pReferencePtcl, u16 calcRemain) {
    nw4r::ef::Particle *particle = nw4r::ef::ParticleManager::CreateParticle(
        life, pos, vel, pSpace, momentum, pSetting, pReferencePtcl, calcRemain);
    if (particle != NULL && mFlags != 0) {
        if (checkFlag(FLAG_LIFE)) {
            particle->mLife = mLife;
        }
        nw4r::ef::ParticleParameter *param = particle->GetParticleParameter();
        if (param != NULL) {
            if (checkFlag(FLAG_SCALE)) {
                param->mSize.x = mScale.x;
                param->mSize.y = mScale.y;
            }
            if (checkFlag(FLAG_ROTATE)) {
                param->mRotate.x = mRotate.x;
                param->mRotate.y = mRotate.y;
                param->mRotate.z = mRotate.z;
            }
            if (checkFlag(FLAG_COLOR << 0)) {
                param->mColor[0][0].r = mColor[0][0].r;
                param->mColor[0][0].g = mColor[0][0].g;
                param->mColor[0][0].b = mColor[0][0].b;
                param->mColor[0][1].r = mColor[0][1].r;
                param->mColor[0][1].g = mColor[0][1].g;
                param->mColor[0][1].b = mColor[0][1].b;
            }
            if (checkFlag(FLAG_COLOR << 1)) {
                param->mColor[1][0].r = mColor[1][0].r;
                param->mColor[1][0].g = mColor[1][0].g;
                param->mColor[1][0].b = mColor[1][0].b;
                param->mColor[1][1].r = mColor[1][1].r;
                param->mColor[1][1].g = mColor[1][1].g;
                param->mColor[1][1].b = mColor[1][1].b;
            }
            if (checkFlag(FLAG_ALPHA << 0)) {
                param->mColor[0][0].a = mColor[0][0].a;
                param->mColor[0][1].a = mColor[0][1].a;
            }
            if (checkFlag(FLAG_ALPHA << 1)) {
                param->mColor[1][0].a = mColor[1][0].a;
                param->mColor[1][1].a = mColor[1][1].a;
            }
        }
    }
    return particle;
}


void ExParticleManager::setLife(u16 life) {
    mFlags |= FLAG_LIFE;
    mLife = life;
}

void ExParticleManager::setScale(nw4r::math::VEC2 *scale) {
    mFlags |= FLAG_SCALE;
    set2(mScale, scale->x, scale->y);
}

void ExParticleManager::setRotate(const nw4r::math::VEC3 *rotate) {
    mFlags |= FLAG_ROTATE;
    set3(mRotate, rotate->x, rotate->y, rotate->z);
}

void ExParticleManager::setRegisterColor(const _GXColor &color0,
                                         const _GXColor &color1, u8 layer) {
    mFlags |= FLAG_COLOR << layer;
    mColor[layer][0].r = color0.r;
    mColor[layer][0].g = color0.g;
    mColor[layer][0].b = color0.b;
    mColor[layer][1].r = color1.r;
    mColor[layer][1].g = color1.g;
    mColor[layer][1].b = color1.b;
}

void ExParticleManager::setRegisterAlpha(u8 alpha0, u8 alpha1, u8 layer) {
    mFlags |= FLAG_ALPHA << layer;
    mColor[layer][0].a = alpha0;
    mColor[layer][1].a = alpha1;
}

void ExParticleManager::reset() {
    mFlags = 0;
    mLife = 0;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mRotate.x = 0.0f;
    mRotate.y = 0.0f;
    mRotate.z = 0.0f;
}

} // namespace EGG
