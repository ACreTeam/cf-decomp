#pragma once

// dol/sound/SoundFader.cpp (801D64F4..801D6618, not decompiled). No RTTI; the class names are
// inferred (the TU name comes from the split). Methods keep their fn_ names.

#include <types.h>
#include <lib/nw4r/snd.h>

// A value faded linearly over a number of frames.
struct SoundFader {
    void fn_801D64F4(f32 value);             // 801D64F4: set at once
    void fn_801D6504(f32 target, int frames); // 801D6504: fade to target
    f32 fn_801D6560();                        // 801D6560: step, returns the value

    /* 0x00 */ f32 mValue;
    /* 0x04 */ f32 mTarget;
    /* 0x08 */ f32 mStep;
    /* 0x0C */ int mFrames;
}; // size 0x10

// A sound handle with fade state (two of them in SoundSystem at 0xC0).
class SoundFadeHandle {
public:
    SoundFadeHandle();  // 801D65A4
    ~SoundFadeHandle(); // 801D65B4: detaches the sound

    /* 0x00 */ u32 _00;
    /* 0x04 */ nw4r::snd::SoundHandle mHandle;
    /* 0x08 */ u8 _08[0x1D - 0x08];
    /* 0x1D */ s8 _1D; // a state (SoundBgm.cpp; getTownTuneState reads it; < 0: idle?)
    /* 0x1E */ u8 _1E[0x28 - 0x1E];
}; // size 0x28
