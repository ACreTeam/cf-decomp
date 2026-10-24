#include <prfile2/pf_api.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cluster.h>
#include <prfile2/pf_code.h>
#include <prfile2/pf_dir.h>
#include <prfile2/pf_driver.h>
#include <prfile2/pf_entry.h>
#include <prfile2/pf_entry_iterator.h>
#include <prfile2/pf_fat.h>
#include <prfile2/pf_file.h>
#include <prfile2/pf_filelock.h>
#include <prfile2/pf_path.h>
#include <prfile2/pf_sector.h>
#include <prfile2/pf_service.h>
#include <prfile2/pf_system.h>
#include <prfile2/pf_volume.h>

static void PFFILE_Cursor_Recalc(struct PF_FILE* p_file) {
    struct PF_VOLUME* p_vol;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    p_file->cursor.file_sector_index = p_file->cursor.position >> p_vol->bpb.log2_bytes_per_sector;
    p_file->cursor.offset_in_sector = p_file->cursor.position & (p_vol->bpb.bytes_per_sector - 1);
}

u32 PFFILE_Cursor_AdvanceToRead(struct PF_FILE* p_file, u32 n, u32 sector) {
    u32 res;
    struct PF_VOLUME* p_vol;
    u32 wk_sector;

    res = TRUE;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    wk_sector = sector + ((p_file->cursor.offset_in_sector + n) >> p_vol->bpb.log2_bytes_per_sector);

    if ((p_file->cursor.position + n) < p_file->p_sfd->dir_entry.file_size) {
        if (((p_file->cursor.offset_in_sector + n) & (p_vol->bpb.bytes_per_sector - 1)) != 0) {
            p_file->cursor.sector = wk_sector;
        } else {
            p_file->cursor.sector = PF_INVALID_SECTOR;
        }
        p_file->cursor.position += n;
    } else {
        if (p_file->p_sfd->dir_entry.file_size == 0) {
            p_file->cursor.sector = PF_INVALID_SECTOR;
            p_file->cursor.position = 0;
            if (n != 0) {
                res = FALSE;
            }
        } else {
            p_file->cursor.sector = PF_INVALID_SECTOR;
            p_file->cursor.position = p_file->p_sfd->dir_entry.file_size;
            res = FALSE;
        }
    }

    PFFILE_Cursor_Recalc(p_file);
    PFCLUSTER_UpdateLastAccessCluster(p_file, wk_sector);
    return res;
}

s32 PFFILE_Cursor_ReadHeadSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 max_readable_size;
    u32 success_size;

    *p_size_read = 0;
    if (p_file->cursor.offset_in_sector == 0) {
        return 0;
    }
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    max_readable_size = p_vol->bpb.bytes_per_sector - p_file->cursor.offset_in_sector;
    if (size > max_readable_size) {
        size = max_readable_size;
    }
    if (p_file->cursor.position + size > p_file->p_sfd->dir_entry.file_size) {
        size = p_file->p_sfd->dir_entry.file_size - p_file->cursor.position;
        if (size < max_readable_size) {
            return 0;
        }
    }

    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        err = PFFAT_GetSectorSpecified(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, FALSE, &p_file->cursor.sector);
        if (err != 0) {
            return err;
        }
        if (p_file->cursor.sector == PF_INVALID_SECTOR) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_INVALID_OPERATION;
        }
        if (p_file->cursor.position + size > p_file->p_sfd->dir_entry.file_size) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_END_OF_FILE;
        }
    }

    err = PFSEC_ReadData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_read = success_size;
    PFFILE_Cursor_AdvanceToRead(p_file, success_size, p_file->cursor.sector);
    if (success_size != size) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_ReadBodySectors(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 num_sector;
    u32 max_readable_size;
    u32 success_size;

    *p_size_read = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    num_sector = 0;

    err = PFFAT_getContinuousSector(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, size, &p_file->cursor.sector, &num_sector);
    if (err != 0) {
        return err;
    }
    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        PFFILE_Cursor_MoveToEnd(p_file);
        return PF_IERR_INVALID_OPERATION;
    }

    max_readable_size = num_sector << p_vol->bpb.log2_bytes_per_sector;
    if (size > max_readable_size) {
        size = max_readable_size;
    }
    if (p_file->cursor.position + size > p_file->p_sfd->dir_entry.file_size) {
        size = p_file->p_sfd->dir_entry.file_size - p_file->cursor.position;
        size -= size & (p_vol->bpb.bytes_per_sector - 1);
        if (size < p_vol->bpb.bytes_per_sector) {
            return 0;
        }
    }

    err = PFSEC_ReadData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_read = success_size;
    PFFILE_Cursor_AdvanceToRead(p_file, success_size, p_file->cursor.sector);
    if (success_size != size - (size & (p_vol->bpb.bytes_per_sector - 1))) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_ReadTailSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 success_size;

    *p_size_read = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    if (size == 0) {
        return 0;
    }
    if (p_file->cursor.position + size > p_file->p_sfd->dir_entry.file_size) {
        size = p_file->p_sfd->dir_entry.file_size - p_file->cursor.position;
        if (size == 0) {
            return 0;
        }
    }

    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        err = PFFAT_GetSectorSpecified(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, FALSE, &p_file->cursor.sector);
        if (err != 0) {
            return err;
        }
        if (p_file->cursor.sector == PF_INVALID_SECTOR) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_INVALID_OPERATION;
        }
        if (p_file->cursor.position + size > p_file->p_sfd->dir_entry.file_size) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_END_OF_FILE;
        }
    }

    err = PFSEC_ReadData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_read = success_size;
    PFFILE_Cursor_AdvanceToRead(p_file, success_size, p_file->cursor.sector);
    if (success_size != size) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_Read(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_read) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 size_read;

    p_file->p_sfd->ffd.p_hint = &p_file->hint;
    *p_size_read = 0;

    err = PFFILE_Cursor_ReadHeadSector(p_file, p_buf, size, &size_read);
    *p_size_read += size_read;
    if (err != 0) {
        return err;
    }
    if (size_read >= size) {
        return 0;
    }

    size -= size_read;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    while (size >= p_vol->bpb.bytes_per_sector) {
        err = PFFILE_Cursor_ReadBodySectors(p_file, p_buf + *p_size_read, size, &size_read);
        *p_size_read += size_read;
        if (err != 0) {
            return err;
        }
        if (size_read == 0) {
            break;
        }
        size -= size_read;
    }

    err = PFFILE_Cursor_ReadTailSector(p_file, p_buf + *p_size_read, size, &size_read);
    *p_size_read += size_read;
    if (err != 0) {
        return err;
    }
    return 0;
}

static void PFFILE_Cursor_AdvanceToWrite(struct PF_FILE* p_file, u32 n, u32 sector) {
    struct PF_VOLUME* p_vol;
    u32 wk_sector;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    wk_sector = sector + ((p_file->cursor.offset_in_sector + n) >> p_vol->bpb.log2_bytes_per_sector);
    if (((p_file->cursor.offset_in_sector + n) & (p_vol->bpb.bytes_per_sector - 1)) != 0) {
        p_file->cursor.sector = wk_sector;
    } else {
        p_file->cursor.sector = PF_INVALID_SECTOR;
    }
    p_file->cursor.position += n;
    if (p_file->cursor.position > p_file->p_sfd->dir_entry.file_size) {
        p_file->p_sfd->dir_entry.file_size = p_file->cursor.position;
    }

    PFFILE_Cursor_Recalc(p_file);
    PFCLUSTER_UpdateLastAccessCluster(p_file, wk_sector);
}

static void PFFILE_Cursor_Initialize(struct PF_FILE* p_file) {
    p_file->cursor.sector = PF_INVALID_SECTOR;
    p_file->cursor.position = 0;
    PFFILE_Cursor_Recalc(p_file);
}

