#include <prfile2/pdm_bpb.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_cache.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pf_cp932.h>
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
#include <prfile2/pfd_nanddrv.h>

#define PFVOL_DATA_CACHE_MIN_SECTORS 2

struct PF_VOLUME_SET pf_vol_set;

static BOOL PFVOL_CheckContextRegistered(s32 context_id) {
    u32 i;

    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        pf_vol_set.context[i].stat &= PF_STAT_USED;
        if (pf_vol_set.context[i].stat != 0 && pf_vol_set.context[i].context_id == context_id) {
            return TRUE;
        }
    }
    return FALSE;
}

static s32 PFVOL_InitCurrentDir(struct PF_VOLUME* p_vol) {
    s32 err;
    s32 context_id;
    u32 i;

    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }

    PFSYS_GetCurrentContextID(&context_id);
    p_vol->current_dir[0].stat |= PF_STAT_USED;
    if (PFVOL_CheckContextRegistered(context_id)) {
        p_vol->current_dir[1].stat |= PF_STAT_USED;
        p_vol->current_dir[1].context_id = context_id;
    }

    for (i = 0; i < PF_CONTEXT_COUNT; i++) {
        err = PFENT_GetRootDir(p_vol, &p_vol->current_dir[i].directory);
        if (err != 0) {
            return err;
        }
    }
    return 0;
}

static s32 PFVOL_FinalizeCurrentDir(struct PF_VOLUME* p_vol) {
    u32 i;

    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }
    for (i = 0; i < PF_CONTEXT_COUNT; i++) {
        p_vol->current_dir[i].stat = 0;
    }
    return 0;
}

s32 PFVOL_DoMountVolume(struct PF_VOLUME* p_vol) {
    s32 err;

    p_vol->last_error = 0;
    p_vol->last_driver_error = 0;
    err = PFDRV_mount(p_vol);
    if (err != 0) {
        return err;
    }

    if (p_vol->bpb.bytes_per_sector == 0 || (p_vol->bpb.bytes_per_sector & (PF_CACHE_PAGE_SIZE - 1)) != 0) {
        return PF_IERR_INVALID_BPB;
    }
    if ((p_vol->bpb.bytes_per_sector >> PF_CACHE_PAGE_SIZE_LOG2) > p_vol->cache.num_fat_pages || (p_vol->bpb.bytes_per_sector >> PF_CACHE_PAGE_SIZE_LOG2) * PFVOL_DATA_CACHE_MIN_SECTORS > p_vol->cache.num_data_pages) {
        return PF_IERR_NO_MEMORY;
    }

    err = PFCACHE_InitCaches(p_vol);
    if (err != 0) {
        return err;
    }

    p_vol->flags |= PF_VOL_FLAG_MOUNTED;
    err = PFVOL_InitCurrentDir(p_vol);
    if (err == 0 && (p_vol->flags & PF_VOL_FLAG_FORMAT_AFTER_MOUNT) != 0) {
        err = PFDRV_format(p_vol, p_vol->format_param);
        if (err == 0 && (p_vol->flags & PF_VOL_FLAG_NO_LOGICAL_FORMAT) == 0) {
            err = PFFAT_InitFATRegion(p_vol);
            if (err == 0) {
                err = PFENT_MakeRootDir(p_vol);
            }
        }
    }
    if (err != 0) {
        p_vol->flags &= ~PF_VOL_FLAG_MOUNTED;
    }
    return err;
}

static s32 PFVOL_DoUnmountVolume(struct PF_VOLUME* p_vol, u32 mode) {
    s32 err;

    err = PFDRV_unmount(p_vol, mode);
    if (err != 0) {
        return err;
    }
    PFVOL_FinalizeCurrentDir(p_vol);
    p_vol->flags &= ~PF_VOL_FLAG_MOUNTED;
    return 0;
}

static void PFVOL_UnmountVolumeByEject(struct PF_VOLUME* p_vol) {
    PFFILE_FinalizeAllFiles(p_vol);
    PFDIR_FinalizeAllDirs(p_vol);
    PFVOL_DoUnmountVolume(p_vol, PF_UNMOUNT_FORCE);
    PFDRV_ClearUnmountRequested(p_vol);
    pf_vol_set.num_mounted_volumes--;
}

static s32 PFVOL_p_attach(struct PF_VOLUME* p_vol, struct PF_DRV_TBL* p_drv, s16 vol_idx) {
    s32 err;

    pf_memset(p_vol, 0, sizeof(struct PF_VOLUME));
    p_vol->p_part = p_drv->p_part;
    p_vol->drv_char = vol_idx + 'A';
    p_vol->tail_entry.tracker_size = 1;
    p_vol->tail_entry.tracker_bits = p_vol->tail_entry.tracker_buff;

    err = PFDRV_init(p_vol);
    if (err != 0) {
        return err;
    }

    PFCACHE_SetCache(p_vol, p_drv->cache->pages, p_drv->cache->buffers, p_drv->cache->num_fat_pages, p_drv->cache->num_data_pages);
    PFCACHE_SetFATBufferSize(p_vol, p_drv->cache->num_fat_buf_size);
    PFCACHE_SetDataBufferSize(p_vol, p_drv->cache->num_data_buf_size);
    return 0;
}

static s32 PFVOL_p_detach(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFDRV_finalize(p_vol);
    if (err != 0) {
        return err;
    }
    return 0;
}

static s32 PFVOL_p_mount(struct PF_VOLUME* p_vol) {
    s32 err;

    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        err = PFVOL_DoMountVolume(p_vol);
        if (err != 0) {
            return err;
        }
        p_vol->fsi_flag &= ~(PF_FSI_FLAG_USE | PF_FSI_FLAG_UPDATE | PF_FSI_FLAG_FREE_COUNT_VALID);
        PFDRV_ClearUnmountRequested(p_vol);
        pf_vol_set.num_mounted_volumes++;
    }
    PFDRV_ClearMountRequested(p_vol);
    return 0;
}

