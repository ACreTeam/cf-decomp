#include <prfile2/pfk_api.h>

s32 pfk_create_task(PFK_TASK_INFO* p_info) {
    if (p_info == NULL) {
        OSReport("prfile2:ERR pfk_create_task() invalid param\n");
        return 0;
    }
    if (!OSCreateThread(p_info->thread, p_info->entry, p_info->param, p_info->stack + p_info->stack_size,
                        p_info->stack_size, p_info->priority, 0)) {
        OSReport("prfile2:ERR pfk_create_task() failed to create thread\n");
        return 0;
    }
    return (s32)p_info->thread;
}

s32 pfk_start_task(s32 tsk_id) {
    if (tsk_id == 0) {
        OSReport("prfile2:ERR pfk_start_task() invalid tsk_id\n");
        return PFK_ERR_PARAM;
    }
    if (OSIsThreadSuspended((OSThread*)tsk_id) == TRUE) {
        OSResumeThread((OSThread*)tsk_id);
    }
    return 0;
}

s32 pfk_get_task_id(s32* p_tsk_id) {
    OSThread* thread;

    if (p_tsk_id == NULL) {
        OSReport("prfile2:ERR pfk_get_task_id() invalid param\n");
        return PFK_ERR_PARAM;
    }
    thread = OSGetCurrentThread();
    if (thread == NULL) {
        OSReport("INFO failed to OSGetCurrentThread\n");
        *p_tsk_id = 0;
        return PFK_ERR_OS;
    }
    *p_tsk_id = (s32)thread;
    return 0;
}

s32 pfk_create_mailbox(PFK_MAILBOX_INFO* p_info) {
    if (p_info == NULL || p_info->queue == NULL || p_info->msgs == NULL) {
        OSReport("prfile2:ERR pfk_create_mailbox() Invalid Param\n");
        return 0;
    }
    OSInitMessageQueue(p_info->queue, p_info->msgs, p_info->num_msgs);
    return (s32)p_info->queue;
}

s32 pfk_receive_message(s32 mbx_id, OSMessage* p_msg) {
    if (mbx_id == 0) {
        OSReport("prfile2:ERR pfk_receive_message() Invalid Param\n");
        return PFK_ERR_PARAM;
    }
    if (!OSReceiveMessage((OSMessageQueue*)mbx_id, p_msg, OS_MESSAGE_BLOCK)) {
        OSReport("prfile2:ERR pfk_receive_message() Faile to OSReceiveMessage()\n");
        return PFK_ERR_OS;
    }
    return 0;
}

s32 pfk_send_message(s32 mbx_id, void* msg) {
    if (mbx_id == 0 || msg == NULL) {
        OSReport("prfile2:ERR pfk_send_message() Invalid Param\n");
        return PFK_ERR_PARAM;
    }
    if (!OSSendMessage((OSMessageQueue*)mbx_id, msg, OS_MESSAGE_NOBLOCK)) {
        OSReport("prfile2:ERR pfk_send_message() Failed to pfk_send_message()\n");
        return PFK_ERR_OS;
    }
    return 0;
}

s32 pfk_create_semaphore(PFK_SEMAPHORE_INFO* p_info) {
    s32 num_wait;

    if (p_info == NULL || p_info->sem == NULL || p_info->initial > p_info->max) {
        OSReport("prfile2:ERR pfk_create_semaphore() Invalid Param\n");
        return 0;
    }
    OSInitSemaphore(p_info->sem, p_info->max);
    for (num_wait = p_info->max - p_info->initial; num_wait != 0; num_wait--) {
        OSWaitSemaphore(p_info->sem);
    }
    return (s32)p_info->sem;
}

s32 pfk_delete_semaphore(s32 sem_id) {
    if (sem_id == 0) {
        OSReport("prfile2:ERR pfk_delete_semaphore() Invalid Param\n");
        return PFK_ERR_PARAM;
    }
    return 0;
}

s32 pfk_get_semaphore(s32 sem_id) {
    if (sem_id == 0) {
        OSReport("prfile2:ERR pfk_get_semaphore() Invalid Param\n");
        return PFK_ERR_PARAM;
    }
    OSWaitSemaphore((OSSemaphore*)sem_id);
    return 0;
}

s32 pfk_release_semaphore(s32 sem_id) {
    if (sem_id == 0) {
        OSReport("prfile2:ERR pfk_release_semaphore() Invalid Param\n");
        return PFK_ERR_PARAM;
    }
    OSSignalSemaphore((OSSemaphore*)sem_id);
    return 0;
}
