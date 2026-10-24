#ifndef PRFILE2_PF_FAT12_H
#define PRFILE2_PF_FAT12_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFFAT12_ReadFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value);
s32 PFFAT12_WriteFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u16 value);
s32 PFFAT12_WriteFATEntryWithBuf(struct PF_VOLUME* p_vol, u16 cluster, u16 value, struct PF_CACHE_PAGE* p_page);
s32 PFFAT12_ReadFATEntryWithBuf(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value, struct PF_CACHE_PAGE* p_page);

#ifdef __cplusplus
}
#endif
#endif
