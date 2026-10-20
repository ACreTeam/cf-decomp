#pragma once

// d_s_select (REL d_s_selectNP, profile S_SELECT): the scene dScSelect_c (its
// RTTI name). It is an empty fBase_c: every virtual is fBase_c's inline
// default, so the REL only holds the profile, the implicit destructor and the
// weak copies of those defaults.

#include <types.h>
#include <game/framework/f_base.hpp>

class dScSelect_c : public fBase_c {};
