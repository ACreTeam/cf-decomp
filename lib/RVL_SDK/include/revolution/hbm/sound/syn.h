#ifndef REVOLUTION_HBM_SOUND_SYN_H
#define REVOLUTION_HBM_SOUND_SYN_H

#include <revolution/types.h>

#define NUM_MIDI_CHANNELS        16
#define NUM_MIDI_NOTES           128
#define HBMSYN_INPUT_BUFFER_SIZE 256

typedef struct WTINST
{
    u16 keyRegion[128];
} WTINST;
typedef struct WTREGION
{
    u8 unityNote;
    u8 keyGroup;
    s16 fineTune;
    s32 attn;
    u32 loopStart;
    u32 loopLength;
    u32 articulationIndex;
    u32 sampleIndex;
} WTREGION;
typedef struct WTART
{
    s32 lfoFreq;
    s32 lfoDelay;
    s32 lfoAtten;
    s32 lfoPitch;
    s32 lfoMod2Atten;
    s32 lfoMod2Pitch;
    s32 eg1Attack;
    s32 eg1Decay;
    s32 eg1Sustain;
    s32 eg1Release;
    s32 eg1Vel2Attack;
    s32 eg1Key2Decay;
    s32 eg2Attack;
    s32 eg2Decay;
    s32 eg2Sustain;
    s32 eg2Release;
    s32 eg2Vel2Attack;
    s32 eg2Key2Decay;
    s32 eg2Pitch;
    s32 pan;
} WTART;
typedef struct WTSAMPLE
{
    u16 format;
    u16 sampleRate;
    u32 offset;
    u32 length;
    u16 adpcmIndex;

} WTSAMPLE;
typedef struct WTADPCM
{
    u16 a[8][2];
    u16 gain;
    u16 pred_scale;
    u16 yn1;
    u16 yn2;
    u16 loop_pred_scale;
    u16 loop_yn1;
    u16 loop_yn2;
} WTADPCM;
typedef struct HBMSYNVOICE HBMSYNVOICE;

typedef struct HBMSYNSYNTH
{
    struct HBMSYNSYNTH* next;
    WTINST* percussiveInst;
    WTINST* melodicInst;
    WTREGION* region;
    WTART* art;
    WTSAMPLE* sample;
    WTADPCM* adpcm;
    u32 samplesBaseWord;
    u32 samplesBaseByte;
    u32 samplesBaseNibble;
    WTINST* inst[NUM_MIDI_CHANNELS];
    s32 masterVolume;
    s32 volAttn[NUM_MIDI_CHANNELS];
    s32 auxAAttn[NUM_MIDI_CHANNELS];
    u8 pan[NUM_MIDI_CHANNELS];
    u8 input[HBMSYN_INPUT_BUFFER_SIZE][3];
    u8* inputPosition;
    u32 inputCounter;
    u32 notes;
    HBMSYNVOICE* voice[NUM_MIDI_CHANNELS][NUM_MIDI_NOTES];
} HBMSYNSYNTH;

#ifdef __cplusplus
extern "C"
{
#endif

    void HBMSYNInit(void);
    void HBMSYNQuit(void);
    void HBMSYNRunAudioFrame(void);
    void HBMSYNInitSynth(HBMSYNSYNTH* synth, u8* wavetable, u8* samples, u8* zeroBuffer);
    void HBMSYNQuitSynth(HBMSYNSYNTH* synth);
    void HBMSYNMidiInput(HBMSYNSYNTH* synth, u8* input);
    void HBMSYNSetMasterVolume(HBMSYNSYNTH* synth, s32 dB);

    extern HBMSYNVOICE* __HBMSYNVoice;

#ifdef __cplusplus
}
#endif

#endif
