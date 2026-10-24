#include <prfile2/pdm_bpb.h>
#include <prfile2/pdm_disk.h>
#include <prfile2/pf_clib.h>
#include <prfile2/pfd_drv.h>
#include <prfile2/pfd_nanddrv.h>
#include <prfile2/pfk_api.h>

static const PDM_FUNCTBL fa_nanddrv_functbl = {
    fa_nanddrv_init,
    fa_nanddrv_finalize,
    fa_nanddrv_mount,
    fa_nanddrv_unmount,
    fa_nanddrv_format,
    fa_nanddrv_pread,
    fa_nanddrv_pwrite,
    fa_nanddrv_get_disk_info,
};

char NAND_Org_Path[NAND_MAX_PATH];
FA_NANDDISK_INFO nanddisk_info[FA_NANDDISK_COUNT];
PFK_SEMAPHORE_INFO nanddisk_sem_info;
OSSemaphore nanddisk_sem;

s32 nanddisk_sem_id;
s32 nanddrv_init_count;
s32 nanddisk_init_count;

static s32 fa_nanddrv_GetDiskInfo(PDM_DISK* p_disk, FA_NANDDISK_INFO** pp_info) {
    u16 i;

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        if (nanddisk_info[i].p_disk == p_disk) {
            *pp_info = &nanddisk_info[i];
            break;
        }
    }
    if (i == FA_NANDDISK_COUNT) {
        return FA_NANDDRV_ERR_NOT_FOUND;
    }
    return 0;
}

static s32 fa_nanddrv_ClearLastError(PDM_DISK* p_disk) {
    u16 i;

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        if (nanddisk_info[i].p_disk == p_disk) {
            nanddisk_info[i].last_error = NAND_RESULT_OK;
            break;
        }
    }
    if (i == FA_NANDDISK_COUNT) {
        return FA_NANDDRV_ERR_NOT_FOUND;
    }
    return 0;
}

static void fa_nanddrv_SetLastError(PDM_DISK* p_disk, s32 err) {
    u16 i;

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        if (nanddisk_info[i].p_disk == p_disk) {
            nanddisk_info[i].last_error = err;
            break;
        }
    }
}

s32 fa_nanddrv_RelateNANDFileToDsk(PDM_DISK* p_disk, FA_NANDDISK_INFO* p_info, BOOL format) {
    s32 err;

    err = NANDChangeDir(NAND_Org_Path);
    if (err != NAND_RESULT_OK) {
        return err;
    }

    if (p_info->flags & FA_NANDDISK_FORMAT) {
        err = NAND_RESULT_OK;
    } else {
        err = NANDOpen(p_info->path, &p_info->file, NAND_ACCESS_RW);
    }
    if (format) {
        err = NAND_RESULT_NOEXISTS;
    }

    switch (err) {
        case NAND_RESULT_NOEXISTS:
            err = fa_nanddrv_CreateNANDFileForDsk(p_info);
            if (err == NAND_RESULT_OK) {
                p_info->p_disk = p_disk;
            }
            break;
        case NAND_RESULT_OK:
            p_info->p_disk = p_disk;
            return 0;
    }
    return err;
}

