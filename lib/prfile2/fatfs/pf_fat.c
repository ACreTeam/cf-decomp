#include <prfile2/pdm_bpb.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cluster.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_entry.h>
#include <prfile2/pf_fat.h>
#include <prfile2/pf_fat12.h>
#include <prfile2/pf_fat16.h>
#include <prfile2/pf_fat32.h>
#include <prfile2/pf_sector.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_volume.h>

static const struct {
    u32 bad;
    u32 eoc1;
    u32 eoc2;
    u32 fat0_mask;
    u32 fat1;
} fat_special_values[3] = {
    {0xFF7, 0xFF8, 0xFFF, 0xF00, 0xFFF},                     // FAT12
    {0xFFF7, 0xFFF8, 0xFFFF, 0xFF00, 0xFFFF},                // FAT16
    {0xFFFFFF7, 0xFFFFFF8, 0xFFFFFFF, 0xFFFFF00, 0xFFFFFFF}  // FAT32
};

static s32 PFFAT_ReadFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value) {
    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            return PFFAT12_ReadFATEntry(p_vol, cluster, p_value);
        case FAT_16:
            return PFFAT16_ReadFATEntry(p_vol, cluster, p_value);
        case FAT_32:
            return PFFAT32_ReadFATEntry(p_vol, cluster, p_value);
        default:
            break;
    }

    *p_value = PF_INVALID_CLUSTER;
    return PF_IERR_INVALID_BPB;
}

static s32 PFFAT_ReadFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value, struct PF_CACHE_PAGE* p_page) {
    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            return PFFAT12_ReadFATEntryWithBuf(p_vol, cluster, p_value, p_page);
        case FAT_16:
            return PFFAT16_ReadFATEntryWithBuf(p_vol, cluster, p_value, p_page);
        case FAT_32:
            return PFFAT32_ReadFATEntryWithBuf(p_vol, cluster, p_value, p_page);
        default:
            break;
    }
    return PF_IERR_INVALID_BPB;
}

