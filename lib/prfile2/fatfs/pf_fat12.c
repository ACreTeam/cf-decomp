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

s32 PFFAT12_ReadFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value) {
    u16 fat_offset;
    u16 fat_sector;
    s32 err;
    u16 offset_in_sector;
    u16 word;
    u8 buf[2];
    u32 current_fat;
    s32 result;

    fat_offset = (u16)(cluster + (cluster >> 1));
    fat_sector = (u16)(p_vol->bpb.active_FAT_sector + (fat_offset >> p_vol->bpb.log2_bytes_per_sector));
    offset_in_sector = (u16)(fat_offset & (p_vol->bpb.bytes_per_sector - 1));

    if ((p_vol->bpb.ext_flags & PF_EXT_FLAG_NO_MIRRORING) == 0) {
        current_fat = 1;
    } else {
        current_fat = p_vol->bpb.ext_flags & PF_EXT_FLAG_ACTIVE_FAT_MASK;
    }

    do {
        if (offset_in_sector < p_vol->bpb.bytes_per_sector - 1) {
            err = PFSEC_ReadFAT(p_vol, buf, fat_sector, offset_in_sector, sizeof(u16));
        } else {
            err = PFSEC_ReadFAT(p_vol, buf, fat_sector, offset_in_sector, 1);
            if (err == 0) {
                err = PFSEC_ReadFAT(p_vol, &buf[1], fat_sector + 1, 0, 1);
            }
        }

        if (err == PF_IERR_DRIVER && p_vol->p_callback != NULL) {
            result = p_vol->p_callback(p_vol->last_driver_error);

            if (result == PF_FAT_READ_RETRY) {
                continue;
            }

            if (result == PF_FAT_READ_NEXT_FAT && p_vol->bpb.num_active_FATs >= 2 && current_fat < p_vol->bpb.num_active_FATs) {
                current_fat++;
                fat_sector += (u16)p_vol->bpb.sectors_per_FAT;
                continue;
            }
        }

        if (err != 0) {
            *p_value = PF_INVALID_CLUSTER;
            return err;
        }
    } while (err != 0);

    word = SWAP16(*(u16*)buf);
    if ((cluster & 1) != 0) {
        *p_value = word >> 4;
    } else {
        *p_value = word & PF_FAT12_ENTRY_MASK;
    }

    return 0;
}

s32 PFFAT12_WriteFATEntry(struct PF_VOLUME* p_vol, u16 cluster, u16 value) {
    s32 err;
    u16 fat_offset;
    u16 fat_sector;
    u16 offset_in_sector;
    u16 fat_sector2;
    u16 offset_in_sector2;
    u16 word;
    u16 i;
    s32 first_err;
    u8 buf[2];

    fat_offset = (u16)(cluster + (cluster >> 1));
    fat_sector = (u16)(p_vol->bpb.active_FAT_sector + (fat_offset >> p_vol->bpb.log2_bytes_per_sector));
    offset_in_sector = (u16)(fat_offset & (p_vol->bpb.bytes_per_sector - 1));

    if ((cluster & 1) != 0) {
        err = PFSEC_ReadFAT(p_vol, buf, fat_sector, offset_in_sector, 1);
        if (err != 0) {
            return err;
        }
        word = (value << 4) + (buf[0] & 0xF);
    } else {
        fat_sector2 = (u16)(p_vol->bpb.active_FAT_sector + ((fat_offset + 1) >> p_vol->bpb.log2_bytes_per_sector));
        offset_in_sector2 = (u16)((fat_offset + 1) & (p_vol->bpb.bytes_per_sector - 1));
        err = PFSEC_ReadFAT(p_vol, buf, fat_sector2, offset_in_sector2, 1);
        if (err != 0) {
            return err;
        }
        word = ((buf[0] << 8) & 0xF000) + (value & PF_FAT12_ENTRY_MASK);
    }

    *(u16*)buf = SWAP16(word);

    first_err = 0;
    for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
        if (offset_in_sector < p_vol->bpb.bytes_per_sector - 1) {
            err = PFSEC_WriteFAT(p_vol, buf, fat_sector, offset_in_sector, sizeof(u16));
            if (err != 0 && first_err == 0) {
                first_err = err;
            }
        } else {
            err = PFSEC_WriteFAT(p_vol, buf, fat_sector, offset_in_sector, 1);
            if (err != 0 && first_err == 0) {
                first_err = err;
            }
            err = PFSEC_WriteFAT(p_vol, &buf[1], fat_sector + 1, 0, 1);
            if (err != 0 && first_err == 0) {
                first_err = err;
            }
        }
        fat_sector += p_vol->bpb.sectors_per_FAT;
    }
    return first_err;
}