s32 fa_nanddrv_CreateNANDFileForDsk(FA_NANDDISK_INFO* p_info) {
    s32 err;
    u32 remain;
    s32 fat_type;
    u32 num_clusters;
    u8 zero[FA_NANDDRV_IO_BUF_SIZE] ATTRIBUTE_ALIGN(32);
    u8 boot[PDM_SECTOR_SIZE] ATTRIBUTE_ALIGN(32);
    u8 fsinfo[PDM_SECTOR_SIZE] ATTRIBUTE_ALIGN(32);

    err = fa_nanddrv_ParseCreateNANDFile(p_info);
    if (err != 0) {
        return err;
    }

    pf_memset(boot, 0, sizeof(boot));
    num_clusters = 0;
    fa_nanddrv_BuildUpBootSector(p_info, boot, &fat_type, num_clusters);
    if (fat_type != fa_nanddrv_VerifyBPB(boot, &num_clusters)) {
        fa_nanddrv_BuildUpBootSector(p_info, boot, &fat_type, num_clusters);
    }

    err = NANDWrite(&p_info->file, boot, sizeof(boot));
    if (err < 0) {
        NANDClose(&p_info->file);
        NANDDelete(p_info->path);
        return err;
    }
    remain = p_info->size - sizeof(boot);

    if (fat_type == PDM_FAT_32) {
        fa_nanddrv_BuildUpFSInfoSector(fsinfo);
        err = NANDWrite(&p_info->file, fsinfo, sizeof(fsinfo));
        if (err < 0) {
            NANDClose(&p_info->file);
            NANDDelete(p_info->path);
            return err;
        }
        remain -= sizeof(fsinfo);
    }

    pf_memset(zero, 0, sizeof(zero));
    while (remain >= sizeof(zero)) {
        err = NANDWrite(&p_info->file, zero, sizeof(zero));
        if (err < 0) {
            NANDClose(&p_info->file);
            NANDDelete(p_info->path);
            return err;
        }
        remain -= sizeof(zero);
    }
    if (remain != 0) {
        err = NANDWrite(&p_info->file, zero, remain);
        if (err < 0) {
            NANDClose(&p_info->file);
            NANDDelete(p_info->path);
            return err;
        }
    }
    return 0;
}

s32 fa_nanddrv_ParseCreateNANDFile(FA_NANDDISK_INFO* p_info) {
    s32 err;
    u16 len;
    char* p_src;
    u16 pos;
    char* p_dst;
    char name[FA_NANDDRV_NAME_BUF_LEN];

    len = 0;
    pos = 0;
    if (p_info->flags & FA_NANDDISK_FORMAT) {
        return 0;
    }

    p_src = p_info->path;
    pf_memset(name, 0, sizeof(name));
    p_dst = name;
    while (1) {
        switch (*p_src) {
            case '/':
                if (len == 0) {
                    if (pos == 0) {
                        err = NANDChangeDir("/");
                        if (err != NAND_RESULT_OK) {
                            return err;
                        }
                    }
                    p_src++;
                } else if (p_src[1] == '\0') {
                    if (len > FA_NANDDRV_FILE_NAME_MAX) {
                        return FA_NANDDRV_ERR_INVALID_PARAM;
                    }
                    err = NANDCreate(name, NAND_PERM_USER, 0);
                    if (err == NAND_RESULT_OK || err == NAND_RESULT_EXISTS) {
                        NANDOpen(name, &p_info->file, NAND_ACCESS_RW);
                        err = NANDChangeDir(NAND_Org_Path);
                    }
                    return err;
                } else {
                    if (len > FA_NANDDRV_DIR_NAME_MAX) {
                        return FA_NANDDRV_ERR_INVALID_PARAM;
                    }
                    if (pf_strcmp((const s8*)name, (const s8*)"..") != 0) {
                        err = NANDCreateDir(name, NAND_PERM_USER, 0);
                    } else {
                        err = NAND_RESULT_OK;
                    }
                    if (err == NAND_RESULT_OK || err == NAND_RESULT_EXISTS || err == NAND_RESULT_ACCESS) {
                        err = NANDChangeDir(name);
                        if (err != NAND_RESULT_OK) {
                            return err;
                        }
                    } else {
                        return err;
                    }
                    p_src++;
                    pf_memset(name, 0, sizeof(name));
                    p_dst = name;
                    len = 0;
                }
                break;
            case '\0':
                if (len > FA_NANDDRV_FILE_NAME_MAX) {
                    return FA_NANDDRV_ERR_INVALID_PARAM;
                }
                err = NANDCreate(name, NAND_PERM_USER, 0);
                if (err == NAND_RESULT_OK || err == NAND_RESULT_EXISTS) {
                    NANDOpen(name, &p_info->file, NAND_ACCESS_RW);
                    err = NANDChangeDir(NAND_Org_Path);
                }
                return err;
        }
        *p_dst++ = *p_src++;
        len++;
        pos++;
    }
}