s32 PFFILE_Cursor_WriteHeadSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 max_writable_size;
    u32 success_size;

    *p_size_write = 0;
    if (p_file->cursor.offset_in_sector == 0) {
        return 0;
    }
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    max_writable_size = p_vol->bpb.bytes_per_sector - p_file->cursor.offset_in_sector;
    if (size > max_writable_size) {
        size = max_writable_size;
    }

    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        err = PFFAT_GetSectorSpecified(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, FALSE, &p_file->cursor.sector);
        if (err != 0) {
            return err;
        }
        if (p_file->cursor.sector == PF_INVALID_SECTOR) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_INVALID_OPERATION;
        }
    }

    err = PFSEC_WriteData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_write = success_size;
    PFFILE_Cursor_AdvanceToWrite(p_file, success_size, p_file->cursor.sector);
    if (success_size != size) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_WriteBodySectors(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 num_sector;
    u32 max_writable_size;
    u32 success_size;

    *p_size_write = 0;
    num_sector = 0;
    err = PFFAT_getContinuousSector(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, size, &p_file->cursor.sector, &num_sector);
    if (err != 0) {
        return err;
    }
    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        PFFILE_Cursor_MoveToEnd(p_file);
        return PF_IERR_INVALID_OPERATION;
    }

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    max_writable_size = num_sector << p_vol->bpb.log2_bytes_per_sector;
    if (size > max_writable_size) {
        size = max_writable_size;
    }

    err = PFSEC_WriteData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_write = success_size;
    PFFILE_Cursor_AdvanceToWrite(p_file, success_size, p_file->cursor.sector);
    if (success_size != size - (size & (p_vol->bpb.bytes_per_sector - 1))) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_WriteTailSector(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 success_size;

    *p_size_write = 0;
    if (size == 0) {
        return 0;
    }
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    if (size > p_vol->bpb.bytes_per_sector - p_file->cursor.offset_in_sector || size > p_vol->bpb.bytes_per_sector) {
        return PF_IERR_INTERNAL;
    }

    if (p_file->cursor.sector == PF_INVALID_SECTOR) {
        err = PFFAT_GetSectorSpecified(&p_file->p_sfd->ffd, p_file->cursor.file_sector_index, FALSE, &p_file->cursor.sector);
        if (err != 0) {
            return err;
        }
        if (p_file->cursor.sector == PF_INVALID_SECTOR) {
            PFFILE_Cursor_MoveToEnd(p_file);
            return PF_IERR_INVALID_OPERATION;
        }
    }

    err = PFSEC_WriteData(p_vol, p_buf, p_file->cursor.sector, p_file->cursor.offset_in_sector, size, &success_size, TRUE);
    if (err != 0 && success_size == 0) {
        return err;
    }
    *p_size_write = success_size;
    PFFILE_Cursor_AdvanceToWrite(p_file, success_size, p_file->cursor.sector);
    if (success_size != size) {
        return err;
    }
    return 0;
}

s32 PFFILE_Cursor_Write(struct PF_FILE* p_file, u8* p_buf, u32 size, u32* p_size_write) {
    s32 err;
    struct PF_VOLUME* p_vol;
    u32 size_write;
    u32 num_cluster;
    u32 append_size;
    u32 size_request;

    p_file->p_sfd->ffd.p_hint = &p_file->hint;
    *p_size_write = 0;
    if (PF_FILE_SIZE_MAX - p_file->cursor.position < size) {
        size = PF_FILE_SIZE_MAX - p_file->cursor.position;
        pf_vol_set.last_error = PF_IERR_FILE_TOO_BIG;
        p_file->p_sfd->ffd.p_vol->last_error = PF_IERR_FILE_TOO_BIG;
        p_file->last_error = PF_IERR_FILE_TOO_BIG;
    }

    err = PFFILE_Cursor_WriteHeadSector(p_file, p_buf, size, &size_write);
    *p_size_write += size_write;
    if (err != 0) {
        return err;
    }
    if (size_write >= size) {
        return 0;
    }

    size -= size_write;
    PFFAT_CountAllocatedClusters(&p_file->p_sfd->ffd, p_file->cursor.position + size, &num_cluster);
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    size_request = ((p_file->cursor.position + size) > (num_cluster << (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster))) ? (p_file->cursor.position + size) - (num_cluster << (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster)) : 0;
    append_size = 0;

    while (size_request != 0 || size >= p_vol->bpb.bytes_per_sector) {
        if (size_request != 0) {
            err = PFCLUSTER_AppendCluster(p_file, size_request, &append_size);
            if (err != 0 && size == size_request) {
                return err;
            }
            if (append_size == 0) {
                size_request = 0;
            }
            size_request -= append_size;
        }
        while (size >= size_request + append_size && size >= p_vol->bpb.bytes_per_sector) {
            err = PFFILE_Cursor_WriteBodySectors(p_file, p_buf + *p_size_write, size, &size_write);
            *p_size_write += size_write;
            if (err != 0) {
                return err;
            }
            if (size_write == 0) {
                return 0;
            }
            size -= size_write;
            if (size == 0) {
                break;
            }
        }
    }

    err = PFFILE_Cursor_WriteTailSector(p_file, p_buf + *p_size_write, size, &size_write);
    *p_size_write += size_write;
    if (err != 0) {
        return err;
    }
    return 0;
}

void PFFILE_Cursor_MoveToEnd(struct PF_FILE* p_file) {
    p_file->cursor.position = p_file->p_sfd->dir_entry.file_size;
    PFFILE_Cursor_Recalc(p_file);
}

void PFFILE_Cursor_MoveToClusterEnd(struct PF_FILE* p_file, u32 size) {
    struct PF_VOLUME* p_vol;
    u32 cluster;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    PFFAT_CountAllocatedClusters(&p_file->p_sfd->ffd, size, &cluster);
    if (cluster > PF_FILE_SIZE_MAX >> (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster)) {
        p_file->cursor.position = PF_FILE_SIZE_MAX;
        PFFILE_Cursor_Recalc(p_file);
    } else {
        p_file->cursor.position = cluster << (p_vol->bpb.log2_bytes_per_sector + p_vol->bpb.log2_sectors_per_cluster);
        PFFILE_Cursor_Recalc(p_file);
    }
}

static void PFFILE_InitSFD(struct PF_SFD* p_sfd, struct PF_DIR_ENT* p_dir_entry) {
    p_sfd->stat = (PF_STAT_USED | PF_STAT_VALID);
    p_sfd->num_handlers = 0;
    p_sfd->dir_entry = *p_dir_entry;
    p_sfd->lock.mode = 0;
    p_sfd->lock.count = 0;
    p_sfd->lock.wcount = 0;
    p_sfd->lock.owner = NULL;
    p_sfd->lock.resource = 0;
    PFFAT_InitFFD(&p_sfd->ffd, NULL, p_dir_entry->p_vol, &p_sfd->dir_entry.start_cluster);
}

static inline void PFFILE_FinalizeSFD(struct PF_SFD* p_sfd) {
    p_sfd->stat = 0;
    PFFAT_FinalizeFFD(&p_sfd->ffd);
}

static inline void PFFILE_FinalizeUFD(struct PF_FILE* p_file) {
    p_file->stat &= ~PF_STAT_USED;
}

struct PF_SFD* PFFILE_GetSFD(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent) {
    u32 i;
    u32 sfd_num;
    struct PF_SFD* p_first_free_SFD;

    sfd_num = 0;
    p_first_free_SFD = NULL;
    for (i = 0; i < PF_SFD_COUNT; i++) {
        if ((p_vol->sfds[i].stat & PF_STAT_USED) == 0 || (p_vol->sfds[i].stat & PF_STAT_USED) != 0 && ((!p_vol->sfds[i].stat) & PF_STAT_VALID)) {
            if (p_first_free_SFD == NULL) {
                p_first_free_SFD = &p_vol->sfds[i];
                sfd_num = i;
            }
        } else if (p_ent->p_vol == p_vol->sfds[i].dir_entry.p_vol && p_ent->entry_sector == p_vol->sfds[i].dir_entry.entry_sector &&
                   p_ent->entry_offset == p_vol->sfds[i].dir_entry.entry_offset) {
            return &p_vol->sfds[i];
        }
    }
    if (p_first_free_SFD == NULL) {
        return NULL;
    }

