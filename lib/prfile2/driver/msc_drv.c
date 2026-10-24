#include <prfile2/pfd_drv.h>

static PFD_MEDIA_CALLBACK s_eject_callback;
static PFD_MEDIA_CALLBACK s_insert_callback;

s32 pfd_mscdrv_registar_callback(PFD_MEDIA_CALLBACK insert, PFD_MEDIA_CALLBACK eject) {
    s_insert_callback = insert;
    s_eject_callback = eject;
    return 0;
}

BOOL pfd_mscdrv_is_media_insert(PDM_DISK* p_disk) {
    return uhf_msc_is_media_insert(p_disk);
}
