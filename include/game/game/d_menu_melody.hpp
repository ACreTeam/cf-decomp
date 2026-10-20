#ifndef GAME_D_MENU_MELODY_HPP
#define GAME_D_MENU_MELODY_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_save_melody.hpp>

namespace dMenuMelody {

// One note of the town tune editor (Layout/melody.arc muramero_note_00): the
// row of the note picks the picture (rest, slur, a note of the scale or the
// random note) and the note name.
// Not derived from m2d::SimpleEx_c: the layout is the first member (a derived
// class would get a vtable of its own, the target stores the SimpleEx_c
// vtable).
class Note_c {
  public:
    // The rows of the tune editor, the order of the notes' animation frames.
    enum ROW_e {
        ROW_REST,
        ROW_SLUR,
        ROW_NOTE_FIRST, // 2..14: the notes of the scale
        ROW_NOTE_LAST = 14,
        ROW_RANDOM, // the "?" note
        ROW_NUM,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_RANDOM = 0xDD, // "?"
        MSG_SLUR = 0xE5,   // the long mark
    };

    // The note shows a note name (a row of the scale).
    bool isNormal() const {
        return mRow != ROW_RANDOM && mRow != ROW_SLUR;
    }

    void create(m2d::ResAccIf_c *res);                    // .text:0x1138
    void draw();                                          // .text:0x1264
    void requestCalc();                                   // .text:0x12E0
    void setPos(m2d::SimpleEx_c *base, const char *name); // .text:0x12F0
    void setRow(int row);                                 // .text:0x1308
    void setSelected(BOOL select);                        // .text:0x1414
    bool isPushEnd();                                     // .text:0x14E8
    void setIdle(bool idle);                              // .text:0x14F0

    /* 0x000 */ m2d::SimpleEx_c mLyt;
    /* 0x098 */ m2d::Anm_c mColorAnm; // the frame is the row
    /* 0x0BC */ m2d::Anm_c mPushAnm;
    /* 0x0E0 */ m2d::Anm_c mPlayAnm; // frame 0 / 1: idle / singing
    /* 0x104 */ int mRow;
    /* 0x108 */ u16 mFlags; // bit 0: needs a calc
}; // size 0x10C

} // namespace dMenuMelody

// The town tune editor (REL d_menu_melodyNP, profile MENU_MELODY): sixteen
// notes to move up and down, the play leaf button and the button bar. Pressing
// the confirm button saves the tune as the town tune and broadcasts it. RTTI
// confirms the global class name; everything else is inferred.
class dMenuMelody_c : public dMenuBase_c {
  public:
    typedef dMenuMelody::Note_c Note_c;

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_CALC,       // the window layout needs a calc
        FLAG_NOTES_CALC, // the notes' layouts need a calc
        FLAG_NOTES_POS,  // the notes must be moved onto their panes
    };

    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,
        SUB_PLAY, // the tune is playing
        SUB_GRAB, // a note is held
    };

    // The cursor (mCursor): 0..2 the button bar, 3..18 the notes, 19 the play
    // leaf button.
    enum {
        CURSOR_NONE = -1,
        CURSOR_BTN_FIRST = 0,
        CURSOR_BTN_LAST = dMenuBtn_c::BTN_NUM - 1,
        CURSOR_NOTE_FIRST = dMenuBtn_c::BTN_NUM,
        CURSOR_NOTE_LAST = CURSOR_NOTE_FIRST + SAVE_MELODY_NOTE_NUM - 1,
        CURSOR_PLAY,
    };

    // The button bar's buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_OK,
    };

    // The animations of the play leaf button (mLeafAnm).
    enum LEAF_ANM_e {
        LEAF_ANM_IN,
        LEAF_ANM_OUT,
        LEAF_ANM_PUSH,
        LEAF_ANM_NUM,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index) and of
    // SYS_2D_UIballoon.bmg.
    enum {
        MSG_QUIT = 0x1F,
        MSG_CONFIRM = 0x22,
    };

    enum {
        NOTE_NUM = SAVE_MELODY_NOTE_NUM,
        NOTE_HEIGHT = 12, // pixels the cursor moves for one row
    };

    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN_FIRST && slot <= CURSOR_BTN_LAST;
    }
    static bool isNote(int slot) {
        return slot >= CURSOR_NOTE_FIRST && slot <= CURSOR_NOTE_LAST;
    }

    virtual int create();     // .text:0x1FC
    virtual int doDelete();   // .text:0x310
    virtual int stateMain();  // .text:0x364
    virtual int stateAnm();   // .text:0x388
    virtual int stateClose(); // .text:0x3E4
    virtual int execute();    // .text:0x42C
    virtual int draw();       // .text:0x49C

    void subStateMain();                   // .text:0x574
    void subStatePush();                   // .text:0x64C
    void subStatePlay();                   // .text:0x688
    void subStateGrab();                   // .text:0x724
    void executeSubState();                // .text:0x83C
    void setSelect(int slot, BOOL select); // .text:0x874
    int hitCheck();                        // .text:0x938
    void push();                           // .text:0x9D8
    BOOL isPushEnd();                      // .text:0xAAC
    void finishPush();                     // .text:0xB50
    void startGrab();                      // .text:0xC70
    void startCloseAnm();                  // .text:0xCDC
    void initButtons();                    // .text:0xD1C
    bool createNextNote();                 // .text:0xD7C
    void drawNotes();                      // .text:0xDE8
    void calcNotes();                      // .text:0xE34
    void initNotePos();                    // .text:0xE80
    void loadTune();                       // .text:0xF0C
    void saveTune();                       // .text:0xF48
    void initAnms();                       // .text:0xF88
    void updateNotes();                    // .text:0x1028
    void calcLeafAnm();                    // .text:0x10F4
    u8 getRow(u8 note) const;              // .text:0x1610
    u8 getNote(u8 row) const;              // .text:0x1620
    void showPlayTutorial();               // .text:0x1630

    virtual int state2() {
        return SUCCEEDED;
    }
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

    // Implicit destructor: .text:0x16AC.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt; // the window
    /* 0x240 */ dMenuBtn_c mBtn;
    /* 0x784 */ Note_c mNoteLyt[NOTE_NUM];
    /* 0x1844 */ m2d::Anm_c mToneAnm[NOTE_NUM]; // one frame per row
    /* 0x1A84 */ m2d::AnmEx_c mLeafAnm;         // LEAF_ANM_e
    /* 0x1AC0 */ int mCursor;
    /* 0x1AC4 */ int mPlayPos; // the note being played, -1 none
    /* 0x1AC8 */ f32 mGrabY;   // the pointer's y when the note was grabbed
    /* 0x1ACC */ int mGrabRow; // the row of the grabbed note
    /* 0x1AD0 */ int mLoadCnt; // the notes created so far
    /* 0x1AD4 */ u8 mTune[NOTE_NUM];
}; // size 0x1AE4

#endif
