#pragma once

// The bulletin board menu (REL d_menu_noticeNP, profile MENU_NOTICE, menu
// number dMenuMng_c::MENU_BBS) and the notice paper layout dNotice_c (RTTI
// names; global scope). The menu is the base module of the board's sub menus:
// it loads Layout/board.arc, shows the paper with one notice on it and opens
// the sub menu that works on the notice (REL d_menu_notice00NP reads the
// notices, REL d_menu_notice01NP writes a new one on the keyboard). The sub
// menus talk to the open menu through the static functions of dMenuNotice_c;
// d_menu_notice00NP also keeps a dNotice_c of its own. Names other than the
// RTTI names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_sv_notice.hpp>
#include <game/mLib/m_vec.hpp>

// The paper of the board (Layout/board.arc, board_win_paper_00.brlyt): six
// lines of text for the notice, the sender (T_msg_07) and the date (T_msg_00).
// It holds a copy of the notice that it shows (mNotice).
//
// Text positions are character indexes into the notice text. The text is
// wrapped into lines by setText; mLineLengths holds their lengths (a line that
// ends at a newline character counts it) and mNewLineBits has a bit for each of
// those lines. A part of the text (mSelectStart, mSelectLength) is drawn in the
// selection color, which is how the keyboard's composition is shown.
class dNotice_c {
  public:
    enum {
        LINE_NUM = 6,
        MAX_LINE_LENGTH = 32, // characters per line
        WORD_BUFFER_LENGTH = 61,
        LINE_BUFFER_LENGTH = WORD_BUFFER_LENGTH + 1,
        NO_LINE = -1,
        LYT_PRIORITY = 0x81, // m2d::Base_c::mPriority of the paper
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_DATE =
            0xB6, // "<month>/<day>/<year>", the numbers are words 1, 2, 0
    };

    // The number words of MSG_DATE.
    enum DATE_WORD_e {
        DATE_YEAR,
        DATE_MONTH,
        DATE_DAY,
    };

    // The animations of mAnm (the order of the names in initAnm): the paper
    // slides out to the right / in from the right / out to the left / in from
    // the left.
    enum ANM_e {
        ANM_R_OUT,
        ANM_R_IN,
        ANM_L_OUT,
        ANM_L_IN,
        ANM_NUM,
    };

    // What getLineEnd tells about the end of a line.
    enum LINE_END_e {
        LINE_END_NONE,    // another character can follow
        LINE_END_NEWLINE, // the line ends at a newline character
        LINE_END_TEXT,    // the line is the last one with text
    };

    // Bits of mFlags.
    enum FLAG_e {
        FLAG_UPDATE = 1,  // the layout changed and needs a calc
        FLAG_UPDATED = 2, // the layout was calculated by the last draw
    };

    // Packed RGBA storage, passed as bytes to the m2d color-tag helpers.
    union Color_c {
        u32 mPacked;
        u8 mBytes[4];
    };

    // The text of a line, of the sender and of the date.
    class Word_c : public m2d::Word_c {
      public:
        Word_c();                     // .text:0x1764
        virtual ~Word_c();            // .text:0x17A8
        virtual u32 getBufferSize();  // .text:0x1800
        virtual wchar_t *getBuffer(); // .text:0x1808

        /* 0x28 */ wchar_t mBuffer[WORD_BUFFER_LENGTH];
    }; // size 0xA4

    dNotice_c();          // .text:0x600
    virtual ~dNotice_c(); // .text:0x6B4

    void build(m2d::ResAccIf_c *res); // .text:0x75C: layout, text boxes, anm
    void draw();                      // .text:0x888: play the animation, calc
                                      // when needed, entry
    void
    copyPos(m2d::SimpleEx_c *lyt,
            const char *pane); // .text:0x914: put the paper on a pane of lyt
    // Wraps text into the lines. FALSE if it does not fit: nothing changes,
    // unless truncate is set, in which case the text is taken and still FALSE
    // is returned.
    BOOL setText(const wchar_t *text, BOOL truncate); // .text:0x95C
    void setNotice(const dNoticeData_c *notice);      // .text:0xB4C
    void updateAlign();  // .text:0xB8C: text alignment of the lines
    void updateAll();    // .text:0xC50: date, lines, sender
    void updateDate();   // .text:0xCA8: the date line
    void updateSender(); // .text:0xDA0: the sender line
    void updateText(BOOL fixedSpaces);     // .text:0xDFC: fill the lines
    void clear();                          // .text:0x1048
    void setPriority(u8 priority);         // .text:0x1084
    void setSelect(int start, int length); // .text:0x108C
    void initNew(); // .text:0x1098: empty notice of the current player,
                    // dated now
    int getLine(int pos, int *linePos); // .text:0x10F4: the line of a text
                                        // position, or -1
    mVec3_c getPos(int pos); // .text:0x1160: screen position of a text position
    int getLineEnd(mVec3_c *out, int line); // .text:0x11EC: LINE_END_e; out is
                                            // the end of the line's text
    int hitCheck(const mVec2_c *pos);       // .text:0x12F4: the line under a
                                            // screen position, or -1
    int getPosAtScreenX(int line, f32 x);   // .text:0x1374
    void setPos(f32 x, f32 y);              // .text:0x14B0
    void setPosX(f32 x);                    // .text:0x1540
    f32 getLineY(int line);                 // .text:0x15BC
    void post();      // .text:0x1604: add the notice to the board
    int getLineNum(); // .text:0x1660: lines used by the text
    void initAnm();   // .text:0x1668
    void resetAnm();  // .text:0x16B4: the paper in place (ANM_R_IN's end)
    void startAnm(int anm, BOOL now); // .text:0x16FC: start anm, or set it as
                                      // the next one
    BOOL isAnmEnd();                  // .text:0x1714: the animation ended

