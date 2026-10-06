#pragma once

// Fireworks show manager (RTTI dFireWorksMgr_c): during the August fireworks event and the New
// Year countdown it launches shells above the town at random intervals, with the "af_fld_firework"
// effect, sounds and a flash of the sky light. Source: src/dol/game/d_fireworks.cpp
// (.text 8008FE50..80090964). dFireWorksMgr_c is from the RTTI; dFireWork_c and the function names
// are inferred. See notes/d_fireworks.txt.

#include <types.h>
#include <nw4r/math.h>

#define FIREWORKS_SHELL_NUM 8

// One firework shell.
class dFireWork_c {
public:
    enum State_e {
        STATE_IDLE,
        STATE_WAIT,   // just launched, before it rises
        STATE_RISE,   // rising: the flash builds up, the burst sound plays
        STATE_BURST,  // burst: the sky light takes the shell's color
        STATE_FADE,
    };

    dFireWork_c();      // 8009042C
    ~dFireWork_c() {}   // 80090924

    void reset();                    // 8008FE50
    void launch(BOOL withEffect);    // 8008FE94
    f32 getTimerF();                 // stripped
    void execute(BOOL withEffect);   // 8009004C
    void draw(BOOL withEffect);      // 80090370

    /* 0x00 */ nw4r::math::VEC3 mPos; // above the town center
    /* 0x0C */ int mId;        // -1
    /* 0x10 */ int mColor;     // 0..2
    /* 0x14 */ int mState;     // State_e
    /* 0x18 */ int mTimer;
    /* 0x1C */ f32 mGlow;      // glow size
    /* 0x20 */ f32 mBrightness;
    /* 0x24 */ f32 mScale;     // 0.7..1.3
}; // size 0x28

class dFireWorksMgr_c {
public:
    dFireWorksMgr_c();                    // 800903D0
    virtual ~dFireWorksMgr_c();           // 80090438

    void init(BOOL withEffect);           // 800904A8
    void execute();                       // 800905A8
    void draw();                          // 800906AC
    BOOL isShowTime();                    // 80090708: fireworks event, or the countdown before 2:00
    BOOL launch();                        // 800907C8
    dFireWork_c *getFreeShell();          // 80090810

    static dFireWorksMgr_c *sInstance;    // 8074E330

    /* 0x004 */ dFireWork_c mShells[FIREWORKS_SHELL_NUM];
    /* 0x144 */ long mBurstLeft;  // shells left in this burst (sLib::chase(long *))
    /* 0x148 */ long mWait;       // frames to the next launch / burst
    /* 0x14C */ u8 mWithEffect;
    /* 0x14D */ u8 mAllowed;      // the current scene allows fireworks
    /* 0x14E */ u8 mWasShowTime;
}; // size 0x150

