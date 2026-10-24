#ifndef PRFILE2_PFK_API_H
#define PRFILE2_PFK_API_H

// PrFILE2 kernel abstraction: tasks, mailboxes and semaphores on the RVL OS; an id is the OS object, 0 on failure.

#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PFK_ERR_PARAM = -2,
    PFK_ERR_OS = -3,
};

typedef struct PFK_TASK_INFO {
    void* (*entry)(void*);  // 0x00
    void* param;            // 0x04
    u32 stack_size;         // 0x08
    u8* stack;              // 0x0C
    s32 priority;           // 0x10
    OSThread* thread;       // 0x14
} PFK_TASK_INFO;

typedef struct PFK_MAILBOX_INFO {
    OSMessageQueue* queue;  // 0x00
    OSMessage* msgs;        // 0x04
    s32 num_msgs;           // 0x08
} PFK_MAILBOX_INFO;

typedef struct PFK_SEMAPHORE_INFO {
    u32 initial;       // 0x00
    u32 max;           // 0x04
    OSSemaphore* sem;  // 0x08
} PFK_SEMAPHORE_INFO;

s32 pfk_create_task(PFK_TASK_INFO* p_info);
s32 pfk_start_task(s32 tsk_id);
s32 pfk_get_task_id(s32* p_tsk_id);
s32 pfk_create_mailbox(PFK_MAILBOX_INFO* p_info);
s32 pfk_receive_message(s32 mbx_id, OSMessage* p_msg);
s32 pfk_send_message(s32 mbx_id, void* msg);
s32 pfk_create_semaphore(PFK_SEMAPHORE_INFO* p_info);
s32 pfk_delete_semaphore(s32 sem_id);
s32 pfk_get_semaphore(s32 sem_id);
s32 pfk_release_semaphore(s32 sem_id);

#ifdef __cplusplus
}
#endif
#endif
