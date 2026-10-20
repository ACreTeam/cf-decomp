#ifndef GAME_D_MENU_CHAT_HPP
#define GAME_D_MENU_CHAT_HPP

#include <game/mLib/m_2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_obj_keyboard.hpp>

// The chat menu of the bottom UI panel (REL d_menu_chatNP, profile MENU_CHAT):
// a text box with the software keyboard (Layout/keyboard_free.arc) to type a
// message, the player's emotion buttons, a say button, and a second mode with
// the preset phrase window (Layout/teikei_win.arc: five genres, six rows,
// scroll buttons and a slider). The change button switches between both modes.
// RTTI confirms the global class names dMenuChat_c and
// dMenuChat_c::KeyboardMng_c; everything else is inferred.
class dMenuChat_c : public dMenuBase_c {
  public:
    // What the pointer / cursor points at (mCursor).
    enum CURSOR_e {
        CURSOR_NONE = -1,
        CURSOR_TAB = 0, // 0..7: the tabs of the bottom UI panel
        CURSOR_TAB_LAST = 7,
        CURSOR_ACTION = 8, // 8..11: the emotion buttons
        CURSOR_ACTION_LAST = 11,
        CURSOR_ENTER = 12,  // the keyboard's enter button
        CURSOR_SAY = 13,    // the say button
        CURSOR_CHANGE = 14, // the mode change button
        CURSOR_ROW = 15,    // 15..20: the phrase rows
        CURSOR_ROW_LAST = 20,
        CURSOR_SCROLL_UP = 21,
        CURSOR_SCROLL_DOWN = 22,
        CURSOR_SLIDER = 23,
        CURSOR_GENRE = 24, // 24..28: the genre buttons
        CURSOR_GENRE_LAST = 28,
    };

    // Sub states (setSubState): the first two are the modes.
    enum SUB_STATE_e {
        SUB_KEYBOARD,
        SUB_PRESET,
        SUB_ACTION_PUSH,      // the emotion button's push animation
        SUB_EMOTION,          // waits for the player to play the emotion
        SUB_EMOTION_END,      // waits for the emotion to end
        SUB_START_CHAT,       // waits for the player to start talking
        SUB_CHAT_END,         // waits for the talk to end
        SUB_CHANGE_TO_PRESET, // the change button's push animation (keyboard
                              // mode)
        SUB_PRESET_PUSH,      // a button's push animation (phrase mode)
        SUB_SCROLL_HOLD,      // a scroll button is held
        SUB_SLIDER,           // the slider is dragged
        SUB_NUM,
    };

    enum MODE_e {
        MODE_KEYBOARD,
        MODE_PRESET,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_TEXT, // the text layout needs a calc
        FLAG_DRAW,
        FLAG_CURSOR,     // the text cursor may be shown
        FLAG_ACTION,     // the action layout needs a calc
        FLAG_KBD,        // the keyboard layout needs a calc
        FLAG_TAB_HIDDEN, // the panel's tabs are hidden
        FLAG_SAY,        // the say layout needs a calc
        FLAG_CHANGE,     // the change button layout needs a calc
        FLAG_PRESET_LYT, // the phrase window needs a calc
        FLAG_PRESET,     // the phrase window is the current mode
        FLAG_SWITCH,     // the window closes to reopen in the other mode
        FLAG_SCROLL,     // the rows are scrolling
        FLAG_KEEP_TIMER, // the text box was shown when an emotion was chosen
    };

    // The animations of a phrase row (mRowAnm).
    enum ROW_ANM_e {
        ROW_ANM_IN,
        ROW_ANM_OUT,
        ROW_ANM_PUSH,
        ROW_ANM_GRAY,
        ROW_ANM_WHITE,
    };

    // The animations of the action buttons (mActionAnm).
    enum ACTION_ANM_e {
        ACTION_ANM_OUT,
        ACTION_ANM_IN,
    };

    // The animations of the enter, say and change buttons (the cursor on it,
    // off it, pushed).
    enum BTN_ANM_e {
        BTN_ANM_FOCUS,
        BTN_ANM_UNFOCUS,
        BTN_ANM_PUSH,
    };