    PFFILE_InitSFD(p_first_free_SFD, p_ent);
    if (p_vol->cluster_link.flag & PF_CLUSTER_LINK_FLAG_ENABLED) {
        p_first_free_SFD->ffd.cluster_link.buffer = &p_vol->cluster_link.buffer[sfd_num * p_vol->cluster_link.link_max];
        pf_memset(p_first_free_SFD->ffd.cluster_link.buffer, 0, p_vol->cluster_link.link_max * sizeof(u32));
        p_first_free_SFD->ffd.cluster_link.max_count = p_vol->cluster_link.link_max;
        p_first_free_SFD->ffd.cluster_link.interval = p_vol->cluster_link.interval;
        p_first_free_SFD->ffd.cluster_link.interval_offset = 0;
        p_first_free_SFD->ffd.cluster_link.position = 0;
        p_first_free_SFD->ffd.cluster_link.save_index = 0;
        if (p_ent->file_size != 0 && p_first_free_SFD->ffd.cluster_link.max_count != 0) {
            *p_first_free_SFD->ffd.cluster_link.buffer = p_ent->start_cluster;
            p_first_free_SFD->ffd.cluster_link.position++;
        }
    }
    return p_first_free_SFD;
}

static struct PF_FILE* PFFILE_GetFreeUFD(struct PF_VOLUME* p_vol) {
    s32 i;

    for (i = 0; i < PF_UFD_COUNT; i++) {
        if ((p_vol->ufds[i].stat & PF_STAT_USED) == 0) {
            return &p_vol->ufds[i];
        }
    }
    return NULL;
}

static inline void PFFILE_InitUFD(struct PF_FILE* p_file, s32 open_mode) {
    p_file->stat = PF_STAT_USED;
    p_file->open_mode = open_mode;
    p_file->last_error = 0;
    p_file->lock_count = 0;
    PFFAT_InitHint(&p_file->hint);
    PFFILE_Cursor_Initialize(p_file);
}

s32 PFFILE_createEmptyFile(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_file_ent, struct PF_STR* p_filename) {
    struct PF_DIR_ENT ent;
    struct PF_FFD ffd;
    u8 buf[PF_DIR_ENTRY_SIZE];
    struct PF_FAT_HINT hint;
    u32 NextChain[2];
    u32 success_size;
    u32 position;
    u32 namelength;
    u32* p_next_chain;
    u8 num_entry_LFNs;
    u32 i;
    u32 OldSector;
    u8 sum;
    s32 err;

    ent = *p_file_ent;
    namelength = PFSTR_StrNumChar(p_filename, PF_STR_TARGET_HEAD);
    if (namelength + ent.path_len > PF_PATH_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }
    if ((s32)namelength > PF_LONG_NAME_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }

    PFFAT_InitFFD(&ffd, &hint, ent.p_vol, &ent.start_cluster);
    if (p_vol->num_opened_files >= PF_UFD_COUNT) {
        return PF_IERR_TOO_MANY_FILES;
    }

    err = PFPATH_parseShortName(p_file_ent->short_name, p_filename);
    if (err != 0 && p_file_ent->short_name[0] == 0) {
        return PF_IERR_INVALID_NAME;
    }
    if (err != 0) {
        err = PFENT_AdjustSFN(&ent, p_file_ent->short_name);
        if (err != 0) {
            return err;
        }
        if (PFSTR_GetCodeMode(p_filename) == PF_STR_CODEMODE_OEM) {
            PFPATH_transformInUnicode((u16*)p_file_ent->long_name, PFSTR_GetStrPos(p_filename, PF_STR_TARGET_HEAD));
        } else {
            pf_w_strcpy((u16*)p_file_ent->long_name, (const u16*)PFSTR_GetStrPos(p_filename, PF_STR_TARGET_HEAD));
        }
    } else {
        p_file_ent->long_name[0] = 0;
    }

    if (p_vol->bpb.fat_type == FAT_32) {
        p_file_ent->start_cluster = 1;
    } else {
        p_file_ent->start_cluster = 0;
    }
    p_file_ent->file_size = 0;
    p_file_ent->p_vol = p_vol;
    p_file_ent->small_letter_flag = 0;
    p_file_ent->attr = PF_ATTR_ARCHIVE;
    p_file_ent->create_time_ms = PFENT_getcurrentDateTimeForEnt(&p_file_ent->create_date, &p_file_ent->create_time);
    p_file_ent->access_date = p_file_ent->create_date;
    p_file_ent->modify_time = p_file_ent->create_time;
    p_file_ent->modify_date = p_file_ent->create_date;

    if (p_file_ent->long_name[0] != 0 && (p_file_ent->small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        num_entry_LFNs = (s32)namelength / PF_LFN_CHARS_PER_ENTRY + ((s32)namelength % PF_LFN_CHARS_PER_ENTRY != 0);
        err = PFENT_allocateEntryPos(p_file_ent, num_entry_LFNs + 1, &ffd, NextChain, p_filename, &position);
        if (err != 0) {
            return err;
        }
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
            PFPATH_AdjustExtShortName(p_file_ent->short_name, position);
        }

        p_file_ent->num_entry_LFNs = num_entry_LFNs;
        OldSector = p_file_ent->entry_sector;
        sum = PFENT_CalcCheckSum(p_file_ent);
        p_next_chain = NextChain;
        for (i = num_entry_LFNs; i >= 1; i--) {
            PFENT_storeLFNEntryFieldsToBuf(buf, p_file_ent, i, sum, i == num_entry_LFNs);
            err = PFSEC_WriteData(p_vol, buf, OldSector, p_file_ent->entry_offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);
            if (err != 0) {
                return err;
            }
            if (success_size != PF_DIR_ENTRY_SIZE) {
                return PF_IERR_IO;
            }
            p_file_ent->entry_offset += PF_DIR_ENTRY_SIZE;
            if (p_file_ent->entry_offset >= p_vol->bpb.bytes_per_sector) {
                p_file_ent->entry_offset = 0;
                OldSector = *p_next_chain++;
            }
        }
        p_file_ent->entry_sector = OldSector;
    } else {
        err = PFENT_allocateEntry(p_file_ent, 1, &ffd, NextChain, p_filename);
        if (err != 0) {
            return err;
        }
    }

    if (p_file_ent->start_cluster == 1) {
        p_file_ent->start_cluster = 0;
    }
    return PFENT_updateEntry(p_file_ent, PFENT_UPDATE_SET_ARCHIVE);
}

static inline void PFFILE_EmptyFile(struct PF_FFD* p_ffd, struct PF_DIR_ENT* p_ent) {
    if (p_ent->start_cluster >= PF_CLUSTER_FIRST && p_ent->start_cluster != PF_INVALID_CLUSTER && p_ent->file_size != 0) {
        PFFAT_FreeChain(p_ffd, p_ent->start_cluster, PF_FAT_NO_CHAIN_INDEX, p_ent->file_size);
    }
    p_ent->start_cluster = 0;
    p_ent->file_size = 0;
}

s32 PFFILE_p_remove(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str) {
    struct PF_STR dir_str;
    struct PF_STR file_str;
    struct PF_FAT_HINT hint;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    u32 start_cluster;
    s32 err;

    err = PFPATH_SplitPath(p_path_str, &dir_str, &file_str);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path_str, FALSE);
    if (err != 0) {
        return err;
    }

    hint = *iter.ffd.p_hint;
    if (ent.attr & (PF_ATTR_READ_ONLY | PF_ATTR_VOLUME | PF_ATTR_DIR)) {
        return PF_IERR_WRITE_PROTECTED;
    }
    if (PFFILE_IsOpened(&ent)) {
        return PF_IERR_IN_USE;
    }

    iter.ffd.p_hint = &hint;
    start_cluster = ent.start_cluster;
    err = PFENT_RemoveEntry(&ent, &iter);
    if (err != 0) {
        return err;
    }
    return PFFAT_FreeChain(&iter.ffd, start_cluster, PF_FAT_NO_CHAIN_INDEX, ent.file_size);
}

