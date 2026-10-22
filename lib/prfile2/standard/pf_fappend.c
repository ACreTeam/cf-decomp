#include <prfile2/pf_api.h>

u32 pf2_fappend(struct PF_FILE* p_file, u32 size) {
    u32 result;

    PFFILE_fappend(p_file, size, &result);
    return result;
}
