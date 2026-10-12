// The positional sound objects (SoundObjBase, SoundObj<N> and their subclasses). .text 801DC544..801E1BD4,
// .data 80503930..80503C90, .bss 80625DB0..80625DC8, .sdata 8074C6B0..8074C700, .sbss 8074EC60..8074EC68,
// .sdata2 80751C80..80751D00. See include/game/snd/snd_obj.hpp.
#include <game/snd/snd_obj_inline.hpp>
#include <game/snd/snd_id.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_system.hpp>
#include <game/game/d_weather.hpp>
#include <lib/nw4r/math/math_arithmetic.h>
#include <revolution/MTX/mtxvec.h>

SoundObjBase *SoundObjBase::sFirst;
f32 SoundObjBase::sFxSend;
f32 SoundObjBase::sMasterVolume = 1.0f;
bool SoundObjBase::sIsPlayable = true;

static inline f32 clamp(f32 value, f32 min, f32 max) {
    if (value > max) {
        value = max;
    } else if (value < min) {
        value = min;
    }
    return value;
}

// 801DC544
SoundObjBase::SoundObjBase()
    : mPrev(NULL), mNext(NULL), mPosVolume(0.0f), mPan(0.0f), mSurroundPan(0.0f), mVolume(1.0f),
      mPos(2000.0f, 2000.0f, 2000.0f) {
    appendList();
}

// 801DC5B4
SoundObjBase::~SoundObjBase() {
    removeList();
}

// 801DC614
void SoundObjBase::setPosition(const nw4r::math::VEC3 *pos) {
    if (pos != NULL) {
        mPos.x = pos->x;
        mPos.y = pos->y;
        mPos.z = pos->z;
        calcParam(&mPos, PARAM_MODE_NORMAL);
    }
}

// 801DC650
void SoundObjBase::setVolume(f32 volume) {
    mVolume = clamp(volume, 0.0f, 1.0f);
}

// 801DC67C: the volume from the distance (curve by mode), the pan from x and the surround pan from z.
void SoundObjBase::calcParam(const nw4r::math::VEC3 *pos, int mode) {
    if (pos == NULL) {
        return;
    }

    nw4r::math::VEC3 vec = *pos;
    f32 x = pos->x;
    f32 y = pos->y;
    f32 z = pos->z;

    if (SoundManager::getInstance()->_778 == 0) {
        vec.y += 280.0f;
    }

    if (mode == PARAM_MODE_NORMAL) {
        f32 volume = clamp((650.0f - PSVECMag(vec)) / 400.0f, 0.0f, 1.0f);
        if (z > -125.0f) {
            volume *= clamp(1.0f - (z - -125.0f) / 250.0f, 0.0f, 1.0f);
        }
        mPosVolume = clamp(volume, 0.0f, 1.0f);
    } else if (mode == PARAM_MODE_INSECT) {
        f32 volume = clamp((433.33334f - PSVECMag(vec)) / 183.33334f, 0.0f, 1.0f);
        if (z > -125.0f) {
            volume *= clamp(1.0f - (z - -205.0f) / 250.0f, 0.0f, 1.0f);
        }
        mPosVolume = clamp(volume, 0.0f, 1.0f);
    } else if (mode == PARAM_MODE_FOUNTAIN) {
        vec.z += 250.0f;
        vec.y *= 0.8f;
        mPosVolume = clamp((350.0f - PSVECMag(vec)) / 100.0f, 0.0f, 1.0f);
    }

    f32 dist = nw4r::math::FSqrt((y * y) / 25.0f + (x * x + (z - -125.0f) * (z - -125.0f)));
    if (dist > 0.0f) {
        mPan = clamp(x / dist, -1.0f, 1.0f);
    } else {
        mPan = 0.0f;
    }

    if (z <= -125.0f) {
        mSurroundPan = clamp((z - -350.0f) / 225.0f, 0.0f, 1.0f);
    } else {
        mSurroundPan = 1.0f + clamp((z - -125.0f) / 125.0f, 0.0f, 1.0f);
    }
}

