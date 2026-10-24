#include <prfile2/pf_standard.h>

u32 pf2_fappend(struct PF_FILE* p_file, u32 size) {
    u32 appended;

    PFFILE_fappend(p_file, size, &appended);
    return appended;
}
