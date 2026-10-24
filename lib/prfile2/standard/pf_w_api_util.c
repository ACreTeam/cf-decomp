#include <prfile2/pf_api.h>

s32 PFWAPI_ParseOpenModeString(const u16* mode_str) {
    s32 open_mode;
    u32 i;

    if (mode_str == NULL) {
        return PF_IERR_INVALID_PARAM;
    }

    i = 1;

    switch (mode_str[0]) {
        case 'r':
            open_mode = PF_OPEN_MODE_READ;
            break;
        case 'w':
            open_mode = PF_OPEN_MODE_WRITE;
            break;
        case 'a':
            open_mode = PF_OPEN_MODE_APPEND;
            break;
        default:
            return 0;
    }

    if (mode_str[1] == 'b') {
        i = 2;
    }

    switch (mode_str[i++]) {
        case '\0':
            return open_mode;
        case '+':
            open_mode |= PF_OPEN_MODE_PLUS;
            break;
        case 't':
        default:
            return 0;
    }

    switch (mode_str[i]) {
        case '\0':
            return open_mode;
    }

    return 0;
}
