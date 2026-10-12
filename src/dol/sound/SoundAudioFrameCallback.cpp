// The sound frame callback of SoundManager. .text 801E1BD4..801E1C78, .data 80503C90..80503CD0, .sdata
// 8074C700..8074C708. See include/game/snd/snd_obj.hpp.
#include <game/snd/snd_obj.hpp>

// 801E1BD4
void SoundAudioFrameCallback::OnBeginSoundFrame() {
    if (mCallback != NULL) {
        mCallback();
    }
}

// 801E1BEC
void SoundAudioFrameCallback::set(Callback callback) {
    if (mCallback == NULL) {
        mCallback = callback;
        nw4r::snd::detail::SoundThread::GetInstance().RegisterSoundFrameCallback(this);
    }
}

// 801E1C30
void SoundAudioFrameCallback::clear() {
    if (mCallback != NULL) {
        mCallback = NULL;
        nw4r::snd::detail::SoundThread::GetInstance().UnregisterSoundFrameCallback(this);
    }
}