s32 PFFAT12_WriteFATEntryWithBuf(struct PF_VOLUME* p_vol, u16 cluster, u16 value, struct PF_CACHE_PAGE* p_page) {
    s32 err;
    u16 fat_offset;
    u16 fat_sector;
    s32 first_err;
    u16 offset_in_sector;
    u16 i;

    fat_offset = (u16)(cluster + (cluster >> 1));
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
        err = PFSEC_ReadFAT(p_vol, p_page->p_buf, fat_sector, 0, p_vol->bpb.bytes_per_sector);
        if (err != 0) {
            return err;
        }
        p_page->sector = fat_sector;
    }

    if ((cluster & 1) != 0) {
        p_page->p_buf[offset_in_sector] = (p_page->p_buf[offset_in_sector] & 0xF) | (u8)(value << 4);
        if (offset_in_sector == p_vol->bpb.bytes_per_sector - 1u) {
            for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
                err = PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
                if (err != 0 && first_err == 0) {
                    first_err = err;
                }
            }
            err = PFSEC_ReadFAT(p_vol, p_page->p_buf, fat_sector + 1, 0, p_vol->bpb.bytes_per_sector);
            if (err != 0) {
                return err;
            }
            p_page->sector = fat_sector + 1;
            p_page->p_buf[0] = (u8)(value >> 4);
        } else {
            p_page->p_buf[offset_in_sector + 1] = (u8)(value >> 4);
        }
    } else {
        p_page->p_buf[offset_in_sector] = (u8)value;
        if (offset_in_sector == p_vol->bpb.bytes_per_sector - 1u) {
            for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
                err = PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
                if (err != 0 && first_err == 0) {
                    first_err = err;
                }
            }
            err = PFSEC_ReadFAT(p_vol, p_page->p_buf, fat_sector + 1, 0, p_vol->bpb.bytes_per_sector);
            if (err != 0) {
                return err;
            }
            p_page->sector = fat_sector + 1;
            p_page->p_buf[0] = (p_page->p_buf[0] & 0xF0) | (u8)(value >> 8);
        } else {
            p_page->p_buf[offset_in_sector + 1] = (p_page->p_buf[offset_in_sector + 1] & 0xF0) | (u8)(value >> 8);
        }
    }

    if (err != 0) {
        first_err = err;
    }
    return first_err;
}

s32 PFFAT12_ReadFATEntryWithBuf(struct PF_VOLUME* p_vol, u16 cluster, u32* p_value, struct PF_CACHE_PAGE* p_page) {
    u32 offset;
    u32 sector;
    u32 current_fat;
    s32 err;
    s32 result;
    u16 i;

    offset = (u16)(cluster + (cluster >> 1));
    sector = (u16)(p_vol->bpb.active_FAT_sector + (offset >> p_vol->bpb.log2_bytes_per_sector));

    if ((p_vol->bpb.ext_flags & PF_EXT_FLAG_NO_MIRRORING) == 0) {
        current_fat = 1;
    } else {
        current_fat = p_vol->bpb.ext_flags & PF_EXT_FLAG_ACTIVE_FAT_MASK;
    }

    err = 0;
    while (1) {
        if (err == PF_IERR_DRIVER && p_vol->p_callback != NULL) {
            result = p_vol->p_callback(p_vol->last_driver_error);
            if (result == PF_FAT_READ_RETRY) {
                err = 0;
                continue;
            }
            if (result != PF_FAT_READ_NEXT_FAT || p_vol->bpb.num_active_FATs < 2 || current_fat >= p_vol->bpb.num_active_FATs) {
                break;
            }
            err = 0;
            current_fat++;
            sector += p_vol->bpb.sectors_per_FAT;
            continue;
        }

        if (p_page->sector != sector) {
            if (p_page->option == PF_CACHE_PAGE_OPT_FAT_MODIFIED) {
                for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
                    PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
                }
            }
            err = PFSEC_ReadFAT(p_vol, p_page->p_buf, sector, 0, p_vol->bpb.bytes_per_sector);
            if (err != 0) {
                continue;
            }
            p_page->sector = sector;
        }

        offset &= (p_vol->bpb.bytes_per_sector - 1);
        if (offset == p_vol->bpb.bytes_per_sector - 1) {
            *p_value = p_page->p_buf[offset];
            if (p_page->option == PF_CACHE_PAGE_OPT_FAT_MODIFIED) {
                for (i = 0; i < p_vol->bpb.num_active_FATs; i++) {
                    PFSEC_WriteFAT(p_vol, p_page->p_buf, p_page->sector + i * p_vol->bpb.sectors_per_FAT, 0, p_vol->bpb.bytes_per_sector);
                }
            }
            err = PFSEC_ReadFAT(p_vol, p_page->p_buf, sector + 1, 0, p_vol->bpb.bytes_per_sector);
            if (err != 0) {
                continue;
            }
            p_page->sector = sector + 1;
            *p_value += p_page->p_buf[0] << 8;
        } else {
            *p_value = (p_page->p_buf[offset + 1] << 8) + p_page->p_buf[offset];
        }
        break;
    }

    if (err != 0) {
        *p_value = PF_INVALID_CLUSTER;
        return err;
    }
    if ((cluster & 1) != 0) {
        *p_value >>= 4;
    } else {
        *p_value &= PF_FAT12_ENTRY_MASK;
    }
    return 0;
}
