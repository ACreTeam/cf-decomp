#include <math.h>
#include <revolution/mtx.h>

// clang-format off

asm void PSVECAdd(const register Vec* a, const register Vec* b, register Vec* ab) {
    psq_l f2, Vec.x(a), 0, 0
    psq_l f4, Vec.x(b), 0, 0
    ps_add f6, f2, f4
    psq_st f6, Vec.x(ab), 0, 0
    psq_l f3, Vec.z(a), 1, 0
    psq_l f5, Vec.z(b), 1, 0
    ps_add f7, f3, f5
    psq_st f7, Vec.z(ab), 1, 0
}

void PSVECNormalize(const register Vec* src, register Vec* unit) {
    register float c_half = 0.5f;
    register float c_three = 3.0f;
    register float v1_xy;
    register float v1_z;
    register float xx_zz;
    register float xx_yy;
    register float sqsum;
    register float rsqrt;
    register float nwork0;
    register float nwork1;

    asm {
        psq_l v1_xy, 0x0(src), 0, 0
        ps_mul xx_yy, v1_xy, v1_xy
        psq_l v1_z, 0x8(src), 1, 0
        ps_madd xx_zz, v1_z, v1_z, xx_yy
        ps_sum0 sqsum, xx_zz, v1_z, xx_yy
        frsqrte rsqrt, sqsum
        fmuls nwork0, rsqrt, rsqrt
        fmuls nwork1, rsqrt, c_half
        fnmsubs nwork0, nwork0, sqsum, c_three
        fmuls rsqrt, nwork0, nwork1
        ps_muls0 v1_xy, v1_xy, rsqrt
        psq_st v1_xy, 0x0(unit), 0, 0
        ps_muls0 v1_z, v1_z, rsqrt
        psq_st v1_z, 0x8(unit), 1, 0
    }
}


f32 PSVECMag(const register Vec* v) {
    register f32 vxy, vzz;
    register f32 sqmag, rmag;
    register f32 nwork0, nwork1;
    register f32 c_three, c_half, c_zero;

    c_half = 0.5f;

    asm {
        psq_l vxy, 0x0(v), 0, 0
        ps_mul vxy, vxy, vxy
        lfs vzz, 0x8(v)
        fsubs c_zero, c_half, c_half
        ps_madd sqmag, vzz, vzz, vxy
        ps_sum0 sqmag, sqmag, vxy, vxy
        fcmpu cr0, sqmag, c_zero
        beq L_000005F0
        frsqrte rmag, sqmag
    }

    c_three = 3.0f;

    asm {
        fmuls nwork0, rmag, rmag
        fmuls nwork1, rmag, c_half
        fnmsubs nwork0, nwork0, sqmag, c_three
        fmuls rmag, nwork0, nwork1
        fmuls sqmag, sqmag, rmag
    L_000005F0:
    }

    return sqmag;
}


asm f32 PSVECDotProduct(const register Vec* a, const register Vec* b) {
    psq_l f2, Vec.y(a), 0, 0
    psq_l f3, Vec.y(b), 0, 0
    ps_mul f2, f2, f3
    psq_l f5, Vec.x(a), 0, 0
    psq_l f4, Vec.x(b), 0, 0
    ps_madd f3, f5, f4, f2
    ps_sum0 f1, f3, f2, f2
}


asm void PSVECCrossProduct(const register Vec* a, const register Vec* b, register Vec* axb) {
    psq_l f1, Vec.x(b), 0, 0
    lfs f2, Vec.z(a)
    psq_l f0, Vec.x(a), 0, 0
    ps_merge10 f6, f1, f1
    lfs f3, Vec.z(b)
    ps_mul f4, f1, f2
    ps_muls0 f7, f1, f0
    ps_msub f5, f0, f3, f4
    ps_msub f8, f0, f6, f7
    ps_merge11 f9, f5, f5
    ps_merge01 f10, f5, f8
    psq_st f9, Vec.x(axb), 1, 0
    ps_neg f10, f10
    psq_st f10, Vec.y(axb), 0, 0
}


void C_VECHalfAngle(const Vec* a, const Vec* b, Vec* half) {
    Vec aTmp;
    Vec bTmp;
    Vec hTmp;


    aTmp.x = -a->x;
    aTmp.y = -a->y;
    aTmp.z = -a->z;
    bTmp.x = -b->x;
    bTmp.y = -b->y;
    bTmp.z = -b->z;

    VECNormalize(&aTmp, &aTmp);
    VECNormalize(&bTmp, &bTmp);
    VECAdd(&aTmp, &bTmp, &hTmp);

    if (VECDotProduct(&hTmp, &hTmp) > 0.0f) {
        VECNormalize(&hTmp, half);
        return;
    }
    *half = hTmp;
}


f32 PSVECSquareDistance(const register Vec* a, const register Vec* b) {
    register f32 v0yz, v1yz, v0xy, v1xy, dyz, dxy;
    register f32 sqdist;

    asm {
        psq_l v0yz, 0x4(a), 0, 0
        psq_l v1yz, 0x4(b), 0, 0
        ps_sub dyz, v0yz, v1yz
        psq_l v0xy, 0x0(a), 0, 0
        psq_l v1xy, 0x0(b), 0, 0
        ps_mul dyz, dyz, dyz
        ps_sub dxy, v0xy, v1xy
        ps_madd sqdist, dxy, dxy, dyz
        ps_sum0 sqdist, sqdist, dyz, dyz
    }

    return sqdist;
}


// clang-format on
