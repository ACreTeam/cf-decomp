#pragma once

// The game camera; not decompiled yet. Only what the sky uses.

#include <nw4r/math.h>

extern "C" {
extern nw4r::math::VEC3 lbl_80623FEC; // 80623FEC: the camera's target (the view center)
}

template <typename T>
T *getViewCenter() {
    return &lbl_80623FEC;
}
