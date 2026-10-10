#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>
#include <lib/nw4r/snd/snd_FxReverbHiDpl2.h>
#include <lib/nw4r/snd/snd_SoundThread.h>
#include <game/snd/snd_effect_reverb.hpp>
#include <game/snd/snd_obj.hpp> // SoundAudioFrameCallback

// The game's sound manager: the 0x784-byte object built by the inline SoundManager::getInstance
// (static local 805641E0, guard 8074E148; the first copy is in d_snd_util, the dtor 800104BC is
// the weak copy kept there). The class has no RTTI; its name comes from the split holding its
// methods (dol/sound/SoundManager.cpp, 801CE58C..801D00E8). The member classes with RTTI are
// SoundLoader, SoundEffectReverb and SoundAudioFrameCallback. Only what is used so far; member
// names are inferred, unknown members keep their offsets.

// RTTI "SoundLoader" (dol/sound/SoundLoader.cpp).
class SoundLoader {
public:
    SoundLoader();          // 801E2C3C
    virtual ~SoundLoader(); // 801E2C70

    /* 0x04 */ u8 _04[0x24 - 0x04];
}; // size 0x24

class SoundManager {
public:
    // A position with an id (-1: none).
    struct Pos_c {
        /* 0x00 */ nw4r::math::VEC3 mPos;
        /* 0x0C */ int mId;
    }; // size 0x10

    SoundManager() {
        _69C = 0;
        _69D = 0;
        _6A0 = 0.0f;
        _6A4 = 0;
        for (int i = 0; i < 4; i++) {
            _6A8[i].mPos.x = 0.0f;
            _6A8[i].mPos.y = 0.0f;
            _6A8[i].mPos.z = 0.0f;
            _6A8[i].mId = -1;
        }
        _760 = 0;
        _764.mPos.x = 0.0f;
        _764.mPos.y = 0.0f;
        _764.mPos.z = 0.0f;
        _764.mId = -1;
        _778 = -1;
        _77C = 0;
        _780 = -1;
    }

    static SoundManager *getInstance() {
        static SoundManager instance;
        return &instance;
    }

    // Methods of the unsplit TU keep their fn_ names.
    void fn_801CE58C(int arg1, int arg2, void *heapBuf, u32 heapSize); // 801CE58C: init with the sound heap
    void fn_801CE978();                 // 801CE978: per frame (d_sys)
    void fn_801CEB70(int frames);       // 801CEB70: (d_reset)
    void fn_801CEC90();                 // 801CEC90: (scene start)
    void fn_801CED38();                 // 801CED38: (d_reset)
    void fn_801CED88();                 // 801CED88: (scene change)
    void fn_801CEE24(int arg);          // 801CEE24
    void fn_801CF6A4(f32 arg, int frames); // 801CF6A4: (d_wifi_err)
    void fn_801CFF40();                 // 801CFF40: (d_caution)
    void fn_801CFF4C();                 // 801CFF4C: (d_caution)

    // The function the sound thread calls each frame (one at a time).
    void setFrameCallback(SoundAudioFrameCallback::Callback callback); // 801CFA68
    void clearFrameCallback();                                         // 801CFA70

    void setVolume(f32 volume, int frames); // 801CF714: clamps volume to [0, 1], fades the fader at 0x6B8
    void stopReverb();  // 801CFA78: fades out _764, shuts down the AUX A effect (mReverb)
    void startReverb(); // 801CFAD8: fades in _764, appends mReverb to AUX A

    /* 0x000 */ nw4r::snd::SoundHeap mHeap;
    /* 0x02C */ nw4r::snd::DvdSoundArchive mArchive;
    /* 0x1B8 */ nw4r::snd::SoundArchivePlayer mPlayer;
    /* 0x2A0 */ nw4r::snd::SoundHeap mHeap2A0;
    /* 0x2CC */ nw4r::snd::SoundHeap mHeap2CC;
    /* 0x2F8 */ nw4r::snd::SoundHeap mHeap2F8;
    /* 0x324 */ nw4r::snd::SoundHeap mHeap324;
    /* 0x350 */ u8 _350[0x35C - 0x350];
    /* 0x35C */ SoundLoader mLoader;
    /* 0x380 */ SoundEffectReverb mReverb;
    /* 0x68C */ SoundAudioFrameCallback mFrameCallback;
    /* 0x69C */ u8 _69C;
    /* 0x69D */ u8 _69D;
    /* 0x6A0 */ f32 _6A0;
    /* 0x6A4 */ u8 _6A4;
    /* 0x6A8 */ Pos_c _6A8[4];
    /* 0x6E8 */ u8 _6E8[0x760 - 0x6E8];
    /* 0x760 */ int _760;
    /* 0x764 */ Pos_c _764;
    /* 0x774 */ u8 _774[4];
    /* 0x778 */ int _778;
    /* 0x77C */ int _77C;
    /* 0x780 */ int _780;
}; // size 0x784