static s32 PFFAT_WriteFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32 value) {
    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            if (value > PF_FAT12_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT12_WriteFATEntry(p_vol, cluster, value);
        case FAT_16:
            if (value > PF_FAT16_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT16_WriteFATEntry(p_vol, cluster, value);
        case FAT_32:
            if (value > PF_FAT32_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT32_WriteFATEntry(p_vol, cluster, value);
        default:
            break;
    }
    return PF_IERR_INVALID_BPB;
}

static s32 PFFAT_WriteFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32 value, struct PF_CACHE_PAGE* p_page) {
    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            if (value > PF_FAT12_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT12_WriteFATEntryWithBuf(p_vol, cluster, value, p_page);
        case FAT_16:
            if (value > PF_FAT16_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT16_WriteFATEntryWithBuf(p_vol, cluster, value, p_page);
        case FAT_32:
            if (value > PF_FAT32_ENTRY_MASK) {
                return PF_IERR_OUT_OF_RANGE;
            }
            return PFFAT32_WriteFATEntryWithBuf(p_vol, cluster, value, p_page);
        default:
            break;
    }
    return PF_IERR_INVALID_BPB;
}

s32 PFFAT_ReadFATSector(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page, u32 cluster) {
    s32 err;
    u32 offset;
    u32 current_fat;
    s32 result;

    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            offset = (u16)(cluster + (cluster >> 1));
            break;
        case FAT_16:
            offset = cluster * sizeof(u16);
            break;
        case FAT_32:
            offset = cluster * sizeof(u32);
            break;
        default:
            return PF_IERR_INVALID_BPB;
    }

    p_page->sector = (u16)(p_vol->bpb.active_FAT_sector + (offset >> p_vol->bpb.log2_bytes_per_sector));

    if ((p_vol->bpb.ext_flags & PF_EXT_FLAG_NO_MIRRORING) == 0) {
        current_fat = 1;
    } else {
        current_fat = p_vol->bpb.ext_flags & PF_EXT_FLAG_ACTIVE_FAT_MASK;
    }

    do {
        err = PFSEC_ReadFAT(p_vol, p_page->p_buf, p_page->sector, 0, p_vol->bpb.bytes_per_sector);
        if (err == PF_IERR_DRIVER && p_vol->p_callback != NULL) {
            result = p_vol->p_callback(p_vol->last_driver_error);
            if (result == PF_FAT_READ_RETRY) {
                continue;
            }
            if (result == PF_FAT_READ_NEXT_FAT && p_vol->bpb.num_active_FATs >= 2 && current_fat < p_vol->bpb.num_active_FATs) {
                current_fat++;
                p_page->sector += p_vol->bpb.sectors_per_FAT;
                continue;
            }
        }

        if (err != 0) {
            return err;
        }
    } while (err != 0);

    return err;
}

s32 PFFAT_SearchForNumFreeClusters(struct PF_VOLUME* p_vol, u32 start_cluster, u32 end_cluster, u32 num_cluster, u32* p_start_free_cluster, u32* p_last_free_cluster) {
    u32 upper_bound_cluster;
    u32 fat_entry;
    u32 success_num;
    u32 save_start_cluster;
    u32 save_success_num;
    s32 err;
    u32 search_flg;
    u32 temp_start_cluster;
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;

    search_flg = FALSE;
    *p_start_free_cluster = PF_INVALID_CLUSTER;
    *p_last_free_cluster = PF_INVALID_CLUSTER;
    success_num = 0;
    save_start_cluster = PF_INVALID_CLUSTER;
    save_success_num = 0;
    upper_bound_cluster = p_vol->bpb.num_clusters + PF_CLUSTER_FIRST;

    if (start_cluster < PF_CLUSTER_FIRST || start_cluster >= upper_bound_cluster) {
        start_cluster = PF_CLUSTER_FIRST;
    }

    if (end_cluster < PF_CLUSTER_FIRST || end_cluster >= upper_bound_cluster) {
        end_cluster = upper_bound_cluster - 1;
    }

    temp_start_cluster = start_cluster;
    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }
    err = PFFAT_ReadFATSector(p_vol, p_page, start_cluster);
    if (err) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    p_page->option = PF_CACHE_PAGE_OPT_NONE;

    while (start_cluster <= end_cluster) {
        if (upper_bound_cluster <= start_cluster) {
            start_cluster = PF_CLUSTER_FIRST;
        }
        err = PFFAT_ReadFATEntryWithBuf(p_vol, start_cluster, &fat_entry, p_page);
        if (err) {
            return err;
        }

        if (fat_entry == PF_CLUSTER_FREE) {
            if (*p_start_free_cluster == PF_INVALID_CLUSTER) {
                *p_start_free_cluster = start_cluster;
            }
            success_num++;
            if (success_num >= num_cluster) {
                *p_last_free_cluster = start_cluster;
                PFCACHE_FreeDataPage(p_vol, p_page);
                return 0;
            }
        } else {
            if (save_success_num < success_num) {
                save_start_cluster = *p_start_free_cluster;
                save_success_num = success_num;
            }
            *p_start_free_cluster = PF_INVALID_CLUSTER;
            success_num = 0;
        }

        start_cluster++;
        if (search_flg == FALSE && success_num == 0 && start_cluster > PF_CLUSTER_FIRST && start_cluster == end_cluster) {
            end_cluster = temp_start_cluster;
            start_cluster = PF_CLUSTER_FIRST;
            search_flg = TRUE;
        }
    }

    if (save_success_num < success_num) {
        save_start_cluster = *p_start_free_cluster;
        save_success_num = success_num;
    }

    if (save_success_num != 0) {
        *p_start_free_cluster = save_start_cluster;
        *p_last_free_cluster = save_success_num + save_start_cluster - 1;
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    return 0;
}

s32 PFFAT_ResetCluster(struct PF_VOLUME* p_vol, u32 cluster) {
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;
    u32 success_size;
    u32 sector;
    u32 i;
    s32 err;

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }

    sector = p_vol->bpb.first_data_sector + ((cluster - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);
    for (i = 0; i < p_vol->bpb.sectors_per_cluster; i++) {
        if (PFSEC_ReadData(p_vol, p_page->p_buf, sector + i, 0, p_vol->bpb.bytes_per_sector, &success_size, TRUE) != 0 ||
            success_size != p_vol->bpb.bytes_per_sector) {
            break;
        }
        pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);
        if (PFSEC_WriteData(p_vol, p_page->p_buf, sector + i, 0, p_vol->bpb.bytes_per_sector, &success_size, TRUE) != 0 ||
            success_size != p_vol->bpb.bytes_per_sector) {
            break;
        }
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    return 0;
}

static s32 PFFAT_UpdateClusterLink(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index) {
    if (p_ffd->cluster_link.max_count > p_ffd->cluster_link.position && p_ffd->cluster_link.max_count) {
        if (chain_index == p_ffd->cluster_link.position * (p_ffd->cluster_link.interval + 1)) {
            p_ffd->cluster_link.interval_offset = 0;
            p_ffd->cluster_link.buffer[p_ffd->cluster_link.position] = cluster;
            ++p_ffd->cluster_link.position;
        } else {
            ++p_ffd->cluster_link.interval_offset;
        }
        p_ffd->cluster_link.save_index = chain_index;
    }
    return 0;
}

static s32 PFFAT_ClearClusterLink(struct PF_FFD* p_ffd, u32 chain_index) {
    u32 position;
    u32 offset;

    position = chain_index / (p_ffd->cluster_link.interval + 1);
    if (p_ffd->cluster_link.max_count >= position) {
        offset = chain_index % (p_ffd->cluster_link.interval + 1);
        if (offset == 0) {
            p_ffd->cluster_link.buffer[position] = 0;
        }
        if (p_ffd->cluster_link.save_index >= chain_index) {
            p_ffd->cluster_link.position = (chain_index - 1) / (p_ffd->cluster_link.interval + 1);
            p_ffd->cluster_link.interval_offset = (chain_index - 1) % (p_ffd->cluster_link.interval + 1);
            p_ffd->cluster_link.save_index = chain_index - 1;
        }
    }
    return 0;
}

s32 PFFAT_FindClusterLink(struct PF_FFD* p_ffd, u32 chain_index, u32* p_cluster, u32* is_found) {
    u32 current_cluster;
    u32 next_cluster;
    u32 position;
    u32 offset;
    u32 i;
    s32 err;

    next_cluster = PF_INVALID_CLUSTER;
    *is_found = FALSE;
    if (p_ffd->cluster_link.position == 0) {
        return 0;
    }
    if (p_ffd->cluster_link.save_index >= chain_index) {
        position = chain_index / (p_ffd->cluster_link.interval + 1);
        offset = chain_index % (p_ffd->cluster_link.interval + 1);

        if (offset == 0) {
            *p_cluster = p_ffd->cluster_link.buffer[position];
            *is_found = TRUE;
        } else {
            current_cluster = p_ffd->cluster_link.buffer[position];
            for (i = offset; i != 0; i--) {
                err = PFFAT_ReadFATEntry(p_ffd->p_vol, current_cluster, &next_cluster);
                if (err) {
                    return err;
                }
                if (next_cluster == PF_CLUSTER_FREE) {
                    return PF_IERR_BROKEN_CHAIN;
                }
                current_cluster = next_cluster;
            }
            if (next_cluster == PF_CLUSTER_FREE) {
                return PF_IERR_BROKEN_CHAIN;
            }
            if (next_cluster == fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
                return 0;
            }
            *p_cluster = next_cluster;
            *is_found = TRUE;
        }
    } else {
        return 0;
    }
    return 0;
}

s32 PFFAT_FindClusterLinkWithBuf(struct PF_FFD* p_ffd, u32 chain_index, u32* p_cluster, u32* is_found, struct PF_CACHE_PAGE* p_page) {
    u32 current_cluster;
    u32 next_cluster;
    u32 position;
    u32 offset;
    u32 i;
    s32 err;

    next_cluster = PF_INVALID_CLUSTER;
    *is_found = FALSE;
    if (p_ffd->cluster_link.position == 0) {
        return 0;
    }
    if (p_ffd->cluster_link.save_index >= chain_index) {
        position = chain_index / (p_ffd->cluster_link.interval + 1);
        offset = chain_index % (p_ffd->cluster_link.interval + 1);

        if (offset == 0) {
            *p_cluster = p_ffd->cluster_link.buffer[position];
            *is_found = TRUE;
        } else {
            current_cluster = p_ffd->cluster_link.buffer[position];
            for (i = offset; i != 0; i--) {
                err = PFFAT_ReadFATEntryWithBuf(p_ffd->p_vol, current_cluster, &next_cluster, p_page);
                if (err) {
                    return err;
                }
                if (next_cluster == PF_CLUSTER_FREE) {
                    return PF_IERR_BROKEN_CHAIN;
                }
                current_cluster = next_cluster;
            }
            if (next_cluster == PF_CLUSTER_FREE) {
                return PF_IERR_BROKEN_CHAIN;
            }
            if (next_cluster == fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
                return 0;
            }
            *p_cluster = next_cluster;
            *is_found = TRUE;
        }
    } else {
        return 0;
    }
    return 0;
}

s32 PFFAT_ReadCluster(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32* next_cluster, u32* chk_clstlnk) {
    s32 err;

    *next_cluster = PF_INVALID_CLUSTER;
    if (p_ffd->cluster_link.buffer != NULL && *chk_clstlnk == TRUE) {
        err = PFFAT_FindClusterLink(p_ffd, chain_index, next_cluster, chk_clstlnk);
        if (err != 0) {
            return err;
        }
    }

    if (p_ffd->cluster_link.buffer == NULL || *chk_clstlnk == FALSE) {
        err = PFFAT_ReadFATEntry(p_ffd->p_vol, cluster, next_cluster);
        if (err) {
            return err;
        }
        if (*next_cluster == PF_CLUSTER_FREE) {
            return PF_IERR_BROKEN_CHAIN;
        }
        if (*next_cluster == PF_INVALID_CLUSTER) {
            return PF_IERR_NO_SPACE;
        }

        if (p_ffd->cluster_link.buffer != NULL && *next_cluster != fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
            PFFAT_UpdateClusterLink(p_ffd, *next_cluster, chain_index);
        }
    }
    return 0;
}

s32 PFFAT_ReadClusterWithBuf(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32* next_cluster, u32* chk_clstlnk, struct PF_CACHE_PAGE* p_page) {
    s32 err;

    *next_cluster = PF_INVALID_CLUSTER;
    if (p_ffd->cluster_link.buffer != NULL && *chk_clstlnk == TRUE) {
        err = PFFAT_FindClusterLinkWithBuf(p_ffd, chain_index, next_cluster, chk_clstlnk, p_page);
        if (err != 0) {
            return err;
        }
    }

    if (p_ffd->cluster_link.buffer == NULL || *chk_clstlnk == FALSE) {
        err = PFFAT_ReadFATEntryWithBuf(p_ffd->p_vol, cluster, next_cluster, p_page);
        if (err) {
            return err;
        }
        if (*next_cluster == PF_CLUSTER_FREE) {
            return PF_IERR_BROKEN_CHAIN;
        }
        if (*next_cluster == PF_INVALID_CLUSTER) {
            return PF_IERR_NO_SPACE;
        }

        if (p_ffd->cluster_link.buffer != NULL && *next_cluster != fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
            PFFAT_UpdateClusterLink(p_ffd, *next_cluster, chain_index);
        }
    }
    return 0;
}

s32 PFFAT_WriteCluster(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32 next_cluster, u32 use_clstlnk) {
    s32 err;
    u32 access_cluster;

    err = 0;
    err = PFFAT_WriteFATEntry(p_ffd->p_vol, cluster, next_cluster);
    if (err) {
        return err;
    }
    if (p_ffd->cluster_link.buffer != NULL && use_clstlnk == TRUE) {
        if (next_cluster == PF_CLUSTER_FREE) {
            PFFAT_ClearClusterLink(p_ffd, chain_index);
        } else if (cluster != fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
            err = PFFAT_FindClusterLink(p_ffd, chain_index, &access_cluster, &use_clstlnk);
            if (err) {
                return err;
            }
            if (!use_clstlnk) {
                PFFAT_UpdateClusterLink(p_ffd, cluster, chain_index);
            }
        }
    }
    return 0;
}

s32 PFFAT_WriteClusterWithBuf(struct PF_FFD* p_ffd, u32 cluster, u32 chain_index, u32 next_cluster, u32 use_clstlnk, struct PF_CACHE_PAGE* p_page) {
    s32 err;
    u32 access_cluster;

    err = 0;
    err = PFFAT_WriteFATEntryWithBuf(p_ffd->p_vol, cluster, next_cluster, p_page);
    if (err) {
        return err;
    }
    if (p_ffd->cluster_link.buffer != NULL && use_clstlnk == TRUE) {
        if (next_cluster == PF_CLUSTER_FREE) {
            PFFAT_ClearClusterLink(p_ffd, chain_index);
        } else if (cluster != fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2) {
            err = PFFAT_FindClusterLinkWithBuf(p_ffd, chain_index, &access_cluster, &use_clstlnk, p_page);
            if (err) {
                return err;
            }
            if (!use_clstlnk) {
                PFFAT_UpdateClusterLink(p_ffd, cluster, chain_index);
            }
        }
    }
    return 0;
}

static s32 PFFAT_UpdateFATEntry(struct PF_VOLUME* p_vol, struct PF_CACHE_PAGE* p_page) {
    s32 err;
    u16 i;
    u32 sector;
    s32 first_err;

    first_err = 0;
    sector = p_page->sector;
    for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
        err = PFSEC_WriteFAT(p_vol, p_page->p_buf, sector, 0, p_vol->bpb.bytes_per_sector);
        if (err != 0 && first_err == 0) {
            first_err = err;
        }
        sector += p_vol->bpb.sectors_per_FAT;
    }
    return first_err;
}

