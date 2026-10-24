#include <prfile2/fa.h>
#include <prfile2/pfd_drv.h>

typedef struct FAMediaInOutCallback {
    PFD_MEDIA_CALLBACK insert;  // 0x00
    PFD_MEDIA_CALLBACK eject;   // 0x04
} FAMediaInOutCallback;

void FANotifySDInsert(s8 drive);
void FANotifySDEject(s8 drive);
void FANotifyUSBInsert(s8 drive);
void FANotifyUSBEject(s8 drive);

FAMediaInOutCallback gMediaInOutCallback[FA_DEVICE_CUSTOM];

s32 FARegistCB(u8 device, PFD_MEDIA_CALLBACK insert, PFD_MEDIA_CALLBACK eject) {
    s32 err;

    switch (device) {
        case FA_DEVICE_SD:
            gMediaInOutCallback[FA_DEVICE_SD].insert = insert;
            gMediaInOutCallback[FA_DEVICE_SD].eject = eject;
            err = pfd_sddrv_registar_callback(FANotifySDInsert, FANotifySDEject);
            break;
        case FA_DEVICE_USB:
            gMediaInOutCallback[FA_DEVICE_USB].insert = insert;
            gMediaInOutCallback[FA_DEVICE_USB].eject = eject;
            err = pfd_mscdrv_registar_callback(FANotifyUSBInsert, FANotifyUSBEject);
            break;
        default:
            return FA_RET_INVALID_PARAM;
    }
    if (err != 0) {
        return FA_RET_ERR;
    }
    return 0;
}

void FANotifySDInsert(s8 drive) {
    if (gMediaInOutCallback[FA_DEVICE_SD].insert != NULL) {
        gMediaInOutCallback[FA_DEVICE_SD].insert(drive);
    }
}

void FANotifySDEject(s8 drive) {
    if (gMediaInOutCallback[FA_DEVICE_SD].eject != NULL) {
        gMediaInOutCallback[FA_DEVICE_SD].eject(drive);
    }
}

void FANotifyUSBInsert(s8 drive) {
    if (gMediaInOutCallback[FA_DEVICE_USB].insert != NULL) {
        gMediaInOutCallback[FA_DEVICE_USB].insert(drive);
    }
}

void FANotifyUSBEject(s8 drive) {
    if (gMediaInOutCallback[FA_DEVICE_USB].eject != NULL) {
        gMediaInOutCallback[FA_DEVICE_USB].eject(drive);
    }
}