s32 fa_nanddrv_VerifyBPB(u8* buf, u32* p_num_clusters) {
    u8 log2_bytes_per_sector;
    u8 log2_sectors_per_cluster;
    u16 val;
    u16 num_root_dir_sectors;
    u32 sectors_per_FAT;
    u32 first_data_sector;
    u32 total_sectors;
    u32 num_data_sectors;

    log2_bytes_per_sector = 0;
    val = PDM_SECTOR_SIZE;
    while ((val >>= 1) != 0) {
        log2_bytes_per_sector++;
    }
    log2_sectors_per_cluster = 0;
    val = FA_NANDDISK_SECTORS_PER_CLUSTER;
    while ((val >>= 1) != 0) {
        log2_sectors_per_cluster++;
    }

    num_root_dir_sectors = ((PDM_SECTOR_SIZE + (FA_LD16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES) * PDM_DIR_ENTRY_SIZE)) - 1) >> log2_bytes_per_sector;
    sectors_per_FAT = (u16)(FA_LD16(buf, PDM_BPB_SECTORS_PER_FAT16));
    if (sectors_per_FAT == 0) {
        sectors_per_FAT = FA_LD32(buf, PDM_BPB_SECTORS_PER_FAT32);
    }
    first_data_sector = num_root_dir_sectors + sectors_per_FAT * FA_NANDDISK_NUM_FATS + FA_NANDDISK_NUM_RESERVED_SECTORS;
    total_sectors = FA_LD16(buf, PDM_BPB_TOTAL_SECTORS16);
    if (total_sectors == 0) {
        total_sectors = FA_LD32(buf, PDM_BPB_TOTAL_SECTORS32);
    }

    num_data_sectors = total_sectors - first_data_sector;
    *p_num_clusters = num_data_sectors;
    return ((num_data_sectors >> log2_sectors_per_cluster) < PDM_FAT12_MAX_CLUSTERS) ? PDM_FAT_12
           : ((num_data_sectors >> log2_sectors_per_cluster) < PDM_FAT16_MAX_CLUSTERS) ? PDM_FAT_16
                                                                                       : PDM_FAT_32;
}

s32 fa_nanddrv_BuildUpFSInfoSector(u8* buf) {
    pf_memset(buf, 0, PDM_SECTOR_SIZE);
    FA_ST32(buf, PDM_FSI_LEAD_SIG, PDM_FSI_LEAD_SIG_VALUE);
    FA_ST32(buf, PDM_FSI_STRUC_SIG, PDM_FSI_STRUC_SIG_VALUE);
    FA_ST32(buf, PDM_FSI_FREE_COUNT, PDM_FSI_UNKNOWN);
    FA_ST32(buf, PDM_FSI_NXT_FREE, PDM_FSI_UNKNOWN);
    FA_ST32(buf, PDM_FSI_TRAIL_SIG, PDM_FSI_TRAIL_SIG_VALUE);
    return 0;
}

