#include <prfile2/pdm_bpb.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cluster.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_entry.h>
#include <prfile2/pf_fat.h>
#include <prfile2/pf_path.h>
#include <prfile2/pf_sector.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_system.h>
#include <prfile2/pf_volume.h>

void PFCLUSTER_UpdateLastAccessCluster(struct PF_FILE* p_file, u32 sector) {
    struct PF_VOLUME* p_vol;

    if (p_file->cursor.position == 0) {
        p_file->p_sfd->ffd.last_access_cluster.cluster = 0;
        p_file->p_sfd->ffd.last_access_cluster.chain_index = 0;
    } else {
        p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
        if ((p_file->cursor.position & (p_vol->bpb.bytes_per_sector - 1)) == 0 && (p_file->cursor.file_sector_index & (p_vol->bpb.sectors_per_cluster - 1)) == 0) {
            if (p_file->cursor.file_sector_index != 0) {
                p_file->p_sfd->ffd.last_access_cluster.chain_index = (p_file->cursor.file_sector_index - 1) >> p_vol->bpb.log2_sectors_per_cluster;
                p_file->p_sfd->ffd.last_access_cluster.cluster = ((sector - 1 - p_vol->bpb.first_data_sector) >> p_vol->bpb.log2_sectors_per_cluster) + PF_CLUSTER_FIRST;
            }
        } else {
            p_file->p_sfd->ffd.last_access_cluster.chain_index = p_file->cursor.file_sector_index >> p_vol->bpb.log2_sectors_per_cluster;
            p_file->p_sfd->ffd.last_access_cluster.cluster = ((sector - p_vol->bpb.first_data_sector) >> p_vol->bpb.log2_sectors_per_cluster) + PF_CLUSTER_FIRST;
        }
    }
}

s32 PFCLUSTER_AppendCluster(struct PF_FILE* p_file, u32 byte, u32* p_success) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 sector;
    u32 num_sector;
    u32 max_appendable_size;
    struct PF_CURSOR save_cursor;
    struct PF_FAT_HINT save_hint;

    *p_success = 0;

    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        return PF_IERR_BAD_FILE;
    }
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    save_cursor = p_file->cursor;
    save_hint = p_file->hint;
    p_file->p_sfd->ffd.p_hint = &p_file->hint;

    sector = PF_INVALID_SECTOR;
    if ((p_vol->fsi_flag & PF_FSI_FLAG_FREE_COUNT_VALID) != 0 && p_vol->num_free_clusters != PDM_FSI_UNKNOWN && p_vol->num_free_clusters == 0) {
        return PF_IERR_NO_SPACE;
    }

    PFFILE_Cursor_MoveToClusterEnd(p_file, p_file->p_sfd->dir_entry.file_size + byte);
    if (p_file->cursor.position == PF_FILE_SIZE_MAX) {
        *p_success = 0;
        return PF_IERR_FILE_TOO_BIG;
    }

    err = PFFAT_GetSectorAllocated(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, byte, &sector);
    if (err) {
        p_file->cursor = save_cursor;
        p_file->hint = save_hint;
        return err;
    }

    num_sector = 0;
    err = PFFAT_getContinuousSector(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, byte, &sector, &num_sector);
    p_file->cursor = save_cursor;
    p_file->hint = save_hint;
    if (err) {
        return err;
    }

    if (sector == PF_INVALID_SECTOR) {
        PFFILE_Cursor_MoveToEnd(p_file);
        return PF_IERR_INVALID_OPERATION;
    }

    max_appendable_size = num_sector << p_vol->bpb.log2_bytes_per_sector;
    if (byte < max_appendable_size) {
        max_appendable_size = byte;
    }
    *p_success = max_appendable_size;
    return 0;
}