s32 PFFILE_p_fopen(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, s32 mode, struct PF_FILE** pp_file) {
    u16 access_time;
    struct PF_STR file_str;
    struct PF_STR dir_str;
    struct PF_FAT_HINT hint;
    struct PF_FFD ffd;
    struct PF_ENT_ITER iter;
    s8 tmp_local[PF_LONG_NAME_OEM_BUF_LEN];
    struct PF_DIR_ENT ent;
    struct PF_SFD* p_sfd;
    struct PF_FILE* p_file;
    s32 err;
    s32 namelength;

    *pp_file = NULL;
    if (PFPATH_SplitPath(p_path_str, &dir_str, &file_str) != 0) {
        return PF_IERR_INVALID_NAME;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path_str, TRUE);
    if (err != 0) {
        return err;
    }
    if ((ent.attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_INVALID_NAME;
    }

    if (PFSTR_GetCodeMode(&file_str) == PF_STR_CODEMODE_UNICODE) {
        namelength = PFSTR_StrNumChar(&file_str, PF_STR_TARGET_HEAD);
        if (namelength > PF_LONG_NAME_LEN_MAX) {
            return PF_IERR_INVALID_NAME;
        }
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&file_str, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&file_str, tmp_local);

    if (mode & PF_OPEN_MODE_READ) {
        PFFAT_InitFFD(&ffd, &hint, ent.p_vol, &ent.start_cluster);
        if (PFENT_findEntry(&ffd, &ent, 0, &file_str, PF_ATTR_ALL, PF_ATTR_NONE) != 0) {
            return PF_IERR_NOT_FOUND;
        }
        if ((mode & PF_OPEN_MODE_PLUS) && (ent.attr & PF_ATTR_READ_ONLY)) {
            return PF_IERR_INVALID_PARAM;
        }
        if (ent.attr & PF_ATTR_DIR) {
            return PF_IERR_IS_DIR;
        }
        PFENT_getcurrentDateTimeForEnt(&ent.access_date, &access_time);
    } else {
        err = PFFILE_createEmptyFile(p_vol, &ent, &file_str);
        if (err == PF_IERR_EXISTS) {
            if (mode & PF_OPEN_MODE_NEW) {
                return PF_IERR_EXISTS;
            }
            if ((mode & PF_OPEN_MODE_WRITE) && PFFILE_IsOpened(&ent)) {
                return PF_IERR_EXISTS;
            }
            if ((ent.attr & PF_ATTR_READ_ONLY) && ((mode & PF_OPEN_MODE_WRITE) || (mode & PF_OPEN_MODE_APPEND) || (mode & PF_OPEN_MODE_PLUS))) {
                return PF_IERR_INVALID_PARAM;
            }
            if (ent.attr & PF_ATTR_DIR) {
                return PF_IERR_IS_DIR;
            }
            if (mode & PF_OPEN_MODE_WRITE) {
                PFFILE_EmptyFile(&iter.ffd, &ent);
                PFENT_getcurrentDateTimeForEnt(&ent.modify_date, &ent.modify_time);
                ent.access_date = ent.modify_date;
            } else {
                PFENT_getcurrentDateTimeForEnt(&ent.access_date, &access_time);
            }
        } else if (err != 0) {
            return err;
        }
    }

    p_vol = ent.p_vol;
    p_sfd = PFFILE_GetSFD(p_vol, &ent);
    if (p_sfd == NULL) {
        return PF_IERR_TOO_MANY_FILES;
    }
    p_file = PFFILE_GetFreeUFD(p_vol);
    if (p_file == NULL) {
        return PF_IERR_TOO_MANY_HANDLES;
    }

    p_file->p_sfd = p_sfd;
    PFFILE_InitUFD(p_file, mode);
    if (mode & PF_OPEN_MODE_WRITE) {
        p_file->p_sfd->stat |= PF_STAT_MODIFIED;
    }
    if (p_file->open_mode & PF_OPEN_MODE_APPEND) {
        PFFILE_Cursor_MoveToEnd(p_file);
    }
    p_sfd->num_handlers++;
    *pp_file = p_file;
    return 0;
}

s32 PFFILE_p_fread(struct PF_VOLUME* p_vol, u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_read) {
    u32 size_read;
    u32 total_size;
    s32 err;

    size_read = 0;
    *p_count_read = 0;

    if ((p_file->open_mode & PF_OPEN_MODE_PLUS) == 0 && ((p_file->open_mode & PF_OPEN_MODE_WRITE) != 0 || (p_file->open_mode & PF_OPEN_MODE_APPEND) != 0)) {
        return PF_IERR_INVALID_PARAM;
    }

    if ((p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) != 0 && p_file->lock_count == 0) {
        return PF_IERR_LOCKED;
    }

    if (p_file->cursor.position >= p_file->p_sfd->dir_entry.file_size) {
        return PF_IERR_INVALID_OPERATION;
    }

    total_size = size * count;
    err = PFFILE_Cursor_Read(p_file, p_buf, total_size, &size_read);
    *p_count_read = size_read / size;

    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFFILE_p_fwrite(struct PF_VOLUME* p_vol, u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_written) {
    u32 append_size;
    struct PF_CACHE_PAGE* p_page;
    u32 is_hit;
    u32 size_written;
    s32 err;

    size_written = 0;
    *p_count_written = 0;
    if ((p_file->open_mode & PF_OPEN_MODE_PLUS) == 0 && (p_file->open_mode & PF_OPEN_MODE_READ) != 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if ((p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) != 0 && ((p_file->p_sfd->lock.mode & PF_LOCK_SHARED) != 0 || (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE) != 0 && p_file->lock_count == 0)) {
        return PF_IERR_LOCKED;
    }

    if ((p_file->open_mode & PF_OPEN_MODE_APPEND) != 0) {
        PFFILE_Cursor_MoveToEnd(p_file);
    }

    if (p_file->cursor.position > p_file->p_sfd->dir_entry.file_size) {
        append_size = p_file->cursor.position - p_file->p_sfd->dir_entry.file_size;
        p_file->cursor.position = p_file->p_sfd->dir_entry.file_size;
        p_file->cursor.file_sector_index = p_file->cursor.position >> p_vol->bpb.log2_bytes_per_sector;
        p_file->cursor.offset_in_sector = p_file->cursor.position & (p_vol->bpb.bytes_per_sector - 1);
        err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
        if (err != 0) {
            return err;
        }
        pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);
        while (append_size != 0) {
            if (append_size > p_vol->bpb.bytes_per_sector) {
                err = PFFILE_Cursor_Write(p_file, p_page->p_buf, p_vol->bpb.bytes_per_sector, &size_written);
                if (err != 0) {
                    return err;
                }
            } else {
                err = PFFILE_Cursor_Write(p_file, p_page->p_buf, append_size, &size_written);
                if (err != 0) {
                    return err;
                }
                break;
            }
            append_size -= p_vol->bpb.bytes_per_sector;
        }
        PFCACHE_FreeDataPage(p_vol, p_page);
    }
    err = PFFILE_Cursor_Write(p_file, p_buf, size * count, &size_written);
    *p_count_written = size_written / size;
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFFILE_p_fappend(struct PF_FILE* p_file, u32 size, u32* p_size_appended) {
    s32 err;
    u32 size_appended;

    *p_size_appended = 0;
    if ((p_file->open_mode & PF_OPEN_MODE_PLUS) == 0 && (p_file->open_mode & PF_OPEN_MODE_READ) != 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if ((p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) != 0 && ((p_file->p_sfd->lock.mode & PF_LOCK_SHARED) != 0 || (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE) != 0 && p_file->lock_count == 0)) {
        return PF_IERR_LOCKED;
    }

    p_file->p_sfd->ffd.p_hint = &p_file->hint;
    if (PF_FILE_SIZE_MAX - p_file->cursor.position < size) {
        size = PF_FILE_SIZE_MAX - p_file->cursor.position;
        pf_vol_set.last_error = PF_IERR_FILE_TOO_BIG;
        p_file->p_sfd->ffd.p_vol->last_error = PF_IERR_FILE_TOO_BIG;
        p_file->last_error = PF_IERR_FILE_TOO_BIG;
    }
    err = PFCLUSTER_AppendCluster(p_file, size, &size_appended);
    *p_size_appended = size_appended;
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFFILE_p_finfo(struct PF_FILE* p_file, struct PF_INFO* p_info) {
    struct PF_VOLUME* p_vol;
    u32 cluster_size;
    s32 err;

    p_file->p_sfd->ffd.p_hint = &p_file->hint;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;

    p_info->file_size = p_file->p_sfd->dir_entry.file_size;
    p_info->io_pointer = p_file->cursor.position;

    cluster_size = p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;

    if (p_file->p_sfd->dir_entry.start_cluster != 0) {
        err = PFCLUSTER_GetAppendSize(p_file, &p_info->allocated_size);
        if (err != 0) {
            return err;
        }

        p_info->empty_size = p_info->allocated_size + ((p_file->p_sfd->dir_entry.file_size % cluster_size != 0) ? cluster_size - p_file->p_sfd->dir_entry.file_size % cluster_size : 0);
    } else {
        p_info->allocated_size = 0;
        p_info->empty_size = 0;
    }

    p_info->lock_mode = p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE);
    p_info->lock_owner = p_file->p_sfd->lock.owner;
    p_info->lock_count = p_file->lock_count;
    p_info->lock_tcount = p_file->p_sfd->lock.count;

    return 0;
}

s32 PFFILE_p_combine(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str1, struct PF_STR* p_path_str2) {
    struct PF_DIR_ENT ent1;
    struct PF_DIR_ENT ent2;
    struct PF_ENT_ITER iter1;
    struct PF_ENT_ITER iter2;
    struct PF_ENT_ITER iter_copy;
    struct PF_FAT_HINT hint1;
    struct PF_FAT_HINT hint2;
    s32 err;

    err = PFENT_ITER_GetEntryOfPath(&iter1, &ent1, p_vol, p_path_str1, FALSE);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter2, &ent2, p_vol, p_path_str2, FALSE);
    if (err != 0) {
        return err;
    }
    if ((ent1.attr & PF_ATTR_DIR) || (ent2.attr & PF_ATTR_DIR)) {
        return PF_IERR_IS_DIR;
    }
    if ((ent1.attr & PF_ATTR_READ_ONLY) || (ent2.attr & PF_ATTR_READ_ONLY)) {
        return PF_IERR_READ_ONLY;
    }
    if (PFFILE_IsOpened(&ent1) || PFFILE_IsOpened(&ent2)) {
        return PF_IERR_IN_USE;
    }
    if (ent1.entry_sector == ent2.entry_sector && ent1.entry_offset == ent2.entry_offset) {
        return PF_IERR_INVALID_PARAM;
    }
    if (PF_FILE_SIZE_MAX - ent1.file_size < ent2.file_size) {
        return PF_IERR_FILE_TOO_BIG;
    }

    PFFAT_InitHint(&hint1);
    PFFAT_InitFFD(&iter1.ffd, &hint1, p_vol, &ent1.start_cluster);
    PFFAT_InitHint(&hint2);
    PFFAT_InitFFD(&iter2.ffd, &hint2, p_vol, &ent2.start_cluster);
    pf_memcpy(&iter_copy, &iter2, sizeof(struct PF_ENT_ITER));

    err = PFENT_RemoveEntry(&ent2, &iter2);
    if (err != 0) {
        return err;
    }
    err = PFCLUSTER_CombineFiles(&iter1, &iter_copy, &ent1, &ent2);
    if (err != 0) {
        return err;
    }
    PFENT_getcurrentDateTimeForEnt(&ent1.modify_date, &ent1.modify_time);
    ent1.access_date = ent1.modify_date;
    err = PFENT_updateEntry(&ent1, PFENT_UPDATE_SET_ARCHIVE);
    return err ? err : 0;
}

s32 PFFILE_p_divide(struct PF_VOLUME* p_vol, struct PF_STR* p_src_str, struct PF_STR* p_dst_str, u32 offset) {
    struct PF_DIR_ENT ent_src;
    struct PF_STR file_str;
    struct PF_STR dir_str;
    struct PF_FAT_HINT hint_src;
    struct PF_FAT_HINT hint_dst;
    struct PF_ENT_ITER iter_src;
    struct PF_ENT_ITER iter_dst;
    s8 tmp_local[PF_LONG_NAME_OEM_BUF_LEN];
    struct PF_DIR_ENT ent_dst;
    struct PF_DIR_ENT ent_copy;
    u32 file_size;
    u32 start_cluster;
    s32 namelength;
    s32 err;

    err = PFENT_ITER_GetEntryOfPath(&iter_src, &ent_src, p_vol, p_src_str, FALSE);
    if (err != 0) {
        return err;
    }
    if (ent_src.attr & PF_ATTR_DIR) {
        return PF_IERR_IS_DIR;
    }
    if (ent_src.attr & PF_ATTR_READ_ONLY) {
        return PF_IERR_READ_ONLY;
    }
    if (PFFILE_IsOpened(&ent_src)) {
        return PF_IERR_IN_USE;
    }
    if (ent_src.file_size == 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if (ent_src.file_size < offset) {
        return PF_IERR_INVALID_PARAM;
    }

    if (PFPATH_SplitPath(p_dst_str, &dir_str, &file_str) != 0) {
        return PF_IERR_INVALID_NAME;
    }
    if (PFSTR_GetCodeMode(&file_str) == PF_STR_CODEMODE_UNICODE) {
        namelength = PFSTR_StrNumChar(&file_str, PF_STR_TARGET_HEAD);
        if (namelength > PF_LONG_NAME_LEN_MAX) {
            return PF_IERR_INVALID_NAME;
        }
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&file_str, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&file_str, tmp_local);

    err = PFENT_ITER_GetEntryOfPath(&iter_dst, &ent_dst, p_vol, p_dst_str, TRUE);
    if (err != 0) {
        return err;
    }
    if ((ent_dst.attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_INVALID_NAME;
    }
    err = PFFILE_createEmptyFile(p_vol, &ent_dst, &file_str);
    if (err != 0) {
        return err;
    }

    PFFAT_InitHint(&hint_src);
    PFFAT_InitFFD(&iter_src.ffd, &hint_src, p_vol, &ent_src.start_cluster);
    PFFAT_InitHint(&hint_dst);
    PFFAT_InitFFD(&iter_dst.ffd, &hint_dst, p_vol, &ent_dst.start_cluster);
    pf_memcpy(&ent_copy, &ent_src, sizeof(struct PF_DIR_ENT));

    file_size = ent_src.file_size;
    start_cluster = ent_src.start_cluster;
    PFENT_getcurrentDateTimeForEnt(&ent_src.modify_date, &ent_src.modify_time);
    ent_src.access_date = ent_src.modify_date;
    if (offset == 0) {
        ent_src.start_cluster = 0;
    }
    ent_src.file_size = offset;
    err = PFENT_updateEntry(&ent_src, PFENT_UPDATE_SET_ARCHIVE);
    if (err != 0) {
        return err;
    }
    if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
        err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
        if (err != 0) {
            return err;
        }
    }

    err = PFCLUSTER_DivideFile(&iter_src, &iter_dst, &ent_copy, &ent_dst, offset);
    if (err != 0) {
        PFCACHE_FlushFATCache(p_vol);
        ent_src.file_size = file_size;
        ent_src.start_cluster = start_cluster;
        PFENT_updateEntry(&ent_src, PFENT_UPDATE_SET_ARCHIVE);
        PFENT_ITER_GetEntryOfPath(&iter_dst, &ent_dst, p_vol, p_dst_str, FALSE);
        PFENT_RemoveEntry(&ent_dst, &iter_dst);
        return err;
    }

    ent_dst.modify_date = ent_src.modify_date;
    ent_dst.modify_time = ent_src.modify_time;
    ent_dst.access_date = ent_src.access_date;
    ent_dst.attr = ent_src.attr;
    err = PFENT_updateEntry(&ent_dst, PFENT_UPDATE_SET_ARCHIVE);
    return err ? err : 0;
}

s32 PFFILE_p_cinsert(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_inserted) {
    struct PF_FAT_HINT hint;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    u32 cluster_size;
    u32 num_cluster;
    s32 err;

    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path_str, FALSE);
    if (err != 0) {
        return err;
    }
    if (ent.attr & PF_ATTR_DIR) {
        return PF_IERR_IS_DIR;
    }
    if (ent.attr & PF_ATTR_READ_ONLY) {
        return PF_IERR_READ_ONLY;
    }
    if (PFFILE_IsOpened(&ent)) {
        return PF_IERR_IN_USE;
    }

    cluster_size = p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;
    num_cluster = ent.file_size / cluster_size;
    if (ent.file_size % cluster_size != 0) {
        num_cluster++;
    }
    if (num_cluster < start) {
        return PF_IERR_INVALID_PARAM;
    }

    PFFAT_InitHint(&hint);
    PFFAT_InitFFD(&iter.ffd, &hint, p_vol, &ent.start_cluster);
    err = PFCLUSTER_InsertCluster(&iter, &ent, start, num, p_num_inserted);
    if (err != 0) {
        return err;
    }
    PFENT_getcurrentDateTimeForEnt(&ent.modify_date, &ent.modify_time);
    ent.access_date = ent.modify_date;
    err = PFENT_updateEntry(&ent, PFENT_UPDATE_SET_ARCHIVE);
    return err ? err : 0;
}

s32 PFFILE_p_cdelete(struct PF_VOLUME* p_vol, struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_deleted) {
    struct PF_FAT_HINT hint;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    u32 cluster_size;
    u32 num_cluster;
    s32 err;

    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path_str, FALSE);
    if (err != 0) {
        return err;
    }
    if (ent.attr & PF_ATTR_DIR) {
        return PF_IERR_IS_DIR;
    }
    if (ent.attr & PF_ATTR_READ_ONLY) {
        return PF_IERR_READ_ONLY;
    }
    if (PFFILE_IsOpened(&ent)) {
        return PF_IERR_IN_USE;
    }
    if (ent.file_size == 0) {
        return PF_IERR_INVALID_PARAM;
    }

    cluster_size = p_vol->bpb.bytes_per_sector << p_vol->bpb.log2_sectors_per_cluster;
    num_cluster = ent.file_size / cluster_size;
    if (ent.file_size % cluster_size != 0) {
        num_cluster++;
    }
    if (num_cluster <= start) {
        return PF_IERR_INVALID_PARAM;
    }

    PFFAT_InitHint(&hint);
    PFFAT_InitFFD(&iter.ffd, &hint, p_vol, &ent.start_cluster);
    err = PFCLUSTER_DeleteCluster(&iter, &ent, start, num, p_num_deleted);
    return err ? err : 0;
}

s32 PFFILE_p_flock(struct PF_FILE* p_file, u32 mode) {
    if (mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) {
        if ((p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) && (mode & PF_LOCK_NO_WAIT) &&
            (((mode & PF_LOCK_SHARED) && (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE)) || ((mode & PF_LOCK_EXCLUSIVE) && (p_file->p_sfd->lock.mode & PF_LOCK_SHARED)) ||
             (p_file->p_sfd->lock.owner != NULL && p_file->p_sfd->lock.owner != p_file))) {
            return PF_IERR_LOCK_FAILED;
        }
        if (mode & PF_LOCK_SHARED) {
            if (p_file->p_sfd->lock.count == 0 || (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE)) {
                p_file->p_sfd->lock.wcount++;
                PF_LockFile(p_file, NULL);
                p_file->p_sfd->lock.wcount--;
                p_file->p_sfd->lock.mode = PF_LOCK_SHARED;
                p_file->p_sfd->lock.owner = NULL;
            }
        } else if (p_file->p_sfd->lock.owner != p_file) {
            p_file->p_sfd->lock.wcount++;
            PF_LockFile(p_file, p_file);
            p_file->p_sfd->lock.wcount--;
            p_file->p_sfd->lock.mode = PF_LOCK_EXCLUSIVE;
            p_file->p_sfd->lock.owner = p_file;
        }
        p_file->p_sfd->lock.count++;
        p_file->lock_count++;
    } else {
        if (p_file->lock_count == 0 || p_file->p_sfd->lock.count == 0) {
            return PF_IERR_LOCK_FAILED;
        }
        if ((p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE) && p_file->p_sfd->lock.owner != p_file) {
            return PF_IERR_LOCK_FAILED;
        }
        p_file->lock_count--;
        p_file->p_sfd->lock.count--;
        if (p_file->p_sfd->lock.count == 0) {
            PF_UnLockFile(p_file);
            if (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE) {
                p_file->p_sfd->lock.owner = NULL;
            }
            p_file->p_sfd->lock.mode &= ~(PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE);
        }
    }
    return 0;
}

s32 PFFILE_FsexecOpenFile(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 mode, struct PF_FILE** pp_file) {
    u16 access_time;
    struct PF_VOLUME* p_vol;
    struct PF_SFD* p_sfd;
    struct PF_FILE* p_file;

    p_vol = p_ent->p_vol;
    *pp_file = NULL;
    if (p_ent->attr & PF_ATTR_DIR) {
        return PF_IERR_IS_DIR;
    }
    if ((p_ent->attr & PF_ATTR_READ_ONLY) && ((mode & PF_OPEN_MODE_WRITE) || (mode & PF_OPEN_MODE_APPEND) || (mode & PF_OPEN_MODE_PLUS))) {
        return PF_IERR_INVALID_PARAM;
    }
    if ((mode & PF_OPEN_MODE_WRITE) && PFFILE_IsOpened(p_ent)) {
        return PF_IERR_EXISTS;
    }
    if (mode & PF_OPEN_MODE_WRITE) {
        PFFILE_EmptyFile(&p_iter->ffd, p_ent);
        PFENT_getcurrentDateTimeForEnt(&p_ent->modify_date, &p_ent->modify_time);
        p_ent->access_date = p_ent->modify_date;
    } else {
        PFENT_getcurrentDateTimeForEnt(&p_ent->access_date, &access_time);
    }

    p_sfd = PFFILE_GetSFD(p_vol, p_ent);
    if (p_sfd == NULL) {
        return PF_IERR_TOO_MANY_FILES;
    }
    p_file = PFFILE_GetFreeUFD(p_vol);
    if (p_file == NULL) {
        return PF_IERR_TOO_MANY_HANDLES;
    }

    p_file->p_sfd = p_sfd;
    PFFILE_InitUFD(p_file, mode);
    if (mode & PF_OPEN_MODE_WRITE) {
        p_file->p_sfd->stat |= PF_STAT_MODIFIED;
    }
    if (p_file->open_mode & PF_OPEN_MODE_APPEND) {
        PFFILE_Cursor_MoveToEnd(p_file);
    }
    p_sfd->num_handlers++;
    *pp_file = p_file;
    return 0;
}

BOOL PFFILE_IsOpened(struct PF_DIR_ENT* p_ent) {
    struct PF_SFD* p_sfd;
    u32 i;

    p_sfd = p_ent->p_vol->sfds;

    for (i = 0; i < PF_SFD_COUNT; i++) {
        if ((p_sfd[i].stat & PF_STAT_USED) && (p_sfd[i].stat & PF_STAT_VALID) && p_ent->p_vol == p_sfd[i].dir_entry.p_vol &&
            p_ent->entry_sector == p_sfd[i].dir_entry.entry_sector && p_ent->entry_offset == p_sfd[i].dir_entry.entry_offset) {
            return TRUE;
        }
    }
    return FALSE;
}

void PFFILE_FinalizeAllFiles(struct PF_VOLUME* p_vol) {
    u16 i;

    for (i = 0; i < PF_SFD_COUNT; i++) {
        PFFILE_FinalizeSFD(&p_vol->sfds[i]);
    }
    for (i = 0; i < PF_UFD_COUNT; i++) {
        PFFILE_FinalizeUFD(&p_vol->ufds[i]);
    }
    p_vol->num_opened_files = 0;
}

s32 PFFILE_remove(struct PF_STR* p_path_str) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path_str);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->cache.signature = NULL;
    err = PFFILE_p_remove(p_vol, p_path_str);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    err = PFCACHE_FlushFATCache(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFFILE_ferror(struct PF_FILE* p_file) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return PF_RET_ERR;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_RET_ERR;
    }
    return p_file->last_error;
}

