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

static void PFDIR_FinalizeSDD(struct PF_SDD* p_sdd) {
    p_sdd->stat = 0;
    PFFAT_FinalizeFFD(&p_sdd->ffd);
}

static void PFDIR_FinalizeUDD(struct PF_DIR* p_dir) {
    p_dir->stat &= ~PF_STAT_USED;
}

static void PFDIR_SetDotEntry(u8* entry) {
    s32 i;

    entry[0] = '.';
    for (i = 1; i < PF_SFN_LEN; i++) {
        entry[i] = ' ';
    }
}

static void PFDIR_SetDotDotEntry(u8* entry) {
    s32 i;

    entry[0] = '.';
    entry[1] = '.';
    for (i = 2; i < PF_SFN_LEN; i++) {
        entry[i] = ' ';
    }
}

static BOOL PFDIR_IsOpened(struct PF_DIR_ENT* p_ent) {
    struct PF_SDD* p_sdd;
    u32 i;

    p_sdd = p_ent->p_vol->sdds;
    for (i = 0; i < PF_SDD_COUNT; i++) {
        if ((p_sdd[i].stat & PF_STAT_USED) && (p_sdd[i].stat & PF_STAT_VALID) && p_ent->p_vol == p_sdd[i].dir_entry.p_vol &&
            p_ent->entry_sector == p_sdd[i].dir_entry.entry_sector && p_ent->entry_offset == p_sdd[i].dir_entry.entry_offset) {
            return TRUE;
        }
    }
    return FALSE;
}

s32 PFDIR_CheckDirIsEmpty(struct PF_ENT_ITER* p_iter, BOOL* p_is_empty) {
    u32 index;
    s32 err;
    u8 attr;

    if (*p_iter->ffd.p_start_cluster == PF_CLUSTER_ROOT_DIR ||
        (p_iter->ffd.p_vol->bpb.fat_type == FAT_32 && *p_iter->ffd.p_start_cluster == p_iter->ffd.p_vol->bpb.root_dir_cluster)) {
        index = 0;
    } else {
        index = PFENT_NUM_DOT_ENTRIES;
    }

    *p_is_empty = TRUE;
    for (err = PFENT_ITER_MoveTo(p_iter, index, FALSE); !PFENT_ITER_IsAtLogicalEnd(p_iter); err = PFENT_ITER_Advance(p_iter, FALSE)) {
        if (err != 0) {
            return (err == PF_IERR_BROKEN_CHAIN) ? 0 : err;
        }
        if (p_iter->buf[0] != PF_ENTRY_DELETED) {
            attr = p_iter->buf[PF_DIRENT_ATTR];
            if ((attr & PF_ATTR_LONG_NAME) != PF_ATTR_LONG_NAME && (attr & PF_ATTR_VOLUME) == 0) {
                if (p_iter->buf[0] == PF_ENTRY_END) {
                    *p_is_empty = TRUE;
                } else {
                    *p_is_empty = FALSE;
                }
                break;
            }
        }
    }
    return 0;
}

struct PF_SDD* PFDIR_GetSDD(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_ent) {
    u32 i;
    struct PF_SDD* p_free_sdd;

    p_free_sdd = NULL;
    for (i = 0; i < PF_SDD_COUNT; i++) {
        if (!(p_vol->sdds[i].stat & PF_STAT_USED) || ((p_vol->sdds[i].stat & PF_STAT_USED) && ((!p_vol->sdds[i].stat) & PF_STAT_VALID))) {
            if (p_free_sdd == NULL) {
                p_free_sdd = &p_vol->sdds[i];
            }
        } else if (p_ent->p_vol == p_vol->sdds[i].dir_entry.p_vol && p_ent->entry_sector == p_vol->sdds[i].dir_entry.entry_sector &&
                   p_ent->entry_offset == p_vol->sdds[i].dir_entry.entry_offset) {
            return &p_vol->sdds[i];
        }
    }
    if (p_free_sdd == NULL) {
        return NULL;
    }

    p_free_sdd->stat = PF_STAT_USED | PF_STAT_VALID;
    p_free_sdd->num_handlers = 0;
    p_free_sdd->dir_entry = *p_ent;
    PFFAT_InitFFD(&p_free_sdd->ffd, NULL, p_ent->p_vol, &p_free_sdd->dir_entry.start_cluster);
    return p_free_sdd;
}

static struct PF_DIR* PFDIR_GetFreeUDD(struct PF_VOLUME* p_vol) {
    u32 i;

    for (i = 0; i < PF_UDD_COUNT; i++) {
        if ((p_vol->udds[i].stat & PF_STAT_USED) == 0) {
            return &p_vol->udds[i];
        }
    }
    return NULL;
}

s32 PFDIR_DoFsexecOpenDir(struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, struct PF_DIR** pp_dir) {
    struct PF_VOLUME* p_vol;
    struct PF_SDD* p_sdd;
    struct PF_DIR* p_dir;

    p_vol = p_ent->p_vol;
    *pp_dir = NULL;
    if ((p_ent->attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_NOT_DIR;
    }
    p_sdd = PFDIR_GetSDD(p_vol, p_ent);
    if (p_sdd == NULL) {
        return PF_IERR_TOO_MANY_FILES;
    }
    p_dir = PFDIR_GetFreeUDD(p_vol);
    if (p_dir == NULL) {
        return PF_IERR_TOO_MANY_HANDLES;
    }

    p_dir->p_sdd = p_sdd;
    p_dir->stat = PF_STAT_USED;
    PFFAT_InitHint(&p_dir->hint);
    p_dir->cursor.physical_entry_index = 0;
    p_dir->cursor.logical_entry_index = 0;
    p_dir->cursor.logical_seek_index = 0;
    p_sdd->num_handlers++;
    *pp_dir = p_dir;
    return 0;
}

s32 PFDIR_p_opendir(struct PF_VOLUME* p_vol, struct PF_STR* p_path, struct PF_DIR** pp_dir) {
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    struct PF_SDD* p_sdd;
    struct PF_DIR* p_dir;
    s32 err;

    if ((PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) &&
        PFSTR_StrNCmp(p_path, (const s8*)"", PF_STR_TARGET_HEAD, 1, 1) == 0) {
        err = PFENT_GetRootDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
    } else if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) == PF_DRIVE_SPEC_LEN && PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0) {
        PFVOL_GetCurrentDir(p_vol, &ent);
    } else if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) == PFENT_ROOT_PATH_LEN && PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0 &&
               (PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0)) {
        err = PFENT_GetRootDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
    } else {
        if (PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0 &&
            (PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0)) {
            PFSTR_MoveStrPos(p_path, PF_DRIVE_SPEC_LEN);
        }
        err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path, FALSE);
        if (err != 0) {
            return err;
        }
        if ((ent.attr & PF_ATTR_DIR) == 0) {
            return PF_IERR_INVALID_NAME;
        }
    }

    p_sdd = PFDIR_GetSDD(p_vol, &ent);
    if (p_sdd == NULL) {
        return PF_IERR_TOO_MANY_FILES;
    }
    p_dir = PFDIR_GetFreeUDD(p_vol);
    if (p_dir == NULL) {
        return PF_IERR_TOO_MANY_HANDLES;
    }

    p_dir->p_sdd = p_sdd;
    p_dir->stat = PF_STAT_USED;
    PFFAT_InitHint(&p_dir->hint);
    p_dir->cursor.physical_entry_index = 0;
    p_dir->cursor.logical_entry_index = 0;
    p_dir->cursor.logical_seek_index = 0;
    p_sdd->num_handlers++;
    *pp_dir = p_dir;
    return 0;
}

s32 PFDIR_p_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent) {
    u32 lpos;
    u32 ppos;
    struct PF_FAT_HINT hint;
    struct PF_STR str;
    struct PF_FFD ffd;
    struct PF_DIR_ENT ent;
    u32 count;
    s32 err;

    PFFAT_InitFFD(&ffd, &hint, p_dir->p_sdd->ffd.p_vol, &p_dir->p_sdd->dir_entry.start_cluster);
    count = p_dir->cursor.logical_seek_index;
    if (p_dir->cursor.logical_entry_index <= count) {
        count -= p_dir->cursor.logical_entry_index;
    } else if (p_dir->cursor.logical_entry_index > count) {
        p_dir->cursor.physical_entry_index = 0;
        p_dir->cursor.logical_entry_index = 0;
    }

    do {
        PFSTR_InitStr(&str, (const s8*)"*", PF_STR_CODEMODE_OEM);
        PFSTR_SetLocalStr(&str, NULL);
        err = PFENT_findEntryPos(&ffd, &ent, p_dir->cursor.physical_entry_index, &str, PF_ATTR_ALL, PF_ATTR_NONE, &lpos, &ppos);
        if (err != 0) {
            return err;
        }
        p_dir->cursor.physical_entry_index = ppos + 1;
        p_dir->cursor.logical_entry_index++;
    } while (count-- != 0);

    p_dir->cursor.logical_seek_index = p_dir->cursor.logical_entry_index;
    pf_strcpy(p_dirent->short_name, ent.short_name);
    if (ent.long_name[0] != 0) {
        PFPATH_transformFromUnicodeToNormal(p_dirent->long_name, ent.long_name);
    } else {
        p_dirent->long_name[0] = 0;
    }
    return 0;
}

