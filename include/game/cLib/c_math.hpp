#pragma once
#include <types.h>
/// @file

/// @brief C Math library.
/// @ingroup clib
namespace cM {

    // Conversion utilities
    s16 rad2s(float rad); ///< Converts an angle from radians to units.
    s16 atan2s(float sin, float cos); ///< Converts a sine and a cosine to an angle in units.
    float atan2f(float sin, float cons);

    /// @brief v * v. As an inline its operand is evaluated once and the calls in an expression are
    /// evaluated right to left (a + b squares b first), which the bg and world code rely on.
    inline f32 square(f32 v) { return v * v; }

    // RNG utilities
    void initRnd(ulong seed); ///< Initializes ::s_rnd with the given seed.
    float rnd(); ///< Generates a floating point number between 0 and 1.
    int rndInt(int max); ///< Generates an integer between 0 and the given max.
    float rndF(float max); ///< Generates a floating point number between 0 and the given max.

    /// @brief Generates a number between @p min and @p max (@p max excluded for integers).
    /// @details Not inline: every instantiation is emitted as a weak function (80090850 for int,
    /// 800908CC for float; the kept copies are in d_fireworks.cpp).
    template <typename T>
    T rndRange(T min, T max) {
        f32 r = cM::rndF(max - min);
        f32 m = min;
        m += r;
        return m;
    }

} // namespace cM
