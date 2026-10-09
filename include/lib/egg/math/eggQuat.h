#pragma once

// EGG quaternion / matrix helpers (lib/egg/math/eggQuat.cpp 804450F0.., eggMatrix.cpp 80444F60..),
// not decompiled. Only what d_a_npc (dAcNpc_c::earCtrl_c::ear_c::calc) uses; names provisional.

#include <types.h>
#include <lib/egg/math/eggVector.h>

namespace EGG {
struct Quatf {
    f32 x, y, z, w;
};
} // namespace EGG

extern "C" {
void fn_804456B0(EGG::Quatf *q, const EGG::Vector3f *from, const EGG::Vector3f *to); // 804456B0: rotation from -> to
void fn_80445344(EGG::Quatf *q);                                                 // 80445344: normalise
void fn_80444FE4(nw4r::math::MTX34 *mtx, const EGG::Quatf *q);                 // 80444FE4 (eggMatrix): rotation matrix of q
}