s32 PFFAT_DoAllocateChain(struct PF_FFD* p_ffd, u32 chain_len, u32 chain_index, u32* p_chain_start, u32* p_last_allocated, u32 is_reset) {
    struct PF_VOLUME* p_vol;
    u32 eoc2;
    u32 first_allocated_cluster;
    u32 last_allocated_cluster;
    s32 err;
    u32 start_cluster;
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;

    p_vol = p_ffd->p_vol;
    eoc2 = fat_special_values[p_vol->bpb.fat_type].eoc2;
    *p_chain_start = PF_INVALID_CLUSTER;
    *p_last_allocated = PF_INVALID_CLUSTER;
    last_allocated_cluster = PF_INVALID_CLUSTER;

    err = PFFAT_SearchForNumFreeClusters(p_vol, p_vol->last_free_cluster, PF_FAT_SEARCH_TO_LAST_CLUSTER, chain_len, &first_allocated_cluster, &last_allocated_cluster);
    if (err) {
        return err;
    }
    if (first_allocated_cluster == PF_INVALID_CLUSTER) {
        return 0;
    }

    if (is_reset) {
        err = PFFAT_ResetCluster(p_vol, first_allocated_cluster);
        if (err) {
            return err;
        }
    }

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }
    err = PFFAT_ReadFATSector(p_vol, p_page, first_allocated_cluster);
    if (err) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }

    p_page->option = PF_CACHE_PAGE_OPT_NONE;
    for (start_cluster = first_allocated_cluster; start_cluster < last_allocated_cluster; start_cluster++) {
        err = PFFAT_WriteClusterWithBuf(p_ffd, start_cluster, chain_index, start_cluster + 1, TRUE, p_page);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if ((p_vol->fsi_flag & PF_FSI_FLAG_FREE_COUNT_VALID) && p_vol->num_free_clusters != PDM_FSI_UNKNOWN && p_vol->num_free_clusters != 0) {
            if (p_vol->bpb.fat_type != FAT_32 || last_allocated_cluster != p_vol->bpb.root_dir_cluster) {
                p_vol->num_free_clusters--;
            }
        }
        if (*p_chain_start == PF_INVALID_CLUSTER) {
            *p_chain_start = start_cluster;
        }
        *p_last_allocated = start_cluster;
        chain_index++;
    }

    if (last_allocated_cluster != PF_INVALID_CLUSTER) {
        err = PFFAT_WriteClusterWithBuf(p_ffd, last_allocated_cluster, chain_index, eoc2, TRUE, p_page);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if (*p_chain_start == PF_INVALID_CLUSTER) {
            *p_chain_start = start_cluster;
        }
        *p_last_allocated = start_cluster;
        if (p_vol->num_free_clusters != PDM_FSI_UNKNOWN && p_vol->num_free_clusters != 0) {
            if (p_vol->bpb.fat_type != FAT_32 || last_allocated_cluster != p_vol->bpb.root_dir_cluster) {
                p_vol->num_free_clusters--;
            }
        }
    }

    p_vol->last_free_cluster = last_allocated_cluster + 1;
    p_ffd->last_cluster.num_last_cluster = last_allocated_cluster;
    p_ffd->last_cluster.max_chain_index = chain_index;
    *p_last_allocated = last_allocated_cluster;

    err = PFFAT_UpdateFATEntry(p_vol, p_page);
    if (err != 0) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    PFCACHE_FreeDataPage(p_vol, p_page);

    if (p_vol->bpb.fat_type == FAT_32 && (p_vol->fsi_flag & PF_FSI_FLAG_UPDATE)) {
        if (PFFAT_RefreshFSINFO(p_vol)) {
            p_vol->num_free_clusters = PDM_FSI_UNKNOWN;
            p_vol->fsi_flag &= ~PF_FSI_FLAG_FREE_COUNT_VALID;
        }
    }
    return 0;
}

