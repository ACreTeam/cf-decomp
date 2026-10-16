#pragma once

// The title screen (REL d_title_demoNP, profile TITLE_DEMO): fades in, plays
// the logo and Nintendo layouts, waits for A with the title BGM, then checks
// the NAND and the Mii library before changing to the player select scene. Can
// also delete the save (message 2). RTTI confirms the global class dTitleDemo_c
// and its nested steps stepCheck_c / stepDelete_c; method and member names are
// inferred from the original state-table labels.

#include <game/framework/f_base.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_state.hpp>
#include <game/game/d_sv_step.hpp>
#include <game/game/d_m2d.hpp>

class dTitleDemo_c : public fBase_c,
                     public dMsg::Rcpt_c,
                     public dState::mode_c<dTitleDemo_c> {
  public:
    enum FLAG_e {
        FLAG_LOGO = 1,       // the title logo layout is shown
        FLAG_NINTENDO = 2,   // the Nintendo layout is shown
        FLAG_PRESS_A = 4,    // the "press A" layout is shown
        FLAG_SMALL_LOGO = 8, // the logo moved to its small position (after the
                             // title BGM ended)
    };

    // The logo animations (l_logoAnmNames).
    enum LogoAnm_e {
        LOGO_ANM_IN,
        LOGO_ANM_WAIT,
        LOGO_ANM_OUT,
        LOGO_ANM_S_IN,
        LOGO_ANM_S_WAIT,
        LOGO_ANM_S_OUT,
    };

    // Unused: constructed but never initialized.
    class stepCheck_c : public dState::step_c<dTitleDemo_c, stepCheck_c> {
      public:
        stepCheck_c() {
        }
        virtual ~stepCheck_c() {
        }
    };

    // Deletes the DL box and the save file.
    class stepDelete_c : public dSvStep_c<dTitleDemo_c, stepDelete_c> {
      public:
        stepDelete_c() {
        }
        virtual ~stepDelete_c() {
        }
        void initDelete(); // 0x1088
        void finish();     // 0x113C
    };

    dTitleDemo_c();          // 0x8C
    virtual ~dTitleDemo_c(); // 0x178

    virtual int create();   // 0x230
    virtual int doDelete(); // 0x320
    virtual int execute();  // 0x3A4
    virtual int draw();     // 0x400

    virtual void onMessageStart(int arg); // 0x4B4
    virtual void onMessageEnd(int kind);  // 0x50C
    virtual void onAnswer(int answer);    // 0x510

    void requestMessage(u16 code); // 0x424

    void initFadeIn();                       // 0x7BC
    void executeFadeIn();                    // 0x7D4
    void initLogoIn();                       // 0x958
    void executeLogoIn();                    // 0x970
    void initNintendo();                     // 0xAF0
    void executeNintendo();                  // 0xB38
    void initWait();                         // 0xCA4
    void executeWait();                      // 0xD0C
    void initFileDelete();                   // 0xED4
    void executeFileDelete();                // 0x1040
    void initToOpening();                    // 0x12B8
    void executeToOpening();                 // 0x12BC
    void initMiiErrorCheck();                // 0x145C
    void executeMiiErrorCheck();             // 0x1460
    void initMiiErrorMessage();              // 0x16FC
    void executeMiiErrorMessage();           // 0x1700
    void initSceneChangeToPlayerSelect();    // 0x18A0
    void executeSceneChangeToPlayerSelect(); // 0x18F0
    void initNone();                         // 0x18F4
    void executeNone();                      // 0x18F8
    void initMessage();                      // 0x18FC
    void executeMessage();                   // 0x1900
    void initSceneChange();                  // 0x1AA0
    void executeSceneChange();               // 0x1AB0
    void initNandInfo();                     // 0x1AB4
    void executeNandInfo();                  // 0x1AB8
    void initNandCheck();                    // 0x1E9C
    void executeNandCheck();                 // 0x1EA0

    bool isSkipTriggered(); // 0x2668
    BOOL checkStart();      // 0x275C
    void createLayout();    // 0x2A34
    void calcLayout();      // 0x2C1C
    void drawLayout();      // 0x2D08
    void resetPressA();     // 0x2D6C
    void setBackScale();    // 0x2DE8

    void lockMessage() {
        dMsg::Comp_c *controller = mpController;
        controller->mLock = 1;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 1;
    }
    void startLogoAnm(int idx) {
        if (idx != mLogoAnm.mCurIdx) {
            mLogoAnm.start(idx);
        }
    }

    /* 0x0EC */ stepCheck_c mStepCheck;
    /* 0x11C */ stepDelete_c mStepDelete;
    /* 0x14C */ s16 mTimer;
    /* 0x14E */ u16 mFlags;
    /* 0x150 */ m2d::ResAccLoader_c mRes;
    /* 0x224 */ m2d::SimpleEx_c mLogoLyt;
    /* 0x2BC */ m2d::AnmEx_c mLogoAnm;
    /* 0x2F8 */ m2d::SimpleEx_c mNintendoLyt;
    /* 0x390 */ m2d::SimpleEx_c mPressLyt;
    /* 0x428 */ m2d::Anm_c mPressAnm;
    /* 0x44C */ f32 mPressWidthL; // width of "T_pressA_00" (large logo)
    /* 0x450 */ f32 mPressWidthS; // width of "T_pressA_01" (small logo)
}; // size 0x454