s32 PFVOL_p_unmount(struct PF_VOLUME* p_vol, u32 mode) {
    s32 err;
    s32 err2;

    err = 0;
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }

    PFFILE_FinalizeAllFiles(p_vol);
    PFDIR_FinalizeAllDirs(p_vol);
    err2 = PFCACHE_FlushAllCaches(p_vol);
    if (err2 != 0) {
        err = err2;
    }

    if (err2 == 0 || (mode & PF_UNMOUNT_FORCE) != 0) {
        PFCACHE_FreeAllCaches(p_vol);
        err2 = PFVOL_DoUnmountVolume(p_vol, mode);
        if (err2 != 0 && err == 0) {
            err = err2;
        }
    }

    if (err == 0 || (mode & PF_UNMOUNT_FORCE) != 0) {
        PFDRV_ClearMountRequested(p_vol);
        PFDRV_ClearUnmountRequested(p_vol);
        pf_vol_set.num_mounted_volumes--;
    }
    return err;
}

s32 PFVOL_p_format(struct PF_VOLUME* p_vol, const u8* param) {
    u32 ecl;
    s32 err;

    if (p_vol == NULL) {
        return PF_IERR_INVALID_PARAM;
    }
    if (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED) {
        return PF_IERR_WRITE_PROTECTED;
    }
    err = PFDRV_format(p_vol, param);
    if (err != 0) {
        return err;
    }
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) != 0) {
        PFCACHE_FreeAllCaches(p_vol);
        err = PFVOL_p_unmount(p_vol, PF_UNMOUNT_NORMAL);
        if (err != 0) {
            return err;
        }
    }
    err = PFVOL_p_mount(p_vol);
    if (err != 0) {
        return err;
    }
    if (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED) {
        return PF_IERR_WRITE_PROTECTED;
    }

    if ((p_vol->flags & PF_VOL_FLAG_NO_LOGICAL_FORMAT) != 0) {
        if (p_vol->bpb.fat_type == FAT_32 && (p_vol->fsi_flag & PF_FSI_FLAG_UPDATE) != 0) {
            p_vol->fsi_flag |= PF_FSI_FLAG_FREE_COUNT_VALID;
            PFFAT_RefreshFSINFO(p_vol);
        } else {
            p_vol->fsi_flag &= ~PF_FSI_FLAG_FREE_COUNT_VALID;
            if (p_vol->bpb.fat_type != FAT_32) {
                p_vol->fsi_flag &= ~PF_FSI_FLAG_USE;
                p_vol->fsi_flag &= ~PF_FSI_FLAG_UPDATE;
            }
            err = PFFAT_CountFreeClusters(p_vol, &ecl);
            if (err != 0) {
                return err;
            }
        }
        return 0;
    }

    err = PFFAT_InitFATRegion(p_vol);
    if (err != 0) {
        return err;
    }
    err = PFENT_MakeRootDir(p_vol);
    if (err != 0) {
        return err;
    }
    if (p_vol->bpb.fat_type != FAT_32) {
        p_vol->fsi_flag &= ~PF_FSI_FLAG_USE;
        p_vol->fsi_flag &= ~PF_FSI_FLAG_UPDATE;
    }
    if (p_vol->bpb.fat_type == FAT_32 && (p_vol->fsi_flag & PF_FSI_FLAG_UPDATE) != 0) {
        err = PFFAT_RefreshFSINFO(p_vol);
        if (err != 0) {
            return err;
        }
    }
    return 0;
}

s32 PFVOL_p_setvol(struct PF_VOLUME* p_vol, const s8* label) {
    u32 next_chain;
    u32 lpos;
    u32 ppos;
    struct PF_FAT_HINT hint;
    struct PF_STR str;
    struct PF_FFD ffd;
    struct PF_DIR_ENT root;
    struct PF_DIR_ENT ent;
    s32 err;

    err = PFENT_GetRootDir(p_vol, &root);
    if (err != 0) {
        return err;
    }
    PFFAT_InitFFD(&ffd, &hint, p_vol, &root.start_cluster);
    PFSTR_InitStr(&str, (const s8*)"*", PF_STR_CODEMODE_OEM);
    PFSTR_SetLocalStr(&str, NULL);
    err = PFENT_findEntryPos(&ffd, &ent, 0, &str, PF_ATTR_VOLUME, PF_ATTR_NONE, &lpos, &ppos);
    if (err != 0 && err != PF_IERR_NOT_FOUND) {
        return err;
    }

    if (ppos != PF_ENTRY_INDEX_END) {
        if (ent.start_cluster == PF_CLUSTER_ROOT_DIR) {
            ent.start_cluster = 0;
        }
    } else {
        PFSTR_InitStr(&str, (const s8*)"", PF_STR_CODEMODE_OEM);
        PFSTR_SetLocalStr(&str, NULL);
        err = PFENT_allocateEntry(&ent, 1, &ffd, &next_chain, &str);
        if (err != 0) {
            return err;
        }
        ent.long_name[0] = 0;
        ent.start_cluster = 0;
        ent.file_size = 0;
        ent.p_vol = p_vol;
        ent.small_letter_flag = 0;
        ent.attr = PF_ATTR_VOLUME;
        ent.path_len = PFENT_ROOT_PATH_LEN;
        ent.create_date = 0;
        ent.create_time = 0;
        ent.access_date = 0;
        ent.create_time_ms = 0;
    }

    pf_memcpy(ent.short_name, label, PDM_BPB_VOL_LABEL_LEN + 1);
    PFENT_getcurrentDateTimeForEnt(&ent.modify_date, &ent.modify_time);
    return PFENT_updateEntry(&ent, PFENT_UPDATE_KEEP_ATTR);
}

