#include <game/mLib/m_quat.hpp>
#include <float.h>

// 802B41E4
BOOL mQuat::makeRotate(EGG::Quatf &out, const EGG::Vector3f &from, const EGG::Vector3f &to) {
    EGG::Vector3f cross;
    cross.setCross(from, to);
    f32 squared = 2.0f * (1.0f + from.dot(to));
    if (squared < 0.0f) {
        squared = 0.0f;
    }
    f32 length = nw4r::math::FSqrt(squared);
    if (length <= FLT_EPSILON) {
        return FALSE;
    }
    f32 reciprocal = 1.0f / length;
    out.set(0.5f * length, cross.x * reciprocal, cross.y * reciprocal, cross.z * reciprocal);
    return TRUE;
}
