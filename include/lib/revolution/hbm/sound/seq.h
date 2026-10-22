#ifndef REVOLUTION_HBM_SOUND_SEQ_H
#define REVOLUTION_HBM_SOUND_SEQ_H

#include <revolution/hbm/sound/syn.h>

#define HBMSEQ_MAX_TRACKS 64

typedef struct _HBMSEQSEQUENCE HBMSEQSEQUENCE;

typedef struct _HBMSEQTRACK
{
    HBMSEQSEQUENCE* sequence;
    u8* start;
    u8* end;
    u8* current;
    u8 status;
    f32 beatsPerSec;
    u32 defaultTicksPerFrame;
    u32 ticksPerFrame;
    u32 delay;
    u32 state;
} HBMSEQTRACK;

struct _HBMSEQSEQUENCE
{
    HBMSEQSEQUENCE* next;
    u32 state;
    u16 nTracks;
    s16 timeFormat;
    u32 tracksRunning;
    u32 end;
    HBMSYNSYNTH synth;
    HBMSEQTRACK track[HBMSEQ_MAX_TRACKS];
};

#ifdef __cplusplus
extern "C"
{
#endif

    void HBMSEQInit(void);
    void HBMSEQQuit(void);
    void HBMSEQRunAudioFrame(void);
    void HBMSEQAddSequence(HBMSEQSEQUENCE* sequence, u8* midiStream, u8* wavetable, u8* samples, u8* zeroBuffer);
    void HBMSEQRemoveSequence(HBMSEQSEQUENCE* sequence);
    void HBMSEQSetState(HBMSEQSEQUENCE* sequence, u32 state);
    u32 HBMSEQGetState(HBMSEQSEQUENCE* sequence);
    void HBMSEQSetVolume(HBMSEQSEQUENCE* sequence, s32 dB);

#ifdef __cplusplus
}
#endif

#endif