    // The animations of the text box and the say button's alpha (mTextAnm,
    // mSayAlphaAnm).
    enum SHOW_ANM_e {
        SHOW_ANM_IN,
        SHOW_ANM_OUT,
    };

    // The animations of a scroll button (mScrollBtnAnm).
    enum SCROLL_BTN_ANM_e {
        SCROLL_BTN_ANM_IN,
        SCROLL_BTN_ANM_OUT,
        SCROLL_BTN_ANM_PUSH,
        SCROLL_BTN_ANM_CAN_PUSH,
        SCROLL_BTN_ANM_NO_PUSH,
    };

    // The animations of a genre button (mGenreAnm) and the slider (mSliderAnm).
    enum GENRE_ANM_e {
        GENRE_ANM_ON,
        GENRE_ANM_OFF,
    };
    enum SLIDER_ANM_e {
        SLIDER_ANM_IN,
        SLIDER_ANM_OUT,
    };

    // The row scroll animation (mScrollAnm).
    enum SCROLL_ANM_e {
        SCROLL_ANM_DOWN,
        SCROLL_ANM_UP,
    };

    enum {
        ACTION_NUM = 4,
        ROW_NUM = 6,
        GENRE_NUM = 5,
        TEXT_MAX = 0x20,
        TEXT_SIZE = 0x21,
        TEXT_SHOW_FRAMES = 1800,
        NO_ACTION = 0xFF,
    };

    static bool isTab(int cursor) {
        return cursor >= CURSOR_TAB && cursor <= CURSOR_TAB_LAST;
    }
    static bool isAction(int cursor) {
        return cursor >= CURSOR_ACTION && cursor <= CURSOR_ACTION_LAST;
    }
    static bool isRow(int cursor) {
        return cursor >= CURSOR_ROW && cursor <= CURSOR_ROW_LAST;
    }
    static bool isScrollBtn(int cursor) {
        return cursor >= CURSOR_SCROLL_UP && cursor <= CURSOR_SCROLL_DOWN;
    }
    static bool isGenre(int cursor) {
        return cursor >= CURSOR_GENRE && cursor <= CURSOR_GENRE_LAST;
    }

    typedef BOOL (dMenuChat_c::*CloseProc)();
    typedef void (dMenuChat_c::*SubStateProc)();

    // The keyboard manager that draws the chat layouts around the keyboard.
    class KeyboardMng_c : public dKbd::Mng_c {
      public:
        enum ORDER_e {
            ORDER_BEFORE, // the enter button is drawn before the keyboard
            ORDER_AFTER,  // and after it, while it is pushed
        };

        virtual ~KeyboardMng_c();
        virtual void draw(); // .text:0x4174

        void setLayers(m2d::Base_c *kbd, m2d::Base_c *action,
                       m2d::Base_c *change); // .text:0x4248

        /* 0x378 */ m2d::Base_c *mpKbdLyt;
        /* 0x37C */ m2d::Base_c *mpActionLyt;
        /* 0x380 */ m2d::Base_c *mpChangeLyt;
        /* 0x384 */ u8 mOrder; // ORDER_e
    }; // size 0x388

    virtual int create();     // .text:0x348
    virtual int doDelete();   // .text:0x6E0
    virtual int stateMain();  // .text:0x768
    virtual int stateAnm();   // .text:0x7D4
    virtual int stateClose(); // .text:0xA8C
    virtual int execute();    // .text:0xB68
    virtual int draw();       // .text:0xBC4