s32 fa_nanddrv_BuildUpBootSector(FA_NANDDISK_INFO* p_info, u8* buf, s32* p_fat_type, u32 num_clusters) {
    u32 num_sectors;
    u32 sectors_per_FAT;
    s32 fat_type;

    if (num_clusters != 0) {
        num_sectors = num_clusters;
    } else {
        num_sectors = p_info->size >> PDM_SECTOR_SHIFT;
    }
    if (num_sectors < PDM_FAT12_MAX_CLUSTERS) {
        fat_type = PDM_FAT_12;
        sectors_per_FAT = ((num_sectors * 3) / 2 + (PDM_SECTOR_SIZE - 1)) >> PDM_SECTOR_SHIFT;
    } else if (num_sectors < PDM_FAT16_MAX_CLUSTERS) {
        fat_type = PDM_FAT_16;
        sectors_per_FAT = (num_sectors * sizeof(u16) + (PDM_SECTOR_SIZE - 1)) >> PDM_SECTOR_SHIFT;
    } else {
        fat_type = PDM_FAT_32;
        sectors_per_FAT = (num_sectors * sizeof(u32) + (PDM_SECTOR_SIZE - 1)) >> PDM_SECTOR_SHIFT;
    }

    pf_memset(buf, 0, PDM_SECTOR_SIZE);
    buf[PDM_BPB_JUMP_BOOT] = PDM_BPB_JUMP_SHORT;
    buf[PDM_BPB_JUMP_BOOT + 2] = PDM_BPB_NOP;
    buf[PDM_BPB_SIGNATURE1] = PDM_BPB_SIGNATURE1_VALUE;
    buf[PDM_BPB_SIGNATURE2] = PDM_BPB_SIGNATURE2_VALUE;
    FA_ST16(buf, PDM_BPB_SECTORS_PER_TRACK, FA_NANDDISK_SECTORS_PER_TRACK);
    FA_ST16(buf, PDM_BPB_NUM_HEADS, FA_NANDDISK_HEADS);
    FA_ST16(buf, PDM_BPB_BYTES_PER_SECTOR, PDM_SECTOR_SIZE);
    buf[PDM_BPB_SECTORS_PER_CLUSTER] = FA_NANDDISK_SECTORS_PER_CLUSTER;
    FA_ST16(buf, PDM_BPB_NUM_RESERVED_SECTORS, FA_NANDDISK_NUM_RESERVED_SECTORS);
    buf[PDM_BPB_NUM_FATS] = FA_NANDDISK_NUM_FATS;
    FA_ST16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES, FA_NANDDISK_ROOT_DIR_ENTRIES);
    buf[PDM_BPB_MEDIA] = PDM_BPB_MEDIA_REMOVABLE;
    if (num_sectors < PDM_BPB_TOTAL_SECTORS16_LIMIT) {
        FA_ST16(buf, PDM_BPB_TOTAL_SECTORS16, num_sectors);
        FA_ST32(buf, PDM_BPB_TOTAL_SECTORS32, 0);
    } else {
        FA_ST16(buf, PDM_BPB_TOTAL_SECTORS16, 0);
        FA_ST32(buf, PDM_BPB_TOTAL_SECTORS32, num_sectors);
    }

    if (fat_type == PDM_FAT_32) {
        FA_ST16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES, 0);
        FA_ST32(buf, PDM_BPB_SECTORS_PER_FAT32, sectors_per_FAT);
        FA_ST16(buf, PDM_BPB_EXT_FLAGS, 0);
        FA_ST16(buf, PDM_BPB_FS_VERSION, 0);
        FA_ST32(buf, PDM_BPB_ROOT_DIR_CLUSTER, PF_CLUSTER_FIRST);
        FA_ST16(buf, PDM_BPB_FS_INFO_SECTOR, PDM_FSINFO_SECTOR);
        buf[PDM_BPB_RESERVED32] = 0;
        buf[PDM_BPB_BOOT_SIG32] = PDM_BPB_EXT_BOOT_SIG;
        FA_ST32(buf, PDM_BPB_VOL_ID32, FA_NANDDISK_VOL_ID);
    } else {
        FA_ST16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES, FA_NANDDISK_ROOT_DIR_ENTRIES);
        FA_ST16(buf, PDM_BPB_SECTORS_PER_FAT16, sectors_per_FAT);
        buf[PDM_BPB_RESERVED] = 0;
        buf[PDM_BPB_BOOT_SIG] = PDM_BPB_EXT_BOOT_SIG;
        FA_ST32(buf, PDM_BPB_VOL_ID, FA_NANDDISK_VOL_ID);
    }

    *p_fat_type = fat_type;
    return 0;
}

s32 fa_nanddrv_init(PDM_DISK* p_disk) {
    if (p_disk == NULL) {
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (nanddrv_init_count == 0) {
        if (NANDInit() != NAND_RESULT_OK) {
            return FA_NANDDRV_ERR_SYSTEM;
        }
        if (NANDGetCurrentDir(NAND_Org_Path) != NAND_RESULT_OK) {
            return FA_NANDDRV_ERR_SYSTEM;
        }
        nanddrv_init_count++;
    }
    pdm_disk_notify_media_insert(p_disk);
    return 0;
}

static inline FA_NANDDISK_INFO* fa_nanddrv_SearchMountable(PDM_DISK* p_disk) {
    FA_NANDDISK_INFO* p_info;
    u16 i;

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        p_info = &nanddisk_info[i];
        if (p_info->p_disk == p_disk && (p_info->flags & FA_NANDDISK_MOUNTED) == 0) {
            return p_info;
        }
    }
    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        p_info = &nanddisk_info[i];
        if (p_info->p_disk == NULL && p_info->path[0] != '\0') {
            return p_info;
        }
    }
    return NULL;
}

