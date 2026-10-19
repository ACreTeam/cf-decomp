#pragma once

#include <types.h>

namespace dSTR {

// 8016A87C (d_str.cpp): always FALSE in this build. A switch that makes every
// opening-hours check count as closed: the town's windows and the shops stay
// unlit (d_a_str_town) and the entrance barricades stay up
// (d_a_rmobj_barricade). Declared apart from d_str.hpp, whose bank classes
// change the weak function order of the units that only need this.
BOOL isForceClosed();

} // namespace dSTR
