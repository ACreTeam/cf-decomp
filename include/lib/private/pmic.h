#ifndef PRIVATE_PMIC_H
#define PRIVATE_PMIC_H

// PMIC (Wii Speak USB microphone) library internals shared by pmic_core.c,
// pmic_data.c and pmic_ctrl.c. Ported from mhtri-decomp's PMIC/pmic.c (a 2009
// revision); CF links the Sep 18 2008 build. Names come from the game's strings
// first (PMICInit), then the Rock Band 3 map (pmic_core.o / pmic_data.o,
// PMICSetGain); the rest are named from what the code does.

#include <private/ios.h>
#include <private/ipc.h>
#include <private/usb.h>
#include <revolution/os.h>
#include <revolution/pmic.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// __state: the library state.
enum {
    PMIC_STATE_FATAL = -3,       // failed; only PMICQuit is accepted
    PMIC_STATE_NONE = 0,         // PMICInit not called
    PMIC_STATE_INITIALIZING = 1, // PMICInit running
    PMIC_STATE_QUITTING = 2,     // PMICQuit closing the device
    PMIC_STATE_NO_DEVICE = 3,    // waiting for the microphone to be plugged in
    PMIC_STATE_ATTACHING = 4, // plugged in: opening and configuring the device
    PMIC_STATE_ATTACHED = 5,  // device open; firmware not running
    PMIC_STATE_BUSY = 6,      // firmware download or shutdown in progress
    PMIC_STATE_READY = 7,     // firmware running and streaming, not recording
    PMIC_STATE_SWITCHING = 8, // a start or stop request is in flight
    PMIC_STATE_RECORDING = 9,
};

// __cmd_step: where the request machine (__alarm_cb) is with the current
// request.
enum {
    PMIC_STEP_ERROR = -1,     // failed: complete the request with __cmd_error
    PMIC_STEP_IDLE = 0,       // take the next queued request
    PMIC_STEP_WRITING = 1,    // command packet being written
    PMIC_STEP_WRITTEN = 2,    // written: read the answer
    PMIC_STEP_READING = 3,    // answer packet being read
    PMIC_STEP_READ = 4,       // answer read: complete the request
    PMIC_STEP_RETRY_WAIT = 5, // a transfer failed: waiting in __cmd_errcheck
};

// Device commands (PMICRequest::command). Odd commands read back the setting of
// the even one below them.
enum {
    PMIC_CMD_STATE = 0x9A00, // value0: 1 = run the capture stage, 0 = stop it
    PMIC_CMD_SET_MODE = 0x9A02,  // value0: mode 0-3
    PMIC_CMD_SET_GAIN = 0x9A04,  // value0: 0x300 | gain step 0-3
    PMIC_CMD_MUTE = 0x9A0C,      // value0: 1 = mute, 0 = unmute
    PMIC_CMD_HCLR = 0x9A14,      // value0: 1 then 0 after playback errors
                                 // (__hclr_off / __hclred)
    PMIC_CMD_SET_PARAM = 0x9A38, // value0: parameter, value1: value
    PMIC_CMD_GET_PARAM = 0x9A39,
};
#define PMIC_CMD_GET(command) ((command) | 1)

// The microphone on the USB bus.
#define PMIC_USB_VENDOR_ID 0x57E
#define PMIC_USB_PRODUCT_ID 0x308
#define PMIC_EP_FIRMWARE 2     // bulk out: firmware download
#define PMIC_EP_ISO_OUT 3      // isochronous out: playback (echo reference)
#define PMIC_EP_ISO_IN 0x81    // isochronous in: capture
#define PMIC_ISO_PACKETS 8     // packets per isochronous transfer
#define PMIC_ISO_IN_SIZE 0x20  // bytes per capture packet
#define PMIC_ISO_OUT_SIZE 0x40 // bytes per playback packet
#define PMIC_FIRM_CHUNK 0x400  // firmware download block size

#define PMIC_TIMEOUT OSMillisecondsToTicks(2000) // USB transfer timeout
#define PMIC_POLL_PERIOD OSMicrosecondsToTicks(1050)

/* size: 0x34 - one queued control request: the command word, its operands and
 * the completion callbacks. */
typedef struct PMICRequest PMICRequest;
typedef void (*PMICRequestDoneFn)(s32 result, PMICRequest *request);

