// dFdUnitFlags_c: one bit per unit of a field block. .text 800811F4..800812C8.
// See include/game/game/d_save_main_field.hpp.
#include <game/game/d_save_main_field.hpp>

// 800811F4
dFdUnitFlags_c::dFdUnitFlags_c() {}

// 800811F8
dFdUnitFlags_c::~dFdUnitFlags_c() {}

// 80081238
BOOL dFdUnitFlags_c::set(u32 unitX, u32 unitZ) {
    if (isValid(unitX, unitZ)) {
        mRows[unitZ] |= 1 << unitX;
        return TRUE;
    }
    return FALSE;
}

// 80081280
BOOL dFdUnitFlags_c::reset(u32 unitX, u32 unitZ) {
    if (isValid(unitX, unitZ)) {
        mRows[unitZ] &= ~(1 << unitX);
        return TRUE;
    }
    return FALSE;
}