s32 PFVOL_p_getvol(struct PF_VOLUME* p_vol, struct PF_VOLTAB* p_voltab) {
    u32 lpos;
    u32 ppos;
    struct PF_FAT_HINT hint;
    struct PF_STR str;
    struct PF_FFD ffd;
    struct PF_DIR_ENT root;
    struct PF_DIR_ENT ent;
    s32 err;

    err = PFENT_GetRootDir(p_vol, &root);
    if (err != 0) {
        return err;
    }
    PFFAT_InitFFD(&ffd, &hint, p_vol, &root.start_cluster);
    PFSTR_InitStr(&str, (const s8*)"*", PF_STR_CODEMODE_OEM);
    PFSTR_SetLocalStr(&str, (s8*)"*");
    err = PFENT_findEntryPos(&ffd, &ent, 0, &str, PF_ATTR_VOLUME, PF_ATTR_NONE, &lpos, &ppos);
    if (err != 0 && err != PF_IERR_NOT_FOUND) {
        return err;
    }

    if (ppos != PF_ENTRY_INDEX_END) {
        p_voltab->v_date = ent.modify_date;
        p_voltab->v_time = ent.modify_time;
        p_voltab->v_attr = ent.attr;
        pf_strcpy(p_voltab->v_name, ent.short_name);
    } else {
        p_voltab->v_date = 0;
        p_voltab->v_time = 0;
        p_voltab->v_attr = PF_ATTR_VOLUME;
        PFDRV_StoreVolumeLabelToBuf((u8*)p_voltab->v_name, p_vol);
    }
    return 0;
}

s32 PFVOL_p_rmvvol(struct PF_VOLUME* p_vol) {
    u8 buf[1] = {PF_ENTRY_DELETED};
    u32 lpos;
    u32 ppos;
    u32 success_size;
    struct PF_FAT_HINT hint;
    struct PF_STR str;
    struct PF_FFD ffd;
    struct PF_DIR_ENT root;
    struct PF_DIR_ENT ent;
    s32 err;

    err = PFENT_GetRootDir(p_vol, &root);
    if (err != 0) {
        return err;
    }
    PFFAT_InitFFD(&ffd, &hint, p_vol, &root.start_cluster);
    PFSTR_InitStr(&str, (const s8*)"*", PF_STR_CODEMODE_OEM);
    PFSTR_SetLocalStr(&str, (s8*)"*");
    err = PFENT_findEntryPos(&ffd, &ent, 0, &str, PF_ATTR_VOLUME, PF_ATTR_NONE, &lpos, &ppos);
    if (err != 0 && err != PF_IERR_NOT_FOUND) {
        return err;
    }

    if (ppos != PF_ENTRY_INDEX_END) {
        err = PFSEC_WriteData(p_vol, buf, ent.entry_sector, ent.entry_offset, 1, &success_size, FALSE);
        if (err != 0) {
            return err;
        }
        if (success_size != 1) {
            return PF_IERR_IO;
        }
    }
    return 0;
}

static s32 PFVOL_CheckMediaInsert(struct PF_VOLUME* p_vol) {
    s32 err;

    if (PFDRV_IsInserted(p_vol)) {
        if (PFDRV_IsUnmountRequested(p_vol) && (p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
            PFVOL_UnmountVolumeByEject(p_vol);
        }
        if (PFDRV_IsMountRequested(p_vol)) {
            err = PFVOL_p_mount(p_vol);
            if (err != 0) {
                return err;
            }
        }
    } else {
        if (PFDRV_IsUnmountRequested(p_vol) && (p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
            PFVOL_UnmountVolumeByEject(p_vol);
        }
        if (PFDRV_IsMountRequested(p_vol)) {
            PFDRV_ClearMountRequested(p_vol);
        }
    }
    return 0;
}

s32 PFVOL_InitModule(u32 config, void* param) {
    s32 vol_idx;
    u32 i;

    if (config & PF_CONFIG_NO_CHAR_CHECK) {
        pf_vol_set.config |= PF_CONFIG_NO_CHAR_CHECK;
    } else {
        pf_vol_set.config &= ~PF_CONFIG_NO_CHAR_CHECK;
    }

    for (i = 0; i < PF_CONTEXT_COUNT; i++) {
        pf_vol_set.current_vol[i].p_vol = &pf_vol_set.volumes[0];
    }
    pf_vol_set.current_vol[0].stat |= PF_STAT_USED;
    pf_vol_set.num_attached_drives = 0;
    pf_vol_set.num_mounted_volumes = 0;

    if (config & PF_CONFIG_NO_CHAR_CHECK) {
        pf_vol_set.config |= PF_CONFIG_NO_CHAR_CHECK;
    } else {
        pf_vol_set.config &= ~PF_CONFIG_NO_CHAR_CHECK;
    }

    pf_vol_set.param = param;
    pf_vol_set.last_error = 0;
    pf_vol_set.last_driver_error = 0;
    pf_vol_set.setting = PF_ENCODE_MODE_OEM;
    pf_vol_set.codeset.oem2unicode = &PFCODE_CP932_OEM2Unicode;
    pf_vol_set.codeset.unicode2oem = &PFCODE_CP932_Unicode2OEM;
    pf_vol_set.codeset.oem_char_width = &PFCODE_CP932_OEMCharWidth;
    pf_vol_set.codeset.is_oem_mb_char = &PFCODE_CP932_isOEMMBchar;
    pf_vol_set.codeset.unicode_char_width = &PFCODE_CP932_UnicodeCharWidth;
    pf_vol_set.codeset.is_unicode_mb_char = &PFCODE_CP932_isUnicodeMBchar;

    for (vol_idx = 0; vol_idx < PF_DRIVE_COUNT; vol_idx++) {
        pf_memset(&pf_vol_set.volumes[vol_idx], 0, sizeof(struct PF_VOLUME));
    }

    PF_InitLockFile();
    return 0;
}

s32 PFVOL_CheckForRead(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFVOL_CheckMediaInsert(p_vol);
    if (err != 0) {
        return err;
    }
    if (p_vol->flags & PF_VOL_FLAG_MOUNTED) {
        return 0;
    }
    return PF_IERR_NOT_MOUNTED;
}

s32 PFVOL_CheckForWrite(struct PF_VOLUME* p_vol) {
    s32 err;

    err = PFVOL_CheckMediaInsert(p_vol);
    if (err != 0) {
        return err;
    }
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }
    if (p_vol->flags & PF_VOL_FLAG_WRITE_PROTECTED) {
        return PF_IERR_WRITE_PROTECTED;
    }
    return 0;
}

s32 PFVOL_CheckCurrentDir(struct PF_VOLUME* p_vol, u32 start_cluster) {
    s32 result;
    s32 context_id;
    u32 i;

    result = 0;
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }

    PFSYS_GetCurrentContextID(&context_id);
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((p_vol->current_dir[i].stat & PF_STAT_USED) != 0 && p_vol->current_dir[i].context_id == context_id) {
            break;
        }
    }
    if (i == PF_CONTEXT_COUNT) {
        i = 0;
    }
    if (start_cluster == p_vol->current_dir[i].directory.start_cluster) {
        result = PF_VOL_CURRENT_DIR_MATCH;
    }
    return result;
}

