// Music playback, track transitions, sound effects, and ambient river/rain
// mixing. .text 801CA0F4..801CE58C.
#include <game/snd/snd_system.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_weather.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/nw4r/math.h>

using nw4r::snd::SeqSoundHandle;
using nw4r::snd::SoundStartable;

// Per hour: fair-weather, rain, and snow track masks.
static const u16 sHourlyTracks[24][3] = {
    {0x00FF, 0x01A0, 0x06A4}, {0x07FF, 0x07F8, 0x1FF8},
    {0x007F, 0x0187, 0x07A7}, {0x03FF, 0x1CB8, 0x6CB8},
    {0x009F, 0x00E4, 0x03E4}, {0x002F, 0x00F4, 0x03A4},
    {0x05FF, 0x1A48, 0x7A48}, {0x09FF, 0x1E3C, 0x7E3C},
    {0x447F, 0x05C3, 0xA5C3}, {0x009F, 0x006D, 0x032D},
    {0x04FF, 0x03D5, 0x1BD5}, {0x13FF, 0x0EDC, 0x6EDC},
    {0x00FF, 0x0338, 0x0F3C}, {0x00FF, 0x4FB0, 0x7EB0},
    {0x03FF, 0x0F28, 0x3F2A}, {0x007F, 0x03AC, 0x0FAC},
    {0x07FF, 0x3D70, 0xF570}, {0x02FF, 0x0D5C, 0x3D5C},
    {0x077F, 0x03AC, 0x1B6C}, {0x01FF, 0x0752, 0x1F52},
    {0x007F, 0x00F8, 0x03F8}, {0x1F7F, 0x6BC1, 0xEBC1},
    {0x19FF, 0x1870, 0x1E70}, {0x003F, 0x183C, 0xFC3C},
};

// Entrance, fish, insects, fossils, art, observatory, and observatory menu.
static const u16 sMuseumTracks[] = {0x00E7, 0x0063, 0x0063, 0x1863,
                                    0x007B, 0x0763, 0x0700};

// City music: the same three weather arrangements.
static const u16 sIndoorTracks[4][3] = {
    {0x99FF, 0xC600, 0xE600},
    {0x07FF, 0x3800, 0x7800},
    {0x01FF, 0x1C20, 0x1E20},
    {0x003F, 0x01C0, 0x03C0},
};

// BGM ID and archive group; SEQ_NULL terminates the lookup.
static const u16 sMusicGroups[28][2] = {
    {BGM_OPENING_ARRIVE, 0x0003},
    {BGM_ARBEIT_PRE, 0x0004},
    {BGM_OPENING_TANUKICHI, 0x0005},
    {BGM_ARBEIT, 0x0006},
    {BGM_ATTIC, 0x0007},
    {BGM_COMMUNICATION, 0x0008},
    {BGM_HELLOWEEN, 0x0009},
    {BGM_FANFARE_GET, 0x000A},
    {BGM_FANFARE_COMPLETE_FISH, 0x000B},
    {BGM_FANFARE_COMPLETE_INSECT, 0x000C},
    {BGM_FANFARE_COMPLETE_MONEY, 0x000D},
    {BGM_FANFARE_COMPLETE_ARBEIT, 0x000E},
    {BGM_FANFARE_COMPLETE_COFFEE, 0x000F},
    {BGM_RESET, 0x0010},
    {BGM_BIRTHDAY, 0x0011},
    {BGM_FIREWORKS, 0x0012},
    {BGM_COWNTDOWN_2300, 0x0013},
    {BGM_COWNTDOWN_2330, 0x0014},
    {BGM_COWNTDOWN_2350, 0x0015},
    {BGM_COWNTDOWN_2355, 0x0016},
    {BGM_NEWYEAR_INTRO, 0x0017},
    {BGM_NEWYEAR_LOOP, 0x0018},
    {BGM_NEWYEAR_ALLDAY, 0x0019},
    {BGM_RACKETSAN, 0x001A},
    {BGM_CARNIVAL, 0x001B},
    {BGM_HARVEST, 0x001C},
    {BGM_CHRISTMAS, 0x001D},
    {SEQ_NULL, 0x0081},
};

// Packed biquad type (high three bits) and strength in 0.05 steps.
static const u8 sMusicFilters[] = {0x00, 0x29, 0x6D, 0x6C, 0x8A,
                                   0xAD, 0x44, 0x46, 0x43};

static const f32 sMusicVolumes[] = {0.8f, 1.0f, 1.0f,  1.0f, 1.0f,
                                    1.0f, 1.0f, 0.85f, 1.0f};

static u32 sPendingMusicId;
static u32 sPendingGroupId;
static int sRiverPanIndex;
static f32 sRiverPanHistory[10];

