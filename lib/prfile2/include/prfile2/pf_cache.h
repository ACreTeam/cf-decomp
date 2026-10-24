#ifndef PRFILE2_PF_CACHE_H
#define PRFILE2_PF_CACHE_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// PF_CACHE_PAGE.stat
enum {
    PF_CACHE_PAGE_STAT_USED = 0x1,      // holds a sector
    PF_CACHE_PAGE_STAT_MODIFIED = 0x2,  // p_mod_sbuf..p_mod_ebuf not yet written to the media
    PF_CACHE_PAGE_STAT_FAT = 0x4,       // belongs to the FAT cache
};

// PF_CACHE_PAGE.option of a data page used as a FAT sector buffer by PFFAT_*WithBuf
enum {
    PF_CACHE_PAGE_OPT_NONE = 0,
    PF_CACHE_PAGE_OPT_FAT_MODIFIED = 1,  // write the sector to every active FAT before reading another one
};

s32 PFCACHE_InitPageList(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* pages, u8 (*buffers)[PF_CACHE_PAGE_SIZE], u16 num, u32 size, u32 is_fat);
struct PF_CACHE_PAGE* PFCACHE_SearchForPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_head, u32 sector);
u32 PFCACHE_SearchForFreePage(struct PF_CACHE_PAGE* p_head, struct PF_CACHE_PAGE** pp_page);
s32 PFCACHE_FlushPageIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page);
s32 PFCACHE_DoAllocatePage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit);
s32 PFCACHE_DoReadPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig);
s32 PFCACHE_DoReadPageAndFlushIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig);
s32 PFCACHE_DoReadNumSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success);
s32 PFCACHE_DoWritePage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, struct PF_CACHE_PAGE* p_page, u32 set_sig);
s32 PFCACHE_DoWriteSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, const u8* p_buf, u32 sector);
s32 PFCACHE_DoWriteNumSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE** pp_head, const u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success);
s32 PFCACHE_DoFlushCache(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_head);
void PFCACHE_SetCache(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_cache_page, u8 (*p_cache_buf)[PF_CACHE_PAGE_SIZE], u16 num_fat_pages, u16 num_data_pages);
void PFCACHE_SetFATBufferSize(struct PF_VOLUME* p_vol, u32 size);
void PFCACHE_SetDataBufferSize(struct PF_VOLUME* p_vol, u32 size);
s32 PFCACHE_InitCaches(struct PF_VOLUME* p_vol);
s32 PFCACHE_SetWriteThroughMode(struct PF_VOLUME* p_vol);
s32 PFCACHE_SetWriteBackMode(struct PF_VOLUME* p_vol);
s32 PFCACHE_AllocateFATPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit);
s32 PFCACHE_AllocateDataPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32* p_is_hit);
void PFCACHE_FreeFATPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page);
void PFCACHE_FreeDataPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page);
s32 PFCACHE_ReadFATPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page);
s32 PFCACHE_ReadDataPage(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig);
s32 PFCACHE_ReadDataPageAndFlushIfNeeded(struct PF_VOLUME* p_vol, u32 sector, struct PF_CACHE_PAGE** pp_page, u32 set_sig);
s32 PFCACHE_ReadDataNumSector(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success);
s32 PFCACHE_WriteFATPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page);
s32 PFCACHE_WriteDataPage(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page, u32 set_sig);
s32 PFCACHE_WriteFATSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector);
s32 PFCACHE_WriteDataNumSectorAndFreeIfNeeded(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u32 num_sector, u32* p_num_success);
struct PF_CACHE_PAGE* PFCACHE_SearchDataCache(struct PF_VOLUME* p_vol, u32 sector);
s32 PFCACHE_FlushFATCache(struct PF_VOLUME* p_vol);
s32 PFCACHE_FlushDataCache(struct PF_VOLUME* p_vol);
s32 PFCACHE_FlushDataCacheSpecific(struct PF_VOLUME* p_vol, void* signature);
s32 PFCACHE_FlushAllCaches(struct PF_VOLUME* p_vol);
void PFCACHE_FreeAllCaches(struct PF_VOLUME* p_vol);

#ifdef __cplusplus
}
#endif
#endif