s32 PFVOL_SetCurrentDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_dir) {
    s32 context_id;
    u32 i;

    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }

    PFSYS_GetCurrentContextID(&context_id);
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((p_vol->current_dir[i].stat & PF_STAT_USED) != 0 && p_vol->current_dir[i].context_id == context_id) {
            p_vol->current_dir[i].directory = *p_dir;
            break;
        }
    }

    if (i == PF_CONTEXT_COUNT) {
        if (PFVOL_CheckContextRegistered(context_id)) {
            for (i = 1; i < PF_CONTEXT_COUNT; i++) {
                if ((p_vol->current_dir[i].stat & PF_STAT_USED) == 0) {
                    p_vol->current_dir[i].stat |= PF_STAT_USED;
                    p_vol->current_dir[i].context_id = context_id;
                    p_vol->current_dir[i].directory = *p_dir;
                    break;
                }
            }
        }
    }
    p_vol->current_dir[0].directory = *p_dir;
    return 0;
}

s32 PFVOL_GetCurrentDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_current_dir) {
    s32 context_id;
    u32 i;

    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return PF_IERR_NOT_MOUNTED;
    }

    PFSYS_GetCurrentContextID(&context_id);
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((p_vol->current_dir[i].stat & PF_STAT_USED) != 0 && p_vol->current_dir[i].context_id == context_id) {
            *p_current_dir = p_vol->current_dir[i].directory;
            break;
        }
    }

    if (i == PF_CONTEXT_COUNT) {
        if (PFVOL_CheckContextRegistered(context_id)) {
            for (i = 1; i < PF_CONTEXT_COUNT; i++) {
                if ((p_vol->current_dir[i].stat & PF_STAT_USED) == 0) {
                    p_vol->current_dir[i].stat |= PF_STAT_USED;
                    p_vol->current_dir[i].context_id = context_id;
                    *p_current_dir = p_vol->current_dir[i].directory;
                    break;
                }
            }
        } else {
            *p_current_dir = p_vol->current_dir[0].directory;
        }
    }
    return 0;
}

void PFVOL_SetCurrentVolume(struct PF_VOLUME* p_vol) {
    s32 context_id;
    u32 i;

    PFSYS_GetCurrentContextID(&context_id);
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((pf_vol_set.current_vol[i].stat & PF_STAT_USED) != 0 && pf_vol_set.current_vol[i].context_id == context_id) {
            pf_vol_set.current_vol[i].p_vol = p_vol;
            break;
        }
    }

    if (i == PF_CONTEXT_COUNT) {
        if (PFVOL_CheckContextRegistered(context_id)) {
            for (i = 1; i < PF_CONTEXT_COUNT; i++) {
                if ((pf_vol_set.current_vol[i].stat & PF_STAT_USED) == 0) {
                    pf_vol_set.current_vol[i].stat |= PF_STAT_USED;
                    pf_vol_set.current_vol[i].context_id = context_id;
                    pf_vol_set.current_vol[i].p_vol = p_vol;
                    break;
                }
            }
        }
    }
    pf_vol_set.current_vol[0].p_vol = p_vol;
}

struct PF_VOLUME* PFVOL_GetCurrentVolume(void) {
    struct PF_VOLUME* p_vol;
    s32 context_id;
    u32 i;

    p_vol = NULL;
    PFSYS_GetCurrentContextID(&context_id);
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((pf_vol_set.current_vol[i].stat & PF_STAT_USED) != 0 && pf_vol_set.current_vol[i].context_id == context_id) {
            p_vol = pf_vol_set.current_vol[i].p_vol;
            break;
        }
    }

    if (i == PF_CONTEXT_COUNT) {
        if (PFVOL_CheckContextRegistered(context_id)) {
            for (i = 1; i < PF_CONTEXT_COUNT; i++) {
                if ((pf_vol_set.current_vol[i].stat & PF_STAT_USED) == 0) {
                    pf_vol_set.current_vol[i].stat |= PF_STAT_USED;
                    pf_vol_set.current_vol[i].context_id = context_id;
                    p_vol = pf_vol_set.current_vol[i].p_vol;
                    break;
                }
            }
        } else {
            p_vol = pf_vol_set.current_vol[0].p_vol;
        }
    }
    return p_vol;
}