SoundSystem::SoundSystem()
    : _048(0), mFadeInTracks(0), mActiveTracks(0), mWeather(WEATHER_NONE),
      mPendingWeather(WEATHER_NONE), _063(0), mTrackFadeDelay(-1),
      mFadeOutTracks(0), mMuseumTracks(0), mPendingMusicId(0),
      mQueuedWeather(-1), mState(STATE_WAIT), mAllowConductSequence(0),
      mMusicFilterType(0), mMusicFilter(0.0f), mSceneFadeEnabled(false),
      mRestartTick(0), mSceneFadeState(SCENE_FADE_IN), mRestartWait(-1),
      mMenuOpen(false), mTalkOpen(false), _1C0(0), _1C4(0), _1C8(0) {
}

void SoundSystem::init() {
    nw4r::ut::List_Init(&mLists[0], 0x2C);
    nw4r::ut::List_Init(&mLists[1], 0x30);
    nw4r::ut::List_Init(&mLists[2], 0x30);
    for (int i = 0; i < 16; i++) {
        SeqSoundHandle::WriteGlobalVariable(i, 0);
    }
    mMelody.init();
}

void SoundSystem::calc() {
    calcTrackFade();
    if (mMuseumFader.mFrames > 0 && mMuseumTracks != 0 &&
        getMusicId() == BGM_MUSEUM) {
        f32 volume = mMuseumFader.calc();
        SeqSoundHandle handle(&mMusicHandle);
        handle.SetTrackVolume(mMuseumTracks, volume);
    } else if (mMuseumFader.mFrames <= 0 && mMuseumTracks != 0) {
        mMuseumTracks = 0;
    }
    mVoice.calc();
    mMelody.calc();
    mConductorKK.calc();
    mConductor.calc();
    mAmbient.calc();
    if (SoundManager::getInstance()->_778 == SCENE_FIELD ||
        SoundManager::getInstance()->_778 == SCENE_DM_TITLE) {
        calcRiverSound();
    }
    if (mPendingMusicId != 0) {
        if (SoundManager::getInstance()->isGroupReady(mPendingGroupId)) {
            if (sPendingMusicId == 0) {
                startMusic(mPendingMusicId, 0);
            }
            mPendingMusicId = 0;
            mPendingGroupId = 0;
        }
    } else if (sPendingMusicId != 0) {
        if (SoundManager::getInstance()->mLoader.mState == 0) {
            startMusic(sPendingMusicId, 0);
            sPendingMusicId = 0;
            sPendingGroupId = 0;
        }
    }
    if (mState == STATE_WAIT) {
        if (getMusicId() == STRM_TITLE) {
            mState = STATE_LOADING;
        }
    } else if (mState == STATE_LOADING) {
        if (!isTitleMusic()) {
            mState = STATE_READY;
            SoundManager::getInstance()->setObjectVolume(0.0f, 0);
            SoundManager::getInstance()->setObjectVolume(1.0f, 120);
        }
    } else if (mState == STATE_WAIT_TITLE_END) {
        if (SoundManager::getInstance()->_778 != SCENE_DM_TITLE) {
            mState = STATE_READY;
        }
    } else {
        mState = STATE_READY;
    }
    if (getMusicId() >= BGM_MU01_SANBIKA && getMusicId() <= BGM_MU75_SONATA) {
        mMusicHandle.SetBiquadFilter(mMusicFilterType, mMusicFilter);
    }
    if (mSceneFadeEnabled) {
        if (mRestartWait > 0) {
            if (--mRestartWait == 0) {
                startMusic(BGM_KAPPEIBUS, 0);
            }
        } else {
            mSceneFader.calc();
            if (mMusicHandle.IsAttachedSound()) {
                mMusicHandle.SetVolume(getSceneVolume(), 0);
                mMusicHandle.SetPan(getScenePan());
            }
        }
    }
    if (mMusicStopped && !mMusicHandle.IsAttachedSound()) {
        mMusicStopped = false;
        mConductor.resetHaniwa();
    }
}

int SoundSystem::getRandom(int max) {
    return cM::rndInt(max);
}
void SoundSystem::playSe(int se) {
    playSe(se, 0.0f);
}
void SoundSystem::playSe(int se, f32 pan) {
    if (mState != STATE_READY) {
        u32 player = SoundManager::getInstance()->getSoundPlayerId(se);
        if (player == 11 || player == 14 || player == 20) {
            return;
        }
    }
    if (SoundManager::getInstance()->startSound(&mHandle, se)) {
        if (pan > 1.0f) {
            pan = 1.0f;
        } else if (pan < -1.0f) {
            pan = -1.0f;
        }
        mHandle.SetPan(pan);
    }
}
void SoundSystem::holdSe(int se) {
    holdSe(se, 0.0f);
}
void SoundSystem::holdSe(int se, f32 pan) {
    SystemLevHandle *handle = findLevelHandle(se);
    SoundManager *manager = SoundManager::getInstance();
    if (handle != NULL) {
        manager->holdSound(handle, se);
        handle->SetPan(pan);
    } else {
        handle = getFreeLevelHandle();
        if (handle != NULL) {
            manager->holdSound(handle, se);
            handle->SetPan(pan);
        }
    }
}