static s32 PFFAT_AllocateChain(struct PF_FFD* p_ffd, u32 chain_len, u32 chain_index, u32* p_chain_start, u32* p_last_allocated) {
    s32 err;

    err = PFFAT_DoAllocateChain(p_ffd, chain_len, chain_index, p_chain_start, p_last_allocated, FALSE);
    if (err) {
        if (*p_chain_start != PF_INVALID_CLUSTER) {
            PFFAT_FreeChain(p_ffd, *p_chain_start, chain_index, PF_FAT_WHOLE_CHAIN);
        }
        *p_chain_start = PF_INVALID_CLUSTER;
        *p_last_allocated = PF_INVALID_CLUSTER;
        return err;
    }
    return 0;
}

s32 PFFAT_GetClusterInChain(struct PF_FFD* p_ffd, u32 chain_index, u32 mode, u32 num_cluster, u32* locate_start, u32* locate_end) {
    s32 err;
    u32 bad;
    u32 eoc1;
    u32 current_cluster;
    u32 next_cluster;
    u32 trace_cnt;
    u32 append_cnt;
    u32 search_index;
    u32* p_chain_start;
    u32 chain_start;
    u32 is_hit;
    u32 check_use;
    struct PF_CACHE_PAGE* p_page;

    bad = fat_special_values[p_ffd->p_vol->bpb.fat_type].bad;
    eoc1 = fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc1;
    *locate_end = PF_INVALID_CLUSTER;
    *locate_start = PF_INVALID_CLUSTER;

    if (p_ffd->last_access_cluster.chain_index != 0 && p_ffd->last_access_cluster.chain_index <= chain_index) {
        chain_start = p_ffd->last_access_cluster.cluster;
        p_chain_start = &chain_start;
        trace_cnt = chain_index - p_ffd->last_access_cluster.chain_index;
        search_index = p_ffd->last_access_cluster.chain_index + 1;
    } else {
        p_chain_start = p_ffd->p_start_cluster;
        trace_cnt = chain_index;
        search_index = 1;
    }

    current_cluster = *p_chain_start;
    next_cluster = *p_chain_start;

    if (mode == PF_FAT_MODE_APPEND) {
        if (trace_cnt != 0 && num_cluster != 0) {
            trace_cnt += num_cluster - 1;
        } else {
            trace_cnt = num_cluster;
        }
        append_cnt = trace_cnt;
        if (p_ffd->last_cluster.num_last_cluster != 0) {
            trace_cnt = 0;
            append_cnt = p_ffd->last_cluster.max_chain_index + num_cluster - chain_index;
            current_cluster = p_ffd->last_cluster.num_last_cluster;
            next_cluster = fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2;
            search_index = p_ffd->last_cluster.max_chain_index + 1;
        }
    } else {
        if (mode == PF_FAT_MODE_ALLOCATE && current_cluster == 0) {
            trace_cnt++;
        }
        append_cnt = trace_cnt;
    }

    if (trace_cnt != 0) {
        err = PFCACHE_AllocateDataPage(p_ffd->p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
        if (err) {
            return err;
        }
        err = PFFAT_ReadFATSector(p_ffd->p_vol, p_page, current_cluster);
        if (err) {
            PFCACHE_FreeDataPage(p_ffd->p_vol, p_page);
            return err;
        }
        p_page->option = PF_CACHE_PAGE_OPT_NONE;
        check_use = TRUE;

        while (trace_cnt-- != 0 && current_cluster < bad) {
            if (current_cluster != 0) {
                err = PFFAT_ReadClusterWithBuf(p_ffd, current_cluster, search_index, &next_cluster, &check_use, p_page);
                if (err != 0 && next_cluster != PF_INVALID_CLUSTER) {
                    PFCACHE_FreeDataPage(p_ffd->p_vol, p_page);
                    return err;
                }
            } else {
                search_index--;
                next_cluster = fat_special_values[p_ffd->p_vol->bpb.fat_type].eoc2;
            }

            if ((next_cluster < PF_CLUSTER_FIRST || next_cluster >= (p_ffd->p_vol->bpb.num_clusters + PF_CLUSTER_FIRST)) && next_cluster < eoc1) {
                PFCACHE_FreeDataPage(p_ffd->p_vol, p_page);
                return PF_IERR_INVALID_CLUSTER;
            }

            if (mode != PF_FAT_MODE_SEARCH && next_cluster >= eoc1) {
                break;
            }

            current_cluster = next_cluster;
            search_index++;
        }
        append_cnt = trace_cnt;
        PFCACHE_FreeDataPage(p_ffd->p_vol, p_page);
    }

    if (mode != PF_FAT_MODE_SEARCH && next_cluster >= eoc1) {
        *locate_start = 0;
        err = PFFAT_AllocateChain(p_ffd, append_cnt + 1, search_index, locate_start, &next_cluster);
        if (err) {
            return err;
        }
        if (next_cluster == PF_INVALID_CLUSTER) {
            return PF_IERR_NO_SPACE;
        }
        if (*p_chain_start == 0) {
            *p_chain_start = *locate_start;
        } else {
            err = PFFAT_WriteCluster(p_ffd, current_cluster, search_index - 1, *locate_start, TRUE);
            if (err) {
                return err;
            }
        }
        current_cluster = next_cluster;
    }

    *locate_end = current_cluster;
    return 0;
}

static s32 PFFAT_GetNumberOfCluster(struct PF_FFD* p_ffd, u32 chain_index, u32 may_allocate, u32* p_cluster) {
    s32 err;
    u32 locate_start;
    u32 locate_end;
    u32 mode;

    *p_cluster = PF_INVALID_CLUSTER;
    if (may_allocate) {
        mode = PF_FAT_MODE_ALLOCATE;
    } else {
        mode = PF_FAT_MODE_SEARCH;
    }
    err = PFFAT_GetClusterInChain(p_ffd, chain_index, mode, 0, &locate_start, &locate_end);
    if (err) {
        return err;
    }
    if (locate_end >= PF_CLUSTER_FIRST && locate_end < p_ffd->p_vol->bpb.num_clusters + PF_CLUSTER_FIRST) {
        *p_cluster = locate_end;
    }
    return 0;
}

static s32 PFFAT_GetNumberOfAllocatedCluster(struct PF_FFD* p_ffd, u32 chain_index, u32 num_cluster, u32* p_cluster) {
    s32 err;
    u32 locate_start;
    u32 locate_end;

    *p_cluster = PF_INVALID_CLUSTER;
    err = PFFAT_GetClusterInChain(p_ffd, chain_index, PF_FAT_MODE_APPEND, num_cluster, &locate_start, &locate_end);
    if (err) {
        return err;
    }
    if (locate_start >= PF_CLUSTER_FIRST && locate_start < p_ffd->p_vol->bpb.num_clusters + PF_CLUSTER_FIRST) {
        *p_cluster = locate_start;
    }
    return 0;
}

s32 PFFAT_GetClusterContinuousSectorInChain(struct PF_FFD* p_ffd, u32 initial_cluster, u32 chain_index, u32 size, u32* p_num_sector) {
    struct PF_VOLUME* p_vol;
    u32 cluster;
    u32 next_cluster;
    u32 check_use;
    s32 err;

    p_vol = p_ffd->p_vol;
    next_cluster = PF_INVALID_CLUSTER;
    check_use = TRUE;
    for (cluster = initial_cluster; next_cluster != 0; cluster = next_cluster, chain_index++) {
        err = PFFAT_ReadCluster(p_ffd, cluster, chain_index, &next_cluster, &check_use);
        if (err) {
            return err;
        }
        if (cluster + 1 != next_cluster) {
            if (next_cluster == fat_special_values[p_vol->bpb.fat_type].eoc2) {
                p_ffd->last_cluster.num_last_cluster = cluster;
                p_ffd->last_cluster.max_chain_index = chain_index - 1;
            }
            break;
        }
        ++p_ffd->p_hint->chain_index;
        p_ffd->p_hint->cluster = next_cluster;
        *p_num_sector += p_vol->bpb.sectors_per_cluster;
        if (*p_num_sector << p_vol->bpb.log2_bytes_per_sector >= size) {
            break;
        }
    }
    return 0;
}

s32 PFFAT_GetClusterAllocatedInChain(struct PF_FFD* p_ffd, u32 initial_cluster, u32 chain_index, u32 size, u32* p_num_clusters) {
    struct PF_VOLUME* p_vol;
    u32 cluster;
    u32 next_cluster;
    u32 total_size;
    u32 check_use;
    s32 err;

    p_vol = p_ffd->p_vol;
    next_cluster = PF_INVALID_CLUSTER;
    check_use = TRUE;
    total_size = 0;
    for (cluster = initial_cluster; next_cluster != 0; cluster = next_cluster, chain_index++) {
        total_size += p_vol->bpb.bytes_per_sector * p_vol->bpb.sectors_per_cluster;
        (*p_num_clusters)++;
        if (total_size >= size) {
            break;
        }
        err = PFFAT_ReadCluster(p_ffd, cluster, chain_index, &next_cluster, &check_use);
        if (err) {
            return err;
        }
        if (next_cluster == fat_special_values[p_vol->bpb.fat_type].eoc2) {
            p_ffd->last_cluster.num_last_cluster = cluster;
            p_ffd->last_cluster.max_chain_index = chain_index - 1;
            break;
        }
    }
    return 0;
}

s32 PFFAT_GetClusterSpecified(struct PF_FFD* p_ffd, u32 chain_index, u32 may_allocate, u32* p_cluster) {
    struct PF_VOLUME* p_vol;
    struct PF_FAT_HINT* p_hint;
    PF_FAT_TYPE fat_type;
    u32 is_found;
    s32 err;

    p_vol = p_ffd->p_vol;
    fat_type = p_vol->bpb.fat_type;
    p_hint = p_ffd->p_hint;

    if (*p_ffd->p_start_cluster == 0 && !may_allocate) {
        *p_cluster = PF_INVALID_CLUSTER;
        return 0;
    }

    if (*p_ffd->p_start_cluster == PF_CLUSTER_ROOT_DIR) {
        if ((u32)fat_type <= FAT_16) {
            *p_cluster = PF_INVALID_CLUSTER;
            return 0;
        }
        *p_ffd->p_start_cluster = p_vol->bpb.root_dir_cluster;
    }

    if (p_ffd->cluster_link.buffer != NULL && !may_allocate) {
        err = PFFAT_FindClusterLink(p_ffd, chain_index, p_cluster, &is_found);
        if (err) {
            return err;
        }
        if (is_found == TRUE) {
            p_hint->file_version = p_ffd->file_version;
            p_hint->chain_index = chain_index;
            p_hint->cluster = *p_cluster;
            return 0;
        }
    }

    err = PFFAT_GetNumberOfCluster(p_ffd, chain_index, may_allocate, p_cluster);
    if (err != 0) {
        return err;
    }

    if (*p_cluster == PF_INVALID_CLUSTER) {
        return 0;
    }

    p_hint->file_version = p_ffd->file_version;
    p_hint->chain_index = chain_index;
    p_hint->cluster = *p_cluster;
    return 0;
}

s32 PFFAT_GetClusterAllocated(struct PF_FFD* p_ffd, u32 chain_index, u32 num_cluster, u32* p_cluster) {
    struct PF_VOLUME* p_vol;
    struct PF_FAT_HINT* p_hint;
    PF_FAT_TYPE fat_type;
    s32 err;

    p_vol = p_ffd->p_vol;
    fat_type = p_vol->bpb.fat_type;
    p_hint = p_ffd->p_hint;

    if (*p_ffd->p_start_cluster == PF_CLUSTER_ROOT_DIR) {
        if ((u32)fat_type <= FAT_16) {
            *p_cluster = PF_INVALID_CLUSTER;
            return 0;
        }
        *p_ffd->p_start_cluster = p_vol->bpb.root_dir_cluster;
    }
    err = PFFAT_GetNumberOfAllocatedCluster(p_ffd, chain_index, num_cluster, p_cluster);
    if (err) {
        return err;
    }

    if (*p_cluster == PF_INVALID_CLUSTER) {
        return 0;
    }
    p_hint->file_version = p_ffd->file_version;
    p_hint->chain_index = chain_index;
    p_hint->cluster = *p_cluster;
    return 0;
}

static s32 PFFAT_GetSectorInRootDirRegion(struct PF_VOLUME* p_vol, u32 sector_offset, u32* p_sector) {
    if (sector_offset >= p_vol->bpb.num_root_dir_sectors) {
        *p_sector = PF_INVALID_SECTOR;
        return 0;
    } else {
        *p_sector = p_vol->bpb.first_root_dir_sector + sector_offset;
        return 0;
    }
}

s32 PFFAT_GetSector(struct PF_FFD* p_ffd, u32 file_sector_index, u32 mode, u32 size, u32* p_sector) {
    s32 err;
    struct PF_VOLUME* p_vol;
    PF_FAT_TYPE fat_type;
    u32 chain_index;
    u32 cluster;
    u32 num_divide;
    u32 num_remaind;
    u32 num_cluster;

    p_vol = p_ffd->p_vol;
    fat_type = p_vol->bpb.fat_type;

    if (*p_ffd->p_start_cluster == PF_CLUSTER_ROOT_DIR && (u32)fat_type <= FAT_16) {
        err = PFFAT_GetSectorInRootDirRegion(p_vol, file_sector_index, p_sector);
        if (err) {
            return err;
        }
        return 0;
    }

    chain_index = file_sector_index >> p_vol->bpb.log2_sectors_per_cluster;
    if (mode == PF_FAT_MODE_APPEND) {
        num_divide = size / p_vol->bpb.bytes_per_sector;
        num_remaind = (size - num_divide * p_vol->bpb.bytes_per_sector) != 0;
        num_cluster = (num_divide + num_remaind) / p_vol->bpb.sectors_per_cluster;
        num_cluster += ((num_divide + num_remaind) % p_vol->bpb.sectors_per_cluster) != 0;
        err = PFFAT_GetClusterAllocated(p_ffd, chain_index, num_cluster, &cluster);
        if (err) {
            return err;
        }
    } else {
        err = PFFAT_GetClusterSpecified(p_ffd, chain_index, mode, &cluster);
        if (err) {
            return err;
        }
    }

    if (cluster == PF_INVALID_CLUSTER) {
        *p_sector = PF_INVALID_SECTOR;
        return 0;
    }
    *p_sector = p_vol->bpb.first_data_sector + ((cluster - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster) + (file_sector_index & (p_vol->bpb.sectors_per_cluster - 1));
    return 0;
}

s32 PFFAT_GetSectorSpecified(struct PF_FFD* p_ffd, u32 file_sector_index, u32 may_allocate, u32* p_sector) {
    s32 err;
    u32 mode;

    if (may_allocate) {
        mode = PF_FAT_MODE_ALLOCATE;
    } else {
        mode = PF_FAT_MODE_SEARCH;
    }
    err = PFFAT_GetSector(p_ffd, file_sector_index, mode, 0, p_sector);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFFAT_GetSectorAllocated(struct PF_FFD* p_ffd, u32 file_sector_index, u32 size, u32* p_sector) {
    s32 err;

    err = PFFAT_GetSector(p_ffd, file_sector_index, PF_FAT_MODE_APPEND, size, p_sector);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFFAT_getContinuousSector(struct PF_FFD* p_ffd, u32 file_sector_index, u32 size, u32* p_sector, u32* p_num_sector) {
    struct PF_VOLUME* p_vol;
    u32 chain_index;
    u32 cluster;
    s32 err;

    cluster = PF_INVALID_CLUSTER;
    p_vol = p_ffd->p_vol;

    if (*p_sector != PF_INVALID_SECTOR) {
        cluster = ((*p_sector - p_vol->bpb.first_data_sector) >> p_vol->bpb.log2_sectors_per_cluster) + PF_CLUSTER_FIRST;
    } else {
        err = PFFAT_GetSectorSpecified(p_ffd, file_sector_index, FALSE, p_sector);
        if (err) {
            return err;
        }
        if (*p_sector != PF_INVALID_SECTOR) {
            cluster = ((*p_sector - p_vol->bpb.first_data_sector) >> p_vol->bpb.log2_sectors_per_cluster) + PF_CLUSTER_FIRST;
        }
    }

    if (*p_sector != PF_INVALID_SECTOR) {
        *p_num_sector = p_vol->bpb.first_data_sector + ((cluster + 1 - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster) - *p_sector;
    } else {
        *p_num_sector = 0;
    }

    if (*p_sector != PF_INVALID_SECTOR) {
        if ((*p_num_sector << p_vol->bpb.log2_bytes_per_sector) < size) {
            chain_index = (file_sector_index >> p_vol->bpb.log2_sectors_per_cluster) + 1;
            err = PFFAT_GetClusterContinuousSectorInChain(p_ffd, cluster, chain_index, size, p_num_sector);
            if (err) {
                return err;
            }
        }
    }

    return 0;
}

s32 PFFAT_CountAllocatedClusters(struct PF_FFD* p_ffd, u32 size, u32* p_num_alloc_clusters) {
    u32 cluster;
    u32 total_size;
    u32 chain_index;
    s32 err;

    total_size = size;
    *p_num_alloc_clusters = 0;
    if (*p_ffd->p_start_cluster < PF_CLUSTER_FIRST) {
        return 0;
    }
    if (p_ffd->last_cluster.num_last_cluster) {
        *p_num_alloc_clusters = p_ffd->last_cluster.max_chain_index + 1;
        return 0;
    }
    if (p_ffd->p_hint->cluster) {
        if (size <= (p_ffd->p_hint->chain_index + 1) * (p_ffd->p_vol->bpb.bytes_per_sector * p_ffd->p_vol->bpb.sectors_per_cluster)) {
            *p_num_alloc_clusters = p_ffd->p_hint->chain_index + 1;
            return 0;
        }
        *p_num_alloc_clusters = p_ffd->p_hint->chain_index;
        cluster = p_ffd->p_hint->cluster;
        chain_index = p_ffd->p_hint->chain_index + 1;
        total_size = size - p_ffd->p_hint->chain_index * (p_ffd->p_vol->bpb.bytes_per_sector * p_ffd->p_vol->bpb.sectors_per_cluster);
    } else {
        cluster = *p_ffd->p_start_cluster;
        chain_index = 1;
    }
    err = PFFAT_GetClusterAllocatedInChain(p_ffd, cluster, chain_index, total_size, p_num_alloc_clusters);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFFAT_CountFreeClusters(struct PF_VOLUME* p_vol, u32* p_num_free_clusters) {
    u32 free_cluster;
    s32 err;
    u32 next_cluster;
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;

    if ((p_vol->fsi_flag & PF_FSI_FLAG_FREE_COUNT_VALID) && p_vol->num_free_clusters != PDM_FSI_UNKNOWN) {
        *p_num_free_clusters = p_vol->num_free_clusters;
        return 0;
    }
    *p_num_free_clusters = 0;
    free_cluster = PF_CLUSTER_FIRST;
    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }
    err = PFFAT_ReadFATSector(p_vol, p_page, PF_CLUSTER_FIRST);
    if (err) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    p_page->option = PF_CACHE_PAGE_OPT_NONE;
    while (free_cluster >= PF_CLUSTER_FIRST && free_cluster < p_vol->bpb.num_clusters + PF_CLUSTER_FIRST) {
        err = PFFAT_ReadFATEntryWithBuf(p_vol, free_cluster, &next_cluster, p_page);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if (next_cluster == PF_CLUSTER_FREE) {
            (*p_num_free_clusters)++;
        }
        free_cluster++;
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    p_vol->fsi_flag |= PF_FSI_FLAG_FREE_COUNT_VALID;
    p_vol->num_free_clusters = *p_num_free_clusters;
    if (p_vol->bpb.fat_type == FAT_32 && (p_vol->fsi_flag & PF_FSI_FLAG_UPDATE)) {
        if (PFFAT_RefreshFSINFO(p_vol)) {
            p_vol->num_free_clusters = PDM_FSI_UNKNOWN;
            p_vol->fsi_flag &= ~PF_FSI_FLAG_FREE_COUNT_VALID;
        }
    }
    return 0;
}

static s32 PFFAT_FreeFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, struct PF_CACHE_PAGE* p_page) {
    switch (p_vol->bpb.fat_type) {
        case FAT_12:
            return PFFAT12_WriteFATEntryWithBuf(p_vol, cluster, PF_CLUSTER_FREE, p_page);
        case FAT_16:
            return PFFAT16_WriteFATEntryWithBuf(p_vol, cluster, PF_CLUSTER_FREE, p_page);
        case FAT_32:
            return PFFAT32_WriteFATEntryWithBuf(p_vol, cluster, PF_CLUSTER_FREE, p_page);
        default:
            break;
    }
    return PF_IERR_INVALID_BPB;
}

s32 PFFAT_FreeChain(struct PF_FFD* p_ffd, u32 start_cluster, u32 chain_index, u32 size) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 eoc1;
    u32 next_cluster;
    u32 file_size;
    u32 clst_size;
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;

    p_vol = p_ffd->p_vol;
    if (start_cluster == 0) {
        return 0;
    }

    eoc1 = fat_special_values[p_vol->bpb.fat_type].eoc1;
    if (size != 0) {
        file_size = size;
        clst_size = p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;
    } else {
        return 0;
    }

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }
    err = PFFAT_ReadFATSector(p_vol, p_page, start_cluster);
    if (err) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    p_page->option = PF_CACHE_PAGE_OPT_FAT_MODIFIED;

    next_cluster = start_cluster;

    while (start_cluster < eoc1) {
        if (size != 0 && file_size == 0) {
            break;
        }
        err = PFFAT_ReadFATEntryWithBuf(p_vol, start_cluster, &next_cluster, p_page);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if (next_cluster == PF_CLUSTER_FREE) {
            err = PFFAT_UpdateFATEntry(p_vol, p_page);
            if (err) {
                PFCACHE_FreeDataPage(p_vol, p_page);
                return err;
            }
            PFCACHE_FreeDataPage(p_vol, p_page);
            return 0;
        }

        if (chain_index != PF_FAT_NO_CHAIN_INDEX) {
            err = PFFAT_WriteClusterWithBuf(p_ffd, start_cluster, chain_index, PF_CLUSTER_FREE, TRUE, p_page);
            chain_index++;
        } else {
            err = PFFAT_FreeFATEntryWithBuf(p_vol, start_cluster, p_page);
        }
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }

        if (size != 0 && file_size != 0) {
            if (clst_size >= file_size) {
                file_size = 0;
            } else {
                file_size -= clst_size;
            }
        }
        if (p_vol->num_free_clusters != PDM_FSI_UNKNOWN) {
            p_vol->num_free_clusters++;
        }
        start_cluster = next_cluster;
    }

    err = PFFAT_UpdateFATEntry(p_vol, p_page);
    if (err) {
        PFCACHE_FreeDataPage(p_vol, p_page);
        return err;
    }
    PFCACHE_FreeDataPage(p_vol, p_page);

    if (p_vol->bpb.fat_type == FAT_32 && (p_vol->fsi_flag & PF_FSI_FLAG_UPDATE)) {
        if (PFFAT_RefreshFSINFO(p_vol)) {
            p_vol->num_free_clusters = PDM_FSI_UNKNOWN;
            p_vol->fsi_flag &= ~PF_FSI_FLAG_FREE_COUNT_VALID;
        }
    }
    return 0;
}

s32 PFFAT_getBeforeChain(struct PF_VOLUME* p_vol, u32 start_cluster, u32 lActive, u32* p_cluster) {
    s32 err;
    u32 eoc1;
    u32 next_cluster;
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;

    eoc1 = fat_special_values[p_vol->bpb.fat_type].eoc1;
    *p_cluster = PF_INVALID_CLUSTER;

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }

    p_page->option = PF_CACHE_PAGE_OPT_NONE;
    while (start_cluster < eoc1) {
        err = PFFAT_ReadFATEntryWithBuf(p_vol, start_cluster, &next_cluster, p_page);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if (lActive == next_cluster) {
            *p_cluster = start_cluster;
            return 0;
        }
        start_cluster--;
        if ((start_cluster < PF_CLUSTER_FIRST || start_cluster >= p_vol->bpb.num_clusters + PF_CLUSTER_FIRST) && start_cluster < eoc1) {
            *p_cluster = lActive;
            return 0;
        }
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    return PF_IERR_BROKEN_CHAIN;
}

s32 PFFAT_InitFATRegion(struct PF_VOLUME* p_vol) {
    u32 sector;
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;
    s32 err;

    err = PFCACHE_AllocateFATPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }

    pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);
    for (sector = p_vol->bpb.num_reserved_sectors; sector < p_vol->bpb.first_root_dir_sector; sector++) {
        err = PFCACHE_WriteFATSectorAndFreeIfNeeded(p_vol, p_page->p_buf, sector);
        if (err) {
            PFCACHE_FreeFATPage(p_vol, p_page);
            return err;
        }
    }

    PFCACHE_FreeFATPage(p_vol, p_page);

    err = PFFAT_WriteFATEntry(p_vol, PF_FAT_ENTRY_MEDIA, p_vol->bpb.media | fat_special_values[p_vol->bpb.fat_type].fat0_mask);
    if (err) {
        return err;
    }
    err = PFFAT_WriteFATEntry(p_vol, PF_FAT_ENTRY_RESERVED, fat_special_values[p_vol->bpb.fat_type].fat1);
    if (err) {
        return err;
    }
    err = PFCACHE_FlushFATCache(p_vol);
    if (err) {
        return err;
    }
    return 0;
}

