#pragma once
#include <lib/nw4r/ef.h>
#include <lib/egg/util/eggEffect.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_mtx.hpp>

// City Folk's mEf::effect_c (RTTI string 804DF7E0). Replaces the NSMBW-derived header, whose
// EGG::Effect layout and derived classes (levelEffect_c, levelOneEffect_c) do not match City Folk
// and were unused. The new virtuals follow EGG::Effect's at 0x94..0xAC in dEffect_c's vtable
// (804DF6C8), in NSMBW's order; createEffect(const char *, ulong, const mMtx_c *) is at 0x9C.

namespace mEf {

class effect_c : public EGG::Effect {
public:
    effect_c() {}
    ~effect_c() {}

    virtual void createEffect(const char *name, int);                                          // 0x94
    virtual void createEffect(const char *name, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 0x98
    virtual void createEffect(const char *name, ulong, const mMtx_c *mtx);                     // 0x9C
    virtual void vfA0();
    virtual void vfA4();
    virtual bool follow(const mVec3_c *, const mAng3_c *, const mVec3_c *);                    // 0xA8
    virtual bool follow(const mMtx_c *mtx);                                                    // 0xAC
};

} // namespace mEf