void SoundSystem::startMusic(u32 id, int tickOffset) {
    mMusicStopped = false;
    if (mPendingMusicId != 0 && id == SEQ_CONDUCT) {
        return;
    }
    if (!mAllowConductSequence && id == SEQ_CONDUCT) {
        return;
    }
    if (mSceneFadeEnabled) {
        if (mMusicHandle.IsAttachedSound()) {
            return;
        }
        if (mRestartWait > 0) {
            return;
        }
        if (mRestartWait < 0 && mRestartTick > 0) {
            mRestartWait = 1200;
            return;
        }
        if ((mRestartWait >= 0 || mRestartTick != 0) && mRestartWait == 0) {
            tickOffset = mRestartTick;
            mRestartWait = -1;
        }
    }
    if (isMusicPlaying()) {
        stopMusic(0);
    }
    if (id != STRM_TITLE) {
        if (!SoundManager::getInstance()->isSeqReady(id)) {
            u32 group = 0;
            if (id >= BGM_OUTDOOR_00 && id <= BGM_OUTDOOR_23) {
                group = id - BGM_MU01_SANBIKA + 30;
            } else if (id >= BGM_MU01_SANBIKA && id <= BGM_MU75_SONATA) {
                group = id - BGM_MU01_SANBIKA + 30;
            } else {
                for (int i = 0; sMusicGroups[i][0] != SEQ_NULL; i++) {
                    if (id == sMusicGroups[i][0]) {
                        group = sMusicGroups[i][1];
                        break;
                    }
                }
            }
            if (group == 0) {
                return;
            }
            if (!SoundManager::getInstance()->requestMusicGroup(group)) {
                sPendingMusicId = id;
                sPendingGroupId = group;
                return;
            }
            mPendingMusicId = id;
            mPendingGroupId = group;
            return;
        }
    }
    mPendingMusicId = 0;
    mPendingGroupId = 0;
    if (id >= BGM_KK00_TEST && id <= BGM_KK78_DOUBUTSUNOSHIMA) {
        mConductorKK.reset();
    }
    if (tickOffset != 0) {
        SoundStartable::StartInfo info;
        info.seqSoundInfo.seqDataAddress = NULL;
        info.seqSoundInfo.startLocationLabel = NULL;
        info.enableFlag = SoundStartable::StartInfo::ENABLE_START_OFFSET;
        info.startOffsetType =
            SoundStartable::StartInfo::START_OFFSET_TYPE_TICK;
        info.startOffset = tickOffset;
        SoundManager::getInstance()->startSound(&mMusicHandle, id, &info);
    } else {
        SoundManager::getInstance()->startSound(&mMusicHandle, id);
        if (id == BGM_MUSEUM) {
            int weather = TRACK_INIT_MUSEUM_ENTRANCE;
            switch (SoundManager::getInstance()->_778) {
            case SCENE_RM_MM_ENT:
                break;
            case SCENE_RM_MM_FISH:
                weather = TRACK_INIT_MUSEUM_FISH;
                break;
            case SCENE_RM_MM_INS:
                weather = TRACK_INIT_MUSEUM_INSECTS;
                break;
            case SCENE_RM_MM_FOSSIL0:
            case SCENE_RM_MM_FOSSIL1:
                weather = TRACK_INIT_MUSEUM_FOSSILS;
                break;
            case SCENE_RM_MM_PIC0:
            case SCENE_RM_MM_PIC1:
                weather = TRACK_INIT_MUSEUM_ART;
                break;
            case SCENE_RM_MM_CAFE:
                break;
            case SCENE_RM_MM_ASTRO:
                weather = TRACK_INIT_MUSEUM_OBSERVATORY;
                break;
            }
            setWeather(weather);
        }
    }
    if (mQueuedWeather > 0) {
        setWeather(mQueuedWeather);
        mQueuedWeather = -1;
    }
    if (mSceneFadeEnabled) {
        if (mSceneFadeState == SCENE_FADE_OUT) {
            mSceneFader.set(0.0f);
        } else {
            mSceneFader.set(1.0f);
        }
        mMusicHandle.SetVolume(getSceneVolume(), 0);
        mMusicHandle.SetPan(getScenePan());
    }
    if (id >= BGM_OUTDOOR_00 && id <= BGM_OUTDOOR_23) {
        mMusicHandle.SetFxSend(nw4r::snd::AUX_A, 0.25f);
    } else if (id >= BGM_TOWN_EARLY && id <= BGM_TOWN_MIDNIGHT) {
        mMusicHandle.SetFxSend(nw4r::snd::AUX_A, 0.25f);
        SeqSoundHandle handle(&mMusicHandle);
        handle.SetTrackVolume(0x8000, 0.0f);
    } else if (id >= BGM_KK00_TEST && id <= BGM_KK78_DOUBUTSUNOSHIMA) {
        mMusicHandle.SetFxSend(nw4r::snd::AUX_A, 0.28f);
    }
    mConductor.start(id);
}

