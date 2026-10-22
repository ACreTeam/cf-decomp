#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/os.h>

#include <private/fs.h>

#include <private/ipc.h>

#include <stdio.h>

static void  nandOpenCallback(ISFSError, void *);
static void nandReadCloseCallback(ISFSError, void *);
static void nandCloseCallback(ISFSError, void *);
static void nandSafeCloseCallback(ISFSError, void *);
static void nandSafeOpenCallback(const ISFSError, void *);
void nandReadOpenCallback(ISFSError, void *);
s32 nandSafeCloseAsync(NANDFileInfo *, NANDCallback, NANDCommandBlock *, BOOL);
s32 nandSafeOpenAsync(const char *, NANDFileInfo *, const u8, void *, const u32, NANDCallback, NANDCommandBlock *, BOOL, BOOL);
s32 nandSafeOpen(const char* path, NANDFileInfo* info, const u8 accType, void* buf, const u32 length, BOOL privilege_flag, BOOL simple_flag);
s32 nandSafeClose(NANDFileInfo* info, BOOL simple_flag);
static ISFSError nandCopy(s32 fd, s32 origFd, void* buf, const u32 length);
static u32 nandGetUniqueNumber(void);

IOSFd nandOpen(const char* path, const u8 accType, NANDCommandBlock* block, const BOOL async_flag, const BOOL privilege_flag) {
    IOSFd fd = ISFS_ERROR_UNKNOWN;
    char absPath[64] = "";
    u32 access = 0;


    nandGenerateAbsPath(absPath, path);

    if (!privilege_flag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    } else {
        switch (accType) {
        case 3:
            access = 3;
            break;
        case 1:
            access = 1;
            break;
        case 2:
            access = 2;
            break;
        default:
            break;
        }

        if (async_flag) {
            fd = ISFS_OpenAsync((const char*)absPath, access, nandOpenCallback, block);
        } else {
            fd = ISFS_Open((const char*)absPath, access);
        }

        return fd;
    }
}

s32 NANDOpen(const char* path, NANDFileInfo* info, const u8 accType) {
    IOSFd fd = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    fd = nandOpen(path, accType, NULL, FALSE, FALSE);

    if (fd >= 0) {
        info->fileDescriptor = fd;
        info->mark = 1;
        return NAND_RESULT_OK;
    } else {
        return nandConvertErrorCode(fd);
    }
}

s32 NANDPrivateOpen(const char* path, NANDFileInfo* info, const u8 accType) {
    IOSFd fd = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    fd = nandOpen(path, accType, NULL, FALSE, TRUE);

    if (fd >= 0) {
        info->fileDescriptor = fd;
        info->mark = 1;
        return NAND_RESULT_OK;
    } else {
        return nandConvertErrorCode(fd);
    }
}

s32 NANDOpenAsync(const char* path, NANDFileInfo* info, const u8 accType, NANDCallback cb, NANDCommandBlock* block) {
    IOSFd fd = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = cb;
    block->fileInfo = info;
    fd = nandOpen(path, accType, block, TRUE, FALSE);
    return nandConvertErrorCode(fd);
}

s32 NANDPrivateOpenAsync(const char* path, NANDFileInfo* info, const u8 accType, NANDCallback cb, NANDCommandBlock* block) {
    IOSFd fd = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = cb;
    block->fileInfo = info;
    fd = nandOpen(path, accType, block, TRUE, TRUE);
    return nandConvertErrorCode(fd);
}

void nandOpenCallback(ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result >= 0) {
        ((NANDFileInfo*)(b->fileInfo))->fileDescriptor = result;
        ((NANDFileInfo*)(b->fileInfo))->stage = 2;
        ((NANDFileInfo*)(b->fileInfo))->mark = 1;
        ((NANDCallback)(b->callback))(NAND_RESULT_OK, b);
    } else {
        ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
    }
}

s32 NANDClose(NANDFileInfo* info) {
    ISFSError err = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->mark != 1) {
        return NAND_RESULT_INVALID;
    }

    err = ISFS_Close(info->fileDescriptor);

    if (err == ISFS_ERROR_OK) {
        info->mark = 2;
    }

    return nandConvertErrorCode(err);
}