s32 fa_nanddrv_mount(PDM_DISK* p_disk) {
    s32 err;
    FA_NANDDISK_INFO* p_info;
    FA_NANDDISK_INFO* p_mounted;

    pfk_get_semaphore(nanddisk_sem_id);
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    if (fa_nanddrv_GetDiskInfo(p_disk, &p_mounted) == 0 && (p_mounted->flags & FA_NANDDISK_MOUNTED)) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_SYSTEM;
    }

    p_info = fa_nanddrv_SearchMountable(p_disk);
    if (p_info == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_NOT_FOUND;
    }

    if (p_info->p_disk != NULL) {
        err = 0;
    } else {
        err = fa_nanddrv_RelateNANDFileToDsk(p_disk, p_info, FALSE);
    }
    if (err == 0) {
        p_info->flags |= FA_NANDDISK_MOUNTED;
    }
    pfk_release_semaphore(nanddisk_sem_id);
    return err;
}

s32 fa_nanddrv_format(PDM_DISK* p_disk, const u8* param) {
    s32 err;
    FA_NANDDISK_INFO* p_info;

    p_info = NULL;
    pfk_get_semaphore(nanddisk_sem_id);
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (fa_nanddrv_GetDiskInfo(p_disk, &p_info) != 0) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    p_info->flags |= FA_NANDDISK_FORMAT;
    err = fa_nanddrv_RelateNANDFileToDsk(p_disk, p_info, TRUE);
    p_info->flags &= ~FA_NANDDISK_FORMAT;
    pfk_release_semaphore(nanddisk_sem_id);
    return err;
}