s32 PFFAT_MakeRootDir(struct PF_VOLUME* p_vol) {
    s32 err;
    u32 sector;
    u32 success_size;
    u32 num_sectors;
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;

    err = PFFAT_WriteFATEntry(p_vol, p_vol->bpb.root_dir_cluster, fat_special_values[p_vol->bpb.fat_type].eoc2);
    if (err) {
        return err;
    }
    err = PFCACHE_FlushFATCache(p_vol);
    if (err) {
        return err;
    }

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err) {
        return err;
    }

    pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);

    sector = p_vol->bpb.first_data_sector + ((p_vol->bpb.root_dir_cluster - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);

    for (num_sectors = 0; num_sectors < p_vol->bpb.sectors_per_cluster; num_sectors++) {
        err = PFSEC_WriteData(p_vol, p_page->p_buf, sector + num_sectors, 0, p_vol->bpb.bytes_per_sector, &success_size, FALSE);
        if (err) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return err;
        }
        if (success_size != p_vol->bpb.bytes_per_sector) {
            PFCACHE_FreeDataPage(p_vol, p_page);
            return PF_IERR_IO;
        }
    }

    PFCACHE_FreeDataPage(p_vol, p_page);
    return 0;
}

s32 PFFAT_RefreshFSINFO(struct PF_VOLUME* p_vol) {
    s32 err;
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;
    u32 success_size;

    err = 0;
    if ((p_vol->fsi_flag & PF_FSI_FLAG_FREE_COUNT_VALID) != 0) {
        err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
        if (err) {
            return err;
        }
        err = PFSEC_ReadData(p_vol, p_page->p_buf, p_vol->bpb.fs_info_sector, 0, p_vol->bpb.bytes_per_sector, &success_size, FALSE);
        if (err == 0 && success_size != p_vol->bpb.bytes_per_sector) {
            err = PF_IERR_IO;
        }
        if (err == 0) {
            if (((u32)&p_page->p_buf[PDM_FSI_FREE_COUNT] & (sizeof(u32) - 1)) != 0) {
                p_page->p_buf[PDM_FSI_FREE_COUNT] = (u8)p_vol->num_free_clusters;
                p_page->p_buf[PDM_FSI_FREE_COUNT + 1] = (u8)(p_vol->num_free_clusters >> 8);
                p_page->p_buf[PDM_FSI_FREE_COUNT + 2] = (u8)(p_vol->num_free_clusters >> 16);
                p_page->p_buf[PDM_FSI_FREE_COUNT + 3] = (u8)(p_vol->num_free_clusters >> 24);
            } else {
                ((u32*)p_page->p_buf)[PDM_FSI_FREE_COUNT / sizeof(u32)] = SWAP32(p_vol->num_free_clusters);
            }
            err = PFSEC_WriteData(p_vol, p_page->p_buf, p_vol->bpb.fs_info_sector, 0, p_vol->bpb.bytes_per_sector, &success_size, FALSE);
            if (err == 0 && success_size != p_vol->bpb.bytes_per_sector) {
                err = PF_IERR_IO;
            }
        }
        PFCACHE_FreeDataPage(p_vol, p_page);
    }
    return err;
}