s32 PFCLUSTER_AdjustCluster(struct PF_FILE* p_file) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 chain_index;
    u32 last_cluster;
    u32 next_cluster;
    u32 eoc2;

    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        return PF_IERR_BAD_FILE;
    }
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    chain_index = PF_FAT_NO_CHAIN_INDEX;
    p_file->p_sfd->ffd.p_hint = &p_file->hint;

    if (p_file->p_sfd->ffd.last_cluster.max_chain_index != 0 &&
        p_file->p_sfd->ffd.last_cluster.max_chain_index + 1 ==
            (p_file->p_sfd->dir_entry.file_size >> (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster)) +
                ((p_file->p_sfd->dir_entry.file_size & ((p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster) - 1)) ? 1 : 0)) {
        return 0;
    }

    if (p_file->p_sfd->dir_entry.file_size != 0) {
        err = PFFAT_TraceClustersChain(&p_file->p_sfd->ffd, p_file->p_sfd->dir_entry.start_cluster, p_file->p_sfd->dir_entry.file_size, &last_cluster, &next_cluster);
        if (err) {
            return err;
        }
    } else {
        last_cluster = 0;
        next_cluster = p_file->p_sfd->dir_entry.start_cluster;
    }

    if (next_cluster != 0) {
        eoc2 = PFFAT_GetValueOfEOC2(p_vol);
        if (next_cluster != eoc2) {
            if (last_cluster != 0) {
                err = PFFAT_WriteValueToSpecifiedCluster(p_vol, last_cluster, eoc2);
                if (err) {
                    return err;
                }
                p_file->p_sfd->ffd.last_cluster.num_last_cluster = last_cluster;
                p_file->p_sfd->ffd.last_cluster.max_chain_index =
                    (p_file->p_sfd->dir_entry.file_size >> (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster)) +
                    ((p_file->p_sfd->dir_entry.file_size & ((p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster) - 1)) ? 1 : 0) - 1;
                chain_index = p_file->p_sfd->ffd.last_cluster.max_chain_index + 1;
            } else {
                p_file->p_sfd->dir_entry.start_cluster = 0;
                *p_file->p_sfd->ffd.p_start_cluster = 0;
                p_file->p_sfd->ffd.last_cluster.num_last_cluster = 0;
                p_file->p_sfd->ffd.last_cluster.max_chain_index = 0;
                err = PFENT_updateEntry(&p_file->p_sfd->dir_entry, PFENT_UPDATE_SET_ARCHIVE);
                if (err) {
                    return err;
                }
                if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
                    err = PFCACHE_FlushDataCacheSpecific(p_vol, p_file);
                    if (err) {
                        return err;
                    }
                }
            }
            err = PFFAT_FreeChain(&p_file->p_sfd->ffd, next_cluster, chain_index, PF_FAT_WHOLE_CHAIN);
            if (err) {
                return err;
            }
        }
    }
    return 0;
}

s32 PFCLUSTER_GetAppendSize(struct PF_FILE* p_file, u32* p_size) {
    struct PF_VOLUME* p_vol;
    u32 cluster_size;
    u32 file_cluster_size;
    u32 total_allocated_size;
    u32 file_end_cluster;
    u32 unused_cluster;
    u32 next_cluster;
    u32 num_append_cluster;
    u32 sig_eoc;
    s32 err;

    *p_size = 0;

    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        return PF_IERR_BAD_FILE;
    }

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    cluster_size = p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;

    if (p_file->p_sfd->dir_entry.start_cluster != 0) {
        file_cluster_size = p_file->p_sfd->dir_entry.file_size +
                            ((p_file->p_sfd->dir_entry.file_size % cluster_size) ? cluster_size - p_file->p_sfd->dir_entry.file_size % cluster_size : 0);
        if (p_file->p_sfd->ffd.last_cluster.max_chain_index) {
            total_allocated_size = (p_file->p_sfd->ffd.last_cluster.max_chain_index + 1) * cluster_size;
        } else {
            if (p_file->p_sfd->dir_entry.file_size != 0) {
                err = PFFAT_TraceClustersChain(&p_file->p_sfd->ffd, p_file->p_sfd->dir_entry.start_cluster, p_file->p_sfd->dir_entry.file_size, &file_end_cluster, &unused_cluster);
                if (err) {
                    return err;
                }
            } else {
                unused_cluster = p_file->p_sfd->dir_entry.start_cluster;
            }

            sig_eoc = PFFAT_GetValueOfEOC2(p_vol);
            num_append_cluster = 0;

            while (unused_cluster != sig_eoc) {
                num_append_cluster++;
                err = PFFAT_ReadValueToSpecifiedCluster(p_vol, unused_cluster, &next_cluster);
                if (err != 0) {
                    return err;
                }
                unused_cluster = next_cluster;
            }

            total_allocated_size = file_cluster_size + num_append_cluster * cluster_size;
        }
        *p_size = total_allocated_size - file_cluster_size;
    }
    return 0;
}

static inline u32 PFCLUSTER_GetClusterSize(struct PF_VOLUME* p_vol) {
    return p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;
}