// 801DCB60
void SoundObjBase::appendList() {
    SoundObjBase *obj = sFirst;
    SoundObjBase *last = NULL;
    while (obj != NULL) {
        last = obj;
        obj = obj->mNext;
    }

    if (last != NULL) {
        last->mNext = this;
    } else {
        sFirst = this;
    }
    mPrev = last;
}

// 801DCB98
void SoundObjBase::removeList() {
    SoundObjBase *prev = mPrev;
    SoundObjBase *next = mNext;

    if (prev != NULL && next != NULL) {
        prev->mNext = next;
        next->mPrev = prev;
    } else if (prev != NULL && next == NULL) {
        prev->mNext = NULL;
    } else if (prev == NULL && next != NULL) {
        next->mPrev = NULL;
        sFirst = next;
    } else {
        sFirst = NULL;
    }
}

// 801DCC04
void SoundObjBase::stopAllObj(int fadeFrames) {
    for (SoundObjBase *obj = sFirst; obj != NULL; obj = obj->mNext) {
        obj->stopAll(fadeFrames);
    }
    sIsPlayable = false;
}

// 801DCC68
void SoundObjBase::setFxSend(f32 send) {
    sFxSend = send;
}

// 801DCC70
f32 SoundObjBase::getFxSend() {
    return sFxSend;
}

// 801DCC78
void SoundObjBase::setMasterVolume(f32 volume) {
    sMasterVolume = volume;
}

// 801DCC80
f32 SoundObjBase::getMasterVolume() {
    return sMasterVolume;
}

// 801DCC88
SoundObjPlayer::SoundObjPlayer() : mDist(0.0f) {
    mKind = KIND_PLAYER;
    mFoot.mOwner = this;
}

// 801DCD20
SoundObjPlayer::~SoundObjPlayer() {}

// 801DCDE4
void SoundObjPlayer::update(const nw4r::math::VEC3 *pos) {
    SoundObj<4>::update(pos);
    mDist = PSVECMag(*pos);
    mFoot.update();
}

// 801DCE30
SoundObjNPC::SoundObjNPC() {
    mVolume = 0.75f;
    mKind = KIND_NPC;
    mFoot.mOwner = this;
}

// 801DCECC
SoundObjNPC::~SoundObjNPC() {}

// 801DCF90
void SoundObjNPC::update(const nw4r::math::VEC3 *pos) {
    SoundObj<2>::update(pos);
    mFoot.update();
}

// 801DCFC4
SoundObjInsect::SoundObjInsect() {
    mKind = KIND_INSECT;
}

// 801DD034
SoundObjInsect::~SoundObjInsect() {}

// 801DD0C4: like SoundObj<1>::update, with PARAM_MODE_INSECT for the fly, mosquito and bee sounds.
void SoundObjInsect::update(const nw4r::math::VEC3 *pos) {
    if (pos != NULL) {
        int mode = PARAM_MODE_NORMAL;
        if (mHandles[0].IsAttachedSound()) {
            u32 id = mHandles[0].GetId();
            if (id == SE_mosqueto || id == SE_hae || id == SE_mitsubachi_fly) {
                mode = PARAM_MODE_INSECT;
            }
        }

        mPos.x = pos->x;
        mPos.y = pos->y;
        mPos.z = pos->z;
        calcParam(&mPos, mode);

        mPosVolume = clamp(mPosVolume, 0.0f, 1.0f);
        mVolume = clamp(mVolume, 0.0f, 1.0f);
        if (mHandles[0].IsAttachedSound()) {
            mHandles[0].SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
            mHandles[0].SetPan(mPan);
        }
    }
}

// 801DD1F8: whether the sequence's first track still has channels.
bool SoundObjInsect::isSounding() {
    bool sounding = false;
    if (mHandles[0].IsAttachedSound()) {
        nw4r::snd::SeqSoundHandle handle(&mHandles[0]);
        nw4r::snd::detail::SeqTrack *track = handle.detail_GetAttachedSound()->GetSeqPlayer().GetPlayerTrack(0);
        if (track != NULL && track->GetChannelCount() > 0) {
            sounding = true;
        }
    }
    return sounding;
}

// 801DD270
SoundObjSimple::SoundObjSimple() {
    mKind = KIND_SIMPLE;
}

// 801DD2E0
SoundObjSimple::~SoundObjSimple() {}

