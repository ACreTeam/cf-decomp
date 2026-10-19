// Sound helpers. .text 8000DE5C..800107AC. See include/game/game/d_snd_util.hpp.
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_system.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_thunder.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_perf.hpp>
#include <lib/revolution/OS/OSThread.h>
#include <revolution/MTX/mtxvec.h>
#include <game/game/d_date.hpp>             // [B]
#include <game/game/d_save_data.hpp>        // [B]
#include <game/game/d_save_melody.hpp>      // [B]
#include <game/game/d_script.hpp>           // [B]
#include <game/mLib/m_vec.hpp>              // [B]

// ==== [A] 8000DE5C..8000F540 ====

static OSThread l_sndThread;                     // 80562EB8
static u8 l_sndThreadStack[0x1000] ALIGN(32);    // 805631E0
static bool l_sndReset;                          // 8074E14A: set by sndResetStart, handled at the scene start
static bool l_summerMorningStart;                           // 8074E14B: the play session started on a spring/summer morning (checkSummerMorningStart)
static bool l_sndReady;                          // 8074E14C: the init thread has finished


// 8000DE5C: the sound init thread: gives the whole sound heap to the sound manager.
void *sndInitThread(void *arg) {
    u32 size = dHeap::soundHeap_p->getAllocatableSize(4);
    void *buf = dHeap::soundHeap_p->alloc(size, 4);
    SoundManager::getInstance()->init(4, 3, buf, size);
    SoundSystem::getInstance()->init();
    dBgm::l_mgr.fn_8008001C();
    l_summerMorningStart = false;
    return NULL;
}

// 8000E06C
void fn_8000E06C() {}

// 8000E070: starts the sound init thread (once).
void startSoundInit() {
    if (!l_sndReady) {
        OSCreateThread(&l_sndThread, sndInitThread, NULL, l_sndThreadStack + sizeof(l_sndThreadStack),
                       sizeof(l_sndThreadStack), 29, 1);
        mPerf::setThreadColor(&l_sndThread, dPerf::getThreadColor(13));
        OSResumeThread(&l_sndThread);
    }
}

// 8000E0EC: whether the sound init thread has finished.
BOOL isSoundInitDone() {
    if (!OSIsThreadTerminated(&l_sndThread)) {
        return FALSE;
    }
    l_sndReady = true;
    return TRUE;
}

// 8000E130: per frame (d_sys).
void executeSound() {
    if (l_sndReady) {
        dBgm::l_mgr.execute();
        SoundManager::getInstance()->calc();
    }
}

// 8000E2A0
void fn_8000E2A0(int arg) {
    SoundManager::getInstance()->setupStage(arg);
}

// 8000E408: scene start.
void sndSceneStart() {
    if (l_sndReset) {
        SoundManager::getInstance()->resume();
        l_sndReset = false;
        l_summerMorningStart = false;
    }
    dBgm::l_mgr.sceneStart();
}

// 8000E584: scene end.
void sndSceneEnd() {
    dBgm::l_mgr.sceneEnd();
}

// 8000E590: scene change.
void sndSceneChange(int fade) {
    dBgm::l_mgr.sceneChange(fade);
    SoundManager::getInstance()->beginSceneChange();
}

// 8000E6F8: (d_reset)
void sndResetStart(int frames) {
    SoundManager::getInstance()->stopAll(frames);
    l_sndReset = true;
}

// 8000E868: (d_reset)
void fn_8000E868() {
    SoundManager::getInstance()->shutdown();
}

// 8000E9C0: (d_wifi_err)
void fn_8000E9C0() {
    SoundManager::getInstance()->setMasterVolume(0.0f, 0);
}

// 8000EB20: (d_caution)
void fn_8000EB20() {
    SoundManager::getInstance()->mute();
}

// 8000EC78: (d_caution)
void fn_8000EC78() {
    SoundManager::getInstance()->unmute();
}

// 8000EDD0: a field position to the sound's listener space: relative to the camera eye while the
// field is curved, else into view space.
void getSoundPos(const mVec3_c *src, mVec3_c *dst) {
    if (dWorld::isCurve()) {
        *dst = *src - dCamera_c::sEye;
    } else {
        PSMTXMultVec(dCamera_c::sInstance->mCamera.getViewMatrix(), *src, *dst);
    }
}

// 8000EE60
void sndFadeOut() {
    SoundSystem::getInstance()->fadeOut();
}

// 8000EEC8
void sndFadeIn() {
    SoundSystem::getInstance()->fadeIn();
}

// 8000EF30
void sndMenuOpen() {
    SoundSystem::getInstance()->menuOpen();
}

// 8000EF98
void sndMenuClose() {
    SoundSystem::getInstance()->menuClose();
}

// 8000F000
void sndTalkStart() {
    SoundSystem::getInstance()->talkStart();
}

// 8000F068
void sndTalkEnd() {
    SoundSystem::getInstance()->talkEnd();
}

// 8000F0D0: start the BGM.
void sndStartBgm(u32 id, int fadeIn) {
    SoundSystem::getInstance()->startMusic(id, fadeIn);
}

// 8000F158: stop the BGM.
void sndStopBgm(int fade) {
    SoundSystem::getInstance()->stopMusic(fade);
}

// 8000F1D0
void sndSetWeather(int weather) {
    SoundSystem::getInstance()->setWeather(weather);
}

// 8000F248
void fn_8000F248() {
    SoundSystem::getInstance()->fadeOutMuseum();
}

