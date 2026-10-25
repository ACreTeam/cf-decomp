#include <private/ios.h>
#include <private/ipc.h>
#include <private/usb_kbd.h>
#include <revolution/os.h>

typedef enum {
    USB_KBD_MSG_ATTACH,
    USB_KBD_MSG_DETACH,
    USB_KBD_MSG_EVENT
} USBKBDMsgType;

typedef struct USBKBDMsg {
    /* 0x0 */ USBKBDMsgType type;
    /* 0x4 */ USBKBDId id;
    /* 0x8 */ USBKBDReport report;
} USBKBDMsg; // size 0x10

#define USB_KBD_IOCTL_GET_MSG 0
#define USB_KBD_LED_CMD_SIZE 8

static IOSFd usbKbdFd;
static USBKBDAttachCallback usbKbdAttachCallback;
static USBKBDDetachCallback usbKbdDetachCallback;
static USBKBDEventCallback usbKbdEventCallback;
static USBKBDMsg* usbKbdIoctlMsg;
static BOOL usbKbdIoctlStarted;
static BOOL usbKbdInitialized;
static u8 usbKbdIoctlBuf[OSRoundUp32B(sizeof(USBKBDMsg))] ATTRIBUTE_ALIGN(32);

static IOSError usbKbdIoctlCallback(IOSError result, void* arg) {
    USBKBDMsg* msg = arg;

    switch (msg->type) {
        case USB_KBD_MSG_ATTACH:
            if (usbKbdAttachCallback != NULL) {
                usbKbdAttachCallback(msg->id);
            }
            break;
        case USB_KBD_MSG_DETACH:
            if (usbKbdDetachCallback != NULL) {
                usbKbdDetachCallback(msg->id);
            }
            break;
        case USB_KBD_MSG_EVENT:
            if (usbKbdEventCallback != NULL) {
                usbKbdEventCallback(msg->id, &msg->report);
            }
            break;
    }

    IOS_IoctlAsync(usbKbdFd, USB_KBD_IOCTL_GET_MSG, NULL, 0, usbKbdIoctlMsg, sizeof(USBKBDMsg), usbKbdIoctlCallback, usbKbdIoctlMsg);
    return IPC_RESULT_OK;
}

USBKBDEc USBKBDInitialize(USBKBDAttachCallback attach_cb, USBKBDDetachCallback detach_cb) {
    if (usbKbdInitialized) {
        return USB_KBD_ALREADY_INITIALIZED;
    }

    usbKbdFd = IOS_Open("/dev/usb/kbd", IPC_ACCESS_NONE);
    if (usbKbdFd < 0) {
        return USB_KBD_CANNOT_OPEN_DRIVER;
    }

    usbKbdAttachCallback = attach_cb;
    usbKbdDetachCallback = detach_cb;
    usbKbdEventCallback = NULL;
    usbKbdInitialized = TRUE;
    usbKbdIoctlMsg = (USBKBDMsg*)usbKbdIoctlBuf;

    if (!usbKbdIoctlStarted) {
        if (IOS_IoctlAsync(usbKbdFd, USB_KBD_IOCTL_GET_MSG, NULL, 0, usbKbdIoctlMsg, sizeof(USBKBDMsg), usbKbdIoctlCallback, usbKbdIoctlMsg) != IPC_RESULT_OK) {
            return USB_KBD_ASYNC_SETUP_FAILED;
        }

        usbKbdIoctlStarted = TRUE;
    } else {
        return USB_KBD_SUCCESS;
    }

    return USB_KBD_SUCCESS;
}

USBKBDEc USBKBDSetLED(USBKBDId id, u8 state, USBKBDCmd* buf) {
    if (usbKbdFd < 0) {
        return USB_KBD_NOT_INITIALIZED;
    }

    buf->data[0] = state;
    buf->id = id;
    return IOS_Write(usbKbdFd, buf, USB_KBD_LED_CMD_SIZE) != IPC_RESULT_OK ? USB_KBD_ERROR : USB_KBD_SUCCESS;
}

static IOSError usbKbdLedCallback(IOSError result, void* arg) {
    USBKBDCmd* cmd = arg;

    cmd->cb(result == IPC_RESULT_OK, cmd->cbarg);
    return IPC_RESULT_OK;
}

USBKBDEc USBKBDSetLEDAsync(USBKBDId id, u8 state, USBKBDCmd* buf, USBKBDCallback cb, void* cbarg) {
    if (usbKbdFd < 0) {
        return USB_KBD_NOT_INITIALIZED;
    }

    buf->id = id;
    buf->data[0] = state;
    buf->cb = cb;
    buf->cbarg = cbarg;
    return IOS_WriteAsync(usbKbdFd, buf, USB_KBD_LED_CMD_SIZE, usbKbdLedCallback, buf) != IPC_RESULT_OK ? USB_KBD_ERROR : USB_KBD_SUCCESS;
}

USBKBDEc USBKBDRegisterEventCallback(USBKBDEventCallback cb) {
    if (usbKbdInitialized) {
        usbKbdEventCallback = cb;
        return USB_KBD_SUCCESS;
    }

    return USB_KBD_NOT_INITIALIZED;
}
