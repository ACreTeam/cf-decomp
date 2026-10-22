#pragma once
#include <game/mLib/m_effect.hpp>

// Game effect (RTTI dEffect_c, vtable 804DF6C8; bases EGG::Effect, mEf::effect_c). Its code is in
// src/dol/game/d_effect.cpp. The ctor and dtor are
// inline (users call EGG::Effect's directly and store the dEffect_c vtable).

class dEffect_c : public mEf::effect_c {
public:
    dEffect_c() {}
    // No user-declared dtor: the implicit one is emitted at its first use (d_a_npc keeps it at 8001AF00,
    // right after manpu_c::createOneShot); an inline {} dtor would go to the weak tail instead.

    BOOL createEffect(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 80087820
    BOOL createEffect(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *, void (*)(nw4r::ef::Particle *, u32), u32); // 800878DC
    bool createEffect(const char *name, const mMtx_c *mtx); // 80087AA0: createEffect(name, 0, mtx)

    virtual bool createEffect(const char *name, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 80087064
    virtual bool createEffect(const char *name, ulong, const mMtx_c *mtx);                     // 8008757C
    virtual bool follow(const mVec3_c *, const mAng3_c *, const mVec3_c *);                    // 80087158
    virtual bool follow(const mMtx_c *mtx);                                                    // 8008724C
};

// Game level effect (RTTI dLevelEffect_c, vtable 804DF5E8; bases EGG::Effect, mEf::effect_c,
// mEf::levelEffect_c). Its ctor and dtor are inline (users call EGG::Effect's ctor and store the
// vtable directly).
class dLevelEffect_c : public mEf::levelEffect_c {
public:
    dLevelEffect_c() {}
    ~dLevelEffect_c() {}

    virtual void fade(); // 8008771C
    virtual bool follow(const mVec3_c *, const mAng3_c *, const mVec3_c *); // 800872A0
    virtual bool createEffect(const char *, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 80087394
    virtual bool follow(const mMtx_c *); // 800875F0
    virtual bool createEffect(const char *, ulong, const mMtx_c *); // 80087644
    BOOL create(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *, int); // 800879C4
    bool create(const char *, const mMtx_c *, int); // 80087B40

    // Non-virtual wrappers (used by d_a_npc). create*: cleanup() first if the handle is still valid.
    BOOL create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale); // 80087934
    BOOL followEffect(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale);              // 800879B4
    bool create(const char *name, const mMtx_c *mtx);                                              // 80087AB8
    bool followEffect(const mMtx_c *mtx);                                                          // 80087B30

    /* 0x90 */ u8 _90; // set by create; read by vf10
}; // size 0x94

// A single-emitter game effect (RTTI dOneEmitterEffect_c).
class dOneEmitterEffect_c : public mEf::oneEmitter_c {
public:
    dOneEmitterEffect_c() {}
    virtual ~dOneEmitterEffect_c() {}
    BOOL emit(const mVec3_c *, const mAng3_c *, const mVec3_c *); // 800879EC
    BOOL emitFootprint(const mVec3_c *pos, const mAng3_c *angle, int alpha) {
        BOOL result = create(NULL, pos, angle, NULL);
        if (result) setRegisterAlpha(alpha, alpha, 1);
        return result;
    }
    virtual BOOL create(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *);
    virtual BOOL create(const char *, const mMtx_c *);
    virtual void create(const char *);
};

// Free-function names inferred from behaviour. RTTI keeps the effect classes global.
namespace dEffect {
BOOL create(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 80087790
BOOL create(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *, void (*)(nw4r::ef::Particle *, u32), u32); // 80087844
bool create(const char *, const mMtx_c *); // 80087A30
BOOL createFootprint(const mVec3_c *, const mAng3_c *, int, int); // 80087B5C
BOOL initialize(); // 80087DCC
void start(); // 80087EC0
void shutdown(); // 80087F3C
void calc(); // 80087F8C
void draw(int, int); // 8008816C
u32 getResourceHeapSize(); // 800887DC
u32 getManagerHeapSize(); // 800887E8
u32 getCallbackHeapSize(); // 800887F4
void setTexturePalette(const char *, u16 *); // 800887FC
BOOL isActive(); // 8008897C
}
