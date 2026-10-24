#include <prfile2/pfd_drv.h>
#include <prfile2/pf_api.h>

u8 pfd_get_media_drv_char(PDM_DISK* p_disk, s8* p_drv_char, u8 num) {
    u32 i;
    u8 j;
    PF_VOLUME* p_vol;
    u8 cnt;
    s8* p_drv;
    PDM_PARTITION* p_part;

    if (p_drv_char == NULL || num == 0) {
        return 0;
    }
    p_drv = p_drv_char;
    cnt = 0;
    p_part = pdm_disk_set.partition;
    for (i = 0; i < PDM_DRIVE_COUNT; i++, p_part++) {
        if (pdm_part_is_attached_partition(p_part) && p_part->p_disk == p_disk) {
            p_vol = pf_vol_set.volumes;
            for (j = 0; j < PF_DRIVE_COUNT; j++, p_vol++) {
                if (p_vol->p_part == p_part) {
                    cnt++;
                    *p_drv++ = p_vol->drv_char;
                    if (cnt == num) {
                        break;
                    }
                }
            }
        }
        if (cnt == num) {
            break;
        }
    }
    return cnt;
}
