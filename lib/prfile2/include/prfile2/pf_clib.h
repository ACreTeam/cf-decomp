#ifndef PRFILE2_PF_CLIB_H
#define PRFILE2_PF_CLIB_H

// C library replacements of the standalone PrFILE2 (lib/prfile2/common/pf_clib.c).

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 pf_toupper(s32 c);
void* pf_memcpy(void* dst, const void* src, u32 length);
void* pf_memset(void* dst, s32 c, u32 length);
s32 pf_memcmp(const void* s1, const void* s2, u32 length);
u32 pf_strlen(const s8* s);
s8* pf_strcpy(s8* dst, const s8* src);
s32 pf_strcmp(const s8* s1, const s8* s2);
s32 pf_strncmp(const s8* s1, const s8* s2, u32 length);
u32 pf_w_strlen(const u16* s);
u16* pf_w_strcpy(u16* dst, const u16* src);
s32 pf_w_strcmp(const u16* s1, const u16* s2);
s32 pf_w_strncmp(const u16* s1, const u16* s2, u32 length);

#ifdef __cplusplus
}
#endif
#endif