void SoundSystem::stopMusic(int fade) {
    if (mSceneFadeEnabled) {
        SeqSoundHandle handle(&mMusicHandle);
        if (handle.IsAttachedSound()) {
            u32 tick = handle.GetTick();
            tick %= 0x4800;
            tick = tick / 0x180 * 0x180;
            if (tick == 0) {
                tick = 0x180;
            }
            mRestartTick = tick;
        }
        mRestartWait = -1;
    }
    if (mMusicHandle.IsAttachedSound()) {
        mMusicHandle.Stop(fade);
        mMusicStopped = true;
    }
    mPendingWeather = WEATHER_NONE;
    mQueuedWeather = -1;
    _063 = 0;
    mTrackFadeDelay = -1;
    mFadeInTracks = 0xFFFF;
    mFadeOutTracks = 0;
    mTrackFader.set(0.0f);
    mMuseumFader.set(0.0f);
    mMuseumTracks = 0;
    if (mState == STATE_LOADING) {
        mState = STATE_WAIT_TITLE_END;
    }
    if (mPendingMusicId != 0) {
        mPendingMusicId = 0;
    }
}
void SoundSystem::pauseMusic(bool pause) {
    mMusicHandle.Pause(pause, 0);
}
BOOL SoundSystem::isMusicPlaying() {
    return mMusicHandle.IsAttachedSound();
}
u32 SoundSystem::getMusicId() {
    u32 id = SEQ_NULL;
    if (isMusicPlaying()) {
        id = mMusicHandle.GetId();
    }
    return id;
}
int SoundSystem::getMusicTick() {
    int tick = 0;
    if (mMusicHandle.IsAttachedSound()) {
        SeqSoundHandle handle(&mMusicHandle);
        if (handle.IsAttachedSound()) {
            tick = handle.GetTick();
        }
    }
    return tick;
}

void SoundSystem::setMusicPosition(const mVec3_c *pos) {
    u32 id = getMusicId();
    if (id >= BGM_MU01_SANBIKA && id <= BGM_MU75_SONATA) {
        if (pos != NULL) {
            f32 volume = (695.0f - PSVECMag((const Vec *)pos)) / 345.0f;
            if (volume > 1.0f) {
                volume = 1.0f;
            } else if (volume < 0.5f) {
                volume = 0.5f;
            }
            f32 pan = pos->x / 196.0f;
            if (pan > 1.0f) {
                pan = 1.0f;
            } else if (pan < -1.0f) {
                pan = -1.0f;
            }
            mMusicHandle.SetVolume(volume * mMusicVolume, 0);
            mMusicHandle.SetPan(pan);
        }
    } else if (id >= BGM_TOWN_EARLY && id <= BGM_TOWN_MIDNIGHT) {
        if (pos != NULL) {
            SeqSoundHandle handle(&mMusicHandle);
            Vec p = *(const Vec *)pos;
            p.z += 275.0f;
            p.y = 0.0f;
            f32 volume = (310.0f - PSVECMag(&p)) / 240.0f;
            if (volume > 1.0f) {
                volume = 1.0f;
            } else if (volume < 0.0f) {
                volume = 0.0f;
            }
            f32 cosine = nw4r::math::CosRad(volume * 3.1415927f);
            volume = (-cosine + 1.0f) / 2.0f;
            f32 pan = p.x / 270.0f * 1.4f;
            if (pan > 1.0f) {
                pan = 1.0f;
            } else if (pan < -1.0f) {
                pan = -1.0f;
            }
            handle.SetTrackVolume(0x8000, volume);
            handle.SetTrackPan(0x8000, pan);
        } else {
            SeqSoundHandle handle(&mMusicHandle);
            handle.SetTrackVolume(0x8000, 0.0f);
        }
    }
}
BOOL SoundSystem::isTitleMusic() {
    BOOL result = FALSE;
    if (getMusicId() == STRM_TITLE) {
        result = TRUE;
    }
    return result;
}
void SoundSystem::setMusicFilter(int type) {
    u8 filter = sMusicFilters[type];
    mMusicFilterType = filter >> 5;
    mMusicFilter = (filter & 31) * 0.05f;
    f32 value = mMusicFilter;
    if (value > 1.0f) {
        value = 1.0f;
    } else if (value < 0.0f) {
        value = 0.0f;
    }
    mMusicFilter = value;
    mMusicVolume = sMusicVolumes[type];
}
u32 SoundSystem::getMusicPlayerBgm(int song) {
    u32 id = BGM_MU01_SANBIKA;
    if (song >= 1 && song <= 75) {
        id = song + SEQ_CONDUCT;
    } else if (song == 0) {
        id = SEQ_CONDUCT;
        mAllowConductSequence = true;
    }
    return id;
}
u32 SoundSystem::getKKSongBgm(int song) {
    u32 id = BGM_KK01_SANBIKA;
    if (song >= 1 && song <= 78) {
        id = song + BGM_KK00_TEST;
    }
    return id;
}

