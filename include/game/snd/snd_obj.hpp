#pragma once
#include <types.h>
#include <lib/nw4r/math/math_types.h>
#include <lib/nw4r/snd/snd_SeqSoundHandle.h>
#include <lib/nw4r/snd/snd_SoundHandle.h>
#include <lib/nw4r/snd/snd_SoundStartable.h>
#include <lib/nw4r/snd/snd_SoundThread.h>
#include <lib/nw4r/ut/ut_list.h>

// The sound library's positional sound objects (DOL TU dol/sound/SoundObj.cpp, 801DC544..801E1BD4).
// Class names are from the RTTI:
//   SoundObjBase
//     SoundObj<N>        N sound handles at 0x2C
//       SoundObjSimple, SoundObjInsect, SoundHaniwa, SoundObjPinwheel, SoundObjFall, SoundObjPond,
//       SoundObjFurniture (SoundObj<1>), SoundObjNPC (SoundObj<2>), SoundObjPlayer (SoundObj<4>)
//     SoundObjRiver
// Every object is linked into one list (SoundObjBase::sFirst) while it exists. Member and method names
// are inferred from what the code does.
//
// The inline virtual bodies are included selectively through snd_obj_inline.hpp. SoundObj.cpp and
// d_audio_obj.cpp hold the weak copies; d_menu_number needs the pending-parse file-selection effect.
// Defining SoundObj<N>'s inline virtuals here would make every includer (all of d_a_npc.hpp's) instantiate
// its getters in MWCC's end-of-file pending loop. That leaves the compiler's current-function file info
// pointing into this header, so with -sym on and -ipa file the TU's first generated function that creates
// a string literal is filed into this header's .text section (d_npc_talk_quest_delivery's msgDelivery).

class SoundConductor;

// Base of the positional sound objects (vtable 80503C60). The position is relative to the listener;
// calcParam turns it into a volume, pan and surround pan.
class SoundObjBase {
public:
    // mKind: set by the subclass ctors (the others keep 0).
    enum Kind_e {
        KIND_PLAYER = 0,
        KIND_NPC = 1,
        KIND_INSECT = 2, // also gets the foot step fx send (sFxSend)
        KIND_SIMPLE = 3,
        KIND_RIVER = 4,  // KIND_RIVER..KIND_FALL: in SoundSystem's ambient lists
        KIND_POND = 5,
        KIND_FALL = 6,
    };

    // calcParam's volume curve.
    enum ParamMode_e {
        PARAM_MODE_NORMAL = 0,   // silent from 650 units
        PARAM_MODE_INSECT = 1,   // shorter range (fly, mosquito, bee sounds)
        PARAM_MODE_FOUNTAIN = 2, // the town fountain (SE_LEV_funsui_machi): close range, flattened
    };

    SoundObjBase();          // 801DC544
    virtual ~SoundObjBase(); // 801DC5B4

    virtual void setPosition(const nw4r::math::VEC3 *pos); // 801DC614: sets mPos, then calcParam(mode 0)
    virtual void startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo) = 0;
    virtual void holdSound(u32 id) = 0;
    virtual void update(const nw4r::math::VEC3 *pos) = 0; // sets the position and applies it to the sounds
    virtual void setVolume(f32 volume);                     // 801DC650: clamped to [0, 1]
    virtual f32 getVolume();                                // 80012080 (weak copy in d_audio_obj)
    virtual void stopAll(int fadeFrames) = 0;
    virtual void calcParam(const nw4r::math::VEC3 *pos, int mode); // 801DC67C

    void appendList(); // 801DCB60
    void removeList(); // 801DCB98

    static void stopAllObj(int fadeFrames); // 801DCC04: stopAll on every object, clears sIsPlayable
    static void setFxSend(f32 send);        // 801DCC68 (SoundManager, from the reverb settings)
    static f32 getFxSend();                 // 801DCC70
    static void setMasterVolume(f32 volume); // 801DCC78 (SoundManager, from its fader)
    static f32 getMasterVolume();            // 801DCC80

    static SoundObjBase *sFirst;  // 8074EC60: head of the object list
    static f32 sFxSend;           // 8074EC64: effect send of the foot sounds
    static f32 sMasterVolume;     // 8074C6B0
    static bool sIsPlayable;      // 8074C6B4: set by SoundManager; sounds only start while it is set

    /* 0x00 vtable */
    /* 0x04 */ SoundObjBase *mPrev;
    /* 0x08 */ SoundObjBase *mNext;
    /* 0x0C */ f32 mPosVolume;   // from the distance (calcParam)
    /* 0x10 */ f32 mPan;         // from x (calcParam), -1..1
    /* 0x14 */ f32 mSurroundPan; // from z (calcParam), 0..2
    /* 0x18 */ int mKind; // Kind_e
    /* 0x1C */ f32 mVolume;
    /* 0x20 */ nw4r::math::VEC3 mPos;
}; // size 0x2C

