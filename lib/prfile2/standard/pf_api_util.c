#include <prfile2/pf_api.h>

s32 pf_error_to_api_error[40] = {
    0, PF_ERR_EINVAL, PF_ERR_EINVAL, PF_ERR_ENOENT,
    PF_ERR_EBUSY, PF_ERR_ENOTEMPTY, PF_ERR_ENOSPC, PF_ERR_ENOEXEC,
    PF_ERR_EEXIST, PF_ERR_ENOEXEC, PF_ERR_EINVAL, PF_ERR_EWRTPROTECT,
    PF_ERR_ENOSYS, PF_ERR_ENOEXEC, PF_ERR_ENOEXEC, PF_ERR_ENOEXEC,
    PF_ERR_EACCES, PF_ERR_EIO, PF_ERR_ENOEXEC, PF_ERR_EACCES,
    PF_ERR_ENOENT, PF_ERR_ENFILE, PF_ERR_EMFILE, PF_ERR_EISDIR,
    PF_ERR_EACCES, PF_ERR_EPERM, PF_ERR_ENOEXEC, PF_ERR_ENOEXEC,
    PF_ERR_ENOEXEC, PF_ERR_ENOEXEC, PF_ERR_ENOMEM, PF_ERR_EINVAL,
    PF_ERR_EINVAL, PF_ERR_ENOEXEC, PF_ERR_ENOENT, PF_ERR_ENOEXEC,
    PF_ERR_ENOEXEC, PF_ERR_EFBIG, PF_ERR_EBADF, PF_ERR_ENOLCK,
};

s32 PFAPI_ParseOpenModeString(const s8* mode_str) {
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

s32 PFAPI_convertError(s32 err) {
    if (err == 0) {
        return 0;
    }
    if (err == PF_IERR_SYSTEM) {
        return PF_ERR_SYSTEM;
    }
    if (err > 0 && err < (s32)sizeof(pf_error_to_api_error)) {
        return pf_error_to_api_error[err];
    }
    if (err == PF_IERR_DRIVER) {
        return PF_ERR_EIO;
    }
    return err;
}

s32 PFAPI_convertDriverError(s32 err) {
    if (err == 0) {
        return 0;
    }
    if (err > 0 && err < sizeof(pf_error_to_api_error)) {
        return 1;
    }
    return err;
}

s32 PFAPI_convertReturnValue(s32 err) {
    if (err == 0) {
        return PF_RET_NO_ERR;
    }
    return PF_RET_ERR;
}

s32 PFAPI_convertReturnValue4feof(s32 err) {
    if (err == TRUE) {
        return PF_RET_EOF;
    }
    if (err == 0) {
        return PF_RET_NO_ERR;
    }
    return PF_RET_ERR;
}

void* PFAPI_convertReturnValue2NULL(s32 err, void* p_stream) {
    if (err != 0) {
        return NULL;
    }
    return p_stream;
}

s32 PFAPI_convertReturnValue4unmount(s32 err) {
    if (err == 0) {
        return PF_RET_NO_ERR;
    }
    if (err == PF_RET_UNMOUNT_FORCED) {
        return PF_RET_UNMOUNT_FORCED;
    }
    return PF_RET_ERR;
}