    BOOL isUpdated() const {
        return (u16)(mFlags & FLAG_UPDATED) != 0;
    }

  private:
    /* 0x004 */ m2d::ResAccIf_c *mpRes;
    /* 0x008 */ m2d::SimpleEx_c mLyt;
    /* 0x0A0 */ m2d::AnmEx_c mAnm;
    /* 0x0DC */ dNoticeData_c mNotice;
    /* 0x278 */ Word_c mWords[LINE_NUM];
    /* 0x650 */ Word_c mSenderWord;
    /* 0x6F4 */ Word_c mDateWord;
    /* 0x798 */ int mLineNum;
    /* 0x79C */ int mLineLengths[LINE_NUM];
    /* 0x7B4 */ u32 mNewLineBits;
    /* 0x7B8 */ int mSelectLength;
    /* 0x7BC */ int mSelectStart;
    /* 0x7C0 */ Color_c mTextWhite;
    /* 0x7C4 */ Color_c mTextBlack;
    /* 0x7C8 */ Color_c mSelectWhite;
    /* 0x7CC */ Color_c mSelectBlack;
    /* 0x7D0 */ u16 mFlags;
}; // size 0x7D4

class dMenuNotice_c : public dMenuBase_c {
  public:
    // The sub menus (mMenuIdx; the numbers are in l_openMenu).
    enum OPEN_MENU_e {
        OPEN_NEWEST, // the board with the newest notice open (REL
                     // d_menu_notice00NP)
        OPEN_LIST,   // the list of the notices (REL d_menu_notice00NP)
        OPEN_WRITE,  // write a notice (REL d_menu_notice01NP)
        OPEN_NUM,
    };

    // The sub states (setSubState).
    enum SUB_STATE_e {
        SUB_IDLE,        // the sub menu runs
        SUB_OPEN,        // open the sub menu mMenuIdx selects
        SUB_WAIT_CLOSE,  // wait for the sub menu to close, then close this one
        SUB_WAIT_REOPEN, // wait for the sub menu to close, then SUB_OPEN
        SUB_NUM,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW, // the paper is drawn
        FLAG_HIDE, // ... but not now
    };

    typedef void (dMenuNotice_c::*SubStateProc)();

    virtual int create();     // .text:0xB8
    virtual int doDelete();   // .text:0x144
    virtual int stateMain();  // .text:0x188
    virtual int stateAnm();   // .text:0x1AC
    virtual int stateClose(); // .text:0x1B0
    virtual int execute();    // .text:0x1D4
    virtual int draw();       // .text:0x1D8

    void subStateIdle();       // .text:0x230
    void subStateOpen();       // .text:0x234
    void subStateWaitClose();  // .text:0x290
    void subStateWaitReopen(); // .text:0x2A8
    void executeSubState();    // .text:0x2CC

    // Implicit destructor: .text:0x1810.

    // The interface of the sub menus (all work on the open menu, sInstance).

    // The requests fail (FALSE) while the menu is still handling a previous
    // one.
    static BOOL requestClose(); // .text:0x304: the sub menu closes, then this
                                // menu closes
    static BOOL requestOpenMenu(int idx); // .text:0x354: the sub menu closes,
                                          // then OPEN_MENU_e idx opens

    // Resources and flags.
    static m2d::ResAccLoader_c *getResAcc(); // .text:0x3AC: Layout/board.arc
    static void setHidden(BOOL hidden);      // .text:0x5B0: FLAG_HIDE

    // The paper.
    static void copyPos(m2d::SimpleEx_c *lyt,
                        const char *pane);              // .text:0x3BC
    static void setNotice(const dNoticeData_c *notice); // .text:0x404
    static void setPriority(u8 priority);               // .text:0x418
    static BOOL setText(const wchar_t *text);           // .text:0x42C
    static void updateText(BOOL fixedSpaces);           // .text:0x444
    static void setSelect(int start, int length);       // .text:0x458
    static void initNew();                              // .text:0x470
    static mVec3_c getPos(int pos);                     // .text:0x480
    static int getLineEnd(mVec3_c *out, int line);      // .text:0x494
    static int getLine(int pos);                        // .text:0x4AC
    static int hitCheck(const mVec2_c *pos);            // .text:0x4C4
    static int getPosAtScreenX(int line, f32 x);        // .text:0x4D8
    static void setPos(f32 x, f32 y);                   // .text:0x4EC
    static void setPosX(f32 x);                         // .text:0x4FC
    static f32 getLineY(int line);                      // .text:0x50C
    static void post();                                 // .text:0x520
    static void removeNotice(int idx); // .text:0x530: from the board
    static int getLineNum();           // .text:0x580
    static BOOL isUpdated();           // .text:0x590
    static void setNextAnm(int anm);   // .text:0x5D8
    static BOOL isAnmEnd();            // .text:0x5F0

    // Inline virtuals of the base class: .text:0x188C .. 0x18A4.
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

    static dMenuNotice_c *sInstance;

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ dNotice_c mNotice;
    /* 0x97C */ int mMenuIdx; // OPEN_MENU_e
}; // size 0x980

namespace dMenuNotice {
void *classInit();
}
