#include <dwc/dwc_reliable.h>
#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>

typedef struct _tagDWCiSendQueue {
    GT2Connection connection;
    void *buffer;
    u32 size;
    struct _tagDWCiSendQueue *next;
} DWCiSendQueue;

static struct {
    DWCiSendQueue *queue;
    int usedheap;
} s_queueinfo;

void DWCi_InitReliableQueue(void) {
    s_queueinfo.queue = NULL;
    s_queueinfo.usedheap = 0;
}

void DWCi_CleanupReliableQueue(void) {
    DWCiSendQueue *it = s_queueinfo.queue;

    while (it != NULL) {
        s_queueinfo.queue = it->next;
        DWC_Free(DWC_ALLOCTYPE_BASE, it->buffer, it->size);
        DWC_Free(DWC_ALLOCTYPE_BASE, it, 0);
        it = s_queueinfo.queue;
    }

    s_queueinfo.queue = NULL;
    s_queueinfo.usedheap = 0;
}

BOOL DWCi_IsReliableQueueFree(void) {
    return s_queueinfo.usedheap < 0x6000;
}

BOOL DWCi_ProcessReliableQueue(void) {
    return DWCi_ProcessReliableQueueEx(gt2Send);
}

BOOL DWCi_ProcessReliableQueueEx(DWCReliableSendFunc func) {
    DWCiSendQueue *it;
    int retrycount;

    for (it = s_queueinfo.queue; it != NULL; it = it->next) {
        for (retrycount = 0; retrycount < 3; retrycount++) {
            if (func(it->connection, it->buffer, it->size,
                         GT2True) == GT2Success) {
                s_queueinfo.usedheap -= it->size;
                DWC_Free(DWC_ALLOCTYPE_BASE, it->buffer, it->size);
                it->buffer = NULL;
                it->size = 0;
                it->connection = NULL;
                break;
            }
        }

        if (retrycount == 3) {
            DWC_Printf(DWC_REPORTFLAG_TRANSPORT,
                       "DWCi_ProcessReliableQueue() retry count exceeded!\n");
            break;
        }
    }

    it = s_queueinfo.queue;
    while (it != NULL) {
        if (it->buffer != NULL) {
            break;
        }

        s_queueinfo.queue = it->next;
        DWC_Free(DWC_ALLOCTYPE_BASE, it, 0);
        it = s_queueinfo.queue;
    }

    return s_queueinfo.queue == NULL;
}

BOOL DWCi_InsertReliableQueue(GT2Connection connection, const void *buffer,
                              int size) {
    DWCiSendQueue **target = &s_queueinfo.queue;
    DWCiSendQueue *it;

    if (s_queueinfo.usedheap + size > 0x8000) {
        return FALSE;
    }

    for (it = s_queueinfo.queue; it != NULL; it = it->next) {
        target = &it->next;
    }

    *target = DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(DWCiSendQueue));
    if (*target == NULL) {
        return FALSE;
    }

    (*target)->buffer = DWC_Alloc(DWC_ALLOCTYPE_BASE, size);
    if ((*target)->buffer == NULL) {
        DWC_Free(DWC_ALLOCTYPE_BASE, *target, sizeof(DWCiSendQueue));
        *target = NULL;
        return FALSE;
    }

    (*target)->connection = connection;
    (*target)->size = size;
    (*target)->next = NULL;
    DWCi_Np_CPUCopyFast((*target)->buffer, buffer, size);
    s_queueinfo.usedheap += size;
    return TRUE;
}