s32 PFCLUSTER_CombineFiles(struct PF_ENT_ITER* p_iter1, struct PF_ENT_ITER* p_iter2, struct PF_DIR_ENT* p_ent1, struct PF_DIR_ENT* p_ent2) {
    s32 err;
    u32 clst_size;
    u32 num_clst1;
    u32 num_clst2;
    u32 unused_clst1;
    u32 unused_clst2;
    u32 over_clst;
    u32 tail_clst1;
    u32 start_clst1;
    u32 start_clst2;
    u32 eoc2;
    struct PF_VOLUME* p_vol;
    u32 max_clst;
    u32 last_clst;
    u32 next_clst;

    p_vol = p_ent1->p_vol;
    num_clst1 = 0;
    num_clst2 = 0;
    unused_clst1 = 0;
    unused_clst2 = 0;
    start_clst1 = 0;

    err = PFFAT_TraceClustersChain(&p_iter1->ffd, *p_iter1->ffd.p_start_cluster, p_ent1->file_size, &last_clst, &next_clst);
    if (err) {
        return err;
    }
    tail_clst1 = last_clst;
    clst_size = PFCLUSTER_GetClusterSize(p_vol);
    if (p_ent1->file_size != 0) {
        num_clst1 = p_ent1->file_size / clst_size;
        if (p_ent1->file_size % clst_size) {
            num_clst1++;
        }
    }

    eoc2 = PFFAT_GetValueOfEOC2(p_vol);
    if (next_clst != eoc2) {
        if (p_ent1->file_size != 0 && next_clst != 0) {
            start_clst1 = next_clst;
        } else {
            start_clst1 = *p_iter1->ffd.p_start_cluster;
        }
        err = PFFAT_TraceClustersChain(&p_iter1->ffd, start_clst1, PF_FAT_WHOLE_CHAIN, &last_clst, &next_clst);
        if (err) {
            return err;
        }
        if (*p_iter1->ffd.p_start_cluster != 0) {
            unused_clst1 = p_iter1->ffd.last_cluster.max_chain_index + 1 - num_clst1;
        }
    }

    err = PFFAT_TraceClustersChain(&p_iter2->ffd, *p_iter2->ffd.p_start_cluster, p_ent2->file_size, &last_clst, &next_clst);
    if (err) {
        return err;
    }
    if (p_ent2->file_size != 0) {
        num_clst2 = p_ent2->file_size / clst_size;
        if (p_ent2->file_size % clst_size) {
            num_clst2++;
        }
    }

    if (next_clst != eoc2) {
        if (p_ent2->file_size != 0 && next_clst != 0) {
            start_clst2 = next_clst;
        } else {
            start_clst2 = *p_iter2->ffd.p_start_cluster;
        }
        err = PFFAT_TraceClustersChain(&p_iter2->ffd, start_clst2, PF_FAT_WHOLE_CHAIN, &last_clst, &next_clst);
        if (err) {
            return err;
        }
        if (*p_iter2->ffd.p_start_cluster != 0) {
            unused_clst2 = p_iter2->ffd.last_cluster.max_chain_index + 1 - num_clst2;
        }

        max_clst = PF_FILE_SIZE_MAX / clst_size + 1;
        if (max_clst < unused_clst2 + num_clst1 + num_clst2) {
            over_clst = unused_clst2 + num_clst1 + num_clst2 - max_clst;
            err = PFFAT_TraceClustersChain(&p_iter2->ffd, start_clst2, clst_size * (p_iter2->ffd.last_cluster.max_chain_index + 1 - over_clst - num_clst2), &last_clst, &next_clst);
            if (err) {
                return err;
            }
            err = PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, eoc2);
            if (err) {
                return err;
            }
            err = PFFAT_FreeChain(&p_iter2->ffd, next_clst, PF_FAT_NO_CHAIN_INDEX, over_clst * clst_size);
            if (err) {
                return err;
            }
            if (p_ent2->file_size == 0 && unused_clst2 == over_clst) {
                *p_iter2->ffd.p_start_cluster = 0;
                p_ent2->start_cluster = 0;
            }
        }
    }

    if (unused_clst1 != 0) {
        err = PFFAT_FreeChain(&p_iter1->ffd, start_clst1, PF_FAT_NO_CHAIN_INDEX, unused_clst1 * clst_size);
        if (err) {
            return err;
        }
        if (p_ent1->file_size == 0) {
            *p_iter1->ffd.p_start_cluster = 0;
            p_ent1->start_cluster = 0;
        }
    }

    if (*p_iter1->ffd.p_start_cluster != 0 && *p_iter2->ffd.p_start_cluster != 0) {
        err = PFFAT_WriteValueToSpecifiedCluster(p_vol, tail_clst1, *p_iter2->ffd.p_start_cluster);
        if (err) {
            return err;
        }
    } else if (*p_iter1->ffd.p_start_cluster == 0 && *p_iter2->ffd.p_start_cluster != 0) {
        p_ent1->start_cluster = p_ent2->start_cluster;
    }

    p_ent1->file_size = p_ent2->file_size + num_clst1 * clst_size;
    return 0;
}