s32 PFFILE_fopen(struct PF_STR* p_path_str, s32 mode, struct PF_FILE** pp_file) {
    struct PF_VOLUME* p_vol;
    s32 err;

    *pp_file = NULL;
    p_vol = PFPATH_GetVolumeFromPath(p_path_str);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0 && (err != PF_IERR_WRITE_PROTECTED || mode != PF_OPEN_MODE_READ)) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    err = PFFILE_p_fopen(p_vol, p_path_str, mode, pp_file);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        p_vol->num_opened_files++;
    }
    return err;
}

s32 PFFILE_fclose(struct PF_FILE* p_file) {
    struct PF_VOLUME* p_vol;
    s32 err;
    s32 result;

    result = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0 && (err != PF_IERR_WRITE_PROTECTED || p_file->open_mode != PF_OPEN_MODE_READ)) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }

    p_vol->cache.signature = p_file;
    if (p_file->p_sfd->num_handlers - 1 <= 0 && (p_file->p_sfd->dir_entry.attr & (PF_ATTR_READ_ONLY | PF_ATTR_VOLUME | PF_ATTR_DIR)) == 0 && (err & PF_IERR_WRITE_PROTECTED) == 0 &&
        ((p_file->p_sfd->stat & PF_STAT_MODIFIED) || (p_vol->file_config & PF_FILE_CONFIG_UPDATE_IF_MODIFIED) != PF_FILE_CONFIG_UPDATE_IF_MODIFIED)) {
        result = PFENT_updateEntry(&p_file->p_sfd->dir_entry, PFENT_UPDATE_SET_ARCHIVE);
    }

    if (p_file->lock_count != 0) {
        if (p_file->p_sfd->lock.mode & PF_LOCK_SHARED) {
            p_file->p_sfd->lock.count -= p_file->lock_count;
            p_file->lock_count = 0;
            if (p_file->p_sfd->lock.count == 0) {
                PF_UnLockFile(p_file);
            }
            p_file->p_sfd->lock.mode &= ~(PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE);
        } else if (p_file->p_sfd->lock.owner != p_file) {
            result = PF_IERR_LOCKED;
        } else {
            p_file->p_sfd->lock.count = 0;
            p_file->lock_count = 0;
            p_file->p_sfd->lock.owner = NULL;
            PF_UnLockFile(p_file);
            p_file->p_sfd->lock.mode &= ~(PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE);
        }
    }

    if (result != 0) {
        pf_vol_set.last_error = result;
        p_file->p_sfd->ffd.p_vol->last_error = result;
        p_file->last_error = result;
    } else {
        if (p_vol->cache.mode & PF_CACHE_MODE_FLUSH_ON_CLOSE) {
            result = PFCACHE_FlushFATCache(p_vol);
            if (result != 0) {
                pf_vol_set.last_error = result;
                p_file->p_sfd->ffd.p_vol->last_error = result;
                p_file->last_error = result;
            } else {
                result = PFCACHE_FlushDataCacheSpecific(p_vol, p_file);
                if (result != 0) {
                    pf_vol_set.last_error = result;
                    p_file->p_sfd->ffd.p_vol->last_error = result;
                    p_file->last_error = result;
                }
            }
        }
        if (result == 0) {
            if (--p_file->p_sfd->num_handlers == 0) {
                p_file->p_sfd->stat &= ~PF_STAT_USED;
                p_file->p_sfd->ffd.cluster_link.buffer = NULL;
            }
            p_file->stat &= ~PF_STAT_USED;
            p_vol->num_opened_files--;
        }
    }

    p_vol->cache.signature = NULL;
    return result;
}