s32 NANDCloseAsync(NANDFileInfo* info, NANDCallback cb, NANDCommandBlock* block) {
    ISFSError err = ISFS_ERROR_UNKNOWN;


    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->mark != 1) {
        return NAND_RESULT_INVALID;
    }

    block->callback = cb;
    block->fileInfo = info;
    err = ISFS_CloseAsync(info->fileDescriptor, nandCloseCallback, block);
    return nandConvertErrorCode(err);
}

s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, const u8 accType, void* buf, const u32 length) {
    return nandSafeOpen(path, info, accType, buf, length, FALSE, TRUE);
}

s32 nandSafeOpen(const char* path, NANDFileInfo* info, const u8 accType, void* buf, const u32 length, BOOL privilege_flag, BOOL simple_flag) {
    
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (simple_flag && ((length & 0x3FFF) != 0)) {
        return NAND_RESULT_INVALID;
    }
    
    info->accType = accType;
    info->stage = 0;
    nandGenerateAbsPath(info->origPath, path);

    if (!privilege_flag && nandIsPrivatePath(info->origPath)) {
        return NAND_RESULT_ACCESS;
    }

    if (accType == 1) {
        IOSFd fd = ISFS_Open((char*)(info->origPath), 1);
        if (fd >= ISFS_ERROR_OK) {
            info->fileDescriptor = fd;
            info->stage = 2;

            if (!simple_flag) {
                info->mark = 3;
            } else {
                info->mark = 5;
            }

            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(fd);
        }
    } else if (accType == 2 || accType == 3) {
        char dirName[64];
        char relName[13] = {0};
        ISFSError ret = ISFS_ERROR_UNKNOWN;
        u32 ownerId;
        u16 groupId;
        u32 attr;
        u32 ownerAcc;
        u32 groupAcc;
        u32 othersAcc;
        u32 sp48 = -1;

        ret = ISFS_CreateDir((char*)"/tmp/sys", 0, 3, 3, 3);
        if (ret != ISFS_ERROR_OK && ret != ISFS_ERROR_EXISTS) {
            return nandConvertErrorCode(ret);
        } else {
            info->stage = 1;
            ret = ISFS_GetAttr((const char*)info->origPath, &ownerId, &groupId, &attr, &ownerAcc, &groupAcc, &othersAcc);
            if (ret != ISFS_ERROR_OK) {
                return nandConvertErrorCode(ret);
            }

            info->origFd = ISFS_Open((char*)info->origPath, 1);
            if (info->origFd < 0) {
                return nandConvertErrorCode(info->origFd);
            }

            info->stage = 2;

            if (!simple_flag) {
                sp48 = nandGetUniqueNumber();
                sprintf(dirName, "%s/%08x", "/tmp/sys", sp48);
                ret = ISFS_CreateDir((const char*)dirName, 0, 3, 0, 0);
                if (ret != 0) {
                    return nandConvertErrorCode(ret);
                }

                info->stage = 3;
            }

            nandGetRelativeName(relName, info->origPath);

            if (!simple_flag) {
                sprintf(info->tmpPath, "%s/%08x/%s", "/tmp/sys", sp48, relName);
            } else {
                sprintf(info->tmpPath, "%s/%s", "/tmp/sys", relName);
            }

            ret = ISFS_CreateFile((const char*)info->tmpPath, attr, ownerAcc, groupAcc, othersAcc);
            if (ret != 0) {
                return nandConvertErrorCode(ret);
            }

            info->stage = 4;

            if (accType == 2) {
                info->fileDescriptor = ISFS_Open((const char*)info->tmpPath, 2);
            } else if (accType == 3) {
                info->fileDescriptor = ISFS_Open((const char*)info->tmpPath, 3);
            }

            if (info->fileDescriptor < 0) {
                return nandConvertErrorCode(info->fileDescriptor);
            }

            info->stage = 5;

            ret = nandCopy(info->fileDescriptor, info->origFd, buf, length);
            if (ret != 0) {
                return nandConvertErrorCode(ret);
            }

            ret = ISFS_Seek(info->fileDescriptor, 0, 0);
            if (ret == 0) {
                ret = 0;
            } else {
                return nandConvertErrorCode(ret);
            }

            if (ret == 0) {
                if (simple_flag) {
                    info->mark = 5;
                } else {
                    info->mark = 3;
                }
            }

            return nandConvertErrorCode(ret);
        }
    } else {
        return NAND_RESULT_INVALID;
    }
}

s32 NANDSimpleSafeClose(NANDFileInfo* info) {
    return nandSafeClose(info, TRUE);
}