// A sound object with N sound handles (RTTI "SoundObj<1>", "SoundObj<2>", "SoundObj<4>"). The inline
// ctor/dtor and getters are weak: the SoundObj<1>/<2> copies are kept in d_audio_obj, the SoundObj<4>
// ones here.
template <int N>
class SoundObj : public SoundObjBase {
public:
    SoundObj() {}
    virtual ~SoundObj() { // SoundObj<1>: 80011FF4, SoundObj<2>: 80011F44, SoundObj<4>: 801E1AFC
        for (int i = 0; i < N; i++) {
            mHandles[i].DetachSound();
        }
    }

    virtual void startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo); // <1> 801DE294
    virtual void holdSound(u32 id);                    // <1> 801DE838
    virtual void update(const nw4r::math::VEC3 *pos); // <1> 801DF140
    virtual f32 getPosVolume();                        // <1> 80012088, <2> 80012098, <4> 801E1BC4
    virtual f32 getPan();                              // <1> 80012090, <2> 800120A0, <4> 801E1BCC
    virtual void stopSound(u32 id, int fadeFrames);    // <1> 801DF424, <2> 801E06AC, <4> 801E1A08
    virtual void stopAll(int fadeFrames);              // <1> 801DF45C, <2> 801E073C, <4> 801E1A98

    /* 0x2C */ nw4r::snd::SoundHandle mHandles[N];
}; // size 0x2C + N * 4

// The foot step sounds of SoundObjNPC and SoundObjPlayer (no RTTI, no vtable; class name inferred).
class SoundFoot {
public:
    enum {
        FLAG_FOOT = 1 << 0, // set: foot 0 (id + 0, alternative id + 2), clear: foot 1 (id + 1 / id + 3)
        FLAG_ALT0 = 1 << 1,
        FLAG_ALT1 = 1 << 2,
    };

    SoundFoot() : mOwner(NULL), mVolume(1.0f), mFlags(0), mMix(0) {}
    ~SoundFoot(); // 801DD374

    void start(u32 id, u8 mix, f32 speed); // 801DD3EC
    void update();                          // 801DDADC

    /* 0x00 */ nw4r::snd::SoundHandle mHandle;
    /* 0x04 */ nw4r::snd::SoundHandle mHandle2;
    /* 0x08 */ SoundObjBase *mOwner;
    /* 0x0C */ f32 mVolume;
    /* 0x10 */ u8 mFlags; // FLAG_FOOT toggles every step; FLAG_ALT0/1: the last step of that foot used
                          // the alternative sound
    /* 0x11 */ u8 mMix;          // share of the second sound (0..254), 0xFF: none
}; // size 0x14

// RTTI "SoundObjPlayer" (vtable 80503BB0).
class SoundObjPlayer : public SoundObj<4> {
public:
    SoundObjPlayer();          // 801DCC88
    virtual ~SoundObjPlayer(); // 801DCD20

    virtual void update(const nw4r::math::VEC3 *pos); // 801DCDE4
    virtual f32 getDist();                             // 801E1BBC

    /* 0x3C */ SoundFoot mFoot;
    /* 0x50 */ f32 mDist; // length of the last update position
}; // size 0x54

// RTTI "SoundObjNPC" (vtable 80503B58).
class SoundObjNPC : public SoundObj<2> {
public:
    SoundObjNPC();          // 801DCE30
    virtual ~SoundObjNPC(); // 801DCECC

    virtual void update(const nw4r::math::VEC3 *pos); // 801DCF90

    /* 0x34 */ SoundFoot mFoot;
}; // size 0x48

class SoundObjInsect : public SoundObj<1> {
public:
    SoundObjInsect();          // 801DCFC4
    virtual ~SoundObjInsect(); // 801DD034

    virtual void update(const nw4r::math::VEC3 *pos); // 801DD0C4

    bool isSounding(); // 801DD1F8: the sequence's first track still has channels
}; // size 0x30

class SoundObjSimple : public SoundObj<1> {
public:
    SoundObjSimple();          // 801DD270
    virtual ~SoundObjSimple(); // 801DD2E0

    virtual void update(const nw4r::math::VEC3 *pos); // 801DD370: only calls SoundObj<1>::update
}; // size 0x30

// RTTI "SoundObjRiver" (vtable 80503A4C): no sound handles, in SoundSystem's ambient list 0 (mKind 4).
class SoundObjRiver : public SoundObjBase {
public:
    SoundObjRiver();          // 801DDC0C
    virtual ~SoundObjRiver(); // 801DDCA0

