#pragma once

#include <nw4r/math.h>

namespace nw4r { namespace g3d { class Camera; } }

// EGG cameras. Only what is used so far: the vtable slot order follows EGG (getViewMatrix first,
// called through the vtable on dCamera_c's camera at 0xCC by d_snd_util fn_8000EDD0); LookAtCamera's
// mPos/mAt/mUp offsets match dCamera_c's ctor (801870E4).

namespace EGG {

class BaseCamera {
public:
    virtual nw4r::math::MTX34 &getViewMatrix() = 0;
    virtual const nw4r::math::MTX34 &getViewMatrixOld() const = 0;
    virtual void update(); // 804459FC: calls updateMatrix
    virtual void updateMatrix() = 0;

    void CopyToG3D(nw4r::g3d::Camera camera) const; // 80445A5C
};

class LookAtCamera : public BaseCamera {
public:
    LookAtCamera() : mPos(0.0f, 10.0f, 0.0f), mAt(0.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f) {}
    // inline: the kept weak copies are 800107A4 (d_snd_util) and 80445D10 (eggCamera.cpp's tail)
    virtual nw4r::math::MTX34 &getViewMatrix() { return mViewMtx; }
    virtual const nw4r::math::MTX34 &getViewMatrixOld() const { return mViewMtx; }
    virtual void updateMatrix(); // 80445AA8

    /* 0x04 */ nw4r::math::MTX34 mViewMtx;
    /* 0x34 */ nw4r::math::MTX34 mOldViewMtx;
    /* 0x64 */ nw4r::math::VEC3 mPos;
    /* 0x70 */ nw4r::math::VEC3 mAt;
    /* 0x7C */ nw4r::math::VEC3 mUp;
}; // size 0x88

} // namespace EGG
