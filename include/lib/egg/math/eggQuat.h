#pragma once

// EGG quaternion / matrix helpers (lib/egg/math/eggQuat.cpp 804450F0.., eggMatrix.cpp 80444F60..),
// not decompiled. Members use EGG's names; the fn_ helpers are still provisional.

#include <types.h>
#include <lib/egg/math/eggVector.h>

namespace EGG {
struct Quatf : public Vector3f {
    Quatf() {}
    Quatf(f32 fw, Vector3f v) : Vector3f(v), w(fw) {} // v by value (as in ogws): never inlined

    friend Quatf operator*(const Quatf &a, const Quatf &b) {
        return Quatf(a.w * b.w - a.dot(b), a.cross(b) + b * a.w + a * b.w);
    }

    void set(f32 fw, f32 fx, f32 fy, f32 fz);                // 804450F0
    void setRPY(f32 roll, f32 pitch, f32 yaw);               // 8044524C: radians
    void setAxisRotation(const Vector3f &axis, f32 rot);     // 8044527C: radians
    void normalise();                                        // 80445344
    void slerpTo(const Quatf &q2, f32 t, Quatf &out) const;  // 804454EC

    f32 w;
};
} // namespace EGG

extern "C" {
void fn_804456B0(EGG::Quatf *q, const EGG::Vector3f *from, const EGG::Vector3f *to); // 804456B0: rotation from -> to
void fn_80444FE4(nw4r::math::MTX34 *mtx, const EGG::Quatf *q);                 // 80444FE4 (eggMatrix): rotation matrix of q
EGG::Vector3f fn_8044540C(const EGG::Quatf *q, const EGG::Vector3f *v);         // 8044540C: v rotated by q
}