// 801DD370
void SoundObjSimple::update(const nw4r::math::VEC3 *pos) {
    SoundObj<1>::update(pos);
}

// 801DD374
SoundFoot::~SoundFoot() {
    mHandle.DetachSound();
    mHandle2.DetachSound();
}

// 801DD3EC: a foot step: id + 0/1 alternating per foot, sometimes the alternative id + 2/3. With a mix
// below 0xFF, sound SE_walk_dirt1 + variant is layered on mHandle2.
void SoundFoot::start(u32 id, u8 mix, f32 speed) {
    if (SoundSystem::getInstance()->mState != SoundSystem::STATE_READY || mOwner == NULL ||
        !SoundObjBase::sIsPlayable) {
        return;
    }

    int scene = SoundManager::getInstance()->_778;
    if ((scene == 0 || scene == 0x2D || scene == 0x38) && dWeather_c::sInstance != NULL &&
        (u32)(dWeather_c::sInstance->_5884 - 3) <= 1) {
        if (id == SE_walk_grass1) {
            id = SE_walk_nukarumi1;
        } else if (id == SE_walk_leaf1) {
            id = SE_walk_leaf_rain1;
        }
    }

    int rnd = SoundSystem::getInstance()->getRandom(4);
    if (id != SE_walk_grass1 && id != SE_walk_leaf1 && id != SE_walk_snow1 && id != SE_walk_nukarumi1 &&
        id != SE_walk_leaf_rain1) {
        mix = 0xFF;
    }

    int variant = 0;
    bool alt0 = (mFlags >> 1) & 1;
    bool alt1 = (mFlags >> 2) & 1;
    if (mFlags & FLAG_FOOT) {
        if (rnd == 0 && !alt0 && !alt1) {
            mFlags |= FLAG_ALT0;
            variant = 2;
        } else {
            mFlags &= ~FLAG_ALT0;
        }
    } else {
        if (rnd == 0 && !alt0 && !alt1) {
            mFlags |= FLAG_ALT1;
            variant = 3;
        } else {
            mFlags &= ~FLAG_ALT1;
            variant = 1;
        }
    }

    mVolume = 0.4f + (1.0f - 0.4f) * speed;
    mFlags ^= FLAG_FOOT;
    f32 pitch = 0.8f + (1.0f - 0.8f) * speed;

    SoundManager::getInstance()->startSound(&mHandle, id + variant);
    if (mix < 0xFF) {
        SoundManager::getInstance()->startSound(&mHandle2, variant + SE_walk_dirt1);
        mHandle2.SetPitch(pitch);
    }
    mHandle.SetPitch(pitch);
    mMix = mix;

    f32 posVolume = mOwner->mPosVolume;
    f32 volume = SoundObjBase::getMasterVolume() * (mVolume * (posVolume * mOwner->getVolume()));
    f32 pan = mOwner->mPan;
    if (mMix < 0xFF) {
        f32 ratio = mMix / 255.0f;
        mHandle2.SetVolume(volume * (1.0f - ratio), 0);
        mHandle2.SetPan(pan);
        volume *= ratio;
    }
    mHandle.SetVolume(volume, 0);
    mHandle.SetPan(pan);
    mHandle.SetFxSend(nw4r::snd::AUX_A, SoundObjBase::getFxSend());
}

// 801DDADC: follows the owner's volume and pan.
void SoundFoot::update() {
    if (mHandle.IsAttachedSound() && mOwner != NULL) {
        f32 posVolume = mOwner->mPosVolume;
        f32 volume = SoundObjBase::getMasterVolume() * (mVolume * (posVolume * mOwner->getVolume()));
        f32 pan = mOwner->mPan;
        if (mMix < 0xFF) {
            f32 ratio = mMix / 255.0f;
            mHandle2.SetVolume(volume * (1.0f - ratio), 0);
            mHandle2.SetPan(pan);
            volume *= ratio;
        }
        mHandle.SetVolume(volume, 0);
        mHandle.SetPan(pan);
    }
}

// 801DDC0C
SoundObjRiver::SoundObjRiver() {
    mKind = KIND_RIVER;
    SoundSystem::getInstance()->appendAmbientObj(this);
}

