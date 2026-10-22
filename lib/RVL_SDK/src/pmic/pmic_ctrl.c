#include <private/pmic.h>

// Device control and echo cancellation. CF keeps these in one unit (RB3's 2009
// build splits pmic_level.o and pmic_gain.o out). Only PMICSetGain is named by
// RB3; PMICSetModeAsync, PMICSetParamAsync and the echo names are from what the
// code does.

f32 PMICEchoPreset0[8] = {
    0.6666f,       0.014252141f, 0.5011872f, 0.1000061f,
    0.0005493164f, 0.5011872f,   0.1000061f, 0.0005493164f,
};
f32 PMICEchoPreset1[8] = {
    0.2f, 0.03f, 0.217f, 0.23f, 0.0005493164f, 0.217f, 0.23f, 0.0005493164f,
};
f32 PMICEchoPreset2[8] = {
    0.2f, 0.03f, 0.183f, 0.27f, 0.0005493164f, 0.183f, 0.27f, 0.0005493164f,
};
f32 PMICEchoPreset3[8] = {
    0.2f, 0.03f, 0.25f, 0.2f, 0.2f, 0.25f, 0.2f, 0.2f,
};
f32 PMICEchoPreset4[8] = {
    0.2f, 0.03f, 0.217f, 0.23f, 0.23f, 0.217f, 0.23f, 0.23f,
};
f32 PMICEchoPreset5[8] = {
    0.2f, 0.03f, 0.183f, 0.27f, 0.27f, 0.183f, 0.27f, 0.27f,
};
f32 PMICEchoPreset6[8] = {
    0.2f, 0.03f, 0.15f, 0.3f, 0.0005493164f, 0.15f, 0.3f, 0.0005493164f,
};

f32 __echo_ratio = 0.6666f;
f32 __echo_threshold = 0.014252141f;
f32 __mic_gain_min = 0.5011872f;
f32 __mic_gain_up = 0.1000061f;
f32 __mic_gain_down = 0.0005493164f;
f32 __spk_gain_min = 0.5011872f;
f32 __spk_gain_up = 0.1000061f;
f32 __spk_gain_down = 0.0005493164f;
f32 __gain_target = 0.5011872f;

s32 __spk_active;
s32 __near_talk;
s32 __far_talk;
s32 __spk_duck;
f32 __mic_gain;
f32 __spk_gain;
f32 __spk_level;
f32 __mic_level;

