#ifndef PRIVATE_USB_H
#define PRIVATE_USB_H

#include <revolution/types.h>

#include <private/ios/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SWAP32(val)                                                                                                                                      ((u32)((((u32)(val) & (u32)0x000000ffUL) << 24) | (((u32)(val) & (u32)0x0000ff00UL) << 8) | (((u32)(val) & (u32)0x00ff0000UL) >> 8) |                       (((u32)(val) & (u32)0xff000000UL) >> 24)))

#define SWAP16(val) ((u16)((((u16)(val) & (u16)0x00ffUL) << 8) | (((u16)(val) & (u16)0xff00) >> 8)))

typedef struct USB_DevDescr {
    u8 bLength;
    u8 bDescriptorType;
    u16 bcdUSB;
    u8 bDeviceClass;
    u8 bDeviceSubClass;
    u8 bDeviceProtocol;
    u8 bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber;
    u8 bNumConfigurations;
} USB_DevDescr;

typedef struct USBDeviceInfo {
    IOSFd fd;
    u16 vid;
    u16 pid;
} USBDeviceInfo;

typedef struct IsoTransfer {
    void* buf;
    u8 numPackets;
    u16* packets;
} IsoTransfer;

typedef void (*USBCallback)(IOSError result, void* arg);
typedef void (*USBIsoCallback)(IOSError err, IsoTransfer* xfer, void* cbArg);

IOSError IUSB_OpenLib();
IOSError IUSB_CloseLib();

IOSError IUSB_OpenDeviceIds(const char* interface, u16 vid, u16 pid, IOSError* resultOut);

IOSError IUSB_CloseDeviceAsync(s32 fd, USBCallback callback, void* callbackArg);

IOSError IUSB_ReadIntrMsgAsync(s32 fd, u32 endpoint, u32 length, void* buffer, USBCallback callback, void* callbackArg);
IOSError IUSB_ReadBlkMsgAsync(s32 fd, u32 endpoint, u32 length, void* buffer, USBCallback callback, void* callbackArg);

IOSError IUSB_WriteBlkMsgAsync(s32 fd, u32 endpoint, u32 length, void* buffer, USBCallback callback, void* callbackArg);
IOSError IUSB_WriteCtrlMsgAsync(s32 fd, u8 requestType, u8 request, u16 value, u16 index, u16 length, void* buffer, USBCallback callback,
                                void* callbackArg);

IOSError IUSB_OpenDeviceIdsAsync(const char* interface, u16 vid, u16 pid, USBCallback callback, void* callbackArg);
IOSError IUSB_CloseDevice(s32 fd);
IOSError IUSB_GetDeviceList(const char* path, USBDeviceInfo* deviceList, u8 maxDev, u8 deviceClass, u8* numDev);

IOSError IUSB_ReadIntrMsg(s32 fd, u32 endpoint, u32 length, char* buffer);
IOSError IUSB_WriteIntrMsg(s32 fd, u32 endpoint, u32 length, char* buffer);
IOSError IUSB_WriteIntrMsgAsync(s32 fd, u32 endpoint, u32 length, char* buffer, USBCallback callback, void* callbackArg);

IOSError IUSB_ReadCtrlMsg(s32 fd, u8 requestType, u8 request, u16 value, u16 index, u16 length, char* buffer);
IOSError IUSB_ReadCtrlMsgAsync(s32 fd, u8 requestType, u8 request, u16 value, u16 index, u16 length, char* buffer, USBCallback callback,
                               void* callbackArg);
IOSError IUSB_WriteCtrlMsg(s32 fd, u8 requestType, u8 request, u16 value, u16 index, u16 length, char* buffer);

IOSError IUSB_GetAsciiStr(s32 fd, u8 ep, u16 index, u16 langId, char* buffer, u16 length);
IOSError IUSB_GetAsciiStrAsync(s32 fd, u8 ep, u16 index, u16 langId, char* buffer, u16 length, USBCallback callback, void* callbackArg);
IOSError IUSB_GetDevDescr(s32 fd, USB_DevDescr* descr);
IOSError IUSB_GetDevDescrAsync(s32 fd, USB_DevDescr* descr, USBCallback callback, void* callbackArg);

IOSError IUSB_SuspendDevice(s32 fd);
IOSError IUSB_ResumeDevice(s32 fd);
IOSError IUSB_ResetDevice(s32 fd);

IOSError IUSB_GetRhDesca(s32 fd, u32* desca);
IOSError IUSB_GetRhPortStatus(s32 fd, u8 port, u32* status);
IOSError IUSB_SetRhPortStatus(s32 fd, u8 port, u32 status);

IOSError IUSB_DeviceRemovalNotifyAsync(s32 fd, IOSIpcCb callback, void* callbackArg);
IOSError IUSB_IsoMsgAsync(s32 fd, u8 endpoint, IsoTransfer* xfer, USBIsoCallback callback, void* callbackArg);
IOSError IUSB_DeviceInsertionNotifyAsync(const char* path, u16 vid, u16 pid, USBCallback callback, void* callbackArg);
IOSError IUSB_DeviceClassInsertionNotifyAsync(const char* path, u8 devClass, USBCallback callback, void* callbackArg);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_USB_H
