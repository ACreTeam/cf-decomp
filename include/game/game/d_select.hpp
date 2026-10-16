#pragma once

// The choice window (TU d_select.cpp, .text 8015B680..8015F2B0). Class names
// from the RTTI: dSelect::BaseComp_c, dSelect::BComp_c, dSelect::Comp_c,
// dSelect::MComp_c, dSelect::Separate_c, dSelect::Word_c. Mode names from the
// mode table; member and function names are inferred from behaviour.
// - BaseComp_c: the interface the message controller drives
// (dMsg::Comp_c::mpCurSelect); also the first base of
//   dAnalogSelect::Comp_c. Its virtuals (and so its vtable) are in
//   d_select.cpp; the weak copies of its inline dtor and RTTI, and Separate_c's
//   ctor, dtor, vtable and RTTI, are kept in d_analog_select (earlier in link
//   order).
// - BComp_c: the up to five answer window ("Layout/chc_win.arc"), with a dState
// mode table.
// - Comp_c: the message controller's answers (dMsg::Comp_c::mSelect, +0x5610);
// the controller's ctor/dtor
//   inline its ctor/dtor.
// - MComp_c: the menu variant ("sys_2D/SYS_2D_menu" answers placed at a screen
// position; only RELs use it).

#include <types.h>
#include <game/game/d_string.hpp>
#include <game/game/d_state.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_item.hpp>
#include <nw4r/lyt.h>

class dPersonalID_c;
class dAnmPersonalID_c;

namespace dSelect {

BOOL isDecideTrig(); // 8015B680
BOOL isCancelTrig(); // 8015B684
BOOL isUpHold();     // 8015B688: up held (pad or core controller)
BOOL isDownHold();   // 8015B6E4: down held (pad or core controller)

// The text of an answer.
class Word_c : public dString::WordBase_c {
  public:
    Word_c();                         // 8015EE64
    virtual ~Word_c();                // 8015EEA8
    virtual u32 getBufferSize();      // 8015EF00
    virtual wchar_t *getBuffer();     // 8015EF08
    virtual int procRubyTag(int pos); // 8015EF10

    /* 0x24 */ wchar_t mBuffer[101];
}; // size 0xF0

// An answer.
class Separate_c {
  public:
    Separate_c() {
    }
    virtual ~Separate_c() {
    }
    void clear();                     // 8015B740
    void setLabel(const char *label); // 8015B798
    void applyText() {
        mpTextBox->SetString(mWord.getBuffer(), 0);
    }
    Word_c *getWord() {
        return &mWord;
    }

    /* 0x004 */ nw4r::lyt::TextBox *mpTextBox;
    /* 0x008 */ u16 mMsgCode; // the message shown when this answer is taken
    /* 0x00A */ char mLabel[61];
    /* 0x048 */ Word_c mWord;
    /* 0x138 */ int
        mIsCancel; // the decide SE (0 normal, 1 cancel: no close SE, 2, 3)
    /* 0x13C */ bool
        mAddPeriod; // appends a period for languages 1, 3, 4, 6, 7 and 8
}; // size 0x140

class BaseComp_c {
  public:
    virtual ~BaseComp_c() {
    }
    virtual void open(); // 8015B7EC
    virtual bool
    isClosed(); // 8015B7F0: an answer was taken and the window is gone
    virtual void reset();   // 8015B7F8
    virtual void onOpen();  // 8015B818: plays the open SE
    virtual void onClose(); // 8015B820

    /* 0x004 */ u8 mCursor; // the answer taken
    /* 0x005 */ u8 _005;
    /* 0x006 */ u16 mCode;       // the answer's message code
    /* 0x008 */ char mLabel[61]; // the answer's message label
}; // size 0x48

class BComp_c : public BaseComp_c, public dState::mode_c<BComp_c> {
  public:
    enum { NUM_ANSWERS = 5 };

    BComp_c();          // 8015B824
    virtual ~BComp_c(); // 8015B8E4

    virtual void open() {
        changeMode(&BComp_c::executeCheck);
        mNextMode = &BComp_c::executeSelectMsg;
    } // 8015EFB8
    virtual bool isClosed() {
        return isMode(&BComp_c::executeHidden);
    }                          // 8015EF14
    virtual void reset();      // 8015D158
    virtual void onOpen();     // 8015E18C
    virtual void onClose();    // 8015E244
    virtual void cancel();     // 8015E01C: takes the cancel answer
    virtual void decide();     // 8015DBDC: takes the current answer
    virtual void preExecute(); // 8015E248