s32 PFDIR_p_mkdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta) {
    u32* p_chain;
    struct PF_STR dir_path;
    struct PF_STR filename;
    u32 is_found;
    struct PF_DIR_ENT entry_dir;
    struct PF_ENT_ITER iter;
    u32 i;
    u16 filename_size;
    struct PF_DIR_ENT entry;
    u8 buf[PF_DIR_ENTRY_SIZE];
    u32 is_hit;
    struct PF_CACHE_PAGE* p_page;
    u8 num_entry_LFNs;
    u32 save_offset;
    u8 sum;
    s32 pozIdx;
    u32 NextChain[2];
    s32 err;
    struct PF_FFD ffd;
    struct PF_FAT_HINT fat_hint;
    u32 position;
    u32 SVSect;
    u32 success_size;
    u32 OldSector;
    s8 tmp_local[PF_LONG_NAME_OEM_BUF_LEN];
    u32 sector;

    num_entry_LFNs = 0;
    SVSect = 0;

    err = PFPATH_SplitPath(p_path, &dir_path, &filename);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &entry_dir, p_vol, &dir_path, TRUE);
    if (err != 0) {
        return err;
    }
    if ((entry_dir.attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_NOT_DIR;
    }
    filename_size = PFSTR_StrNumChar(&filename, PF_STR_TARGET_HEAD);
    if (filename_size + entry_dir.path_len > PF_PATH_LEN_MAX) {
        return PF_IERR_INVALID_PATH;
    }
    if (filename_size > PF_LONG_NAME_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }
    if (PFSTR_GetCodeMode(&filename) == PF_STR_CODEMODE_UNICODE) {
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&filename, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&filename, tmp_local);
    if (p_dta != NULL) {
        p_dta->p_file = NULL;
        p_dta->p_dir = NULL;
        p_dta->parent_start_cluster = entry_dir.start_cluster;
        p_dta->p_vol = entry_dir.p_vol;
        p_dta->attr = PF_ATTR_DIR;
        if (PFSTR_GetCodeMode(&filename) == PF_STR_CODEMODE_OEM) {
            PFPATH_SetSearchPattern(p_dta->reg_exp, NULL, &filename);
            p_dta->status |= PF_DTA_STAT_OEM;
        } else {
            PFPATH_SetSearchPattern(p_dta->reg_exp, p_dta->reg_exp_w, &filename);
            p_dta->status |= PF_DTA_STAT_UNICODE;
        }
    }

    PFFAT_InitFFD(&ffd, &fat_hint, p_vol, &entry_dir.start_cluster);
    iter.ffd = ffd;
    PFFAT_ResetFFD(&iter.ffd, &entry_dir.start_cluster);
    err = PFENT_ITER_IteratorInitialize(&iter, 0);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_FindEntry(&iter, &entry, &filename, PF_ATTR_DIR, PF_ATTR_NONE, &is_found);
    if (err != 0) {
        return err;
    }
    if (is_found != 0) {
        return PF_IERR_EXISTS;
    }

    pozIdx = PFPATH_parseShortName(entry.short_name, &filename);
    if (pozIdx != 0 && entry.short_name[0] == 0) {
        return PF_IERR_INVALID_NAME;
    }
    if (pozIdx != 0) {
        err = PFENT_AdjustSFN(&entry_dir, entry.short_name);
        if (err != 0) {
            return err;
        }
        if (PFSTR_GetCodeMode(&filename) == PF_STR_CODEMODE_OEM) {
            filename_size = PFPATH_transformInUnicode(entry.long_name, PFSTR_GetStrPos(&filename, PF_STR_TARGET_HEAD));
        } else {
            pf_w_strcpy(entry.long_name, (const u16*)PFSTR_GetStrPos(&filename, PF_STR_TARGET_HEAD));
        }
    } else {
        entry.long_name[0] = 0;
    }

    entry.start_cluster = 0;
    entry.file_size = 0;
    entry.p_vol = p_vol;
    entry.small_letter_flag = 0;
    entry.attr = PF_ATTR_DIR;
    entry.create_time_ms = PFENT_getcurrentDateTimeForEnt(&entry.create_date, &entry.create_time);
    entry.access_date = entry.create_date;
    entry.modify_time = entry.create_time;
    entry.modify_date = entry.create_date;
    save_offset = entry.entry_offset;
    if (entry.long_name[0] != 0 && (entry.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        num_entry_LFNs = (u32)filename_size / PF_LFN_CHARS_PER_ENTRY + ((u32)filename_size % PF_LFN_CHARS_PER_ENTRY != 0);
        err = PFENT_allocateEntryPos(&entry, num_entry_LFNs + 1, &ffd, NextChain, &filename, &OldSector);
        if (err != 0) {
            return err;
        }
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
            PFPATH_AdjustExtShortName(entry.short_name, OldSector);
        }
    } else {
        err = PFENT_allocateEntryPos(&entry, 1, &ffd, NextChain, &filename, &OldSector);
        if (err != 0) {
            return err;
        }
    }

    PFFAT_ResetFFD(&ffd, &entry.start_cluster);
    err = PFFAT_GetSectorSpecified(&ffd, 0, TRUE, &SVSect);
    if (err != 0) {
        return err;
    }
    if (SVSect == PF_INVALID_SECTOR) {
        return PF_IERR_NO_SPACE;
    }

    position = ffd.p_hint->cluster;
    if (p_vol->bpb.fat_type == FAT_32 && *iter.ffd.p_start_cluster == p_vol->bpb.root_dir_cluster && save_offset == 0 &&
        position == p_vol->bpb.root_dir_cluster) {
        iter.ffd = ffd;
        err = PFENT_ITER_IteratorInitialize(&iter, num_entry_LFNs);
        if (err != 0) {
            return err;
        }
        if (iter.buf[0] == PF_ENTRY_END) {
            PFFAT_ResetFFD(&ffd, &entry.start_cluster);
            *ffd.p_start_cluster = 0;
            ffd.p_hint->cluster = 0;
            position = p_vol->bpb.root_dir_cluster + 1;
            err = PFFAT_GetSectorSpecified(&ffd, 0, TRUE, &SVSect);
            if (err != 0) {
                return err;
            }
        }
    }

    err = PFCACHE_AllocateDataPage(p_vol, PF_INVALID_SECTOR, &p_page, &is_hit);
    if (err != 0) {
        return err;
    }
    pf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);
    for (i = p_vol->bpb.sectors_per_cluster; i != 0; i--) {
        if (i == 1) {
            PFDIR_SetDotEntry(p_page->p_buf);
            PFENT_StoreEntryNumericFieldsToBuf(p_page->p_buf, &entry);
            PFDIR_SetDotDotEntry(p_page->p_buf + PF_DIR_ENTRY_SIZE);
            if (entry_dir.entry_sector == PF_INVALID_SECTOR) {
                entry.start_cluster = 0;
            } else {
                entry.start_cluster = entry_dir.start_cluster;
            }
            PFENT_StoreEntryNumericFieldsToBuf(p_page->p_buf + PF_DIR_ENTRY_SIZE, &entry);
        }
        err = PFSEC_WriteData(p_vol, p_page->p_buf, i + SVSect - 1, 0, p_vol->bpb.bytes_per_sector, &success_size, FALSE);
        if (err != 0) {
            break;
        }
        if (success_size != p_vol->bpb.bytes_per_sector) {
            err = PF_IERR_IO;
            break;
        }
    }
    PFCACHE_FreeDataPage(p_vol, p_page);
    if (err != 0) {
        return err;
    }

    entry.num_entry_LFNs = num_entry_LFNs;
    sector = entry.entry_sector;
    sum = PFENT_CalcCheckSum(&entry);
    if ((entry_dir.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        p_chain = NextChain;
        for (i = num_entry_LFNs; i >= 1; i--) {
            PFENT_storeLFNEntryFieldsToBuf(buf, &entry, i, sum, i == num_entry_LFNs);
            err = PFSEC_WriteData(p_vol, buf, sector, entry.entry_offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);
            if (err != 0) {
                return err;
            }
            if (success_size != PF_DIR_ENTRY_SIZE) {
                return PF_IERR_IO;
            }
            entry.entry_offset += PF_DIR_ENTRY_SIZE;
            if (entry.entry_offset >= p_vol->bpb.bytes_per_sector) {
                entry.entry_offset = 0;
                sector = *p_chain++;
            }
        }
        entry.entry_sector = sector;
    }
    entry.start_cluster = position;
    err = PFENT_updateEntry(&entry, PFENT_UPDATE_KEEP_ATTR);
    if (err != 0) {
        return err;
    }

    if (p_dta != NULL) {
        p_dta->parent_pos = OldSector + 1;
        pf_strcpy(p_dta->FileName, entry.short_name);
        if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
            PFPATH_transformInUnicode(p_dta->FileName_w, p_dta->FileName);
        }
        if (entry.long_name[0] != 0) {
            if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
                pf_vol_set.setting = (pf_vol_set.setting & ~PF_ENCODE_MODE_MASK) | PF_ENCODE_MODE_OEM;
                PFPATH_transformFromUnicodeToNormal(p_dta->LongName, entry.long_name);
                pf_vol_set.setting = (pf_vol_set.setting & ~PF_ENCODE_MODE_MASK) | PF_ENCODE_MODE_UNICODE;
            } else {
                PFPATH_transformFromUnicodeToNormal(p_dta->LongName, entry.long_name);
            }
            if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
                pf_w_strcpy(p_dta->LongName_w, entry.long_name);
            }
            p_dta->check_sum = sum;
            p_dta->ordinal = PF_LFN_FIRST_ORDINAL;
        } else {
            p_dta->LongName[0] = 0;
            if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
                p_dta->LongName_w[0] = 0;
            }
            p_dta->ordinal = 0;
            p_dta->check_sum = 0;
        }
        p_dta->FileSize = entry.file_size;
        p_dta->Time = entry.modify_time;
        p_dta->Date = entry.modify_date;
        p_dta->Attribute = entry.attr;
        p_dta->num_entry_LFNs = entry.num_entry_LFNs;
    }
    return 0;
}

