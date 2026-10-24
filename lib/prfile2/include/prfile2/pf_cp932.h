#ifndef PRFILE2_PF_CP932_H
#define PRFILE2_PF_CP932_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFCODE_CP932_OEM2Unicode(const s8* cp932_src, u16* uc_dst);
s32 PFCODE_CP932_Unicode2OEM(const u16* uc_src, s8* cp932_dst);
s32 PFCODE_CP932_OEMCharWidth(const s8* buf);
BOOL PFCODE_CP932_isOEMMBchar(const s8 cp932, BOOL num);
s32 PFCODE_CP932_UnicodeCharWidth(const u16* buf);
BOOL PFCODE_CP932_isUnicodeMBchar(const u16 uc_src, BOOL num);

#ifdef __cplusplus
}
#endif
#endif
