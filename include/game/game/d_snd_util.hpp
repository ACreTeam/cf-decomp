#pragma once

// Sound helpers (src/dol/game/d_snd_util.cpp, .text 8000DE5C..800107AC; see notes/d_snd_util.txt): the sound
// init thread, scene hooks and one-line wrappers around SoundManager / SoundSystem (snd/snd_manager.hpp,
// snd/snd_system.hpp). Functions whose meaning is unknown keep their target names (extern "C").

#include <types.h>

class mVec3_c;

void *sndInitThread(void *arg);                    // 8000DE5C: the sound init thread: gives the whole sound heap to the sound manager.
void startSoundInit();                              // 8000E070: starts the sound init thread (once).
BOOL isSoundInitDone();                             // 8000E0EC: whether the sound init thread has finished.
void executeSound();                                // 8000E130: per frame (d_sys).
void sndSceneStart();                               // 8000E408: scene start.
void sndSceneEnd();                                 // 8000E584: scene end.
void sndSceneChange(int fade);                      // 8000E590: scene change.
void sndResetStart(int frames);                     // 8000E6F8: (d_reset)
void getSoundPos(const mVec3_c *src, mVec3_c *dst); // 8000EDD0: field is curved, else into view space.
void sndMenuOpen();                                 // 8000EF30: a menu opened (dBgm VolStateMenu_c)
void sndMenuClose();                                // 8000EF98: the menu closed (dBgm VolStateMenu_c)
void sndTalkStart();                                // 8000F000: a conversation started (dBgm VolStateTalk_c)
void sndTalkEnd();                                  // 8000F068: the conversation ended (dBgm VolStateTalk_c)
void sndStartBgm(u32 id, int fadeIn);                  // 8000F0D0: start the BGM.
void sndStopBgm(int fade);                             // 8000F158: stop the BGM.
void sndSetWeather(int weather);                    // 8000F1D0: the weather for the ambient sounds (dBgm Player_c)
u32 getMusicPlayerBgm(int song);                    // 8000F318: a music player song's BGM id.
BOOL sndIsKKPlaying();                                 // 8000F390: K.K. Slider is playing (dBgm StgCafe_c)
u32 getKKSongBgm(int song);                         // 8000F3F8: a K.K. song's BGM id.
const s8 *sndGetKKInfo();                              // 8000F470: the current K.K. song info (dBgm StgCafe_c; NULL when none)
void setSoundVolume(f32 volume, int frames);        // 8000F540: the sound manager volume (d_bgm)
void setAmbientPos(const mVec3_c *pos);             // 8000F6C0: updates the positional ambient sound (d_bgm)
void playSe(int se);                                // 8000F828: plays a sound effect
void playSePan(int se, f32 pan);                    // 8000F8A0: plays a sound effect with a pan
const u8 *getDefaultTownTune();                     // 8000F9A0: the default town tune
void syncTownTune();                                // 8000FA0C: hands the saved town tune to the sound system
void playMelody(const u8 *notes);                   // 8000FA98: plays a melody with the given notes
int getTownTuneState();                             // 8000FB88: the town tune player state (< 0: idle)
BOOL isTownTunePlaying();                           // 8000FBF4: whether the town tune player is busy
void playTune(u32 id);                              // 8000FC1C: plays a tune (with the saved town tune)
void playVoiceMelody(u16 soundId, u8 *notes);       // 8000FC98: plays a melody (the town tune) with a voice
void playHourlyChime();                             // 8000FD24: the hourly chime
void playSeAt(const mVec3_c *pos, int arg);         // 8000FD90: plays a sound at a world position
void sndStopReverb();                               // 8000FF44: stops the reverb
void sndStartReverb();                              // 8001009C: starts the reverb
bool isSummerMorningStart();                        // 8001026C: no callers in the DOL or the RELs
void checkSummerMorningStart();                     // 80010274: at game start (d_sv_proc): 16 Mar..15 Sep, 6:00..9:59
void clearSummerMorningStart();                     // 800102F8: also cleared by sndInitThread and the first sndSceneStart after a reset

extern "C" {
void fn_8000E06C();                                          // 8000E06C: empty (d_sys)
void fn_8000E2A0(int arg);                                   // 8000E2A0: no callers
void fn_8000E868();                                          // 8000E868: (d_reset)
void fn_8000E9C0();                                          // 8000E9C0: (d_wifi_err)
void fn_8000EB20();                                          // 8000EB20: (d_caution)
void fn_8000EC78();                                          // 8000EC78: (d_caution)
void fn_8000EE60();                                          // 8000EE60: no callers
void fn_8000EEC8();                                          // 8000EEC8: no callers
void fn_8000F248();                                          // 8000F248: museum room change keeping the music (dBgm StgMuseum_c)
void fn_8000F2B0();                                          // 8000F2B0: museum room change done (dBgm StgMuseum_c)
int fn_8000F4D8();                                           // 8000F4D8: K.K. playback value (dBgm StgCafe_c)
void fn_8000F738(int arg);                                   // 8000F738: (dBgm StgRoom_c)
void fn_8000F7B0(int arg);                                   // 8000F7B0: (dBgm StgBus_c)
void fn_8000F928(int se);                                    // 8000F928: a sound effect (d_money, d_msg)
void fn_8000FB10(u32 arg);                                   // 8000FB10: no callers
void fn_8000FE18(int arg);                                   // 8000FE18: (d_msg)
void fn_8000FE90(int a, int b, wchar_t c, wchar_t d, f32 e); // 8000FE90: (d_msg) passes the sort keys of two characters
void fn_800101F4(f32 arg);                                   // 800101F4: no callers
void fn_80010304(f32 arg);                                   // 80010304: no callers
}
