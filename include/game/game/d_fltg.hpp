#pragma once

// The flying targets (balloons, the UFO): up to FLTG_MAX actors registered while they fly, for the slingshot /
// d_uki and for each other. Proposed TU d_fltg.cpp (.text 800A7368..800A75B0, .bss 8058B6D8..8058B6F0; today
// inside the d_font split). Namespace and class name from the RTTI "dFltg::actor_c" (base of daBalloon_c and
// daUfo_c in d_a_balloonNP / d_a_ufoNP); the function names are inferred.

#include <types.h>
#include <game/game/d_actor.hpp>
#include <game/game/d_snd_obj.hpp>

// 112.0f (.sdata2 80750728, owner not split yet): the flying height above the ground (d_a_balloon, d_a_ufo).
extern "C" const f32 lbl_80750728;

namespace dFltg {

#define FLTG_MAX 5

class actor_c : public dActor_c {
public:
    virtual int getKind() = 0;               // vtable 0x4C: 0 balloon, 1 UFO
    virtual int getRaccoIdx() { return -1; } // vtable 0x50: the balloon/UFO route (dFdBase_c::getOtherRaccoIdx)
    // actor_c's vtable and these inlines are weak copies in d_a_balloonNP / d_a_ufoNP.

    /* 0x0A0 */ dSndObjSimple_c mSnd;
};

void clear();                          // 800A7368 (from the str manager, d_str_managerNP)
BOOL add(actor_c *actor);              // 800A738C: TRUE if registered (or already); FALSE when full
BOOL remove(actor_c *actor);           // 800A7448: FALSE if not registered
u32 getCount();                        // 800A74A0
actor_c *getNth(u32 n);                // 800A74FC: the nth registered actor, or NULL
int getOtherRaccoIdx(actor_c *self);   // 800A7554: getRaccoIdx() of the first other actor, or -1

} // namespace dFltg
