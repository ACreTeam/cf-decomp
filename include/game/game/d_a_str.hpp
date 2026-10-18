#pragma once

// The structure actor (RELs d_a_str / d_a_str_town, not decompiled; RTTI daSTR_c). Only its bases.

#include <game/game/d_demo_actor.hpp>
#include <game/game/d_str.hpp>

// Also derives from dMsg::Rcpt_c (at 0xC0); registers itself in dSTR::getAcList().
class daSTR_c : public dDemoActor_c, public dSTR::alwaysAc_c {};

// Inferred names for exports of the structure REL.
namespace dAStr {
extern const float cNpcDoorOffset;
mVec3_c getNpcDoorPos(daSTR_c *str, const mVec3_c *pos, int door, float offset);
BOOL requestNpcEnter(daSTR_c *str); // d_a_strNP .text 4774: the door opens for an entering player
BOOL requestNpcExit(daSTR_c *str);  // d_a_strNP .text 4828
} // namespace dAStr

// d_a_strNP .text 48DC: 0x24 (frames to wait for the door) if the structure has an opening
// door (fn_99_464), else 0.
extern "C" int fn_99_48DC(daSTR_c *str);
// d_a_strNP .text 490C: the exit door's wait (a u16 of the structure's door data), 0 without one.
extern "C" int fn_99_490C(daSTR_c *str);
