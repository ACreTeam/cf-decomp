#include <private/nhttp.h>

#include <revolution/ncd.h>
#include <revolution/os.h>

void NHTTPi_InitMutexInfo(NHTTPMutexInfo* info)
{
    info->initialized = FALSE;
}

void NHTTPi_initLockReqList(NHTTPMutexInfo* info)
{
    if (!info->initialized)
    {
        OSInitMutex(&info->mutex);
        info->initialized = TRUE;
    }
}

void NHTTPi_exitLockReqList(void)
{
}

void NHTTPi_lockReqList(NHTTPMutexInfo* info)
{
    OSLockMutex(&info->mutex);
}

void NHTTPi_unlockReqList(NHTTPMutexInfo* info)
{
    OSUnlockMutex(&info->mutex);
}

BOOL NHTTPi_createCommThread(NHTTPThreadInfo* info, s32 priority, void* stack)
{
    if (!NHTTPi_IsCreateCommThreadMessageQueue(info))
    {
        OSInitMessageQueue(&info->messageQueue, info->messages, 3);
        NHTTPi_IsCreateCommThreadMessageQueueOn(info);
    }

    OSCreateThread(&info->thread, NHTTPi_CommThreadProc, NULL, (u8*)stack + 0x2000, 0x2000, priority, 0);
    OSResumeThread(&info->thread);
    return TRUE;
}

void NHTTPi_destroyCommThread(NHTTPThreadInfo* info, NHTTPBgnEndInfo* bgnEndInfo)
{
    bgnEndInfo->_unk7DC = 1;
    OSSendMessage(&info->messageQueue, NULL, 0);
    OSJoinThread(&info->thread, NULL);
}

void NHTTPi_idleCommThread(NHTTPThreadInfo* info)
{
    OSMessage message;

    OSReceiveMessage(&info->messageQueue, &message, OS_MESSAGE_BLOCK);
}

void NHTTPi_kickCommThread(NHTTPThreadInfo* info)
{
    OSSendMessage(&info->messageQueue, NULL, 0);
}

void NHTTPi_CheckCurrentThread(NHTTPThreadInfo* info, BOOL notCommThread)
{
    OSThread* current = OSGetCurrentThread();
    OSThread* commThread = &info->thread;

    if (current != NULL
        && ((!notCommThread && current != commThread)
            || (notCommThread && current == commThread)))
    {
        OSReport("%s:illegal thread\n", __FUNCTION__);
        OSPanic(__FILE__, 223, "halt\n");
    }
}

void* NHTTPi_CommThreadProc(void* argument)
{
    NHTTPi_CommThreadProcMain(argument);
    return NULL;
}
