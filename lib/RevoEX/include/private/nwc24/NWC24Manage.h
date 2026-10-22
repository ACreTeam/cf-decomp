#ifndef PRIVATE_NWC24_MANAGE_H
#define PRIVATE_NWC24_MANAGE_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24Types.h>

#include <private/nwc24/NWC24Config.h>
#include <private/nwc24/NWC24Download.h>
#include <private/nwc24/NWC24FriendList.h>
#include <private/nwc24/NWC24MBox.h>
#include <private/nwc24/NWC24Mime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24i_IO_BUFFER_SIZE 512
#define NWC24i_STRING_WORK_SIZE 1024
#define NWC24i_PATH_WORK_SIZE 256

#define NWC24i_APP_ID_IPL 'HAEA'

typedef enum NWC24iMsgArrivedFlags {
    NWC24_MSG_ARRIVED = 1 << 0,
    NWC24_MSG_ARRIVED_FOR_MENU = 1 << 1,
} NWC24iMsgArrivedFlags;

#define NWC24i_WORK_SIZE(x) (ROUNDUP(sizeof(x), 0x100))

typedef struct NWC24Work {
    char stringWork[NWC24i_STRING_WORK_SIZE];  // 0x000
    char mainWork[0x800 - 0x400];              // 0x400
    char pathWork[NWC24i_PATH_WORK_SIZE];      // 0x800

    u8 readBuffer[NWC24i_IO_BUFFER_SIZE];  // 0x900
    u8 writeBuffer[2048];                  // 0xB00

    u8 configData[NWC24i_WORK_SIZE(NWC24ConfigData)];  // 0x1300

    NWC24iMBCHeader sendCtrl;  // 0x1700
    NWC24iMBCHeader recvCtrl;  // 0x1780
    NWC24iMBCEntry mbcEntry;   // 0x1800
    char unk_0x1880[128];      // 0x1880
    char base64Work[NWC24_BASE64_TABLE_SIZE];  // 0x1900
    char unk_0x1A00[0x2A00 - 0x1A00];          // 0x1A00

    u8 flHead[NWC24i_WORK_SIZE(NWC24FLHeader)];               // 0x2A00
    u8 secretFlHead[NWC24i_WORK_SIZE(NWC24SecretFLHeader)];  // 0x2E00

    u8 dlHead[NWC24i_WORK_SIZE(NWC24iDlHeader)];  // 0x3600
    u8 dlTask[NWC24i_WORK_SIZE(NWC24iDlTask)];    // 0x3E00
} NWC24Work;

extern NWC24Work* NWC24WorkP;

void NWC24iRegister();
void NWC24iRegisterSecretFList();

NWC24Err NWC24iSetNewMsgArrived(u32 flags);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_MANAGE_H
