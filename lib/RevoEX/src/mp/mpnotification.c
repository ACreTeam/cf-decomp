#include <private/mp.h>
#include <private/mpdl.h>
#include <private/wdp.h>
#include <private/wd.h>
#include <revolution/ncd.h>
#include <revolution/nwc24.h>
#include <revolution/net.h>
#include <revolution/vi.h>
#include <revolution/wpad.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

u32 mpNotificationBuffer[40];

u32 MPiStartupNotification(void) {
    int iVar1;
    iVar1 = mpWork;
    memset((void *)((void *)(mpWork + 0x61c0)), (int)(0), (u32)(0x2000));
    memset((void *)((void *)(iVar1 + 0xe5f0)), (int)(0), (u32)(0x318));
    return 0;
}

s32 MPiStartupNotificationLater(void) {
    if (!OSCreateThread(&MPi_Work->notificationThread,
                        (void *(*)(void *))HandleNotification, NULL,
                        MPi_Work->notificationStack +
                            sizeof(MPi_Work->notificationStack),
                        sizeof(MPi_Work->notificationStack),
                        MPi_Work->config.priority + 2, 0)) {
        return -239;
    }
    OSResumeThread(&MPi_Work->notificationThread);
    return 0;
}

void MPiCleanupNotificationEarlier(void) {
    MPWork *work = MPi_Work;
    if (work) {
        if (MPi_Work->notificationThread.state) {
            OSJoinThread(&MPi_Work->notificationThread, NULL);
        }
        memset(&work->notificationThread, 0, sizeof(OSThread));
    }
}

void MPiCleanupNotification(void) {
    return;
}

u32 HandleNotification(void) {
    WDNotification *notification = (WDNotification *)mpNotificationBuffer;
    for (;;) {
        s32 result = WD_ReceiveNotification(mpNotificationBuffer);
        if (result == (s32)0x80008000) {
            break;
        }
        switch (result) {
        case 0: {
            switch (notification->type) {
            case 0:
                MPiProcessConnectedNotification(
                    notification->aid, notification->macAddress,
                    (u32)notification->gameInfo, notification->reason);
                break;
            case 1:
                MPiProcessDisconnectedNotification(
                    notification->aid, notification->reason,
                    (u32)notification->macAddress);
                break;
            case 2:
                break;
            case 3:
                MPiProcessBeaconSentNotification();
                break;
            case 4:
                break;
            case 5:
                MPiProcessParentDataSendError();
                break;
            case 6:
                break;
            }
        } break;
        default:
            goto done;
        }
    }
done:
    return 0;
}
