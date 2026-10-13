#pragma once
#include <types.h>
#include <game/snd/snd_obj.hpp>

// The TV programme sounds (DOL TU dol/sound/SoundTv.cpp, 801D0550..801D42AC). Class names from the RTTI:
// SoundTv_base (a SoundObj<2>) and one subclass per channel/programme. d_audio_obj's factory builds them
// with inline ctors; mGroupId is the sound archive group holding the programme's sounds. The programme
// scripts (execute) play voice lines and jingles at fixed frames of the show. Member and method names are
// inferred from what the code does.

// MWCC places a derived class's first members in the base's tail padding, so every programme's own
// members start at 0x3A.
class SoundTv_base : public SoundObj<2> {
public:
    SoundTv_base() : mLoading(0) {}
    // A pure virtual dtor prevents MWCC from emitting a SoundTv_base vtable in d_audio_obj; the target
    // has only its RTTI. The body is still needed by derived destructors.
    virtual ~SoundTv_base() = 0;

    virtual void reset();                                          // 801D0550: restart the programme, muted
    virtual void stop();                                           // 801D05AC
    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D05C0: follows pos (NULL: mute)
    virtual void setOn(u8 on);                                     // 801D0638
    virtual void mute();                                           // 801D0650
    virtual bool isReady();                                        // 801D0664: loads the group if needed
    virtual void startTvSound(nw4r::snd::SoundHandle *handle, u32 id); // 801D0954
    virtual void startProgram() = 0;                               // the programme's opening sounds

    /* 0x34 */ u32 mGroupId;
    /* 0x38 */ u8 mLoading; // the group load was requested
    /* 0x39 */ u8 mOn;
}; // size 0x3C (0x3A used)

inline SoundTv_base::~SoundTv_base() {}

class SoundTv_00_storm : public SoundTv_base {
public:
    SoundTv_00_storm() { mGroupId = 0xA2; }

    virtual void startProgram(); // 801D0BC0
}; // size 0x3C

class SoundTv_01_color : public SoundTv_base {
public:
    SoundTv_01_color() { mGroupId = 0xA3; }

    virtual void startProgram(); // 801D0BD8
}; // size 0x3C

class SoundTv_02_news : public SoundTv_base {
public:
    SoundTv_02_news() { mGroupId = 0xA4; }

    virtual void reset();                                          // 801D0BF0
    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D0C3C
    virtual void startProgram();                                   // 801D0C24

    /* 0x3A */ s8 mTopic; // 0..2: the opening of this broadcast (rotates every loop)
}; // size 0x3C

class SoundTv_03_drama : public SoundTv_base {
public:
    SoundTv_03_drama() { mGroupId = 0xA5; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D1068
    virtual void startProgram();                                   // 801D1020

    /* 0x3A */ s8 mScene; // which of the two scene sets plays (toggles every loop)
}; // size 0x3C

class SoundTv_04_song : public SoundTv_base {
public:
    SoundTv_04_song() { mGroupId = 0xA6; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D1238
    virtual void startProgram();                                   // 801D11E8

    int readVariable(); // 801D133C: the song sequence's variable 0 when it changed, else -1

    /* 0x3A */ s16 mVariable;
    /* 0x3C */ int mBank; // 0/1: which set of the four verses plays (alternates)
}; // size 0x40

class SoundTv_05_quiz : public SoundTv_base {
public:
    SoundTv_05_quiz() { mGroupId = 0xA7; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D13B8
    virtual void startProgram();                                   // 801D13AC

    /* 0x3A */ s8 mRound; // which of the two quiz rounds plays (alternates)
}; // size 0x3C

class SoundTv_06_travel : public SoundTv_base {
public:
    SoundTv_06_travel() { mGroupId = 0xA8; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D18F4
    virtual void startProgram();                                   // 801D1E24

    /* 0x3A */ bool mNarrating; // the narration lines play this loop (alternates every loop)
}; // size 0x3C

class SoundTv_07_hero : public SoundTv_base {
public:
    SoundTv_07_hero() { mGroupId = 0xA9; }

    virtual void reset();                                          // 801D23E8
    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D1E88
    virtual void startProgram();                                   // 801D2420

    /* 0x3A */ u16 mPhase;       // odd: the hero tracks (0x3F) are muted
    /* 0x3C */ u16 mCount;       // episodes since mPhase last flipped
    /* 0x3E */ u16 mLastEpisode;
    /* 0x40 */ u16 mEpisode;     // 0..5: which BGM/effect pair plays
}; // size 0x44

class SoundTv_08_variety : public SoundTv_base {
public:
    SoundTv_08_variety() { mGroupId = 0xAA; }

    virtual void reset();                                          // 801D2AE0
    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D246C
    virtual void startProgram();                                   // 801D2B14

    /* 0x3A */ u16 mReaction; // 0..4: the audience reaction of this loop
    /* 0x3C */ u16 mTimer;    // frames since the BGM cycle started (BGM1 at 309, BGM2 at 2100, BGM0 at 2700)
}; // size 0x40

class SoundTv_09_movie : public SoundTv_base {
public:
    SoundTv_09_movie() { mGroupId = 0xAB; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D2BB8
    virtual void startProgram();                                   // 801D2B5C
}; // size 0x3C

class SoundTv_10_soccer : public SoundTv_base {
public:
    SoundTv_10_soccer() { mGroupId = 0xAC; }

    virtual void reset();                                          // 801D36CC
    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D2E2C
    virtual void startProgram();                                   // 801D3708

    /* 0x3A */ u8 mChange;      // a new play is due this frame
    /* 0x3C */ u16 mState;      // zone * 5 + play (0..14), 15 goal, 16 miss
    /* 0x3E */ u16 mPrevState;
    /* 0x40 */ u16 mTimer;      // frames into the current play
}; // size 0x44

class SoundTv_255_weather : public SoundTv_base {
public:
    SoundTv_255_weather() { mGroupId = 0xAD; }

    virtual void execute(int frame, const nw4r::math::VEC3 *pos); // 801D38A8
    virtual void startProgram();                                   // 801D3754

    void startNextSong(); // 801D3800: a random KK song other than the last one

    /* 0x3A */ u8 mLoops;    // loops of the programme with this song (a new song after 7)
    /* 0x3B */ u8 mSong;     // 0..2: TV_SEQ_KK_SWING + mSong
    /* 0x3C */ u8 mForecast; // 0..2 (2: no forecast lines)
}; // size 0x40
