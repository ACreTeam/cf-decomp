#pragma once

// The structure actor (RELs d_a_str / d_a_str_town, not decompiled; RTTI daSTR_c). Only its bases.

#include <game/game/d_demo_actor.hpp>
#include <game/game/d_str.hpp>

// Also derives from dMsg::Rcpt_c (at 0xC0); registers itself in dSTR::getAcList().
class daSTR_c : public dDemoActor_c, public dSTR::alwaysAc_c {};
