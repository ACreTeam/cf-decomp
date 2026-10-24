#include <prfile2/pf_standard.h>
#include <prfile2/pf_stub.h>

void pfstub_call_function_unicode(struct PF_STUB_MSG* p_msg) {
    switch (p_msg->func) {
        case PFSTUB_W_CREATE:
            p_msg->result = (s32)pf2_w_create(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_W_FOPEN:
            p_msg->result = (s32)pf2_w_fopen(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_REMOVE:
            p_msg->result = pf2_w_remove(p_msg->ptr0);
            break;
        case PFSTUB_W_RENAME:
            p_msg->result = pf2_w_rename(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_MOVE:
            p_msg->result = pf2_w_move(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_MKDIR:
            p_msg->result = pf2_w_mkdir(p_msg->ptr0);
            break;
        case PFSTUB_W_CREATEDIR:
            p_msg->result = pf2_w_createdir(p_msg->ptr0, p_msg->arg0, p_msg->ptr1);
            break;
        case PFSTUB_W_RMDIR:
            p_msg->result = pf2_w_rmdir(p_msg->ptr0);
            break;
        case PFSTUB_W_CHDIR:
            p_msg->result = pf2_w_chdir(p_msg->ptr0);
            break;
        case PFSTUB_W_FSTAT:
            p_msg->result = pf2_w_fstat(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_CHMOD:
            p_msg->result = pf2_w_chmod(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_W_CHDMOD:
            p_msg->result = pf2_w_chdmod(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_W_FCONCAT:
            p_msg->result = pf2_w_fconcat(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_FSFIRST:
            p_msg->result = pf2_w_fsfirst(p_msg->ptr0, p_msg->arg0, p_msg->ptr1);
            break;
        case PFSTUB_W_FSNEXT:
            p_msg->result = pf2_w_fsnext(p_msg->ptr0);
            break;
        case PFSTUB_W_FSEXEC:
            p_msg->result = pf2_w_fsexec(p_msg->ptr0, p_msg->arg0, p_msg->drive);
            break;
        case PFSTUB_W_COMBINE:
            p_msg->result = pf2_w_combine(p_msg->ptr0, p_msg->ptr1);
            break;
        case PFSTUB_W_DIVIDE:
            p_msg->result = pf2_w_divide(p_msg->ptr0, p_msg->ptr1, p_msg->arg0);
            break;
        case PFSTUB_W_XDIVIDE:
            p_msg->result = pf2_w_xdivide(p_msg->ptr0, p_msg->ptr1, p_msg->arg0);
            break;
        case PFSTUB_W_CINSERT:
            p_msg->result = pf2_w_cinsert(p_msg->ptr0, p_msg->arg0, p_msg->arg1);
            break;
        case PFSTUB_W_INSERT:
            p_msg->result = pf2_w_insert(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_W_CDELETE:
            p_msg->result = pf2_w_cdelete(p_msg->ptr0, p_msg->arg0, p_msg->arg1);
            break;
        case PFSTUB_W_CUT:
            p_msg->result = pf2_w_cut(p_msg->ptr0, p_msg->arg0);
            break;
        case PFSTUB_W_OPENDIR:
            p_msg->result = (s32)pf2_w_opendir(p_msg->ptr0);
            break;
        default:
            p_msg->result = PF_RET_ERR;
            break;
    }
}
