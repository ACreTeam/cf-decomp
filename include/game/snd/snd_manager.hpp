#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>
#include <lib/nw4r/snd/snd_FxReverbHiDpl2.h>
#include <lib/nw4r/snd/snd_SoundThread.h>

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

// RTTI "SoundEffectReverb" (dol/sound/SoundEffectReverb.cpp), vtable 80500678.
class SoundEffectReverb : public nw4r::snd::FxReverbHiDpl2 {
public:
    SoundEffectReverb() {}
    virtual ~SoundEffectReverb() {} // 800103F8 (weak copy in d_snd_util)
    virtual bool StartUp();         // 801D00E8
    virtual void Shutdown();        // 801D013C

    /* 0x308 */ bool mStarted;
}; // size 0x30C

// RTTI "SoundAudioFrameCallback" (vtable 80503C90; methods in dol/sound/SoundObj.cpp): calls a
// function at the start of each sound frame while registered with the sound thread.
class SoundAudioFrameCallback : public nw4r::snd::detail::SoundThread::SoundFrameCallback {
public:
    typedef void (*Callback)();

    SoundAudioFrameCallback() : mCallback(NULL) {}
    virtual ~SoundAudioFrameCallback() {} // 8001047C (weak copy in d_snd_util)
    virtual void OnBeginSoundFrame();     // 801E1BD4

    void set(Callback callback); // 801E1BEC: registers with the sound thread
    void clear();                // 801E1C30

    /* 0x0C */ Callback mCallback;
}; // size 0x10

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

    // The function the sound thread calls each frame (one at a time).
    void setFrameCallback(SoundAudioFrameCallback::Callback callback); // 801CFA68
    void clearFrameCallback();                                         // 801CFA70

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
