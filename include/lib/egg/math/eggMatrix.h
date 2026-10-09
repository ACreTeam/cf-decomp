#pragma once

#include <types.h>

namespace EGG {
    /// @brief A 3x4 matrix (lib/egg/math/eggMatrix.cpp, not decompiled yet).
    class Matrix34f {
    public:
        Matrix34f(f32 _00, f32 _01, f32 _02, f32 _03, f32 _10, f32 _11, f32 _12, f32 _13, f32 _20, f32 _21, f32 _22,
            f32 _23); // 80444FA0

        f32 m[3][4];
    };
}
