#ifndef GAME_D_MENU_STAR_HPP
#define GAME_D_MENU_STAR_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_star_draw.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

// The star chart of the constellation menus (REL d_menu_starNP; used by
// d_menu_star00 and d_menu_star01): the window (Layout/star.arc) with four
// buttons, a cursor layout, and a dStarDraw_c sky drawn between the 2D layouts.
// RTTI shows the nested class name; everything else is inferred.
class dMenuStar_c {
  public:
    // The window's buttons (hitCheck, setSelect, push, move).
    enum BTN_e {
        BTN_NONE = -1,
        BTN_ZOOM_IN,
        BTN_ZOOM_OUT,
        BTN_TURN_LEFT,  // the sky turns by +TURN_STEP
        BTN_TURN_RIGHT, // the sky turns by -TURN_STEP
        BTN_NUM,
    };

    // Button animations (mBtnAnm).
    enum BTN_ANM_e {
        BTN_ANM_IN,
        BTN_ANM_OUT,
        BTN_ANM_PUSH,
        BTN_ANM_CAN_PUSH,
        BTN_ANM_NO_PUSH,
        BTN_ANM_NUM,
    };

    // Window animations (startWinAnm).
    enum WIN_ANM_e {
        WIN_ANM_IN,
        WIN_ANM_OUT,
        WIN_ANM_NUM,
    };

    // Cursor animations (mCursorAnm).
    enum CURSOR_ANM_e {
        CURSOR_ANM_FINISH,
        CURSOR_ANM_START,
        CURSOR_ANM_NUM,
    };

    enum FLAG_e {
        FLAG_CALC = 0x1,
        FLAG_WIN_ANM = 0x2,
        FLAG_DRAW_CURSOR = 0x4, // for one frame, after setCursorPos
        FLAG_TURN = 0x8,        // turning to mTargetAngle
    };

    enum {
        TURN_STEP = 0x50,
        LYT_PRIORITY = 0x87, // both layouts (m2d::Base_c::mPriority)
    };

    // Draws the sky (mpStar) in the 2D callback list, between the window's z
    // clear and its layout.
    class starOrder_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0xB24

        BOOL create(dStarDraw_c *star); // .text:0x9F0
        bool unload();                  // .text:0xA50
        void init();                    // .text:0xA58
        void calc();                    // .text:0xAAC

        mAng getAngle() const {
            return mAngle;
        }
        void setAngle(mAng angle) {
            mAngle = angle;
        }

        /* 0x10 */ f32 mScale;
        /* 0x14 */ mAng mAngle;
        /* 0x18 */ dStarDraw_c *mpStar;
    }; // size 0x1C

    BOOL load();                          // .text:0x5C
    void create();                        // .text:0xC4
    void draw();                          // .text:0x190
    BOOL remove();                        // .text:0x2A8
    void execute();                       // .text:0x310
    void startWinAnm(int anm);            // .text:0x354
    BOOL isWinAnmEnd();                   // .text:0x3A8
    bool setCursorAnm(int anm);           // .text:0x3BC
    BOOL isCursorAnmEnd();                // .text:0x3C4
    void initAnms();                      // .text:0x3CC
    void calcAnms();                      // .text:0x488
    int hitCheck(const mVec2_c *pos);     // .text:0x544
    void setSelect(int btn, BOOL select); // .text:0x5E8
    BOOL isBtnAnmEnd(int btn);            // .text:0x600
    void push(int btn);                   // .text:0x610
    void move(int btn);                   // .text:0x624: zooms or turns the sky
    bool canZoomIn();                     // .text:0x818
    bool canZoomOut();                    // .text:0x838
    void calcScale();                     // .text:0x858
    void
    setAngle(const dStarSign_c *sign); // .text:0x8B8: shows the sign at once
    void startTurn(const dStarSign_c *sign); // .text:0x8F4: turns to the sign
    bool isTurning();                        // .text:0x938
    void calcTurn();                         // .text:0x950
    void setCursorPos(const mVec2_c *pos);   // .text:0x9C0

    /* 0x000 */ m2d::ResAccLoader_c mRes;
    /* 0x0D4 */ m2d::SimpleEx_c mWinLyt;
    /* 0x16C */ m2d::SimpleEx_c mCursorLyt;
    /* 0x204 */ m2d::Anm_c mWinAnm;
    /* 0x228 */ m2d::AnmEx_c mCursorAnm;
    /* 0x264 */ m2d::AnmEx2_c mBtnAnm[BTN_NUM];
    /* 0x374 */ m2d::zBuffClear_c
        mZClear; // the whole screen, behind everything
    /* 0x39C */ m2d::zBuffClear_c
        mWinZClear; // the window's inside, before the sky
    /* 0x3C4 */ starOrder_c mOrder;
    /* 0x3E0 */ u16 mFlags;
    /* 0x3E4 */ dStarDraw_c mStar;
    /* 0x528 */ f32 mTargetScale;
    /* 0x52C */ mAng mTargetAngle;
}; // size 0x530

#endif
