#pragma once
#include <game/mLib/m_effect.hpp>

// Game effect (RTTI dEffect_c, vtable 804DF6C8; bases EGG::Effect, mEf::effect_c). Its code is in
// the unsplit TU around 80087000; only what d_fireworks uses is declared. The ctor and dtor are
// inline (users call EGG::Effect's directly and store the dEffect_c vtable).

class dEffect_c : public mEf::effect_c {
public:
    dEffect_c() {}
    ~dEffect_c() {}

    void createEffect(const char *name, const mMtx_c *mtx); // 80087AA0: createEffect(name, 0, mtx)

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
};
