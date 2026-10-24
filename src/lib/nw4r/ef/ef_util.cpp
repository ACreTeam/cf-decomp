#include <nw4r/ef.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>

#include <cmath>

namespace nw4r {
namespace ef {

static f32 sPS_Three_Half[2] = {3.0f, 0.5f};
static const f32 EMITTER_ZERO_SCALE = 0.00001f;

// Layout witness for constants contributed by stripped SDK code. The original
// owner of these early literals has not been recovered.
bool StrippedUtilLiteralLayoutWitness(register f32 *pair, f32 value) {
    register __vec2x32float__ idxmax = 65536.0f;
    // clang-format off
    asm { psq_st idxmax, 0(pair), 0, 0 }
    // clang-format on
    value *= NW4R_MATH_RAD_TO_FIDX(1.0f);
    return value == 0.0f || value == 1.0f || value == 0.00001f ||
           value == -1.0f;
}

u16 UtlistToArray(const ut::List *pList, void **ppArray, int maxlen) {
    u16 num = 0;

    NW4R_UT_LIST_FOREACH(void, it, *pList, {
        ppArray[num++] = it;

        if (num >= maxlen) {
            break;
        }
    })

    return num;
}

void GetDirMtxY(math::MTX34 *pMtx, const math::VEC3 &rVec) {
    f32 sx, cx;
    f32 sz, cz;

    sx = rVec.z;
    cx = 1.0f - sx * sx;
    if (cx < 0.0f) {
        cx = 0.0f;
    }
    cx = math::FSqrt(cx);

    if (cx > NW4R_MATH_FLT_EPSILON) {
        cz = rVec.y / cx;
        sz = rVec.x / -cx;
    } else {
        sz = 0.0f;
        cz = 1.0f;
    }

    pMtx->_00 = cz;
    pMtx->_01 = rVec.x;
    pMtx->_02 = sx * sz;
    pMtx->_03 = 0.0f;

    pMtx->_10 = sz;
    pMtx->_11 = rVec.y;
    pMtx->_12 = -cz * sx;
    pMtx->_13 = 0.0f;

    pMtx->_20 = 0.0f;
    pMtx->_21 = rVec.z;
    pMtx->_22 = cx;
    pMtx->_23 = 0.0f;
}

void MtxGetRotationMtx(const math::MTX34 &rMtx, math::MTX34 *pDst) {
    math::VEC3 x, y, z;

    MtxColVec(&rMtx, 0, &x);
    if (!Normalize(&x, &x)) {
        x.x = 1.0f;
    }
    MtxColVec(&rMtx, 1, &y);
    if (!Normalize(&y, &y)) {
        y.y = 1.0f;
    }

    math::VEC3Cross(&z, &x, &y);
    math::VEC3Cross(&y, &z, &x);

    MtxSetColVec(pDst, 0, &x);
    MtxSetColVec(pDst, 1, &y);
    MtxSetColVec(pDst, 2, &z);

    pDst->_03 = 0.0f;
    pDst->_13 = 0.0f;
    pDst->_23 = 0.0f;
}

inline void MtxSetColVec(math::MTX34 *pMtx, int col, const math::VEC3 *pVec) {
    pMtx->m[0][col] = pVec->x;
    pMtx->m[1][col] = pVec->y;
    pMtx->m[2][col] = pVec->z;
}

inline math::VEC3 *MtxColVec(const math::MTX34 *pMtx, int col,
                             math::VEC3 *pVec) {
    pVec->x = pMtx->m[0][col];
    pVec->y = pMtx->m[1][col];
    pVec->z = pMtx->m[2][col];

    return pVec;
}

void MtxGetRotation(const math::MTX34 &rMtx, math::VEC3 *pRot) {
    f32 C;
    f32 sx, sy, sz;
    f32 f;

    if (!((sx = MTXColLen(&rMtx, 0)) < NW4R_MATH_FLT_MIN) &&
        !((sy = MTXColLen(&rMtx, 1)) < NW4R_MATH_FLT_MIN) &&
        !((sz = MTXColLen(&rMtx, 2)) < NW4R_MATH_FLT_MIN)) {

        f = -rMtx._20 / sx;
        if (f > 1.0f) {
            f = 1.0f;
        }
        if (f < -1.0f) {
            f = -1.0f;
        }
        pRot->y = std::asinf(f);
        C = math::CosRad(pRot->y);

        if (C >= NW4R_MATH_FLT_MIN) {
            pRot->x = std::atan2f(rMtx._21 / sy, rMtx._22 / sz);
            pRot->z = std::atan2f(rMtx._10, rMtx._00);
        } else {
            pRot->x = std::atan2f(rMtx._01, rMtx._11);
            pRot->z = 0.0f;
        }
    } else {
        pRot->x = 0.0f;
        pRot->y = 0.0f;
        pRot->z = 0.0f;
    }
}

void MtxGetTranslate(const math::MTX34 &rMtx, math::VEC3 *pTrans) {
    pTrans->x = rMtx._03;
    pTrans->y = rMtx._13;
    pTrans->z = rMtx._23;
}

void MtxGetScale(const math::MTX34 &rMtx, math::VEC3 *pScale) {
    f32 sh[3];
    math::VEC3 v0, v1, v2, v;
    f32 mag;

    MtxColVec(&rMtx, 0, &v0);
    mag = math::VEC3LenSq(&v0);

    if (mag > NW4R_MATH_FLT_EPSILON) {
        mag = math::FrSqrt(mag);
        pScale->x = math::FInv(mag);

        math::VEC3Scale(&v0, &v0, mag);
        MtxColVec(&rMtx, 1, &v1);

        sh[0] = math::VEC3Dot(&v0, &v1);
        math::VEC3Scale(&v, &v0, sh[0]);
        math::VEC3Sub(&v1, &v1, &v);

        mag = math::VEC3LenSq(&v1);

        if (mag > NW4R_MATH_FLT_EPSILON) {
            mag = math::FrSqrt(mag);
            pScale->y = math::FInv(mag);

            sh[0] *= mag;
            math::VEC3Scale(&v1, &v1, mag);
            MtxColVec(&rMtx, 2, &v2);

            sh[2] = math::VEC3Dot(&v1, &v2);
            math::VEC3Scale(&v, &v1, sh[2]);
            math::VEC3Sub(&v2, &v2, &v);

            sh[1] = math::VEC3Dot(&v0, &v2);
            math::VEC3Scale(&v, &v0, sh[1]);
            math::VEC3Sub(&v2, &v2, &v);

            mag = math::VEC3LenSq(&v2);

            if (mag > NW4R_MATH_FLT_EPSILON) {
                pScale->z = math::FSqrt(mag);
                math::VEC3Cross(&v, &v1, &v2);

                if (math::VEC3Dot(&v0, &v) < 0.0f) {
                    // Double precision
                    pScale->x *= -1.0;
                    pScale->y *= -1.0;
                    pScale->z *= -1.0;
                }
            } else {
                pScale->z = 0.0f;
            }
        } else {
            pScale->y = 0.0f;
            MtxColVec(&rMtx, 2, &v2);

            sh[1] = math::VEC3Dot(&v0, &v2);
            math::VEC3Scale(&v, &v0, sh[1]);
            math::VEC3Sub(&v2, &v2, &v);

            mag = math::VEC3LenSq(&v2);

            if (mag > NW4R_MATH_FLT_EPSILON) {
                pScale->z = math::FSqrt(mag);
            } else {
                pScale->z = 0.0f;
            }
        }

    } else {
        pScale->x = 0.0f;

        MtxColVec(&rMtx, 1, &v1);
        mag = math::VEC3LenSq(&v1);

        if (mag > NW4R_MATH_FLT_EPSILON) {
            mag = math::FrSqrt(mag);
            pScale->y = math::FInv(mag);

            math::VEC3Scale(&v1, &v1, mag);
            MtxColVec(&rMtx, 2, &v2);

            sh[2] = math::VEC3Dot(&v1, &v2);
            math::VEC3Scale(&v, &v1, sh[2]);
            math::VEC3Sub(&v2, &v2, &v);
            pScale->z = math::VEC3Len(&v2);
        } else {
            pScale->y = 0.0f;

            MtxColVec(&rMtx, 2, &v2);
            pScale->z = math::VEC3Len(&v2);
        }
    }
}

bool Normalize(register math::VEC3 *dst, register const math::VEC3 *vec) {
    register f32 c_half = 0.5f;
    register f32 c_three = 3.0f;
    register f32 c_zero;
    register __vec2x32float__ v1_xy, v1_z, xx_zz, xx_yy;
    register f32 sqsum, rsqrt, nwork0, nwork1;
    // clang-format off
    asm {
        psq_l v1_xy, 0(vec), 0, 0
        ps_mul xx_yy, v1_xy, v1_xy
        psq_l v1_z, 8(vec), 1, 0
        fsubs c_zero, c_half, c_half
        ps_madd xx_zz, v1_z, v1_z, xx_yy
        ps_sum0 sqsum, xx_zz, v1_z, xx_yy
        fcmpu cr0, sqsum, c_zero
        beq zero
        frsqrte rsqrt, sqsum
        fmuls nwork0, rsqrt, rsqrt
        fmuls nwork1, rsqrt, c_half
        fnmsubs nwork0, nwork0, sqsum, c_three
        fmuls rsqrt, nwork0, nwork1
        ps_muls0 v1_xy, v1_xy, rsqrt
        ps_muls0 v1_z, v1_z, rsqrt
        psq_st v1_xy, 0(dst), 0, 0
        psq_st v1_z, 8(dst), 1, 0
    }
    // clang-format on
    return true;
zero:
    // clang-format off
    asm {
        psq_st v1_xy, 0(dst), 0, 0
        psq_st v1_z, 8(dst), 1, 0
    }
    // clang-format on
    return false;
}

void _PSSinCosRad(register f32 *pRes, register f32 val) {
    register f32 fidx;
    register u32 idx;
    register f32 abs_fidx, r;
    register __vec2x32float__ result;
    register f32 idxmax = 65536.0f;
    register __vec2x32float__ scval, scdel;
    register f32 c_radToFidx = NW4R_MATH_RAD_TO_FIDX(1.0f);
    register f32 c_zero;
    register const f32 *pTbl = &math::detail::gSinCosTbl[0].sin_val;
    // clang-format off
    asm {
        fmuls fidx, val, c_radToFidx
        fabs abs_fidx, fidx
        psq_st abs_fidx, 0(pRes), 1, 3
        fcmpu cr0, abs_fidx, idxmax
        ble done_wrap
    wrap:
        fsubs abs_fidx, abs_fidx, idxmax
        fcmpu cr0, abs_fidx, idxmax
        bge wrap
        psq_st abs_fidx, 0(pRes), 1, 3
    done_wrap:
        lhz idx, 0(pRes)
        fsubs c_zero, idxmax, idxmax
        rlwinm idx, idx, 4, 20, 27
        add pTbl, pTbl, idx
        psq_l r, 0(pRes), 1, 3
        fcmpu cr0, fidx, c_zero
        psq_l scval, 0(pTbl), 0, 0
        fsubs r, abs_fidx, r
        psq_l scdel, 8(pTbl), 0, 0
        ps_madds0 result, scdel, r, scval
        bge positive
        ps_neg fidx, result
        ps_merge01 result, fidx, result
    positive:
        psq_st result, 0(pRes), 0, 0
    }
    // clang-format on
}

void PSSinCosRad(register f32 *pSin, register f32 *pCos, register f32 val) {
    register f32 fidx;
    register u32 idx;
    register f32 abs_fidx, r;
    register __vec2x32float__ result;
    register f32 idxmax = 65536.0f;
    register __vec2x32float__ scval, scdel;
    register f32 c_radToFidx = NW4R_MATH_RAD_TO_FIDX(1.0f);
    register f32 c_zero;
    register const f32 *pTbl = &math::detail::gSinCosTbl[0].sin_val;
    // clang-format off
    asm {
        fmuls fidx, val, c_radToFidx
        fabs abs_fidx, fidx
        psq_st abs_fidx, 0(pSin), 1, 3
        fcmpu cr0, abs_fidx, idxmax
        ble done_wrap
    wrap:
        fsubs abs_fidx, abs_fidx, idxmax
        fcmpu cr0, abs_fidx, idxmax
        bge wrap
        psq_st abs_fidx, 0(pSin), 1, 3
    done_wrap:
        lhz idx, 0(pSin)
        fsubs c_zero, idxmax, idxmax
        rlwinm idx, idx, 4, 20, 27
        add pTbl, pTbl, idx
        psq_l r, 0(pSin), 1, 3
        fcmpu cr0, fidx, c_zero
        psq_l scval, 0(pTbl), 0, 0
        fsubs r, abs_fidx, r
        psq_l scdel, 8(pTbl), 0, 0
        ps_madds0 result, scdel, r, scval
        ps_merge10 r, result, result
        psq_st r, 0(pCos), 1, 0
        bge positive
        ps_neg result, result
    positive:
        psq_st result, 0(pSin), 1, 0
    }
    // clang-format on
}

math::VEC3 &Rotation2VecY(const math::VEC3 &rRot, register math::VEC3 *pOut) {
    register const f32 *pAngle = &rRot.x;
    register f32 angleX = pAngle[0], angleY = pAngle[1], angleZ = pAngle[2];
    register __vec2x32float__ sincosr, sincosp, sincosh;
    register f32 work;
    register __vec2x32float__ sssc, res1, add1;
    {
        register u32 idxXY;
        register u32 idxZ;
        register const f32 *pTblBase = &math::detail::gSinCosTbl[0].sin_val;
        register f32 *pTbl;
        register f32 *pTblY;
        register __vec2x32float__ c_idxmax = 65536.0f;
        register __vec2x32float__ c_idxmaxneg;
        register f32 c_radToFidx = NW4R_MATH_RAD_TO_FIDX(1.0f);
        register __vec2x32float__ c_zero;
        register __vec2x32float__ abs_fidxXY;
        register f32 abs_fidxZ;
        register __vec2x32float__ fidxXY;
        register f32 fidxZ;
        register __vec2x32float__ r;
        register __vec2x32float__ scval, scdel;
        register f32 work;
        register f32 *pTmp;
        f32 tmp;

        pTmp = &tmp;

        // clang-format off
        asm {
            ps_merge00 fidxXY, angleX, angleY
            ps_merge00 c_idxmax, c_idxmax, c_idxmax
            ps_muls0 fidxXY, fidxXY, c_radToFidx
            ps_neg c_idxmaxneg, c_idxmax
            ps_abs abs_fidxXY, fidxXY
            ps_sub c_zero, c_idxmax, c_idxmax
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            ble x_done
        x_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bge x_loop
        x_done:
            ps_cmpu1 cr0, abs_fidxXY, c_idxmax
            ble y_done
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bge y_loop
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_done:
            psq_st abs_fidxXY, 0(pTmp), 0, 3
            fmuls fidxZ, angleZ, c_radToFidx
            psq_l r, 0(pTmp), 0, 3
            fabs abs_fidxZ, fidxZ
            lwz idxXY, 0(pTmp)
            fcmpu cr0, abs_fidxZ, c_idxmax
            ble z_done
        z_loop:
            fsubs abs_fidxZ, abs_fidxZ, c_idxmax
            fcmpu cr0, abs_fidxZ, c_idxmax
            bge z_loop
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
            ps_madds0 sincosr, scdel, r, scval
            psq_l scval, 0(pTblY), 0, 0
            psq_l scdel, 8(pTblY), 0, 0
            lhz idxZ, 0(pTmp)
            bge x_positive
            ps_neg work, sincosr
            ps_merge01 sincosr, work, sincosr
        x_positive:
            ps_madds1 sincosp, scdel, r, scval
            psq_l r, 0(pTmp), 1, 3
            rlwinm idxZ, idxZ, 4, 20, 27
            ps_cmpu1 cr0, fidxXY, c_zero
            add pTbl, pTblBase, idxZ
            fsubs r, abs_fidxZ, r
            psq_l scval, 0(pTbl), 0, 0
            psq_l scdel, 8(pTbl), 0, 0
            bge y_positive
            ps_neg work, sincosp
            ps_merge01 sincosp, work, sincosp
        y_positive:
            fcmpu cr0, fidxZ, c_zero
            ps_madds0 sincosh, scdel, r, scval
            bge z_positive
            ps_neg work, sincosh
            ps_merge01 sincosh, work, sincosh
        z_positive:
        }
        // clang-format on
    }

    // clang-format off
    asm {
        ps_muls1 sssc, sincosh, sincosr
        ps_muls0 sincosp, sincosp, sincosr
        ps_merge10 res1, sincosh, sincosh
        ps_neg work, sssc
        ps_merge01 sssc, work, sssc
        ps_madds0 res1, res1, sincosp, sssc
        ps_merge10 sincosp, sincosp, sincosp
        psq_st res1, 0(pOut), 0, 0
        psq_st sincosp, 8(pOut), 1, 0
    }
    // clang-format on
    return *pOut;
}

math::MTX34 *PSMTX34RotXYZRad(register math::MTX34 *pOut, register f32 fIdxX,
                              register f32 fIdxY, register f32 fIdxZ) {
    register __vec2x32float__ sincosx, sincosy, sincosz;
    register __vec2x32float__ sincosxneg, mcosxsinx, scx_sinz, scz_cosy, res;
    register f32 c_zero;
    register __vec2x32float__ mf11f21, f22mf12;

    {
        register u32 idxXY;
        register u32 idxZ;
        register const f32 *pTblBase = &math::detail::gSinCosTbl[0].sin_val;
        register f32 *pTbl;
        register f32 *pTblY;
        register __vec2x32float__ c_idxmax = 65536.0f;
        register __vec2x32float__ c_idxmaxneg;
        register f32 c_radToFidx = NW4R_MATH_RAD_TO_FIDX(1.0f);
        register __vec2x32float__ c_zero;
        register __vec2x32float__ abs_fidxXY;
        register f32 abs_fidxZ;
        register __vec2x32float__ fidxXY;
        register f32 fidxZ;
        register __vec2x32float__ r;
        register __vec2x32float__ scval, scdel;
        register f32 work;
        register f32 *pTmp;
        f32 tmp;

        pTmp = &tmp;

        // clang-format off
        asm {
            ps_merge00 fidxXY, fIdxX, fIdxY
            ps_merge00 c_idxmax, c_idxmax, c_idxmax
            ps_muls0 fidxXY, fidxXY, c_radToFidx
            ps_neg c_idxmaxneg, c_idxmax
            ps_abs abs_fidxXY, fidxXY
            ps_sub c_zero, c_idxmax, c_idxmax
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            ble x_done
        x_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bge x_loop
        x_done:
            ps_cmpu1 cr0, abs_fidxXY, c_idxmax
            ble y_done
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_loop:
            ps_sum0 abs_fidxXY, abs_fidxXY, abs_fidxXY, c_idxmaxneg
            ps_cmpu0 cr0, abs_fidxXY, c_idxmax
            bge y_loop
            ps_merge10 abs_fidxXY, abs_fidxXY, abs_fidxXY
        y_done:
            psq_st abs_fidxXY, 0(pTmp), 0, 3
            fmuls fidxZ, fIdxZ, c_radToFidx
            psq_l r, 0(pTmp), 0, 3
            fabs abs_fidxZ, fidxZ
            lwz idxXY, 0(pTmp)
            fcmpu cr0, abs_fidxZ, c_idxmax
            ble z_done
        z_loop:
            fsubs abs_fidxZ, abs_fidxZ, c_idxmax
            fcmpu cr0, abs_fidxZ, c_idxmax
            bge z_loop
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
            fcmpu cr0, fidxZ, c_zero
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

math::MTX34 *MTX34ScaleNonZero(register math::MTX34 *pOut,
                               register const math::MTX34 *pM,
                               register const math::VEC3 *pS) {
    register f32 xy, z1, row0a, row0b, row1a, row1b, row2a, row2b;
    register f32 c_zero, c_eps = 0.00001f;
    // clang-format off
    asm {
        psq_l xy, 0(pS), 0, 0
        ps_sub c_zero, xy, xy
        psq_l z1, 8(pS), 1, 0
        psq_l row0a, 0(pM), 0, 0
        psq_l row0b, 8(pM), 0, 0
        ps_cmpu0 cr0, xy, c_zero
        bne x_nonzero
        ps_merge01 xy, c_eps, xy
    x_nonzero:
        ps_cmpu1 cr0, xy, c_zero
        psq_l row1a, 16(pM), 0, 0
        psq_l row1b, 24(pM), 0, 0
        bne y_nonzero
        ps_merge00 xy, xy, c_eps
    y_nonzero:
        ps_cmpu0 cr0, z1, c_zero
        psq_l row2a, 32(pM), 0, 0
        psq_l row2b, 40(pM), 0, 0
        bne z_nonzero
        ps_merge01 z1, c_eps, z1
    z_nonzero:
        ps_mul row0a, row0a, xy
        ps_mul row1a, row1a, xy
        ps_mul row2a, row2a, xy
        ps_mul row0b, row0b, z1
        psq_st row0a, 0(pOut), 0, 0
        ps_mul row1b, row1b, z1
        ps_mul row2b, row2b, z1
        psq_st row1a, 16(pOut), 0, 0
        psq_st row0b, 8(pOut), 0, 0
        psq_st row2a, 32(pOut), 0, 0
        psq_st row1b, 24(pOut), 0, 0
        psq_st row2b, 40(pOut), 0, 0
    }
    // clang-format on
    return pOut;
}

math::MTX34 *MTX34ScaleNonZero(register math::MTX34 *dst,
                               register const math::VEC3 *pS,
                               register const math::MTX34 *src) {
    // clang-format off
    asm {
        lis r6, EMITTER_ZERO_SCALE@ha
        lfs f9, 0(pS)
        lfs f12, EMITTER_ZERO_SCALE@l(r6)
        lfs f10, 4(pS)
        fsubs f13, f12, f12
        lfs f11, 8(pS)
        psq_l f4, 0(src), 0, 0
        psq_l f5, 8(src), 0, 0
        fcmpu cr0, f9, f13
        bne x_nonzero
        fmr f9, f12
    x_nonzero:
        fcmpu cr0, f10, f13
        psq_l f6, 16(src), 0, 0
        psq_l f7, 24(src), 0, 0
        bne y_nonzero
        fmr f10, f12
    y_nonzero:
        fcmpu cr0, f11, f13
        psq_l f8, 32(src), 0, 0
        psq_l f2, 40(src), 0, 0
        bne z_nonzero
        fmr f11, f12
    z_nonzero:
        ps_muls0 f4, f4, f9
        ps_muls0 f5, f5, f9
        ps_muls0 f6, f6, f10
        ps_muls0 f7, f7, f10
        psq_st f4, 0(dst), 0, 0
        ps_muls0 f8, f8, f11
        ps_muls0 f2, f2, f11
        psq_st f5, 8(dst), 0, 0
        psq_st f6, 16(dst), 0, 0
        psq_st f7, 24(dst), 0, 0
        psq_st f8, 32(dst), 0, 0
        psq_st f2, 40(dst), 0, 0
    }
    // clang-format on
    return dst;
}

math::MTX34 *MTX34ScaleInv(math::MTX34 *pOut, const math::MTX34 *pM,
                           const math::VEC3 *pS) {
    math::VEC3 vec;
    vec.x = pS->x != 0.0f ? math::FInv(pS->x) : 100000.0f;
    vec.y = pS->y != 0.0f ? math::FInv(pS->y) : 100000.0f;
    vec.z = pS->z != 0.0f ? math::FInv(pS->z) : 100000.0f;
    return math::MTX34Scale(pOut, pM, &vec);
}

f32 MTXColLen(register const math::MTX34 *pSrc, register int col) {
    register f32 vxy, vzz, sqmag, rmag, nwork0, nwork1, c_zero;
    register __vec2x32float__ c_three_half;
    register const f32 *v;
    // clang-format off
    asm {
        slwi v, col, 2
        add v, pSrc, v
        psq_l vxy, 0(v), 1, 0
        psq_l sqmag, 16(v), 1, 0
        lfs vzz, 32(v)
        ps_merge00 vxy, vxy, sqmag
        ps_mul vxy, vxy, vxy
        ps_madd sqmag, vzz, vzz, vxy
        fsubs c_zero, vxy, vxy
        ps_sum0 sqmag, sqmag, vxy, vxy
        fcmpu cr0, sqmag, c_zero
        beq done
        frsqrte rmag, sqmag
        lis r6, sPS_Three_Half@ha
        addi r6, r6, sPS_Three_Half@l
        psq_l c_three_half, 0(r6), 0, 0
        fmuls nwork0, rmag, rmag
        ps_muls1 nwork1, rmag, c_three_half
        fnmsubs nwork0, nwork0, sqmag, c_three_half
        fmuls rmag, nwork0, nwork1
        fmuls sqmag, sqmag, rmag
    done:
    }
    // clang-format on
    return sqmag;
}

} // namespace ef
} // namespace nw4r
