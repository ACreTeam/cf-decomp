#ifndef RVL_SDK_VCT_H
#define RVL_SDK_VCT_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VCTSession {
    /* 0x00 */ u32 mode;
    /* 0x04 */ u8 aid;
    /* 0x05 */ u8 talking;
    /* 0x08 */ u32 aidBitmap;
    /* 0x0C */ u32 state;
    /* 0x10 */ void* userData;
    /* 0x14 */ struct VCTSession* next;
} VCTSession; // size 0x18 (d_voice_chat keeps three)

// Called by the library on session events
typedef void (*VCTEventCallback)(u8 aid, int event, VCTSession* session, void* data);

typedef struct VCTConfig {
    /* 0x00 */ VCTSession* session;      // session array
    /* 0x04 */ u32 numSession;
    /* 0x08 */ u32 mode;
    /* 0x0C */ u8 aid;                    // own aid
    /* 0x0D */ u8 codec;                  // ADPCM codec (VCTCodec)
    /* 0x0E */ u8 frameLength;            // audio frame length in ms (0 = default 68, 68..192)
    /* 0x0F */ u8 flags;                  // 1 = VAD, 2 = echo cancel (ignored on Wii), 4 = high pass filter
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
void VCTi_NotifyMicStartResult(s32 result);        // empty

#ifdef __cplusplus
}
#endif
#endif
