#include <nw4r/math.h>

#define FSEL_MAX(_fx, _fy)                                                     \
    fx = (_fx);                                                                \
    fy = (_fy);                                                                \
    dt = fx - fy;                                                              \
    ASM ( fsel work, dt, fx, fy )

#define FSEL_MIN(_fx, _fy)                                                     \
    fx = (_fx);                                                                \
    fy = (_fy);                                                                \
    dt = fy - fx;                                                              \
    ASM ( fsel work, dt, fx, fy )

namespace nw4r {
namespace math {

VEC3* VEC3Maximize(VEC3* pOut, const VEC3* pA, const VEC3* pB) {
    register f32 fx, fy;
    register f32 dt, work;

    FSEL_MAX(pA->x, pB->x);
    pOut->x = work;

    FSEL_MAX(pA->y, pB->y);
    pOut->y = work;

    FSEL_MAX(pA->z, pB->z);
    pOut->z = work;

    return pOut;
}

VEC3* VEC3Minimize(VEC3* pOut, const VEC3* pA, const VEC3* pB) {
    register f32 fx, fy;
    register f32 dt, work;

    FSEL_MIN(pA->x, pB->x);
    pOut->x = work;

    FSEL_MIN(pA->y, pB->y);
    pOut->y = work;

    FSEL_MIN(pA->z, pB->z);
    pOut->z = work;

    return pOut;
}

MTX33* MTX33Identity(register MTX33* pMtx) {
    register f32 c_00 = 0.0f, c_10 = 1.0f;
    register f32 c_11;

    ASM (
        ps_merge00 c_11, c_10, c_00

        psq_st c_00, MTX33._02(pMtx), 0, 0 // _02=0, _10=0
        psq_st c_00, MTX33._20(pMtx), 0, 0 // _20=0, _21=0
        psq_st c_11, MTX33._00(pMtx), 0, 0 // _00=1, _01=0
        psq_st c_11, MTX33._11(pMtx), 0, 0 // _11=1, _12=0

        stfs c_10,   MTX33._22(pMtx)       // _22=1
    )

    return pMtx;
}

MTX33* MTX34ToMTX33(register MTX33* pOut, register const MTX34* pIn) {
    register f32 row0a, row0b, row1a, row1b, row2a, row2b;

    ASM (
        psq_l row0a, MTX34._00(pIn), 0, 0
        psq_l row0b, MTX34._02(pIn), 0, 0
        psq_l row1a, MTX34._10(pIn), 0, 0
        psq_l row1b, MTX34._12(pIn), 0, 0
        psq_l row2a, MTX34._20(pIn), 0, 0
        psq_l row2b, MTX34._22(pIn), 0, 0

        psq_st row0a, MTX33._00(pOut), 0, 0
        psq_st row0b, MTX33._02(pOut), 1, 0
        psq_st row1a, MTX33._10(pOut), 0, 0
        psq_st row1b, MTX33._12(pOut), 1, 0
        psq_st row2a, MTX33._20(pOut), 0, 0
        psq_st row2b, MTX33._22(pOut), 1, 0
    )

    return pOut;
}

#define nofralloc
#undef PURE_ASM

asm u32 MTX34InvTranspose(register MTX33* pOut, register const MTX34* pIn){
    // clang-format off
    nofralloc

    /**
     * Calculate determinant of 3x3 submatrix
     * [a b c X]
     * [d e f X]
     * [g h i X]
     */

    psq_l      f0, MTX34._00(pIn), 1, 0 // (a, 1.0)
    psq_l      f1, MTX34._01(pIn), 0, 0 // (b, c)
    psq_l      f2, MTX34._10(pIn), 1, 0 // (d, 1.0)
    ps_merge10 f6, f1, f0               // (c, a)
    psq_l      f3, MTX34._11(pIn), 0, 0 // (e, f)
    psq_l      f4, MTX34._20(pIn), 1, 0 // (g, 1.0)
    ps_merge10 f7, f3, f2               // (f, d)
    psq_l      f5, MTX34._21(pIn), 0, 0 // (h, i)

    ps_mul     f11, f3, f6      // (e*c,       f*a)
    ps_merge10 f8,  f5, f4      // (i,         g)
    ps_mul     f13, f5, f7      // (h*f,       i*d)
    ps_msub    f11, f1, f7, f11 // (b*f - e*c, c*d - f*a)
    ps_mul     f12, f1, f8      // (b*i,       c*g)

    ps_msub f13, f3, f8, f13 // (e*i - h*f, f*g - i*d)
    ps_msub f12, f5, f6, f12 // (h*c - b*i, i*a - c*g)
    
    // TODO(kiwi) Stop being lazy and finish documentation
    ps_mul  f10, f3, f4
    ps_mul  f9, f0, f5
    ps_mul  f8, f1, f2
    ps_msub f10, f2, f5, f10
    ps_msub f9, f1, f4, f9
    ps_msub f8, f0, f3, f8
    ps_mul  f7, f0, f13
    ps_sub  f1, f1, f1 // Set f1 to zero
    ps_madd f7, f2, f12, f7
    ps_madd f7, f4, f11, f7

    // Zero determinant = singular matrix, inverse does not exist
    ps_cmpo0 cr0, f7, f1
    bne inverse_exists

    li r3, FALSE
    blr

inverse_exists:
    fres     f0, f7
    ps_add   f6, f0, f0
    ps_mul   f5, f0, f0
    ps_nmsub f0, f7, f5, f6
    ps_add   f6, f0, f0
    ps_mul   f5, f0, f0
    ps_nmsub f0, f7, f5, f6
    ps_muls0 f13, f13, f0
    ps_muls0 f12, f12, f0
    psq_st   f13, MTX33._00(pOut), 0, 0
    ps_muls0 f11, f11, f0
    psq_st   f12, MTX33._10(pOut), 0, 0
    ps_muls0 f10, f10, f0
    psq_st   f11, MTX33._20(pOut), 0, 0
    ps_muls0 f9, f9, f0
    psq_st   f10, MTX33._02(pOut), 1, 0
    ps_muls0 f8, f8, f0
    psq_st   f9, MTX33._12(pOut),  1, 0
    psq_st   f8, MTX33._22(pOut),  1, 0

    // Inverse matrix exists
    li r3, TRUE
    blr
    // clang-format on
}

MTX34* MTX34Zero(register MTX34* pMtx) {
    register f32 c_zero = 0.0f;

    ASM (
        psq_st c_zero, MTX34._00(pMtx), 0, 0
        psq_st c_zero, MTX34._02(pMtx), 0, 0
        psq_st c_zero, MTX34._10(pMtx), 0, 0
        psq_st c_zero, MTX34._12(pMtx), 0, 0
        psq_st c_zero, MTX34._20(pMtx), 0, 0
        psq_st c_zero, MTX34._22(pMtx), 0, 0
    )

    return pMtx;
}

MTX34* MTX34Scale(register MTX34* pOut, register const MTX34* pIn,
                  register const VEC3* pScale) {
    register f32 xy, z;
    register f32 row0a, row0b;
    register f32 row1a, row1b;
    register f32 row2a, row2b;

    ASM (
        psq_l xy, VEC3.x(pScale), 0, 0 // (XXXX, YYYY)
        psq_l z,  VEC3.z(pScale), 1, 0 // (ZZZZ, 1111)

        psq_l row0a, MTX34._00(pIn), 0, 0
        psq_l row0b, MTX34._02(pIn), 0, 0
        psq_l row1a, MTX34._10(pIn), 0, 0
        psq_l row1b, MTX34._12(pIn), 0, 0
        psq_l row2a, MTX34._20(pIn), 0, 0
        psq_l row2b, MTX34._22(pIn), 0, 0
        
        ps_mul row0a, row0a, xy
        ps_mul row0b, row0b, z
        ps_mul row1a, row1a, xy
        ps_mul row1b, row1b, z
        ps_mul row2a, row2a, xy
        ps_mul row2b, row2b, z
    
        psq_st row0a, MTX34._00(pOut), 0, 0
        psq_st row0b, MTX34._02(pOut), 0, 0
        psq_st row1a, MTX34._10(pOut), 0, 0
        psq_st row1b, MTX34._12(pOut), 0, 0
        psq_st row2a, MTX34._20(pOut), 0, 0
        psq_st row2b, MTX34._22(pOut), 0, 0
    )

    return pOut;
}

MTX34* MTX34Trans(register MTX34* pOut, register const MTX34* pIn,
                  register const VEC3* pTrans) {
    register f32 xy, z;
    register f32 row0a, row0b;
    register f32 row1a, row1b;
    register f32 row2a, row2b;
    register f32 work0, work1, work2;

    ASM (
        psq_l xy, VEC3.x(pTrans), 0, 0 // (XXXX, YYYY)
        psq_l z,  VEC3.z(pTrans), 1, 0 // (ZZZZ, 1111)

        /**
         * Copy inner 3x3 matrix
         */

        psq_l row0a, MTX34._00(pIn), 0, 0
        psq_l row0b, MTX34._02(pIn), 0, 0
        psq_l row1a, MTX34._10(pIn), 0, 0
        psq_l row1b, MTX34._12(pIn), 0, 0
        psq_l row2a, MTX34._20(pIn), 0, 0
        psq_l row2b, MTX34._22(pIn), 0, 0

        psq_st row0a, MTX34._00(pOut), 0, 0
        psq_st row0b, MTX34._02(pOut), 0, 0
        psq_st row1a, MTX34._10(pOut), 0, 0
        psq_st row1b, MTX34._12(pOut), 0, 0
        psq_st row2a, MTX34._20(pOut), 0, 0
        psq_st row2b, MTX34._22(pOut), 0, 0

        /**
         * Perform translation
         */

        ps_mul  work0, row0a, xy            // (_00*x, _01*y)
        ps_madd work1, row0b, z, work0      // (_02*z + _00*x, _03 + _01*y)
        ps_sum0 work2, work1, work2, work1
        psq_st  work2, MTX34._03(pOut), 1, 0

        ps_mul  work0, row1a, xy            // (_10*x, _11*y)
        ps_madd work1, row1b, z, work0      // (_12*z + _10*x, _13 + _11*y)
        ps_sum0 work2, work1, work2, work1
        psq_st  work2, MTX34._13(pOut), 1, 0
        
        ps_mul  work0, row2a, xy            // (_20*x, _21*y)
        ps_madd work1, row2b, z, work0      // (_22*z + _20*x, _23 + _21*y)
        ps_sum0 work2, work1, work2, work1
        psq_st  work2, MTX34._23(pOut), 1, 0
    )

    return pOut;
}

MTX34* MTX34RotAxisFIdx(MTX34* pMtx, const VEC3* pAxis, f32 fidx) {
    PSMTXRotAxisRad(*pMtx, *pAxis, NW4R_MATH_FIDX_TO_RAD(fidx));
    return pMtx;
}

MTX34* MTX34RotXYZFIdx(register MTX34* pOut, register f32 fIdxX,
                       register f32 fIdxY, register f32 fIdxZ) {
    register __vec2x32float__ sincosx, sincosy, sincosz;
    register __vec2x32float__ sincosxneg, mcosxsinx, scx_sinz, scz_cosy, res;
    register f32 c_zero;
    register __vec2x32float__ mf11f21, f22mf12;

    // The angle setup is its own scope: the DWARF has a second c_zero there (a
    // __vec2x32float__ beside the outer f32 one).
    {
        register u32 idxXY;
        register u32 idxZ;
        register const f32* pTblBase = &detail::gSinCosTbl[0].sin_val;
        register f32* pTbl;
        register f32* pTblY;
        register __vec2x32float__ c_idxmax = 65536.0f;
        register __vec2x32float__ c_idxmaxneg;
        register __vec2x32float__ c_zero;
        register __vec2x32float__ abs_fidxXY;
        register f32 abs_fidxZ;
        register __vec2x32float__ fidxXY;
        register __vec2x32float__ r;
        register __vec2x32float__ scval, scdel;
        register f32 work;
        register f32* pTmp;
        f32 tmp;

        pTmp = &tmp;

        // clang-format off
        asm {
            ps_merge00 fidxXY, fIdxX, fIdxY
            ps_merge00 c_idxmax, c_idxmax, c_idxmax
            ps_abs abs_fidxXY, fidxXY
            ps_neg c_idxmaxneg, c_idxmax
            ps_sub c_zero, c_idxmax, c_idxmax
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            ble x_done
        x_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bgt x_loop
        x_done:
            ps_cmpu1 cr0, abs_fidxXY, c_idxmax
            ble y_done
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bgt y_loop
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_done:
            psq_st abs_fidxXY, 0(pTmp), 0, 3
            psq_l r, 0(pTmp), 0, 3
            fabs abs_fidxZ, fIdxZ
            lwz idxXY, 0(pTmp)
            fcmpu cr0, abs_fidxZ, c_idxmax
            ble z_done
        z_loop:
            fsubs abs_fidxZ, abs_fidxZ, c_idxmax
            fcmpu cr0, abs_fidxZ, c_idxmax
            bgt z_loop
        z_done:
            psq_st abs_fidxZ, 0(pTmp), 1, 3
            rlwinm pTbl, idxXY, 20, 20, 27
            add pTbl, pTblBase, pTbl
            ps_sub r, abs_fidxXY, r
            psq_l scval, 0(pTbl), 0, 0
            rlwinm pTblY, idxXY, 4, 20, 27
            psq_l scdel, 8(pTbl), 0, 0
            ps_cmpu0 cr0, fidxXY, c_zero
            add pTblY, pTblBase, pTblY
            ps_madds0 sincosx, scdel, r, scval
            psq_l scval, 0(pTblY), 0, 0
            psq_l scdel, 8(pTblY), 0, 0
            lhz idxZ, 0(pTmp)
            bge x_positive
            ps_neg work, sincosx
            ps_merge01 sincosx, work, sincosx
        x_positive:
            ps_madds1 sincosy, scdel, r, scval
            psq_l r, 0(pTmp), 1, 3
            rlwinm idxZ, idxZ, 4, 20, 27
            ps_cmpu1 cr0, fidxXY, c_zero
            add pTbl, pTblBase, idxZ
            fsubs r, abs_fidxZ, r
            psq_l scval, 0(pTbl), 0, 0
            psq_l scdel, 8(pTbl), 0, 0
            bge y_positive
            ps_neg work, sincosy
            ps_merge01 sincosy, work, sincosy
        y_positive:
            fcmpu cr0, fIdxZ, c_zero
            ps_madds0 sincosz, scdel, r, scval
            bge z_positive
            ps_neg work, sincosz
            ps_merge01 sincosz, work, sincosz
        z_positive:
        }
        // clang-format on
    }

    // clang-format off
    asm {
        ps_neg sincosxneg, sincosx
        ps_muls1 scz_cosy, sincosz, sincosy
        ps_sub c_zero, sincosx, sincosx
        ps_merge10 mcosxsinx, sincosxneg, sincosx
        ps_merge10 res, scz_cosy, scz_cosy
        ps_muls0 scx_sinz, sincosx, sincosz
        psq_st c_zero, 44(pOut), 1, 0
        ps_muls0 mf11f21, mcosxsinx, sincosz
        psq_st res, 0(pOut), 1, 0
        ps_muls1 res, sincosx, sincosz
        ps_muls1 f22mf12, mcosxsinx, sincosz
        ps_madds0 res, res, sincosy, mf11f21
        ps_neg f22mf12, f22mf12
        psq_st res, 4(pOut), 0, 0
        ps_merge00 res, c_zero, scz_cosy
        psq_st res, 12(pOut), 0, 0
        ps_madds0 res, scx_sinz, sincosy, f22mf12
        psq_st res, 20(pOut), 0, 0
        ps_neg res, sincosy
        ps_merge00 res, c_zero, res
        psq_st res, 28(pOut), 0, 0
        ps_muls1 res, sincosx, sincosy
        psq_st res, 36(pOut), 0, 0
    }
    // clang-format on

    return pOut;
}

VEC3* VEC3TransformNormal(VEC3* pOut, const MTX34* pMtx, const VEC3* pVec) {
    VEC3 tmp;
    tmp.x = pMtx->_00 * pVec->x + pMtx->_01 * pVec->y + pMtx->_02 * pVec->z;
    tmp.y = pMtx->_10 * pVec->x + pMtx->_11 * pVec->y + pMtx->_12 * pVec->z;
    tmp.z = pMtx->_20 * pVec->x + pMtx->_21 * pVec->y + pMtx->_22 * pVec->z;

    pOut->x = tmp.x;
    pOut->y = tmp.y;
    pOut->z = tmp.z;

    return pOut;
}

MTX44* MTX44Identity(register MTX44* pMtx) {
    register f32 c_zero = 0.0f, c_one = 1.0f;
    register f32 c_01, c_10;

    ASM (
        ps_merge01 c_01, c_zero, c_one
        ps_merge10 c_10, c_one, c_zero

        psq_st c_zero, MTX44._02(pMtx), 0, 0  // _02=0, _03=0
        psq_st c_zero, MTX44._12(pMtx), 0, 0  // _12=0, _13=0
        psq_st c_zero, MTX44._20(pMtx), 0, 0  // _20=0, _21=0
        psq_st c_01,   MTX44._10(pMtx), 0, 0  // _10=0, _11=1
        psq_st c_10,   MTX44._00(pMtx), 0, 0  // _00=1, _01=0
        psq_st c_10,   MTX44._22(pMtx), 0, 0  // _22=1, _23=0
        psq_st c_zero, MTX44._30(pMtx), 0, 0  // _30=0, _31=0
        psq_st c_01,   MTX44._32(pMtx), 0, 0  // _32=0, _33=1
    )

    return pMtx;
}

MTX44* MTX44Copy(register MTX44* pDst, register const MTX44* pSrc) {
    register f32 row0a, row0b;
    register f32 row1a, row1b;
    register f32 row2a, row2b;
    register f32 row3a, row3b;

    ASM (
        psq_l row0a, MTX44._00(pSrc), 0, 0
        psq_l row0b, MTX44._02(pSrc), 0, 0
        psq_l row1a, MTX44._10(pSrc), 0, 0
        psq_l row1b, MTX44._12(pSrc), 0, 0
        psq_l row2a, MTX44._20(pSrc), 0, 0
        psq_l row2b, MTX44._22(pSrc), 0, 0
        psq_l row3a, MTX44._30(pSrc), 0, 0
        psq_l row3b, MTX44._32(pSrc), 0, 0

        psq_st row0a, MTX44._00(pDst), 0, 0
        psq_st row0b, MTX44._02(pDst), 0, 0
        psq_st row1a, MTX44._10(pDst), 0, 0
        psq_st row1b, MTX44._12(pDst), 0, 0
        psq_st row2a, MTX44._20(pDst), 0, 0
        psq_st row2b, MTX44._22(pDst), 0, 0
        psq_st row3a, MTX44._30(pDst), 0, 0
        psq_st row3b, MTX44._32(pDst), 0, 0
    )

    return pDst;
}

} // namespace math
} // namespace nw4r
