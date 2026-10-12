#pragma once

// dol/sound/SoundFader.cpp (801D64F4..801D6618). No RTTI; the class names are inferred (the TU name comes
// from the split).

#include <types.h>
#include <lib/nw4r/snd.h>

// A value faded linearly over a number of frames.
struct SoundFader {
    void set(f32 value);                // 801D64F4: at once
    void fadeTo(f32 target, int frames); // 801D6504: linearly over frames (0: at once)
    f32 calc();                          // 801D6560: one frame step; returns the value

    /* 0x00 */ f32 mValue;
    /* 0x04 */ f32 mTarget;
    /* 0x08 */ f32 mStep;
    /* 0x0C */ int mFrames; // frames left, -1: idle
}; // size 0x10

class SoundMelody;

// One part of SoundMelody: plays the 16 notes of a melody (the town tune) as single sounds mSoundId + 1 +
// note, one note per step. The sequence mSoundId gives the timing in its variables (readParams). Its ctor and
// dtor are in SoundFader.cpp, the methods in SoundBgm.cpp.
class SoundMelodyPart {
public:
    SoundMelodyPart();  // 801D65A4
    ~SoundMelodyPart(); // 801D65B4: detaches the sound

    void init(SoundMelody *owner, f32 volume, int filterType); // 801D6618
    void start(u32 soundId, const u8 *notes);                  // 801D6690
    void playJingle(u32 idx);                                  // 801D6834: 0..13
    void stop();                                               // 801D69D0
    void reset();                                              // 801D6A18
    void readParams();                                         // 801D6A48
    u8 getNote(u8 idx);                                        // 801D6BCC: 14 rest, 15 hold, 13 random
    void playNote();                                           // 801D6D18
    BOOL checkEnd();                                           // 801D71F8
    int calc();                                                // 801D7360: the note index, -1 when idle
    void end();                                                // 801D778C

    /* 0x00 */ SoundMelody *mOwner;
    /* 0x04 */ nw4r::snd::SoundHandle mHandle;
    /* 0x08 */ const u8 *mNotes;
    /* 0x0C */ u32 mSoundId;
    /* 0x10 */ f32 mVolume;
    /* 0x14 */ f32 mFilter;
    /* 0x18 */ int mFilterType;
    /* 0x1C */ bool mPlaying;
    /* 0x1D */ s8 mNote;      // getTownTuneState (< 0: idle)
    /* 0x1E */ u8 mTick;
    /* 0x1F */ u8 _1F;
    /* 0x20 */ s8 mStepTicks; // ticks per note
    /* 0x21 */ s8 mEndTicks;
    /* 0x22 */ s8 mType;      // how the notes are spread (getNote)
    /* 0x23 */ s8 mHold;
    /* 0x24 */ s8 mSwing;
    /* 0x25 */ s8 mSwing2;
}; // size 0x28

// The town tune player (SoundSystem::mMelody): the tune's notes, the hourly chime (mPart0) and melodies or
// jingles (mPart1). Methods in SoundBgm.cpp.
class SoundMelody {
public:
    // mState
    enum State_e {
        STATE_IDLE,
        STATE_MELODY,   // play()
        STATE_INST,     // SoundObjInst plays the tune
        STATE_POSITION, // playAt()
    };

    static const u8 *getMonthNotes(int month); // 801D7438: the island tunes by month
    void init();                                // 801D7460
    const u8 *getDefaultNotes();               // 801D74E0
    void setNotes(const u8 *notes);            // 801D74EC
    u8 *getNotes();                            // 801D7570
    void play(u32 soundId, const u8 *notes);   // 801D7578: NULL: the town tune
    void stop();                               // 801D7748
    void playChime();                          // 801D77C0
    void playAt(const nw4r::math::VEC3 *pos, u32 kind); // 801D7960
    void playJingle(u32 idx);                  // 801D7BD0
    f32 calcVolume(const nw4r::math::VEC3 *pos); // 801D7BE8
    f32 calcPan(const nw4r::math::VEC3 *pos);    // 801D7C40
    void calc();                               // 801D7C74

    f32 getVolume() const { return mVolume; }

    // two members, not an array: the implicit dtor (inlined in 800105AC) destroys them one by one
    /* 0x00 */ SoundMelodyPart mPart0;
    /* 0x28 */ SoundMelodyPart mPart1;
    /* 0x50 */ u8 mNotes[16];
    /* 0x60 */ bool mChimeBusy;
    /* 0x64 */ f32 mDist;
    /* 0x68 */ f32 mVolume; // -1: none
    /* 0x6C */ f32 mPan;
    /* 0x70 */ u8 _70;
    /* 0x71 */ u8 mState;   // State_e
}; // size 0x74
