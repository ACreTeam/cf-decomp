#pragma once
#include <nw4r/ef/ef_handle.h>
#include <nw4r/math.h>
#include <revolution/GX/GXTypes.h>

namespace nw4r {
namespace ef {
class Effect;
class Emitter;
} // namespace ef
} // namespace nw4r

// City Folk's EGG::Effect (vtable 80561BE0, ctor 8044A11C, dtor 8044A188): a single effect handle
// wrapping nw4r::ef::Effect. Virtual names follow NSMBW's EGG::Effect (same order), minus
// setInitVelocityRandom and setPowerYAxis, which this EGG revision does not have; the argument
// lists differ from NSMBW's too (pointers instead of references, no ERecursive on most).
// Size 0x7C.

namespace EGG {

class Effect {
public:
    enum ERecursive {
        RECURSIVE_0 = 0,
        RECURSIVE_3 = 3
    };

    enum {
        FLAG_POS = 1 << 0,
        FLAG_MTX = 1 << 1,
        FLAG_SCALE = 1 << 2
    };

    Effect();          // 8044A11C
    virtual ~Effect(); // 8044A188
    virtual BOOL create();                                   // 0x0C
    virtual void fade();                                     // 0x10
    virtual void followFade();                               // 0x14
    virtual void kill();                                     // 0x18
    virtual void setDisableCalc(bool disable);               // 0x1C
    virtual void setDisableDraw(bool disable);               // 0x20
    virtual void setDisableCalcDraw(bool disable);           // 0x24
    virtual void setLife(u16 life);                          // 0x28
    virtual void setEmitRatio(f32 ratio);                    // 0x2C (d_field_assessment)
    virtual void setEmitInterval(u16 interval);              // 0x30
    virtual void setEmitEmitDiv(u16 div);                    // 0x34
    virtual void setPowerRadiationDir(f32 power);            // 0x38
    virtual void setPowerSpecDir(f32 power);                 // 0x3C
    virtual void setPowerSpecDirAdd(f32 power);              // 0x40
    virtual void setSpecDir(const nw4r::math::VEC3 *dir);    // 0x44 (d_fgobj_manager: the wind direction)
    virtual void setSpecDirAdd(const nw4r::math::VEC3 *dir); // 0x48
    virtual void setVelocity(const nw4r::math::VEC3 *vel);   // 0x4C
    virtual void setColor(u8 r, u8 g, u8 b, u8 a, bool recursive); // 0x50 (d_fish_field)
    virtual void setRegisterColor(const _GXColor &color0, const _GXColor &color1, u8 index); // 0x54
    virtual void setRegisterAlpha(u8 alpha0, u8 alpha1, u8 index); // 0x58 (d_a_npc)
    virtual void setDefaultParticleSize(const nw4r::math::VEC2 *size);                 // 0x5C
    virtual void setParticleScale(const nw4r::math::VEC2 *scale, bool recursive);      // 0x60
    virtual void setDefaultParticleRotate(const nw4r::math::VEC3 *rotate);             // 0x64
    virtual void setParticleRotate(const nw4r::math::VEC3 *rotate, bool recursive);    // 0x68
    virtual void setEmitterSize(const nw4r::math::VEC3 *size, int recursive);          // 0x6C
    virtual void setLocalScale(const nw4r::math::VEC3 *scale);                         // 0x70
    virtual void setDynamicsScale(const nw4r::math::VEC3 *scale, const nw4r::math::VEC2 *ptclScale); // 0x74
    virtual void setScale(f32 scale);                                                  // 0x78
    virtual void setScale(const nw4r::math::VEC3 *scale);                              // 0x7C
    virtual void setPos(const nw4r::math::VEC3 *pos);                                  // 0x80
    virtual void setMtx(const nw4r::math::MTX34 *mtx);                                 // 0x84
    virtual void setPtclAnim(int index, bool play);                                    // 0x88
    virtual void update(); // 0x8C (8044B6C4)
    virtual void reset();  // 0x90

    f32 getBaseEmitRatio();                // 8044B83C
    nw4r::ef::Emitter *getRootEmitter();   // 8044B8DC

    /* 0x04 */ char mName[0x20];
    /* 0x24 */ u32 mGroupID;
    /* 0x28 */ u32 mFlags;
    /* 0x2C */ nw4r::math::VEC3 mScale;
    /* 0x38 */ nw4r::math::VEC3 mPos;
    /* 0x44 */ nw4r::math::MTX34 mMtx;
    /* 0x74 */ nw4r::ef::Handle<nw4r::ef::Effect> mHandle;
}; // size 0x7C

} // namespace EGG
