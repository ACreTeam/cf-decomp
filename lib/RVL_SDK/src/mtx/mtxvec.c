#include <revolution/mtx.h>

// clang-format off
asm void PSMTXMultVec(const register Mtx m, const register Vec* src, register Vec* dst)
{
    nofralloc
    psq_l f0, Vec.x(src), 0, 0
    psq_l f2, 0(m), 0, 0
    psq_l f1, Vec.z(src), 1, 0
    ps_mul f4, f2, f0
    psq_l f3, 8(m), 0, 0
    ps_madd f5, f3, f1, f4
    psq_l f8, 16(m), 0, 0
    ps_sum0 f6, f5, f6, f5
    psq_l f9, 24(m), 0, 0
    ps_mul f10, f8, f0
    psq_st f6, Vec.x(dst), 1, 0
    ps_madd f11, f9, f1, f10
    psq_l f2, 32(m), 0, 0
    ps_sum0 f12, f11, f12, f11
    psq_l f3, 40(m), 0, 0
    ps_mul f4, f2, f0
    psq_st f12, Vec.y(dst), 1, 0
    ps_madd f5, f3, f1, f4
    ps_sum0 f6, f5, f6, f5
    psq_st f6, Vec.z(dst), 1, 0
    blr
}
// clang-format on

// clang-format off
asm void PSMTXMultVecSR(const register Mtx m, const register Vec* src, register Vec* dst)
{
    nofralloc
    psq_l f0, 0(m), 0, 0
    psq_l f6, 0(src), 0, 0
    psq_l f2, 16(m), 0, 0
    ps_mul f8, f0, f6
    psq_l f4, 32(m), 0, 0
    ps_mul f10, f2, f6
    psq_l f7, 8(src), 1, 0
    ps_mul f12, f4, f6
    psq_l f3, 24(m), 0, 0
    ps_sum0 f8, f8, f8, f8
    psq_l f5, 40(m), 0, 0
    ps_sum0 f10, f10, f10, f10
    psq_l f1, 8(m), 0, 0
    ps_sum0 f12, f12, f12, f12
    ps_madd f9, f1, f7, f8
    psq_st f9, 0(dst), 1, 0
    ps_madd f11, f3, f7, f10
    psq_st f11, 4(dst), 1, 0
    ps_madd f13, f5, f7, f12
    psq_st f13, 8(dst), 1, 0
    blr
}
// clang-format on