/* 0x80229A2C (0xD8): queues the command that sets one of four device modes. */
s32 PMICSetModeAsync(u16 mode, PMICCallback callback, s32 arg) {
    u32 level;
    PMICRequest *request;
    s32 state;
    s32 result;
    level = OSDisableInterrupts();
    state = __PMICGetState();
    if (state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (mode > 3) {
        result = PMIC_ERR_INVALID;
    } else if (state < PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        request = __PMICAllocCmd();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            request->command = PMIC_CMD_SET_MODE;
            request->value0 = mode;
            request->done = __PMICCallUcbCb;
            request->callback = callback;
            request->callbackArg = arg;
            request->keep = 0;
            result = __PMICControlAsync(request);
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x80229B04 (0xE0): queues the 'set parameter' command (0x9A38) that turns
 * device parameter 0x1010 on or off. */
s32 PMICSetParamAsync(u16 value, PMICCallback callback, s32 arg) {
    u32 level;
    PMICRequest *request;
    s32 state;
    s32 result;
    level = OSDisableInterrupts();
    state = __PMICGetState();
    if (state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (value > 1) {
        result = PMIC_ERR_INVALID;
    } else if (state < PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        request = __PMICAllocCmd();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            request->command = PMIC_CMD_SET_PARAM;
            request->value0 = 0x1010;
            request->value1 = value;
            request->done = __PMICCallUcbCb;
            request->callback = callback;
            request->callbackArg = arg;
            request->keep = 0;
            result = __PMICControlAsync(request);
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

#pragma fp_contract off

/* 0x80229BE4 (0x30C): half-duplex echo suppression over `samples` sample pairs.
 * Follows the mic and speaker levels; while the far end (speaker) dominates it
 * lowers the mic gain, and it eases the speaker gain toward
 * __gain_target / mic gain. Scales both buffers in place and returns whether
 * the speaker gain was rising for at least 80% of the block. */
BOOL PMICCancelEcho(s16 *mic, s16 *speaker, s32 samples, BOOL reset) {
    f32 micIn;
    f32 spkIn;
    f32 micMag;
    f32 spkMag;
    f32 micLevel;
    f32 spkLevel;
    f32 spkGain;
    f32 micGain;
    f32 diff;
    f32 target;
    s32 gated;
    s32 i;
    if (mic == NULL || speaker == NULL || samples < 0) {
        return 0;
    }
    if (reset != 0) {
        __mic_level = 0.0f;
        __spk_level = 0.0f;
        __spk_gain = 0.0f;
        __mic_gain = 0.0f;
        __far_talk = 0;
        __spk_duck = 0;
    }
    gated = 0;
    for (i = samples; i > 0; i--) {
        micLevel = __mic_level;
        micIn = (f32)*mic;
        micMag = micIn / 32768.0f;
        spkIn = (f32)*speaker;
        if (micMag < 0.0f) {
            micMag = -1.0f * micMag;
        }
        micLevel = micLevel + 0.0078125f * (micMag - micLevel);
        if (micLevel > 1.0f) {
            micLevel = 1.0f;
        } else if (micLevel < 0.0f) {
            micLevel = 0.0f;
        }
        spkMag = spkIn / 32768.0f;
        spkLevel = __spk_level;
        __mic_level = micLevel;
        if (spkMag < 0.0f) {
            spkMag = -1.0f * spkMag;
        }
        diff = spkMag - spkLevel;
        if (diff > 0.0f) {
            spkLevel = spkLevel + 0.0078125f * diff;
        } else {
            spkLevel = spkLevel + 0.00048828125f * diff;
        }
        if (spkLevel > 1.0f) {
            spkLevel = 1.0f;
        } else if (spkLevel < 0.0f) {
            spkLevel = 0.0f;
        }
        __spk_level = spkLevel;
        if (__spk_level >= __echo_threshold) {
            __spk_active = 1;
            if (__spk_level * __echo_ratio > __mic_level) {
                __near_talk = 0;
                __far_talk = 1;
            } else {
                __near_talk = 1;
                __far_talk = 0;
            }
        } else {
            __far_talk = 0;
            __near_talk = 0;
            __spk_active = 0;
        }
        {
            f32 previous = __mic_gain;
            if (__far_talk == 1) {
                micGain = previous - __mic_gain_down;
                if (micGain <= __mic_gain_min) {
                    micGain = __mic_gain_min;
                }
            } else {
                micGain = previous + __mic_gain_up;
                if (micGain >= 1.0f) {
                    micGain = 1.0f;
                }
            }
            __mic_gain = micGain;
        }
        target = __gain_target / __mic_gain;
        if (target > 1.0f) {
            target = 1.0f;
        } else if (target < 0.0f) {
            target = 0.0f;
        }
        if (target > __spk_gain) {
            __spk_duck = 0;
            gated++;
        } else {
            __spk_duck = 1;
        }
        spkGain = __spk_gain;
        if (__spk_duck == 1) {
            spkGain = spkGain - __spk_gain_down;
            if (spkGain <= __spk_gain_min) {
                spkGain = __spk_gain_min;
            }
        } else {
            spkGain = spkGain + __spk_gain_up;
            if (spkGain >= 1.0f) {
                spkGain = 1.0f;
            }
        }
        __spk_gain = spkGain;
        micIn *= __mic_gain;
        spkIn *= __spk_gain;
        *mic++ = (s16)(0.5f + micIn);
        *speaker++ = (s16)(0.5f + spkIn);
    }
    return (f32)gated >= 0.8f * (f32)samples;
}

#pragma fp_contract reset

/* 0x80229EF0 (0x148): installs a set of eight echo parameters (each must lie
 * in 0..1) and resets the filter state. */
void PMICSetEchoParams(const f32 *params) {
    u32 level;
    if (params == NULL || params[0] < 0.0f || params[0] > 1.0f ||
        params[1] < 0.0f || params[1] > 1.0f || params[2] < 0.0f ||
        params[2] > 1.0f || params[3] < 0.0f || params[3] > 1.0f ||
        params[4] < 0.0f || params[4] > 1.0f || params[5] < 0.0f ||
        params[5] > 1.0f || params[6] < 0.0f || params[6] > 1.0f ||
        params[7] < 0.0f || params[7] > 1.0f) {
        return;
    }
    level = OSDisableInterrupts();
    __echo_ratio = params[0];
    __echo_threshold = params[1];
    __mic_gain_min = params[2];
    __mic_gain_up = params[3];
    __mic_gain_down = params[4];
    __spk_gain_min = params[5];
    __spk_gain_up = params[6];
    __spk_gain_down = params[7];
    __gain_target = __mic_gain_min;
    __mic_level = 0.0f;
    __spk_level = 0.0f;
    __spk_gain = 0.0f;
    __mic_gain = 0.0f;
    __far_talk = 0;
    __spk_duck = 0;
    OSRestoreInterrupts(level);
}

/* 0x8022A038 (0xDC): queues the command (0x9A04) that selects one of four gain
 * steps. */
s32 PMICSetGain(u16 gain, PMICCallback callback, s32 arg) {
    u32 level;
    PMICRequest *request;
    s32 state;
    s32 result;
    level = OSDisableInterrupts();
    state = __PMICGetState();
    if (state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (gain > 3) {
        result = PMIC_ERR_INVALID;
    } else if (state < PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        request = __PMICAllocCmd();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            request->command = PMIC_CMD_SET_GAIN;
            request->value0 = gain | 0x300;
            request->done = __PMICCallUcbCb;
            request->callback = callback;
            request->callbackArg = arg;
            request->keep = 0;
            result = __PMICControlAsync(request);
        }
    }
    OSRestoreInterrupts(level);
    return result;
}

/* 0x8022A114 (0xC8): switches the device echo-cancel stage on or off and waits
 * for the answer. */
s32 PMICSetEchoEnable(u32 enable) {
    u32 level;
    PMICRequest *request;
    s32 state;
    s32 result;
    level = OSDisableInterrupts();
    state = __PMICGetState();
    if (state == PMIC_STATE_FATAL) {
        result = PMIC_ERR_FATAL;
    } else if (enable > 1) {
        result = PMIC_ERR_INVALID;
    } else if (state < PMIC_STATE_READY) {
        result = PMIC_ERR_STATE;
    } else {
        request = __PMICAllocCmd();
        if (request == NULL) {
            result = PMIC_ERR_BUSY;
        } else {
            request->command = PMIC_CMD_HCLR;
            request->value0 = enable;
            request->done = __PMICCallUcbCb;
            request->callback = __PMICWakeupCb;
            request->callbackArg = (s32)request;
            request->keep = 1;
            result = __PMICControlSync(request);
        }
    }
    OSRestoreInterrupts(level);
    return result;
}
