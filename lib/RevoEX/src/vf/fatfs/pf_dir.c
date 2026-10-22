#include <revolution/vf/vf_all.h>

static void VFiPFDIR_FinalizeSDD(struct PF_SDD* p_sdd) {
    p_sdd->stat = 0;
    VFiPFFAT_FinalizeFFD(&p_sdd->ffd);
}

static void VFiPFDIR_FinalizeUDD(struct PF_DIR* p_dir) {
    p_dir->stat &= ~1u;
}

static s32 VFiPFDIR_p_mkdir(struct PF_VOLUME* p_vol, struct PF_STR* p_path, u32 option, struct PF_DTA* p_dta) {
    u16 filename_size;
    struct PF_STR dir_path;
    struct PF_STR filename;
    u32 is_found;
    struct PF_DIR_ENT entry_dir;
    struct PF_ENT_ITER iter;
    u32 i;
    u32 spc_cnt;
    struct PF_DIR_ENT entry;
    u8 buf[32];
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
    s8 tmp_local[512];
    u32 sector;

    num_entry_LFNs = 0;
    SVSect = 0;

    err = VFiPFPATH_SplitPath(p_path, &dir_path, &filename);
    if (err != 0) {
        return err;
    }
    err = VFiPFENT_ITER_GetEntryOfPath(&iter, &entry_dir, p_vol, &dir_path, 1);
    if (err != 0) {
        return err;
    }
    if ((entry_dir.attr & 0x10) == 0) {
        return 20;
    }
    filename_size = VFiPFSTR_StrNumChar(&filename, 1);
    if (filename_size > 255) {
        return 1;
    }
    if (filename_size + entry_dir.path_len > 259) {
        return 2;
    }
    if (VFiPFSTR_GetCodeMode(&filename) == 2) {
        VFiPFPATH_transformFromUnicodeToNormal(tmp_local, (const u16*)VFiPFSTR_GetStrPos(&filename, 1));
    }
    VFiPFSTR_SetLocalStr(&filename, tmp_local);
    if (p_dta != NULL) {
        p_dta->p_file = NULL;
        p_dta->p_dir = NULL;
        p_dta->parent_start_cluster = entry_dir.start_cluster;
        p_dta->p_vol = entry_dir.p_vol;
        p_dta->attr = 0x10;
        if (VFiPFSTR_GetCodeMode(&filename) == 1) {
            VFiPFPATH_SetSearchPattern(p_dta->reg_exp, NULL, &filename);
            p_dta->status |= 1;
        } else {
            VFiPFPATH_SetSearchPattern(p_dta->reg_exp, (u16*)((u8*)p_dta + 0x444), &filename);
            p_dta->status |= 2;
        }
    }
    VFiPFFAT_InitFFD(&ffd, &fat_hint, p_vol, &entry_dir.start_cluster);
    iter.ffd = ffd;
    VFiPFFAT_ResetFFD(&iter.ffd, &entry_dir.start_cluster);
    err = VFiPFENT_ITER_IteratorInitialize(&iter, 0);
    if (err != 0) {
        return err;
    }
    VFipf_memcpy(&entry, &entry_dir, sizeof(struct PF_DIR_ENT));
    err = VFiPFENT_ITER_FindEntry(&iter, &entry, &filename, 0x10, 0, &is_found, 1);
    if (err != 0) {
        return err;
    }
    if (is_found != 0) {
        return 8;
    }
    pozIdx = VFiPFPATH_parseShortName(entry.short_name, &filename);
    if (pozIdx != 0 && entry.short_name[0] == 0) {
        return 1;
    }
    if (pozIdx != 0) {
        err = VFiPFENT_AdjustSFN(&entry_dir, entry.short_name);
        if (err != 0) {
            return err;
        }
        if (VFiPFSTR_GetCodeMode(&filename) == 1) {
            filename_size = VFiPFPATH_transformInUnicode(entry.long_name, VFiPFSTR_GetStrPos(&filename, 1));
        } else {
            VFipf_w_strcpy(entry.long_name, (u16*)VFiPFSTR_GetStrPos(&filename, 1));
        }
    } else {
        entry.long_name[0] = 0;
    }
    entry.start_cluster = 0;
    entry.file_size = 0;
    entry.p_vol = p_vol;
    entry.small_letter_flag = 0;
    entry.attr = 0x10;
    entry.create_time_ms = VFiPFENT_getcurrentDateTimeForEnt(&entry.create_date, &entry.create_time);
    entry.access_date = entry.create_date;
    entry.modify_time = entry.create_time;
    entry.modify_date = entry.create_date;
    save_offset = entry.entry_offset;
    if (entry.long_name[0] != 0 && (entry.small_letter_flag & 0x18) == 0) {
        num_entry_LFNs = (u32)filename_size / 13 + ((u32)filename_size % 13 != 0);
        err = VFiPFENT_allocateEntryPos(&entry, num_entry_LFNs + 1, &ffd, NextChain, &filename, &OldSector);
        if (err != 0) {
            return err;
        }
        if ((VFipf_vol_set.setting & 2) == 2) {
            VFiPFPATH_AdjustExtShortName(entry.short_name, OldSector);
        }
    } else {
        err = VFiPFENT_allocateEntryPos(&entry, 1, &ffd, NextChain, &filename, &OldSector);
        if (err != 0) {
            return err;
        }
    }
    VFiPFFAT_ResetFFD(&ffd, &entry.start_cluster);
    err = VFiPFFAT_GetSectorSpecified(&ffd, 0, 1, &SVSect);
    if (err != 0) {
        return err;
    }
    if (SVSect == 0xFFFFFFFF) {
        return 6;
    }
    position = ffd.p_hint->cluster;
    if (p_vol->bpb.fat_type == 2 && *iter.ffd.p_start_cluster == p_vol->bpb.root_dir_cluster && save_offset == 0 && ffd.p_hint->cluster == p_vol->bpb.root_dir_cluster) {
        iter.ffd = ffd;
        err = VFiPFENT_ITER_IteratorInitialize(&iter, num_entry_LFNs);
        if (err != 0) {
            return err;
        }
        if (iter.buf[0] == 0) {
            VFiPFFAT_ResetFFD(&ffd, &entry.start_cluster);
            *ffd.p_start_cluster = 0;
            ffd.p_hint->cluster = 0;
            position = p_vol->bpb.root_dir_cluster + 1;
            err = VFiPFFAT_GetSectorSpecified(&ffd, 0, 1, &SVSect);
            if (err != 0) {
                return err;
            }
        }
    }
    err = VFiPFCACHE_AllocateDataPage(p_vol, 0xFFFFFFFF, &p_page);
    if (err != 0) {
        return err;
    }
    VFipf_memset(p_page->p_buf, 0, p_vol->bpb.bytes_per_sector);
    for (i = p_vol->bpb.sectors_per_cluster; i != 0; i--) {
        if (i == 1) {
            VFiPFENT_SetDotEntry(p_page->p_buf);
            VFiPFENT_StoreEntryNumericFieldsToBuf(p_page->p_buf, &entry);
            VFiPFENT_SetDotDotEntry(p_page->p_buf + 32);
            if (entry_dir.entry_sector == 0xFFFFFFFF) {
                entry.start_cluster = 0;
            } else {
                entry.start_cluster = entry_dir.start_cluster;
            }
            VFiPFENT_StoreEntryNumericFieldsToBuf(p_page->p_buf + 32, &entry);
        }
        err = VFiPFSEC_WriteData(p_vol, p_page->p_buf, SVSect + i - 1, 0, p_vol->bpb.bytes_per_sector, &success_size, 0);
        if (err == 0) {
            if (success_size != p_vol->bpb.bytes_per_sector) {
                err = 17;
                break;
            }
        } else {
            break;
        }
    }
    VFiPFCACHE_FreeDataPage(p_vol, p_page);
    if (err != 0) {
        return err;
    }
    entry.num_entry_LFNs = num_entry_LFNs;
    sector = entry.entry_sector;
    sum = VFiPFENT_CalcCheckSum(&entry);
    if ((entry.small_letter_flag & 0x18) == 0) {
        u32* p_chain = NextChain;
        for (i = num_entry_LFNs; i >= 1; i--) {
            VFiPFENT_storeLFNEntryFieldsToBuf(buf, &entry, i, sum, i == num_entry_LFNs);
            err = VFiPFSEC_WriteData(p_vol, buf, sector, entry.entry_offset, 32, &success_size, 0);
            if (err != 0) {
                return err;
            }
            if (success_size != 32) {
                return 17;
            }
            entry.entry_offset += 32;
            if (entry.entry_offset >= p_vol->bpb.bytes_per_sector) {
                entry.entry_offset = 0;
                sector = *p_chain;
                p_chain++;
            }
        }
        entry.entry_sector = sector;
    }
    entry.start_cluster = position;
    err = VFiPFENT_updateEntry(&entry, 0);
    if (err != 0) {
        return err;
    }
    if (p_dta != NULL) {
        p_dta->parent_pos = OldSector + 1;
        VFipf_strcpy(p_dta->FileName, entry.short_name);
        if ((p_dta->status & 2) == 2) {
            VFiPFPATH_transformInUnicode((u16*)((u8*)p_dta + 0x64C), p_dta->FileName);
        }
        if (entry.long_name[0] != 0) {
            if ((VFipf_vol_set.setting & 2) == 2) {
                VFipf_vol_set.setting = (VFipf_vol_set.setting & ~3) | 1;
                VFiPFPATH_transformFromUnicodeToNormal(p_dta->LongName, entry.long_name);
                VFipf_vol_set.setting = (VFipf_vol_set.setting & ~3) | 2;
            } else {
                VFiPFPATH_transformFromUnicodeToNormal(p_dta->LongName, entry.long_name);
            }
            if ((p_dta->status & 2) == 2) {
                VFipf_w_strcpy((u16*)((u8*)p_dta + 0x666), entry.long_name);
            }
            p_dta->check_sum = sum;
            p_dta->ordinal = 1;
        } else {
            p_dta->LongName[0] = 0;
            if ((p_dta->status & 2) == 2) {
                *(u16*)((u8*)p_dta + 0x666) = 0;
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

void VFiPFDIR_FinalizeAllDirs(struct PF_VOLUME* p_vol) {
    u16 i;

    for (i = 0; i < 3u; ++i) {
        VFiPFDIR_FinalizeSDD(&p_vol->sdds[i]);
    }
    for (i = 0; i < 3u; ++i) {
        VFiPFDIR_FinalizeUDD(&p_vol->udds[i]);
    }
    p_vol->num_opened_directories = 0;
}

s32 VFiPFDIR_mkdir(struct PF_STR* p_path_str) {
    struct PF_VOLUME* p_vol;
    s32 err;

    if (p_path_str == NULL) {
        return VFipf_vol_set.last_error = 10;
    }
    if (VFiPFSTR_StrNCmp(p_path_str, (const s8*)"\0", 1, 0, 1) == 0) {
        return VFipf_vol_set.last_error = 10;
    }
    p_vol = VFiPFPATH_GetVolumeFromPath(p_path_str);
    if (p_vol == NULL) {
        return VFipf_vol_set.last_error = 9;
    }
    err = VFiPFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        VFipf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->cache.signature = NULL;
    err = VFiPFDIR_p_mkdir(p_vol, p_path_str, 0, NULL);
    if (err != 0) {
        VFipf_vol_set.last_error = err;
        p_vol->last_error = err;
    } else {
        err = VFiPFCACHE_FlushFATCache(p_vol);
        if (err != 0) {
            VFipf_vol_set.last_error = err;
            p_vol->last_error = err;
        } else {
            err = VFiPFCACHE_FlushDataCacheSpecific(p_vol, NULL);
            if (err != 0) {
                VFipf_vol_set.last_error = err;
                p_vol->last_error = err;
            }
        }
    }
    p_vol->cache.signature = NULL;
    return err;
}
