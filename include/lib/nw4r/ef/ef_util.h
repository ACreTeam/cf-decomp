#ifndef NW4R_EF_UTIL_H
#define NW4R_EF_UTIL_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ut.h>

namespace nw4r {
namespace ef {

math::MTX34 *PSMTX34RotXYZRad(math::MTX34 *pOut, f32 x, f32 y, f32 z);
math::MTX34 *MTX34ScaleNonZero(math::MTX34 *pOut, const math::MTX34 *pM,
                               const math::VEC3 *pS);
math::MTX34 *MTX34ScaleNonZero(math::MTX34 *pOut, const math::VEC3 *pS,
                               const math::MTX34 *pM);
math::MTX34 *MTX34ScaleInv(math::MTX34 *pOut, const math::MTX34 *pM,
                           const math::VEC3 *pS);
f32 MTXColLen(const math::MTX34 *pSrc, int col);

u16 UtlistToArray(const ut::List *pList, void **ppArray, int maxlen);

bool Normalize(math::VEC3 *dst, const math::VEC3 *vec);

void _PSSinCosRad(f32 *sincos, f32 angle);
void PSSinCosRad(f32 *sin, f32 *cos, f32 angle);

math::VEC3 &Rotation2VecY(const math::VEC3 &rRot, math::VEC3 *pVec);
void GetDirMtxY(math::MTX34 *pMtx, const math::VEC3 &rVec);

void MtxGetRotationMtx(const math::MTX34 &rMtx, math::MTX34 *pDst);

void MtxSetColVec(math::MTX34 *pMtx, int col, const math::VEC3 *pVec);
math::VEC3 *MtxColVec(const math::MTX34 *pMtx, int col, math::VEC3 *pVec);

void MtxGetRotation(const math::MTX34 &rMtx, math::VEC3 *pRot);
void MtxGetTranslate(const math::MTX34 &rMtx, math::VEC3 *pTrans);
void MtxGetScale(const math::MTX34 &rMtx, math::VEC3 *pScale);

} // namespace ef
} // namespace nw4r

#endif
