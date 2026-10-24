#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>
#include <string.h>

namespace EGG {

ExOneEmitter::ExOneEmitter(Effect *effect) : mFlags(0) {
    mEffect = effect;
    reset();
}

ExOneEmitter::~ExOneEmitter() {
}

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
                    reinterpret_cast<nw4r::ef::EmitterInheritSetting *>(
                        &mSetting),
                    NULL, 0, &mPos);
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

extern "C" inline size_t strlen(const char *str) {
    size_t length = -1;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(str) - 1;
    do {
        ++length;
    } while (*++p);
    return length;
}

// The original string caller was stripped. This witness retains its inline
// copies in order; the caller's historical name and body are not recovered.
#pragma push
#pragma dont_inline on
namespace EGG {
size_t strippedStringUseWitness(const ExOneEmitter &emitter, const char *name) {
    if (emitter.ExOneEmitter::isEmitter() || emitter.ExOneEmitter::isEffect()) {
        return strlen(name);
    }
    return 0;
}
} // namespace EGG
#pragma pop
