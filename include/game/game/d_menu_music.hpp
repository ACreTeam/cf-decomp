#ifndef GAME_D_MENU_MUSIC_HPP
#define GAME_D_MENU_MUSIC_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_private_data.hpp>

// The music list (REL d_menu_musicNP, profile MENU_MUSIC): the K.K. songs the
// player owns (Layout/music.arc, music_catalog_win_00), seven lines of which
// the four in the middle can be picked (the outer ones fade out), scrolled by
// two buttons or a slider. Under the list: add music (a song from the pockets),
// eject (a song back into a pocket) and the play / stop buttons for the music
// player of the room (dFTR::playMusic). A dialog box (dMenuSlctB_c) asks for
// the ejecting and shows the "can't" messages. RTTI confirms the class name
// dMenuMusic_c; everything else is inferred.
class dMenuMusic_c : public dMenuBase_c {
  public:
    // What the cursor (mCursor) points at.
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_BTN = 0, // 0..2: dMenuBtn_c buttons (BTN_e)
        CURSOR_BTN_LAST = CURSOR_BTN + dMenuBtn_c::BTN_NUM - 1,
        CURSOR_ROW = 3, // 3..9: the lines (only 4..7 can be picked)
        CURSOR_ROW_LAST = CURSOR_ROW + 6,
        CURSOR_SCRL = 10, // 10, 11: scroll up, scroll down
        CURSOR_SCRL_UP = CURSOR_SCRL,
        CURSOR_SCRL_DOWN = CURSOR_SCRL + 1,
        CURSOR_ADD = 12,   // the square buttons: add music
        CURSOR_EJECT = 13, // ... eject the picked song
        CURSOR_SQUARE = CURSOR_ADD,
        CURSOR_SQUARE_LAST = CURSOR_EJECT,
        CURSOR_SLIDER = 14, // the slider
        CURSOR_PLAY = 15,   // the circle buttons: play the picked song
        CURSOR_STOP = 16,   // ... stop the music
        CURSOR_CIRCLE = CURSOR_PLAY,
        CURSOR_CIRCLE_LAST = CURSOR_STOP,
    };

    // The dialog box's buttons (dMenuSlctB_c).
    enum DIALOG_BTN_e {
        DIALOG_BTN_NO,
        DIALOG_BTN_YES,
        DIALOG_BTN_OK, // the messages have only this one
    };

    // The dMenuBtn_c buttons (the first two are hidden).
    enum BTN_e {
        BTN_UNUSED_0,
        BTN_UNUSED_1,
        BTN_CLOSE,
    };

    enum {
        ROW_NUM = 7,
        SCRL_NUM = 2,
        SQUARE_NUM = 2,
        CIRCLE_NUM = 2,
        SONG_NUM = 78, // the K.K. songs, item indexes ITEM_IDX_KK_CHORALE..
        ROW_PICK_FIRST = 1, // the lines 1..4 can be picked
        ROW_PICK_END = 5,
        POCKET_MASK_ALL =
            (1 << PLAYER_POCKETS_COUNT) - 1, // every pocket masked out
        SCROLL_MIN = -1,
        SCROLL_PAGE = 4,   // the lines a left / right press scrolls
        SCROLL_ROWS = 5,   // mScroll + SCROLL_ROWS < mCount: more to scroll
        SLIDER_OFFSET = 4, // the slider spans mCount - SLIDER_OFFSET steps
    };

    // Line animations (mRowAnm).
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_BEFORE,
        ROW_ANM_GRAY, // the fading outer lines
    };

    // Scroll button animations (mScrlBtnAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_IN,
        SCRL_ANM_OUT,
        SCRL_ANM_PUSH,
        SCRL_ANM_CAN_PUSH,
        SCRL_ANM_NO_PUSH,
    };

    // Circle button animations (mCircleAnm).
    enum CIRCLE_ANM_e {
        CIRCLE_ANM_IN,
        CIRCLE_ANM_OUT,
        CIRCLE_ANM_YELLOW, // can be pushed
        CIRCLE_ANM_GRAY,
    };

    // Square button animations (mSquareAnm).
    enum SQUARE_ANM_e {
        SQUARE_ANM_YELLOW, // can be pushed
        SQUARE_ANM_GRAY,
    };

    // The list scroll animations (mScrlAnm).
    enum SCRL_LIST_ANM_e {
        SCRL_LIST_ANM_DOWN,
        SCRL_LIST_ANM_UP,
    };

    // The window frame animations (mFrmAnm).
    enum FRM_ANM_e {
        FRM_ANM_OFF,
        FRM_ANM_ON, // a song is playing
    };

    // The slider animations (mSliderAnm).
    enum SLIDER_ANM_e {
        SLIDER_ANM_IN,
        SLIDER_ANM_OUT,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC, // the window layout needs a calc
        FLAG_UNUSED_2,
        FLAG_DIALOG,      // draw the dialog box
        FLAG_UPDATE_BTN,  // update the buttons' states
        FLAG_UPDATE_LIST, // update the lines
        FLAG_ADD,         // after the close animation: open the pocket picker
    };

    // Sub states (setSubState).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,         // waits for the pushed button to finish
        SUB_SCROLL,       // keeps scrolling while the button is held
        SUB_DIALOG,       // waits for the dialog's answer
        SUB_DIALOG_PUSH,  // waits for the dialog's button to finish
        SUB_DIALOG_CLOSE, // the dialog closes
        SUB_SLIDER,       // the slider is dragged
        SUB_NUM,
    };

    // dMenuMng_c::setResult values.
    enum {
        RESULT_CANCEL = 0,
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_POCKETS_FULL =
            0xAC, // "You can't take music out if your pockets are full."
        MSG_NO_MUSIC = 0xAD, // "There is no music in your list of possessions!"
        MSG_CLOSE = 0xEB,
        MSG_OK = 0xEC,
        MSG_EJECT = 0x126, // "Do you want to eject <song>?"
        MSG_PLAYING_ELSEWHERE =
            0x144, // "You can't take out music that's playing in another room!"
    };

    // The button tutorial balloons (SYS_2D_UIballoon.bmg): add, eject, play,
    // stop.
    enum {
        BALLOON_MSG_ADD = 0x2A,
    };

    static bool isBtn(int cursor) {
        return cursor >= CURSOR_BTN && cursor <= CURSOR_BTN_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrl(int cursor) {
        return cursor >= CURSOR_SCRL && cursor <= CURSOR_SCRL_DOWN;
    }
    static bool isSquare(int cursor) {
        return cursor >= CURSOR_SQUARE && cursor <= CURSOR_SQUARE_LAST;
    }
    static bool isCircle(int cursor) {
        return cursor >= CURSOR_CIRCLE && cursor <= CURSOR_CIRCLE_LAST;
    }

    virtual int create();                                   // .text:0x188
    virtual int doDelete();                                 // .text:0x2CC
    virtual int stateMain();                                // .text:0x338
    virtual int stateAnm();                                 // .text:0x3C0
    virtual int stateClose();                               // .text:0x410
    virtual int execute();                                  // .text:0x46C
    virtual int draw();                                     // .text:0x518
    void subStateMain();                                    // .text:0x580
    void subStatePush();                                    // .text:0x658
    void subStateScroll();                                  // .text:0x694
    void subStateDialog();                                  // .text:0x7B0
    void subStateDialogPush();                              // .text:0x810
    void subStateDialogClose();                             // .text:0x874
    void subStateSlider();                                  // .text:0x8E8
    void executeSubState();                                 // .text:0xA58
    void setSelect(int cursor, BOOL select);                // .text:0xA90
    int hitCheck();                                         // .text:0xC94
    void checkPadMove();                                    // .text:0xDB4
    void pushCursor();                                      // .text:0xF2C
    BOOL isPushEnd();                                       // .text:0x11C0
    void finishPush();                                      // .text:0x12E0
    void startCloseAnm();                                   // .text:0x14BC
    void startAddAnm();                                     // .text:0x14FC
    void initList();                                        // .text:0x154C
    void startScroll(int cursor);                           // .text:0x15B4
    void setScrlBtnAnms();                                  // .text:0x166C
    bool canUse(int cursor);                                // .text:0x16EC
    bool isEnabled(int cursor);                             // .text:0x17B0
    bool canScrollUp();                                     // .text:0x18D8
    bool canScrollDown();                                   // .text:0x18F8
    void selectRow(int cursor);                             // .text:0x191C
    void initSlider();                                      // .text:0x19D0
    void calcSliderAnm();                                   // .text:0x1A64
    void updateSlider();                                    // .text:0x1AA8
    void setRowAnm(int cursor, BOOL highlight, int offset); // .text:0x1B58
    void resetRowAnms(int offset);                          // .text:0x1BF0
    void refreshList();                                     // .text:0x1C50
    void initButtons();                                     // .text:0x1C90
    void initAnms();                                        // .text:0x1CEC
    void calcAnms();                                        // .text:0x1EDC
    void setFrmAnm(bool on);                                // .text:0x200C
    void initRows();                                        // .text:0x2014
    void updateSelMarks();                                  // .text:0x209C
    void updateRowWords();                                  // .text:0x212C
    wchar_t *getSongName(int idx);                          // .text:0x2200
    void updatePlayingName();                               // .text:0x22C4
    dPrivateSongs_c *getSongs();                            // .text:0x2348
    void loadSongs();                                       // .text:0x2370
    void openMessage(u16 msg);                              // .text:0x24E4
    void openEjectDialog();                                 // .text:0x259C
    void stopMusic();                                       // .text:0x2684
    void playMusic();                                       // .text:0x26F4
    bool isPlaying();                                       // .text:0x2780
    void pushEject();                                       // .text:0x27A4
    void ejectSong();                                       // .text:0x2824
    void pushAdd();                                         // .text:0x2950
    void updateButtons();                                   // .text:0x29AC
    void stubUpdate();             // .text:0x2ABC (empty)
    void showTutorial(int cursor); // .text:0x2AC0
    void playSeAdd();              // .text:0x2BBC
    void playSeEject();            // .text:0x2BC4
    void playSeStop();             // .text:0x2BCC
    void playSePlay();             // .text:0x2BD4

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x3A4
    virtual int state3() {
        return SUCCEEDED;
    }
    virtual void postDraw(MAIN_STATE_e) {
    }
    virtual void deleteReady() {
    }
    virtual bool createHeap() {
        return true;
    }

    // Implicit destructor: .text:0x2BDC.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0x41C */ m2d::stringWord_c mRowWord[ROW_NUM]; // the song names
    /* 0xAC8 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM];
    /* 0xB40 */ m2d::AnmEx2_c mCircleAnm[CIRCLE_NUM];
    /* 0xBC8 */ m2d::AnmEx2_c mSquareAnm[SQUARE_NUM];
    /* 0xC50 */ m2d::Anm_c mSquarePushAnm;
    /* 0xC74 */ m2d::Anm_c mCirclePushAnm;
    /* 0xC98 */ m2d::Anm_c mScrlIconAnm; // the note on the slider
    /* 0xCBC */ m2d::Anm_c mPlayingAnm;  // while a song is playing
    /* 0xCE0 */ m2d::AnmEx_c mFrmAnm;
    /* 0xD1C */ m2d::AnmEx_c mScrlAnm;
    /* 0xD58 */ dMenuSlctB_c mSlct;
    /* 0x1448 */ dMenuBtn_c mBtn;
    /* 0x198C */ int mCursor;
    /* 0x1990 */ int mScroll; // list index of the second line
    /* 0x1994 */ int mSel;    // the picked song (list index), -1 none
    /* 0x1998 */ int _1998;
    /* 0x199C */ int mHover; // the square button with the frame, -1 none
    /* 0x19A0 */ int mCount; // songs in the list
    /* 0x19A4 */ dItem::Item mSongs[SONG_NUM];
    /* 0x1A40 */ dItem::Item mPlaying; // the song playing in the room
    /* 0x1A44 */ m2d::AnmEx_c mSliderAnm;
    /* 0x1A80 */ f32 mSliderTop;
    /* 0x1A84 */ f32 mSliderBottom;
    /* 0x1A88 */ nw4r::lyt::Pane *mpSlider;
    /* 0x1A8C */ f32 mDragOffset;
}; // size 0x1A90

#endif