// 801DDCA0
SoundObjRiver::~SoundObjRiver() {
    SoundSystem::getInstance()->removeAmbientObj(this);
}

// 801DDD58
void SoundObjRiver::update(const nw4r::math::VEC3 *pos) {
    calcParam(pos, PARAM_MODE_NORMAL);
    mPos.x = pos->x;
    mPos.y = pos->y;
    mPos.z = pos->z;
}

// 801DDDB8
SoundObjPond::SoundObjPond() {
    mKind = KIND_POND;
    SoundSystem::getInstance()->appendAmbientObj(this);
}

// 801DDE78
SoundObjPond::~SoundObjPond() {
    SoundSystem::getInstance()->removeAmbientObj(this);
}

// 801DDF64
SoundObjFall::SoundObjFall() {
    mKind = KIND_FALL;
    SoundSystem::getInstance()->appendAmbientObj(this);
}

// 801DE024
SoundObjFall::~SoundObjFall() {
    SoundSystem::getInstance()->removeAmbientObj(this);
}

// 801DE110
SoundObjPinwheel::SoundObjPinwheel() : _30(0.0f) {}

// 801DE180
void SoundObjPinwheel::update(const nw4r::math::VEC3 *pos, f32 speed) {
    speed = clamp(speed, 0.0f, 1.0f);
    SoundObj<1>::update(pos);

    if (speed > 0.3f) {
        f32 volume = mPosVolume * mVolume;
        volume = volume * (0.1f + (1.0f - 0.1f) * speed);
        volume = clamp(volume, 0.0f, 1.0f);
        f32 pitch = 1.35f + (1.5f - 1.35f) * speed;
        mHandles[0].SetVolume(volume * sMasterVolume, 0);
        mHandles[0].SetPitch(pitch);
    } else {
        mHandles[0].SetVolume(0.0f, 0);
    }
}

template <int N>
void SoundObj<N>::startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo) {
    SoundManager *mgr = SoundManager::getInstance();
    if (SoundSystem::getInstance()->mState != SoundSystem::STATE_READY) {
        return;
    }
    if (!sIsPlayable && !((mgr->mArchive.GetSoundUserParam(id) >> 1) & 1)) {
        return;
    }

    nw4r::snd::SoundHandle *handle = NULL;
    for (int i = 0; i < N; i++) {
        if (!mHandles[i].IsAttachedSound()) {
            handle = &mHandles[i];
            break;
        }
    }

    if (handle == NULL) {
        int lowest = 0x7F;
        int index = -1;
        for (int i = 0; i < N; i++) {
            if (mHandles[i].IsAttachedSound()) {
                int priority = mgr->getSoundPriority(mHandles[i].GetId());
                if (priority <= lowest) {
                    index = i;
                    lowest = priority;
                }
            }
        }

        if (lowest < mgr->getSoundPriority(id)) {
            handle = &mHandles[index];
            handle->DetachSound();
        }
    }

    calcParam(&mPos, PARAM_MODE_NORMAL);
    if (mPosVolume > 0.0f) {
        nw4r::snd::SoundHandle tempHandle;
        if (handle == NULL) {
            handle = &tempHandle;
        }

        if (startInfo != NULL) {
            SoundManager::getInstance()->startSound(handle, id, startInfo);
        } else {
            SoundManager::getInstance()->startSound(handle, id);
        }

        if (mgr->isFixedVolume(id)) {
            handle->SetVolume(mPosVolume * mVolume, 0);
        } else {
            handle->SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
        }
        handle->SetPan(mPan);
        if (mKind == KIND_INSECT) {
            handle->SetFxSend(nw4r::snd::AUX_A, sFxSend);
        }
    }
}

