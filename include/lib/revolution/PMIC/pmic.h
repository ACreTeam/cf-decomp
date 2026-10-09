#ifndef RVL_SDK_PMIC_H
#define RVL_SDK_PMIC_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// PMIC: the Wii Speak (USB microphone) library ("<< RVL_SDK - PMIC release build: Sep 18 2008 >>").
// lib/RVL_SDK/pmic/pmic.c, pmic_stream.c, pmic_ctrl.c are not decompiled; the real function names
// are unknown, so the functions keep their addresses. Only what the game uses so far
// (d_voice_chat.cpp); the behaviour notes come from how d_voice_chat drives them.

// Completion callback of the asynchronous requests: the result (0 = done, < 0 = error).
typedef void (*PMICCallback)(s32 result);

// Error codes seen in d_voice_chat.
#define PMIC_ERR_NONE 0
#define PMIC_ERR_NO_DEVICE -2 // returned by the status check when no microphone is attached
#define PMIC_ERR_BUSY -5      // request not accepted now: retry next frame
#define PMIC_ERR_RETRY -6     // (as a request result / completion) go back one step

// pmic.c
s32 fn_80225FA8(void);                         // device status
s32 fn_8022607C(PMICCallback callback, void* arg); // open
s32 fn_802261A4(PMICCallback callback, void* arg); // close
s32 fn_80226234(PMICCallback callback, void* arg); // start
s32 fn_80226348(PMICCallback callback, void* arg); // stop

// pmic_stream.c
s32 fn_80228EB0(s16* buffer, u32 samples);     // read microphone samples, returns the count
void fn_80228F98(s16* buffer, s32 samples);    // feed the echo-cancel reference (speaker) samples
s32 fn_802290D4(const void* dmaBuffer, u32 samples, u32 arg2, s16* out, BOOL reset); // AI output -> reference

// pmic_ctrl.c
s32 fn_80229A2C(u32 arg0, PMICCallback callback, void* arg);
s32 fn_80229B04(u16 value, PMICCallback callback, void* arg);
BOOL fn_80229BE4(s16* mic, const s16* speaker, u32 samples, u32 mode); // echo cancel
void fn_80229EF0(const f32* params);           // select a parameter preset (lbl_80521120...)
s32 fn_8022A038(u16 value, PMICCallback callback, void* arg);
void fn_8022A114(u32 arg);

// The parameter presets d_voice_chat chooses from by the number of members (pmic_ctrl.c .data).
extern f32 lbl_80521120[8];
extern f32 lbl_80521140[8];
extern f32 lbl_80521160[8];
extern f32 lbl_80521180[8];
extern f32 lbl_805211A0[8];
extern f32 lbl_805211C0[8];
extern f32 lbl_805211E0[8];

#ifdef __cplusplus
}
#endif
#endif
