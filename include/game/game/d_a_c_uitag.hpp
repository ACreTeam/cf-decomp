#pragma once

// The ui tag actor (DOL TU d_a_c_uitag.cpp, not decompiled). Only what other TUs use.

#include <types.h>

namespace dAcUitag {
// Inferred: the bottom UI panel exists and its animation is in the shown state.
bool isBottomPanelShown(); // 8017FF74
}

extern "C" {
BOOL fn_8017FE28(); // 8017FE28: the pointer is shown (dObjCursor_c::isVisible) and inside the screen
void fn_80180AD0(); // 80180AD0: d_obj_keyboard, when the typed text changes
void fn_80180FC4(int slot); // 80180FC4: d_obj_keyboard, starts the emotion of slot
}
