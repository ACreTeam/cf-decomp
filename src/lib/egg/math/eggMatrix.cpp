#include <lib/egg/math/eggMatrix.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>

namespace EGG {

void Matrix34f::inverseTransposeTo(Matrix34f &to) const {
    PSMTXInvXpose(m, to.m);
}

void Matrix34f::makeIdentity() {
    makeZero();
    m[2][2] = 1.0f;
    m[1][1] = 1.0f;
    m[0][0] = 1.0f;
}

Matrix34f::Matrix34f(f32 xx, f32 xy, f32 xz, f32 xw, f32 yx, f32 yy, f32 yz, f32 yw, f32 zx, f32 zy, f32 zz, f32 zw) {
    m[0][0] = xx;
    m[0][1] = xy;
    m[0][2] = xz;
    m[0][3] = xw;
    m[1][0] = yx;
    m[1][1] = yy;
    m[1][2] = yz;
    m[1][3] = yw;
    m[2][0] = zx;
    m[2][1] = zy;
    m[2][2] = zz;
    m[2][3] = zw;
}

void Matrix34f::makeQ(const Quatf &q) {
    m[0][0] = 1.0f - (2.0f * q.y * q.y) - (2.0f * q.z * q.z);
    m[0][1] = (2.0f * q.x * q.y) - (2.0f * q.w * q.z);
    m[0][2] = (2.0f * q.x * q.z) + (2.0f * q.w * q.y);
    m[1][0] = (2.0f * q.x * q.y) + (2.0f * q.w * q.z);
    m[1][1] = 1.0f - (2.0f * q.x * q.x) - (2.0f * q.z * q.z);
    m[1][2] = (2.0f * q.y * q.z) - (2.0f * q.w * q.x);
    m[2][0] = (2.0f * q.x * q.z) - (2.0f * q.w * q.y);
    m[2][1] = (2.0f * q.y * q.z) + (2.0f * q.w * q.x);
    m[2][2] = 1.0f - (2.0f * q.x * q.x) - (2.0f * q.y * q.y);
    m[2][3] = 0.0f;
    m[1][3] = 0.0f;
    m[0][3] = 0.0f;
}

void Matrix34f::loadPosMtx(u32 posMtxId) {
    GXLoadPosMtxImm(m, posMtxId);
}

Matrix34f Matrix34f::ident(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

} // namespace EGG