s32 PFFILE_fread(u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_read) {
    struct PF_VOLUME* p_vol;
    u32 count_read;
    s32 err;

    *p_count_read = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    p_vol->cache.signature = p_file;
    err = PFFILE_p_fread(p_vol, p_buf, size, count, p_file, &count_read);
    *p_count_read = count_read;
    p_vol->cache.signature = NULL;
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
    }
    return err;
}

s32 PFFILE_fwrite(u8* p_buf, u32 size, u32 count, struct PF_FILE* p_file, u32* p_count_written) {
    struct PF_VOLUME* p_vol;
    u32 count_written;
    s32 err;

    *p_count_written = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    p_vol->cache.signature = p_file;
    err = PFFILE_p_fwrite(p_vol, p_buf, size, count, p_file, &count_written);
    *p_count_written = count_written;
    p_vol->cache.signature = NULL;
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
    }
    if (count_written != 0) {
        PFENT_getcurrentDateTimeForEnt(&p_file->p_sfd->dir_entry.modify_date, &p_file->p_sfd->dir_entry.modify_time);
        p_file->p_sfd->dir_entry.access_date = p_file->p_sfd->dir_entry.modify_date;
        p_file->p_sfd->stat |= PF_STAT_MODIFIED;
    }
    return err;
}

s32 PFFILE_feof(struct PF_FILE* p_file) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }
    if (p_file->cursor.position >= p_file->p_sfd->dir_entry.file_size) {
        err = TRUE;
    }
    return err;
}

