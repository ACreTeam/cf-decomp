#ifndef PRFILE2_PF_FAT_H
#define PRFILE2_PF_FAT_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// value bits of a FAT entry; the top 4 bits of a FAT32 entry are reserved
enum {
    PF_FAT12_ENTRY_MASK = 0xFFF,
    PF_FAT16_ENTRY_MASK = 0xFFFF,
    PF_FAT32_ENTRY_MASK = 0x0FFFFFFF,
};

// PF_BPB.ext_flags (FAT32 BPB_ExtFlags)
enum {
    PF_EXT_FLAG_ACTIVE_FAT_MASK = 0x07,
    PF_EXT_FLAG_NO_MIRRORING = 0x80,  // only the FAT selected by PF_EXT_FLAG_ACTIVE_FAT_MASK is used
};

// return value of PF_VOLUME.p_callback after a driver error while reading the FAT
enum {
    PF_FAT_READ_RETRY = 0,     // read the same sector again
    PF_FAT_READ_NEXT_FAT = 1,  // read the sector of the next FAT copy
};

// mode of PFFAT_GetClusterInChain and PFFAT_GetSector
enum {
    PF_FAT_MODE_SEARCH = 0,
    PF_FAT_MODE_ALLOCATE = 1,  // extend the chain up to chain_index
    PF_FAT_MODE_APPEND = 2,    // allocate num_cluster clusters after chain_index
};

#define PF_FAT_NO_CHAIN_INDEX 0xFFFFFFFF  // PFFAT_FreeChain chain_index: position unknown, cluster link left alone
#define PF_FAT_WHOLE_CHAIN 0xFFFFFFFF  // size argument covering the chain up to its end
#define PF_FAT_SEARCH_TO_LAST_CLUSTER 0xFFFFFFFF  // PFFAT_SearchForNumFreeClusters end_cluster: no upper bound

// reserved FAT entries in front of PF_CLUSTER_FIRST
enum {
    PF_FAT_ENTRY_MEDIA = 0,     // media descriptor in the low byte
    PF_FAT_ENTRY_RESERVED = 1,  // end-of-chain mark
};

s32 PFFAT_ReadFATSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page, u32 cluster);
s32 PFFAT_SearchForNumFreeClusters(struct PF_VOLUME* p_vol, u32 start_cluster, u32 end_cluster, u32 num_cluster, u32* p_start_free_cluster, u32* p_last_free_cluster);
s32 PFFAT_ResetCluster(struct PF_VOLUME* p_vol, u32 cluster);
s32 PFFAT_FindClusterLink(struct PF_FFD* p_ffd, u32 chain_index, u32* p_cluster, u32* is_found);
s32 PFFAT_FindClusterLinkWithBuf(struct PF_FFD* p_ffd, u32 chain_index, u32* p_cluster, u32* is_found, struct PF_CACHE_PAGE* p_page);
s32 PFFAT_ReadCluster(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32* next_cluster, u32* chk_clstlnk);
s32 PFFAT_ReadClusterWithBuf(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32* next_cluster, u32* chk_clstlnk, struct PF_CACHE_PAGE* p_page);
s32 PFFAT_WriteCluster(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32 next_cluster, u32 use_clstlnk);
s32 PFFAT_WriteClusterWithBuf(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32 next_cluster, u32 use_clstlnk, struct PF_CACHE_PAGE* p_page);
s32 PFFAT_DoAllocateChain(struct PF_FFD* p_ffd, u32 chain_len, u32 chain_index, u32* p_chain_start, u32* p_last_allocated, u32 is_reset);
s32 PFFAT_GetClusterInChain(struct PF_FFD* p_ffd, u32 chain_index, u32 mode, u32 num_cluster, u32* locate_start, u32* locate_end);
s32 PFFAT_GetClusterContinuousSectorInChain(struct PF_FFD* p_ffd, u32 initial_cluster, u32 chain_index, u32 size, u32* p_num_sector);
s32 PFFAT_GetClusterAllocatedInChain(struct PF_FFD* p_ffd, u32 initial_cluster, u32 chain_index, u32 size, u32* p_num_clusters);
s32 PFFAT_GetClusterSpecified(struct PF_FFD* p_ffd, u32 chain_index, u32 may_allocate, u32* p_cluster);
s32 PFFAT_GetClusterAllocated(struct PF_FFD* p_ffd, u32 chain_index, u32 num_cluster, u32* p_cluster);
s32 PFFAT_GetSector(struct PF_FFD* p_ffd, u32 file_sector_index, u32 mode, u32 size, u32* p_sector);
s32 PFFAT_GetSectorSpecified(struct PF_FFD* p_ffd, u32 file_sector_index, u32 may_allocate, u32* p_sector);
s32 PFFAT_GetSectorAllocated(struct PF_FFD* p_ffd, u32 file_sector_index, u32 size, u32* p_sector);
s32 PFFAT_getContinuousSector(struct PF_FFD* p_ffd, u32 file_sector_index, u32 size, u32* p_sector, u32* p_num_sector);
s32 PFFAT_CountAllocatedClusters(struct PF_FFD* p_ffd, u32 size, u32* p_num_alloc_clusters);
s32 PFFAT_CountFreeClusters(struct PF_VOLUME* p_vol, u32* p_num_free_clusters);
s32 PFFAT_FreeChain(struct PF_FFD* p_ffd, u32 start_cluster, u32 chain_index, u32 size);
s32 PFFAT_getBeforeChain(struct PF_VOLUME* p_vol, u32 start_cluster, u32 lActive, u32* p_cluster);
s32 PFFAT_InitFATRegion(struct PF_VOLUME* p_vol);
s32 PFFAT_MakeRootDir(struct PF_VOLUME* p_vol);
s32 PFFAT_RefreshFSINFO(struct PF_VOLUME* p_vol);
void PFFAT_InitHint(struct PF_FAT_HINT* p_hint);
s32 PFFAT_TraceClustersChain(struct PF_FFD* p_ffd, u32 start_clst, u32 size, u32* p_target_clst, u32* p_next_clst);
s32 PFFAT_WriteValueToSpecifiedCluster(struct PF_VOLUME* p_vol, u32 cluster, u32 value);
s32 PFFAT_ReadValueToSpecifiedCluster(struct PF_VOLUME* p_vol, u32 cluster, u32* value);
s32 PFFAT_AllocateNumClusters(struct PF_FFD* p_ffd, u32 size, u32* p_chain_start, u32* p_last_allocated, u32* p_allocated_size);
s32 PFFAT_ResetFFD(struct PF_FFD* p_ffd, u32* p_start_cluster);
s32 PFFAT_InitFFD(struct PF_FFD* p_ffd, struct PF_FAT_HINT* p_hint, struct PF_VOLUME* p_vol, u32* p_start_cluster);
s32 PFFAT_FinalizeFFD(struct PF_FFD* p_ffd);
u32 PFFAT_GetValueOfEOC2(struct PF_VOLUME* p_vol);

#ifdef __cplusplus
}
#endif
#endif
