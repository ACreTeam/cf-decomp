#ifndef PRFILE2_PF_FAT16_H
#define PRFILE2_PF_FAT16_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFFAT16_ReadFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value);
s32 PFFAT16_WriteFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32 value);
s32 PFFAT16_WriteFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32 value, struct PF_CACHE_PAGE* p_page);
s32 PFFAT16_ReadFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value, struct PF_CACHE_PAGE* p_page);

#ifdef __cplusplus
}
#endif
#endif