s32 PFDIR_p_chdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path) {
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    s32 err;

    if ((PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, 0, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, 0, 1) == 0) &&
        PFSTR_StrNCmp(p_path, (const s8*)"", PF_STR_TARGET_HEAD, 1, 1) == 0) {
        err = PFENT_GetRootDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
        err = PFVOL_SetCurrentDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
        return 0;
    }
    if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) == PF_DRIVE_SPEC_LEN && PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0) {
        return 0;
    }
    if (PFSTR_StrNumChar(p_path, PF_STR_TARGET_HEAD) == PFENT_ROOT_PATH_LEN && PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0 &&
        (PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0)) {
        err = PFENT_GetRootDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
        err = PFVOL_SetCurrentDir(p_vol, &ent);
        if (err != 0) {
            return err;
        }
        return 0;
    }

    if (PFSTR_StrNCmp(p_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0 &&
        (PFSTR_StrNCmp(p_path, (const s8*)"\\", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0 || PFSTR_StrNCmp(p_path, (const s8*)"/", PF_STR_TARGET_HEAD, PF_DRIVE_SPEC_LEN, 1) == 0)) {
        PFSTR_MoveStrPos(p_path, PF_DRIVE_SPEC_LEN);
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path, FALSE);
    if (err != 0) {
        return err;
    }
    if ((ent.attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_NOT_DIR;
    }
    err = PFVOL_SetCurrentDir(p_vol, &ent);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_p_chmod(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u8 attr) {
    u16 access_time;
    struct PF_STR dir_str;
    struct PF_STR file_str;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    u32 is_dir;
    s32 err;

    err = PFPATH_SplitPath(p_path, &dir_str, &file_str);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path, FALSE);
    if (err != 0) {
        return err;
    }

    is_dir = ent.attr & PF_ATTR_DIR;
    if ((is_dir && (attr & PF_ATTR_DIR) == 0) || (!is_dir && (attr & PF_ATTR_DIR))) {
        return PF_IERR_INVALID_PARAM;
    }
    if (attr & PF_ATTR_NORMAL) {
        attr &= ~PF_ATTR_NORMAL;
    }

    if (is_dir) {
        if (PFDIR_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
        if ((attr & PF_ATTR_SYSTEM) && (attr & PF_ATTR_LONG_NAME) != PF_ATTR_LONG_NAME) {
            return PF_IERR_INVALID_PARAM;
        }
        ent.attr = attr | PF_ATTR_DIR;
    } else {
        if (PFFILE_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
        ent.attr = attr;
    }

    PFENT_getcurrentDateTimeForEnt(&ent.access_date, &access_time);
    if (ent.file_size == 0 && (ent.attr & PF_ATTR_DIR) == 0) {
        ent.start_cluster = 0;
    }
    err = PFENT_updateEntry(&ent, PFENT_UPDATE_KEEP_ATTR);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_p_rmdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path) {
    BOOL is_empty;
    struct PF_STR dir_str;
    struct PF_STR file_str;
    struct PF_FAT_HINT hint;
    struct PF_FAT_HINT save_hint;
    struct PF_FFD ffd;
    struct PF_ENT_ITER iter;
    struct PF_ENT_ITER save_iter;
    struct PF_DIR_ENT ent;
    u32 start_cluster;
    s32 err;

    err = PFPATH_SplitPath(p_path, &dir_str, &file_str);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path, FALSE);
    if (err != 0) {
        return err;
    }

    save_iter = iter;
    save_hint = *iter.ffd.p_hint;
    if ((ent.attr & PF_ATTR_DIR) == 0 || (ent.attr & PF_ATTR_READ_ONLY)) {
        return PF_IERR_NOT_DIR;
    }
    if (PFDIR_IsOpened(&ent)) {
        return PF_IERR_IN_USE;
    }
    if (PFVOL_CheckCurrentDir(p_vol, ent.start_cluster) != 0 || ent.start_cluster == PF_CLUSTER_ROOT_DIR ||
        (p_vol->bpb.fat_type == FAT_32 && ent.start_cluster == p_vol->bpb.root_dir_cluster)) {
        return PF_IERR_INVALID_OPERATION;
    }

    PFFAT_InitFFD(&ffd, &hint, p_vol, &ent.start_cluster);
    iter.ffd = ffd;
    err = PFENT_ITER_IteratorInitialize(&iter, 0);
    if (err != 0) {
        return err;
    }
    err = PFDIR_CheckDirIsEmpty(&iter, &is_empty);
    if (err != 0) {
        return err;
    }
    if (!is_empty) {
        return PF_IERR_NOT_EMPTY;
    }

    start_cluster = ent.start_cluster;
    iter = save_iter;
    iter.ffd.p_hint = &save_hint;
    err = PFENT_RemoveEntry(&ent, &iter);
    if (err != 0) {
        return err;
    }
    err = PFFAT_FreeChain(&iter.ffd, start_cluster, PF_FAT_NO_CHAIN_INDEX, PF_FAT_WHOLE_CHAIN);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_p_fstat(struct PF_VOLUME* p_vol, struct PF_STR* p_path, struct PF_STAT* p_stat) {
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    s32 i;
    s32 err;

    i = 0;
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_path, FALSE);
    if (err != 0) {
        return err;
    }

    if (p_vol->num_opened_files != 0) {
        for (i = 0; i < PF_SFD_COUNT; i++) {
            if ((p_vol->sfds[i].stat & PF_STAT_USED) && (p_vol->sfds[i].stat & PF_STAT_VALID) && ent.p_vol == p_vol->sfds[i].dir_entry.p_vol &&
                ent.entry_sector == p_vol->sfds[i].dir_entry.entry_sector && ent.entry_offset == p_vol->sfds[i].dir_entry.entry_offset) {
                p_stat->file_size = p_vol->sfds[i].dir_entry.file_size;
                p_stat->attr = p_vol->sfds[i].dir_entry.attr;
                p_stat->modify_time = p_vol->sfds[i].dir_entry.modify_time;
                p_stat->modify_date = p_vol->sfds[i].dir_entry.modify_date;
                p_stat->access_date = p_vol->sfds[i].dir_entry.access_date;
                p_stat->create_time = p_vol->sfds[i].dir_entry.create_time;
                p_stat->create_date = p_vol->sfds[i].dir_entry.create_date;
                p_stat->create_time_ms = p_vol->sfds[i].dir_entry.create_time_ms;
                break;
            }
        }
    }

    if (p_vol->num_opened_files == 0 || i == PF_SFD_COUNT) {
        if (ent.attr & PF_ATTR_DIR) {
            p_stat->file_size = 0;
        } else {
            p_stat->file_size = ent.file_size;
        }
        p_stat->attr = ent.attr;
        p_stat->modify_time = ent.modify_time;
        p_stat->modify_date = ent.modify_date;
        p_stat->access_date = ent.access_date;
        p_stat->create_time = ent.create_time;
        p_stat->create_date = ent.create_date;
        p_stat->create_time_ms = ent.create_time_ms;
    }
    return 0;
}

static BOOL PFDIR_IsCurrentDir(struct PF_DIR_ENT* p_ent) {
    struct PF_VOLUME* p_vol;
    struct PF_CUR_DIR* p_cur;
    u32 i;
    u32 j;

    for (i = 0; i < PF_CONTEXT_COUNT; i++) {
        p_vol = pf_vol_set.current_vol[i].p_vol;
        if (p_ent->p_vol == p_vol) {
            p_cur = p_vol->current_dir;
            for (j = 0; j < PF_CONTEXT_COUNT; j++, p_cur++) {
                if ((p_cur->stat & PF_STAT_USED) && p_cur->directory.start_cluster == p_ent->start_cluster) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

s32 PFDIR_p_rename(struct PF_VOLUME* p_vol, struct PF_STR* p_old_path, struct PF_STR* p_new_path) {
    struct PF_DIR_ENT ent;
    struct PF_DIR_ENT new_ent;
    struct PF_DIR_ENT ent_dir;
    s8 tmp_local[PF_LONG_NAME_OEM_BUF_LEN];
    struct PF_ENT_ITER iter;
    struct PF_ENT_ITER new_iter;
    u8 buf[PF_DIR_ENTRY_SIZE];
    struct PF_STR dir_str;
    struct PF_STR old_name;
    struct PF_STR new_name;
    struct PF_FAT_HINT hint;
    struct PF_FAT_HINT new_hint;
    u32 next_chain[2];
    u32 success_size;
    u32 pos;
    u32 is_found;
    u16 access_time;
    u8 del_mark[1] = {PF_ENTRY_DELETED};
    struct PF_SFD* p_sfd;
    u32 i;
    u32 ord;
    u32 lfn_ord;
    u32* p_chain;
    u32 lfn_sector;
    struct PF_DIR_ENT* p_ent;
    u32 sector;
    s32 name_len;
    s32 pozIdx;
    s32 result;
    u16 saved_first_char;
    u8 num_lfn;
    s32 cmp;
    s32 err;

    result = 0;
    saved_first_char = 0;
    err = PFPATH_SplitPath(p_old_path, &dir_str, &old_name);
    if (err != 0) {
        return err;
    }
    if (PFSTR_GetCodeMode(&old_name) == PF_STR_CODEMODE_UNICODE) {
        if ((s32)PFSTR_StrNumChar(&old_name, PF_STR_TARGET_HEAD) > PF_LONG_NAME_LEN_MAX) {
            return PF_IERR_INVALID_NAME;
        }
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&old_name, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&old_name, tmp_local);

    err = PFENT_ITER_GetEntryOfPath(&iter, &ent_dir, p_vol, p_old_path, TRUE);
    if (err != 0) {
        return err;
    }
    PFFAT_InitFFD(&iter.ffd, &hint, p_vol, &iter.ffd.start_cluster);
    iter.ffd.start_cluster = ent_dir.start_cluster;
    err = PFENT_ITER_IteratorInitialize(&iter, 0);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_FindEntry(&iter, &ent, &old_name, PF_ATTR_ALL, PF_ATTR_NONE, &is_found);
    if (err != 0) {
        return err;
    }
    if (!is_found) {
        return PF_IERR_NOT_FOUND;
    }
    if (ent.attr & PF_ATTR_VOLUME) {
        return PF_IERR_NOT_FOUND;
    }
    if (ent.attr & PF_ATTR_READ_ONLY) {
        return PF_IERR_READ_ONLY;
    }
    if (PFSTR_StrNCmp(p_new_path, (const s8*)":", PF_STR_TARGET_HEAD, PF_DRIVE_LETTER_LEN, 1) == 0 && p_vol != PFPATH_GetVolumeFromPath(p_new_path)) {
        return PF_IERR_DIFFERENT_DRIVE;
    }

    err = PFPATH_SplitPath(p_new_path, &dir_str, &new_name);
    if (err != 0) {
        return err;
    }
    if (PFSTR_GetCodeMode(&new_name) == PF_STR_CODEMODE_UNICODE) {
        if ((s32)PFSTR_StrNumChar(&new_name, PF_STR_TARGET_HEAD) > PF_LONG_NAME_LEN_MAX) {
            return PF_IERR_INVALID_NAME;
        }
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&new_name, tmp_local);

    if (PFSTR_GetCodeMode(&new_name) == PF_STR_CODEMODE_OEM) {
        cmp = pf_strcmp(PFSTR_GetStrPos(&dir_str, PF_STR_TARGET_HEAD), PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
    } else {
        cmp = pf_w_strcmp((const u16*)PFSTR_GetStrPos(&dir_str, PF_STR_TARGET_HEAD), (const u16*)PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
    }
    if (cmp != 0) {
        err = PFENT_ITER_GetEntryOfPath(&new_iter, &new_ent, p_vol, p_new_path, TRUE);
        if (err != 0) {
            return err;
        }
        if (new_ent.start_cluster != ent_dir.start_cluster) {
            return PF_IERR_DIFFERENT_DIR;
        }
    }

    new_ent.start_cluster = ent_dir.start_cluster;
    PFFAT_InitFFD(&new_iter.ffd, &new_hint, p_vol, &new_ent.start_cluster);
    err = PFENT_findEntry(&new_iter.ffd, &new_ent, 0, &new_name, PF_ATTR_ALL, PF_ATTR_NONE);
    if (err == 0) {
        return PF_IERR_EXISTS;
    } else if (err != 0 && err != PF_IERR_NOT_FOUND) {
        return err;
    }

    if ((ent.attr & PF_ATTR_DIR) == 0) {
        if (PFFILE_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
    } else {
        if (PFDIR_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
        if (PFDIR_IsCurrentDir(&ent)) {
            return PF_IERR_INVALID_OPERATION;
        }
        if (PFSTR_StrNCmp(&old_name, (const s8*)"..", PF_STR_TARGET_HEAD, 0, 2) == 0) {
            return PF_IERR_INVALID_OPERATION;
        }
        if (PFSTR_StrNCmp(&old_name, (const s8*)".", PF_STR_TARGET_HEAD, 0, 1) == 0) {
            return PF_IERR_INVALID_OPERATION;
        }

        p_sfd = p_vol->sfds;
        sector = p_vol->bpb.first_data_sector + ((ent.start_cluster - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);
        for (i = 0; i < PF_SFD_COUNT; i++, p_sfd++) {
            if ((p_sfd->stat & PF_STAT_USED) && (p_sfd->stat & PF_STAT_VALID)) {
                if (p_vol == p_sfd->dir_entry.p_vol && sector == p_sfd->dir_entry.entry_sector) {
                    return PF_IERR_IN_USE;
                }
                is_found = 0;
                err = PFENT_ITER_FindCluster(&p_sfd->dir_entry, ent.start_cluster, &is_found);
                if (err != 0) {
                    return err;
                }
                if (is_found) {
                    return PF_IERR_IN_USE;
                }
            }
        }
    }

    name_len = PFSTR_StrNumChar(&new_name, PF_STR_TARGET_HEAD);
    if (name_len + ent_dir.path_len > PF_PATH_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }
    if ((ent.attr & PF_ATTR_DIR) == 0 && name_len > PF_LONG_NAME_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }

    pf_memcpy(&new_ent, &ent, sizeof(struct PF_DIR_ENT));
    pozIdx = PFPATH_parseShortName(new_ent.short_name, &new_name);
    if (new_ent.short_name[0] == 0) {
        return PF_IERR_INVALID_PATH;
    }
    if (pozIdx == 0 && ent.num_entry_LFNs == 0) {
        new_ent.long_name[0] = 0;
        new_ent.num_entry_LFNs = 0;
        PFENT_getcurrentDateTimeForEnt(&new_ent.access_date, &access_time);
        err = PFENT_updateEntry(&new_ent, PFENT_UPDATE_SET_ARCHIVE);
        if (err != 0) {
            return err;
        }
    } else {
        if (ent.num_entry_LFNs != 0 && (ent.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
            saved_first_char = ent.long_name[0];
            ent.long_name[0] = 0;
            for (ord = 1; ord <= ent.num_entry_LFNs; ord++) {
                err = PFENT_ITER_Retreat(&iter, FALSE);
                if (err != 0) {
                    return err;
                }
                err = PFSEC_WriteData(p_vol, del_mark, iter.sector, iter.offset, 1, &success_size, FALSE);
                if (err != 0) {
                    return err;
                }
                if (success_size != 1) {
                    return PF_IERR_IO;
                }
            }
        }
        err = PFSEC_WriteData(p_vol, del_mark, ent.entry_sector, ent.entry_offset, 1, &success_size, FALSE);
        if (err != 0) {
            return err;
        }
        if (success_size != 1) {
            return PF_IERR_IO;
        }
        if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
            err = PFCACHE_FlushDataCache(p_vol);
            if (err != 0) {
                return err;
            }
        }

        if (pozIdx != 0) {
            err = PFENT_AdjustSFN(&ent_dir, new_ent.short_name);
            if (err != 0) {
                return err;
            }
            if (PFSTR_GetCodeMode(&new_name) == PF_STR_CODEMODE_OEM) {
                name_len = PFPATH_transformInUnicode(new_ent.long_name, PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
            } else {
                pf_w_strcpy(new_ent.long_name, (const u16*)PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
            }
            new_ent.num_entry_LFNs = name_len / PF_LFN_CHARS_PER_ENTRY + (name_len % PF_LFN_CHARS_PER_ENTRY != 0);
        } else {
            new_ent.num_entry_LFNs = 0;
        }
        new_ent.small_letter_flag = 0;

        PFFAT_InitFFD(&iter.ffd, &new_hint, p_vol, &ent_dir.start_cluster);
        result = PFENT_allocateEntryPos(&new_ent, new_ent.num_entry_LFNs + 1, &iter.ffd, next_chain, &new_name, &pos);
        if (result != 0) {
            num_lfn = ent.num_entry_LFNs;
            if (num_lfn != 0 && (ent.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
                ent.long_name[0] = saved_first_char;
                ent.entry_offset -= num_lfn * PF_DIR_ENTRY_SIZE;
            }
            p_ent = &ent;
        } else {
            if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
                PFPATH_AdjustExtShortName(new_ent.short_name, pos);
            }
            new_ent.check_sum = PFENT_CalcCheckSum(&new_ent);
            PFENT_getcurrentDateTimeForEnt(&new_ent.access_date, &access_time);
            p_ent = &new_ent;
        }

        if (p_ent->num_entry_LFNs != 0 && (p_ent->small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
            lfn_sector = p_ent->entry_sector;
            p_chain = next_chain;
            for (lfn_ord = p_ent->num_entry_LFNs; lfn_ord >= 1; lfn_ord--) {
                PFENT_storeLFNEntryFieldsToBuf(buf, p_ent, lfn_ord, p_ent->check_sum, lfn_ord == p_ent->num_entry_LFNs);
                err = PFSEC_WriteData(p_vol, buf, lfn_sector, p_ent->entry_offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);
                if (err != 0) {
                    return err;
                }
                if (success_size != PF_DIR_ENTRY_SIZE) {
                    return PF_IERR_IO;
                }
                p_ent->entry_offset += PF_DIR_ENTRY_SIZE;
                if (p_ent->entry_offset >= p_vol->bpb.bytes_per_sector) {
                    p_ent->entry_offset = 0;
                    lfn_sector = *p_chain++;
                }
            }
            p_ent->entry_sector = lfn_sector;
        }
        err = PFENT_updateEntry(p_ent, PFENT_UPDATE_SET_ARCHIVE);
        if (err != 0) {
            return err;
        }
    }
    return result;
}

s32 PFDIR_p_move(struct PF_VOLUME* p_vol, struct PF_STR* p_src_path, struct PF_STR* p_dst_path) {
    u32* p_chain;
    u32 cluster;
    struct PF_DIR_ENT ent;
    struct PF_DIR_ENT new_ent;
    struct PF_DIR_ENT dst_dir;
    s8 tmp_local[PF_LONG_NAME_OEM_BUF_LEN];
    struct PF_ENT_ITER iter;
    struct PF_ENT_ITER dst_iter;
    u8 buf[PF_DIR_ENTRY_SIZE];
    struct PF_STR dir_str;
    struct PF_STR old_name;
    struct PF_STR new_name;
    struct PF_STR dotdot;
    struct PF_FAT_HINT new_hint;
    struct PF_FAT_HINT dst_hint;
    u32 next_chain[2];
    u32 success_size;
    u32 pos;
    u32 is_found;
    u16 access_time;
    u8 del_mark[1] = {PF_ENTRY_DELETED};
    struct PF_SFD* p_sfd;
    u32 lfn_sector;
    u32 sector;
    u32 lfn_ord;
    u32 i;
    u32 ord;
    s32 name_len;
    s32 pozIdx;
    u16 saved_first_char;
    u8 num_lfn;
    struct PF_DIR_ENT* p_ent;
    s32 result;
    s32 err;

    result = 0;
    saved_first_char = 0;
    err = PFENT_ITER_GetEntryOfPath(&iter, &ent, p_vol, p_src_path, FALSE);
    if (err != 0) {
        return err;
    }
    if (p_vol != PFPATH_GetVolumeFromPath(p_dst_path)) {
        return PF_IERR_DIFFERENT_DRIVE;
    }

    err = PFPATH_SplitPath(p_dst_path, &dir_str, &new_name);
    if (err != 0) {
        return err;
    }
    if (PFSTR_GetCodeMode(&new_name) == PF_STR_CODEMODE_UNICODE) {
        if ((s32)PFSTR_StrNumChar(&new_name, PF_STR_TARGET_HEAD) > PF_LONG_NAME_LEN_MAX) {
            return PF_IERR_INVALID_NAME;
        }
        PFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
    }
    PFSTR_SetLocalStr(&new_name, tmp_local);

    err = PFENT_ITER_GetEntryOfPath(&dst_iter, &dst_dir, p_vol, p_dst_path, TRUE);
    if (err != 0) {
        return err;
    }
    new_ent.start_cluster = dst_dir.start_cluster;
    PFFAT_InitFFD(&dst_iter.ffd, &dst_hint, p_vol, &new_ent.start_cluster);
    err = PFENT_findEntry(&dst_iter.ffd, &new_ent, 0, &new_name, PF_ATTR_ALL, PF_ATTR_NONE);
    if (err == 0) {
        if (new_ent.entry_sector == ent.entry_sector && new_ent.entry_offset == ent.entry_offset) {
            return 0;
        }
        return PF_IERR_EXISTS;
    } else if (err != 0 && err != PF_IERR_NOT_FOUND) {
        return err;
    }

    if ((ent.attr & PF_ATTR_DIR) == 0) {
        if (PFFILE_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
    } else {
        if (PFDIR_IsOpened(&ent)) {
            return PF_IERR_IN_USE;
        }
        if (PFDIR_IsCurrentDir(&ent)) {
            return PF_IERR_INVALID_OPERATION;
        }
        err = PFPATH_SplitPath(p_src_path, &dir_str, &old_name);
        if (err != 0) {
            return err;
        }
        if (PFSTR_StrNCmp(&old_name, (const s8*)"..", PF_STR_TARGET_HEAD, 0, 2) == 0) {
            return PF_IERR_INVALID_PARAM;
        }
        if (PFSTR_StrNCmp(&old_name, (const s8*)".", PF_STR_TARGET_HEAD, 0, 1) == 0) {
            return PF_IERR_INVALID_PARAM;
        }

        p_sfd = p_vol->sfds;
        sector = p_vol->bpb.first_data_sector + ((ent.start_cluster - PF_CLUSTER_FIRST) << p_vol->bpb.log2_sectors_per_cluster);
        for (i = 0; i < PF_SFD_COUNT; i++, p_sfd++) {
            if ((p_sfd->stat & PF_STAT_USED) && (p_sfd->stat & PF_STAT_VALID)) {
                if (p_vol == p_sfd->dir_entry.p_vol && sector == p_sfd->dir_entry.entry_sector) {
                    return PF_IERR_IN_USE;
                }
                is_found = 0;
                err = PFENT_ITER_FindCluster(&p_sfd->dir_entry, ent.start_cluster, &is_found);
                if (err != 0) {
                    return err;
                }
                if (is_found) {
                    return PF_IERR_IN_USE;
                }
            }
        }

        new_ent.start_cluster = dst_dir.start_cluster;
        while (1) {
            if (new_ent.start_cluster == PF_CLUSTER_ROOT_DIR || new_ent.start_cluster == PF_CLUSTER_DOTDOT_ROOT) {
                break;
            }
            PFFAT_InitFFD(&dst_iter.ffd, &dst_hint, p_vol, &new_ent.start_cluster);
            PFSTR_InitStr(&dotdot, (const s8*)"..", PF_STR_CODEMODE_OEM);
            PFSTR_SetLocalStr(&dotdot, NULL);
            err = PFENT_findEntry(&dst_iter.ffd, &new_ent, 0, &dotdot, PF_ATTR_ALL, PF_ATTR_NONE);
            if (err != 0) {
                return err;
            }
            cluster = new_ent.start_cluster;
            if (cluster == PF_CLUSTER_ROOT_DIR || cluster == PF_CLUSTER_DOTDOT_ROOT) {
                break;
            }
            PFFAT_ResetFFD(&dst_iter.ffd, &new_ent.start_cluster);
            err = PFENT_ITER_IteratorInitialize(&dst_iter, 0);
            if (err != 0) {
                return err;
            }
            err = PFENT_ITER_FindDirEntryFromCluster(&dst_iter, &new_ent, cluster, &is_found);
            if (err == 0) {
                if (new_ent.start_cluster == ent.start_cluster) {
                    return PF_IERR_INVALID_PARAM;
                }
            } else if (err != 0) {
                return err;
            }
        }
    }

    name_len = PFSTR_StrNumChar(&new_name, PF_STR_TARGET_HEAD);
    if (name_len + dst_dir.path_len > PF_PATH_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }
    if ((ent.attr & PF_ATTR_DIR) == 0 && name_len > PF_LONG_NAME_LEN_MAX) {
        return PF_IERR_INVALID_NAME;
    }

    pf_memcpy(&new_ent, &ent, sizeof(struct PF_DIR_ENT));
    pozIdx = PFPATH_parseShortName(new_ent.short_name, &new_name);
    if (new_ent.short_name[0] == 0) {
        return PF_IERR_INVALID_PATH;
    }

    if (ent.num_entry_LFNs != 0 && (ent.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        saved_first_char = ent.long_name[0];
        ent.long_name[0] = 0;
        for (ord = 1; ord <= ent.num_entry_LFNs; ord++) {
            err = PFENT_ITER_Retreat(&iter, FALSE);
            if (err != 0) {
                return err;
            }
            err = PFSEC_WriteData(p_vol, del_mark, iter.sector, iter.offset, 1, &success_size, FALSE);
            if (err != 0) {
                return err;
            }
            if (success_size != 1) {
                return PF_IERR_IO;
            }
        }
    }
    err = PFSEC_WriteData(p_vol, del_mark, ent.entry_sector, ent.entry_offset, 1, &success_size, FALSE);
    if (err != 0) {
        return err;
    }
    if (success_size != 1) {
        return PF_IERR_IO;
    }
    if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
        err = PFCACHE_FlushDataCache(p_vol);
        if (err != 0) {
            return err;
        }
    }

    if (pozIdx != 0) {
        err = PFENT_AdjustSFN(&dst_dir, new_ent.short_name);
        if (err != 0) {
            return err;
        }
        if (PFSTR_GetCodeMode(&new_name) == PF_STR_CODEMODE_OEM) {
            name_len = PFPATH_transformInUnicode(new_ent.long_name, PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
        } else {
            pf_w_strcpy(new_ent.long_name, (const u16*)PFSTR_GetStrPos(&new_name, PF_STR_TARGET_HEAD));
        }
        new_ent.num_entry_LFNs = name_len / PF_LFN_CHARS_PER_ENTRY + (name_len % PF_LFN_CHARS_PER_ENTRY != 0);
    } else {
        new_ent.num_entry_LFNs = 0;
    }
    new_ent.small_letter_flag = 0;

    PFFAT_InitFFD(&dst_iter.ffd, &new_hint, p_vol, &dst_dir.start_cluster);
    result = PFENT_allocateEntryPos(&new_ent, new_ent.num_entry_LFNs + 1, &dst_iter.ffd, next_chain, &new_name, &pos);
    if (result != 0) {
        num_lfn = ent.num_entry_LFNs;
        if (num_lfn != 0 && (ent.small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
            ent.long_name[0] = saved_first_char;
            ent.entry_offset -= num_lfn * PF_DIR_ENTRY_SIZE;
        }
        p_ent = &ent;
    } else {
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
            PFPATH_AdjustExtShortName(new_ent.short_name, pos);
        }
        new_ent.check_sum = PFENT_CalcCheckSum(&new_ent);
        PFENT_getcurrentDateTimeForEnt(&new_ent.access_date, &access_time);
        p_ent = &new_ent;
        if (ent.attr & PF_ATTR_DIR) {
            PFFAT_InitFFD(&iter.ffd, &new_hint, p_vol, &ent.start_cluster);
            PFSTR_InitStr(&dotdot, (const s8*)"..", PF_STR_CODEMODE_OEM);
            PFSTR_SetLocalStr(&dotdot, NULL);
            err = PFENT_findEntry(&iter.ffd, &ent, 0, &dotdot, PF_ATTR_ALL, PF_ATTR_NONE);
            if (err != 0) {
                return err;
            }
            cluster = dst_dir.start_cluster;
            if (cluster == PF_CLUSTER_ROOT_DIR || cluster == PF_CLUSTER_DOTDOT_ROOT || (cluster == PF_CLUSTER_FIRST && p_vol->bpb.fat_type == FAT_32)) {
                ent.start_cluster = PF_CLUSTER_DOTDOT_ROOT;
            } else {
                ent.start_cluster = cluster;
            }
            err = PFENT_updateEntry(&ent, PFENT_UPDATE_SET_ARCHIVE);
            if (err != 0) {
                return err;
            }
            if (p_vol->cache.mode & PF_CACHE_MODE_SYNC_UPDATES) {
                err = PFCACHE_FlushDataCache(p_vol);
                if (err != 0) {
                    return err;
                }
            }
        }
    }

    if (p_ent->num_entry_LFNs != 0 && (p_ent->small_letter_flag & PF_SMALL_LETTER_MASK) == 0) {
        lfn_sector = p_ent->entry_sector;
        p_chain = next_chain;
        for (lfn_ord = p_ent->num_entry_LFNs; lfn_ord >= 1; lfn_ord--) {
            PFENT_storeLFNEntryFieldsToBuf(buf, p_ent, lfn_ord, p_ent->check_sum, lfn_ord == p_ent->num_entry_LFNs);
            err = PFSEC_WriteData(p_vol, buf, lfn_sector, p_ent->entry_offset, PF_DIR_ENTRY_SIZE, &success_size, FALSE);
            if (err != 0) {
                return err;
            }
            if (success_size != PF_DIR_ENTRY_SIZE) {
                return PF_IERR_IO;
            }
            p_ent->entry_offset += PF_DIR_ENTRY_SIZE;
            if (p_ent->entry_offset >= p_vol->bpb.bytes_per_sector) {
                p_ent->entry_offset = 0;
                lfn_sector = *p_chain++;
            }
        }
        p_ent->entry_sector = lfn_sector;
    }
    err = PFENT_updateEntry(p_ent, PFENT_UPDATE_SET_ARCHIVE);
    if (err != 0) {
        result = err;
    }
    return result;
}

s32 PFDIR_p_fsnext(struct PF_DTA* p_dta) {
    u32 lpos;
    u32 ppos;
    struct PF_FAT_HINT hint;
    struct PF_STR str;
    struct PF_FFD ffd;
    struct PF_DIR_ENT ent;
    s32 i;
    s32 err;

    lpos = 0;
    ppos = 0;
    i = 0;
    if ((p_dta->status & PF_DTA_STAT_OEM) == PF_DTA_STAT_OEM) {
        err = PFSTR_InitStr(&str, p_dta->reg_exp, PF_STR_CODEMODE_OEM);
    } else {
        err = PFSTR_InitStr(&str, (const s8*)p_dta->reg_exp_w, PF_STR_CODEMODE_UNICODE);
    }
    if (err != 0) {
        return err;
    }
    PFSTR_SetLocalStr(&str, p_dta->reg_exp);

    while (ppos < PF_ENTRY_INDEX_END) {
        if (p_dta->parent_pos >= PF_ENTRY_INDEX_END) {
            return PF_IERR_INVALID_PARAM;
        }
        ent.start_cluster = p_dta->parent_start_cluster;
        PFFAT_InitFFD(&ffd, &hint, p_dta->p_vol, &ent.start_cluster);
        err = PFENT_findEntryPos(&ffd, &ent, p_dta->parent_pos, &str, p_dta->attr, PF_ATTR_NONE, &lpos, &ppos);
        if (err != 0) {
            return err;
        }
        p_dta->parent_pos = ppos + 1;
        if (ppos == PF_ENTRY_INDEX_END) {
            return PF_IERR_NOT_FOUND;
        }
        break;
    }

    pf_strcpy(p_dta->FileName, ent.short_name);
    if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
        PFPATH_transformInUnicode(p_dta->FileName_w, p_dta->FileName);
    }
    if (ent.long_name[0] != 0) {
        if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
            pf_vol_set.setting = (pf_vol_set.setting & ~PF_ENCODE_MODE_MASK) | PF_ENCODE_MODE_OEM;
            PFPATH_transformFromUnicodeToNormal(p_dta->LongName, ent.long_name);
            pf_vol_set.setting = (pf_vol_set.setting & ~PF_ENCODE_MODE_MASK) | PF_ENCODE_MODE_UNICODE;
        } else {
            PFPATH_transformFromUnicodeToNormal(p_dta->LongName, ent.long_name);
        }
        if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
            pf_w_strcpy(p_dta->LongName_w, ent.long_name);
        }
    } else {
        p_dta->LongName[0] = 0;
        if ((p_dta->status & PF_DTA_STAT_UNICODE) == PF_DTA_STAT_UNICODE) {
            p_dta->LongName_w[0] = 0;
        }
    }

    if (p_dta->p_vol->num_opened_files != 0) {
        for (i = 0; i < PF_SFD_COUNT; i++) {
            if ((p_dta->p_vol->sfds[i].stat & PF_STAT_USED) && (p_dta->p_vol->sfds[i].stat & PF_STAT_VALID) && ent.p_vol == p_dta->p_vol->sfds[i].dir_entry.p_vol &&
                ent.entry_sector == p_dta->p_vol->sfds[i].dir_entry.entry_sector && ent.entry_offset == p_dta->p_vol->sfds[i].dir_entry.entry_offset) {
                p_dta->FileSize = p_dta->p_vol->sfds[i].dir_entry.file_size;
                p_dta->Time = p_dta->p_vol->sfds[i].dir_entry.modify_time;
                p_dta->Date = p_dta->p_vol->sfds[i].dir_entry.modify_date;
                p_dta->Attribute = p_dta->p_vol->sfds[i].dir_entry.attr;
                p_dta->num_entry_LFNs = p_dta->p_vol->sfds[i].dir_entry.num_entry_LFNs;
                p_dta->ordinal = p_dta->p_vol->sfds[i].dir_entry.ordinal;
                p_dta->check_sum = p_dta->p_vol->sfds[i].dir_entry.check_sum;
                break;
            }
        }
    }

    if (p_dta->p_vol->num_opened_files == 0 || i == PF_SFD_COUNT) {
        p_dta->FileSize = ent.file_size;
        p_dta->Time = ent.modify_time;
        p_dta->Date = ent.modify_date;
        p_dta->Attribute = ent.attr;
        p_dta->num_entry_LFNs = ent.num_entry_LFNs;
        p_dta->ordinal = ent.ordinal;
        p_dta->check_sum = ent.check_sum;
    }
    return 0;
}

s32 PFDIR_p_fsfirst(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta) {
    struct PF_STR dir_str;
    struct PF_STR pattern;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    s32 err;

    err = PFPATH_SplitPathPattern(p_path, &dir_str, &pattern);
    if (err != 0) {
        return err;
    }
    err = PFENT_ITER_GetEntryOfPattern(&iter, &ent, p_vol, &dir_str);
    if (err != 0) {
        return err;
    }
    if ((ent.attr & PF_ATTR_DIR) == 0) {
        return PF_IERR_NOT_DIR;
    }

    p_dta->parent_start_cluster = ent.start_cluster;
    p_dta->parent_pos = 0;
    p_dta->p_vol = ent.p_vol;
    p_dta->attr = attr;
    p_dta->status = 0;
    if (PFSTR_GetCodeMode(&pattern) == PF_STR_CODEMODE_OEM) {
        PFPATH_SetSearchPattern(p_dta->reg_exp, NULL, &pattern);
        p_dta->status |= PF_DTA_STAT_OEM;
    } else {
        PFPATH_SetSearchPattern(p_dta->reg_exp, p_dta->reg_exp_w, &pattern);
        p_dta->status |= PF_DTA_STAT_UNICODE;
    }
    return PFDIR_p_fsnext(p_dta);
}

s32 PFDIR_p_fsexec_chmod(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, u32 type, u8 attr) {
    u16 access_time;
    s32 err;

    if (attr & PF_ATTR_NORMAL) {
        attr &= ~PF_ATTR_NORMAL;
    }
    if (p_ent->attr & PF_ATTR_DIR) {
        if (attr & PF_ATTR_SYSTEM) {
            return PF_IERR_INVALID_PARAM;
        }
        if (PFDIR_IsOpened(p_ent)) {
            return PF_IERR_IN_USE;
        }
        p_ent->attr = attr | PF_ATTR_DIR;
    } else {
        if (attr & PF_ATTR_DIR) {
            return PF_IERR_INVALID_PARAM;
        }
        if (PFFILE_IsOpened(p_ent)) {
            return PF_IERR_IN_USE;
        }
        p_ent->attr = attr;
    }

    PFENT_getcurrentDateTimeForEnt(&p_ent->access_date, &access_time);
    err = PFENT_updateEntry(p_ent, PFENT_UPDATE_KEEP_ATTR);
    if (err != 0) {
        return err;
    }
    if ((type & PF_FSEXEC_DIRECT) == 0) {
        p_dta->Attribute = p_ent->attr;
    }
    err = PFCACHE_FlushDataCacheSpecific(p_ent->p_vol, NULL);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_p_fsexec_remove(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter) {
    BOOL is_empty;
    struct PF_FAT_HINT save_hint;
    struct PF_ENT_ITER save_iter;
    u32 start_cluster;
    u32 size;
    s32 err;

    if (p_ent->attr & PF_ATTR_READ_ONLY) {
        return PF_IERR_READ_ONLY;
    }
    if (p_ent->attr & PF_ATTR_DIR) {
        if (PFVOL_CheckCurrentDir(p_ent->p_vol, p_ent->start_cluster) != 0 || p_ent->start_cluster == PF_CLUSTER_ROOT_DIR ||
            (p_ent->p_vol->bpb.fat_type == FAT_32 && p_ent->start_cluster == p_ent->p_vol->bpb.root_dir_cluster)) {
            return PF_IERR_INVALID_OPERATION;
        }
        if (PFDIR_IsOpened(p_ent)) {
            return PF_IERR_IN_USE;
        }
        save_iter = *p_iter;
        save_hint = *p_iter->ffd.p_hint;
        PFFAT_ResetFFD(&p_iter->ffd, &p_ent->start_cluster);
        err = PFENT_ITER_IteratorInitialize(p_iter, 0);
        if (err != 0) {
            return err;
        }
        err = PFDIR_CheckDirIsEmpty(p_iter, &is_empty);
        if (err != 0) {
            return err;
        }
        if (!is_empty) {
            return PF_IERR_NOT_EMPTY;
        }
        *p_iter = save_iter;
        p_iter->ffd.p_hint = &save_hint;
        size = PF_FAT_WHOLE_CHAIN;
    } else {
        if (PFFILE_IsOpened(p_ent)) {
            return PF_IERR_IN_USE;
        }
        size = p_ent->file_size;
    }

    start_cluster = p_ent->start_cluster;
    p_ent->num_entry_LFNs = p_dta->num_entry_LFNs;
    p_ent->ordinal = p_dta->ordinal;
    p_ent->check_sum = p_dta->check_sum;
    err = PFENT_RemoveEntry(p_ent, p_iter);
    if (err != 0) {
        return err;
    }
    err = PFFAT_FreeChain(&p_iter->ffd, start_cluster, PF_FAT_NO_CHAIN_INDEX, size);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_p_fsexec_fopen(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 type, u32 mode) {
    s8 long_name[PF_SFN_BUF_LEN];
    s32 err;

    if (type & PF_FSEXEC_DIRECT) {
        p_ent->num_entry_LFNs = p_dta->num_entry_LFNs;
        p_ent->ordinal = p_dta->ordinal;
        p_ent->check_sum = p_dta->check_sum;
        if (p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG] & PF_SMALL_LETTER_MASK) {
            PFPATH_getLongNameformShortName(p_ent->short_name, long_name, p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG]);
            PFPATH_transformInUnicode(p_ent->long_name, long_name);
        } else {
            err = PFENT_ITER_GetLFNEntryName(p_iter, p_ent);
            if (err != 0) {
                return err;
            }
        }
    }

    err = PFFILE_FsexecOpenFile(p_ent, p_iter, mode, &p_dta->p_file);
    if (err != 0) {
        return err;
    }
    p_dta->p_vol->num_opened_files++;
    return 0;
}

s32 PFDIR_p_fsexec_opendir(struct PF_DTA* p_dta, struct PF_DIR_ENT* p_ent, struct PF_ENT_ITER* p_iter, u32 type) {
    s8 long_name[PF_SFN_BUF_LEN];
    s32 err;

    if (type & PF_FSEXEC_DIRECT) {
        p_ent->num_entry_LFNs = p_dta->num_entry_LFNs;
        p_ent->ordinal = p_dta->ordinal;
        p_ent->check_sum = p_dta->check_sum;
        if (p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG] & PF_SMALL_LETTER_MASK) {
            PFPATH_getLongNameformShortName(p_ent->short_name, long_name, p_iter->buf[PF_DIRENT_SMALL_LETTER_FLAG]);
            PFPATH_transformInUnicode(p_ent->long_name, long_name);
        } else {
            err = PFENT_ITER_GetLFNEntryName(p_iter, p_ent);
            if (err != 0) {
                return err;
            }
        }
    }

    err = PFDIR_DoFsexecOpenDir(p_ent, p_iter, &p_dta->p_dir);
    if (err != 0) {
        return err;
    }
    p_dta->p_vol->num_opened_directories++;
    return 0;
}

s32 PFDIR_p_fsexec(struct PF_DTA* p_dta, u32 type, u8 param) {
    u32 is_found;
    struct PF_FAT_HINT hint;
    u8 dot_name[PF_SFN_LEN];
    struct PF_STR str;
    struct PF_STR long_str;
    struct PF_FFD ffd;
    struct PF_ENT_ITER iter;
    struct PF_DIR_ENT ent;
    u32 is_chmod;
    u32 is_direct;
    u32 index;
    s32 result;
    s32 err;

    is_chmod = type & (PF_FSEXEC_CHMOD | PF_FSEXEC_CHDMOD);
    if (is_chmod || (type & PF_FSEXEC_REMOVE) || (type & PF_FSEXEC_FOPEN) || (type & PF_FSEXEC_OPENDIR)) {
        if (is_chmod || (type & PF_FSEXEC_REMOVE) || ((type & PF_FSEXEC_FOPEN) && param != PF_OPEN_MODE_READ)) {
            err = PFVOL_CheckForWrite(p_dta->p_vol);
            if (err != 0) {
                return err;
            }
        }

        ent.short_name[0] = 0;
        ent.start_cluster = p_dta->parent_start_cluster;
        PFFAT_InitFFD(&ffd, &hint, p_dta->p_vol, &ent.start_cluster);
        iter.ffd = ffd;
        is_direct = type & PF_FSEXEC_DIRECT;
        index = p_dta->parent_pos - 1;
        if (!is_direct) {
            index -= p_dta->num_entry_LFNs;
        }
        err = PFENT_ITER_IteratorInitialize(&iter, index);
        if (err != 0) {
            return err;
        }

        if (!is_direct) {
            if ((p_dta->status & PF_DTA_STAT_OEM) == PF_DTA_STAT_OEM) {
                err = PFSTR_InitStr(&str, p_dta->FileName, PF_STR_CODEMODE_OEM);
            } else if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
                err = PFSTR_InitStr(&str, (const s8*)p_dta->LongName_w, PF_STR_CODEMODE_UNICODE);
            } else {
                err = PFSTR_InitStr(&str, (const s8*)p_dta->FileName_w, PF_STR_CODEMODE_UNICODE);
            }
            if (err != 0) {
                return err;
            }
            if ((pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) == PF_ENCODE_MODE_UNICODE) {
                PFSTR_SetLocalStr(&str, p_dta->LongName);
            } else {
                PFSTR_SetLocalStr(&str, p_dta->FileName);
            }
            err = PFENT_ITER_FindEntryNoPathCheck(&iter, &ent, &str, PF_ATTR_ALL, PF_ATTR_NONE, &is_found);
            if (err != 0) {
                return err;
            }
            if ((p_dta->status & PF_DTA_STAT_UNICODE) != PF_DTA_STAT_UNICODE || (pf_vol_set.setting & PF_ENCODE_MODE_UNICODE) != PF_ENCODE_MODE_UNICODE) {
                if (PFPATH_cmpName(ent.short_name, &str, TRUE) != 0) {
                    return PF_IERR_NOT_FOUND;
                }
            }
            result = PFSTR_InitStr(&long_str, p_dta->LongName, PF_STR_CODEMODE_OEM);
            if (result != 0) {
                return result;
            }
            PFSTR_SetLocalStr(&long_str, p_dta->LongName);
            if (PFPATH_cmpNameUni(ent.long_name, &long_str) != 0) {
                return PF_IERR_NOT_FOUND;
            }
        } else {
            if (iter.buf[0] == PF_ENTRY_END || iter.buf[0] == PF_ENTRY_DELETED) {
                return PF_IERR_NOT_FOUND;
            }
            if ((type & PF_FSEXEC_OPENDIR) == 0 && (iter.buf[PF_DIRENT_ATTR] & (PF_ATTR_VOLUME | PF_ATTR_DIR))) {
                return PF_IERR_NOT_FOUND;
            }
            if ((type & PF_FSEXEC_OPENDIR) && ((iter.buf[PF_DIRENT_ATTR] & PF_ATTR_DIR) == 0 || (iter.buf[PF_DIRENT_ATTR] & PF_ATTR_VOLUME))) {
                return PF_IERR_NOT_DIR;
            }
            result = PFENT_ITER_GetEntryOfIter(&iter, &ent);
            if (result != 0) {
                return result;
            }
        }

        PFDIR_SetDotEntry(dot_name);
        if (pf_strncmp(ent.short_name, (const s8*)dot_name, 1) == 0) {
            return PF_IERR_INVALID_OPERATION;
        }
        PFDIR_SetDotDotEntry(dot_name);
        if (pf_strncmp(ent.short_name, (const s8*)dot_name, 2) == 0) {
            return PF_IERR_INVALID_OPERATION;
        }

        if (is_chmod) {
            result = PFDIR_p_fsexec_chmod(p_dta, &ent, type, param);
        } else if (type & PF_FSEXEC_REMOVE) {
            result = PFDIR_p_fsexec_remove(p_dta, &ent, &iter);
        } else if (type & PF_FSEXEC_FOPEN) {
            result = PFDIR_p_fsexec_fopen(p_dta, &ent, &iter, type, param);
        } else if (type & PF_FSEXEC_OPENDIR) {
            result = PFDIR_p_fsexec_opendir(p_dta, &ent, &iter, type);
        }
    } else {
        return PF_IERR_INVALID_PARAM;
    }
    return result;
}

void PFDIR_FinalizeAllDirs(struct PF_VOLUME* p_vol) {
    u16 i;

    for (i = 0; i < PF_SDD_COUNT; i++) {
        PFDIR_FinalizeSDD(&p_vol->sdds[i]);
    }
    for (i = 0; i < PF_UDD_COUNT; i++) {
        PFDIR_FinalizeUDD(&p_vol->udds[i]);
    }
    p_vol->num_opened_directories = 0;
}

s32 PFDIR_fsfirst(struct PF_STR* p_path, u8 attr, struct PF_DTA* p_dta) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDIR_p_fsfirst(p_vol, p_path, attr, p_dta);
    if (err != 0) {
        p_dta->p_vol = NULL;
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFDIR_fsnext(struct PF_DTA* p_dta) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = p_dta->p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDIR_p_fsnext(p_dta);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFDIR_fsexec(struct PF_DTA* p_dta, u32 type, u8 param) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = p_dta->p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_fsexec(p_dta, type, param);
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
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFDIR_fstat(struct PF_STR* p_path, struct PF_STAT* p_stat) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDIR_p_fstat(p_vol, p_path, p_stat);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFDIR_rename(struct PF_STR* p_old_path, struct PF_STR* p_new_path) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_old_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_rename(p_vol, p_old_path, p_new_path);
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

s32 PFDIR_move(struct PF_STR* p_src_path, struct PF_STR* p_dst_path) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_src_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_move(p_vol, p_src_path, p_dst_path);
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

s32 PFDIR_opendir(struct PF_STR* p_path, struct PF_DIR** pp_dir) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDIR_p_opendir(p_vol, p_path, pp_dir);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->num_opened_directories++;
    return err;
}

s32 PFDIR_closedir(struct PF_DIR* p_dir) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_dir == NULL) ? NULL : p_dir->p_sdd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_dir->p_sdd->stat & PF_STAT_USED) == 0 || (p_dir->p_sdd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }

    if (--p_dir->p_sdd->num_handlers == 0) {
        p_dir->p_sdd->stat &= ~PF_STAT_USED;
    }
    p_dir->stat &= ~PF_STAT_USED;
    p_vol->num_opened_directories--;
    return 0;
}

s32 PFDIR_readdir(struct PF_DIR* p_dir, struct PF_DIRENT* p_dirent) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_dir == NULL) ? NULL : p_dir->p_sdd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_dir->p_sdd->stat & PF_STAT_USED) == 0 || (p_dir->p_sdd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    err = PFDIR_p_readdir(p_dir, p_dirent);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFDIR_telldir(struct PF_DIR* p_dir, u32* p_offset) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_dir == NULL) ? NULL : p_dir->p_sdd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_dir->p_sdd->stat & PF_STAT_USED) == 0 || (p_dir->p_sdd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    if (p_dir->cursor.logical_seek_index != 0) {
        *p_offset = p_dir->cursor.logical_seek_index;
    } else {
        *p_offset = p_dir->cursor.logical_entry_index;
    }
    return err;
}

s32 PFDIR_seekdir(struct PF_DIR* p_dir, u32 offset) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_dir == NULL) ? NULL : p_dir->p_sdd->dir_entry.p_vol;
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_dir->p_sdd->stat & PF_STAT_USED) == 0 || (p_dir->p_sdd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    p_dir->cursor.logical_seek_index = offset;
    return err;
}

s32 PFDIR_rmdir(struct PF_STR* p_path) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_rmdir(p_vol, p_path);
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
    p_vol->cache.signature = NULL;
    return err;
}

s32 PFDIR_mkdir(struct PF_STR* p_path_str) {
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
    err = PFDIR_p_mkdir(p_vol, p_path_str, 0, NULL);
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

s32 PFDIR_createdir(struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_mkdir(p_vol, p_path, option, p_dta);
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

s32 PFDIR_chdir(struct PF_STR* p_path) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDIR_p_chdir(p_vol, p_path);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        PFVOL_SetCurrentVolume(p_vol);
    }
    return err;
}

static s32 PFDIR_p_fchdir(struct PF_VOLUME* p_vol, struct PF_DIR* p_dir) {
    s32 err;

    err = PFVOL_SetCurrentDir(p_vol, &p_dir->p_sdd->dir_entry);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 PFDIR_fchdir(struct PF_DIR* p_dir) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = (p_dir == NULL) ? NULL : p_dir->p_sdd->dir_entry.p_vol;
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_dir->p_sdd->stat & PF_STAT_USED) == 0 || (p_dir->p_sdd->stat & PF_STAT_VALID) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    err = PFDIR_p_fchdir(p_vol, p_dir);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        PFVOL_SetCurrentVolume(p_vol);
    }
    return err;
}

s32 PFDIR_chmod(struct PF_STR* p_path, u8 attr) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_chmod(p_vol, p_path, attr);
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

s32 PFDIR_chdmod(struct PF_STR* p_path, u8 attr) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFPATH_GetVolumeFromPath(p_path);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = PFDIR_p_chmod(p_vol, p_path, attr | PF_ATTR_DIR);
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
