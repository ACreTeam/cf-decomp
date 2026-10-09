#ifndef RVL_SDK_RFL_MODEL_H
#define RVL_SDK_RFL_MODEL_H
#include <types.h>
#include <revolution/MTX.h>
#include <revolution/RFL/RFL_Types.h>
#ifdef __cplusplus
extern "C" {
#endif

// RFL_Model.c (0x802BDC60..0x802BFC78). Names follow the RVL SDK order of RFL_Model.c.
u32 RFLGetModelBufferSize(RFLResolution resolution, u32 expressionFlag);
RFLErrcode RFLInitCharModel(RFLCharModel *charModel, RFLDataSource source, RFLMiddleDB *middleDB, u16 index,
                            void *buffer, RFLResolution resolution, u32 expressionFlag);
void RFLSetMtx(RFLCharModel *charModel, const Mtx mvMtx);
void RFLSetExpression(RFLCharModel *charModel, RFLExpression expression);

void RFLLoadVertexSetting(const RFLDrawCoreSetting *setting);
void RFLLoadMaterialSetting(const RFLDrawCoreSetting *setting);
void RFLDrawOpaCore(const RFLCharModel *charModel, const RFLDrawCoreSetting *setting);
void RFLDrawXluCore(const RFLCharModel *charModel, const RFLDrawCoreSetting *setting);

#ifdef __cplusplus
}
#endif
#endif