s32 PFCLUSTER_DivideFile(struct PF_ENT_ITER* p_iter_src, struct PF_ENT_ITER* p_iter_dst, struct PF_DIR_ENT* p_ent_src, struct PF_DIR_ENT* p_ent_dst, u32 offset) {
    s32 err;
    u32 src_sector;
    u32 dst_sector;
    u32 clst_size;
    u32 i;
    u32 prev_clst;
    u32 allocated;
    u32 head_clst;
    u32 rest_size;
    struct PF_VOLUME* p_vol;
    u32 save_next_clst;
    u32 eoc2;
    u32 last_clst;
    u32 alloc_size;
    u32 alloc_start;
    u32 alloc_last;
    u32 next_clst;
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;

    alloc_start = 0;
    p_vol = p_ent_src->p_vol;
    head_clst = 0;
    prev_clst = 0;
    rest_size = 0;

    if (offset == 0) {
        next_clst = p_ent_src->start_cluster;
    } else {
        err = PFFAT_TraceClustersChain(&p_iter_src->ffd, *p_iter_src->ffd.p_start_cluster, offset, &last_clst, &next_clst);
        if (err) {
            return err;
        }
        save_next_clst = next_clst;
        eoc2 = PFFAT_GetValueOfEOC2(p_vol);
        err = PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, eoc2);
        if (err) {
            return err;
        }

        if (offset == p_ent_src->file_size) {
            if (next_clst == eoc2) {
                next_clst = 0;
            }
        } else {
            clst_size = PFCLUSTER_GetClusterSize(p_vol);
            rest_size = offset % clst_size;
            if (rest_size != 0) {
                for (allocated = 0; allocated < clst_size; allocated += alloc_size) {
                    err = PFFAT_AllocateNumClusters(&p_iter_dst->ffd, clst_size, &alloc_start, &alloc_last, &alloc_size);
                    if (err) {
                        PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                        return err;
                    }
                    if (alloc_size == 0) {
                        break;
                    }
                    if (allocated == 0) {
                        head_clst = alloc_start;
                    } else {
                        err = PFFAT_WriteValueToSpecifiedCluster(p_vol, prev_clst, alloc_start);
                        if (err) {
                            PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                            PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                            return err;
                        }
                    }
                    prev_clst = alloc_last;
                }

                if (clst_size != allocated) {
                    if (allocated != 0) {
                        err = PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                        if (err) {
                            PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                            return err;
                        }
                    }
                    PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                    PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                    return PF_IERR_NO_SPACE;
                }

                err = PFFAT_WriteValueToSpecifiedCluster(p_vol, prev_clst, next_clst);
                if (err) {
                    PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                    PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                    return err;
                }

                next_clst = head_clst;
                src_sector = p_vol->bpb.first_data_sector + ((last_clst - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);
                dst_sector = p_vol->bpb.first_data_sector + ((alloc_start - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);
                err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
                if (err) {
                    PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                    PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                    return err;
                }

                for (i = 0; i < p_vol->bpb.sectors_per_cluster; src_sector++, dst_sector++, i++) {
                    err = PFSEC_ReadData(p_vol, p_page->p_buf, src_sector, 0, p_vol->bpb.bytes_per_sector, &alloc_size, FALSE);
                    if (err) {
                        PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                        PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                        return err;
                    }
                    if (alloc_size != p_vol->bpb.bytes_per_sector) {
                        PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                        PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                        return PF_IERR_IO;
                    }
                    err = PFSEC_WriteData(p_vol, p_page->p_buf, dst_sector, 0, p_vol->bpb.bytes_per_sector, &alloc_size, FALSE);
                    if (err) {
                        PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                        PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                        return err;
                    }
                    if (alloc_size != p_vol->bpb.bytes_per_sector) {
                        PFFAT_FreeChain(&p_iter_dst->ffd, head_clst, PF_FAT_NO_CHAIN_INDEX, allocated);
                        PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, save_next_clst);
                        return PF_IERR_IO;
                    }
                }
                PFCACHE_FreeDataPage(p_vol, p_page);
            }
        }
    }

    p_ent_dst->start_cluster = next_clst;
    p_ent_dst->file_size = p_ent_src->file_size - offset + rest_size;
    return 0;
}

