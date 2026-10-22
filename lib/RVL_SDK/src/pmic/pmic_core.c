#include <private/pmic.h>

u32 PMIC_FIRM_DATA[0x17CC8 / 4] = {
#include "pmic_firmware.inc"
};

const char *__PMICVersion =
    "<< RVL_SDK - PMIC \trelease build: Sep 18 2008 10:07:23 (0x4199_60726) >>";

PMICRequest __cmd_node[20];
OSAlarm __boot_timeout;
OSAlarm __cmd_timeout;
OSAlarm __alarm;

// .sbss is laid out in reverse definition order.
s32 __fd;
USBCallback __cmd_usbfunc_cb;
s32 __retries;
OSThreadQueue __pmic_queue;
PMICCallback __func_user_cb;
s32 __func_user_arg;
u8 *__ctrl_buff;
u8 *__balk_buff;
u8 *__balk_data;
u32 __balk_size;
u8 *__pmic_firm_data;
u32 __pmic_firm_size;
IsoTransfer *__isoin_info[2];
IsoTransfer *__isoout_info[2];
s32 __iso_count;
s32 __isoin_unplug;
s32 __isoout_unplug;
PMICIsoCallback __iso_error_cb;
s32 __iso_error_arg;
PMICPacket *__zeros;
PMICPacket *__cmd;
u16 __cmd_no;
s32 __cmd_step;
s32 __cmd_error;
PMICRequest *__free_cmds;
PMICRequest *__post_cmds;
static PMICRequest *current;
s32 __hooked;
s32 __state;

static inline void PMICInitCommandPool(void) {
    s32 i;
    __free_cmds = __cmd_node;
    for (i = 0; i < 19; i++) {
        memset(&__cmd_node[i], 0, sizeof(PMICRequest));
        __cmd_node[i].next = &__cmd_node[i + 1];
        OSInitThreadQueue(&__cmd_node[i].queue);
    }
    memset(&__cmd_node[i], 0, sizeof(PMICRequest));
    __cmd_node[i].next = NULL;
    OSInitThreadQueue(&__cmd_node[i].queue);
}

/* 0x80225BC0 (0x2FC): brings the library up once, laying its transfer buffers
 * out in the caller's work area. */
