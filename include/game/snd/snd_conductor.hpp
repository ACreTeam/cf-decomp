#pragma once

// dol/sound/SoundConductor.cpp (801D42AC..801D5390): follows the beat of the music player's song (the BGM
// sequence's tick and the global sequence variables 1 = beat, 2 = bar) so that the gyroids (SoundHaniwa)
// and K.K. Slider can play along. Class names from the RTTI ("SoundConductor", vtable 80500FFC;
// "SoundConductorKK", vtable 80500FD0). Both live in SoundSystem (mConductor, mConductorKK).

#include <types.h>

class SoundHaniwa;

class SoundConductor {
public:
    SoundConductor();  // 801D42AC
    ~SoundConductor(); // 801D4344 (not virtual: not in the vtable)

    u32 calcBeat();                  // 801D4384: (bar << 16) | the previous beat
    void start(u32 musicId);         // 801D44D0: a song starts
    void resetHaniwa();              // 801D4608
    virtual void calc();             // 801D4648: per frame
    void updateTempo();              // 801D4720
    void appendHaniwa(SoundHaniwa *haniwa); // 801D4838
    void removeHaniwa(SoundHaniwa *haniwa); // 801D4870

    int getBeatTicks() const { return mBeatTicks; }

    /* 0x04 */ u32 mMusicId;
    /* 0x08 */ int mBeatTicks;   // ticks per beat (24; 48 for style 4, 16 for some songs)
    /* 0x0C */ f32 mBeatTime;    // 1800 / tempo
    /* 0x10 */ bool mBarChanged;
    /* 0x12 */ s16 mTempo;
    /* 0x14 */ s8 mBar;          // 0..15, -1: none
    /* 0x15 */ u8 mBeat;
    /* 0x16 */ s16 mStyle;       // 0..4: the gyroid dance style of the song
    /* 0x18 */ SoundHaniwa *mHaniwaList;
}; // size 0x1C

// K.K. Slider's performance.
class SoundConductorKK : public SoundConductor {
public:
    // What K.K.'s animation reads.
    struct Anim {
        Anim();       // 801D48DC
        void reset(); // 801D490C

        /* 0x00 */ s8 mPhase;  // the song's variable 4 (4 for BGM_KK76_UTATANENOYUME), -1: not started
        /* 0x01 */ s8 mPart;   // the playing instrument
        /* 0x02 */ s8 mVar0;   // the song's variables 0, 2, 3 and 1
        /* 0x03 */ s8 mVar2;
        /* 0x04 */ s8 mVar3;
        /* 0x05 */ s8 mVar1;
        /* 0x08 */ f32 mBeatPos;
        /* 0x0C */ f32 mFrame60;
        /* 0x10 */ f32 mFrame96;
    }; // size 0x14

    SoundConductorKK();  // 801D4938
    ~SoundConductorKK(); // 801D499C

    void reset();         // 801D4A0C
    virtual void calc();  // 801D4A48 (the override)
    void calcPart();      // 801D4CD4: the first instrument track with channels
    void calcBeatPos();   // 801D4E2C
    void calcAnim();      // 801D5068

    /* 0x1C */ Anim mAnim;
    /* 0x30 */ u8 mSongType;  // 1 or 2 for some songs
    /* 0x31 */ u8 mTempoType; // 1..3
    /* 0x32 */ s16 mBeatPos;
    /* 0x34 */ bool mBeatFlip;
}; // size 0x38