    virtual void startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo); // 801E1BB8
    virtual void holdSound(u32 id);                       // 801E1BB4
    virtual void update(const nw4r::math::VEC3 *pos);    // 801DDD58: only calcParam, then mPos
    virtual void stopAll(int fadeFrames);                 // 801E1BB0

    /* 0x2C */ nw4r::ut::Link mLink; // SoundSystem ambient list 0 (link offset 0x2C)
}; // size 0x34

// RTTI "SoundObjPond" (vtable 805039F0): in SoundSystem's ambient list 1 (mKind 5).
class SoundObjPond : public SoundObj<1> {
public:
    SoundObjPond();          // 801DDDB8
    virtual ~SoundObjPond(); // 801DDE78

    /* 0x30 */ nw4r::ut::Link mLink; // SoundSystem ambient list 1 (link offset 0x30)
}; // size 0x38

// RTTI "SoundObjFall" (vtable 80503990): in SoundSystem's ambient list 2 (mKind 6).
class SoundObjFall : public SoundObj<1> {
public:
    SoundObjFall();          // 801DDF64
    virtual ~SoundObjFall(); // 801DE024

    /* 0x30 */ nw4r::ut::Link mLink; // SoundSystem ambient list 2 (link offset 0x30)
}; // size 0x38

// RTTI "SoundObjPinwheel" (vtable 80503930): volume and pitch follow the spin speed.
class SoundObjPinwheel : public SoundObj<1> {
public:
    SoundObjPinwheel();                   // 801DE110
    virtual ~SoundObjPinwheel();          // 80011DAC (weak copy in d_audio_obj)
    virtual void update(const nw4r::math::VEC3 *pos);   // 801E1BAC

    void update(const nw4r::math::VEC3 *pos, f32 speed); // 801DE180: speed 0..1

    /* 0x30 */ f32 _30;
}; // size 0x34

// A furniture item's sounds (dol/sound/SoundObjFurniture.cpp, vtable 80503CF8): switching it on and off,
// a loop while on, and sounds at frames of its animation (FrameSound). Some items play labels "se0".."se9"
// of their own sequence data.
class SoundObjFurniture : public SoundObj<1> {
public:
    // mFlags
    enum {
        FLAG_LOOP_ONCE = 0x0001,  // plays its loop sound only while on
        FLAG_RESTART = 0x0002,    // restarts a sound that is already playing
        FLAG_OFF_AFTER = 0x0004,  // switches off after a frame sound
        FLAG_LOOP_ALWAYS = 0x0008,
        FLAG_TYPE = 0x00F0,       // getType
        FLAG_FRAMES = 0x7F00,     // the row of sFrameSounds (0: none)
    };

    // A sound at an animation frame. mFrame: 0x1FFF the frame; 0x8000 not when playing forward,
    // 0x4000 not backward, 0x2000 not while a sound plays. mSound: 0x8000 a label of the sequence data.
    struct FrameSound {
        u16 mFrame;
        u16 mSound;
    };

    struct Data {
        u16 mOnSound;
        u16 mOffSound;
        u16 mFlags;
        u16 mLoopSound;
    };

    // A furniture's own sequence (setupSeq).
    struct SeqData {
        u16 _00;
        Data mData;
        u16 mSeqId;
        FrameSound mFrames[1]; // up to 10
    };

    // mDir
    enum Dir_e {
        DIR_NONE,
        DIR_FORWARD,
        DIR_WRAP_FORWARD,
        DIR_BACKWARD,
        DIR_WRAP_BACKWARD,
        DIR_JUMP,
    };

    SoundObjFurniture();          // 801E1C78
    virtual ~SoundObjFurniture(); // 801E1D30
    virtual void startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo); // 801E1DC4: not in the house rooms
    virtual void holdSound(u32 id); // 801E1F48: not in the house rooms

    void setup(u8 item, bool on);                              // 801E20D4
    void setupSeq(const SeqData *data, const void *seq, bool on); // 801E2148
    void update(const nw4r::math::VEC3 *pos, f32 frame);       // 801E21AC
    void turnOn();                                             // 801E2744
    void turnOff();                                            // 801E29E8
    int getType();                                             // 801E2AA8
    int calcDir(f32 prevFrame, f32 frame);                     // 801E2AC4
    void playLabel(int label);                                 // 801E2B60

    /* 0x30 */ bool mOn;
    /* 0x31 */ bool mLoopOnce;
    /* 0x34 */ int mCount;
    /* 0x38 */ int mCycles;
    /* 0x3C */ int mDir; // Dir_e
    /* 0x40 */ f32 mFrame;
    /* 0x44 */ u16 mSeqId;
    /* 0x46 */ u16 mLabel;
    /* 0x48 */ const Data *mData;
    /* 0x4C */ const FrameSound *mFrames;
    /* 0x50 */ const void *mSeq;
}; // size 0x54