s32 PMICInit(void *work) {
    u32 level;
    u8 *base;
    u8 *capIso1;
    IsoTransfer *capIso0;
    u8 *capData0;
    u8 *packet;
    u8 *playIso0;
    u8 *playIso1;
    u8 *playData1;
    u8 *playData0;
    u8 *capData1;
    u8 *playSizes1;
    u8 *playSizes0;
    u8 *capSizes1;
    u8 *capSizes0;
    s32 result;

    level = OSDisableInterrupts();
    if (__state == PMIC_STATE_FATAL) {
        OSRestoreInterrupts(level);
        return PMIC_ERR_FATAL;
    }
    if (__state == PMIC_STATE_INITIALIZING) {
        OSRestoreInterrupts(level);
        return PMIC_ERR_BUSY;
    }
    if (__state != PMIC_STATE_NONE) {
        OSRestoreInterrupts(level);
        return PMIC_ERR_NONE;
    }
    {
        __state = PMIC_STATE_INITIALIZING;
        OSRestoreInterrupts(level);
        OSRegisterVersion(__PMICVersion);
        if (work == NULL) {
            return PMIC_ERR_INVALID;
        }
        if (IPCCltInit() < 0) {
            OSReport("PMICInit() : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
        } else if (IUSB_OpenLib() < 0) {
            OSReport("PMICInit() : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
        } else {
            __pmic_firm_data = (u8 *)PMIC_FIRM_DATA;
            __pmic_firm_size = sizeof(PMIC_FIRM_DATA);
            __func_user_cb = NULL;
            __func_user_arg = 0;
            __iso_count = 0;
            __isoin_unplug = -1;
            __isoout_unplug = -1;
            __iso_error_cb = NULL;
            __iso_error_arg = 0;
            OSInitThreadQueue(&__pmic_queue);
            base = (u8 *)(((u32)work + 0x1F) & ~0x1F);
            __ctrl_buff = base;
            __balk_buff = base + 0x20;
            capIso0 = (IsoTransfer *)(base + 0x420);
            __isoin_info[0] = (IsoTransfer *)capIso0;
            capData0 = base + 0x440;
            capSizes0 = capData0 + PMIC_ISO_PACKETS * PMIC_ISO_IN_SIZE;
            capIso0->numPackets = PMIC_ISO_PACKETS;
            capIso1 = capSizes0 + 0x20;
            capData1 = capIso1 + 0x20;
            capSizes1 = capData1 + PMIC_ISO_PACKETS * PMIC_ISO_IN_SIZE;
            playIso0 = capSizes1 + 0x20;
            __isoin_info[0]->buf = capData0;
            playData0 = playIso0 + 0x20;
            playSizes0 = playData0 + PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE;
            playIso1 = playSizes0 + 0x20;
            playData1 = playIso1 + 0x20;
            __isoin_info[0]->packets = (u16 *)capSizes0;
            playSizes1 = playData1 + PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE;
            packet = playSizes1 + 0x20;
            __isoin_info[1] = (IsoTransfer *)capIso1;
            ((IsoTransfer *)capIso1)->numPackets = PMIC_ISO_PACKETS;
            __isoin_info[1]->buf = capData1;
            __isoin_info[1]->packets = (u16 *)capSizes1;
            __isoout_info[0] = (IsoTransfer *)playIso0;
            ((IsoTransfer *)playIso0)->numPackets = PMIC_ISO_PACKETS;
            __isoout_info[0]->buf = playData0;
            __isoout_info[0]->packets = (u16 *)playSizes0;
            __isoout_info[1] = (IsoTransfer *)playIso1;
            ((IsoTransfer *)playIso1)->numPackets = PMIC_ISO_PACKETS;
            __isoout_info[1]->buf = playData1;
            __isoout_info[1]->packets = (u16 *)playSizes1;
            __zeros = (PMICPacket *)packet;
            __PMICDataInit(packet + 0x20);
            __cmd = (PMICPacket *)__ctrl_buff;
            PMICInitCommandPool();
            __post_cmds = NULL;
            __cmd_usbfunc_cb = NULL;
            OSCreateAlarm(&__boot_timeout);
            OSCreateAlarm(&__cmd_timeout);
            level = OSDisableInterrupts();
            __cmd_step = PMIC_STEP_IDLE;
            OSCreateAlarm(&__alarm);
            OSSetAlarm(&__alarm, PMIC_POLL_PERIOD, __alarm_cb);
            __state = PMIC_STATE_NO_DEVICE;
            OSRestoreInterrupts(level);
        }
        result = PMIC_ERR_NONE;
        if (__state == PMIC_STATE_FATAL) {
            result = PMIC_ERR_FATAL;
        }
        return result;
    }
}

/* 0x80225EBC (0xEC): tears the library down when no transfer is in flight. */
s32 PMICQuit(void) {
    u32 level = OSDisableInterrupts();
    s32 result;
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state == PMIC_STATE_NONE) {
        result = PMIC_ERR_NONE;
    } else if (__state == PMIC_STATE_INITIALIZING ||
               __state == PMIC_STATE_ATTACHING || __state == PMIC_STATE_BUSY ||
               __state == PMIC_STATE_QUITTING) {
        result = PMIC_ERR_BUSY;
    } else if (__state >= PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        OSCancelAlarm(&__alarm);
        if (__state == PMIC_STATE_ATTACHED) {
            __state = PMIC_STATE_QUITTING;
            OSRestoreInterrupts(level);
            IUSB_ResetDevice(__fd);
            level = OSDisableInterrupts();
            IUSB_CloseDevice(__fd);
        }
        __PMICDataQuit();
        __state = PMIC_STATE_NONE;
        result = PMIC_ERR_NONE;
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x80225FA8 (0xD4): asks the USB stack to report the device when it is plugged
 * in. */
s32 PMICProbe(void) {
    u32 level = OSDisableInterrupts();
    s32 result;
    s32 status;
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state < PMIC_STATE_NO_DEVICE) {
        result = PMIC_ERR_STATE;
    } else {
        result = PMIC_ERR_NONE;
        if (__state < PMIC_STATE_ATTACHED) {
            result = PMIC_ERR_NO_DEVICE;
        }
        if (__hooked == 0) {
            __hooked = 1;
            status = IUSB_DeviceInsertionNotifyAsync(
                "/dev/usb/oh0", PMIC_USB_VENDOR_ID, PMIC_USB_PRODUCT_ID,
                __insert_cb, NULL);
            if (status < 0) {
                if (status == IPC_RESULT_INTR) {
                    __state = PMIC_STATE_FATAL;
                    result = PMIC_ERR_FATAL;
                } else {
                    __hooked = 0;
                    result = PMIC_ERR_RETRY;
                }
            }
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x8022607C (0x128): starts the firmware upload to the opened device and
 * reports the outcome to `callback`. */
s32 PMICOpenAsync(PMICCallback callback, s32 arg) {
    u32 level = OSDisableInterrupts();
    s32 result;
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state == PMIC_STATE_BUSY) {
        result = PMIC_ERR_BUSY;
    } else if (__state != PMIC_STATE_ATTACHED) {
        result = PMIC_ERR_STATE;
    } else {
        __func_user_cb = callback;
        __func_user_arg = arg;
        if (IUSB_WriteCtrlMsgAsync(__fd, 0x41, 0, 0, 0, 0, NULL,
                                   __confirm_not_ready, NULL) < 0) {
            __func_user_cb = NULL;
            result = PMIC_ERR_RETRY;
            __func_user_arg = 0;
        } else {
            __state = PMIC_STATE_BUSY;
            __cmd_usbfunc_cb = __confirm_not_ready;
            OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
            result = PMIC_ERR_NONE;
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x802261A4 (0x90): asks a running device to stop and reports the outcome to
 * `callback`. */
s32 PMICCloseAsync(PMICCallback callback, s32 arg) {
    s32 result;
    u32 level = OSDisableInterrupts();
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state <= PMIC_STATE_ATTACHED) {
        result = PMIC_ERR_STATE;
    } else if (__state == PMIC_STATE_BUSY || __state == PMIC_STATE_SWITCHING) {
        result = PMIC_ERR_BUSY;
    } else {
        __func_user_cb = callback;
        result = PMIC_ERR_NONE;
        __func_user_arg = arg;
        __state = PMIC_STATE_BUSY;
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x80226234 (0x114): queues the request that starts recording once the device
 * runs. */
s32 PMICStartAsync(PMICCallback callback, s32 arg) {
    PMICRequest *request;
    s32 result;
    u32 level = OSDisableInterrupts();
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state == PMIC_STATE_SWITCHING) {
        result = PMIC_ERR_BUSY;
    } else if (__state != PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        request = PMICPopFree();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            __state = PMIC_STATE_SWITCHING;
            request->command = PMIC_CMD_STATE;
            request->value0 = 1;
            request->done = __mute_off;
            request->callback = callback;
            request->callbackArg = arg;
            request->keep = 0;
            PMICPushQueue(request);
            result = PMIC_ERR_NONE;
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x80226348 (0x114): queues the request that stops recording while the device
 * runs. */
s32 PMICStopAsync(PMICCallback callback, s32 arg) {
    PMICRequest *request;
    s32 result;
    u32 level = OSDisableInterrupts();
    if (__state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (__state == PMIC_STATE_SWITCHING) {
        result = PMIC_ERR_BUSY;
    } else if (__state != PMIC_STATE_RECORDING) {
        result = PMIC_ERR_STATE;
    } else {
        request = PMICPopFree();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            __state = PMIC_STATE_SWITCHING;
            request->command = PMIC_CMD_MUTE;
            request->value0 = 1;
            request->done = __state_stop;
            request->callback = callback;
            request->callbackArg = arg;
            request->keep = 0;
            PMICPushQueue(request);
            result = PMIC_ERR_NONE;
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

#pragma dont_inline on

/* 0x8022645C (0x20): takes a request from the free list. */
PMICRequest *__PMICAllocCmd(void) {
    PMICRequest *request = NULL;
    if (__free_cmds != NULL) {
        request = __free_cmds;
        __free_cmds = __free_cmds->next;
    }
    return request;
}

/* 0x8022647C (0x88): reports a finished request to its callback and recycles
 * it. */
void __PMICCallUcbCb(s32 result, PMICRequest *request) {
    PMICCallback callback = request->callback;
    if (callback != NULL) {
        callback(result, request->callbackArg);
    }
    if (request->keep == 0) {
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
}

/* 0x80226504 (0xC): stores the stop result and wakes the thread waiting in the
 * synchronous call. */
void __PMICWakeupCb(s32 result, s32 arg) {
    PMICRequest *request = (PMICRequest *)arg;
    request->syncResult = result;
    OSWakeupThread(&request->queue);
}

/* 0x80226510 (0x44): appends a request to the queue. */
s32 __PMICControlAsync(PMICRequest *request) {
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
    return 0;
}

/* 0x80226554 (0xAC): queues a request and sleeps until its completion
 * callback wakes the caller, then recycles the request. */
s32 __PMICControlSync(PMICRequest *request) {
    PMICRequest *tail = __post_cmds;
    s32 result;
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
    OSSleepThread(&request->queue);
    result = request->syncResult;
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
    return result;
}

/* 0x80226600 (0x8): returns the library state. */
s32 __PMICGetState(void) {
    return __state;
}

#pragma dont_inline reset

/* 0x80226608 (0xFC): opens the device once the USB stack reports it. */
void __insert_cb(s32 result, void *arg) {
    if (__state != PMIC_STATE_ATTACHING && __state != PMIC_STATE_FATAL) {
        if ((u32)__state <= PMIC_STATE_INITIALIZING) {
            __hooked = 0;
            return;
        }
        if (result == IPC_RESULT_NOEXISTS) {
            __hooked = 0;
            return;
        }
        if (result < 0) {
            __state = PMIC_STATE_FATAL;
            return;
        }
        if (IUSB_OpenDeviceIdsAsync("oh0", PMIC_USB_VENDOR_ID,
                                    PMIC_USB_PRODUCT_ID, __open_cb, NULL) < 0) {
            OSReport("PMIC lib : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
            return;
        }
        __state = PMIC_STATE_ATTACHING;
        __cmd_usbfunc_cb = __open_cb;
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
    }
}

/* 0x80226704 (0x154): configures the opened device and starts the configuration
 * reads. */
void __open_cb(s32 result, void *arg) {
    if (__cmd_usbfunc_cb != NULL) {
        OSCancelAlarm(&__boot_timeout);
        __cmd_usbfunc_cb = NULL;
    }
    __fd = result;
    if (__state != PMIC_STATE_FATAL) {
        if (result == IPC_RESULT_NOEXISTS) {
            __state = PMIC_STATE_NO_DEVICE;
            __hooked = 0;
            return;
        }
        if (result < 0) {
            __state = PMIC_STATE_FATAL;
            return;
        }
        if (IUSB_DeviceRemovalNotifyAsync(result, (IOSIpcCb)__remove_cb, NULL) <
            0) {
            OSReport("PMIC lib : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
            return;
        }
        if (IUSB_WriteCtrlMsgAsync(__fd, 1, 0xB, 1, 0, 0, NULL, __get_interface,
                                   NULL) < 0) {
            OSReport("PMIC lib : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
            return;
        }
        __cmd_usbfunc_cb = __get_interface;
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
    }
}

/* 0x80226858 (0x150): reads the device's configuration back once it is set. */
void __get_interface(s32 result, void *arg) {
    if (__cmd_usbfunc_cb != NULL) {
        OSCancelAlarm(&__boot_timeout);
        __cmd_usbfunc_cb = NULL;
    }
    if (__state != PMIC_STATE_FATAL && __state != PMIC_STATE_NO_DEVICE &&
        result != IPC_RESULT_NOEXISTS && result != IPC_RESULT_INVALID) {
        if (result < 0) {
            OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
            return;
        }
        DCInvalidateRange(__ctrl_buff, 0x20);
        if (IUSB_ReadCtrlMsgAsync(__fd, 0x81, 0xA, 0, 0, 1, (char *)__ctrl_buff,
                                  __confirm_interface, NULL) < 0) {
            OSReport("PMIC lib : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
            return;
        }
        __cmd_usbfunc_cb = __confirm_interface;
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
    }
}

/* 0x802269A8 (0xDC): accepts the configuration the device reports and marks it
 * ready. */
void __confirm_interface(s32 result, void *arg) {
    if (__cmd_usbfunc_cb != NULL) {
        OSCancelAlarm(&__boot_timeout);
        __cmd_usbfunc_cb = NULL;
    }
    if (__state != PMIC_STATE_FATAL && __state != PMIC_STATE_NO_DEVICE &&
        result != IPC_RESULT_NOEXISTS && result != IPC_RESULT_INVALID) {
        if (result < 0) {
            OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
            return;
        }
        if (*__ctrl_buff != 1) {
            __state = PMIC_STATE_FATAL;
            return;
        }
        __state = PMIC_STATE_ATTACHED;
        __cmd_no = 0;
    }
}

/* 0x80226A84 (0x88): clears the rings when the device goes away and closes it.
 */
s32 __remove_cb(s32 result) {
    __PMICDataReadReset();
    __PMICDataWriteResetAll();
    if (__state != PMIC_STATE_FATAL) {
        if (__state == PMIC_STATE_QUITTING) {
            __hooked = 0;
        } else if (IUSB_CloseDeviceAsync(__fd, __close_cb, NULL) < 0) {
            OSReport("PMIC lib : too busy, can't work stably.\n");
            __state = PMIC_STATE_FATAL;
        }
    }
    return result;
}

/* 0x80226B0C (0xFC): finishes the removal and reports it to the waiting
 * callers. */
void __close_cb(s32 result, void *arg) {
    if (__state != PMIC_STATE_FATAL) {
        __state = PMIC_STATE_NO_DEVICE;
        __hooked = 0;
    }
    OSCancelAlarm(&__boot_timeout);
    if (__state == PMIC_STATE_NO_DEVICE) {
        if (__func_user_cb != NULL) {
            __func_user_cb(PMIC_ERR_NO_DEVICE, __func_user_arg);
        }
    } else {
        if (__func_user_cb != NULL) {
            __func_user_cb(PMIC_ERR_FATAL, __func_user_arg);
        }
        __state = PMIC_STATE_FATAL;
    }
    __func_user_cb = NULL;
    __func_user_arg = 0;
    OSCancelAlarm(&__cmd_timeout);
    if (__cmd_step == PMIC_STEP_RETRY_WAIT) {
        if (__state == PMIC_STATE_NO_DEVICE ||
            (u32)(__state - PMIC_STATE_ATTACHED) <= 1) {
            __cmd_step = PMIC_STEP_ERROR;
            __cmd_error = PMIC_ERR_NO_DEVICE;
            return;
        }
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_FATAL;
        __state = PMIC_STATE_FATAL;
    }
}

/* 0x80226C08 (0x1B8): reads the device status after the start command was
 * acknowledged. */
void __confirm_not_ready(s32 result, void *arg) {
    PMIC_CLEAR_OP_PENDING();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
        return;
    }
    DCInvalidateRange(__ctrl_buff, 0x20);
    if (IUSB_ReadCtrlMsgAsync(__fd, 0xC1, 6, 0, 0, 1, (char *)__ctrl_buff,
                              __start_dl_firm, NULL) < 0) {
        PMIC_REPORT_OP(PMIC_ERR_RETRY);
        __state = PMIC_STATE_ATTACHED;
        return;
    }
    __cmd_usbfunc_cb = __start_dl_firm;
    OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
}

/* 0x80226DC0 (0x238): sends the first firmware chunk once the device reports
 * itself idle. */
void __start_dl_firm(s32 result, void *arg) {
    u32 chunk;
    PMIC_CLEAR_OP_PENDING();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
        return;
    }
    if (*__ctrl_buff != 0) {
        PMIC_REPORT_OP(PMIC_ERR_DEVICE);
        __state = PMIC_STATE_FATAL;
        return;
    }
    chunk = PMIC_FIRM_CHUNK;
    __balk_data = __pmic_firm_data;
    __balk_size = __pmic_firm_size;
    if (__balk_size <= PMIC_FIRM_CHUNK) {
        chunk = __balk_size;
    }
    memcpy(__balk_buff, __balk_data, chunk);
    DCFlushRange(__balk_buff, chunk);
    if (IUSB_WriteBlkMsgAsync(__fd, PMIC_EP_FIRMWARE, chunk, __balk_buff,
                              __continue_dl_firm, NULL) < 0) {
        PMIC_REPORT_OP(PMIC_ERR_RETRY);
        __state = PMIC_STATE_ATTACHED;
        return;
    }
    __balk_data += chunk;
    __balk_size -= chunk;
    __cmd_usbfunc_cb = __continue_dl_firm;
    OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
}

/* 0x80226FF8 (0x2B4): sends the next firmware chunk, then polls the device
 * status once the image is out. */
void __continue_dl_firm(s32 result, void *arg) {
    u32 chunk;
    PMIC_CLEAR_OP_PENDING();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
        return;
    }
    if (__balk_size != 0) {
        chunk = PMIC_FIRM_CHUNK;
        if (__balk_size <= PMIC_FIRM_CHUNK) {
            chunk = __balk_size;
        }
        memcpy(__balk_buff, __balk_data, chunk);
        DCFlushRange(__balk_buff, chunk);
        if (IUSB_WriteBlkMsgAsync(__fd, PMIC_EP_FIRMWARE, chunk, __balk_buff,
                                  __continue_dl_firm, NULL) < 0) {
            PMIC_REPORT_OP(PMIC_ERR_RETRY);
            __state = PMIC_STATE_ATTACHED;
            return;
        }
        __balk_data += chunk;
        __balk_size -= chunk;
        __cmd_usbfunc_cb = __continue_dl_firm;
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
        return;
    }
    DCInvalidateRange(__ctrl_buff, 0x20);
    if (IUSB_ReadCtrlMsgAsync(__fd, 0xC1, 6, 0, 0, 1, (char *)__ctrl_buff,
                              __confirm_ready, NULL) < 0) {
        PMIC_REPORT_OP(PMIC_ERR_RETRY);
        __state = PMIC_STATE_ATTACHED;
        return;
    }
    __retries = 0;
    __cmd_usbfunc_cb = __confirm_ready;
    OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
}

/* 0x802272AC (0x284): polls the device until it reports itself running, then
 * starts the isochronous streams. */
void __confirm_ready(s32 result, void *arg) {
    PMIC_CLEAR_OP_PENDING();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
        return;
    }
    if (*__ctrl_buff == 0) {
        __retries++;
        if (__retries > 0x104) {
            PMIC_REPORT_OP(PMIC_ERR_FATAL);
            __state = PMIC_STATE_FATAL;
            return;
        }
        DCInvalidateRange(__ctrl_buff, 0x20);
        if (IUSB_ReadCtrlMsgAsync(__fd, 0xC1, 6, 0, 0, 1, (char *)__ctrl_buff,
                                  __confirm_ready, NULL) < 0) {
            PMIC_REPORT_OP(PMIC_ERR_RETRY);
            __state = PMIC_STATE_ATTACHED;
            return;
        }
        __cmd_usbfunc_cb = __confirm_ready;
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
        return;
    }
    if (__start_iso_transfer() != 0) {
        PMIC_REPORT_OP(PMIC_ERR_NONE);
        __PMICDataWriteResetAll();
        __state = PMIC_STATE_READY;
        return;
    }
    PMIC_REPORT_OP(PMIC_ERR_RETRY);
    __state = PMIC_STATE_ATTACHED;
}

/* 0x80227530 (0x2F8): sizes the isochronous packets, clears the playback data
 * and submits the four transfers. */
s32 __start_iso_transfer(void) {
    s32 ok = 1;
    s32 i;
    for (i = 0; i < PMIC_ISO_PACKETS; i++) {
        __isoin_info[0]->packets[i] = PMIC_ISO_IN_SIZE;
        __isoin_info[1]->packets[i] = PMIC_ISO_IN_SIZE;
        __isoout_info[0]->packets[i] = PMIC_ISO_OUT_SIZE;
        __isoout_info[1]->packets[i] = PMIC_ISO_OUT_SIZE;
    }
    memset(__isoout_info[0]->buf, 0, PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE);
    memset(__isoout_info[1]->buf, 0, PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE);
    DCFlushRange(__isoout_info[0]->buf, PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE);
    DCFlushRange(__isoout_info[1]->buf, PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE);
    if (IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_IN, __isoin_info[0], __isoin_cb,
                         NULL) < 0) {
        __iso_count = 0;
        ok = 0;
    } else if (IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_IN, __isoin_info[1],
                                __isoin_cb, NULL) < 0) {
        __iso_count = 1;
        ok = 0;
    } else if (IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_OUT, __isoout_info[0],
                                __isoout_cb, NULL) < 0) {
        __iso_count = 2;
        ok = 0;
    } else if (IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_OUT, __isoout_info[1],
                                __isoout_cb, NULL) < 0) {
        __iso_count = 3;
        ok = 0;
    } else {
        __iso_count = 4;
        __isoin_unplug = -1;
        __isoout_unplug = -1;
    }
    return ok;
}

/* 0x80227828 (0x160): clears the rings after the streams stopped and sends the
 * stop command. */
void __stop_iso_transfer(s32 result) {
    __PMICDataReadReset();
    __PMICDataWriteResetAll();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_INVALID ||
        result == IPC_RESULT_NOEXISTS || result == -0x1B6E) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (IUSB_WriteCtrlMsgAsync(__fd, 0x41, 0, 0, 0, 0, NULL, __close_device,
                               NULL) < 0) {
        PMIC_REPORT_OP(PMIC_ERR_RETRY);
        __state = PMIC_STATE_ATTACHED;
        return;
    }
    __cmd_usbfunc_cb = __close_device;
    OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
}

/* 0x80227988 (0x130): reports the stop once the device acknowledged it. */
void __close_device(s32 result, void *arg) {
    PMIC_CLEAR_OP_PENDING();
    if (__state == PMIC_STATE_FATAL) {
        PMIC_REPORT_OP(PMIC_ERR_FATAL);
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        PMIC_REPORT_OP(PMIC_ERR_NO_DEVICE);
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__boot_timeout, PMIC_TIMEOUT, __boot_errcheck);
        return;
    }
    PMIC_REPORT_OP(PMIC_ERR_NONE);
    __state = PMIC_STATE_ATTACHED;
}

/* 0x80227AB8 (0x2AC): takes the captured packets of one transfer, resubmits it
 * and tracks the stream errors. */
void __isoin_cb(s32 result, IsoTransfer *iso, void *arg) {
    s32 i;
    s32 offset;
    u16 word;
    s32 length;
    s32 status;
    s32 bad;
    s32 cancelled;
    s32 resubmitFailed;
    s32 tooMany;
    if (__state >= PMIC_STATE_READY && __iso_count == 4) {
        bad = 0;
        cancelled = 0;
        resubmitFailed = 0;
        tooMany = 0;
        if (result == 0 || result == -0x1B60 || result == -0x1B61 ||
            (u32)(result + 0x1B67) <= 1) {
            offset = 0;
            for (i = 0; i < PMIC_ISO_PACKETS; i++) {
                word = iso->packets[i];
                length = word & 0xFFF;
                status = (word >> 12) & 0xF;
                if ((length == PMIC_ISO_IN_SIZE && status == 0) ||
                    (length == 0x16 && status == 9) ||
                    (length == 0x18 && status == 9) ||
                    (length == 0x10 && status == 9)) {
                    __PMICDataRecvData((s16 *)((u8 *)iso->buf + offset),
                                       length);
                } else {
                    bad = 1;
                }
                offset += PMIC_ISO_IN_SIZE;
                iso->packets[i] = PMIC_ISO_IN_SIZE;
                if (status != 0 && status != 8 && status != 9 && status != 14 &&
                    status != 15) {
                    __isoin_unplug = 0;
                }
            }
        } else if (result == IPC_RESULT_NOEXISTS || result == -0x1B6E) {
            cancelled = 1;
        } else {
            if (__isoin_unplug < 0) {
                __isoin_unplug = 0;
            }
            bad = 1;
        }
        if (__isoin_unplug >= 0) {
            __isoin_unplug++;
            if (__isoin_unplug > 200) {
                tooMany = 1;
            }
        }
        if (cancelled == 0 && tooMany == 0 &&
            IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_IN, iso, __isoin_cb, NULL) < 0) {
            resubmitFailed = 1;
        }
        if (cancelled != 0 || resubmitFailed != 0 || tooMany != 0) {
            __iso_count--;
        }
        if (tooMany != 0) {
            PMIC_REPORT_ISO(PMIC_ERR_FATAL);
            __state = PMIC_STATE_FATAL;
        } else if (resubmitFailed != 0) {
            PMIC_REPORT_ISO(PMIC_ERR_RETRY);
            __state = PMIC_STATE_ATTACHED;
        } else if (cancelled != 0) {
            __state = PMIC_STATE_BUSY;
        } else if (bad != 0) {
            PMIC_REPORT_ISO(PMIC_ERR_STREAM);
        }
    } else {
        __iso_count--;
    }
    if (__iso_count == 0) {
        __stop_iso_transfer(result);
    }
}

/* 0x80227D64 (0x344): fills the playback packets of one transfer, resubmits it
 * and tracks the stream errors. */
void __isoout_cb(s32 result, IsoTransfer *iso, void *arg) {
    s32 i;
    s32 offset;
    u16 word;
    s32 status;
    s32 bad;
    s32 cancelled;
    s32 resubmitFailed;
    s32 tooMany;
    PMICRequest *request;
    if (__state >= PMIC_STATE_READY && __iso_count == 4) {
        bad = 0;
        cancelled = 0;
        resubmitFailed = 0;
        tooMany = 0;
        if (result == 0 || result == -0x1B60 || result == -0x1B61 ||
            (u32)(result + 0x1B67) <= 1) {
            offset = 0;
            for (i = 0; i < PMIC_ISO_PACKETS; i++) {
                word = iso->packets[i];
                status = (word >> 12) & 0xF;
                if (__PMICDataSendData((s16 *)((u8 *)iso->buf + offset),
                                       PMIC_ISO_OUT_SIZE) == 0) {
                    bad = 1;
                }
                offset += PMIC_ISO_OUT_SIZE;
                iso->packets[i] = PMIC_ISO_OUT_SIZE;
                if (status != 0) {
                    if (status != 8 && status != 9 && status != 14 &&
                        status != 15) {
                        __isoout_unplug = 0;
                    }
                    bad = 1;
                }
            }
        } else if (result == IPC_RESULT_NOEXISTS || result == -0x1B6E) {
            cancelled = 1;
        } else {
            if (__isoout_unplug < 0) {
                __isoout_unplug = 0;
            }
            bad = 1;
        }
        if (__isoout_unplug >= 0) {
            __isoout_unplug++;
            if (__isoout_unplug > 200) {
                tooMany = 1;
            }
        }
        if (cancelled == 0 && tooMany == 0) {
            iso->numPackets = PMIC_ISO_PACKETS;
            DCFlushRange(iso->buf, PMIC_ISO_PACKETS * PMIC_ISO_OUT_SIZE);
            if (IUSB_IsoMsgAsync(__fd, PMIC_EP_ISO_OUT, iso, __isoout_cb,
                                 NULL) < 0) {
                resubmitFailed = 1;
            }
        }
        if (cancelled != 0 || resubmitFailed != 0 || tooMany != 0) {
            __iso_count--;
        }
        if (tooMany != 0) {
            PMIC_REPORT_ISO(PMIC_ERR_FATAL);
            __state = PMIC_STATE_FATAL;
        } else if (resubmitFailed != 0) {
            PMIC_REPORT_ISO(PMIC_ERR_RETRY);
            __state = PMIC_STATE_ATTACHED;
        } else if (cancelled != 0) {
            __state = PMIC_STATE_BUSY;
        } else if (bad != 0) {
            request = PMICPopFree();
            if (request != NULL) {
                request->command = PMIC_CMD_HCLR;
                request->value0 = 1;
                request->done = __hclr_off;
                request->callback = NULL;
                request->callbackArg = 0;
                request->keep = 0;
                request->retries = 0;
                PMICPushQueue(request);
                __PMICDataWriteReset();
                PMIC_REPORT_ISO(PMIC_ERR_STREAM);
            } else {
                PMIC_REPORT_ISO(PMIC_ERR_RETRY);
                __state = PMIC_STATE_ATTACHED;
            }
        }
    } else {
        __iso_count--;
    }
    if (__iso_count == 0) {
        __stop_iso_transfer(result);
    }
}

/* 0x802280A8 (0x518): the periodic task that runs the request machine: sends a
 * queued request, reads its answer and completes it. */
void __alarm_cb(OSAlarm *alarm, OSContext *context) {
    PMICRequest *request;
    PMICCallback callback;
    PMICRequestDoneFn done;
    u16 *out;
    if (__cmd_step == PMIC_STEP_IDLE) {
        request = PMICPopQueue();
        current = request;
        if (__state < PMIC_STATE_READY) {
            while (request != NULL) {
                callback = request->callback;
                if (callback != NULL) {
                    if (__state == PMIC_STATE_FATAL) {
                        callback(PMIC_ERR_FATAL, request->callbackArg);
                    } else {
                        callback(PMIC_ERR_STATE, request->callbackArg);
                    }
                }
                if (current->keep == 0) {
                    PMICPushFree(current);
                }
                request = PMICPopQueue();
                current = request;
            }
        } else if (request != NULL) {
            __cmd->command = request->command;
            __cmd->value0 = request->value0;
            __cmd->value1 = request->value1;
            __cmd->value2 = request->value2;
            __cmd->sequence = __cmd_no++;
            DCFlushRange(__cmd, 0xA);
            if (IUSB_WriteCtrlMsgAsync(__fd, 0x41, 1, 0, 0, 0xA, __cmd,
                                       __write_cb, NULL) < 0) {
                done = current->done;
                if (done != NULL) {
                    done(PMIC_ERR_RETRY, current);
                }
            } else {
                __cmd_step = PMIC_STEP_WRITING;
                __cmd_usbfunc_cb = __write_cb;
                OSSetAlarm(&__cmd_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
            }
        }
    } else if (__cmd_step == PMIC_STEP_WRITTEN) {
        if (__state < PMIC_STATE_READY) {
            callback = current->callback;
            if (callback != NULL) {
                if (__state == PMIC_STATE_FATAL) {
                    callback(PMIC_ERR_FATAL, current->callbackArg);
                } else {
                    callback(PMIC_ERR_STATE, current->callbackArg);
                }
            }
            if (current->keep == 0) {
                PMICPushFree(current);
            }
            __cmd_step = PMIC_STEP_IDLE;
        } else {
            DCInvalidateRange(__cmd, 0xA);
            if (IUSB_ReadCtrlMsgAsync(__fd, 0xC1, 2, 0, 0, 0xA, (char *)__cmd,
                                      __read_cb, NULL) < 0) {
                done = current->done;
                if (done != NULL) {
                    done(PMIC_ERR_RETRY, current);
                }
                __cmd_step = PMIC_STEP_IDLE;
            } else {
                __cmd_step = PMIC_STEP_READING;
                __cmd_usbfunc_cb = __read_cb;
                OSSetAlarm(&__cmd_timeout, PMIC_TIMEOUT, __cmd_timeout_cb);
            }
        }
    } else if (__cmd_step == PMIC_STEP_READ) {
        request = current;
        request->value0 = __cmd->value0;
        request->value1 = __cmd->value1;
        request->value2 = __cmd->value2;
        out = (u16 *)request->reply;
        if (out != NULL) {
            u16 command = request->command;
            if (command == PMIC_CMD_GET(PMIC_CMD_STATE) ||
                command == PMIC_CMD_GET(PMIC_CMD_SET_MODE) ||
                command == 0x9A07 || command == 0x9A09 || command == 0x9A0B ||
                command == PMIC_CMD_GET(PMIC_CMD_MUTE) || command == 0x9A0F ||
                command == 0x9A11 || command == 0x9A13 ||
                command == PMIC_CMD_GET(PMIC_CMD_HCLR) || command == 0x9A33) {
                *out = request->value0;
            } else if (command == PMIC_CMD_GET(PMIC_CMD_SET_GAIN)) {
                *out = (u8)request->value0;
            } else if (command == 0x9A35 || command == PMIC_CMD_GET_PARAM) {
                *out = request->value1;
            } else if (command == 0x9A37 || command == 0x9A3B) {
                *(u32 *)out = (request->value1 << 16) | request->value2;
            } else if (command == 0x9A3D) {
                *out = request->value2;
            }
        }
        done = request->done;
        if (done != NULL) {
            done(PMIC_ERR_NONE, request);
        }
        __cmd_step = PMIC_STEP_IDLE;
    } else if (__cmd_step == PMIC_STEP_ERROR) {
        if (current != NULL) {
            done = current->done;
            if (done != NULL) {
                done(__cmd_error, current);
            }
        }
        __cmd_step = PMIC_STEP_IDLE;
    }
    OSSetAlarm(alarm, PMIC_POLL_PERIOD, __alarm_cb);
}

/* 0x802285C0 (0xE0): moves the request machine on once the command packet was
 * written. */
void __write_cb(s32 result, void *arg) {
    if (__cmd_usbfunc_cb != NULL) {
        OSCancelAlarm(&__cmd_timeout);
        __cmd_usbfunc_cb = NULL;
    }
    if (__state == PMIC_STATE_FATAL) {
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_FATAL;
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_NO_DEVICE;
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__cmd_timeout, PMIC_TIMEOUT, __cmd_errcheck);
        __cmd_step = PMIC_STEP_RETRY_WAIT;
        return;
    }
    __cmd_step = PMIC_STEP_WRITTEN;
}

/* 0x802286A0 (0xE0): moves the request machine on once the answer packet was
 * read. */
void __read_cb(s32 result, void *arg) {
    if (__cmd_usbfunc_cb != NULL) {
        OSCancelAlarm(&__cmd_timeout);
        __cmd_usbfunc_cb = NULL;
    }
    if (__state == PMIC_STATE_FATAL) {
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_FATAL;
        return;
    }
    if (__state == PMIC_STATE_NO_DEVICE || result == IPC_RESULT_NOEXISTS) {
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_NO_DEVICE;
        return;
    }
    if (result < 0) {
        OSSetAlarm(&__cmd_timeout, PMIC_TIMEOUT, __cmd_errcheck);
        __cmd_step = PMIC_STEP_RETRY_WAIT;
        return;
    }
    __cmd_step = PMIC_STEP_READ;
}

/* 0x80228780 (0x104): chains the second command of the recording start, or
 * reports the failure. */
void __mute_off(s32 result, PMICRequest *request) {
    PMICCallback callback;
    if (result == 0) {
        request->command = PMIC_CMD_MUTE;
        request->value0 = 0;
        request->done = __started;
        request->keep = 0;
        PMICPushQueue(request);
        return;
    }
    callback = request->callback;
    if (callback != NULL) {
        callback(result, request->callbackArg);
    }
    if (request->keep == 0) {
        PMICPushFree(request);
    }
    if (__state == PMIC_STATE_SWITCHING) {
        __state = PMIC_STATE_READY;
    }
}

/* 0x80228884 (0xC0): finishes the recording start and clears the capture ring.
 */
void __started(s32 result, PMICRequest *request) {
    PMICCallback callback = request->callback;
    if (callback != NULL) {
        callback(result, request->callbackArg);
    }
    if (request->keep == 0) {
        PMICPushFree(request);
    }
    if (__state == PMIC_STATE_SWITCHING) {
        if (result == 0) {
            __PMICDataReadReset();
            __state = PMIC_STATE_RECORDING;
            return;
        }
        __state = PMIC_STATE_READY;
    }
}

/* 0x80228944 (0x104): chains the second command of the recording stop, or
 * reports the failure. */
void __state_stop(s32 result, PMICRequest *request) {
    PMICCallback callback;
    if (result == 0) {
        request->command = PMIC_CMD_STATE;
        request->value0 = 0;
        request->done = __stopped;
        request->keep = 0;
        PMICPushQueue(request);
        return;
    }
    callback = request->callback;
    if (callback != NULL) {
        callback(result, request->callbackArg);
    }
    if (request->keep == 0) {
        PMICPushFree(request);
    }
    if (__state == PMIC_STATE_SWITCHING) {
        __state = PMIC_STATE_RECORDING;
    }
}

/* 0x80228A48 (0xB4): finishes the recording stop. */
void __stopped(s32 result, PMICRequest *request) {
    PMICCallback callback = request->callback;
    if (callback != NULL) {
        callback(result, request->callbackArg);
    }
    if (request->keep == 0) {
        PMICPushFree(request);
    }
    if (__state == PMIC_STATE_SWITCHING) {
        s32 next = PMIC_STATE_RECORDING;
        if (result == 0) {
            next = PMIC_STATE_READY;
        }
        __state = next;
    }
}

/* 0x80228AFC (0x174): issues the follow-up query command, retrying a busy
 * device up to 100 times. */
void __hclr_off(s32 result, PMICRequest *request) {
    if (result == 0) {
        request->command = PMIC_CMD_HCLR;
        request->value0 = 0;
        request->done = __hclred;
        request->keep = 0;
        request->retries = 0;
        PMICPushQueue(request);
        return;
    }
    if (result == PMIC_ERR_RETRY) {
        s32 retries = request->retries;
        if (retries > 100) {
            PMICPushFree(request);
            __state = PMIC_STATE_FATAL;
            return;
        }
        request->command = PMIC_CMD_HCLR;
        request->value0 = 1;
        request->done = __hclr_off;
        request->keep = 0;
        request->retries = retries + 1;
        PMICPushQueue(request);
        return;
    }
    PMICPushFree(request);
}

/* 0x80228C70 (0x104): repeats the query command while the device is busy. */
void __hclred(s32 result, PMICRequest *request) {
    if (result == PMIC_ERR_RETRY) {
        s32 retries = request->retries;
        if (retries > 100) {
            PMICPushFree(request);
            __state = PMIC_STATE_FATAL;
            return;
        }
        request->command = PMIC_CMD_HCLR;
        request->value0 = 0;
        request->done = __hclred;
        request->keep = 0;
        request->retries = retries + 1;
        PMICPushQueue(request);
        return;
    }
    PMICPushFree(request);
}

/* 0x80228D74 (0x30): fails the library when the pending request does not answer
 * in time. */
void __cmd_timeout_cb(OSAlarm *alarm, OSContext *context) {
    USBCallback pending = __cmd_usbfunc_cb;
    __state = PMIC_STATE_FATAL;
    if (pending == NULL) {
        return;
    }
    __cmd_usbfunc_cb = NULL;
    pending(0, NULL);
}

/* 0x80228DA4 (0x8C): reports a failed operation after the retry delay. */
void __boot_errcheck(OSAlarm *alarm, OSContext *context) {
    if (alarm == NULL) {
        OSCancelAlarm(&__boot_timeout);
    }
    if (__state == PMIC_STATE_NO_DEVICE) {
        if (__func_user_cb != NULL) {
            __func_user_cb(PMIC_ERR_NO_DEVICE, __func_user_arg);
        }
    } else {
        if (__func_user_cb != NULL) {
            __func_user_cb(PMIC_ERR_FATAL, __func_user_arg);
        }
        __state = PMIC_STATE_FATAL;
    }
    __func_user_cb = NULL;
    __func_user_arg = 0;
}

/* 0x80228E30 (0x80): reports a failed request after the retry delay. */
void __cmd_errcheck(OSAlarm *alarm, OSContext *context) {
    if (alarm == NULL) {
        OSCancelAlarm(&__cmd_timeout);
    }
    if (__cmd_step == PMIC_STEP_RETRY_WAIT) {
        if (__state == PMIC_STATE_NO_DEVICE ||
            (u32)(__state - PMIC_STATE_ATTACHED) <= 1) {
            __cmd_step = PMIC_STEP_ERROR;
            __cmd_error = PMIC_ERR_NO_DEVICE;
            return;
        }
        __cmd_step = PMIC_STEP_ERROR;
        __cmd_error = PMIC_ERR_FATAL;
        __state = PMIC_STATE_FATAL;
    }
}
