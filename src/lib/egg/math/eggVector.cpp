#include <lib/egg/math/eggVector.h>

namespace EGG {

const Vector2f Vector2f::zero(0.0f, 0.0f);
const Vector2f Vector2f::ex(1.0f, 0.0f);
const Vector2f Vector2f::ey(0.0f, 1.0f);

const Vector3f Vector3f::zero(0.0f, 0.0f, 0.0f);
const Vector3f Vector3f::ex(1.0f, 0.0f, 0.0f);
const Vector3f Vector3f::ey(0.0f, 1.0f, 0.0f);
const Vector3f Vector3f::ez(0.0f, 0.0f, 1.0f);

f32 Vector3f::normalise() {
    f32 len = length();
    if (len > 0.0f) {
        x *= 1.0f / len;
        y *= 1.0f / len;
        z *= 1.0f / len;
    }
    return len;
}

} // namespace EGG
