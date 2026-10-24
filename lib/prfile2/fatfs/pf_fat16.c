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

static inline u32 PFFAT16_GetFATOffset(u32 cluster) {
    return cluster * sizeof(u16);
}

s32 PFFAT16_ReadFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value) {
    u32 fat_offset;
    u32 fat_sector;
    u16 offset_in_sector;
    u8 buf[2];
    s32 err;
    u32 current_fat;
    s32 result;

    fat_offset = PFFAT16_GetFATOffset(cluster);
    fat_sector = (u16)(p_vol->bpb.active_FAT_sector + (fat_offset >> p_vol->bpb.log2_bytes_per_sector));
    offset_in_sector = fat_offset & (p_vol->bpb.bytes_per_sector - 1);

    if ((p_vol->bpb.ext_flags & PF_EXT_FLAG_NO_MIRRORING) == 0) {
        current_fat = 1;
    } else {
        current_fat = p_vol->bpb.ext_flags & PF_EXT_FLAG_ACTIVE_FAT_MASK;
    }

    do {
        err = PFSEC_ReadFAT(p_vol, buf, fat_sector, offset_in_sector, sizeof(u16));

        if (err == PF_IERR_DRIVER && p_vol->p_callback != NULL) {
            result = p_vol->p_callback(p_vol->last_driver_error);
            if (result == PF_FAT_READ_RETRY) {
                continue;
            }
            if (result == PF_FAT_READ_NEXT_FAT && p_vol->bpb.num_active_FATs >= 2 && current_fat < p_vol->bpb.num_active_FATs) {
                current_fat++;
                fat_sector += p_vol->bpb.sectors_per_FAT;
                continue;
            }
        }

        if (err != 0) {
            *p_value = PF_INVALID_CLUSTER;
            return err;
        }
    } while (err != 0);

    *p_value = SWAP16(*(u16*)buf);
    return 0;
}

s32 PFFAT16_WriteFATEntry(struct PF_VOLUME* p_vol, u32 cluster, u32 value) {
    s32 err;
    u32 fat_offset;
    u16 fat_sector;
    u16 offset_in_sector;
    u32 i;
    s32 first_err;
    u8 buf[2];

    fat_offset = PFFAT16_GetFATOffset(cluster);
    fat_sector = (u16)(p_vol->bpb.active_FAT_sector + (fat_offset >> p_vol->bpb.log2_bytes_per_sector));
    offset_in_sector = fat_offset & (p_vol->bpb.bytes_per_sector - 1);

    *(u16*)buf = SWAP16(value);

    first_err = 0;
    for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
        err = PFSEC_WriteFAT(p_vol, buf, fat_sector, offset_in_sector, sizeof(u16));
        if (err != 0 && first_err == 0) {
            first_err = err;
        }
        fat_sector += p_vol->bpb.sectors_per_FAT;
    }
    return first_err;
}

s32 PFFAT16_WriteFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32 value, struct PF_CACHE_PAGE* p_page) {
    s32 err;
    u32 fat_offset;
    u16 fat_sector;
    u16 offset_in_sector;
    u32 i;
    s32 first_err;

    fat_offset = PFFAT16_GetFATOffset(cluster);
    fat_sector = (u16)(p_vol->bpb.active_FAT_sector + (fat_offset >> p_vol->bpb.log2_bytes_per_sector));
    offset_in_sector = fat_offset & (p_vol->bpb.bytes_per_sector - 1);

    err = 0;
    first_err = 0;
    if (p_page->sector != fat_sector) {
        for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
            err = PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
            if (err != 0 && first_err == 0) {
                first_err = err;
            }
        }
        p_page->sector = fat_sector;
        err = PFSEC_ReadFAT(p_vol, p_page->p_buf, fat_sector, 0, p_vol->bpb.bytes_per_sector);
    }

    p_page->p_buf[offset_in_sector] = (u8)value;
    p_page->p_buf[offset_in_sector + 1] = (u8)(value >> 8);
    if (err != 0) {
        first_err = err;
    }
    return first_err;
}

s32 PFFAT16_ReadFATEntryWithBuf(struct PF_VOLUME* p_vol, u32 cluster, u32* p_value, struct PF_CACHE_PAGE* p_page) {
    u32 offset;
    u32 sector;
    s32 err;
    u32 current_fat;
    s32 result;
    u16 i;

    offset = PFFAT16_GetFATOffset(cluster);
    sector = (u16)(p_vol->bpb.active_FAT_sector + (offset >> p_vol->bpb.log2_bytes_per_sector));

    if ((p_vol->bpb.ext_flags & PF_EXT_FLAG_NO_MIRRORING) == 0) {
        current_fat = 1;
    } else {
        current_fat = p_vol->bpb.ext_flags & PF_EXT_FLAG_ACTIVE_FAT_MASK;
    }

    if (p_page->sector != sector) {
        if (p_page->option == PF_CACHE_PAGE_OPT_FAT_MODIFIED) {
            for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
                PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
            }
        }

        do {
            err = PFSEC_ReadFAT(p_vol, p_page->p_buf, sector, 0, p_vol->bpb.bytes_per_sector);
            if (err == PF_IERR_DRIVER && p_vol->p_callback != NULL) {
                result = p_vol->p_callback(p_vol->last_driver_error);
                if (result == PF_FAT_READ_RETRY) {
                    continue;
                }
                if (result == PF_FAT_READ_NEXT_FAT && p_vol->bpb.num_active_FATs >= 2 && current_fat < p_vol->bpb.num_active_FATs) {
                    current_fat++;
                    sector += p_vol->bpb.sectors_per_FAT;
                    continue;
                }
            }

            if (err != 0) {
                return err;
            }
            p_page->sector = sector;
        } while (err != 0);
    }

    offset &= (p_vol->bpb.bytes_per_sector - 1);
    *p_value = SWAP16(*(u16*)(p_page->p_buf + offset));
    return 0;
}