void PFFAT_InitHint(struct PF_FAT_HINT* p_hint) {
    p_hint->chain_index = 0;
    p_hint->cluster = 0;
    p_hint->file_version = 0;
}

s32 PFFAT_TraceClustersChain(struct PF_FFD* p_ffd, u32 start_clst, u32 size, u32* p_target_clst, u32* p_next_clst) {
    s32 err;
    u32 next_cluster;
    u32 save_cluster;
    u32 chain_index;
    u32 clst_size;
    u32 clst_cnt;
    struct PF_VOLUME* p_vol;
    u32 cnt;
    u32 check_use;

    save_cluster = start_clst;

    *p_target_clst = 0;
    *p_next_clst = 0;

    if (*p_ffd->p_start_cluster < PF_CLUSTER_FIRST) {
        return 0;
    }
    if (start_clst < PF_CLUSTER_FIRST) {
        return 0;
    }
    if (size == 0) {
        return 0;
    }

    p_vol = p_ffd->p_vol;
    if (size == PF_FAT_WHOLE_CHAIN && p_ffd->last_cluster.num_last_cluster != 0) {
        *p_target_clst = p_ffd->last_cluster.num_last_cluster;
        *p_next_clst = fat_special_values[p_vol->bpb.fat_type].eoc2;
        return 0;
    }

    clst_size = p_vol->bpb.bytes_per_sector * p_vol->bpb.sectors_per_cluster;
    clst_cnt = size / clst_size;
    if (size % clst_size) {
        clst_cnt++;
    }

    if (*p_ffd->p_start_cluster == start_clst) {
        chain_index = 1;
    } else {
        chain_index = p_ffd->p_hint->chain_index + 1;
    }

    next_cluster = PF_INVALID_CLUSTER;
    check_use = TRUE;

    for (cnt = 0; cnt < clst_cnt; cnt++) {
        err = PFFAT_ReadCluster(p_ffd, start_clst, chain_index, &next_cluster, &check_use);
        if (err) {
            return err;
        }
        save_cluster = start_clst;
        if (next_cluster == fat_special_values[p_vol->bpb.fat_type].eoc2) {
            p_ffd->last_cluster.num_last_cluster = start_clst;
            p_ffd->last_cluster.max_chain_index = chain_index - 1;
            break;
        }
        start_clst = next_cluster;
        chain_index++;
    }

    p_ffd->p_hint->chain_index = chain_index - 1;
    *p_target_clst = save_cluster;
    *p_next_clst = next_cluster;
    return 0;
}

