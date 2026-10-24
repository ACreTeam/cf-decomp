#include <prfile2/pf_standard.h>

s32 pf2_telldir(struct PF_DIR* p_dir, u32* p_offset) {
    s32 err;

    err = PFDIR_telldir(p_dir, p_offset);
    err = PFAPI_convertReturnValue(err);
    return err;
}