s32 PFFILE_fseek(struct PF_FILE* p_file, s32 lOffset, s32 nOrigin) {
    struct PF_VOLUME* p_vol;
    s32 err;
    u32 file_io;
    u32 wk_offset;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    switch (nOrigin) {
        case PF_SEEK_CUR:
            file_io = p_file->cursor.position;
            break;
        case PF_SEEK_SET:
            file_io = 0;
            break;
        case PF_SEEK_END:
            file_io = p_file->p_sfd->dir_entry.file_size;
            break;
        default:
            pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
            p_file->p_sfd->ffd.p_vol->last_error = PF_IERR_INVALID_PARAM;
            p_file->last_error = PF_IERR_INVALID_PARAM;
            return PF_IERR_INVALID_PARAM;
    }

    if (lOffset & 0x80000000) {
        wk_offset = (~lOffset & 0x7FFFFFFF) + 1;
        if (file_io < wk_offset) {
            pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
            p_file->p_sfd->ffd.p_vol->last_error = PF_IERR_INVALID_PARAM;
            p_file->last_error = PF_IERR_INVALID_PARAM;
            return PF_IERR_INVALID_PARAM;
        }
        file_io -= wk_offset;
    } else {
        if (lOffset > PF_FILE_SIZE_MAX - file_io) {
            pf_vol_set.last_error = PF_IERR_FILE_TOO_BIG;
            p_file->p_sfd->ffd.p_vol->last_error = PF_IERR_FILE_TOO_BIG;
            p_file->last_error = PF_IERR_FILE_TOO_BIG;
            return PF_IERR_FILE_TOO_BIG;
        }
        file_io += lOffset;
    }
    PFFILE_Cursor_Initialize(p_file);
    p_file->cursor.position = file_io;
    PFFILE_Cursor_Recalc(p_file);
    return err;
}

