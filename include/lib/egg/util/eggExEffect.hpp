#pragma once
#include <lib/egg/util/eggEffect.hpp>
#include <nw4r/ef/ef_effect.h>
#include <nw4r/ef/ef_emitter.h>
#include <nw4r/ef/ef_particle.h>
#include <nw4r/ef/ef_particlemanager.h>

// City Folk's EGG customisation of nw4r::ef: the four object types the effect
// memory manager pools (see eggEffectManager.cpp) derive from nw4r's, adding
// per-object overrides that EGG::Effect applies to every emitter / particle
// manager of an effect. Their ctors, dtors and the small virtuals are emitted
// at the start of eggExEffect.cpp's range (8044D34C..).

namespace EGG {

// RTTI: EGG::ExEffect. No members; the extra virtual (0x24) is an empty hook
// the ctor calls.
class ExEffect : public nw4r::ef::Effect {
  public:
    ExEffect();           // 8044D34C
    virtual ~ExEffect();  // 8044D39C
    virtual void reset(); // 8044D3F4
};

// RTTI: EGG::ExEmitter (size 0x158). mFlags bit 0 stops it creating child
// emitters.
class ExEmitter : public nw4r::ef::Emitter {
  public:
    ExEmitter(); // 8044D3F8
    virtual bool Initialize(nw4r::ef::Effect *pParent,
                            nw4r::ef::EmitterResource *pResource,
                            u8 drawWeight); // 8044D4A8
    virtual nw4r::ef::Emitter *
    CreateEmitter(nw4r::ef::EmitterResource *pResource,
                  nw4r::ef::EmitterInheritSetting *pSetting,
                  nw4r::ef::Particle *pParticle, u16 calcRemain,
                  nw4r::math::VEC3 *position = NULL); // 8044D518
    virtual ~ExEmitter();                             // 8044D450
    virtual void disableChildEmitter();               // 8044D530
    virtual void reset();                             // 8044D540

    /* 0x154 */ u32 mFlags;
};
typedef char ExEmitterSizeCheck[sizeof(ExEmitter) == 0x158 ? 1 : -1];

// RTTI: EGG::ExParticle (size 0xE4). Empty hook at 0x18, as in ExEffect.
class ExParticle : public nw4r::ef::Particle {
  public:
    ExParticle();          // 8044D54C
    virtual ~ExParticle(); // 8044D59C
    virtual void reset();  // 8044D5F4
};

// RTTI: EGG::ExParticleManager (size 0xE8). EGG::Effect's setLife / scale /
// rotate / register colour calls are stored here and applied to every particle
// created afterwards.
class ExParticleManager : public nw4r::ef::ParticleManager {
  public:
    enum {
        FLAG_LIFE = 1 << 0,
        FLAG_SCALE = 1 << 1,
        FLAG_ROTATE = 1 << 2,
        FLAG_COLOR = 1 << 3, // << layer
        FLAG_ALPHA = 1 << 5  // << layer
    };

    ExParticleManager(); // 8044D5F8
    virtual bool Initialize(nw4r::ef::Emitter *pParent,
                            nw4r::ef::EmitterResource *pResource); // 8044D6A8
    virtual nw4r::ef::Particle *
    CreateParticle(u16 life, nw4r::math::VEC3 pos, nw4r::math::VEC3 vel,
                   const nw4r::math::MTX34 *pSpace, f32 momentum,
                   const nw4r::ef::EmitterInheritSetting *pSetting,
                   nw4r::ef::Particle *pReferencePtcl,
                   u16 calcRemain);                         // 8044D708
    virtual ~ExParticleManager();                           // 8044D650
    virtual void setLife(u16 life);                         // 8044D8A4
    virtual void setScale(nw4r::math::VEC2 *scale);         // 8044D8B8
    virtual void setRotate(const nw4r::math::VEC3 *rotate); // 8044D8D8
    virtual void setRegisterColor(const _GXColor &color0,
                                  const _GXColor &color1, u8 layer); // 8044D900
    virtual void setRegisterAlpha(u8 alpha0, u8 alpha1, u8 layer);   // 8044D950
    virtual void reset();                                            // 8044D978

    bool checkFlag(u32 flag) const {
        return (mFlags & flag) != 0;
    }

    /* 0xBC */ u32 mFlags;
    /* 0xC0 */ u16 mLife;
    /* 0xC4 */ nw4r::math::_VEC2 mScale;
    /* 0xCC */ nw4r::math::_VEC3 mRotate;
    /* 0xD8 */ _GXColor mColor[2][2];
};

// RTTI: EGG::ExOneEmitter (size 0x28): emits through an Effect when one can be
// created (mFlags bit 0), otherwise runs the effect's first emitter resource
// directly at mPos (bit 1).
class ExOneEmitter {
  public:
    enum { FLAG_EFFECT = 1 << 0, FLAG_EMITTER = 1 << 1 };

    // CF's nw4r::ef::EmitterInheritSetting (10 bytes: the two bytes after
    // `flag` are used by EF).
    struct InheritSetting {
        s16 speed;
        u8 scale;
        u8 alpha;
        u8 color;
        u8 weight;
        u8 type;
        u8 flag;
        u8 _08;
        u8 _09;
    };

    ExOneEmitter(Effect *effect);                                  // 8044D9A4
    virtual ~ExOneEmitter();                                       // 8044D9F4
    virtual void create();                                         // 8044DA34
    virtual void fade();                                           // 8044DA9C
    virtual void setPos(const nw4r::math::VEC3 *pos);              // 8044DAF0
    virtual void setInheritSetting(const InheritSetting *setting); // 8044DB34
    virtual void update();                                         // 8044DB8C
    virtual bool isEffect() const {
        return mFlags & FLAG_EFFECT;
    } // 8044DC8C
    virtual bool isEmitter() const {
        return (mFlags & FLAG_EMITTER) != 0;
    } // 8044DC80
    virtual void reset(); // 8044DC3C

  private:
    /* 0x04 */ Effect *mEffect;
    /* 0x08 */ u32 mFlags;
    /* 0x0C */ f32 mTime;
    /* 0x10 */ nw4r::math::VEC3 mPos;
    /* 0x1C */ InheritSetting mSetting;
}; // 0x28

} // namespace EGG
