#ifndef GAME_D_MENU_MAP_HPP
#define GAME_D_MENU_MAP_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>

// The shared base of the town map menus (REL d_menu_mapNP; d_menu_map00NP..
// derive from it and construct it): the town map texture, an icon per
// building, the player's position and a scrolling list of the residents'
// houses or the facilities. RTTI confirms the global class name; everything
// else is inferred.
class dMenuMap_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_ROW = 0, // 0..5: the list rows
        SLOT_ROW_LAST = 5,
        SLOT_SCROLL_UP = 6,
        SLOT_SCROLL_DOWN = 7,
        SLOT_HOUSE_BUTTON = 8,    // "N_mapBtn_00": show the houses
        SLOT_FACILITY_BUTTON = 9, // "N_mapBtn_01": show the facilities
        SLOT_ICON = 10,           // 10..29: the building icons
        SLOT_ICON_LAST = 29,
        SLOT_TAB = 30, // 30..37: dAcUitag tabs (the derived menus)
        SLOT_TAB_LAST = 37,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_CALC_WINDOW,
        FLAG_DRAW, // the map texture is complete
        FLAG_SCROLL,
        FLAG_FACILITY, // the list shows the facilities
        FLAG_BUTTON_ANM,
        FLAG_CALC_ICONS,
    };

    // The list entries and the icons: the four player houses, the ten
    // villager houses and the six facilities.
    enum ENTRY_e {
        ENTRY_PLAYER = 0,
        ENTRY_PLAYER_LAST = 3,
        ENTRY_ANIMAL = 4,
        ENTRY_ANIMAL_LAST = 13,
        ENTRY_SHOP = 14,
        ENTRY_TAILOR,
        ENTRY_TOWN_HALL,
        ENTRY_MUSEUM,
        ENTRY_GATE,
        ENTRY_BUS_STOP,
        ENTRY_NUM,
        ENTRY_FACILITY = ENTRY_SHOP,
        ENTRY_FACILITY_LAST = ENTRY_BUS_STOP,
    };

    enum {
        ROW_NUM = 6,
        SCROLL_ROW_NUM = ROW_NUM - 1, // rows that count as shown when scrolling
        RESIDENT_NUM = ENTRY_FACILITY,
        FACILITY_NUM = ENTRY_NUM - ENTRY_FACILITY,
    };

    // Animations of the scroll and house/facility buttons (m2d::AnmEx_c
    // indices).
    enum BTN_ANM_e {
        BTN_ANM_OUT,
        BTN_ANM_IN,
        BTN_ANM_PUSH,
        BTN_ANM_NONE,
        BTN_ANM_NO_PUSH,  // scroll buttons only
        BTN_ANM_CAN_PUSH, // scroll buttons only
        SCROLL_BTN_ANM_NUM,
        MAP_BTN_ANM_NUM = BTN_ANM_NONE + 1,
    };

    // Animations of a list row's text and icon (m2d::AnmEx2_c indices).
    enum ROW_ANM_e {
        ROW_ANM_NORMAL,   // the icon shows its entry frame (getIconFrame)
        ROW_ANM_CURSOR,   // under the cursor
        ROW_ANM_PUSH,     // text only
        ROW_ANM_SELECTED, // the selected entry
        ROW_ANM_NUM,
    };

    // Messages: sys_2D/SYS_2D_menu, and sys_2D/SYS_2D_UIballoon for the
    // tutorials.
    enum {
        MSG_TOWN_MAP = 0x2B,
        MSG_SHOP = 0x2C,
        MSG_TAILOR = 0x2D,
        MSG_TOWN_HALL = 0x2E,
        MSG_MUSEUM = 0x2F,
        MSG_GATE = 0x30,
        MSG_BUS_STOP = 0xAA,
        TUTORIAL_MSG_HOUSES = 0x17, // "Houses"; + 1: "Buildings"
    };

    // Results of getIconScrollDir (and directions of startScroll).
    enum SCROLL_e {
        SCROLL_NEXT,
        SCROLL_PREV,
        SCROLL_NONE,
    };

    static bool isRow(int slot) {
        return slot >= SLOT_ROW && slot <= SLOT_ROW_LAST;
    }
    static bool isButton(int slot) {
        return slot >= SLOT_HOUSE_BUTTON && slot <= SLOT_FACILITY_BUTTON;
    }
    static bool isIcon(int slot) {
        return slot >= SLOT_ICON && slot <= SLOT_ICON_LAST;
    }
    static bool isTab(int slot) {
        return slot >= SLOT_TAB && slot <= SLOT_TAB_LAST;
    }

    virtual int preCreate();                     // .text:0x5C
    virtual void postCreate(MAIN_STATE_e state); // .text:0x88
    virtual void postDelete(MAIN_STATE_e state); // .text:0xA4
    virtual int execute();                       // .text:0xA8
    virtual int draw();                          // .text:0x260
    bool load();                                 // .text:0x2DC
    void createLayouts();                        // .text:0x2EC
    void createIcons();                          // .text:0x3BC
    BOOL remove();                               // .text:0x744
    int hitCheck(const mVec2_c *pos);            // .text:0x74C
    BOOL pushSlot(int slot);                     // .text:0x864
    void initAnms();                             // .text:0xAC8
    void calcButtonAnms();                       // .text:0xBAC
    BOOL makeMapTex();                           // .text:0xC70
    void initRows();                             // .text:0xCF8
    void setSlotAnm(int slot, BOOL on);          // .text:0xE48
    void setScrollAnm(int dir);                  // .text:0x1050
    void setListAnm();                           // .text:0x1070
    void calcScroll();                           // .text:0x10C8
    u8 *getList();                               // .text:0x115C
    int getListNum();                            // .text:0x11A0
    void initLists();                            // .text:0x11E4
    void updateRowWords();                       // .text:0x13C0
    f32 getIconFrame(u8 entry);                  // .text:0x1528
    void updateRowIcons();                       // .text:0x15AC
    bool showEntry(int entry);                   // .text:0x1678
    bool setFacilityMode(BOOL facility);         // .text:0x1710
    int findEntry(int entry);                    // .text:0x1798
    bool select(int entry);                      // .text:0x1818
    void changeList(BOOL facility);              // .text:0x18A0
    int getIconScrollDir();                      // .text:0x18E4
    void refreshList();                          // .text:0x1948
    void setRowCursor(int row);                  // .text:0x1990
    void updateRowSelect();                      // .text:0x1A14
    void startScroll(int dir);                   // .text:0x1AD8
    void updateScrollButtons();                  // .text:0x1B48
    BOOL canScrollPrev();                        // .text:0x1BE0
    BOOL canScrollNext();                        // .text:0x1BF4
    void setRowAnm(int row, int anm);            // .text:0x1C3C
    void calcBlink();                            // .text:0x1D20
    void showSlotTutorial(int slot);             // .text:0x1E98
    void hideTutorial();                         // .text:0x1F70
    void playSeModeChange();                     // .text:0x1F74
    void playSeListChoice();                     // .text:0x1F7C

    // True when row shows the selected entry.
    bool isSelectedRow(int row) {
        return mSelected >= 0 && mScroll + row < getListNum() &&
               mSelected == getList()[mScroll + row];
    }

    // Implicit destructor: .text:0x1F84.

    /* 0x00D4 */ m2d::ResAccLoader_c mRes;
    /* 0x01A8 */ m2d::SimpleEx_c mWinLyt;
    /* 0x0240 */ m2d::AnmEx_c mScrollPrevBtnAnm;
    /* 0x027C */ m2d::AnmEx_c mScrollNextBtnAnm;
    /* 0x02B8 */ m2d::AnmEx_c mHouseBtnAnm;
    /* 0x02F4 */ m2d::AnmEx_c mFacilityBtnAnm;
    /* 0x0330 */ m2d::Anm_c mScrollAnm;
    /* 0x0354 */ m2d::Anm_c mListAnm;
    /* 0x0378 */ m2d::AnmEx2_c mRowMsgAnms[ROW_NUM];
    /* 0x0510 */ m2d::AnmEx2_c mRowIconAnms[ROW_NUM];
    /* 0x06A8 */ m2d::Anm_c mScrollIconAnm;
    /* 0x06CC */ m2d::SimpleEx_c mIcons[ENTRY_NUM];
    /* 0x12AC */ m2d::Anm_c mIconAnms[ENTRY_NUM];
    /* 0x157C */ m2d::SimpleEx_c mPlayerIcon;
    /* 0x1614 */ int mCursor;    // SLOT_e
    /* 0x1618 */ int mMapColumn; // the next column of the map texture
    /* 0x161C */ u8 mResidents[RESIDENT_NUM]; // ENTRY_e
    /* 0x162A */ u8 mFacilities[FACILITY_NUM];
    /* 0x1630 */ int mResidentNum;
    /* 0x1634 */ int mFacilityNum;
    /* 0x1638 */ int mScroll;      // the first row's list index
    /* 0x163C */ int mShownScroll; // mScroll when the rows were written
    /* 0x1640 */ int mSelected;    // the selected entry, or -1
    /* 0x1644 */ m2d::stringWord_c mRowWords[ROW_NUM];
    /* 0x1BFC */ int mBlinkTimer;
    /* 0x1C00 */ int mOwnHome; // the current player's house, or -1
}; // size 0x1C04

#endif
