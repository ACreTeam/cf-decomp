#ifndef RVL_SDK_MTX_MTXVEC_H
#define RVL_SDK_MTX_MTXVEC_H
#include <types.h>

#include <revolution/MTX/mtxtypes.h>
#ifdef __cplusplus
extern "C" {
#endif

void PSMTXMultVec(const Mtx mtx, const Vec* vec, Vec* out);   // 803911A4: out = mtx * vec (with the translation)
void PSMTXMultVecSR(const Mtx mtx, const Vec* vec, Vec* out); // 803911F8: rotation / scale only

#ifdef __cplusplus
}
#endif
#endif
