#ifndef DWC_VCT_H
#define DWC_VCT_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// VCT: the DWC voice chat library (lib/dwc/vct/*.c, not decompiled). The real function names
// are unknown, so the functions keep their addresses; the comments say how d_voice_chat.cpp
// uses them. Only what the game uses so far.

typedef struct VCTSession {
    u8 _00[0x18];
} VCTSession; // size 0x18 (d_voice_chat keeps three)

// Called by the library on session events (see d_voice_chat.cpp eventCallback).
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
void fn_80366120(VCTSession* session);              // start streaming (event 7)
void fn_803662F0(VCTSession* session);              // stop streaming (events 9, 12)
BOOL fn_803663C4(const void* data, u32 size);       // send microphone audio
BOOL fn_80366A08(void* data, u32 size, u8* aid);    // receive (mixed) audio
void fn_80368A40(BOOL enable);
void fn_80368A60(u8 value);
void fn_80368AC8(u8 value);
void fn_80368AD0(u32 value);
void fn_80368DE0(u32 value);
void fn_80368DE8(u32 value);

// session.c
void fn_8036A554(VCTSession* session);              // free the session
s32 fn_8036A854(u8 aid);                            // connect to aid (0 = request sent)
void fn_8036AA20(u8 aid);                           // disconnect from aid

// vad.c
void fn_8036CB50(u8 value);

// vct.c
BOOL fn_8036CF1C(const VCTConfig* config);          // init
void fn_8036D1DC(void);                             // cleanup
void fn_8036D218(void);                             // main

// debug.c
void fn_8036D458(u32 level);
u32 fn_8036D50C(void);                              // audio frame size in bytes (16-bit samples)
void fn_8036D510(s32 value);                        // empty

#ifdef __cplusplus
}
#endif
#endif
