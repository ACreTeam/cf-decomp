#include <prfile2/pf_standard.h>
#include <prfile2/pf_stub.h>

void pfstub_call_function_standard(struct PF_STUB_MSG* p_msg) {
    switch (p_msg->func) {
        case PFSTUB_CREATE:
            p_msg->result = (s32)pf2_create(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_FOPEN:
            p_msg->result = (s32)pf2_fopen(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_FCLOSE:
            p_msg->result = pf2_fclose(p_msg->ptr0);
            break;
        case PFSTUB_FREAD:
            p_msg->result = pf2_fread(p_msg->ptr0, p_msg->arg0, p_msg->arg1, p_msg->ptr1);
            break;
        case PFSTUB_FWRITE:
            p_msg->result = pf2_fwrite(p_msg->ptr0, p_msg->arg0, p_msg->arg1, p_msg->ptr1);
            break;
        case PFSTUB_FSEEK:
            p_msg->result = pf2_fseek(p_msg->ptr0, p_msg->arg0, p_msg->arg1);
            break;
        case PFSTUB_RESERVED_6:
            break;
        case PFSTUB_REMOVE:
            p_msg->result = pf2_remove(p_msg->ptr0);
            break;
        case PFSTUB_RENAME:
            p_msg->result = pf2_rename(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_MOVE:
            p_msg->result = pf2_move(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_MKDIR:
            p_msg->result = pf2_mkdir(p_msg->ptr0);
            break;
        case PFSTUB_CREATEDIR:
            p_msg->result = pf2_createdir(p_msg->ptr0, p_msg->arg0, p_msg->ptr1);
            break;
        case PFSTUB_RMDIR:
            p_msg->result = pf2_rmdir(p_msg->ptr0);
            break;
        case PFSTUB_CHDIR:
            p_msg->result = pf2_chdir(p_msg->ptr0);
            break;
        case PFSTUB_FSTAT:
            p_msg->result = pf2_fstat(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_CHMOD:
            p_msg->result = pf2_chmod(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_MOUNT:
            p_msg->result = pf2_mount(p_msg->drive);
            break;
        case PFSTUB_UNMOUNT:
            p_msg->result = pf2_unmount(p_msg->drive, p_msg->arg0);
            break;
        case PFSTUB_FORMAT:
            p_msg->result = pf2_format(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_BUFFERING:
            p_msg->result = pf2_buffering(p_msg->drive, p_msg->arg0);
            break;
        case PFSTUB_FERROR:
            p_msg->result = pf2_ferror(p_msg->ptr0);
            break;
        case PFSTUB_FEOF:
            p_msg->result = pf2_feof(p_msg->ptr0);
            break;
        case PFSTUB_ERRNUM:
            p_msg->result = pf2_errnum();
            break;
        case PFSTUB_DEVINF:
            p_msg->result = pf2_devinf(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_SETVOL:
            p_msg->result = pf2_setvol(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_GETVOL:
            p_msg->result = pf2_getvol(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_RMVVOL:
            p_msg->result = pf2_rmvvol(p_msg->drive);
            break;
        case PFSTUB_CHDMOD:
            p_msg->result = pf2_chdmod(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_FCONCAT:
            p_msg->result = pf2_fconcat(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_ATTACH:
            p_msg->result = pf2_attach(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_DETACH:
            p_msg->result = pf2_detach(p_msg->drive);
            break;
        case PFSTUB_FSFIRST:
            p_msg->result = pf2_fsfirst(p_msg->ptr0, p_msg->arg0, p_msg->ptr1);
            break;
        case PFSTUB_FSNEXT:
            p_msg->result = pf2_fsnext(p_msg->ptr0);
            break;
        case PFSTUB_FSEXEC:
            p_msg->result = pf2_fsexec(p_msg->ptr0, p_msg->arg0, p_msg->drive);
            break;
        case PFSTUB_SETUPFSI:
            p_msg->result = pf2_setupfsi(p_msg->drive, p_msg->arg0);
            break;
        case PFSTUB_SETCLSTLINK:
            p_msg->result = pf2_setclstlink(p_msg->drive, p_msg->arg0, p_msg->ptr0);
            break;
        case PFSTUB_FSETCLSTLINK:
            p_msg->result = pf2_fsetclstlink(p_msg->ptr0, p_msg->arg0, p_msg->ptr1);
            break;
        case PFSTUB_SYNC:
            p_msg->result = pf2_sync(p_msg->drive, p_msg->arg0);
            break;
        case PFSTUB_FSYNC:
            p_msg->result = pf2_fsync(p_msg->ptr0);
            break;
        case PFSTUB_FAPPEND:
            p_msg->result = pf2_fappend(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_FADJUST:
            p_msg->result = pf2_fadjust(p_msg->ptr0);
            break;
        case PFSTUB_FINFO:
            p_msg->result = pf2_finfo(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_SETTAILBUF:
            p_msg->result = pf2_settailbuf(p_msg->drive, p_msg->arg0, p_msg->ptr0);
            break;
        case PFSTUB_DERRNUM:
            p_msg->result = pf2_derrnum(p_msg->drive);
            break;
        case PFSTUB_COMBINE:
            p_msg->result = pf2_combine(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_DIVIDE:
            p_msg->result = pf2_divide(p_msg->ptr0, p_msg->ptr1, p_msg->arg0);
            break;
        case PFSTUB_XDIVIDE:
            p_msg->result = pf2_xdivide(p_msg->ptr0, p_msg->ptr1, p_msg->arg0);
            break;
        case PFSTUB_CINSERT:
            p_msg->result = pf2_cinsert(p_msg->ptr0, p_msg->arg0, p_msg->arg1);
            break;
        case PFSTUB_INSERT:
            p_msg->result = pf2_insert(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_CDELETE:
            p_msg->result = pf2_cdelete(p_msg->ptr0, p_msg->arg0, p_msg->arg1);
            break;
        case PFSTUB_CUT:
            p_msg->result = pf2_cut(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_SETVOLCFG:
            p_msg->result = pf2_setvolcfg(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_GETVOLCFG:
            p_msg->result = pf2_getvolcfg(p_msg->drive, p_msg->ptr0);
            break;
        case PFSTUB_SETCODE:
            p_msg->result = pf2_setcode(p_msg->ptr0);
            break;
        case PFSTUB_OPENDIR:
            p_msg->result = (s32)pf2_opendir(p_msg->ptr0);
            break;
        case PFSTUB_CLOSEDIR:
            p_msg->result = pf2_closedir(p_msg->ptr0);
            break;
        case PFSTUB_READDIR:
            p_msg->result = pf2_readdir(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_TELLDIR:
            p_msg->result = pf2_telldir(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_SEEKDIR:
            p_msg->result = pf2_seekdir(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_REWINDDIR:
            p_msg->result = pf2_rewinddir(p_msg->ptr0);
            break;
        case PFSTUB_FCHDIR:
            p_msg->result = pf2_fchdir(p_msg->ptr0);
            break;
        case PFSTUB_REGCTX:
            p_msg->result = pf2_regctx();
            break;
        case PFSTUB_UNREGCTX:
            p_msg->result = pf2_unregctx();
            break;
        case PFSTUB_FLOCK:
            p_msg->result = pf2_flock(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_SETENCODE:
            p_msg->result = pf2_setencode(p_msg->arg0);
            break;
        case PFSTUB_ISWRITEPROTECTED:
            p_msg->result = pf2_iswriteprotected(p_msg->drive);
            break;
        default:
            p_msg->result = PF_RET_ERR;
            break;
    }
}
