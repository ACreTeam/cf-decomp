#include <nw4r/snd/snd_StrmSoundHandle.h>

#include <types.h>

#include <nw4r/snd/snd_SoundHandle.h>
#include <nw4r/snd/snd_StrmSound.h>
#include <nw4r/ut/ut_RuntimeTypeInfo.h>

namespace nw4r {
namespace snd {
StrmSoundHandle::StrmSoundHandle(SoundHandle *handle) : mSound(nullptr) {
    if (!handle) {
        return;
    }

    detail::BasicSound *basicSound = handle->detail_GetAttachedSound();
    if (!basicSound) {
        return;
    }

    if (detail::StrmSound *sound =
            ut::DynamicCast<detail::StrmSound *>(basicSound)) {
        mSound = sound;

        if (mSound->IsAttachedTempSpecialHandle()) {
            mSound->DetachTempSpecialHandle();
        }

        mSound->mTempSpecialHandle = this;
    }
}

void StrmSoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (mSound->mTempSpecialHandle == this) {
            mSound->mTempSpecialHandle = nullptr;
        }
    }

    if (mSound) {
        mSound = nullptr;
    }
}
} // namespace snd
} // namespace nw4r