s32 PFFAT_WriteValueToSpecifiedCluster(struct PF_VOLUME* p_vol, u32 cluster, u32 value) {
    s32 err;

    err = PFFAT_WriteFATEntry(p_vol, cluster, value);
    if (err) {
        return err;
    }

    return 0;
}

s32 PFFAT_ReadValueToSpecifiedCluster(struct PF_VOLUME* p_vol, u32 cluster, u32* value) {
    s32 err;

    err = PFFAT_ReadFATEntry(p_vol, cluster, value);
    if (err) {
        return err;
    }

    return 0;
}

s32 PFFAT_AllocateNumClusters(struct PF_FFD* p_ffd, u32 size, u32* p_chain_start, u32* p_last_allocated, u32* p_allocated_size) {
    s32 err;
    u32 clst_size;
    u32 num_cluster;
    u32 num_allocated;

    clst_size = p_ffd->p_vol->bpb.bytes_per_sector << p_ffd->p_vol->bpb.log2_sectors_per_cluster;
    num_cluster = size / clst_size;
    if (size % clst_size) {
        num_cluster++;
    }

    err = PFFAT_AllocateChain(p_ffd, num_cluster, 1, p_chain_start, p_last_allocated);
    if (err) {
        return err;
    }

    if (*p_chain_start == PF_INVALID_CLUSTER || *p_last_allocated == PF_INVALID_CLUSTER) {
        *p_allocated_size = 0;
        return 0;
    }

    num_allocated = *p_last_allocated - *p_chain_start + 1;
    if (num_cluster == num_allocated) {
        *p_allocated_size = size;
    } else {
        *p_allocated_size = num_allocated * clst_size;
    }
    return 0;
}