s32 fa_nanddrv_pread(PDM_DISK* p_disk, u8* p_buf, u32 block, u32 num_blocks, u32* p_num_success) {
    s32 err;

    pfk_get_semaphore(nanddisk_sem_id);
    *p_num_success = 0;
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (p_buf == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    err = fa_nanddrv_physical_read(num_blocks, p_buf, block, PDM_SECTOR_SIZE, p_num_success, p_disk);
    if (err != 0) {
        pfk_release_semaphore(nanddisk_sem_id);
        return err;
    }
    pfk_release_semaphore(nanddisk_sem_id);
    return 0;
}

s32 fa_nanddrv_pwrite(PDM_DISK* p_disk, const u8* p_buf, u32 block, u32 num_blocks, u32* p_num_success) {
    s32 err;

    pfk_get_semaphore(nanddisk_sem_id);
    *p_num_success = 0;
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (p_buf == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    err = fa_nanddrv_physical_write(num_blocks, p_buf, block, PDM_SECTOR_SIZE, p_num_success, p_disk);
    if (err != 0) {
        pfk_release_semaphore(nanddisk_sem_id);
        return err;
    }
    pfk_release_semaphore(nanddisk_sem_id);
    return 0;
}

static s32 fa_nanddrv_ReopenNANDFile(FA_NANDDISK_INFO* p_info) {
    s32 err;
    u32 pos;

    err = NANDTell(&p_info->file, &pos);
    if (err != NAND_RESULT_OK) {
        return err;
    }
    err = NANDClose(&p_info->file);
    if (err < 0) {
        return err;
    }
    err = NANDOpen(p_info->path, &p_info->file, NAND_ACCESS_RW);
    if (err < 0) {
        return err;
    }
    err = NANDSeek(&p_info->file, pos, NAND_SEEK_BEG);
    if (err < 0) {
        return err;
    }
    return 0;
}

s32 fa_nanddrv_unmount(PDM_DISK* p_disk) {
    s32 err;
    FA_NANDDISK_INFO* p_info;

    p_info = NULL;
    pfk_get_semaphore(nanddisk_sem_id);
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    err = fa_nanddrv_ClearLastError(p_disk);
    if (err == 0) {
        if (fa_nanddrv_GetDiskInfo(p_disk, &p_info) != 0) {
            pfk_release_semaphore(nanddisk_sem_id);
            return FA_NANDDRV_ERR_INVALID_PARAM;
        }

        p_info->flags &= ~FA_NANDDISK_MOUNTED;
        err = fa_nanddrv_ReopenNANDFile(p_info);
        if (err < 0) {
            fa_nanddrv_SetLastError(p_disk, err);
            pfk_release_semaphore(nanddisk_sem_id);
            return err;
        }
    }
    pfk_release_semaphore(nanddisk_sem_id);
    return err;
}

s32 fa_nanddrv_finalize(PDM_DISK* p_disk) {
    FA_NANDDISK_INFO* p_info;
    s32 err;
    u16 i;

    err = NAND_RESULT_OK;
    pfk_get_semaphore(nanddisk_sem_id);
    if (p_disk == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        p_info = &nanddisk_info[i];
        if (p_info->p_disk == p_disk) {
            err = NANDClose(&p_info->file);
            pf_memset(p_info, 0, sizeof(FA_NANDDISK_INFO));
            break;
        }
    }
    pfk_release_semaphore(nanddisk_sem_id);
    if (err != NAND_RESULT_OK) {
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (i == FA_NANDDISK_COUNT) {
        return FA_NANDDRV_ERR_NOT_FOUND;
    }
    return 0;
}

s32 fa_nanddrv_get_disk_info(PDM_DISK* p_disk, PDM_DISK_INFO* p_disk_info) {
    FA_NANDDISK_INFO* p_info;
    u32 total_sectors;

    p_info = NULL;
    pfk_get_semaphore(nanddisk_sem_id);
    if (p_disk == NULL || p_disk_info == NULL) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }
    if (fa_nanddrv_GetDiskInfo(p_disk, &p_info) != 0) {
        pfk_release_semaphore(nanddisk_sem_id);
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    total_sectors = p_info->size >> PDM_SECTOR_SHIFT;
    p_disk_info->total_sectors = total_sectors;
    p_disk_info->cylinders = total_sectors / FA_NANDDISK_HEADS / FA_NANDDISK_SECTORS_PER_TRACK / PDM_SECTOR_SIZE;
    p_disk_info->heads = FA_NANDDISK_HEADS;
    p_disk_info->sectors_per_track = FA_NANDDISK_SECTORS_PER_TRACK;
    p_disk_info->bytes_per_sector = PDM_SECTOR_SIZE;
    p_disk_info->media_attr = 0;
    p_disk_info->format_param = NULL;
    pfk_release_semaphore(nanddisk_sem_id);
    return 0;
}

s32 fa_nanddrv_init_drv_tbl(PDM_DISK_TBL* p_disk_tbl, u32 ui_ext) {
    p_disk_tbl->p_func = (PDM_FUNCTBL*)&fa_nanddrv_functbl;
    p_disk_tbl->ui_ext = ui_ext;
    return 0;
}

s32 fa_nanddrv_NotifyNANDFile(const char* path, u32 size) {
    FA_NANDDISK_INFO* p_info;
    u16 i;

    if (pf_strlen((const s8*)path) >= FA_NANDDISK_PATH_LEN_MAX) {
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    if (nanddisk_init_count == 0) {
        pf_memset(nanddisk_info, 0, sizeof(nanddisk_info));
        nanddisk_init_count++;
        nanddisk_sem_info.initial = 1;
        nanddisk_sem_info.max = 1;
        nanddisk_sem_info.sem = &nanddisk_sem;
        nanddisk_sem_id = pfk_create_semaphore(&nanddisk_sem_info);
        if (nanddisk_sem_id == 0) {
            return FA_NANDDRV_ERR_SYSTEM;
        }
    }

    for (i = 0; i < FA_NANDDISK_COUNT; i++) {
        p_info = &nanddisk_info[i];
        if (pf_strcmp((const s8*)p_info->path, (const s8*)path) == 0) {
            return 0;
        }
        if (p_info->p_disk == NULL) {
            pf_strcpy((s8*)nanddisk_info[i].path, (const s8*)path);
            nanddisk_info[i].size = size;
            break;
        }
    }
    return (i == FA_NANDDISK_COUNT) ? FA_NANDDRV_ERR_NOT_FOUND : 0;
}

s32 fa_nanddrv_physical_read(u32 num_blocks, u8* buf, u32 block, u32 bps, u32* p_num_success, PDM_DISK* p_disk) {
    s32 err;
    u32 fileSize;
    u32 offset;
    FA_NANDDISK_INFO* p_info;
    u32 done;
    u32 size;
    u32 pos;
    u8 tmp[FA_NANDDRV_IO_BUF_SIZE] ATTRIBUTE_ALIGN(32);

    p_info = NULL;
    if (fa_nanddrv_GetDiskInfo(p_disk, &p_info) != 0) {
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    size = num_blocks * bps;
    fileSize = p_info->size;
    offset = block * bps;
    if (NANDTell(&p_info->file, &pos) != NAND_RESULT_OK || pos != offset) {
        err = NANDSeek(&p_info->file, offset, NAND_SEEK_BEG);
    } else {
        err = NAND_RESULT_OK;
    }

    if (err >= 0) {
        if (offset + size > fileSize) {
            size = fileSize - offset;
            done = 0;
            while (size >= sizeof(tmp)) {
                err = NANDRead(&p_info->file, tmp, sizeof(tmp));
                if (err < 0) {
                    return FA_NANDDRV_ERR_NOT_FOUND;
                }
                pf_memcpy(buf + done, tmp, err);
                done += err;
                size -= err;
            }
            if (size != 0) {
                err = NANDRead(&p_info->file, tmp, size);
                if (err < 0) {
                    return FA_NANDDRV_ERR_NOT_FOUND;
                }
                pf_memcpy(buf + done, tmp, err);
            }
            return FA_NANDDRV_ERR_NOT_FOUND;
        }

        done = 0;
        while (size >= sizeof(tmp)) {
            err = NANDRead(&p_info->file, tmp, sizeof(tmp));
            if (err < 0) {
                fa_nanddrv_SetLastError(p_disk, err);
                return err;
            }
            pf_memcpy(buf + done, tmp, err);
            done += err;
            size -= err;
        }
        if (size != 0) {
            err = NANDRead(&p_info->file, tmp, size);
            if (err < 0) {
                fa_nanddrv_SetLastError(p_disk, err);
                return err;
            }
            pf_memcpy(buf + done, tmp, err);
        }
        *p_num_success = num_blocks;
        return 0;
    }

    fa_nanddrv_SetLastError(p_disk, err);
    return err;
}

s32 fa_nanddrv_physical_write(u32 num_blocks, const u8* buf, u32 block, u32 bps, u32* p_num_success, PDM_DISK* p_disk) {
    s32 err;
    u32 fileSize;
    u32 offset;
    u32 size;
    FA_NANDDISK_INFO* p_info;
    u32 done;
    u32 pos;
    u8 tmp[FA_NANDDRV_IO_BUF_SIZE] ATTRIBUTE_ALIGN(32);

    p_info = NULL;
    if (fa_nanddrv_GetDiskInfo(p_disk, &p_info) != 0) {
        return FA_NANDDRV_ERR_INVALID_PARAM;
    }

    size = num_blocks * bps;
    fileSize = p_info->size;
    offset = block * bps;
    if (NANDTell(&p_info->file, &pos) != NAND_RESULT_OK || pos != offset) {
        err = NANDSeek(&p_info->file, offset, NAND_SEEK_BEG);
    } else {
        err = NAND_RESULT_OK;
    }
    if (err >= 0) {
        if (offset + size > fileSize) {
            size = fileSize - offset;
            done = 0;
            while (size >= sizeof(tmp)) {
                pf_memcpy(tmp, buf + done, sizeof(tmp));
                err = NANDWrite(&p_info->file, tmp, sizeof(tmp));
                if (err < 0) {
                    return FA_NANDDRV_ERR_NOT_FOUND;
                }
                size -= err;
                done += err;
            }
            if (size != 0) {
                pf_memcpy(tmp, buf + done, size);
                NANDWrite(&p_info->file, tmp, size);
            }
            return FA_NANDDRV_ERR_NOT_FOUND;
        }

        done = 0;
        while (size >= sizeof(tmp)) {
            pf_memcpy(tmp, buf + done, sizeof(tmp));
            err = NANDWrite(&p_info->file, tmp, sizeof(tmp));
            if (err < 0) {
                fa_nanddrv_SetLastError(p_disk, err);
                return err;
            }
            size -= err;
            done += err;
        }
        if (size != 0) {
            pf_memcpy(tmp, buf + done, size);
            err = NANDWrite(&p_info->file, tmp, size);
            if (err < 0) {
                fa_nanddrv_SetLastError(p_disk, err);
                return err;
            }
        }
        *p_num_success = num_blocks;
        return 0;
    }

    fa_nanddrv_SetLastError(p_disk, err);
    return err;
}
