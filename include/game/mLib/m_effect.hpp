#pragma once
#include <lib/nw4r/ef.h>
#include <lib/egg/util/eggEffect.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_mtx.hpp>
namespace EGG { class Heap; }

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
    virtual bool createEffect(const char *name, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 0x98
    virtual bool createEffect(const char *name, ulong, const mMtx_c *mtx);                     // 0x9C
    virtual bool createEffect(const char *, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *, void (*)(nw4r::ef::Particle *, u32), u32);
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
    virtual void vf18(); // 802B899C
    virtual void update(); // 802B885C
    virtual void createEffect(const char *, int); // 802B85AC
    virtual bool createEffect(const char *, ulong, const mVec3_c *, const mAng3_c *, const mVec3_c *); // 802B86AC
    virtual bool createEffect(const char *, ulong, const mMtx_c *); // 802B875C
    virtual bool follow(const mVec3_c *, const mAng3_c *, const mVec3_c *); // 802B89AC
    virtual bool follow(const mMtx_c *); // 802B89F8

    void cleanup(); // 802B88C4

    /* 0x7C */ levelEffect_c *mPrev;
    /* 0x80 */ levelEffect_c *mNext;
    /* 0x84 */ bool mActive;
    /* 0x85 */ bool _85;
    /* 0x88 */ u32 _88;
    /* 0x8C */ u32 _8C;
}; // size 0x90

} // namespace mEf

namespace mEf {
class oneEmitter_c : public EGG::Effect {
public:
    oneEmitter_c() {}
    virtual ~oneEmitter_c() {}
    virtual BOOL create(const char *, const mVec3_c *, const mAng3_c *, const mVec3_c *);
    virtual BOOL create(const char *, const mMtx_c *);
    virtual void create(const char *);
};
}

namespace mEf {
BOOL initialize(EGG::Heap *, int, int, int, int, int, int); // 802B7A6C
void shutdown(); // 802B7B0C
void setCallbackHeap(EGG::Heap *); // 802B7BFC
void beginCalc(); // 802B7DE8
void endCalc(); // 802B7E0C
void setCamera(const nw4r::math::VEC3 *, const nw4r::math::MTX34 *, f32, f32, int); // 802B7E10
void draw(nw4r::ef::DrawInfo *, int, int); // 802B7E30
}

#include <lib/egg/util/eggEffectManager.hpp>
#include <lib/egg/core/eggHeap.h>
namespace mEf {
// 80088AD4: the game installs its EffectCreator subclass for each creator slot.
template <class Creator>
BOOL initialize(EGG::Heap *heap, int effects, int emitters, int managers,
                int particles, int creators, int groups) {
    if (initialize(heap, effects, emitters, managers, particles, creators, groups) == FALSE) return FALSE;
    for (int i = 0; i < creators; i++) {
        Creator *creator = new (heap, 4) Creator(i);
        EGG::EffectManager::sEffectManager->setCreator(creator, i);
    }
    return TRUE;
}
}