void SoundSystem::setWeather(int weather) {
    if (getMusicId() != BGM_MUSEUM) {
        mWeather = weather;
    }
    if (mTrackFadeDelay > 0 || mTrackFader.mFrames > 0) {
        _063 = 0;
    } else {
        mPendingWeather = weather;
        if (!mMusicHandle.IsAttachedSound()) {
            mQueuedWeather = weather;
        } else {
            u32 id = mMusicHandle.GetId();
            const u16 *tracks = NULL;
            if (id >= BGM_OUTDOOR_00 && id <= BGM_OUTDOOR_23) {
                tracks = sHourlyTracks[id - BGM_OUTDOOR_00];
            } else if (id != BGM_MUSEUM) {
                if (id >= BGM_TOWN_EARLY && id <= BGM_TOWN_MIDNIGHT) {
                    tracks = sIndoorTracks[id - BGM_TOWN_EARLY];
                } else {
                    return;
                }
            }
            mTrackFadeDelay = -1;
            mTrackFader.fadeTo(0.0f, 0);
            u16 mask = 0xFF;
            bool mute = false;
            u16 from = 0;
            u16 to = 0;
            if (mPendingWeather >= TRACK_INIT_MUSEUM_ENTRANCE &&
                mPendingWeather <= TRACK_INIT_MUSEUM_MENU) {
                mute = true;
                mask =
                    sMuseumTracks[mPendingWeather - TRACK_INIT_MUSEUM_ENTRANCE];
                mActiveTracks =
                    sMuseumTracks[mPendingWeather - TRACK_INIT_MUSEUM_ENTRANCE];
            } else if (mPendingWeather >= WEATHER_FAIR &&
                       mPendingWeather <= WEATHER_SNOW) {
                mute = true;
                mask = tracks[mPendingWeather - WEATHER_FAIR];
            } else if (mPendingWeather >= TRACK_MUSEUM_ENTRANCE &&
                       mPendingWeather <= TRACK_MUSEUM_MENU) {
                mTrackFadeDelay = 5;
                from = mActiveTracks;
                to = sMuseumTracks[mPendingWeather - TRACK_MUSEUM_ENTRANCE];
            } else if (mPendingWeather == WEATHER_FAIR_TO_RAIN) {
                mute = true;
                mTrackFadeDelay = 600;
                from = tracks[0];
                mask = from;
                to = tracks[1];
            } else if (mPendingWeather == WEATHER_RAIN_TO_FAIR) {
                mute = true;
                mTrackFadeDelay = 600;
                from = tracks[1];
                mask = from;
                to = tracks[0];
            } else {
                mute = true;
                mTrackFadeDelay = 600;
                from = tracks[0];
                mask = from;
                to = tracks[2];
            }
            if (mTrackFadeDelay > 0) {
                mActiveTracks = to;
                u16 invertTo = ~to;
                u16 invertFrom = ~from;
                mFadeInTracks = to & (from ^ to);
                mFadeOutTracks = invertTo & (invertFrom ^ invertTo);
            }
            if (mute) {
                {
                    SeqSoundHandle handle(&mMusicHandle);
                    handle.SetTrackVolume(~mask, 0.0f);
                }
                _063 = 0;
            }
        }
    }
}

