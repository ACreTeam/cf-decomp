#pragma once
#include <game/mLib/m_effect.hpp>

// Game effect (RTTI dEffect_c, vtable 804DF6C8; bases EGG::Effect, mEf::effect_c). Its code is in
// the unsplit TU around 80087000; only what d_fireworks uses is declared. The ctor and dtor are
// inline (users call EGG::Effect's directly and store the dEffect_c vtable).

class dEffect_c : public mEf::effect_c {
public:
    dEffect_c() {}
    // No user-declared dtor: the implicit one is emitted at its first use (d_a_npc keeps it at 8001AF00,
    // right after manpu_c::createOneShot); an inline {} dtor would go to the weak tail instead.

    bool createEffect(const char *name, const mMtx_c *mtx); // 80087AA0: createEffect(name, 0, mtx)

    virtual void createEffect(const char *name, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 80087064
    virtual void createEffect(const char *name, ulong, const mMtx_c *mtx);                     // 8008757C
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

    virtual void vf10(); // 8008771C

    // Non-virtual wrappers (used by d_a_npc). create*: cleanup() first if the handle is still valid.
    BOOL create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale); // 80087934
    BOOL followEffect(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale);              // 800879B4
    bool create(const char *name, const mMtx_c *mtx);                                              // 80087AB8
    bool followEffect(const mMtx_c *mtx);                                                          // 80087B30

    /* 0x90 */ u8 _90; // set by fn_80087B40; read by vf10
}; // size 0x94

extern "C" {
// One-shot effect: a temporary dEffect_c created at pos.
BOOL fn_80087790(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale); // 80087790
// _90 = flag; effect->createEffect(name, 0, mtx).
bool fn_80087B40(dLevelEffect_c *effect, const char *name, const mMtx_c *mtx, int flag); // 80087B40
// Footprint effect (side 0 left / 1 right).
void fn_80087B5C(const mVec3_c *pos, const mAng3_c *angle, int arg, int side); // 80087B5C
}