template <int N>
void SoundObj<N>::holdSound(u32 id) {
    SoundManager *mgr = SoundManager::getInstance();
    if (SoundSystem::getInstance()->mState != SoundSystem::STATE_READY) {
        return;
    }
    if (!sIsPlayable && !((mgr->mArchive.GetSoundUserParam(id) >> 1) & 1)) {
        return;
    }

    nw4r::snd::SoundHandle *handle = NULL;
    bool held = false;
    for (int i = 0; i < N; i++) {
        if (mHandles[i].IsAttachedSound()) {
            if (id == mHandles[i].GetId()) {
                if (mPosVolume > 0.0f) {
                    mgr->holdSound(&mHandles[i], id);
                    held = true;
                }
                break;
            }
        } else {
            handle = &mHandles[i];
        }
    }
    if (held) {
        return;
    }

    if (handle != NULL) {
        int mode = PARAM_MODE_NORMAL;
        if (id == SE_mosqueto || id == SE_hae || id == SE_mitsubachi_fly) {
            mode = PARAM_MODE_INSECT;
        } else if (id == SE_LEV_funsui_machi) {
            mode = PARAM_MODE_FOUNTAIN;
        }

        calcParam(&mPos, mode);
        if (mPosVolume > 0.0f) {
            SoundManager::getInstance()->holdSound(handle, id);
            if (SoundManager::getInstance()->isFixedVolume(id)) {
                handle->SetVolume(mPosVolume * mVolume, 0);
            } else {
                handle->SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
            }
            handle->SetPan(mPan);
            if (mKind == KIND_INSECT) {
                mHandles[0].SetFxSend(nw4r::snd::AUX_A, sFxSend);
            }
        }
    } else {
        SoundManager *mgr2 = SoundManager::getInstance();
        int lowest = 0x7F;
        int index = -1;
        for (int i = 0; i < N; i++) {
            if (mHandles[i].IsAttachedSound()) {
                int priority = mgr2->getSoundPriority(mHandles[i].GetId());
                if (priority <= lowest) {
                    index = i;
                    lowest = priority;
                }
            }
        }

        if (lowest > mgr2->getSoundPriority(id)) {
            return;
        }
        if (index < 0) {
            return;
        }

        calcParam(&mPos, PARAM_MODE_INSECT); // whatever the sound
        if (mPosVolume > 0.0f) {
            SoundManager *manager = SoundManager::getInstance();
            handle = &mHandles[index];
            handle->DetachSound();
            manager->holdSound(handle, id);
            if (manager->isFixedVolume(id)) {
                handle->SetVolume(mPosVolume * mVolume, 0);
            } else {
                handle->SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
            }
            handle->SetPan(mPan);
            if (mKind == KIND_INSECT) {
                mHandles[0].SetFxSend(nw4r::snd::AUX_A, sFxSend);
            }
        }
    }
}

template <int N>
void SoundObj<N>::update(const nw4r::math::VEC3 *pos) {
    if (pos == NULL) {
        return;
    }

    int mode = PARAM_MODE_NORMAL;
    mPos.x = pos->x;
    mPos.y = pos->y;
    mPos.z = pos->z;
    if (mHandles[0].GetId() == SE_LEV_funsui_machi) {
        mode = PARAM_MODE_FOUNTAIN;
    }
    calcParam(&mPos, mode);

    nw4r::snd::SoundHandle *handle = mHandles;
    bool playing = false;
    for (int i = 0; i < N; i++, handle++) {
        if (handle->IsAttachedSound()) {
            playing = true;
            break;
        }
    }
    if (!playing) {
        return;
    }

    if (mPosVolume < 0.0f || mVolume < 0.0f) {
        mPosVolume = clamp(mPosVolume, 0.0f, 1.0f);
        mVolume = clamp(mVolume, 0.0f, 1.0f);
    }

    for (int i = 0; i < N; i++) {
        nw4r::snd::SoundHandle &handle = mHandles[i];
        if (handle.IsAttachedSound()) {
            if (SoundManager::getInstance()->isFixedVolume(handle.GetId())) {
                handle.SetVolume(mPosVolume * mVolume, 0);
            } else {
                handle.SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
            }
            handle.SetPan(mPan);
        }
    }
}

template <int N>
void SoundObj<N>::stopSound(u32 id, int fadeFrames) {
    for (int i = 0; i < N; i++) {
        if (mHandles[i].IsAttachedSound() && id == mHandles[i].GetId()) {
            mHandles[i].Stop(fadeFrames);
        }
    }
}

template <int N>
void SoundObj<N>::stopAll(int fadeFrames) {
    for (int i = 0; i < N; i++) {
        mHandles[i].Stop(fadeFrames);
    }
}