s32 nandSafeClose(NANDFileInfo* info, BOOL simple_flag) {
    ISFSError err = ISFS_ERROR_UNKNOWN;
    char tmpdir[64] = "";
    
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }
    
    if (!((info->mark == 3 && !simple_flag) || (info->mark == 5 && simple_flag))) {
        return NAND_RESULT_INVALID;
    }

    if (info->accType == 1) {
        err = ISFS_Close(info->fileDescriptor);
        if (err == 0) {
            info->stage = 7;

            if (!simple_flag) {
                info->mark = 4;
            } else {
                info->mark = 6;
            }
        }

        return nandConvertErrorCode(err);
    } else if (info->accType == 2 || info->accType == 3) {
        err = ISFS_Close(info->fileDescriptor);
        if (err != 0) {
            return nandConvertErrorCode(err);
        }

        info->stage = 6;

        err = ISFS_Close(info->origFd);
        if (err != 0) {
            return nandConvertErrorCode(err);
        }

        info->stage = 7;

        err = ISFS_Rename((const char*)info->tmpPath, (const char*)info->origPath);
        if (err != 0) {
            return nandConvertErrorCode(err);
        }

        info->stage = 8;

        if (!simple_flag) {
            nandGetParentDirectory(tmpdir, info->tmpPath);
            err = ISFS_Delete((const char*)tmpdir);
            if (err == 0) {
                info->stage = 9;
                info->mark = 4;
            }
        } else {
            info->mark = 6;
        }

        return nandConvertErrorCode(err);
    } else {
        OSReport("Illegal NANDFileInfo.\n");
    }

    return NAND_RESULT_INVALID;
}

s32 NANDPrivateSafeOpenAsync(const char* path, NANDFileInfo* info, const u8 accType, void* buf, const u32 length, NANDCallback cb, NANDCommandBlock* block) {
    return nandSafeOpenAsync(path, info, accType, buf, length, cb, block, TRUE, FALSE);
}

s32 nandSafeOpenAsync(const char* path, NANDFileInfo* info, const u8 accType, void* buf, const u32 length, NANDCallback cb, NANDCommandBlock* block, BOOL privilege_flag, BOOL simple_flag) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (simple_flag && ((length & 0x3FFF) != 0)) {
        return NAND_RESULT_INVALID;
    }
    
    info->accType = accType;
    info->stage = 0;
    block->simpleFlag = simple_flag;
    nandGenerateAbsPath(info->origPath, path);

    if (!privilege_flag && nandIsPrivatePath(info->origPath)) {
        return NAND_RESULT_ACCESS;
    }

    if (accType == 1) {
        IOSFd fd = -1;
        block->fileInfo = info;
        block->callback = cb;
        fd = ISFS_OpenAsync((char*)(info->origPath), 1, nandReadOpenCallback, block);

        if (fd == ISFS_ERROR_OK) {
            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(fd);
        }
    } else if (accType == 2 || accType == 3) {
        ISFSError ret = ISFS_ERROR_UNKNOWN;
        block->fileInfo = info;
        block->callback = cb;
        block->state = 0;
        block->copyBuf = buf;
        block->bufLength = length;
        ret = ISFS_CreateDirAsync((char*)"/tmp/sys", 0, 3, 3, 3, nandSafeOpenCallback, block);
    
        if (ret == ISFS_ERROR_OK) {
            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(ret);
        }
    } else {
        return NAND_RESULT_INVALID;
    }
}

