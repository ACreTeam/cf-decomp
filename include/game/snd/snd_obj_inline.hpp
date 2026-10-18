#pragma once
#include <game/snd/snd_obj.hpp>

// The inline virtual bodies of the sound objects (see snd_obj.hpp for why they are kept out of it).
// TUs that need these bodies include this. SoundObj.cpp and d_audio_obj.cpp hold the weak copies;
// d_menu_number also needs their pending-parse effect on literal-function section placement.

inline f32 SoundObjBase::getVolume() {
    return mVolume;
}

template <int N>
inline f32 SoundObj<N>::getPosVolume() {
    return mPosVolume;
}

template <int N>
inline f32 SoundObj<N>::getPan() {
    return mPan;
}

inline f32 SoundObjPlayer::getDist() {
    return mDist;
}

inline void SoundObjRiver::startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo) {}

inline void SoundObjRiver::holdSound(u32 id) {}

inline void SoundObjRiver::stopAll(int fadeFrames) {}

#ifndef D_AUDIO_OBJ_PINWHEEL_DTOR_IN_SOURCE
inline SoundObjPinwheel::~SoundObjPinwheel() {}
#endif

inline void SoundObjPinwheel::update(const nw4r::math::VEC3 *pos) {}