    bool isMode(Method method) const {
        return mCurrentMethod == method;
    }
    bool isNextMode(Method method) const {
        return mNextMode == method;
    }
    bool isIn() {
        return isMode(&BComp_c::executeIn);
    }
    bool isOut() {
        return isMode(&BComp_c::executeOut);
    }
    void requestClose() {
        if (!isClosed()) {
            changeMode(&BComp_c::executeOut);
        }
    }
    f32 getWindowWidth() {
        return mMarginRight + (mTextWidth + mMarginLeft);
    }
    f32 getWindowHeight() {
        return mMarginBottom + (mTextHeight + mMarginTop);
    }

    BOOL load();          // 8015B998
    void build();         // 8015B9A8
    void setPinkWindow(); // 8015BB88
    BOOL create();        // 8015BBE8
    BOOL remove();        // 8015BC44
    BOOL update();        // 8015BCE0
    BOOL draw();          // 8015BD44
    void setWord(int idx, dScript::Word_c *word, u16 code, const char *label,
                 int isCancel,
                 int addPeriod); // 8015BE70
    void setMessage(int idx, u16 index, const char *group, u16 code,
                    const char *label, int isCancel,
                    int addPeriod); // 8015BEF4
    void setSysMessage(int idx, u16 index, u16 code, const char *label,
                       int isCancel,
                       int addPeriod); // 8015BF74: "sys_SELECT/SYS_Select"
    void setNpcMessage(int idx, u16 index, u16 code, const char *label,
                       int isCancel,
                       int addPeriod); // 8015BF9C: "sys_SELECT/SYS_SelectNPC"
    void setPlayerName(int idx, const dPersonalID_c *id, u16 code,
                       const char *label, int isCancel,
                       int addPeriod); // 8015BFC4
    void setAnimalName(int idx, dAnmPersonalID_c *id, u16 code,
                       const char *label, int isCancel,
                       int addPeriod); // 8015C06C
    void setItemName(int idx, dItem::Item item, u16 code, const char *label,
                     int isCancel,
                     int addPeriod);               // stripped
    f32 getTextWidth(nw4r::lyt::TextBox *textBox); // 8015C0F8

    void initHidden();     // 8015C108
    void executeHidden();  // 8015C10C
    void initCheck();      // 8015C110: collects the words for the NG word check
    void executeCheck();   // 8015C1CC
    void initReplace();    // 8015C474
    void executeReplace(); // 8015C478: censors the NG words
    void initIn();         // 8015C69C
    void executeIn();      // 8015C718
    void initSelectMsg();  // 8015C89C
    void executeSelectMsg(); // 8015C908
    void initSelectDpd();    // 8015C9D4
    void executeSelectDpd(); // 8015CA20
    void initSelectKey();    // 8015CB34
    void executeSelectKey(); // 8015CB6C
    void initDecide();       // 8015CC00
    void executeDecide();    // 8015CC74
    void initLock();         // 8015CF58
    void executeLock();      // 8015CF6C
    void initOut();          // 8015CF70
    void executeOut();       // 8015CFD8

    void unlock(bool close);                // 8015D214
    Separate_c *getSeparate(int idx);       // 8015D4B0
    void setResultLabel(const char *label); // 8015D4C0
    void updateVisible();                   // 8015D514
    f32 getMaxTextWidth();                  // 8015D5F8
    f32 getTextHeight();                    // 8015D680
    void calcWindowSize();                  // 8015D6B0
    void resizeWindow();                    // 8015D760
    void updateCursor();                    // 8015D960
    void changeTextAnm(int idx, int anm);   // 8015DAEC
    void setKeyMode(bool keyMode);          // 8015DB9C
    void updatePointer();                   // 8015DDA0
    void updateKey();                       // 8015DEAC
    void setTexts();                        // 8015E1C0

