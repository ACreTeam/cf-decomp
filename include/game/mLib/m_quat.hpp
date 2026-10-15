#pragma once

#include <lib/egg/math/eggQuat.h>

namespace mQuat {
// 802B41E4: rotation between unit vectors; fails when they point in opposite directions.
BOOL makeRotate(EGG::Quatf &out, const EGG::Vector3f &from, const EGG::Vector3f &to);
}
