#ifndef RVL_SDK_MTX_MTXVEC_H
#define RVL_SDK_MTX_MTXVEC_H
#include <types.h>

#include <revolution/MTX/mtxtypes.h>
#ifdef __cplusplus
extern "C" {
#endif

void fn_803911A4(const Mtx mtx, const Vec* vec, Vec* out); // like PSMTXMultVec (also applies the translation)
void PSMTXMultVec(const Mtx mtx, const Vec* vec, Vec* out);

#ifdef __cplusplus
}
#endif
#endif