// 8000F2B0
void fn_8000F2B0() {
    SoundSystem::getInstance()->fadeInMuseum();
}

// 8000F318: a music player song's BGM id.
u32 getMusicPlayerBgm(int song) {
    return SoundSystem::getInstance()->getMusicPlayerBgm(song);
}

// 8000F390
BOOL sndIsKKPlaying() {
    return SoundSystem::getInstance()->isMusicPlaying();
}

// 8000F3F8: a K.K. song's BGM id.
u32 getKKSongBgm(int song) {
    return SoundSystem::getInstance()->getKKSongBgm(song);
}

// 8000F470
const s8 *sndGetKKInfo() {
    return SoundSystem::getInstance()->getKKInfo();
}

// 8000F4D8
int fn_8000F4D8() {
    return SoundSystem::getInstance()->getMusicTick();
}


// ==== [B] 8000F540..800107AC ====

// 8000F540: the sound manager volume (d_bgm)
void setSoundVolume(f32 volume, int frames) {
    SoundManager::getInstance()->setVolume(volume, frames);
}

// 8000F6C0: updates the positional ambient sound (d_bgm)
void setAmbientPos(const mVec3_c *pos) {
    SoundSystem::getInstance()->setMusicPosition(pos);
}

// 8000F738
void fn_8000F738(int arg) {
    SoundSystem::getInstance()->setMusicFilter(arg);
}

// 8000F7B0
void sndSetSceneFade(int arg) {
    SoundSystem::getInstance()->setSceneFade(arg);
}

// 8000F828: plays a sound effect
void playSe(int se) {
    SoundSystem::getInstance()->playSe(se);
}

// 8000F8A0: plays a sound effect with a pan
void playSePan(int se, f32 pan) {
    SoundSystem::getInstance()->playSe(se, pan);
}

// 8000F928
void holdSe(int se) {
    SoundSystem::getInstance()->holdSe(se);
}

// 8000F9A0: the default town tune
const u8 *getDefaultTownTune() {
    return SoundSystem::getInstance()->mMelody.getDefaultNotes();
}

// 8000FA0C: hands the saved town tune to the sound system
void syncTownTune() {
    u8 *notes = dSaveData_c::getRaw()->mVillageMelody.getNotes();
    SoundSystem::getInstance()->mMelody.setNotes(notes);
}

// 8000FA98: plays a melody with the given notes
void playMelody(const u8 *notes) {
    SoundSystem::getInstance()->playMelody(notes);
}

// 8000FB10
void fn_8000FB10(u32 arg) {
    SoundSystem::getInstance()->playJingle(arg);
}

// 8000FB88: the town tune player state (< 0: idle)
int getTownTuneState() {
    return SoundSystem::getInstance()->mMelody.mPart1.mNote;
}

// 8000FBF4: whether the town tune player is busy
BOOL isTownTunePlaying() {
    return getTownTuneState() >= 0;
}

// 8000FC1C: plays a tune (with the saved town tune)
void playTune(u32 id) {
    syncTownTune();
    SoundSystem::getInstance()->playTune(id);
}

// 8000FC98: plays a melody (the town tune) with a voice
void playVoiceMelody(u16 soundId, u8 *notes) {
    SoundSystem::getInstance()->mMelody.play(soundId, notes);
}

// 8000FD24: the hourly chime
void playHourlyChime() {
    syncTownTune();
    SoundSystem::getInstance()->playChime();
}

// 8000FD90: plays a sound at a world position
void playSeAt(const mVec3_c *pos, int arg) {
    mVec3_c out;
    getSoundPos(pos, &out);
    syncTownTune();
    SoundSystem::getInstance()->playMelodyAt(&out, arg);
}

// 8000FE18
void sndSetVoiceType(int type) {
    SoundSystem::getInstance()->setVoiceType(type);
}

// 8000FE90: the voice works on the font codes (sort keys) of the character and the next one
void sndSpeakVoice(int mode, int tone, wchar_t c, wchar_t next, f32 volume) {
    u16 code = dScript::getSortKey(c);
    u16 nextCode = dScript::getSortKey(next);
    SoundSystem::getInstance()->speakVoice(mode, tone, code, nextCode, volume);
}

// 8000FF44: stops the reverb
void sndStopReverb() {
    SoundManager::getInstance()->stopReverb();
}

// 8001009C: starts the reverb
void sndStartReverb() {
    SoundManager::getInstance()->startReverb();
}

// 800101F4
void sndSetSeaDist(f32 dist) {
    SoundSystem::getInstance()->setSeaDist(dist);
}

// 8001026C
bool isSummerMorningStart() {
    return l_summerMorningStart;
}

// 80010274: set from the date: 16 Mar..15 Sep (months are 0-based), 6:00..9:59
void checkSummerMorningStart() {
    dTime_c *now = dTime_c::getCurrent();
    if (((now->month >= 3 && now->month <= 7) || (now->month == 2 && now->mday >= 16) || (now->month == 8 && now->mday < 16)) &&
        now->hour >= 6 && now->hour <= 9) {
        l_summerMorningStart = true;
    } else {
        l_summerMorningStart = false;
    }
}

// 800102F8
void clearSummerMorningStart() {
    l_summerMorningStart = false;
}

// 80010304
void fn_80010304(f32 arg) {
    SoundSystem::getInstance()->setIndoorRain(arg);
}