    void setKbdPos(const mVec3_c *pos);                         // .text:0x96C
    BOOL checkTabInput();                                       // .text:0xC84
    void subStateKeyboard();                                    // .text:0xD2C
    void subStatePreset();                                      // .text:0xFDC
    void subStateActionPush();                                  // .text:0x1124
    void subStateEmotion();                                     // .text:0x1188
    void subStateEmotionEnd();                                  // .text:0x11E0
    void subStateStartChat();                                   // .text:0x123C
    void subStateChatEnd();                                     // .text:0x1288
    void subStateChangeToPreset();                              // .text:0x12C8
    void subStatePresetPush();                                  // .text:0x1304
    void subStateScrollHold();                                  // .text:0x1340
    void subStateSlider();                                      // .text:0x13F4
    BOOL closeMenu();                                           // .text:0x1554
    BOOL checkCloseRequest();                                   // .text:0x15A0
    void executeSubState();                                     // .text:0x16E0
    BOOL checkLength();                                         // .text:0x1718
    void updateText();                                          // .text:0x1814
    void showPreset();                                          // .text:0x18D8
    void restoreText();                                         // .text:0x197C
    void reopenKbd();                                           // .text:0x19C8
    int getTabAt(const mVec2_c *pos);                           // .text:0x1A08
    BOOL selectTab(int tab, BOOL delay);                        // .text:0x1A6C
    void startClose();                                          // .text:0x1ABC
    void startSwitch();                                         // .text:0x1B18
    int hitAction(const mVec2_c *pos);                          // .text:0x1B78
    void selectAction(int slot);                                // .text:0x1BCC
    u8 getAction(int slot);                                     // .text:0x1C6C
    BOOL checkActionKey();                                      // .text:0x1CD4
    void updateCursor();                                        // .text:0x1D4C
    BOOL checkPointerCursor();                                  // .text:0x1DA4
    void showActionTutorial(int slot);                          // .text:0x1E70
    void showGenreTutorial(int genre);                          // .text:0x1F2C
    void initTextAnm();                                         // .text:0x1FD8
    void resetTimer();                                          // .text:0x20A0
    void updateTimer();                                         // .text:0x20AC
    void saveTimer();                                           // .text:0x211C
    void initKbdLyt();                                          // .text:0x216C
    void updateKbdVisible();                                    // .text:0x2258
    BOOL hitKbdEnter();                                         // .text:0x2354
    BOOL sendText(BOOL kbd);                                    // .text:0x23D4
    void sendPreset();                                          // .text:0x2494
    void clearText();                                           // .text:0x24E8
    BOOL hitSay();                                              // .text:0x2544
    void initChangeBtn();                                       // .text:0x25D8
    void setChangeIcon(int mode);                               // .text:0x2664
    BOOL hitChange();                                           // .text:0x26D0
    void initTeikei();                                          // .text:0x2750
    void initRowAnm();                                          // .text:0x2868
    void playRowAnm();                                          // .text:0x2930
    void updateRowWords();                                      // .text:0x29AC
    void getRowWord(dScript::Word_c *word, int group, int idx); // .text:0x2A1C
    void updateRowSelect();                                     // .text:0x2AAC
    void setRowAnms(int select, int shift);                     // .text:0x2B68
    void updateRows();                                          // .text:0x2C18
    void initGenreAnm();                                        // .text:0x2C5C
    void playGenreAnm();                                        // .text:0x2D08
    void selectGroup(int group);                                // .text:0x2D68
    void initScrollAnm();                                       // .text:0x2DE8
    void playScrollAnm();                                       // .text:0x2E74
    void setScrollAnm(int dir);                                 // .text:0x2F8C
    BOOL startScroll(int dir);                                  // .text:0x2FAC
    void updateScrollBtns();                                    // .text:0x3090
    bool canScroll(int dir);                                    // .text:0x3110
    bool canScrollUp();                                         // .text:0x3120
    bool canScrollDown();                                       // .text:0x3140
    void initSlider();                                          // .text:0x3164
    void updateSliderPos();                                     // .text:0x31D4
    void setSelect(int cursor, BOOL select);                    // .text:0x32A0
    int hitCheck();                                             // .text:0x34A0
    int hitCheckKbd();                                          // .text:0x3550
    int hitCheckPreset();                                       // .text:0x35B4
    BOOL pushCursorKbd();                                       // .text:0x36AC
    BOOL isChangePushEnd();                                     // .text:0x37DC
    void finishChangePush();                                    // .text:0x37F8
    void padPreset();                                           // .text:0x3844
    BOOL pushCursorPreset();                                    // .text:0x3984
    BOOL isPresetPushEnd();                                     // .text:0x3C9C
    void finishPresetPush();                                    // .text:0x3D48
    void restoreSubState();                                     // .text:0x3E98
    void setMode(u8 mode);                                      // .text:0x3EAC
    void playAnm();                                             // .text:0x3EB4
    void calcLayouts();                                         // .text:0x4018

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x4558
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x4550
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x454C
    virtual void deleteReady() {
    } // .text:0x4548
    virtual bool createHeap() {
        return true;
    } // .text:0x4540