    /* 0x06C */ Method
        mNextMode; // the select mode open() goes to after the in animation
    /* 0x078 */ m2d::ResAccLoader_c mResources;
    /* 0x14C */ m2d::SimpleEx_c mLayout;
    /* 0x1E4 */ m2d::Anm_c mWindowAnm;
    /* 0x208 */ m2d::Anm_c mTextAnms[NUM_ANSWERS];
    /* 0x2BC */ m2d::Anm_c mCursorAnm;
    /* 0x2E0 */ nw4r::lyt::Pane *mpAll;
    /* 0x2E4 */ nw4r::lyt::Pane *mpWindow;
    /* 0x2E8 */ nw4r::lyt::Pane *mpShadow;
    /* 0x2EC */ nw4r::lyt::Pane *mpCursor;
    /* 0x2F0 */ Separate_c mSeparates[NUM_ANSWERS];
    /* 0x930 */ int mNum;     // number of answers
    /* 0x934 */ int mCurrent; // the answer under the cursor (-1: none)
    /* 0x938 */ int mCancel;  // the answer taken on cancel (-1: none)
    /* 0x93C */ f32 mTextBoxWidth;
    /* 0x940 */ f32 mLineWidth;
    /* 0x944 */ f32 mButtonWidth;
    /* 0x948 */ f32 mTextWidth;
    /* 0x94C */ f32 mTextHeight;
    /* 0x950 */ f32 mMarginLeft;
    /* 0x954 */ f32 mMarginRight;
    /* 0x958 */ f32 mMarginTop;
    /* 0x95C */ f32 mMarginBottom;
    /* 0x960 */ u32
        mLockMask; // answers that lock the window instead of closing it
    /* 0x964 */ bool mKeyMode; // the cursor follows the pad, not the pointer
    /* 0x968 */ dScript::Word_c *mCheckWords[NUM_ANSWERS];
    /* 0x97C */ const u16 *mCheckTexts[NUM_ANSWERS];
    /* 0x990 */ u8 mCheckResults[NUM_ANSWERS];
    /* 0x998 */ int mCheckNum;
    /* 0x99C */ int mCheckHits;
    /* 0x9A0 */ int mRepeatTimer;
    /* 0x9A4 */ int mRepeatDir;
    /* 0x9A8 */ bool mCreated;
}; // size 0x9AC

// The message controller's answers.
class Comp_c : public BComp_c {
  public:
    Comp_c() : mDecideDelay(0), mCancelDelay(0) {
    }
    virtual ~Comp_c() {
    }                          // 8015F130
    virtual void onOpen();     // 8015E388
    virtual void onClose();    // 8015E3D4
    virtual void cancel();     // 8015E260
    virtual void decide();     // 8015E24C
    virtual void preExecute(); // 8015E3D8

    bool create();       // 8015E274
    bool remove();       // 8015E2A0
    void updateLayout(); // 8015E2CC
    void moveWindow();   // 8015E310

    /* 0x9AC */ int mDecideDelay;
    /* 0x9B0 */ int mCancelDelay;
}; // size 0x9B4

// The menu answers.
class MComp_c : public BComp_c {
  public:
    virtual ~MComp_c() {
    } // 8015EF60

    // Opens with pointer (DPD) or key selection, like BComp_c::open (used by
    // d_menu_itemxxNP).
    void openPointer() {
        changeMode(&BComp_c::executeCheck);
        mNextMode = &BComp_c::executeSelectDpd;
    }
    void openKey() {
        changeMode(&BComp_c::executeCheck);
        mNextMode = &BComp_c::executeSelectKey;
    }
    virtual void onOpen();     // 8015EE58
    virtual void onClose();    // 8015EE5C
    virtual void preExecute(); // 8015EE60

    void addItem(u16 index, u16 code, int lock, int isCancel); // 8015E404
    void setCancelLast();                                      // 8015E4B8
    void placeAbove(mVec2_c pos);                              // 8015E4C8
    void placeBelow(mVec2_c pos);                              // 8015E5C0
    void place(mVec2_c pos);                                   // 8015E6B8
    void place(mVec2_c *below, mVec2_c *above);                // 8015E844
    void updateLayout();                                       // 8015E9FC
    void fitScreen();                                          // 8015EA40
    f32 getLeft();                                             // 8015EA74
    f32 getCursorY();                                          // 8015EAA0
    BOOL isReady();                                            // 8015EAE4
    void close();                                              // 8015EB94
    void clampToScreen();                                      // 8015ED08
};

} // namespace dSelect
