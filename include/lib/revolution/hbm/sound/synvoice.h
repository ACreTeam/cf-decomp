#ifndef REVOLUTION_HBM_SOUND_SYNVOICE_H
#define REVOLUTION_HBM_SOUND_SYNVOICE_H

#include <revolution/hbm/sound/syn.h>
#include <revolution/ax/AXVPB.h>

struct HBMSYNVOICE
{
    void* next;
    AXVPB* axvpb;
    HBMSYNSYNTH* synth;
    u8 midiChannel;
    u8 keyNum;
    u8 keyVel;
    u8 pan;
    WTREGION* region;
    WTART* art;
    WTSAMPLE* sample;
    WTADPCM* adpcm;
    u32 type;
    f32 srcRatio;
    s32 cents;
    s32 attn;
    u32 veState;
    s32 veAttn;
    s32 veAttack;
    s32 veAttackDelta;
    s32 veDecay;
    s32 veSustain;
    s32 veRelease;
};

#ifdef __cplusplus
extern "C"
{
#endif

    void __HBMSYNClearVoiceReferences(void* voice);
    void __HBMSYNSetVoiceToRelease(HBMSYNVOICE* voice);
    void __HBMSYNServiceVoice(int index);

#ifdef __cplusplus
}
#endif

#endif
