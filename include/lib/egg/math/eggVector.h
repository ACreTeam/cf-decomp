#pragma once

#include <lib/egg/math/eggMath.h>
#include <nw4r/math.h>
#include <types.h>

namespace EGG {
    /// @brief A two-dimensional floating point vector.
    class Vector2f : public nw4r::math::VEC2 {
    public:
        /// @brief Constructs an empty vector.
        Vector2f() {}

        ~Vector2f() {}

        /// @brief Constructs a vector from two floating point values.
        Vector2f(f32 fx, f32 fy) { set(fx, fy); }

        void set(float x, float y) {
            this->x = x;
            this->y = y;
        }

        Vector2f operator+(const Vector2f &v) const { return Vector2f(x + v.x, y + v.y); }
        Vector2f operator-(const Vector2f &v) const { return Vector2f(x - v.x, y - v.y); }
        Vector2f operator*(f32 f) const { return Vector2f(x * f, y * f); }

        float normalise();

        /// @brief Gets the squared length of the vector.
        float squaredLength() const { return x * x + y * y; }

        /// @brief Gets the length of the vector.
        float length() const { return EGG::Mathf::sqrt(squaredLength()); }
    };

    /// @brief A three-dimensional floating point vector.
    class Vector3f : public nw4r::math::VEC3 {
    public:
        /// @brief Constructs an empty vector.
        Vector3f() {}

        ~Vector3f() {}

        /// @brief Constructs a vector from two floating point values.
        Vector3f(f32 fx, f32 fy, f32 fz) { set(fx, fy, fz); }

        /// @brief Dot product.
        f32 dot(const Vector3f &v) const { return x * v.x + y * v.y + z * v.z; }

        f32 &operator()(int i) {
            return ((f32 *)this)[i];
        }

        Vector3f operator+(const Vector3f &v) const { return Vector3f(x + v.x, y + v.y, z + v.z); }
        Vector3f operator*(f32 f) const { return Vector3f(x * f, y * f, z * f); }
        Vector3f cross(const Vector3f &v) const {
            f32 cx = y * v.z - z * v.y;
            f32 cy = z * v.x - x * v.z;
            f32 cz = x * v.y - y * v.x;
            return Vector3f(cx, cy, cz);
        }

        void setCross(const Vector3f &a, const Vector3f &b); // 800681FC, shared by quaternion helpers.

        void set(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }
    };

}
