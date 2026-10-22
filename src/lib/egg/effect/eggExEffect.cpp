#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>

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

ExEffect::ExEffect() {
    reset();
}

ExEffect::~ExEffect() {}

void ExEffect::reset() {}

ExEmitter::ExEmitter() : mFlags(0) {
    reset();
}

bool ExEmitter::Initialize(nw4r::ef::Effect *pParent, nw4r::ef::EmitterResource *pResource,
                           u8 drawWeight) {
    reset();
    return nw4r::ef::Emitter::Initialize(pParent, pResource, drawWeight);
}

nw4r::ef::Emitter *ExEmitter::CreateEmitter(nw4r::ef::EmitterResource *pResource,
                                            nw4r::ef::EmitterInheritSetting *pSetting,
                                            nw4r::ef::Particle *pParticle, u16 calcRemain) {
    if ((mFlags & 1) == 0) {
        return nw4r::ef::Emitter::CreateEmitter(pResource, pSetting, pParticle, calcRemain);
    }
    return 0;
}

ExEmitter::~ExEmitter() {}

void ExEmitter::disableChildEmitter() {
    mFlags |= 1;
}

void ExEmitter::reset() {
    mFlags = 0;
}

ExParticle::ExParticle() {
    reset();
}

ExParticle::~ExParticle() {}

void ExParticle::reset() {}

ExParticleManager::ExParticleManager() : mFlags(0) {
    reset();
}

bool ExParticleManager::Initialize(nw4r::ef::Emitter *pParent, nw4r::ef::EmitterResource *pResource) {
    reset();
    return nw4r::ef::ParticleManager::Initialize(pParent, pResource);
}

nw4r::ef::Particle *ExParticleManager::CreateParticle(u16 life, nw4r::math::VEC3 pos,
                                                      nw4r::math::VEC3 vel,
                                                      const nw4r::math::MTX34 *pSpace, f32 momentum,
                                                      const nw4r::ef::EmitterInheritSetting *pSetting,
                                                      nw4r::ef::Particle *pReferencePtcl,
                                                      u16 calcRemain) {
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

ExParticleManager::~ExParticleManager() {}

void ExParticleManager::setLife(u16 life) {
    mFlags |= FLAG_LIFE;
    mLife = life;
}

void ExParticleManager::setScale(const nw4r::math::VEC2 *scale) {
    mFlags |= FLAG_SCALE;
    set2(mScale, scale->x, scale->y);
}

void ExParticleManager::setRotate(const nw4r::math::VEC3 *rotate) {
    mFlags |= FLAG_ROTATE;
    set3(mRotate, rotate->x, rotate->y, rotate->z);
}

void ExParticleManager::setRegisterColor(const _GXColor &color0, const _GXColor &color1, u8 layer) {
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
    mScale.x = 0.0f;
    mScale.y = 0.0f;
    mRotate.x = 1.0f;
    mRotate.y = 1.0f;
    mRotate.z = 1.0f;
}

ExOneEmitter::ExOneEmitter(Effect *effect) : mFlags(0) {
    mEffect = effect;
    reset();
}

ExOneEmitter::~ExOneEmitter() {}

void ExOneEmitter::create() {
    if (mEffect->create()) {
        mFlags |= FLAG_EFFECT;
        mFlags &= ~FLAG_EMITTER;
    } else {
        mFlags &= ~FLAG_EFFECT;
        mFlags |= FLAG_EMITTER;
    }
}

void ExOneEmitter::fade() {
    if (mFlags & FLAG_EFFECT) {
        mEffect->fade();
    }
    mFlags &= ~(FLAG_EFFECT | FLAG_EMITTER);
}

void ExOneEmitter::setPos(const nw4r::math::VEC3 *pos) {
    if (mFlags & FLAG_EFFECT) {
        mEffect->setPos(pos);
    } else if (mFlags & FLAG_EMITTER) {
        mPos = *pos;
    }
}

void ExOneEmitter::setInheritSetting(const InheritSetting *setting) {
    if (mFlags & FLAG_EMITTER) {
        mSetting.speed = setting->speed;
        mSetting.scale = setting->scale;
        mSetting.alpha = setting->alpha;
        mSetting.color = setting->color;
        mSetting.weight = setting->weight;
        mSetting.type = setting->type;
        mSetting.flag = setting->flag;
        mSetting._08 = setting->_08;
        mSetting._09 = setting->_09;
    }
}

void ExOneEmitter::update() {
    if (mFlags & FLAG_EFFECT) {
        mEffect->update();
    } else if (mFlags & FLAG_EMITTER) {
        nw4r::ef::Emitter *emitter = mEffect->getRootEmitter();
        if (emitter != NULL && emitter->mResource != NULL) {
            mTime += emitter->mParameter.mEmitRatio;
            if (mTime >= 1.0f) {
                mTime -= 1.0f;
                emitter->CreateEmitterTmp(
                    emitter->mResource,
                    reinterpret_cast<nw4r::ef::EmitterInheritSetting *>(&mSetting), NULL, 0, &mPos);
            }
        }
    }
}

void ExOneEmitter::reset() {
    mFlags = 0;
    mTime = 0.0f;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mSetting.speed = 0;
    mSetting.scale = 0;
    mSetting.alpha = 0;
    mSetting.color = 0;
    mSetting.weight = 0x80;
    mSetting.flag = 0;
    mSetting._08 = 0;
    mSetting._09 = 0;
}

} // namespace EGG
