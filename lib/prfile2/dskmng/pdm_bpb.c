#include <prfile2/pdm_bpb.h>
#include <prfile2/pdm_partition.h>

void pdm_bpb_CalcBPBFields(PF_BPB* p_bpb) {
    u16 val;
    u32 num_data_sectors;

    p_bpb->log2_bytes_per_sector = 0;
    val = p_bpb->bytes_per_sector;
    while ((val >>= 1) != 0) {
        p_bpb->log2_bytes_per_sector++;
    }

    p_bpb->log2_sectors_per_cluster = 0;
    val = p_bpb->sectors_per_cluster;
    while ((val >>= 1) != 0) {
        p_bpb->log2_sectors_per_cluster++;
    }

    p_bpb->num_root_dir_sectors = ((p_bpb->bytes_per_sector + (p_bpb->num_root_dir_entries * PDM_DIR_ENTRY_SIZE)) - 1) >> p_bpb->log2_bytes_per_sector;
    p_bpb->first_data_sector = p_bpb->num_FATs * p_bpb->sectors_per_FAT + (p_bpb->num_root_dir_sectors + p_bpb->num_reserved_sectors);
    num_data_sectors = p_bpb->total_sectors - p_bpb->first_data_sector;
    p_bpb->num_clusters = num_data_sectors >> p_bpb->log2_sectors_per_cluster;

    if (p_bpb->num_clusters < PDM_FAT12_MAX_CLUSTERS) {
        p_bpb->fat_type = FAT_12;
        p_bpb->first_root_dir_sector = p_bpb->num_reserved_sectors + p_bpb->num_FATs * p_bpb->sectors_per_FAT;
    } else if (p_bpb->num_clusters < PDM_FAT16_MAX_CLUSTERS) {
        p_bpb->fat_type = FAT_16;
        p_bpb->first_root_dir_sector = p_bpb->num_reserved_sectors + p_bpb->num_FATs * p_bpb->sectors_per_FAT;
    } else {
        p_bpb->fat_type = FAT_32;
        p_bpb->first_root_dir_sector = 0;
    }
}

s32 pdm_bpb_InitVolumeWithBootSector(PF_BPB* p_bpb, u8* buf) {
    u16 total_sectors16;
    s32 err;
    u16 sectors_per_FAT16;
    u16 fs_version;

    err = 0;
    if (!pdm_part_is_boot_sector(buf)) {
        err = PDM_ERR_INVALID_BOOT_SECTOR;
    } else {
        p_bpb->bytes_per_sector = PDM_LE16(buf, PDM_BPB_BYTES_PER_SECTOR);
        p_bpb->sectors_per_cluster = buf[PDM_BPB_SECTORS_PER_CLUSTER];
        p_bpb->num_reserved_sectors = PDM_LE16(buf, PDM_BPB_NUM_RESERVED_SECTORS);
        p_bpb->num_FATs = buf[PDM_BPB_NUM_FATS];
        p_bpb->num_root_dir_entries = PDM_LE16(buf, PDM_BPB_NUM_ROOT_DIR_ENTRIES);
        p_bpb->media = buf[PDM_BPB_MEDIA];

        total_sectors16 = PDM_LE16(buf, PDM_BPB_TOTAL_SECTORS16);
        p_bpb->total_sectors = (total_sectors16 == 0) ? PDM_LE32(buf + PDM_BPB_TOTAL_SECTORS32) : total_sectors16;

        sectors_per_FAT16 = PDM_LE16(buf, PDM_BPB_SECTORS_PER_FAT16);
        p_bpb->sectors_per_FAT = (sectors_per_FAT16 == 0) ? PDM_LE32(buf + PDM_BPB_SECTORS_PER_FAT32) : sectors_per_FAT16;

        pdm_bpb_CalcBPBFields(p_bpb);

        switch (p_bpb->fat_type) {
            case FAT_12:
            case FAT_16:
                if (sectors_per_FAT16 == 0) {
                    err = PDM_ERR_INVALID_BPB;
                }
                break;
            case FAT_32:
                if (total_sectors16 != 0 || sectors_per_FAT16 != 0) {
                    err = PDM_ERR_INVALID_BPB;
                }
                fs_version = PDM_LE16(buf, PDM_BPB_FS_VERSION);
                p_bpb->ext_flags = PDM_LE16(buf, PDM_BPB_EXT_FLAGS);
                p_bpb->root_dir_cluster = PDM_LE32(buf + PDM_BPB_ROOT_DIR_CLUSTER);
                p_bpb->fs_info_sector = PDM_LE16(buf, PDM_BPB_FS_INFO_SECTOR);
                p_bpb->backup_boot_sector = PDM_LE16(buf, PDM_BPB_BACKUP_BOOT_SECTOR);
                p_bpb->first_root_dir_sector = p_bpb->first_data_sector + ((p_bpb->root_dir_cluster - PF_CLUSTER_FIRST) << p_bpb->log2_sectors_per_cluster);
                if (fs_version != 0) {
                    err = PDM_ERR_INVALID_BPB;
                }
                break;
            default:
                err = PDM_ERR_INVALID_BPB;
                break;
        }

        if (p_bpb->ext_flags & PDM_BPB_EXT_FLAG_NO_MIRRORING) {
            p_bpb->num_active_FATs = 1;
            p_bpb->active_FAT_sector = p_bpb->num_reserved_sectors + (p_bpb->ext_flags & PDM_BPB_EXT_FLAG_ACTIVE_FAT_MASK) * p_bpb->sectors_per_FAT;
        } else {
            p_bpb->num_active_FATs = p_bpb->num_FATs;
            p_bpb->active_FAT_sector = p_bpb->num_reserved_sectors;
        }
    }

    return err;
}

s32 pdm_bpb_InitVolumeWithFSINFOSector(PF_BPB* p_bpb, u8* buf, u32* p_free_count, u32* p_next_free) {
    u32 free_count;
    u32 next_free;

    *p_free_count = PDM_FSI_UNKNOWN;
    *p_next_free = PDM_FSI_UNKNOWN;

    if (PDM_LE32(buf + PDM_FSI_LEAD_SIG) == PDM_FSI_LEAD_SIG_VALUE && PDM_LE32(buf + PDM_FSI_STRUC_SIG) == PDM_FSI_STRUC_SIG_VALUE &&
        PDM_LE32(buf + PDM_FSI_TRAIL_SIG) == PDM_FSI_TRAIL_SIG_VALUE) {
        free_count = PDM_LE32(buf + PDM_FSI_FREE_COUNT);
        next_free = PDM_LE32(buf + PDM_FSI_NXT_FREE);
        if (free_count >= 2 && free_count < p_bpb->num_clusters + 2) {
            *p_free_count = free_count;
        }
        if (next_free >= PF_CLUSTER_FIRST && next_free < p_bpb->num_clusters + PF_CLUSTER_FIRST) {
            *p_next_free = next_free;
        }
    }

    return 0;
}
