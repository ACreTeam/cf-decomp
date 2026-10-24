#ifndef NW4R_SND_STRM_SOUND_HANDLE_H
#define NW4R_SND_STRM_SOUND_HANDLE_H

#include <types.h>

#include <nw4r/snd/snd_SoundHandle.h>
#include <nw4r/snd/snd_StrmSound.h>
#include <nw4r/ut/ut_NonCopyable.h>

namespace nw4r {
namespace snd {
namespace detail {
class StrmSound;
}
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
class StrmSoundHandle : private ut::NonCopyable {
  public:
    StrmSoundHandle(SoundHandle *);
    ~StrmSoundHandle() {
        DetachSound();
    }
    bool IsAttachedSound() const {
        return mSound != nullptr;
    }

    void DetachSound();

    void SetVolume(u32 trackFlags, f32 volume, int fadeFrames) {
        if (IsAttachedSound()) {
            mSound->SetTrackVolume(trackFlags, volume, fadeFrames);
        }
    }

    u32 GetPlaySamplePosition() const {
        if (!IsAttachedSound()) {
            return -1;
        }
        return mSound->GetPlaySamplePosition();
    }

  private:
    /* base NonCopyable */     // size 0x00, offset 0x00
    detail::StrmSound *mSound; // size 0x04, offset 0x00
}; // size 0x04
} // namespace snd
} // namespace nw4r

#endif