struct PMICRequest {
    /* +0x00 */ PMICRequest *next;
    /* +0x04 */ u16 command;
    /* +0x06 */ u16 value0;
    /* +0x08 */ u16 value1;
    /* +0x0A */ u16 value2;
    /* +0x0C */ u16 reserved; // only ever cleared
    /* +0x0E */ u8 pad_0x0E[2];
    /* +0x10 */ s32 syncResult; // result handed to a __PMICControlSync caller
    /* +0x14 */ u32 *reply; // where a read command stores the device's answer
    /* +0x18 */ PMICRequestDoneFn done;
    /* +0x1C */ PMICCallback callback;
    /* +0x20 */ s32 callbackArg;
    /* +0x24 */ u8
        keep; // don't return the request to the free list when it completes
    /* +0x25 */ u8 pad_0x25[3];
    /* +0x28 */ OSThreadQueue queue; // a __PMICControlSync caller sleeps here
    /* +0x30 */ s32 retries;
}; /* size: 0x34 */

/* size: 0x0A - the control packet that carries a request to the device and its
 * answer. */
typedef struct PMICPacket {
    /* +0x00 */ u16 command;
    /* +0x02 */ u16 value0;
    /* +0x04 */ u16 value1;
    /* +0x06 */ u16 value2;
    /* +0x08 */ u16 sequence;
} PMICPacket;

/* size: 0x40 - the capture ring (+0x00) and the playback ring (+0x18) with
 * their counters. */
typedef struct PMICRings {
    /* +0x00 */ s32 captureCapacity;
    /* +0x04 */ s32 captureCount;
    /* +0x08 */ s32 captureWrite;
    /* +0x0C */ s32 captureRead;
    /* +0x10 */ s16 *captureBuffer;
    /* +0x14 */ s32 captureOverflow;
    /* +0x18 */ s32 playCapacity;
    /* +0x1C */ s32 playCount;
    /* +0x20 */ s32 playWrite;
    /* +0x24 */ s32 playRead;
    /* +0x28 */ s32 playPrimed;
    /* +0x2C */ s16 *playBuffer;
    /* +0x30 */ s32 playUnderrun;
    /* +0x34 */ s32 playOverrun;
    /* +0x38 */ s32 playFault;
    /* +0x3C */ u8 pad_0x3C[4];
} PMICRings;

/* size: 0x10 - PMICConvertOutput's state: the decimation filter's input history
 * for each channel. */
typedef struct PMICScratch {
    /* +0x00 */ s32 mode;    // the mode of the previous call
    /* +0x04 */ s32 histLen; // samples of history carried into the next call
    /* +0x08 */ u8 *histRight;
    /* +0x0C */ u8 *histLeft;
} PMICScratch;

typedef void (*PMICIsoCallback)(s32 result, s32 arg);

// pmic_core.c
extern u32 PMIC_FIRM_DATA[0x17CC8 / 4];
extern const char *__PMICVersion;
extern PMICRequest __cmd_node[20];
extern OSAlarm __boot_timeout;
extern OSAlarm __cmd_timeout;
extern OSAlarm __alarm;
extern s32 __state;
extern s32 __hooked;
extern PMICRequest *__post_cmds;
extern PMICRequest *__free_cmds;
extern s32 __cmd_error;
extern s32 __cmd_step;
extern u16 __cmd_no;
extern PMICPacket *__cmd;
extern PMICPacket *__zeros;
extern s32 __iso_error_arg;
extern PMICIsoCallback __iso_error_cb;
extern s32 __isoout_unplug;
extern s32 __isoin_unplug;
extern s32 __iso_count;
extern IsoTransfer *__isoout_info[2];
extern IsoTransfer *__isoin_info[2];
extern u32 __pmic_firm_size;
extern u8 *__pmic_firm_data;
extern u32 __balk_size;
extern u8 *__balk_data;
extern u8 *__balk_buff;
extern u8 *__ctrl_buff;
extern s32 __func_user_arg;
extern PMICCallback __func_user_cb;
extern OSThreadQueue __pmic_queue;
extern s32 __retries;
extern USBCallback __cmd_usbfunc_cb;
extern s32 __fd;

// pmic_data.c
extern PMICRings __iinfo;
extern PMICScratch __sinfo;

/* Pops a request from the free list. */
static inline PMICRequest *PMICPopFree(void) {
    PMICRequest *request = NULL;
    if (__free_cmds != NULL) {
        request = __free_cmds;
        __free_cmds = __free_cmds->next;
    }
    return request;
}