struct PF_VOLUME* PFVOL_GetVolumeFromDrvChar(s8 drv_char) {
    s16 vol_idx;

    vol_idx = pf_toupper(drv_char) - 'A';
    if (vol_idx < 0 || vol_idx >= PF_DRIVE_COUNT) {
        return NULL;
    }
    return &pf_vol_set.volumes[vol_idx];
}

s32 PFVOL_derrnum(s8 drv_char) {
    return PFVOL_GetVolumeFromDrvChar(drv_char)->last_driver_error;
}

s32 PFVOL_errnum(void) {
    return pf_vol_set.last_error;
}

s32 PFVOL_setvol(s8 drv_char, struct PF_STR* p_str) {
    s8 label[PDM_BPB_VOL_LABEL_LEN + 1];
    struct PF_VOLUME* p_vol;
    u32 len;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    len = PFSTR_StrLen(p_str);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    PFSTR_ToUpperNStr(p_str, PDM_BPB_VOL_LABEL_LEN, label);
    if (len < PDM_BPB_VOL_LABEL_LEN) {
        label[len] = '\0';
    }
    err = PFVOL_p_setvol(p_vol, label);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

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
    if (err == 0) {
        err = PFDRV_StoreVolumeLabelToBPB(p_vol, (const u8*)label);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
        }
    }
    return err;
}

s32 PFVOL_getvol(s8 drv_char, struct PF_VOLTAB* p_voltab) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if (pf_memcmp(p_vol->label, PDM_BPB_VOL_LABEL_NO_NAME, PDM_BPB_VOL_LABEL_LEN) == 0) {
        pf_vol_set.last_error = PF_IERR_NOT_FOUND;
        p_vol->last_error = PF_IERR_NOT_FOUND;
        return PF_IERR_NOT_FOUND;
    }
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFVOL_p_getvol(p_vol, p_voltab);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    return 0;
}

s32 PFVOL_rmvvol(s8 drv_char) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if (pf_memcmp(p_vol->label, PDM_BPB_VOL_LABEL_NO_NAME, PDM_BPB_VOL_LABEL_LEN) == 0) {
        pf_vol_set.last_error = PF_IERR_NOT_FOUND;
        p_vol->last_error = PF_IERR_NOT_FOUND;
        return PF_IERR_NOT_FOUND;
    }
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFDRV_StoreVolumeLabelToBPB(p_vol, (const u8*)PDM_BPB_VOL_LABEL_NO_NAME);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFVOL_p_rmvvol(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFCACHE_FlushDataCacheSpecific(p_vol, NULL);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
    }
    return err;
}

s32 PFVOL_getdev(s8 drv_char, struct PF_DEV_INF* dev_inf) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    dev_inf->bps = p_vol->bpb.bytes_per_sector;
    dev_inf->spc = p_vol->bpb.sectors_per_cluster;
    dev_inf->cls = p_vol->bpb.num_clusters;
    err = PFFAT_CountFreeClusters(p_vol, &dev_inf->ecl);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    return 0;
}

s32 PFVOL_buffering(s8 drv_char, u8 mode) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    if ((mode & PF_CACHE_MODE_SYNC_UPDATES) && (p_vol->cache.mode & PF_CACHE_MODE_WRITE_THROUGH) == 0) {
        err = PFCACHE_FlushFATCache(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
            return err;
        }
    }
    if (mode & PF_CACHE_MODE_WRITE_THROUGH) {
        err = PFCACHE_SetWriteThroughMode(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
            return err;
        }
    } else {
        err = PFCACHE_SetWriteBackMode(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
            return err;
        }
    }

    if (mode & PF_CACHE_MODE_FLUSH_ON_CLOSE) {
        p_vol->cache.mode |= PF_CACHE_MODE_FLUSH_ON_CLOSE;
    } else {
        p_vol->cache.mode &= ~PF_CACHE_MODE_FLUSH_ON_CLOSE;
    }
    if (mode & PF_CACHE_MODE_SYNC_UPDATES) {
        p_vol->cache.mode |= PF_CACHE_MODE_SYNC_UPDATES;
    } else {
        p_vol->cache.mode &= ~PF_CACHE_MODE_SYNC_UPDATES;
    }
    return 0;
}