void nandSafeOpenCallback(const ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result >= 0 || (result == ISFS_ERROR_EXISTS && b->state == 0)) {
        NANDFileInfo* info = b->fileInfo;
        ISFSError ret = ISFS_ERROR_UNKNOWN;

        if (b->state == 0) {
            info->stage = 1;
        }

        if (b->state == 2) {
            info->origFd = result;
            info->stage = 2;
        }

        if (b->state == 2 && b->simpleFlag) {
            b->state += 2;
        } else {
            ++b->state;
        }

        if (b->state == 1) {
            ret = ISFS_GetAttrAsync((char*)(info->origPath), &b->ownerId, &b->groupId, &b->attr, &b->ownerAcc, &b->groupAcc, &b->othersAcc, nandSafeOpenCallback, ctxt);
        } else if (b->state == 2) {
            ret = ISFS_OpenAsync((char*)(info->origPath), 1, nandSafeOpenCallback, ctxt);
        } else if (b->state == 3) {
            char tmpDir[64];
            b->uniqueNo = nandGetUniqueNumber();
            sprintf(tmpDir, "%s/%08x", "/tmp/sys", b->uniqueNo);
            ret = ISFS_CreateDirAsync((char*)tmpDir, 0, 3, 0, 0, nandSafeOpenCallback, ctxt);
        } else if (b->state == 4) {
            char filename[13];
            nandGetRelativeName(filename, info->origPath);

            if (!b->simpleFlag) {
                info->stage = 3;
                sprintf(info->tmpPath, "%s/%08x/%s", "/tmp/sys", b->uniqueNo, filename);
            }
            else {
                sprintf(info->tmpPath, "%s/%s", "/tmp/sys", filename);
            }

            ret = ISFS_CreateFileAsync((char*)info->tmpPath, b->attr, b->ownerAcc, b->groupAcc, b->othersAcc, nandSafeOpenCallback, ctxt);
        } else if (b->state == 5) {
            info->stage = 4;

            if (info->accType == 2) {
                ret = ISFS_OpenAsync((char*)info->tmpPath, 2, nandSafeOpenCallback, ctxt);
            } else if (info->accType == 3) {
                ret = ISFS_OpenAsync((char*)info->tmpPath, 3, nandSafeOpenCallback, ctxt);
            } else {
                ret = ISFS_ERROR_UNKNOWN;
            }
        } else if (b->state == 6) {
            info->fileDescriptor = result;
            info->stage = 5;
            b->state = 7;
            ret = ISFS_ReadAsync(info->origFd, b->copyBuf, b->bufLength, nandSafeOpenCallback, ctxt);
        } else if (b->state == 7) {
            ret = ISFS_ReadAsync(info->origFd, b->copyBuf, b->bufLength, nandSafeOpenCallback, ctxt);
        } else if (b->state == 8) {
            if (result > 0) {
                b->state = 6;
                ret = ISFS_WriteAsync(info->fileDescriptor, b->copyBuf, (u32)result, nandSafeOpenCallback, ctxt);
            } else if (result == 0) {
                ret = ISFS_SeekAsync(info->fileDescriptor, 0, 0, nandSafeOpenCallback, ctxt);
            }
        }  else if (b->state == 9) {
            if (result == 0) {
                if (!b->simpleFlag) {
                    info->mark = 3;
                } else {
                    info->mark = 5;
                }

                ((NANDCallback)(b->callback))(nandConvertErrorCode(ISFS_ERROR_OK), b);
            } else {
                ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
            }
            return;
        }

        if (ret != ISFS_ERROR_OK) {
            ((NANDCallback)(b->callback))(nandConvertErrorCode(ret), b);
        }
    } else {
        ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
    }
}

void nandReadOpenCallback(ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result >= 0) {
        ((NANDFileInfo*)(b->fileInfo))->fileDescriptor = result;
        ((NANDFileInfo*)(b->fileInfo))->stage = 2;

        if (!(b->simpleFlag)) {
            ((NANDFileInfo*)(b->fileInfo))->mark = 3;
        } else {
            ((NANDFileInfo*)(b->fileInfo))->mark = 5;
        }

        ((NANDCallback)(b->callback))(NAND_RESULT_OK, b);
    } else {
        ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
    }
}

s32 NANDSafeCloseAsync(NANDFileInfo* info, NANDCallback cb, NANDCommandBlock* block) {
    return nandSafeCloseAsync(info, cb, block, FALSE);
}

s32 nandSafeCloseAsync(NANDFileInfo* info, NANDCallback cb, NANDCommandBlock* block, BOOL simple_flag) {
    ISFSError err = ISFS_ERROR_UNKNOWN;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (!((info->mark == 3 && !simple_flag) || (info->mark == 5 && simple_flag))) {
        return NAND_RESULT_INVALID;
    }

    block->simpleFlag = simple_flag;
    if (info->accType == 1) {
        block->fileInfo = info;
        block->callback = cb;
        err = ISFS_CloseAsync(info->fileDescriptor, nandReadCloseCallback, block);
    } else if (info->accType == 2 || info->accType == 3) {
        block->fileInfo = info;
        block->callback = cb;
        block->state = 10;
        err = ISFS_CloseAsync(info->fileDescriptor, nandSafeCloseCallback, block);
    } else {
        err = ISFS_ERROR_INVALID;
    }

    return nandConvertErrorCode(err);
}

