#include <lib/egg/math/eggQuat.h>

namespace EGG {

void Quatf::set(f32 fw, f32 fx, f32 fy, f32 fz) {
    w = fw;
    x = fx;
    y = fy;
    z = fz;
}

void Quatf::setRPY(const Vector3f &rpy) {
    const f32 cy = Math<f32>::cos(rpy.z * 0.5f);
    const f32 cp = Math<f32>::cos(rpy.y * 0.5f);
    const f32 cr = Math<f32>::cos(rpy.x * 0.5f);
    const f32 sy = Math<f32>::sin(rpy.z * 0.5f);
    const f32 sp = Math<f32>::sin(rpy.y * 0.5f);
    const f32 sr = Math<f32>::sin(rpy.x * 0.5f);

    const f32 cy_cp = cy * cp;
    const f32 sy_sp = sy * sp;
    const f32 cy_sp = cy * sp;
    const f32 sy_cp = sy * cp;

    w = (cy_cp * cr) + (sy_sp * sr);
    x = (cy_cp * sr) - (sy_sp * cr);
    y = (cy_sp * cr) + (sy_cp * sr);
    z = (sy_cp * cr) - (cy_sp * sr);
}

void Quatf::setRPY(f32 roll, f32 pitch, f32 yaw) {
    setRPY(Vector3f(roll, pitch, yaw));
}

void Quatf::setAxisRotation(const Vector3f &axis, f32 rot) {
#ifdef MUST_MATCH
    // Layout only: 2.0f belongs to a stripped function between setRPY and normalise;
    // this keeps its place in the .sdata2 literal pool (after 0.5f, before 0.0f).
    2.0f;
#endif
    const f32 half_angle = rot * 0.5f;
    const f32 cos = Math<f32>::cos(half_angle);
    const f32 sin = Math<f32>::sin(half_angle);
    set(cos, sin * axis.x, sin * axis.y, sin * axis.z);
}

f32 Quatf::norm() {
    return w * w + dot(*this);
}

void Quatf::normalise() {
    f32 mag = Math<f32>::sqrt(norm());
    if (mag > 0.0f) {
        f32 inv = 1.0f / mag;
        w *= inv;
        x *= inv;
        y *= inv;
        z *= inv;
    }
}

Quatf Quatf::conjugate() const {
    Quatf q;
    q.w = w;
    (Vector3f &)q = -1.0f * (const Vector3f &)*this;
    return q;
}

Vector3f Quatf::rotateVector(const Vector3f &vec) {
    Quatf conj, mult;
    conj = conjugate();
    mult = *this * vec;
    mult = mult * conj;
    return (Vector3f &)mult;
}

void Quatf::slerpTo(const Quatf &q2, f32 t, Quatf &out) const {
    f32 dot = x * q2.x + y * q2.y + z * q2.z + w * q2.w;

    if (dot > 1.0f) {
        dot = 1.0f;
    } else if (dot < -1.0f) {
        dot = -1.0f;
    }

    bool lt0;
    if (dot < 0.0) {
        dot = -dot;
        lt0 = true;
    } else {
        lt0 = false;
    }

    const f32 theta_0 = Math<f32>::acos(dot);
    const f32 sin_theta_0 = Math<f32>::sin(theta_0);

    f32 a, b;
    if ((sin_theta_0 > 0.0f ? sin_theta_0 : -sin_theta_0) < 1e-5f) {
        a = 1.0f - t;
        b = t;
    } else {
        const f32 sin_theta_0_inv = 1.0f / sin_theta_0;
        const f32 theta = t * theta_0;
        a = sin_theta_0_inv * Math<f32>::sin(theta_0 - theta);
        b = sin_theta_0_inv * Math<f32>::sin(theta);
    }

    if (lt0) {
        b = -b;
    }

    out.x = a * x + b * q2.x;
    out.y = a * y + b * q2.y;
    out.z = a * z + b * q2.z;
    out.w = a * w + b * q2.w;
}

void Quatf::makeVectorRotation(Vector3f &from, Vector3f &to) {
    Vector3f cross;
    cross.setCross(from, to);
    f32 t0 = (from.dot(to) + 1) * 2.0f;

    if (t0 < 0.0f) {
        t0 = 0.0f;
    }
    const f32 s = Math<f32>::sqrt(t0);

    if (s <= 1.192092896e-07f) {
        set(1.0f, 0.0f, 0.0f, 0.0f);
    } else {
        const f32 inv = 1.0f / s;
        set(s * 0.5f, cross.x * inv, cross.y * inv, cross.z * inv);
    }
}

Quatf operator*(const Quatf &q, const Vector3f &v) {
    Vector3f crossed;
    crossed.setCross(q, v);
    Vector3f scaled = v * q.w;
    return Quatf(-q.dot(v), crossed + scaled);
}

} // namespace EGG
