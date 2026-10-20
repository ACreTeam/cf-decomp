#ifndef GAME_D_MENU_MII_HPP
#define GAME_D_MENU_MII_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>

// The Mii picker (REL d_menu_miiNP, profile MENU_MII): a window of Mii icons
// (Layout/slct_mii.arc) from which the player's Mii is chosen out of the
// official Mii database. RTTI confirms the global class name; everything else
// is inferred.
//
// The list (mMiiIndex) holds the database indexes of the available Miis,
// favorites first. It is shown ten to a page, in columns: each page is 5
// columns of 2 icons (the upper row holds the first 5 Miis of the page, the
// lower row the next 5). The window shows 6 columns of 2 icons (ICON_NUM),
// scrolled by mScroll columns; the icon textures are captured from the Miis
// into 12 buffers that are used as a ring of 6 columns.
class dMenuMii_c : public dMenuBase_c {
  public:
    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,
        FLAG_UPDATE,       // the layout changed
        FLAG_BEFORE_ALPHA, // the "before" frame follows its icon's alpha
    };

    enum SUB_STATE_e { SUB_OPEN, SUB_MAIN, SUB_PUSH, SUB_SCROLL };

    // The cursor (mCursor): 0..11 the Mii icons (6 columns of 2 rows, the
    // lower row from 6), 12 and 13 the scroll buttons, 14..16 the button bar's
    // buttons.
    enum {
        CURSOR_NONE = -1,
        CURSOR_ICON_FIRST = 0,
        ICON_COLUMNS = 6,
        ICON_NUM = ICON_COLUMNS * 2,
        CURSOR_ICON_LAST = ICON_NUM - 1,
        CURSOR_SCRL_FIRST = ICON_NUM,
        SCRL_NUM = 2,
        CURSOR_BTN_FIRST = CURSOR_SCRL_FIRST + SCRL_NUM,
    };

    // The scroll buttons (CURSOR_SCRL_FIRST + SCRL_e).
    enum SCRL_e { SCRL_LEFT, SCRL_RIGHT };

    // The button bar's buttons.
    enum BTN_e {
        BTN_CANCEL,
        BTN_UNUSED, // hidden
        BTN_OK,
    };

    // The animations of mScrlBtnAnm (the order of the names in initAnm).
    enum SCRL_ANM_e {
        SCRL_ANM_IN,
        SCRL_ANM_OUT,
        SCRL_ANM_PUSH,
        SCRL_ANM_NO_PUSH,
        SCRL_ANM_CAN_PUSH,
    };

    // The animations of mFlipAnm: the icons slide one column to the right (the
    // left button) or to the left, each as start / middle / end.
    enum FLIP_ANM_e {
        FLIP_ANM_RIGHT_START,
        FLIP_ANM_RIGHT_MIDDLE,
        FLIP_ANM_RIGHT_END,
        FLIP_ANM_LEFT_START,
        FLIP_ANM_LEFT_MIDDLE,
        FLIP_ANM_LEFT_END,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_CONFIRM = 0x22,
        MSG_CHOOSE_MII = 0x84,
        MSG_PAGE = 0x85, // "<page>/<pages>"
    };

    enum {
        MII_MAX = 100,        // the official database's slots
        MII_NONE = 0xFFFF,    // an empty list entry
        PAGE_COLUMNS = 5,     // columns of a page
        PAGE_SIZE = 10,       // Miis of a page
        CAPTURE_SIZE = 0x58,  // the captured icon's width and height
        CAPTURE_BUF = 0x3C80, // ... and its buffer (RGB565)
        SCROLL_REPEAT = 5,    // columns a held scroll button scrolls
    };

    static bool isIcon(int slot) {
        return slot >= CURSOR_ICON_FIRST && slot <= CURSOR_ICON_LAST;
    }
    static bool isScrl(int slot) {
        return slot >= CURSOR_SCRL_FIRST &&
               slot <= CURSOR_SCRL_FIRST + SCRL_NUM - 1;
    }
    static bool isBtn(int slot) {
        return slot >= CURSOR_BTN_FIRST &&
               slot <= CURSOR_BTN_FIRST + dMenuBtn_c::BTN_NUM - 1;
    }

    virtual int create();     // .text:0x2A0
    virtual int doDelete();   // .text:0x3AC
    virtual int stateMain();  // .text:0x404
    virtual int stateAnm();   // .text:0x428
    virtual int stateClose(); // .text:0x478
    virtual int execute();    // .text:0x49C
    virtual int draw();       // .text:0x570

    void subStateOpen();                   // .text:0x5BC
    void subStateMain();                   // .text:0x638
    void subStatePush();                   // .text:0x700
    void subStateScroll();                 // .text:0x73C
    void executeSubState();                // .text:0x800
    void initButtons();                    // .text:0x838
    void setOkDisable(int disable);        // .text:0x894
    void initAnm();                        // .text:0x8B0
    void calcAnm();                        // .text:0x9C0
    void setPushPos(int icon);             // .text:0xA58
    BOOL selectIcon(int icon);             // .text:0xAD8
    void setBeforeIcon(int icon);          // .text:0xB64
    void hideBeforeIcon();                 // .text:0xC14
    void updateBeforeIcon();               // .text:0xC58
    void updateBeforeAlpha();              // .text:0xCA8
    void initScrlBtnAnm();                 // .text:0xD14
    BOOL canScroll(int btn);               // .text:0xD94
    BOOL canScrollLeft();                  // .text:0xDA4
    BOOL canScrollRight();                 // .text:0xDB8
    void scroll(int slot, BOOL first);     // .text:0xDDC
    void setSelect(int slot, BOOL select); // .text:0xF04
    int hitCheck();                        // .text:0x106C
    void checkPadScroll();                 // .text:0x1198
    void push();                           // .text:0x124C
    BOOL isPushEnd();                      // .text:0x136C
    void onPushEnd();                      // .text:0x1428
    void startCloseAnm();                  // .text:0x156C
    void initMessage();                    // .text:0x15AC
    void initCaptures();                   // .text:0x15C4
    void freeCaptures();                   // .text:0x1678
    void setPage(int page);                // .text:0x1704
    void initMiiList();                    // .text:0x17A8
    void bindIconBuffers();                // .text:0x1988
    void updateMiiIcon(int icon);          // .text:0x1A28
    void updateMiiIcons();                 // .text:0x1B3C
    int getListIndex(int icon);            // .text:0x1B88
    int getIcon(int listIndex);            // .text:0x1BF4
    u16 getMiiId(int icon);                // .text:0x1C74
    void refreshIcons();                   // .text:0x1CC0
    BOOL applyMii();                       // .text:0x1D08
    u16 getCurrentMiiIndex();              // .text:0x1DC0

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

    // Implicit destructor: .text:0x1E08.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::AnmEx_c mScrlBtnAnm[SCRL_NUM]; // scrlBtn_00 / _01
    /* 0x2B8 */ m2d::Anm_c mPushAnm;                // icnHole_Psh
    /* 0x2DC */ m2d::AnmEx_c mFlipAnm;              // the columns sliding
    /* 0x318 */ m2d::Anm_c mScrlIconAnm;            // scrlIcon_00
    /* 0x33C */ dMenuBtn_c mBtn;
    /* 0x880 */ nw4r::lyt::Pane *mpIconPane[ICON_NUM]; // MiiIcon_XX
    /* 0x8B0 */ void *mpIconBuf[ICON_NUM];             // their captured images
    /* 0x8E0 */ void *mpOrigImage; // the first icon's own image
    /* 0x8E4 */ m2d::captureNode_c mCapture[ICON_NUM];
    /* 0xA34 */ int mCursor;
    /* 0xA38 */ int mFocusIcon;   // the icon with the focus frame, or -1
    /* 0xA3C */ int mBeforeIcon;  // the icon of the current Mii, or -1
    /* 0xA40 */ int mScroll;      // the first visible column
    /* 0xA44 */ int mPage;        // the page shown in the page counter
    /* 0xA48 */ int mPageNum;     // the number of pages
    /* 0xA4C */ int mMiiNum;      // the available Miis
    /* 0xA50 */ int mSelected;    // the chosen list index, or -1
    /* 0xA54 */ int mScrollCount; // columns left to scroll
    /* 0xA58 */ u16 mMiiIndex[MII_MAX];
}; // size 0xB20

#endif
