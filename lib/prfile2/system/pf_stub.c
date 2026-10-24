#include <prfile2/fa.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_stub.h>
#include <prfile2/pf_system.h>
#include <prfile2/pfd_drv.h>
#include <prfile2/pfk_api.h>

void* pfstub_entry(void* param);

void pfstub_init_stub(void) {
    PFK_MAILBOX_INFO mbx_info;
    PFK_TASK_INFO tsk_info;

    if (pf_sys_set.initialized == FALSE) {
        mbx_info.queue = &pf_sys_set.queue;
        mbx_info.msgs = pf_sys_set.msgs;
        mbx_info.num_msgs = PF_STUB_MSG_NUM;
        pf_sys_set.mbx_id = pfk_create_mailbox(&mbx_info);
        tsk_info.entry = pfstub_entry;
        tsk_info.param = NULL;
        tsk_info.priority = PF_STUB_PRIORITY;
        tsk_info.stack_size = PF_STUB_STACK_SIZE;
        tsk_info.stack = pf_sys_set.stack;
        tsk_info.thread = &pf_sys_set.thread;
        pf_sys_set.tsk_id = pfk_create_task(&tsk_info);
        pfk_start_task(pf_sys_set.tsk_id);
        pf_sys_set.initialized = TRUE;
    }
}

void* pfstub_entry(void* param) {
    void* p_rcv = NULL;
    struct PF_STUB_MSG* p_msg;

    while (1) {
        if (pfk_receive_message(pf_sys_set.mbx_id, &p_rcv) != 0) {
            continue;
        }
        p_msg = p_rcv;
        if (p_msg == NULL) {
            continue;
        }
        pf_vol_set.context_id = p_msg->tsk_id;
        if (p_msg->func >= 0 && p_msg->func < PFSTUB_STANDARD_END) {
            pfstub_call_function_standard(p_msg);
        } else {
            struct PF_STUB_MSG* p_unicode_msg = p_rcv;

            if (p_unicode_msg->func >= PFSTUB_STANDARD_END && p_unicode_msg->func < PFSTUB_UNICODE_END) {
                pfstub_call_function_unicode(p_unicode_msg);
            } else {
                p_msg->result = PF_RET_ERR;
            }
        }
        if (p_msg->callback == NULL) {
            pfk_release_semaphore(p_msg->sem_id);
        } else {
            if (p_msg->func == PFSTUB_ATTACH) {
                s32 idx = p_msg->p_drv_tbl->drive - 'A';

                gOpenDisk[idx] = p_msg->p_disk;
                gOpenPartition[idx] = p_msg->p_part;
                if (p_msg->arg0 == FA_DEVICE_SD) {
                    if (pfd_sddrv_is_media_insert()) {
                        p_msg->p_drv_tbl->stat |= PF_DRV_STAT_INSERTED;
                    }
                } else if (p_msg->arg0 == FA_DEVICE_USB) {
                    if (pfd_mscdrv_is_media_insert(p_msg->p_disk)) {
                        p_msg->p_drv_tbl->stat |= PF_DRV_STAT_INSERTED;
                    }
                }
            }
            p_msg->callback(pfstub_store_result(p_msg), p_msg->callback_param);
        }
    }
}

s32 pfstub_com_massage(struct PF_STUB_MSG* p_msg) {
    s32 tsk_id;
    PFK_SEMAPHORE_INFO sem_info;
    OSSemaphore sem;
    s32 err;

    if (p_msg->callback == NULL) {
        sem_info.initial = 0;
        sem_info.max = 1;
        sem_info.sem = &sem;
        p_msg->sem_id = pfk_create_semaphore(&sem_info);
    } else {
        p_msg->sem_id = 0;
    }
    if (pfk_get_task_id(&tsk_id) != 0) {
        return PF_RET_ERR;
    }
    p_msg->tsk_id = tsk_id;
    if (pfk_send_message(pf_sys_set.mbx_id, p_msg) != 0) {
        return PF_RET_ERR;
    }
    if (p_msg->callback == NULL) {
        err = pfk_get_semaphore(p_msg->sem_id);
        pfk_delete_semaphore(p_msg->sem_id);
        if (err != 0) {
            return PF_RET_ERR;
        }
    }
    return 0;
}

s32 pfstub_store_result(struct PF_STUB_MSG* p_msg) {
    s32 ret = 0;

    if (p_msg->p_result != NULL) {
        *p_msg->p_result = p_msg->result;
        switch (p_msg->func) {
            case PFSTUB_CREATE:
            case PFSTUB_FOPEN:
            case PFSTUB_OPENDIR:
            case PFSTUB_W_CREATE:
            case PFSTUB_W_FOPEN:
            case PFSTUB_W_OPENDIR:
                if (p_msg->result == 0) {
                    ret = PF_RET_ERR;
                }
                break;
            case PFSTUB_FAPPEND:
                if (p_msg->result != p_msg->arg0) {
                    ret = PF_RET_ERR;
                }
                break;
            case PFSTUB_FREAD:
            case PFSTUB_FWRITE:
                if (p_msg->result != p_msg->arg1) {
                    ret = PF_RET_ERR;
                }
                break;
        }
    } else {
        ret = p_msg->result;
    }
    return ret;
}