void nandSafeCloseCallback(ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result == 0) {
        NANDFileInfo* info = b->fileInfo;
        ISFSError ret = ISFS_ERROR_UNKNOWN;

        if (b->state == 12) {
            info->stage = 8;
        }

        if (b->state == 12 && b->simpleFlag) {
            b->state += 2;
        } else {
            ++b->state;
        }

        if (b->state == 11) {
            info->stage = 6;
            ret = ISFS_CloseAsync(info->origFd, nandSafeCloseCallback, ctxt);
        } else if (b->state == 12) {
            info->stage = 7;
            ret = ISFS_RenameAsync((char*)(info->tmpPath), (char*)(info->origPath), nandSafeCloseCallback, ctxt);
        } else if (b->state == 13) {
            char tmpdir[64] = "";
            nandGetParentDirectory(tmpdir, info->tmpPath);
            ret = ISFS_DeleteAsync((char*)tmpdir, nandSafeCloseCallback, ctxt);
        } else if (b->state == 14) {
            if (!(b->simpleFlag)) {
                info->stage = 9;
            }

            info->mark = 4;
            ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
            return;
        }

        if (ret != 0) {
            ((NANDCallback)(b->callback))(nandConvertErrorCode(ret), b);
        }
    } else {
        ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
    }
}

static void nandReadCloseCallback(ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result == 0) {
        ((NANDFileInfo*)(b->fileInfo))->stage = 7;
        ((NANDFileInfo*)(b->fileInfo))->mark = 4;
    }
    
    ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
}

static void nandCloseCallback(ISFSError result, void* ctxt) {
    NANDCommandBlock* b = (NANDCommandBlock*)ctxt;

    if (result == 0) {
        ((NANDFileInfo*)(b->fileInfo))->stage = 7;
        ((NANDFileInfo*)(b->fileInfo))->mark = 2;
    }
    
    ((NANDCallback)(b->callback))(nandConvertErrorCode(result), b);
}

static ISFSError nandCopy(s32 fd, s32 origFd, void* buf, const u32 length) {
    while (1) {
        ISFSError ret;
        ISFSError ret2;

        ret = ISFS_Read(origFd, buf, length);
        if (ret == 0) {
            return 0;
        }
        
        if (ret < 0) {
            return ret;
        }

        ret2 = ISFS_Write(fd, buf, ret);
        if (ret2 < 0) {
            return ret2;
        }

        if (ret2 != ret) {}
    }
}

static u32 nandGetUniqueNumber(void) {
    static u32 s_counter = 0;
    u32 ret;
    BOOL enabled = OSDisableInterrupts();
    ret = s_counter++;
    OSRestoreInterrupts(enabled);
    return ret;
}

s32 NANDSimpleSafeCancel(NANDFileInfo* info) {
    ISFSError err;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->accType == 1) {
        if (info->stage == 0) {
            return NAND_RESULT_OK;
        }
        if (info->stage == 2) {
            return nandConvertErrorCode(ISFS_Close(info->fileDescriptor));
        }
        return NAND_RESULT_INVALID;
    } else if (info->accType == 2 || info->accType == 3) {
        if (info->stage == 8) {
            return NAND_RESULT_INVALID;
        }

        if (info->stage == 7) {
            err = ISFS_Delete((const char*)info->tmpPath);
            if (err == 0) {
                info->stage = 0;
            } else {
                return nandConvertErrorCode(err);
            }
        }

        if (info->stage == 6) {
            err = ISFS_Delete((const char*)info->tmpPath);
            if (err == 0) {
                info->stage = 2;
            } else {
                return nandConvertErrorCode(err);
            }
        }

        if (info->stage == 5) {
            err = ISFS_Close(info->fileDescriptor);
            if (err == 0) {
                info->stage = 4;
            } else {
                return nandConvertErrorCode(err);
            }
        }

        if (info->stage == 4) {
            err = ISFS_Delete((const char*)info->tmpPath);
            if (err == 0) {
                info->stage = 2;
            } else {
                return nandConvertErrorCode(err);
            }
        }

        if (info->stage == 2) {
            err = ISFS_Close(info->origFd);
            if (err == 0) {
                info->stage = 0;
            } else {
                return nandConvertErrorCode(err);
            }
        }

        if (info->stage <= 1) {
            return NAND_RESULT_OK;
        }
        return NAND_RESULT_UNKNOWN;
    } else {
        return NAND_RESULT_INVALID;
    }
}