s32 PFCLUSTER_InsertCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 start, u32 num, u32* p_num_inserted) {
    s32 err;
    u32 clst_size;
    u32 total_clst;
    u32 max_clst;
    u32 size;
    u32 allocated;
    u32 head_clst;
    u32 prev_clst;
    struct PF_VOLUME* p_vol;
    u32 file_clst;
    u32 pad;
    u32 last_clst;
    u32 next_clst;
    u32 alloc_size;
    u32 alloc_start;
    u32 alloc_last;

    p_vol = p_ent->p_vol;
    clst_size = PFCLUSTER_GetClusterSize(p_vol);
    head_clst = 0;
    prev_clst = 0;

    err = PFFAT_TraceClustersChain(&p_iter->ffd, *p_iter->ffd.p_start_cluster, start * clst_size, &last_clst, &next_clst);
    if (err) {
        return err;
    }

    total_clst = p_ent->file_size / clst_size;
    if (p_ent->file_size % clst_size) {
        total_clst++;
    }
    total_clst += num;
    max_clst = PF_FILE_SIZE_MAX / clst_size + 1;
    if (max_clst < total_clst) {
        num = total_clst - max_clst;
    }

    size = num * clst_size;
    for (allocated = 0; allocated < size; allocated += alloc_size) {
        err = PFFAT_AllocateNumClusters(&p_iter->ffd, size, &alloc_start, &alloc_last, &alloc_size);
        if (err) {
            return err;
        }
        if (alloc_size == 0) {
            if (allocated == 0) {
                return PF_IERR_NO_SPACE;
            }
            break;
        }
        if (allocated == 0) {
            head_clst = alloc_start;
        } else {
            err = PFFAT_WriteValueToSpecifiedCluster(p_vol, prev_clst, alloc_start);
            if (err) {
                return err;
            }
        }
        prev_clst = alloc_last;
    }

    if (next_clst == 0) {
        if (p_ent->start_cluster != 0) {
            next_clst = p_ent->start_cluster;
        } else {
            next_clst = PFFAT_GetValueOfEOC2(p_vol);
        }
    }

    err = PFFAT_WriteValueToSpecifiedCluster(p_vol, prev_clst, next_clst);
    if (err) {
        return err;
    }

    if (start != 0) {
        err = PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, head_clst);
        if (err) {
            return err;
        }
    } else {
        p_ent->start_cluster = head_clst;
    }

    file_clst = p_ent->file_size / clst_size;
    if (p_ent->file_size % clst_size) {
        file_clst++;
    }
    pad = 0;
    if (start == file_clst && p_ent->file_size % clst_size != 0) {
        pad = clst_size - p_ent->file_size % clst_size;
    }
    p_ent->file_size += allocated + pad;
    *p_num_inserted = allocated / clst_size;
    return 0;
}

s32 PFCLUSTER_DeleteCluster(struct PF_ENT_ITER* p_iter, struct PF_DIR_ENT* p_ent, u32 start, u32 num, u32* p_num_deleted) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 clst_size;
    u32 last_clst;
    u32 del_next_clst;
    u32 next_clst;
    u32 del_last_clst;
    u32 size;

    p_vol = p_ent->p_vol;
    clst_size = PFCLUSTER_GetClusterSize(p_vol);

    err = PFFAT_TraceClustersChain(&p_iter->ffd, *p_iter->ffd.p_start_cluster, start * clst_size, &last_clst, &next_clst);
    if (err) {
        return err;
    }
    if (start == 0) {
        next_clst = p_ent->start_cluster;
    }

    size = num * clst_size > p_ent->file_size - start * clst_size ? p_ent->file_size - start * clst_size : num * clst_size;

    err = PFFAT_TraceClustersChain(&p_iter->ffd, next_clst, size, &del_last_clst, &del_next_clst);
    if (err) {
        return err;
    }

    if (start == 0) {
        if (del_next_clst == PFFAT_GetValueOfEOC2(p_vol)) {
            del_next_clst = 0;
        }
        p_ent->start_cluster = del_next_clst;
    }

    p_ent->file_size -= size;
    PFENT_getcurrentDateTimeForEnt(&p_ent->modify_date, &p_ent->modify_time);
    p_ent->access_date = p_ent->modify_date;
    err = PFENT_updateEntry(p_ent, PFENT_UPDATE_SET_ARCHIVE);
    if (err) {
        return err;
    }
    if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
        err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
        if (err) {
            return err;
        }
    }

    err = PFFAT_FreeChain(&p_iter->ffd, next_clst, PF_FAT_NO_CHAIN_INDEX, size);
    if (err) {
        return err;
    }

    if (start != 0) {
        err = PFFAT_WriteValueToSpecifiedCluster(p_vol, last_clst, del_next_clst);
        if (err) {
            return err;
        }
    }

    *p_num_deleted = size / clst_size;
    if (size % clst_size) {
        *p_num_deleted = size / clst_size + 1;
    }
    return 0;
}
