#ifndef NW4R_SND_SOUND_HANDLE_H
#define NW4R_SND_SOUND_HANDLE_H

#include <types.h>

#include <nw4r/snd/snd_BasicSound.h>

#include <nw4r/ut/ut_algorithm.h>

namespace nw4r {
namespace snd {
class SoundHandle : private ut::NonCopyable {
  public:
    SoundHandle() : mSound() {
    }
    ~SoundHandle() {
        DetachSound();
    }

    void detail_AttachSoundAsTempHandle(detail::BasicSound *pSound);
    void detail_AttachSound(detail::BasicSound *sound);
    bool IsAttachedSound() const {
        return mSound != nullptr;
    }
    detail::BasicSound *detail_GetAttachedSound() {
        return mSound;
    }
    const detail::BasicSound *detail_GetAttachedSound() const {
        return mSound;
    }
    void DetachSound();

    void SetOutputLineFlag(int outputLineFlag) {
        if (IsAttachedSound()) {
            mSound->SetOutputLineFlag(outputLineFlag);
        }
    }

    void FadeIn(int fadeFrames) {
        if (IsAttachedSound()) {
            mSound->FadeIn(fadeFrames);
        }
    }

    void SetVolume(f32 volume, int frames) {
        if (IsAttachedSound()) {
            mSound->SetVolume(volume, frames);
        }
    }

    void SetPitch(f32 pitch) {
        if (IsAttachedSound()) {
            mSound->SetPitch(pitch);
        }
    }

    void Stop(int fadeFrames) {
        if (IsAttachedSound()) {
            mSound->Stop(fadeFrames);
        }
    }

    void Pause(bool flag, int fadeFrames) {
        if (IsAttachedSound()) {
            mSound->Pause(flag, fadeFrames);
        }
    }

    void SetPan(f32 pan) {
        if (IsAttachedSound()) {
            mSound->SetPan(pan);
        }
    }

    void SetFxSend(AuxBus bus, f32 send) {
        if (IsAttachedSound()) {
            mSound->SetFxSend(bus, send);
        }
    }

    void SetLpfFreq(f32 lpfFreq) {
        if (IsAttachedSound()) {
            mSound->SetLpfFreq(lpfFreq);
        }
    }

    void SetBiquadFilter(int type, f32 value) {
        if (IsAttachedSound()) {
            mSound->SetBiquadFilter(type, value);
        }
    }

    void SetSurroundPan(f32 pan) {
        if (IsAttachedSound()) {
            mSound->SetSurroundPan(pan);
        }
    }

    bool IsPause() const {
        return IsAttachedSound() && mSound->IsPause();
    }

    bool IsPrepared() const {
        return IsAttachedSound() && mSound->IsPrepared();
    }

    int GetRemainingFadeFrames() const {
        if (IsAttachedSound()) {
            return mSound->GetRemainingFadeFrames();
        }

        return 0;
    }

    u32 GetId() const {
        if (IsAttachedSound()) {
            return mSound->GetId();
        }

        return detail::BasicSound::INVALID_ID;
    }

    void StartPrepared() {
        if (IsAttachedSound()) {
            mSound->StartPrepared();
        }
    }

  private:
    /* base NonCopyable */      // size 0x00, offset 0x00
    detail::BasicSound *mSound; // size 0x04, offset 0x00
}; // size 0x04
} // namespace snd
} // namespace nw4r

#endif