SystemLevHandle *SoundSystem::findLevelHandle(int id) {
    SystemLevHandle *found = NULL;
    for (int i = 0; i < 4; i++) {
        if (mLevHandles[i].IsAttachedSound() && id == mLevHandles[i].GetId()) {
            found = &mLevHandles[i];
        }
    }
    return found;
}
SystemLevHandle *SoundSystem::getFreeLevelHandle() {
    SystemLevHandle *handle = NULL;
    for (int i = 0; i < 4; i++) {
        if (!mLevHandles[i].IsAttachedSound()) {
            handle = &mLevHandles[i];
            break;
        }
    }
    return handle;
}
void SoundSystem::calcTrackFade() {
    int stage = SoundManager::getInstance()->_778;
    u32 id = getMusicId();
    if (((id >= BGM_OUTDOOR_00 && id <= BGM_OUTDOOR_23) || id == BGM_MUSEUM ||
         (id >= BGM_TOWN_EARLY && id <= BGM_TOWN_MIDNIGHT)) &&
        (mTrackFadeDelay >= 0 || mTrackFader.mFrames > 0)) {
        if (mTrackFadeDelay > 0) {
            mTrackFadeDelay--;
        } else if (mTrackFadeDelay == 0) {
            mTrackFader.set(0.0f);
            mTrackFader.fadeTo(1.0f,
                               mPendingWeather < WEATHER_MODE_END ? 1800 : 60);
            mTrackFadeDelay = -1;
        } else if (mTrackFader.mFrames > 0) {
            u16 fadeIn = mFadeInTracks;
            u16 fadeOut = mFadeOutTracks;
            if (stage == SCENE_TOWN) {
                fadeIn &= 0x7FFF;
                fadeOut &= 0x7FFF;
            }
            f32 volume = mTrackFader.calc();
            SeqSoundHandle handle(&mMusicHandle);
            if (handle.IsAttachedSound() && !mMusicStopped) {
                handle.SetTrackVolume(fadeIn, volume);
                handle.SetTrackVolume(fadeOut, 1.0f - volume);
            }
            if (mTrackFader.mFrames <= 0) {
                finishTrackFade();
            }
        }
    }
}
const s8 *SoundSystem::getKKInfo() {
    return &mConductorKK.mAnim.mPhase;
}
BOOL SoundSystem::isKKMusic() {
    BOOL result = FALSE;
    if (getMusicId() >= BGM_KK00_TEST &&
        getMusicId() <= BGM_KK78_DOUBUTSUNOSHIMA) {
        result = TRUE;
    }
    return result;
}
void SoundSystem::playMelody(const u8 *notes) {
    mMelody.play(SE_MELODY_EDIT, notes);
}
void SoundSystem::playJingle(u32 idx) {
    mMelody.playJingle(idx);
}
void SoundSystem::playTune(u32 id) {
    mMelody.play(id, NULL);
}
void SoundSystem::playChime() {
    mMelody.playChime();
}
void SoundSystem::playMelodyAt(const mVec3_c *pos, int arg) {
    mMelody.playAt(pos, arg);
}
void SoundSystem::finishTrackFade() {
    if (getMusicId() >= BGM_OUTDOOR_00 && getMusicId() <= BGM_OUTDOOR_23) {
        switch (mWeather) {
        case WEATHER_FAIR_TO_RAIN:
            mWeather = WEATHER_RAIN;
            break;
        case WEATHER_RAIN_TO_FAIR:
        case WEATHER_SNOW_TO_FAIR:
            mWeather = WEATHER_FAIR;
            break;
        case WEATHER_FAIR_TO_SNOW:
            mWeather = WEATHER_SNOW;
            break;
        }
    }
}
void SoundSystem::speakVoice(int mode, int tone, u16 c, u16 next, f32 volume) {
    if (mode != mVoice.mMode) {
        mVoice.setMode(mode);
    }
    if (tone != mVoice.mTone) {
        mVoice.setTone(tone);
    }
    mVoice.speak(c, next, volume);
}
void SoundSystem::setVoiceType(int type) {
    mVoice.setVoiceType(type);
}
void SoundSystem::appendAmbientObj(SoundObjBase *obj) {
    nw4r::ut::List *list = NULL;
    switch (obj->mKind) {
    case SoundObjBase::KIND_RIVER:
        list = &mLists[0];
        break;
    case SoundObjBase::KIND_POND:
        list = &mLists[1];
        break;
    case SoundObjBase::KIND_FALL:
        list = &mLists[2];
        break;
    }
    nw4r::ut::List_Append(list, obj);
}
void SoundSystem::removeAmbientObj(SoundObjBase *obj) {
    nw4r::ut::List *list = NULL;
    switch (obj->mKind) {
    case SoundObjBase::KIND_RIVER:
        list = &mLists[0];
        break;
    case SoundObjBase::KIND_POND:
        list = &mLists[1];
        break;
    case SoundObjBase::KIND_FALL:
        list = &mLists[2];
        break;
    }
    nw4r::ut::List_Remove(list, obj);
}

