#pragma once
#include <nw4r/ut.h>

/// @brief A 32-bit RGBA color.
/// @ingroup mlib
struct mColor : public nw4r::ut::Color {
public:
    mColor() : nw4r::ut::Color() {} ///< Creates the default color.
    mColor(nw4r::ut::Color color) : nw4r::ut::Color(color) {} ///< Creates a color from from a base nw4r::ut::Color instance.
    mColor(u8 r, u8 g, u8 b, u8 a) { Set(r, g, b, a); } ///< Creates a color from its channels (after the default white).

    /// @brief Linearly interpolates all RGBA channels between two color values.
    /// @param c1 The first color.
    /// @param c2 The second color.
    /// @param t The blend weight, between @p 0.0f and @p 1.0f.
    void lerp(const GXColor &c1, const GXColor &c2, float t);
};
