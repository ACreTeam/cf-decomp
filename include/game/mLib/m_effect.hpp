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

// mEf::levelEffect_c (RTTI string 804DF7B8, vtable 805242D0): an effect_c kept in a global list
// (mPrev / mNext). Its dtor is inline (cleanup(), then the bases'). Only what d_fish_field uses
// is declared.
class levelEffect_c : public effect_c {
public:
    levelEffect_c() : mPrev(NULL), mNext(NULL), mActive(false), _85(false), _88(0), _8C(0) {}
    ~levelEffect_c() { cleanup(); }

    virtual void vf10(); // 802B898C

    void cleanup(); // 802B88C4

    /* 0x7C */ levelEffect_c *mPrev;
    /* 0x80 */ levelEffect_c *mNext;
    /* 0x84 */ bool mActive;
    /* 0x85 */ bool _85;
    /* 0x88 */ u32 _88;
    /* 0x8C */ u32 _8C;
}; // size 0x90

} // namespace mEf
