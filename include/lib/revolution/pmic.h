#ifndef REVOLUTION_PMIC_H
#define REVOLUTION_PMIC_H

// PMIC: the Wii Speak (USB microphone) library.

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Results of the PMIC calls and of their completion callbacks.
#define PMIC_ERR_NONE 0
#define PMIC_ERR_INVALID                                                       \
    -1 // bad argument, or the stream buffers are not set up
#define PMIC_ERR_NO_DEVICE -2 // no microphone attached (or it was unplugged)
#define PMIC_ERR_DEVICE -3    // the device did not accept the firmware download
#define PMIC_ERR_STATE -4     // not possible in the current library state
#define PMIC_ERR_BUSY -5  // another operation is in progress: try again later
#define PMIC_ERR_RETRY -6 // the USB request could not be issued: retry the step
#define PMIC_ERR_STREAM -7  // isochronous packets were lost or damaged
#define PMIC_ERR_FATAL -100 // the library has failed and must be shut down

typedef void (*PMICCallback)(s32 result, s32 arg);

// pmic_core.c
s32 PMICInit(void *work);
s32 PMICQuit(void);
s32 PMICProbe(void);
s32 PMICOpenAsync(PMICCallback callback, s32 arg);
s32 PMICCloseAsync(PMICCallback callback, s32 arg);
s32 PMICStartAsync(PMICCallback callback, s32 arg);
s32 PMICStopAsync(PMICCallback callback, s32 arg);

// pmic_data.c
s32 PMICRead(s16 *dst, s32 count);
s32 PMICWrite(s16 *src, s32 count);
s32 PMICConvertOutput(s16 *dmaBuffer, s32 samples, u32 mode, s16 *out,
                      BOOL reset);

// pmic_ctrl.c
s32 PMICSetModeAsync(u16 mode, PMICCallback callback, s32 arg);
s32 PMICSetParamAsync(u16 value, PMICCallback callback, s32 arg);
BOOL PMICCancelEcho(s16 *mic, s16 *speaker, s32 samples, BOOL reset);
void PMICSetEchoParams(const f32 *params);
s32 PMICSetGain(u16 gain, PMICCallback callback, s32 arg);
s32 PMICSetEchoEnable(u32 enable);

// Echo-cancel parameter sets for PMICSetEchoParams (preset 0 holds the library
// defaults).
extern f32 PMICEchoPreset0[8];
extern f32 PMICEchoPreset1[8];
extern f32 PMICEchoPreset2[8];
extern f32 PMICEchoPreset3[8];
extern f32 PMICEchoPreset4[8];
extern f32 PMICEchoPreset5[8];
extern f32 PMICEchoPreset6[8];

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_PMIC_H
