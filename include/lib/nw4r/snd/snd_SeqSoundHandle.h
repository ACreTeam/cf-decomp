#ifndef NW4R_SND_SEQ_SOUND_HANDLE_H
#define NW4R_SND_SEQ_SOUND_HANDLE_H
#include <nw4r/types_nw4r.h>

#include <nw4r/snd/snd_SeqSound.h>
#include <nw4r/ut.h>

namespace nw4r {
namespace snd {

// Forward declarations
class SoundHandle;

class SeqSoundHandle : private ut::NonCopyable {
public:
    explicit SeqSoundHandle(SoundHandle* pHandle);
    ~SeqSoundHandle() {
        DetachSound();
    }

    bool IsAttachedSound() const {
        return mSound != NULL;
    }

    void DetachSound();

    detail::SeqSound* detail_GetAttachedSound() {
        return mSound;
    }
    const detail::SeqSound* detail_GetAttachedSound() const {
        return mSound;
    }

    void SetTempoRatio(f32 tempo) {
        if (IsAttachedSound()) {
            mSound->SetTempoRatio(tempo);
        }
    }

    void SetTrackVolume(u32 trackFlags, f32 volume) {
        if (IsAttachedSound()) {
            mSound->SetTrackVolume(trackFlags, volume);
        }
    }

    void SetTrackPan(u32 trackFlags, f32 pan) {
        if (IsAttachedSound()) {
            mSound->SetTrackPan(trackFlags, pan);
        }
    }

    void SetTrackPitch(u32 trackFlags, f32 pitch) {
        if (IsAttachedSound()) {
            mSound->SetTrackPitch(trackFlags, pitch);
        }
    }

    bool ReadVariable(int idx, s16* pValue) const {
        if (IsAttachedSound()) {
            return mSound->ReadVariable(idx, pValue);
        }

        return false;
    }

    void SetTrackMute(u32 trackFlags, bool mute) {
        if (IsAttachedSound()) {
            mSound->SetTrackMute(trackFlags, mute);
        }
    }

    void WriteVariable(int idx, s16 value) {
        if (IsAttachedSound()) {
            mSound->WriteVariable(idx, value);
        }
    }

    u32 GetTick() const {
        if (IsAttachedSound()) {
            return mSound->GetTick();
        }
        return 0;
    }

    static bool ReadGlobalVariable(int idx, s16* pValue) {
        return detail::SeqSound::ReadGlobalVariable(idx, pValue);
    }

    static void WriteGlobalVariable(int idx, s16 value) {
        detail::SeqSound::WriteGlobalVariable(idx, value);
    }

private:
    detail::SeqSound* mSound; // at 0x0
};

} // namespace snd
} // namespace nw4r

#endif
