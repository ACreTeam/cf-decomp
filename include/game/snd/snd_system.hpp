#pragma once

// The game's sound system (dol/sound/SoundSystem.cpp, 801CA0F4..801CE58C). No
// RTTI; the class name comes from the split. Owns music, level sound effects,
// melody/voice players, and ambient mixing. The inline singleton's instance is
// 80564964, guarded by 8074E149. Names without RTTI are inferred.

#include <types.h>
#include <lib/nw4r/snd.h>
#include <lib/nw4r/ut/ut_list.h>
#include <game/snd/snd_conductor.hpp>
#include <game/snd/snd_fader.hpp>
#include <game/snd/snd_voice.hpp>
#include <game/snd/snd_ambient.hpp>

class mVec3_c;
class SoundObjBase;

// RTTI "SystemLevHandle" (vtable 80500594; base nw4r::snd::SoundHandle). A
// sound handle with a level; the weak ctor copy at 801CE564 is in SoundSystem.cpp.
class SystemLevHandle : public nw4r::snd::SoundHandle {
  public:
    SystemLevHandle(); // weak copy in SoundSystem.cpp
    virtual ~SystemLevHandle() {} // 800106C4 (weak copy in d_snd_util)

    /* 0x04 vtable */
    /* 0x08 */ u32 _08;
    /* 0x0C */ u32 _0C;
}; // size 0x10

class SoundSystem {
  public:
    enum State_e {
        STATE_WAIT = 0,    // until the title stream (STRM_TITLE) plays
        STATE_LOADING = 1, // until the title stream stops
        STATE_READY = 2,   // sounds may start
        STATE_WAIT_TITLE_END = 3,
    };

    // Track arrangements for hourly/city BGM and transitions between them.
    enum Weather_e {
        WEATHER_NONE,
        WEATHER_FAIR,
        WEATHER_RAIN,
        WEATHER_SNOW,
        WEATHER_FAIR_TO_RAIN,
        WEATHER_RAIN_TO_FAIR,
        WEATHER_FAIR_TO_SNOW,
        WEATHER_SNOW_TO_FAIR,
        WEATHER_MODE_END,
    };

    // The same track selector also handles museum rooms. INIT modes apply
    // immediately.
    enum MuseumTrack_e {
        TRACK_MUSEUM_ENTRANCE = 9,
        TRACK_MUSEUM_FISH,
        TRACK_MUSEUM_INSECTS,
        TRACK_MUSEUM_FOSSILS,
        TRACK_MUSEUM_ART,
        TRACK_MUSEUM_OBSERVATORY,
        TRACK_MUSEUM_MENU,
        TRACK_INIT_MUSEUM_ENTRANCE,
        TRACK_INIT_MUSEUM_FISH,
        TRACK_INIT_MUSEUM_INSECTS,
        TRACK_INIT_MUSEUM_FOSSILS,
        TRACK_INIT_MUSEUM_ART,
        TRACK_INIT_MUSEUM_OBSERVATORY,
        TRACK_INIT_MUSEUM_MENU,
    };

    enum SceneFade_e {
        SCENE_FADE_OUT = 1,
        SCENE_FADE_IN = 2,
    };

    SoundSystem(); // 801CA0F4
    // declared inline (800105AC, first copy in d_snd_util): an implicit dtor
    // would join the open weak section (snd_obj.hpp's, after ~SoundManager) and
    // ~SystemLevHandle would then follow ~SoundHandle; the target tail has
    // ~SoundSystem, ~SystemLevHandle, ~SoundHandle.
    ~SoundSystem() {
    }

    static SoundSystem *getInstance() {
        static SoundSystem instance;
        return &instance;
    }