s32 PFVOL_attach(struct PF_DRV_TBL* p_drv, struct PF_ATTACH_PARAM* p_param, u32 nand_size) {
    s32 vol_idx;
    struct PF_VOLUME* p_vol;
    s32 err;

    if (p_drv->cache->num_fat_buf_size == 0) {
        p_drv->cache->num_fat_buf_size = 1;
    }
    if (p_drv->cache->num_data_buf_size == 0) {
        p_drv->cache->num_data_buf_size = 1;
    }
    p_drv->stat = 0;
    p_drv->drive = 0;

    if (p_param != NULL && fa_nanddrv_NotifyNANDFile((const char*)p_param->path, nand_size) != 0) {
        pf_vol_set.last_error = PF_IERR_IO;
        return PF_IERR_IO;
    }

    for (vol_idx = 0; vol_idx < PF_DRIVE_COUNT; vol_idx++) {
        p_vol = &pf_vol_set.volumes[vol_idx];
        if ((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) {
            break;
        }
    }
    if (vol_idx < 0 || vol_idx >= PF_DRIVE_COUNT || pf_vol_set.num_attached_drives < 0 || pf_vol_set.num_attached_drives >= PF_DRIVE_COUNT) {
        pf_vol_set.last_error = PF_IERR_NO_FREE_DRIVE;
        return PF_IERR_NO_FREE_DRIVE;
    }

    p_vol->num_free_clusters = PDM_FSI_UNKNOWN;
    p_vol->last_free_cluster = PDM_FSI_UNKNOWN;
    err = PFVOL_p_attach(p_vol, p_drv, vol_idx);
    if (err != 0) {
        pf_vol_set.last_error = err;
        return err;
    }

    p_vol->flags |= PF_VOL_FLAG_ATTACHED;
    p_drv->stat |= PF_DRV_STAT_ATTACHED;
    p_vol->drv_char = vol_idx + 'A';
    p_drv->drive = vol_idx + 'A';
    pf_vol_set.num_attached_drives++;

    if (PFDRV_IsInserted(p_vol)) {
        p_drv->stat |= PF_DRV_STAT_INSERTED;
        err = PFVOL_p_mount(p_vol);
        if (err != 0) {
            pf_vol_set.last_error = err;
            p_vol->last_error = err;
            return 0;
        }
        p_drv->stat |= PF_DRV_STAT_MOUNTED;
    }
    return 0;
}

s32 PFVOL_detach(s8 drv_char) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) != 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    if ((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }

    err = PFVOL_p_detach(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->flags &= ~PF_VOL_FLAG_ATTACHED;
    pf_vol_set.num_attached_drives--;
    return 0;
}

s32 PFVOL_format(s8 drv_char, const u8* param) {
    struct PF_VOLUME* p_vol;
    BOOL mount_requested;
    s32 err;

    mount_requested = FALSE;
    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if ((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        p_vol->last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }
    if (!PFDRV_IsInserted(p_vol)) {
        pf_vol_set.last_error = PF_IERR_NOT_MOUNTED;
        p_vol->last_error = PF_IERR_NOT_MOUNTED;
        return PF_IERR_NOT_MOUNTED;
    }
    if (PFDRV_IsMountRequested(p_vol)) {
        PFDRV_ClearMountRequested(p_vol);
        mount_requested = TRUE;
    }

    err = PFVOL_CheckMediaInsert(p_vol);
    if (err != 0) {
        if (mount_requested) {
            p_vol->flags |= PF_VOL_FLAG_MOUNT_REQUESTED;
        }
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    if (p_vol->num_opened_files != 0) {
        PFFILE_FinalizeAllFiles(p_vol);
        PFCACHE_FreeAllCaches(p_vol);
    }
    if (p_vol->num_opened_directories != 0) {
        PFDIR_FinalizeAllDirs(p_vol);
    }

    err = PFVOL_p_format(p_vol, param);
    if (err != 0) {
        if (mount_requested) {
            p_vol->flags |= PF_VOL_FLAG_MOUNT_REQUESTED;
        }
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    p_vol->fsi_flag |= PF_FSI_FLAG_FREE_COUNT_VALID;
    if (p_vol->bpb.fat_type == FAT_32) {
        p_vol->num_free_clusters = p_vol->bpb.num_clusters - 1;
    } else {
        p_vol->num_free_clusters = p_vol->bpb.num_clusters;
    }
    return 0;
}

s32 PFVOL_mount(s8 drv_char) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckMediaInsert(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if (!PFDRV_IsInserted(p_vol)) {
        pf_vol_set.last_error = PF_IERR_NOT_MOUNTED;
        p_vol->last_error = PF_IERR_NOT_MOUNTED;
        return PF_IERR_NOT_MOUNTED;
    }
    if (p_vol->flags & PF_VOL_FLAG_MOUNTED) {
        return 0;
    }

    err = PFVOL_p_mount(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    return 0;
}

s32 PFVOL_unmount(s8 drv_char, u32 mode) {
    struct PF_VOLUME* p_vol;
    s32 err;
    s32 err2;

    err2 = 0;
    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if ((p_vol->flags & PF_VOL_FLAG_ATTACHED) == 0) {
        pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
        return PF_IERR_INVALID_PARAM;
    }

    PFDRV_ClearMountRequested(p_vol);
    err = PFVOL_CheckMediaInsert(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if ((p_vol->flags & PF_VOL_FLAG_MOUNTED) == 0) {
        return 0;
    }

    if (p_vol->num_opened_files != 0 || p_vol->num_opened_directories != 0) {
        err2 = PF_RET_UNMOUNT_FORCED;
        pf_vol_set.last_error = PF_IERR_IN_USE;
        p_vol->last_error = PF_IERR_IN_USE;
        if ((mode & PF_UNMOUNT_FORCE) == 0) {
            return PF_IERR_IN_USE;
        }
    }

    err = PFVOL_p_unmount(p_vol, mode);
    if (err != 0 && err2 == 0) {
        err2 = PF_RET_UNMOUNT_FORCED;
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        if ((mode & PF_UNMOUNT_FORCE) == 0) {
            err2 = err;
        }
    }
    return err2;
}

s32 PFVOL_setupfsi(s8 drv_char, u16 mode) {
    u32 num_free_clusters;
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if (p_vol->bpb.fat_type != FAT_32) {
        pf_vol_set.last_error = PF_IERR_NOT_SUPPORTED;
        p_vol->last_error = PF_IERR_NOT_SUPPORTED;
        return PF_IERR_NOT_SUPPORTED;
    }

    if ((mode & PF_FSI_MODE_USE_ON) == PF_FSI_MODE_USE_ON) {
        p_vol->fsi_flag |= (PF_FSI_FLAG_USE | PF_FSI_FLAG_FREE_COUNT_VALID);
    } else if ((mode & PF_FSI_MODE_USE_OFF) == PF_FSI_MODE_USE_OFF) {
        p_vol->fsi_flag &= ~PF_FSI_FLAG_USE;
    }
    if ((mode & PF_FSI_MODE_UPDATE_ON) == PF_FSI_MODE_UPDATE_ON) {
        p_vol->fsi_flag |= PF_FSI_FLAG_UPDATE;
        if ((p_vol->fsi_flag & PF_FSI_FLAG_FREE_COUNT_VALID) == 0) {
            PFFAT_CountFreeClusters(p_vol, &num_free_clusters);
        }
    } else if ((mode & PF_FSI_MODE_UPDATE_OFF) == PF_FSI_MODE_UPDATE_OFF) {
        p_vol->fsi_flag &= ~PF_FSI_FLAG_UPDATE;
    }
    return 0;
}

s32 PFVOL_setclstlink(s8 drv_char, u32 mode, struct PF_CLUSTER_LINK_RES* p_link) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    if (p_vol->num_opened_files != 0) {
        pf_vol_set.last_error = PF_IERR_IN_USE;
        p_vol->last_error = PF_IERR_IN_USE;
        return PF_IERR_IN_USE;
    }
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    if (mode == PF_CLUSTER_LINK_ENABLE) {
        if (p_link->buffer == NULL) {
            pf_vol_set.last_error = PF_IERR_INVALID_PARAM;
            p_vol->last_error = PF_IERR_INVALID_PARAM;
            return PF_IERR_INVALID_PARAM;
        }
        p_vol->cluster_link.flag |= PF_CLUSTER_LINK_FLAG_ENABLED;
        p_vol->cluster_link.buffer = p_link->buffer;
        pf_memset(p_link->buffer, 0, p_link->max_count * PF_SFD_COUNT * sizeof(u32));
        p_vol->cluster_link.interval = p_link->interval;
        p_vol->cluster_link.link_max = p_link->max_count;
    } else {
        p_vol->cluster_link.flag &= ~PF_CLUSTER_LINK_FLAG_ENABLED;
        p_vol->cluster_link.buffer = NULL;
        p_vol->cluster_link.interval = 0;
        p_vol->cluster_link.link_max = 0;
    }
    return err;
}

s32 PFVOL_sync(s8 drv_char, u32 mode) {
    struct PF_VOLUME* p_vol;
    u16 i;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckForWrite(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    err = PFCACHE_FlushFATCache(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }

    for (i = 0; i < PF_UFD_COUNT; i++) {
        if (p_vol->ufds[i].stat & PF_STAT_USED) {
            err = PFENT_updateEntry(&p_vol->ufds[i].p_sfd->dir_entry, PFENT_UPDATE_SET_ARCHIVE);
            if (err != 0) {
                pf_vol_set.last_error = err;
                p_vol->last_error = err;
                return err;
            }
        }
    }

    err = PFCACHE_FlushDataCache(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    if (mode == PF_SYNC_FREE_CACHES) {
        PFCACHE_FreeAllCaches(p_vol);
    }
    return err;
}

s32 PFVOL_settailbuf(s8 drv_char, u32 size, u32* buf) {
    struct PF_VOLUME* p_vol;
    s32 err;

    p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
    err = PFVOL_CheckForRead(p_vol);
    if (err != 0) {
        pf_vol_set.last_error = err;
        p_vol->last_error = err;
        return err;
    }
    p_vol->tail_entry.tracker_size = size;
    p_vol->tail_entry.tracker_bits = buf;
    return err;
}

s32 PFVOL_setvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg) {
    struct PF_VOLUME* p_vol;

    if (drv_char == PF_DRV_CHAR_ALL) {
        if (p_cfg->volume_config & PF_CONFIG_NO_CHAR_CHECK) {
            pf_vol_set.config |= PF_CONFIG_NO_CHAR_CHECK;
        } else if (p_cfg->volume_config & PF_CONFIG_CHAR_CHECK) {
            pf_vol_set.config &= ~PF_CONFIG_NO_CHAR_CHECK;
        }
    } else {
        p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
        if (p_cfg->fat_buff_size == 0) {
            p_cfg->fat_buff_size = 1;
        }
        if (p_cfg->data_buff_size == 0) {
            p_cfg->data_buff_size = 1;
        }
        PFVOL_CheckMediaInsert(p_vol);
        if (p_vol->num_opened_files != 0 || p_vol->num_opened_directories != 0) {
            pf_vol_set.last_error = PF_IERR_IN_USE;
            p_vol->last_error = PF_IERR_IN_USE;
            return PF_IERR_IN_USE;
        }

        if ((p_cfg->volume_config & PF_VOL_CFG_SET_FAT_BUFF_SIZE) || (p_cfg->volume_config & PF_VOL_CFG_SET_DATA_BUFF_SIZE)) {
            PFCACHE_FlushAllCaches(p_vol);
            PFCACHE_FreeAllCaches(p_vol);
            if ((p_cfg->volume_config & PF_VOL_CFG_SET_FAT_BUFF_SIZE) && p_cfg->fat_buff_size != p_vol->cache.fat_buff_size) {
                PFCACHE_SetFATBufferSize(p_vol, p_cfg->fat_buff_size);
                p_cfg->volume_config &= ~PF_VOL_CFG_SET_FAT_BUFF_SIZE;
            }
            if ((p_cfg->volume_config & PF_VOL_CFG_SET_DATA_BUFF_SIZE) && p_cfg->data_buff_size != p_vol->cache.data_buff_size) {
                PFCACHE_SetDataBufferSize(p_vol, p_cfg->data_buff_size);
                p_cfg->volume_config &= ~PF_VOL_CFG_SET_DATA_BUFF_SIZE;
            }
            PFCACHE_InitCaches(p_vol);
        }

        if (p_cfg->file_config & PF_FILE_CONFIG_UPDATE_IF_MODIFIED) {
            p_vol->file_config |= PF_FILE_CONFIG_UPDATE_IF_MODIFIED;
        } else if (p_cfg->file_config & PF_FILE_CONFIG_UPDATE_ALWAYS) {
            p_vol->file_config &= ~PF_FILE_CONFIG_UPDATE_IF_MODIFIED;
        }
    }
    return 0;
}

s32 PFVOL_getvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg) {
    struct PF_VOLUME* p_vol;

    if (drv_char == PF_DRV_CHAR_ALL) {
        p_cfg->volume_config = pf_vol_set.config;
        if ((pf_vol_set.config & PF_CONFIG_NO_CHAR_CHECK) == 0) {
            p_cfg->volume_config |= PF_CONFIG_CHAR_CHECK;
        }
        p_cfg->file_config = 0;
        p_cfg->fat_buff_size = 0;
        p_cfg->data_buff_size = 0;
    } else {
        p_vol = PFVOL_GetVolumeFromDrvChar(drv_char);
        PFVOL_CheckMediaInsert(p_vol);
        p_cfg->volume_config = 0;
        p_cfg->file_config = p_vol->file_config;
        if ((p_vol->file_config & PF_FILE_CONFIG_UPDATE_IF_MODIFIED) != PF_FILE_CONFIG_UPDATE_IF_MODIFIED) {
            p_cfg->file_config |= PF_FILE_CONFIG_UPDATE_ALWAYS;
        }
        p_cfg->fat_buff_size = p_vol->cache.fat_buff_size;
        p_cfg->data_buff_size = p_vol->cache.data_buff_size;
    }
    return 0;
}

s32 PFVOL_setcode(struct PF_CHARCODE* p_codeset) {
    if (pf_vol_set.num_mounted_volumes > 0) {
        pf_vol_set.last_error = PF_IERR_VOLUME_MOUNTED;
        return PF_IERR_VOLUME_MOUNTED;
    }
    pf_vol_set.codeset.oem2unicode = p_codeset->oem2unicode;
    pf_vol_set.codeset.unicode2oem = p_codeset->unicode2oem;
    pf_vol_set.codeset.oem_char_width = p_codeset->oem_char_width;
    pf_vol_set.codeset.is_oem_mb_char = p_codeset->is_oem_mb_char;
    pf_vol_set.codeset.unicode_char_width = p_codeset->unicode_char_width;
    pf_vol_set.codeset.is_unicode_mb_char = p_codeset->is_unicode_mb_char;
    return 0;
}

s32 PFVOL_regctx(void) {
    s32 context_id;
    u32 i;
    u32 free_idx;
    s32 err;

    err = PFSYS_GetCurrentContextID(&context_id);
    if (err != 0) {
        pf_vol_set.last_error = PF_IERR_INTERNAL;
        return PF_IERR_INTERNAL;
    }

    free_idx = 0;
    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((pf_vol_set.context[i].stat & PF_STAT_USED) != 0 && pf_vol_set.context[i].context_id == context_id) {
            break;
        }
        if ((pf_vol_set.context[i].stat & PF_STAT_USED) == 0 && free_idx == 0) {
            free_idx = i;
        }
    }
    if (i == PF_CONTEXT_COUNT) {
        if (free_idx != 0) {
            pf_vol_set.context[free_idx].stat = PF_STAT_USED;
            pf_vol_set.context[free_idx].context_id = context_id;
        } else {
            pf_vol_set.last_error = PF_IERR_INTERNAL;
            return PF_IERR_INTERNAL;
        }
    }
    return err;
}

s32 PFVOL_unregctx(void) {
    s32 context_id;
    u32 i;
    u32 j;
    u32 vol_idx;
    s32 err;

    err = PFSYS_GetCurrentContextID(&context_id);
    if (err != 0) {
        pf_vol_set.last_error = PF_IERR_INTERNAL;
        return PF_IERR_INTERNAL;
    }

    for (i = 1; i < PF_CONTEXT_COUNT; i++) {
        if ((pf_vol_set.context[i].stat & PF_STAT_USED) != 0 && pf_vol_set.context[i].context_id == context_id) {
            pf_vol_set.context[i].stat &= ~PF_STAT_USED;
            for (j = 1; j < PF_CONTEXT_COUNT; j++) {
                if ((pf_vol_set.current_vol[j].stat & PF_STAT_USED) != 0 && pf_vol_set.current_vol[j].context_id == context_id) {
                    pf_vol_set.current_vol[j].stat &= ~PF_STAT_USED;
                    break;
                }
            }
            for (vol_idx = 0; vol_idx < PF_DRIVE_COUNT; vol_idx++) {
                for (j = 1; j < PF_CONTEXT_COUNT; j++) {
                    if ((pf_vol_set.volumes[vol_idx].current_dir[j].stat & PF_STAT_USED) != 0 && pf_vol_set.volumes[vol_idx].current_dir[j].context_id == context_id) {
                        pf_vol_set.volumes[vol_idx].current_dir[j].stat &= ~PF_STAT_USED;
                        break;
                    }
                }
            }
            break;
        }
    }
    if (i == PF_CONTEXT_COUNT) {
        pf_vol_set.last_error = PF_IERR_INTERNAL;
        return PF_IERR_INTERNAL;
    }
    return err;
}

s32 PFVOL_setencode(u32 mode) {
    pf_vol_set.setting = (pf_vol_set.setting & ~PF_ENCODE_MODE_MASK) | mode;
    return 0;
}