s32 PFFILE_fsetclstlink(struct PF_FILE* p_file, u32 mode, struct PF_CLUSTER_LINK_RES* p_link) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    if (mode == PF_CLUSTER_LINK_ENABLE) {
        p_file->p_sfd->ffd.cluster_link.buffer = p_link->buffer;
        pf_memset(p_link->buffer, 0, p_link->max_count * sizeof(u32));
        p_file->p_sfd->ffd.cluster_link.max_count = p_link->max_count;
        p_file->p_sfd->ffd.cluster_link.interval = p_link->interval;
        p_file->p_sfd->ffd.cluster_link.interval_offset = 0;
        p_file->p_sfd->ffd.cluster_link.position = 0;
        p_file->p_sfd->ffd.cluster_link.save_index = 0;
        if (p_file->p_sfd->dir_entry.file_size != 0 && p_file->p_sfd->ffd.cluster_link.max_count != 0) {
            *p_file->p_sfd->ffd.cluster_link.buffer = *p_file->p_sfd->ffd.p_start_cluster;
            p_file->p_sfd->ffd.cluster_link.position++;
        }
        if (p_file->p_sfd->ffd.p_hint != NULL) {
            p_file->p_sfd->ffd.p_hint->chain_index = 0;
            p_file->p_sfd->ffd.p_hint->cluster = 0;
            p_file->p_sfd->ffd.p_hint->file_version = 0;
        }
        p_file->p_sfd->ffd.last_cluster.num_last_cluster = 0;
        p_file->p_sfd->ffd.last_cluster.max_chain_index = 0;
        p_file->p_sfd->ffd.last_access_cluster.chain_index = 0;
        p_file->p_sfd->ffd.last_access_cluster.cluster = 0;
    } else {
        p_file->p_sfd->ffd.cluster_link.buffer = NULL;
    }
    return err;
}

s32 PFFILE_fsync(struct PF_FILE* p_file) {
    u16 access_time;
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    err = PFCACHE_FlushFATCache(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    PFENT_getcurrentDateTimeForEnt(&p_file->p_sfd->dir_entry.access_date, &access_time);
    err = PFENT_updateEntry(&p_file->p_sfd->dir_entry, PFENT_UPDATE_SET_ARCHIVE);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    err = PFCACHE_FlushDataCacheSpecific(p_vol, p_file);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
    }
    return err;
}

s32 PFFILE_fappend(struct PF_FILE* p_file, u32 size, u32* p_size_appended) {
    u16 access_time;
    u32 size_appended;
    struct PF_VOLUME* p_vol;
    s32 err;

    *p_size_appended = 0;
    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    p_vol->cache.signature = p_file;
    err = PFFILE_p_fappend(p_file, size, &size_appended);
    *p_size_appended = size_appended;
    p_vol->cache.signature = NULL;
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    PFENT_getcurrentDateTimeForEnt(&p_file->p_sfd->dir_entry.access_date, &access_time);
    return 0;
}

static s32 PFFILE_p_fadjust(struct PF_FILE* p_file) {
    s32 err;

    if ((p_file->open_mode & PF_OPEN_MODE_PLUS) == 0 && (p_file->open_mode & PF_OPEN_MODE_READ) != 0) {
        return PF_IERR_INVALID_PARAM;
    }
    if ((p_file->p_sfd->lock.mode & (PF_LOCK_SHARED | PF_LOCK_EXCLUSIVE)) != 0 && ((p_file->p_sfd->lock.mode & PF_LOCK_SHARED) != 0 || (p_file->p_sfd->lock.mode & PF_LOCK_EXCLUSIVE) != 0 && p_file->lock_count == 0)) {
        return PF_IERR_LOCKED;
    }
    err = PFCLUSTER_AdjustCluster(p_file);
    return err ? err : 0;
}

s32 PFFILE_fadjust(struct PF_FILE* p_file) {
    struct PF_CURSOR save_cursor;
    struct PF_FAT_HINT save_hint;
    u16 access_time;
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    p_vol->cache.signature = p_file;
    save_cursor = p_file->cursor;
    save_hint = p_file->hint;
    err = PFFILE_p_fadjust(p_file);
    p_file->cursor = save_cursor;
    p_file->hint = save_hint;
    p_vol->cache.signature = NULL;
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    PFENT_getcurrentDateTimeForEnt(&p_file->p_sfd->dir_entry.access_date, &access_time);
    return 0;
}

s32 PFFILE_finfo(struct PF_FILE* p_file, struct PF_INFO* p_info) {
    struct PF_CURSOR save_cursor;
    struct PF_FAT_HINT save_hint;
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    p_vol->cache.signature = p_file;
    save_cursor = p_file->cursor;
    save_hint = p_file->hint;
    err = PFFILE_p_finfo(p_file, p_info);
    p_file->cursor = save_cursor;
    p_file->hint = save_hint;
    p_vol->cache.signature = NULL;
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_file->p_sfd->ffd.p_vol->last_error = err;
        p_file->last_error = err;
        return err;
    }
    return 0;
}

s32 PFFILE_combine(struct PF_STR* p_path_str1, struct PF_STR* p_path_str2) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path_str1);
    if (p_vol != PFPATH_GetVolumeFromPath(p_path_str2)) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->cache.signature = NULL;
    err = PFFILE_p_combine(p_vol, p_path_str1, p_path_str2);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        err = PFCACHE_FlushFATCache(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
        } else {
            err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
            if (err != 0) {
                pf_vol_set.last_error = err;
                p_vol->last_error = err;
            }
        }
    }
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFFILE_divide(struct PF_STR* p_src_str, struct PF_STR* p_dst_str, u32 offset) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_src_str);
    if (p_vol != PFPATH_GetVolumeFromPath(p_dst_str)) {
        pf_vol_set.last_error = PF_IERR_DIFFERENT_DRIVE;
        return PF_IERR_DIFFERENT_DRIVE;
    }
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->cache.signature = NULL;
    err = PFFILE_p_divide(p_vol, p_src_str, p_dst_str, offset);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        err = PFCACHE_FlushFATCache(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
        } else {
            err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
            if (err != 0) {
                pf_vol_set.last_error = err;
                p_vol->last_error = err;
            }
        }
    }
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFFILE_cinsert(struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_inserted) {
    struct PF_VOLUME* p_vol;
    s32 err;

    *p_num_inserted = 0;
    p_vol = PFPATH_GetVolumeFromPath(p_path_str);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->cache.signature = NULL;
    err = PFFILE_p_cinsert(p_vol, p_path_str, start, num, p_num_inserted);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        err = PFCACHE_FlushFATCache(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
        } else {
            err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
            if (err != 0) {
                pf_vol_set.last_error = err;
                p_vol->last_error = err;
            }
        }
    }
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFFILE_cdelete(struct PF_STR* p_path_str, u32 start, u32 num, u32* p_num_deleted) {
    struct PF_VOLUME* p_vol;
    s32 err;

    *p_num_deleted = 0;
    p_vol = PFPATH_GetVolumeFromPath(p_path_str);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->cache.signature = NULL;
    err = PFFILE_p_cdelete(p_vol, p_path_str, start, num, p_num_deleted);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
        } else {
            err = PFCACHE_FlushFATCache(p_vol);
            if (err != 0) {
                pf_vol_set.last_error = err;
                p_vol->last_error = err;
            }
        }
    }
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFFILE_flock(struct PF_FILE* p_file, u32 mode) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_file == NULL) ? NULL : p_file->p_sfd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_file->p_sfd->stat & PF_STAT_USED) == 0 || (p_file->p_sfd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_BAD_FILE;
        p_vol->last_error = PF_IERR_BAD_FILE;
        return PF_IERR_BAD_FILE;
    }

    err = PFFILE_p_flock(p_file, mode);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}