s32 PFFAT_ResetFFD(struct PF_FFD* p_ffd, u32* p_start_cluster) {
    p_ffd->p_start_cluster = p_start_cluster;
    p_ffd->file_version = 1;
    if (p_ffd->p_hint != NULL) {
        p_ffd->p_hint->file_version = 0;
    }
    p_ffd->last_access_cluster.cluster = 0;
    p_ffd->last_access_cluster.chain_index = 0;
    p_ffd->last_cluster.num_last_cluster = 0;
    p_ffd->last_cluster.max_chain_index = 0;
    p_ffd->cluster_link.buffer = NULL;
    return 0;
}

s32 PFFAT_InitFFD(struct PF_FFD* p_ffd, struct PF_FAT_HINT* p_hint, struct PF_VOLUME* p_vol, u32* p_start_cluster) {
    p_ffd->p_hint = p_hint;
    p_ffd->p_vol = p_vol;
    PFFAT_ResetFFD(p_ffd, p_start_cluster);
    return 0;
}

s32 PFFAT_FinalizeFFD(struct PF_FFD* p_ffd) {
    p_ffd->cluster_link.buffer = NULL;
    return 0;
}

u32 PFFAT_GetValueOfEOC2(struct PF_VOLUME* p_vol) {
    return fat_special_values[p_vol->bpb.fat_type].eoc2;
}
