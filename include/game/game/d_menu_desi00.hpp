#ifndef GAME_D_MENU_DESI00_HPP
#define GAME_D_MENU_DESI00_HPP

#include <game/game/d_item.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_desi.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_select.hpp>
#include <game/snd/snd_id.hpp>

// The design menu (REL d_menu_desi00NP, profile MENU_DESI00; menus 0xD and
// 0xE): the player's eight custom designs (dMenuDesi::Desi_c), which can be
// dragged to swap or copy them, dragged onto the player's face to wear them, or
// picked to display, edit or place them. RTTI confirms the global class name;
// everything else is inferred.
class dMenuDesi00_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor, mHeld, mDesign).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_TAB = 0, // 0..7: dAcUitag tabs
        SLOT_TAB_LAST = 7,
        SLOT_DESIGN = 8, // 8..15: the eight designs
        SLOT_DESIGN_LAST = 15,
        SLOT_PLAYER = 16, // the player's face
        SLOT_MII = 17,    // the Mii's face
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_SELECTED,   // a slot is highlighted
        FLAG_DRAW,       // draw the design window
        FLAG_FOLLOW,     // the held design follows the cursor
        FLAG_PLACED,     // the held design is drawn at a fixed position
        FLAG_MOVE,       // the held design flies to its target
        FLAG_DIALOG,     // draw the dialog box
        FLAG_MII,        // the player has a Mii to show
        FLAG_WEAR_SHIRT, // the design of the slot is worn as a shirt
        FLAG_WEAR_HAT,   // ... as a hat
        FLAG_WEAR_HELD,  // ... as the held item
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,        // pick something with the cursor
        SUB_GRAB_DESIGN, // a design was pressed: wait for the drag
        SUB_DRAG_DESIGN,
        SUB_GRAB_MII,
        SUB_DRAG_MII,
        SUB_MOVE,       // the held design flies back
        SUB_SELECT,     // the first answer menu
        SUB_SUB_SELECT, // the second answer menu
        SUB_SUB_ANSWER,
        SUB_WAIT_ROOM,   // wait for the floor / wall to change
        SUB_WEAR,        // change the worn item
        SUB_WEAR_OUTFIT, // change the whole outfit
        SUB_WAIT_WEAR,
        SUB_PUT_ON_MII,
        SUB_TAKE_OFF_MII,
        SUB_WAIT_COPY,   // wait for the other consoles to answer the copy
        SUB_DIALOG,      // the dialog box opens
        SUB_DIALOG_PUSH, // a dialog button was pushed
        SUB_DIALOG_END,  // the dialog box answer
        SUB_WAIT_PUT,    // wait for the design to be put on the ground
        SUB_NUM
    };

    // The answers of the answer menus (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_QUIT = 0,
        ANSWER_CANCEL = 1,
        ANSWER_BACK = 2,
        ANSWER_EDIT = 3,
        ANSWER_WEAR = 4,
        ANSWER_PLACE_IN_ROOM = 5,
        ANSWER_INSTALL = 6,
        ANSWER_DISPLAY = 7,
        ANSWER_FLOOR = 8,
        ANSWER_ROTATE_FLOOR = 9,
        ANSWER_WALL = 10,
        ANSWER_ROTATE_WALL = 11,
        ANSWER_HAT = 12,
        ANSWER_CLOTHES = 13,
        ANSWER_UMBRELLA = 14,
        ANSWER_REMOVE_EQUIPMENT = 15,
        ANSWER_REMOVE_HEADGEAR = 16,
        ANSWER_REMOVE_ACCESSORY = 17,
        ANSWER_EASEL = 18,
        ANSWER_ROOM_HAT = 19,
        ANSWER_ROOM_CLOTHES = 20,
        ANSWER_ROOM_UMBRELLA = 21,
        ANSWER_REMOVE_MII = 22,
        ANSWER_SWAP = 23,
        ANSWER_COPY = 24,
    };

    // What the player wears a design as (mWearKind).
    enum WEAR_e {
        WEAR_HELD = 0,
        WEAR_SHIRT = 1,
        WEAR_HAT = 2,
        WEAR_ACCESSORY = 4,
    };

    // The furniture a design is put in the room as.
    enum ROOM_e {
        ROOM_UMBRELLA,
        ROOM_CLOTHES,
        ROOM_HAT,
        ROOM_EASEL,
    };

    // What the dialog box asks (mDialogKind).
    enum DIALOG_e {
        DIALOG_EDIT,
        DIALOG_OVERWRITE,
    };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_QUIT = 0x02,
        MSG_WEAR = 0x0B,
        MSG_HAT = 0x0C,
        MSG_CLOTHES = 0x0D,
        MSG_UMBRELLA = 0x0E,
        MSG_NEVER_MIND = 0x0F,
        MSG_DISPLAY_ON_GROUND = 0x10,
        MSG_EDIT_DESIGN = 0x11,
        MSG_PLACE_IN_ROOM = 0x13,
        MSG_EASEL = 0x14,
        MSG_ROOM_HAT = 0x15,
        MSG_ROOM_CLOTHES = 0x16,
        MSG_ROOM_UMBRELLA = 0x17,
        MSG_ROOM_NEVER_MIND = 0x18,
        MSG_INSTALL_IN_ROOM = 0x19,
        MSG_PLACE_ON_FLOOR = 0x1A,
        MSG_ROTATE_AND_PLACE = 0x1B,
        MSG_HANG_ON_WALL = 0x1C,
        MSG_ROTATE_AND_HANG = 0x1D,
        MSG_INSTALL_NEVER_MIND = 0x1E,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_REMOVE_EQUIPMENT = 0x25,
        MSG_REMOVE_HEADGEAR = 0x26,
        MSG_REMOVE_ACCESSORY = 0x27,
        MSG_REMOVE_MII = 0x86,
        MSG_POCKETS_FULL_DESIGNS = 0x89,
        MSG_POCKETS_FULL_ACCESSORIES = 0x8A,
        MSG_NO_PLACE_FOR_DESIGN = 0x8C,
        MSG_OVERWRITE = 0xD3,
        MSG_COPY = 0xD9,
        MSG_SWAP = 0xDA,
        MSG_OVERWRITE_PRO = 0xDB,
        MSG_OK = 0xEC,
        MSG_COMPANY_FLOOR = 0x12E,
        MSG_COMPANY_WALL = 0x12F,
        MSG_COMPANY_ROOM = 0x130,
        MSG_POCKETS_FULL_REMOVE = 0x139,
        MSG_NOTHING_TO_REMOVE = 0x146,
        MSG_GATE_OPEN = 0x147,
        MSG_HOSTING = 0x148,
    };

    // Messages of SYS_2D_UIballoon.bmg.
    enum {
        MSG_WEAR_MII_MASK = 0x6A,
        MSG_REMOVE_MII_MASK = 0x6B,
    };

    static bool isTab(int slot) {
        return slot >= SLOT_TAB && slot <= SLOT_TAB_LAST;
    }
    static bool isDesign(int slot) {
        return slot >= SLOT_DESIGN && slot <= SLOT_DESIGN_LAST;
    }

    virtual int create();    // .text:0x2F0
    virtual int doDelete();  // .text:0x468
    virtual int stateMain(); // .text:0x4EC
    virtual int execute();   // .text:0x534
    virtual int draw();      // .text:0x5D4

    void subMain();                                    // .text:0x688
    void subGrabDesign();                              // .text:0x8BC
    void subDragDesign();                              // .text:0x97C
    void subGrabMii();                                 // .text:0xB98
    void subDragMii();                                 // .text:0xC50
    void subMove();                                    // .text:0xD7C
    void subSelect();                                  // .text:0xDD0
    void subSubSelect();                               // .text:0xF74
    void subSubAnswer();                               // .text:0x1004
    void subWaitRoom();                                // .text:0x113C
    void subWear();                                    // .text:0x11A0
    void subWearOutfit();                              // .text:0x1260
    void subWaitWear();                                // .text:0x1334
    void subPutOnMii();                                // .text:0x13A8
    void subTakeOffMii();                              // .text:0x13F4
    void subWaitCopy();                                // .text:0x1440
    bool closeMain();                                  // .text:0x149C
    bool closeDrag();                                  // .text:0x14D4
    bool closeSelect();                                // .text:0x1528
    bool closeDialog();                                // .text:0x15A8
    bool executeClose();                               // .text:0x1604
    void executeSubState();                            // .text:0x1780
    void startCloseAnm();                              // .text:0x17B8
    bool selectTab(int tab, BOOL delay);               // .text:0x1810
    mVec2_c getSlotPos(int slot, f32 x, f32 y);        // .text:0x1860
    void setHighlight(int slot, BOOL on);              // .text:0x18FC
    void getSlotName(dScript::Word_c *word, int slot); // .text:0x19B8
    void showSlotTutorial(int slot);                   // .text:0x1A40
    void hideTutorial();                               // .text:0x1B98
    dDesign_c *getDesign(int index);                   // .text:0x1B9C
    int getOrder(int index);                           // .text:0x1BD4
    void swapDesigns(int slotA, int slotB);            // .text:0x1C10
    void copyDesign(int from, int to);                 // .text:0x1C74
    void openSelect(int slot, int target);             // .text:0x1D1C
    void openSubSelect();                              // .text:0x1EE8
    void endSub();                                     // .text:0x20B4
    void addWearAnswers();                             // .text:0x20C8
    void addPlaceAnswers();                            // .text:0x2180
    void addInstallAnswers();                          // .text:0x2220
    void editDesign();                                 // .text:0x22C0
    void displayDesign();                              // .text:0x2358
    void wearDesign(int kind);                         // .text:0x23EC
    void putInRoom(int kind);                          // .text:0x2544
    void putOnMii();                                   // .text:0x264C
    void removeItem(int kind);                         // .text:0x278C
    void takeOffMii();                                 // .text:0x2898
    void copyHeld();                                   // .text:0x28AC
    void swapHeld();                                   // .text:0x28B0
    void buildSelect();                                // .text:0x28FC
    void buildWearSelect();                            // .text:0x2A3C
    void buildRemoveSelect();                          // .text:0x2B18
    void buildSwapSelect();                            // .text:0x2C48
    bool buildSubSelect();                             // .text:0x2CDC
    void calcPickup();                                 // .text:0x2D94
    void holdCursor();                                 // .text:0x2E04
    void pickUpDesign(int slot);                       // .text:0x2E40
    void pickUpMii();                                  // .text:0x2ECC
    void dropHeld(int slot);                           // .text:0x2F38
    void installDesign();                              // .text:0x2FD8
    bool hasMiiMask();                                 // .text:0x31B0
    bool isDesignItem(const dItem::Item *item);        // .text:0x31E4
    int getTab(const mVec2_c *pos);                    // .text:0x3224
    int hitCheck(BOOL tabs);                           // .text:0x3288
    void subDialog();                                  // .text:0x3368
    void subDialogPush();                              // .text:0x33C8
    void subDialogEnd();                               // .text:0x342C
    void subWaitPut();                                 // .text:0x3594
    void showMessage(u16 msg, BOOL cannotLeave);       // .text:0x362C
    void askOverwritePro();                            // .text:0x3700
    void askOverwrite();                               // .text:0x37B8
    void selectDesign();                               // .text:0x3870
    void updateWorn(int index);                        // .text:0x38D8
    bool isTraveling();                                // .text:0x3A60

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

    /* 0x0D4 */ dMenuDesi::Desi_c mDesi;
    /* 0x484 */ dSelect::MComp_c mSelect;
    /* 0xE30 */ dSelect::MComp_c mSubSelect;
    /* 0x17DC */ int mDesign; // the design slot the answer menus are for
    /* 0x17E0 */ int mAnswer;
    /* 0x17E4 */ int mHeld; // the slot being dragged
    /* 0x17E8 */ dMenuSlctB_c mDialog;
    /* 0x1ED8 */ int mCursor; // SLOT_e under the cursor
    /* 0x1EDC */ int _1EDC;
    /* 0x1EE0 */ int mPutRequest;
    /* 0x1EE4 */ int mHoldTimer;
    /* 0x1EE8 */ dItem::Item mItem;
    /* 0x1EEA */ u8 mWearKind;
    /* 0x1EEB */ u8 mDialogKind;
}; // size 0x1EEC

#endif