/* Appends a request to the queue. */
static inline void PMICPushQueue(PMICRequest *request) {
    PMICRequest *tail = __post_cmds;
    if (tail == NULL) {
        __post_cmds = request;
        request->next = NULL;
    } else {
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = request;
        request->next = NULL;
    }
}

/* Returns a request to the free list once its owner is done with it. */
static inline void PMICPushFree(PMICRequest *request) {
    request->command = 0;
    request->value0 = 0;
    request->value1 = 0;
    request->value2 = 0;
    request->reserved = 0;
    request->syncResult = 0;
    request->reply = NULL;
    request->done = NULL;
    request->callback = NULL;
    request->callbackArg = 0;
    request->keep = 0;
    request->next = __free_cmds;
    __free_cmds = request;
}

/* Reports a result to the operation callback and clears it. */
#define PMIC_REPORT_OP(result)                                                 \
    do {                                                                       \
        if (__func_user_cb != NULL) {                                          \
            __func_user_cb((result), __func_user_arg);                         \
        }                                                                      \
        __func_user_cb = NULL;                                                 \
        __func_user_arg = 0;                                                   \
    } while (0)

void __insert_cb(s32 result, void *arg);
void __open_cb(s32 result, void *arg);
void __get_interface(s32 result, void *arg);
void __confirm_interface(s32 result, void *arg);
s32 __remove_cb(s32 result);
void __close_cb(s32 result, void *arg);
void __confirm_not_ready(s32 result, void *arg);
void __close_device(s32 result, void *arg);
s32 __start_iso_transfer(void);
void __stop_iso_transfer(s32 result);
void __isoin_cb(s32 result, IsoTransfer *iso, void *arg);
void __isoout_cb(s32 result, IsoTransfer *iso, void *arg);
void __start_dl_firm(s32 result, void *arg);
void __continue_dl_firm(s32 result, void *arg);
void __confirm_ready(s32 result, void *arg);
void __cmd_timeout_cb(OSAlarm *alarm, OSContext *context);
void __boot_errcheck(OSAlarm *alarm, OSContext *context);
void __cmd_errcheck(OSAlarm *alarm, OSContext *context);
void __alarm_cb(OSAlarm *alarm, OSContext *context);
void __PMICDataQuit(void);
void __PMICDataReadReset(void);
void __PMICDataWriteReset(void);
void __PMICDataWriteResetAll(void);
u8 *__PMICDataInit(u8 *area);
void __PMICWakeupCb(s32 result, s32 arg);
s32 __PMICControlSync(PMICRequest *request);
void __mute_off(s32 result, PMICRequest *request);
void __state_stop(s32 result, PMICRequest *request);
void __started(s32 result, PMICRequest *request);
void __stopped(s32 result, PMICRequest *request);
void __hclr_off(s32 result, PMICRequest *request);
void __hclred(s32 result, PMICRequest *request);
void __write_cb(s32 result, void *arg);
void __read_cb(s32 result, void *arg);
s32 __PMICDataRecvData(s16 *src, s32 bytes);
s32 __PMICDataSendData(s16 *dst, s32 bytes);
PMICRequest *__PMICAllocCmd(void);
void __PMICCallUcbCb(s32 result, PMICRequest *request);
s32 __PMICControlAsync(PMICRequest *request);
s32 __PMICGetState(void);

/* Cancels the operation timeout once the pending USB call has answered. */
#define PMIC_CLEAR_OP_PENDING()                                                \
    do {                                                                       \
        if (__cmd_usbfunc_cb != NULL) {                                        \
            OSCancelAlarm(&__boot_timeout);                                    \
            __cmd_usbfunc_cb = NULL;                                           \
        }                                                                      \
    } while (0)

/* Reports a result to the iso callback. */
#define PMIC_REPORT_ISO(result)                                                \
    do {                                                                       \
        if (__iso_error_cb != NULL) {                                          \
            __iso_error_cb((result), __iso_error_arg);                         \
        }                                                                      \
    } while (0)

/* Pops the oldest request from the queue. */
static inline PMICRequest *PMICPopQueue(void) {
    PMICRequest *request = NULL;
    if (__post_cmds != NULL) {
        request = __post_cmds;
        __post_cmds = __post_cmds->next;
    }
    return request;
}

#ifdef __cplusplus
}
#endif

#endif // PRIVATE_PMIC_H