void SoundSystem::calcRiverSound() {
    if (mState != STATE_READY || !SoundObjBase::sIsPlayable) {
        return;
    }
    struct River {
        SoundObjBase *obj;
        f32 distance;
    };
    River rivers[10];
    for (int i = 0; i < 10; i++) {
        rivers[i].obj = NULL;
        rivers[i].distance = -1.0f;
    }
    SoundObjBase *obj = NULL;
    while ((obj = (SoundObjBase *)nw4r::ut::List_GetNext(&mLists[0], obj)) !=
           NULL) {
        nw4r::math::VEC3 pos;
        pos.x = obj->mPos.x;
        pos.y = obj->mPos.y;
        pos.z = obj->mPos.z;
        pos.y = (pos.y + 310.0f) * 2.0f;
        pos.z += 220.0f;
        f32 distance = PSVECMag(pos);
        int i;
        for (i = 0; i < 10; i++) {
            if (rivers[i].distance < distance && rivers[i].distance > 0.0f) {
                break;
            }
        }
        if (i <= 0) {
            continue;
        }
        for (int j = 0; j < i - 1; j++) {
            rivers[j].distance = rivers[j + 1].distance;
            rivers[j].obj = rivers[j + 1].obj;
        }
        rivers[i - 1].distance = distance;
        rivers[i - 1].obj = obj;
    }
    SoundObj<1> *fall = NULL;
    f32 fallVolume = 0.0f;
    while ((fall = (SoundObj<1> *)nw4r::ut::List_GetNext(&mLists[2], fall)) !=
           NULL) {
        f32 volume = fall->getPosVolume();
        if (volume > fallVolume) {
            fallVolume = volume;
        }
    }
    f32 pondVolume = 0.0f;
    SoundObj<1> *pond = NULL;
    while ((pond = (SoundObj<1> *)nw4r::ut::List_GetNext(&mLists[1], pond)) !=
           NULL) {
        f32 volume = pond->getPosVolume();
        if (volume > 0.0f) {
            if (volume > pondVolume) {
                pondVolume = volume;
            }
            f32 attenuation = fallVolume * 0.6f;
            attenuation = 1.0f - attenuation;
            pond->setVolume(attenuation);
        }
    }
    f32 volume = 0.0f;
    if (rivers[9].distance > 0.0f) {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        f32 total = 0.0f;
        for (int i = 0; i < 10; i++) {
            if (0.0f < rivers[i].distance && rivers[i].distance < 350.0f) {
                total += rivers[i].distance;
            }
        }
        for (int i = 0; i < 10; i++) {
            if (0.0f < rivers[i].distance && rivers[i].distance < 350.0f) {
                pos += rivers[i].obj->mPos *
                       ((350.0f - rivers[i].distance) / total);
            }
        }
        volume = (350.0f - rivers[9].distance) / 300.0f;
        if (volume > 1.0f) {
            volume = 1.0f;
        } else if (volume < 0.0f) {
            volume = 0.0f;
        }
        fallVolume *= 0.9f;
        volume = volume * (1.0f - fallVolume) * SoundObjBase::getMasterVolume();
        f32 distance = nw4r::math::FSqrt(
            pos.y * pos.y / 25.0f +
            (pos.x * pos.x + (pos.z - -125.0f) * (pos.z - -125.0f)));
        if (distance > 0.0f) {
            f32 pan = pos.x / distance;
            if (pan > 1.0f) {
                pan = 1.0f;
            } else if (pan < -1.0f) {
                pan = -1.0f;
            }
            sRiverPanHistory[sRiverPanIndex] = pan;
        } else {
            sRiverPanHistory[sRiverPanIndex] = 0.0f;
        }
        sRiverPanIndex++;
        if (sRiverPanIndex >= 10) {
            sRiverPanIndex = 0;
        }
        f32 pan = 0.0f;
        for (int i = 0; i < 10; i++) {
            pan += sRiverPanHistory[i];
        }
        pan /= 10.0f;
        SoundManager::getInstance()->holdSound(&mRiverHandle, SE_LEV_river);
        mRiverHandle.SetVolume(volume, 0);
        mRiverHandle.SetPan(pan);
    }
}

void SoundSystem::setSeaDist(f32 dist) {
    mAmbient.mSeaDist = dist;
}
void SoundSystem::setMelodyVolume(f32 volume) {
    mMelody.mPart0.mVolume = volume;
}
void SoundSystem::setMelodyFilter(f32 filter) {
    mMelody.mPart0.mFilter = filter;
}
void SoundSystem::fadeOut() {
    SoundManager::getInstance()->setMasterVolume(0.0f, 300);
}
void SoundSystem::fadeIn() {
    SoundManager::getInstance()->setMasterVolume(1.0f, 300);
}
void SoundSystem::enableSceneFade() {
    if (!mSceneFadeEnabled) {
        mSceneFadeEnabled = true;
        mRestartWait = -1;
        mRestartTick = 0;
        mSceneFader.set(0.0f);
    }
}
void SoundSystem::disableSceneFade() {
    if (mSceneFadeEnabled) {
        mSceneFadeEnabled = false;
    }
}
f32 SoundSystem::getSceneVolume() {
    f32 volume = mSceneFader.mValue;
    volume = 0.55f + 0.45f * volume;
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.0f) {
        volume = 0.0f;
    }
    return volume;
}
f32 SoundSystem::getScenePan() {
    f32 pan = mSceneFader.mValue;
    pan = 0.9f + -0.9f * pan;
    if (pan > 1.0f) {
        pan = 1.0f;
    } else if (pan < 0.0f) {
        pan = 0.0f;
    }
    return pan;
}
void SoundSystem::setSceneFade(int state) {
    if (mSceneFadeEnabled) {
        if (state == SCENE_FADE_OUT) {
            mSceneFader.fadeTo(0.0f, 120);
        } else {
            mSceneFader.fadeTo(1.0f, 120);
        }
        mSceneFadeState = state;
    }
}
void SoundSystem::menuOpen() {
    mMenuOpen = true;
    SoundManager::getInstance()->setObjectVolume(0.2f, 30);
}
void SoundSystem::menuClose() {
    mMenuOpen = false;
    f32 volume = 1.0f;
    if (mTalkOpen) {
        volume = 0.3125f;
    }
    SoundManager::getInstance()->setObjectVolume(volume, 15);
}
void SoundSystem::talkStart() {
    mTalkOpen = true;
    if (!mMenuOpen) {
        SoundManager::getInstance()->setObjectVolume(0.3125f, 30);
    }
}
void SoundSystem::talkEnd() {
    mTalkOpen = false;
    SoundManager::getInstance()->setObjectVolume(1.0f, 15);
}

