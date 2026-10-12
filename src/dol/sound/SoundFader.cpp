// A linearly faded value, and SoundMelodyPart's ctor and dtor. .text 801D64F4..801D6618, .sdata2
// 80751BE8..80751BF0. See include/game/snd/snd_fader.hpp.
#include <game/snd/snd_fader.hpp>

// 801D64F4
void SoundFader::set(f32 value) {
    mValue = value;
    mFrames = -1;
}

// 801D6504
void SoundFader::fadeTo(f32 target, int frames) {
    if (frames == 0) {
        mValue = target;
        mFrames = -1;
    } else {
        mTarget = target;
        mStep = (mTarget - mValue) / frames;
        mFrames = frames;
    }
}

// 801D6560
f32 SoundFader::calc() {
    if (mFrames > 0) {
        mFrames--;
        if (mFrames != 0) {
            mValue += mStep;
        } else {
            mValue = mTarget;
            mFrames--;
        }
    }
    return mValue;
}

// 801D65A4
SoundMelodyPart::SoundMelodyPart() : mOwner(NULL) {}

// 801D65B4
SoundMelodyPart::~SoundMelodyPart() {
    mHandle.DetachSound();
}
