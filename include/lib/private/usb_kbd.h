#ifndef PRIVATE_USB_KBD_H
#define PRIVATE_USB_KBD_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    USB_KBD_SUCCESS,
    USB_KBD_ERROR,
    USB_KBD_ALREADY_INITIALIZED,
    USB_KBD_NOT_INITIALIZED,
    USB_KBD_CANNOT_OPEN_DRIVER,
    USB_KBD_INVALID_PARAMETER,
    USB_KBD_ASYNC_SETUP_FAILED
} USBKBDEc;

typedef u32 USBKBDId;
typedef void (*USBKBDCallback)(BOOL success, void* cbarg);

typedef struct _USBKBDCmd {
    /* 0x00 */ volatile USBKBDId id;
    /* 0x04 */ volatile u8 data[20];
    /* 0x18 */ USBKBDCallback cb;
    /* 0x1C */ void* cbarg;
} USBKBDCmd; // size 0x20

#define USB_KBD_NUM_HID_KEY_CODES 6

typedef struct _USBKBDReport {
    /* 0x0 */ u8 modifier;
    /* 0x1 */ u8 reserved;
    /* 0x2 */ u8 keycode[USB_KBD_NUM_HID_KEY_CODES];
} USBKBDReport; // size 0x8

typedef void (*USBKBDAttachCallback)(USBKBDId id);
typedef void (*USBKBDDetachCallback)(USBKBDId id);
typedef void (*USBKBDEventCallback)(USBKBDId id, USBKBDReport* report);

USBKBDEc USBKBDInitialize(USBKBDAttachCallback attach_cb, USBKBDDetachCallback detach_cb);
USBKBDEc USBKBDRegisterEventCallback(USBKBDEventCallback cb);
USBKBDEc USBKBDSetLED(USBKBDId id, u8 state, USBKBDCmd* buf);
USBKBDEc USBKBDSetLEDAsync(USBKBDId id, u8 state, USBKBDCmd* buf, USBKBDCallback cb, void* cbarg);

#ifdef __cplusplus
}
#endif

#endif // PRIVATE_USB_KBD_H