void SoundSystem::setIndoorRain(f32 blend) {
    f32 rain = 0.0f;
    if (0.0f <= blend && blend <= 1.0f && SoundAmbient::sIsStageSet) {
        u32 weather = 7;
        if (dWeather_c::sInstance != NULL) {
            rain = dWeather_c::sInstance->mRainLevel;
            weather = dWeather_c::sInstance->_5884;
        }
        if ((u32)(weather - 3) <= 1 && rain > 0.0f) {
            SoundManager *manager = SoundManager::getInstance();
            BOOL started = FALSE;
            if (!mIndoorRainHandle.IsAttachedSound()) {
                started = TRUE;
            }
            manager->holdSound(&mIndoorRainHandle, SE_LEV_Rain);
            manager->holdSound(&mIndoorHeavyRainHandle, SE_LEV_HeavyRain);
            f32 volume = blend * 0.58f + 0.3f;
            if (volume > 1.0f) {
                volume = 1.0f;
            } else if (volume < 0.0f) {
                volume = 0.0f;
            }
            f32 filter = (1.0f - blend) * 0.65f;
            if (filter > 1.0f) {
                filter = 1.0f;
            } else if (filter < 0.0f) {
                filter = 0.0f;
            }
            f32 lightVolume = 0.0f;
            f32 heavyVolume = lightVolume;
            if (0.0f < rain && rain <= 0.5f) {
                lightVolume = rain * 2.0f;
            } else if (0.5f < rain && rain <= 1.0f) {
                heavyVolume = (rain - 0.5f) * 2.0f;
                lightVolume = 1.0f - heavyVolume;
            }
            SeqSoundHandle light(&mIndoorRainHandle);
            light.SetTrackVolume(0xFFFF, lightVolume * volume);
            SeqSoundHandle heavy(&mIndoorHeavyRainHandle);
            heavy.SetTrackVolume(0xFFFF, heavyVolume * volume);
            if (started) {
                mIndoorRainHandle.SetVolume(0.0f, 0);
                mIndoorHeavyRainHandle.SetVolume(0.0f, 0);
                mIndoorRainHandle.SetVolume(1.0f, 120);
                mIndoorHeavyRainHandle.SetVolume(1.0f, 120);
            }
            mIndoorRainHandle.SetBiquadFilter(3, filter);
            mIndoorHeavyRainHandle.SetBiquadFilter(3, filter);
        } else {
            if (mIndoorRainHandle.IsAttachedSound()) {
                mIndoorRainHandle.Stop(120);
                mIndoorHeavyRainHandle.Stop(120);
            }
        }
    } else {
        if (mIndoorRainHandle.IsAttachedSound()) {
            mIndoorRainHandle.Stop(120);
            mIndoorHeavyRainHandle.Stop(120);
        }
    }
}
void SoundSystem::fadeOutMuseum() {
    int stage = SoundManager::getInstance()->_778;
    u16 tracks = 0;
    switch (stage) {
    case SCENE_RM_MM_ENT:
        tracks = 0x84;
        break;
    case SCENE_RM_MM_PIC0:
        tracks = 0x18;
        break;
    case SCENE_RM_MM_FOSSIL0:
        tracks = 0x1800;
        break;
    case SCENE_RM_MM_ASTRO:
        tracks = 0x700;
        break;
    }
    if (tracks != 0) {
        f32 volume = 1.0f;
        if (mMuseumFader.mFrames > 0) {
            volume = mMuseumFader.mValue;
        }
        mMuseumTracks = tracks;
        mMuseumFader.set(volume);
        mMuseumFader.fadeTo(0.0f, 45);
    }
}
void SoundSystem::fadeInMuseum() {
    int stage = SoundManager::getInstance()->_778;
    u16 tracks = 0;
    switch (stage) {
    case SCENE_RM_MM_ENT:
        tracks = 0x84;
        break;
    case SCENE_RM_MM_PIC0:
        tracks = 0x18;
        break;
    case SCENE_RM_MM_FOSSIL0:
        tracks = 0x1800;
        break;
    case SCENE_RM_MM_ASTRO:
        tracks = 0x700;
        break;
    }
    if (tracks != 0) {
        mMuseumTracks = tracks;
        mMuseumFader.set(0.0f);
        mMuseumFader.fadeTo(1.0f, 45);
    }
}

// Keep the weak definitions together, in their original link order.
DECL_WEAK SystemLevHandle::SystemLevHandle() : _08(0), _0C(0) {}

DECL_WEAK u32 SoundSystem::getSoundHeapSize() {
    return 0xE00000;
}
