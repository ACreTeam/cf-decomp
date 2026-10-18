#pragma once

#include <lib/egg/math/eggQuat.h>

namespace mQuat {
// 802B41E4: rotation between unit vectors; fails when they point in opposite directions.
BOOL makeRotate(EGG::Quatf &out, const EGG::Vector3f &from, const EGG::Vector3f &to);
}

// The game's quaternion wrapper. Default construction leaves the components uninitialized;
// identity rotations are constructed explicitly. EGG products convert through a copy.
class mQuat_c : public EGG::Quatf {
public:
    mQuat_c() {}
    mQuat_c(f32 fw, f32 fx, f32 fy, f32 fz)
        : EGG::Quatf(fw, EGG::Vector3f(fx, fy, fz)) {}
    mQuat_c(const EGG::Quatf &q) : EGG::Quatf(q) {}
    mQuat_c(const EGG::Vector3f &axis, f32 angle)
        : EGG::Quatf(1.0f, EGG::Vector3f(0.0f, 0.0f, 0.0f)) {
        setAxisRotation(axis, angle);
    }
};