// An instrument that plays the town tune (dol/sound/SoundBgm.cpp, vtable 80501090): the notes of
// SoundMelody one per step, in time with SoundSystem::mConductor.
// The base is private (the RTTI base list flags it).
class SoundObjInst : private SoundObj<1> {
public:
    SoundObjInst();          // 801D7D40
    virtual ~SoundObjInst(); // 801D7DA8

    static void incPlaying(); // 801D7CB4
    static void decPlaying(); // 801D7CC4: the melody is free again after the last one

    void setup(int inst);                 // 801D7E38: inst 0..19
    void stop();                          // 801D8044
    void play();                          // 801D80A4
    void calc(const nw4r::math::VEC3 *pos); // 801D81A0
    void readParams();                    // 801D8390
    void playNote();                      // 801D841C
    BOOL checkEnd();                      // 801D854C

    static const u8 *sNotes;  // 8074EC28
    static int sPlayingNum;   // 8074EC2C

    /* 0x30 */ u32 mSoundId;
    /* 0x34 */ s8 mNote;
    /* 0x35 */ u8 mTick;
    /* 0x36 */ s8 mTempo;     // variable 4: 100 = one note per beat
    /* 0x37 */ s8 mHold;      // variable 3
    /* 0x38 */ bool mPlaying;
    /* 0x39 */ u8 mNoteTicks;
}; // size 0x3C

// A gyroid (dol/sound/SoundHaniwa.cpp): plays its sequence (SEQ_HNW_*) in time with the music player's song,
// starting at a bar of SoundSystem::mConductor. The sequence's variables describe its loop: 0 the kind,
// 1..4 the bars it plays (mBarMask), 5 the mode, 6, 7 the start bar, 8 and 12 the current part.
class SoundHaniwa : public SoundObj<1> {
public:
    SoundHaniwa();          // 801D5548
    virtual ~SoundHaniwa(); // 801D5648

    static s16 getStartBar(); // 801D5390: the next bar, or the one after if the next starts too soon
    static s16 getNextBar();  // 801D54C4

    void setup(u32 kind, s8 startBar); // 801D56F4: kind 0..0x7E (SEQ_HNW_BCN_DKA + kind)
    void restart();                    // 801D5940: stops; starts again with the song
    void reset();                      // 801D598C
    void play();                       // 801D59F4: from the next start bar
    void stop();                       // 801D5A64
    int getWaitFrames();               // 801D5A90: frames until the sound starts (>= 1)
    f32 calc(const nw4r::math::VEC3 *pos); // 801D5B84: per frame; returns the loop progress 0..1 (-1: off)
    void calcPart();                   // 801D5CDC
    void readVariables();              // 801D5E34
    void readLoop();                   // 801D5EE0
    void updateTempo();                // 801D6020
    f32 calcProgress();                // 801D6148
    static s16 getGlobalBar();         // 801D646C: global sequence variable 2
    static s16 getGlobalBeat();        // 801D64A0: global sequence variable 1

    /* 0x30 */ SoundConductor *mConductor;
    /* 0x34 */ u32 mSoundId;
    /* 0x38 */ int mInHouse; // > 0: the sound is placed at a fixed point in the house
    /* 0x3C */ u32 _3C;
    /* 0x40 */ s8 mStartBar; // -1: not playing
    /* 0x41 */ bool mWaiting; // waiting for the start bar
    /* 0x42 */ u16 mBarMask;
    /* 0x44 */ s8 mLoopKind;  // variable 0 (-1: not read yet)
    /* 0x45 */ s8 mLoopMode;  // variable 5
    /* 0x46 */ s8 mPart;      // variable 8
    /* 0x47 */ s8 mNextPart;  // variable 12 (-1: none)
    /* 0x48 */ s8 mLoopLen;   // variable 6
    /* 0x49 */ bool mLoopDone;
    /* 0x4C */ SoundHaniwa *mNext; // SoundConductor::mHaniwaList
    /* 0x50 */ SoundHaniwa *mPrev;
}; // size 0x54

// RTTI "SoundAudioFrameCallback" (vtable 80503C90; DOL TU dol/sound/SoundAudioFrameCallback.cpp,
// 801E1BD4..801E1C78, file name inferred): calls a function at the start of each sound frame while
// registered with the sound thread.
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