    void init();                  // 801CA264: after SoundManager::init
    void calc();                  // 801CA2DC
    int getRandom(int max);       // 801CAE00
    void playSe(int se);          // 801CAE08
    void playSe(int se, f32 pan); // 801CAE10
    void holdSe(int se);          // 801CB134
    void holdSe(int se, f32 pan); // 801CB13C
    void startMusic(u32 id, int tickOffset);   // 801CB31C
    void stopMusic(int fade);                  // 801CBC90
    void pauseMusic(bool pause);               // 801CBDD0
    BOOL isMusicPlaying();                     // 801CBDE8
    u32 getMusicId();                          // 801CBDFC: SEQ_NULL when none
    int getMusicTick();                        // 801CBE58
    void setMusicPosition(const mVec3_c *pos); // 801CBEC8
    BOOL isTitleMusic();                       // 801CC124
    void setMusicFilter(int type);             // 801CC160
    u32 getMusicPlayerBgm(int song);           // 801CC1E8
    u32 getKKSongBgm(int song);                // 801CC220
    void setWeather(int weather); // 801CC23C: Weather_e or MuseumTrack_e
    SystemLevHandle *findLevelHandle(int id); // 801CC4C4
    SystemLevHandle *getFreeLevelHandle();    // 801CC584
    void calcTrackFade();                     // 801CC5C8
    const s8 *getKKInfo(); // 801CC874: the conductor's animation state
    BOOL isKKMusic();      // 801CC87C
    void playMelody(const u8 *notes);               // 801CC8D4
    void playJingle(u32 idx);                       // 801CC8E4
    void playTune(u32 id);                          // 801CC8EC
    void playChime();                               // 801CC8F8
    void playMelodyAt(const mVec3_c *pos, int arg); // 801CC900
    void finishTrackFade();                         // 801CC908
    void speakVoice(int mode, int tone, u16 c, u16 next,
                    f32 volume);              // 801CC998
    void setVoiceType(int type);              // 801CCA30
    void appendAmbientObj(SoundObjBase *obj); // 801CCA38
    void removeAmbientObj(SoundObjBase *obj); // 801CCA80
    void calcRiverSound();                    // 801CCAC8
    void setSeaDist(f32 dist);                // 801CD374
    void setMelodyVolume(f32 volume);         // 801CD37C
    void setMelodyFilter(f32 filter);         // 801CD384
    void fadeOut();                           // 801CD38C
    void fadeIn();                            // 801CD4EC
    void enableSceneFade();                   // 801CD64C
    void disableSceneFade();                  // 801CD680
    f32 getSceneVolume();                     // 801CD698
    f32 getScenePan();                        // 801CD6D4
    void setSceneFade(int state);             // 801CD710: SceneFade_e
    void menuOpen();                          // 801CD780
    void menuClose();                         // 801CD8EC
    void talkStart();                         // 801CDA78
    void talkEnd();                           // 801CDBF0
    void setIndoorRain(f32 blend);            // 801CDD5C
    void fadeOutMuseum();                     // 801CE1B4
    void fadeInMuseum();                      // 801CE394

    static u32 getSoundHeapSize(); // 801CE584 (weak)

    // These embedded faders start idle, before the later player members are
    // constructed.
    struct Fade_c : SoundFader {
        Fade_c() {
            mValue = 0.0f;
            mTarget = 0.0f;
            mStep = 0.0f;
            mFrames = -1;
        }
    };

    /* 0x000 */ nw4r::snd::SoundHandle mHandle;
    /* 0x004 */ SystemLevHandle mLevHandles[4];
    /* 0x044 */ nw4r::snd::SoundHandle mMusicHandle; // the current BGM
    /* 0x048 */ u32 _048;
    /* 0x04C */ Fade_c mTrackFader;
    /* 0x05C */ u16 mFadeInTracks;
    /* 0x05E */ u16 mActiveTracks;
    /* 0x060 */ bool mMusicStopped;
    /* 0x061 */ u8
        mWeather; // Weather_e, including a transition until it finishes
    /* 0x062 */ u8 mPendingWeather;
    /* 0x063 */ u8 _063;
    /* 0x064 */ s16
        mTrackFadeDelay; // frames before the track crossfade, -1: idle
    /* 0x066 */ u16 mFadeOutTracks;
    /* 0x068 */ u16 mMuseumTracks;
    /* 0x06C */ Fade_c mMuseumFader;
    /* 0x07C */ u32 mPendingMusicId;
    /* 0x080 */ u32 mPendingGroupId;
    /* 0x084 */ int mQueuedWeather;
    /* 0x088 */ u32 mState; // State_e
    /* 0x08C */ u8 mAllowConductSequence;
    /* 0x08D */ u8 _08D[3];
    /* 0x090 */ int mMusicFilterType;
    /* 0x094 */ f32 mMusicFilter;
    /* 0x098 */ f32 mMusicVolume;
    /* 0x09C */ bool mSceneFadeEnabled;
    /* 0x0A0 */ int mRestartTick;
    /* 0x0A4 */ int mSceneFadeState;
    /* 0x0A8 */ int mRestartWait;
    /* 0x0AC */ Fade_c mSceneFader;
    /* 0x0BC */ bool mMenuOpen;
    /* 0x0BD */ bool mTalkOpen;
    /* 0x0C0 */ SoundMelody mMelody;
    /* 0x134 */ SoundConductorKK mConductorKK;
    /* 0x16C */ SoundConductor mConductor;
    /* 0x188 */ SoundVoice mVoice;
    /* 0x1AC */ SoundAmbient mAmbient;
    /* 0x1C0 */ u32 _1C0;
    /* 0x1C4 */ u32 _1C4;
    /* 0x1C8 */ u32 _1C8;
    /* 0x1CC */ nw4r::ut::List mLists[3];
    /* 0x1F0 */ nw4r::snd::SoundHandle mRiverHandle;
    /* 0x1F4 */ nw4r::snd::SoundHandle mIndoorRainHandle;
    /* 0x1F8 */ nw4r::snd::SoundHandle mIndoorHeavyRainHandle;
}; // size 0x1FC
