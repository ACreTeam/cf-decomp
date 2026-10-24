#include <nw4r/snd/snd_SoundHandle.h>

#include <types.h>

#include <nw4r/snd/snd_BasicSound.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
void SoundHandle::detail_AttachSoundAsTempHandle(detail::BasicSound *pSound) {
    mSound = pSound;

    if (pSound->IsAttachedTempGeneralHandle()) {
        mSound->DetachTempGeneralHandle();
    }

    mSound->mTempGeneralHandle = this;
}

void SoundHandle::detail_AttachSound(detail::BasicSound *sound) {
    NW4RAssertPointerNonnull_Line(81, sound);

    mSound = sound;

    if (sound->IsAttachedGeneralHandle()) {
        mSound->DetachGeneralHandle();
    }

    mSound->mGeneralHandle = this;
}

void SoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (mSound->mGeneralHandle == this) {
            mSound->mGeneralHandle = nullptr;
        }

        if (mSound->mTempGeneralHandle == this) {
            mSound->mTempGeneralHandle = nullptr;
        }
    }

    if (mSound) {
        mSound = nullptr;
    }
}
} // namespace snd
} // namespace nw4r
