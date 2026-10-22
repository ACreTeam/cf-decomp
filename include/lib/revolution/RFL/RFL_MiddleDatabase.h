#ifndef RVL_SDK_RFL_MIDDLE_DATABASE_H
#define RVL_SDK_RFL_MIDDLE_DATABASE_H

#include <revolution/RFL/RFL_Types.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 RFLGetMiddleDBBufferSize(u16 num);                                         // 802C4F78
void RFLInitMiddleDB(RFLMiddleDB *db, int type, void *buf, u16 num);          // 802C4F80
RFLErrcode RFLAddMiddleDBStoreData(RFLMiddleDB *db, const RFLStoreData *data); // 802C51E0

#ifdef __cplusplus
}
#endif
#endif
