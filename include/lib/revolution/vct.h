#ifndef RVL_SDK_VCT_H
#define RVL_SDK_VCT_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VCTSession {
    u8 _00[0x18];
} VCTSession; // size 0x18 (d_voice_chat keeps three)

// Called by the library on session events
typedef void (*VCTEventCallback)(u8 aid, int event, VCTSession* session, void* data);

typedef struct VCTConfig {
    /* 0x00 */ VCTSession* session;      // session array
    /* 0x04 */ u32 numSession;
    /* 0x08 */ u32 mode;
    /* 0x0C */ u8 aid;                    // own aid
    /* 0x0D */ u8 _0D;
    /* 0x0E */ u8 _0E;
    /* 0x0F */ u8 _0F;
    /* 0x10 */ void* audioBuffer;         // work memory
    /* 0x14 */ u32 audioBufferSize;
    /* 0x18 */ VCTEventCallback callback;
    /* 0x1C */ void* userData;
} VCTConfig; // size 0x20

// audio.c
void VCT_StartStreaming(VCTSession* session);              // start streaming (event 7)
void VCT_StopStreaming(VCTSession* session);              // stop streaming (events 9, 12)
BOOL VCT_SendAudio(const void* data, u32 size);       // send microphone audio
BOOL VCT_ReceiveAudio(void* data, u32 size, u8* aid);    // receive (mixed) audio
void VCT_EnableQualityAlert(BOOL enable);
void VCT_EnableVAD(u8 value);
void VCT_SetDropRecoverMode(u8 value);
void VCTi_SetContinuousDropLimit(u32 value);
void VCTi_SetQualityAlertFrameNum(u32 value);
void VCT_SetQualityAlertThreshold(u32 value);

// session.c
void VCT_DeleteSession(VCTSession* session);              // free the session
s32 VCT_AddConferenceClient(u8 aid);                            // connect to aid (0 = request sent)
void VCT_RemoveConferenceClient(u8 aid);                           // disconnect from aid

// vad.c
void VCT_SetVADReleaseTime(u8 value);

// vct.c
BOOL VCT_Init(const VCTConfig* config);          // init
void VCT_Cleanup(void);                             // cleanup
void VCT_Main(void);                             // main

// debug.c
void VCT_SetReportLevel(u32 level);
u32 VCT_GetAudioDataSize(void);                              // audio frame size in bytes (16-bit samples)
void fn_8036D510(s32 value);                        // empty

#ifdef __cplusplus
}
#endif
#endif
