#pragma once

#include <nw4r/math.h>

// EGG cameras. Only what is used so far: the vtable slot order follows EGG (getViewMatrix first,
// called through the vtable on dCamera_c's camera at 0xCC by d_snd_util fn_8000EDD0); LookAtCamera's
// mPos/mAt/mUp offsets match dCamera_c's ctor (801870E4).

namespace EGG {

class BaseCamera {
public:
    virtual nw4r::math::MTX34 &getViewMatrix() = 0;
    virtual const nw4r::math::MTX34 &getViewMatrixOld() const = 0;
};

class LookAtCamera : public BaseCamera {
public:
    // inline: the kept weak copies are 800107A4 (d_snd_util) and 80445D10 (eggCamera.cpp's tail)
    virtual nw4r::math::MTX34 &getViewMatrix() { return mViewMtx; }
    virtual const nw4r::math::MTX34 &getViewMatrixOld() const { return mViewMtx; }

    /* 0x04 */ nw4r::math::MTX34 mViewMtx;
    /* 0x34 */ nw4r::math::MTX34 mOldViewMtx;
    /* 0x64 */ nw4r::math::VEC3 mPos;
    /* 0x70 */ nw4r::math::VEC3 mAt;
    /* 0x7C */ nw4r::math::VEC3 mUp;
}; // size 0x88

} // namespace EGG
