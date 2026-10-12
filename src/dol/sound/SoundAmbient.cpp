// The rain and sea ambience. .text 801E2E58..801E39B0, .bss 80625DD8..80625E00, .sbss 8074EC68..8074EC70,
// .sdata2 80751D18..80751D50. See include/game/snd/snd_ambient.hpp.
#include <game/snd/snd_ambient.hpp>
#include <game/snd/snd_system.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_weather.hpp>
#include <lib/nw4r/math.h>

bool SoundAmbient::sIsStageSet;

// 801E2E58
void SoundAmbient::calc() {
    if (SoundSystem::getInstance()->mState != SoundSystem::STATE_READY) {
        return;
    }

    dWeather_c *weather = dWeather_c::sInstance;
    f32 rain = 0.0f;
    if (weather != NULL) {
        mWeather = weather->_5884;
        rain = weather->mRainLevel;
    }
    calcSea();
    if (mWeather != 3 && mWeather != 4) {
        return;
    }

    int stage = SoundManager::getInstance()->_778;
    u32 lightId = 0;
    u32 heavyId = 0;
    BOOL started = FALSE;
    if (!sIsStageSet) {
        for (int i = 0; i < 2; i++) {
            if (mRainHandles[i].IsAttachedSound()) {
                mRainHandles[i].Stop(60);
            }
        }
        return;
    }

    if (stage == 0 || stage == 0x2D || stage == 0x38) {
        lightId = SE_LEV_Rain;
        heavyId = SE_LEV_HeavyRain;
    } else if ((stage >= 1 && stage <= 0x1A) || stage == 0x1B || stage == 0x27 || stage == 0x2C || stage == 0x2F ||
               stage == 0x33) {
        lightId = SE_LEV_RainRoom;
        heavyId = SE_LEV_HeavyRainRoom;
    }

    if (lightId != 0 && rain > 0.0f) {
        if (!mRainHandles[0].IsAttachedSound()) {
            started = TRUE;
        }
        SoundManager *manager = SoundManager::getInstance();
        manager->holdSound(&mRainHandles[0], lightId);
        manager->holdSound(&mRainHandles[1], heavyId);

        f32 lightVolume = 0.0f;
        f32 heavyVolume = lightVolume;
        if (0.0f < rain && rain <= 0.5f) {
            lightVolume = 2.0f * rain;
        } else if (0.5f < rain && rain <= 1.0f) {
            heavyVolume = 2.0f * (rain - 0.5f);
            lightVolume = 1.0f - heavyVolume;
        }

        nw4r::snd::SeqSoundHandle light(&mRainHandles[0]);
        light.SetTrackVolume(0xFFFF, lightVolume);
        nw4r::snd::SeqSoundHandle heavy(&mRainHandles[1]);
        heavy.SetTrackVolume(0xFFFF, heavyVolume);
        if (started) {
            for (int i = 0; i < 2; i++) {
                mRainHandles[i].SetVolume(0.0f, 0);
                mRainHandles[i].SetVolume(1.0f, 600);
            }
        }
    }
}

// 801E3380
void SoundAmbient::calcSea() {
    int stage = SoundManager::getInstance()->_778;
    if (mSeaDist < 0.0f) {
        mSeaDist = 0.0f;
    }

    BOOL started = FALSE;
    if (!mSeaHandle.IsAttachedSound()) {
        started = TRUE;
    }

    if (!sIsStageSet) {
        if (mSeaHandle.IsAttachedSound()) {
            mSeaHandle.Stop(30);
        }
    } else if (stage == 0 || stage == 0x38) {
        if (mSeaDist < 400.0f) {
            SoundManager::getInstance()->holdSound(&mSeaHandle, getSeaSoundId());
            f32 volume = getSeaVolume();
            nw4r::snd::SeqSoundHandle sea(&mSeaHandle);
            if (sea.IsAttachedSound()) {
                sea.SetTrackVolume(0xFFFF, volume);
            }
            mSeaHandle.SetBiquadFilter(1, 0.0f);
        }
    } else if (stage >= 1 && stage <= 0x1A) {
        if (mSeaDist < 400.0f) {
            SoundManager::getInstance()->holdSound(&mSeaHandle, getSeaSoundId());
            f32 volume = getSeaVolume();
            nw4r::snd::SeqSoundHandle sea(&mSeaHandle);
            if (sea.IsAttachedSound()) {
                sea.SetTrackVolume(0xFFFF, 0.5f * volume);
            }
            mSeaHandle.SetBiquadFilter(1, 0.63f);
        }
    } else {
        if (mSeaHandle.IsAttachedSound()) {
            mSeaHandle.Stop(60);
        }
        mSeaDist = 1000.0f;
    }

    if (started && mSeaHandle.IsAttachedSound()) {
        mSeaHandle.SetVolume(0.0f, 0);
        mSeaHandle.SetVolume(1.0f, 180);
    }
}

// 801E3914
u32 SoundAmbient::getSeaSoundId() {
    u32 id = SE_LEV_sea;
    if (mWeather == 3) {
        id = SE_LEV_sea_Rain;
    } else if (mWeather == 4) {
        id = SE_LEV_sea_RainHeavy;
    }
    return id;
}

// 801E393C
f32 SoundAmbient::getSeaVolume() {
    f32 volume = 0.5 + 0.5f * nw4r::math::CosRad(NW4R_MATH_PI * (mSeaDist / 400.0f));
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.1f) {
        volume = 0.1f;
    }
    return volume;
}