    // Implicit destructor: .text:0x4258.

    /* 0x0D4 */ m2d::ResAccLoader_c mKbdRes;
    /* 0x1A8 */ m2d::ResAccLoader_c mTeikeiRes;
    /* 0x27C */ m2d::SimpleEx_c mLytAll;
    /* 0x314 */ m2d::SimpleEx_c mLytText;
    /* 0x3AC */ m2d::AnmEx_c mTextAnm;
    /* 0x3E8 */ m2d::Anm_c mTextUpAnm;
    /* 0x40C */ m2d::SimpleEx_c mLytAction;
    /* 0x4A4 */ m2d::AnmEx2_c mActionAnm[ACTION_NUM];
    /* 0x5B4 */ m2d::Anm_c mActionPushAnm;
    /* 0x5D8 */ m2d::AnmEx2_c mActionIconAnm[ACTION_NUM];
    /* 0x6E8 */ int mActionSlot;
    /* 0x6EC */ int mCursor;
    /* 0x6F0 */ m2d::SimpleEx_c mLytKbd;
    /* 0x788 */ m2d::AnmEx_c mEnterAnm;
    /* 0x7C4 */ m2d::SimpleEx_c mLytSay;
    /* 0x85C */ m2d::AnmEx_c mSayAlphaAnm;
    /* 0x898 */ m2d::AnmEx_c mSayAnm;
    /* 0x8D4 */ m2d::SimpleEx_c mLytChange;
    /* 0x96C */ m2d::Anm_c mChangeIconAnm;
    /* 0x990 */ m2d::AnmEx_c mChangeAnm;
    /* 0x9CC */ m2d::SimpleEx_c mLytTeikei;
    /* 0xA64 */ m2d::AnmEx2_c mRowAnm[ROW_NUM];
    /* 0xBFC */ m2d::stringWord_c mRowWord[ROW_NUM];
    /* 0x11B4 */ m2d::AnmEx_c mScrollBtnAnm[2];
    /* 0x122C */ m2d::Anm_c mScrollAnm;
    /* 0x1250 */ m2d::Anm_c mScrollIconAnm;
    /* 0x1274 */ m2d::AnmEx2_c mGenreAnm[GENRE_NUM];
    /* 0x13C8 */ m2d::Anm_c mGenrePushAnm;
    /* 0x13EC */ int mPreset;      // the chosen phrase of mPresetGroup, -1 none
    /* 0x13F0 */ int mPresetGroup; // the genre of the chosen phrase
    /* 0x13F4 */ int mScroll;      // the phrase shown on the first row
    /* 0x13F8 */ int mPresetNum;   // the phrases of the current genre
    /* 0x13FC */ int mGroup;       // the current genre
    /* 0x1400 */ KeyboardMng_c mKbd;
    /* 0x1788 */ dKbd::Cur_c mCur;
    /* 0x1940 */ dObjKeyboard_c::Word_c mWord;
    /* 0x19C8 */ wchar_t mText[TEXT_SIZE];
    /* 0x1A0C */ int mTimer;
    /* 0x1A10 */ u8 mMode;
    /* 0x1A11 */ u8 mNextMode;
    /* 0x1A14 */ m2d::AnmEx_c mSliderAnm;
    /* 0x1A50 */ f32 mSliderTop;
    /* 0x1A54 */ f32 mSliderBottom;
    /* 0x1A58 */ nw4r::lyt::Pane *mpSlider;
    /* 0x1A5C */ f32 mSliderGrab;
    /* 0x1A60 */ m2d::zBuffClear_c mZBuff;
}; // size 0x1A88

#endif
