#include <prfile2/pdm_api.h>

s32 pdm_init_diskmanager(u32 config, void* param) {
    return pdm_disk_init_disk_manager(config, param);
}

s32 pdm_open_disk(PDM_INIT_DISK* p_init_disk_tbl, PDM_DISK** pp_disk) {
    s32 err;

    *pp_disk = NULL;
    err = pdm_disk_add_disk(p_init_disk_tbl, pp_disk);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_close_disk(PDM_DISK* p_disk) {
    return pdm_disk_del_disk(p_disk);
}

s32 pdm_open_partition(PDM_DISK* p_disk, s32 part_id, PDM_PARTITION** pp_part) {
    s32 err;

    *pp_part = NULL;
    err = pdm_part_get_partition(p_disk, part_id, pp_part);
    if (err != 0) {
        return err;
    }
    return 0;
}

s32 pdm_close_partition(PDM_PARTITION* p_part) {
    s32 err;

    err = pdm_part_release_partition(p_part);
    if (err != 0) {
        return err;
    }
    return 0;
}
