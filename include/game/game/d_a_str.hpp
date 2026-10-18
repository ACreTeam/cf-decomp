#pragma once

// The structure actor (RELs d_a_str / d_a_str_town, not decompiled; RTTI daSTR_c). Only its bases.

#include <game/game/d_demo_actor.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_msg_rcpt.hpp>

// Registers itself in dSTR::getAcList(). Bases: dDemoActor_c, dSTR::alwaysAc_c
// (at 0xBC), dMsg::Rcpt_c (at 0xC0). Its own virtuals follow the bases' vtables
// in the vtable group (fBase_c vtable +0x114 on, d_a_strNP vtable .data 0x2F8);
// only the one d_insect_fieldNP calls is named.
class daSTR_c : public dDemoActor_c,
                public dSTR::alwaysAc_c,
                public dMsg::Rcpt_c {
  public:
    virtual void vf114(); // unknown
    virtual void vf118(); // unknown
    virtual void vf11C(); // unknown
    virtual void vf120(); // unknown
    virtual void vf124(); // unknown
    virtual void vf128(); // unknown
    virtual void vf12C(); // unknown
    virtual void vf130(); // unknown
    virtual void vf134(); // unknown
    virtual void vf138(); // unknown
    virtual void vf13C(); // unknown
    virtual void vf140(); // unknown
    virtual void vf144(); // unknown
    virtual void vf148(); // unknown
    virtual void vf14C(); // unknown
    virtual void vf150(); // unknown
    virtual void vf154(); // unknown
    virtual void vf158(); // unknown
    virtual void vf15C(); // unknown
    virtual void vf160(); // unknown
    virtual void vf164(); // unknown
    virtual void vf168(); // unknown
    virtual void vf16C(); // unknown
    virtual void vf170(); // unknown
    virtual void vf174(); // unknown
    virtual void vf178(); // unknown
    virtual void vf17C(); // unknown
    virtual void vf180(); // unknown
    virtual void vf184(); // unknown
    virtual void vf188(); // unknown
    virtual void vf18C(); // unknown
    virtual void vf190(); // unknown
    virtual void vf194(); // unknown
    virtual void vf198(); // unknown
    virtual void vf19C(); // unknown
    virtual void vf1A0(); // unknown
    virtual void vf1A4(); // unknown
    virtual void vf1A8(); // unknown
    virtual void vf1AC(); // unknown
    virtual void vf1B0(); // unknown
    virtual void vf1B4(); // unknown
    virtual void vf1B8(); // unknown
    virtual void vf1BC(); // unknown
    virtual void vf1C0(); // unknown
    virtual void vf1C4(); // unknown
    virtual void vf1C8(); // unknown
    // +0x1CC (d_a_strNP fn_99_15EC: vf1C8() && vf1A4() != -1): a window is lit
    // (dInsectField::checkHouseLight, the moth's spawn spot).
    virtual BOOL isWindowLit();
};

// Inferred names for exports of the structure REL.
namespace dAStr {
extern const float cNpcDoorOffset;
mVec3_c getNpcDoorPos(daSTR_c *str, const mVec3_c *pos, int door, float offset);
BOOL requestNpcEnter(daSTR_c *str); // d_a_strNP .text 4774: the door opens for an entering player
BOOL requestNpcExit(daSTR_c *str);  // d_a_strNP .text 4828
// The structures' view area (dSTR::viewArea_c) from dSTR::getHostIO(), scaled.
f32 getViewFront();     // d_a_strNP .text 198: hostIO _14
f32 getViewBack();      // d_a_strNP .text 148: hostIO _0C
f32 getViewHalfWidth(); // d_a_strNP .text 1E8: hostIO _16
} // namespace dAStr

// d_a_strNP .text 48DC: 0x24 (frames to wait for the door) if the structure has an opening
// door (fn_99_464), else 0.
extern "C" int fn_99_48DC(daSTR_c *str);
// d_a_strNP .text 490C: the exit door's wait (a u16 of the structure's door data), 0 without one.
extern "C" int fn_99_490C(daSTR_c *str);
