#pragma once

// EGG::Color: a colour type over nw4r::ut::Color. The name and members come from Big Brain Academy's EGG link
// maps (__ct__Q23EGG5ColorFv, __ct__Q23EGG5ColorFiiii, __dt__Q23EGG5ColorFv, __as__, color()). Its inline
// destructor is a separate weak copy from nw4r::ut::Color's (8000AC6C) and mColor's (8000ACAC); CF keeps
// d_black_wait's copy at 80080D3C (bound by d_black_wait, d_reset and eggDrawHelper).

#include <lib/nw4r/ut/ut_Color.h>

namespace EGG {

struct Color : public nw4r::ut::Color {
    Color() {}
    Color(int red, int green, int blue, int alpha) : nw4r::ut::Color(red, green, blue, alpha) {}
    ~Color() {}
};

} // namespace EGG
