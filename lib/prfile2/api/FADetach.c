#include <prfile2/fa.h>

s32 FADetach(s8 drive) {
    PDM_DISK* p_disk = gOpenDisk[drive - 'A'];
    PDM_PARTITION* p_part = gOpenPartition[drive - 'A'];

    if (pfstub_detach(drive) != 0) {
        return FA_RET_ERR;
    }
    if (p_disk != NULL && p_part != NULL) {
        if (pdm_close_partition(p_part) != 0) {
            return FA_RET_ERR;
        }
        if (pdm_close_disk(p_disk) != 0) {
            return FA_RET_ERR;
        }
    }
    return 0;
}
