#include <prfile2/fa.h>
#include <prfile2/pf_types.h>

static BOOL s_diskmanager_initialized;
static BOOL s_initializing;
static BOOL s_prfile2_initialized;

s32 FAInit(u32 config) {
    s32 err;

    if (s_initializing == TRUE) {
        return FA_RET_ERR;
    }
    s_initializing = TRUE;
    if (!s_diskmanager_initialized) {
        err = pdm_init_diskmanager(0, NULL);
        s_diskmanager_initialized = (err == 0);
        if (err != 0) {
            s_initializing = FALSE;
            return FA_RET_ERR;
        }
    }
    if (!s_prfile2_initialized) {
        err = pfstub_init_prfile2(config, NULL);
        s_prfile2_initialized = (err == 0);
        if (err == PF_RET_ERR) {
            s_initializing = FALSE;
            return FA_RET_ERR;
        }
    }
    s_initializing = FALSE;
    return 0;
}
