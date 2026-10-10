#pragma once

// dol/sound/SoundEffectReverb.cpp (801D00E8..801D0550, not decompiled).
// Its own header: with -sym on each header's inlines get their own weak .text section, and the
// d_snd_util tail has SoundFrameCallback's OnEndSoundFrame between ~SoundEffectReverb and
// ~SoundAudioFrameCallback, so the two classes can't share a header.

#include <types.h>
#include <lib/nw4r/snd/snd_FxReverbHiDpl2.h>

// RTTI "SoundEffectReverb" (dol/sound/SoundEffectReverb.cpp), vtable 80500678.
class SoundEffectReverb : public nw4r::snd::FxReverbHiDpl2 {
public:
    SoundEffectReverb() {}
    virtual ~SoundEffectReverb() {} // 800103F8 (weak copy in d_snd_util)
    virtual bool StartUp();         // 801D00E8
    virtual void Shutdown();        // 801D013C

    /* 0x308 */ bool mStarted;
}; // size 0x30C
