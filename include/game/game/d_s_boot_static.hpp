#pragma once

// The boot scene's static part (DOL TU d_s_boot_static.cpp, .text 80107F00..8010A120). Its one instance shows
// the strap and the ESRB screens behind a color fader while the boot phase loads the resident resources, the
// save and the WC24 state. dScBoot_c (d_s_boot) forwards its create / execute / draw / delete to it. Class names
// are from the RTTI except the screens'; member and function names are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/game/d_state_mode.hpp>
#include <game/game/d_sv_step.hpp>

namespace EGG {
class ColorFader;
}

class dScBootStatic_c {
public:
    // mState
    enum State_e {
        STATE_SHOW,     // the screens are shown
        STATE_END,      // the ESRB screen is done: change to the title scene
        STATE_FADE_OUT, // the strap screen is done: fade out to the ESRB screen
        STATE_FADING,   // fading out
    };

    // mScreen
    enum Screen_e {
        SCREEN_STRAP,
        SCREEN_ESRB,
    };

    // The Wii Strap warning: done after a second, left with a button press (or after 20 seconds).
    class StrapScreen_c {
    public:
        StrapScreen_c();  // 80108AD0
        BOOL create();    // 80108B24
        BOOL remove();    // 80108C94
        void execute();   // 80108CE4
        void draw();      // 80108E08

        /* 0x000 */ m2d::ResAccLoader_c mResAcc;
        /* 0x0D4 */ m2d::Simple_c mLyt;
        /* 0x16C */ m2d::Anm_c mAnm;
        /* 0x190 */ u16 mTimer;
        /* 0x192 */ bool mPressed;
        /* 0x193 */ bool mCreated;
        /* 0x194 */ bool mDone;
    }; // size 0x198

#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    // The ESRB rating notice: done after 1.5 seconds.
    class EsrbScreen_c {
    public:
        EsrbScreen_c();   // 80108E10
        BOOL create();    // 80108E60
        BOOL remove();    // 80108FCC
        void execute();   // 8010901C
        void draw();      // 801090E0

        /* 0x000 */ m2d::ResAccLoader_c mResAcc;
        /* 0x0D4 */ m2d::Simple_c mLyt;
        /* 0x16C */ m2d::Anm_c mAnm;
        /* 0x190 */ u16 mTimer;
        /* 0x192 */ bool mCreated;
        /* 0x193 */ bool mDone;
    }; // size 0x194
#endif

    // Loads the save (the steps of dSvMgr_c::stepLoad_c without its scene changes).
    class LoadSaveState_c : public dState::mode_c<LoadSaveState_c> {
    public:
        class stepLoad_c : public dSvStep_c<LoadSaveState_c, stepLoad_c> {
        public:
            void prepareLoad();       // 801092E0
            void processLoadedSave(); // 801093B8
            void finish();            // 801094EC

            bool isStep(Method step) const { return this->mCurrentMethod == step; }

            stepLoad_c() {}
            inline virtual ~stepLoad_c();
        };

        bool isEnd();       // 801090E8
        void initStep();    // 80109134
        BOOL execute();     // 80109264: TRUE once loaded

        /* 0x24 */ bool mLoadFailed; // no usable save (InitSaveState_c makes a new one)
        /* 0x25 */ bool mStepReady;
        /* 0x28 */ stepLoad_c mStep;
    }; // size 0x58

    // Checks the WC24 state (dWifi::startCheck).
    class Wc24CheckState_c : public dState::mode_c<Wc24CheckState_c> {
    public:
        BOOL execute(); // 801094F0: TRUE once checked

        /* 0x24 */ bool mStarted;
        /* 0x25 */ bool mNetError; // net error 12: the save isn't loaded
    }; // size 0x28

    // Builds the profanity filter (dPrcMng_c::startSetup).
    class PrcInitState_c : public dState::mode_c<PrcInitState_c> {
    public:
        BOOL execute(); // 801095A4: TRUE once built

        /* 0x24 */ bool mStarted;
    }; // size 0x28

    // Prepares the save data (dSvProc_c).
    class InitSaveState_c : public dState::mode_c<InitSaveState_c> {
    public:
        BOOL execute(); // 80109610: TRUE once done

        /* 0x24 */ bool mStarted;
    }; // size 0x28

    dScBootStatic_c(); // 801086D4

    static dScBootStatic_c *getInstance(); // 80108698
    static BOOL isBooting();               // 801086A4: TRUE until both screens are done

    BOOL create();     // 8010875C: FALSE until both screens are created
    BOOL doDelete();   // 80108840: FALSE until the boot phase is done
    void preExecute(); // 80108934: runs the boot phase while the scene waits
    void execute();    // 80108940
    void draw();       // 80108A74

#if VERSION == VERSION_RUUE01_00 || VERSION == VERSION_RUUE01_01
    bool isEnd() const { return mEsrb.mDone && mStrap.mDone; }

    /* 0x000 */ int mScreen; // Screen_e
    /* 0x004 */ int mState;  // State_e
    /* 0x008 */ StrapScreen_c mStrap;
    /* 0x1A0 */ EsrbScreen_c mEsrb;
    /* 0x334 */ EGG::ColorFader *mpFader;
    /* 0x338 */ LoadSaveState_c mLoadSave;
    /* 0x390 */ Wc24CheckState_c mWc24Check;
    /* 0x3B8 */ PrcInitState_c mPrcInit;
    /* 0x3E0 */ InitSaveState_c mInitSave;
}; // size 0x408
#else
    bool isEnd() const { return mStrap.mDone; }

    /* 0x000 */ int mState; // State_e
    /* 0x004 */ StrapScreen_c mStrap;
    /* 0x19C */ LoadSaveState_c mLoadSave;
    /* 0x1F4 */ Wc24CheckState_c mWc24Check;
    /* 0x21C */ PrcInitState_c mPrcInit;
    /* 0x244 */ InitSaveState_c mInitSave;
}; // size 0x26C
#endif

inline dScBootStatic_c::LoadSaveState_c::stepLoad_c::~stepLoad_c() {}
