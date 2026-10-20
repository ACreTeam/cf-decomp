#ifndef GAME_D_MENU_ITEMXX_HPP
#define GAME_D_MENU_ITEMXX_HPP

#include <game/game/d_item.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_item.hpp>
#include <game/game/d_menu_ltr.hpp>
#include <game/game/d_select.hpp>

namespace dScript {
class Word_c;
}

// The shared base of the item menus (REL d_menu_itemxxNP; d_menu_item00NP..
// derive from it): the pocket window, the letter window, two answer menus and
// moving items/letters between pockets, letters, the wallet and the player.
// RTTI confirms the global class name; everything else is inferred. The
// constructor is not in this REL (the derived menus build it).
class dMenuItemXX_c : public dMenuBase_c {
  public:
    // Slots: what the cursor points at (mCursor, mHeldFrom, ...).
    enum SLOT_e {
        SLOT_NONE = -1,
        SLOT_POCKET = 0, // 0..14: dMenuItem::Window_c pockets
        SLOT_POCKET_LAST = 14,
        SLOT_LETTER = 15, // 15..24: dMenuLtr::Window_c letters
        SLOT_LETTER_LAST = 24,
        SLOT_TAB = 25, // 25..32: dAcUitag tabs
        SLOT_TAB_LAST = 32,
        SLOT_MONEY = 33, // the wallet button ("N_btnL_00_all")
        SLOT_FACE = 34,  // the face button ("N_btnL_01_all")
        SLOT_LETTER_BUTTON = 35,
        SLOT_PLAYER = 36, // the player's face ("BC_player")
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_CALC_ITEM,
        FLAG_DRAW_ITEM,
        FLAG_CALC_LETTER,
        FLAG_DRAW_LETTER_BUTTON,
        FLAG_DRAW_LETTER_WINDOW,
        FLAG_LETTER_ANM,
        FLAG_HOLD_ITEM,
        FLAG_RETURN_ITEM,
        FLAG_HOLD_LETTER,
        FLAG_RETURN_LETTER,
    };

    // Results of dropping the held item / letter.
    enum DROP_e {
        DROP_NONE,
        DROP_FACE,
        DROP_EAT,
        DROP_MEDICINE,
        DROP_HAS_PRESENT,
        DROP_INSECT,
        DROP_FISH,
        DROP_TURNIP,
        DROP_WALLET_FULL,
        DROP_DONE,
    };

    static bool isPocket(int slot) {
        return slot >= SLOT_POCKET && slot <= SLOT_POCKET_LAST;
    }
    static bool isLetter(int slot) {
        return slot >= SLOT_LETTER && slot <= SLOT_LETTER_LAST;
    }
    static bool isTab(int slot) {
        return slot >= SLOT_TAB && slot <= SLOT_TAB_LAST;
    }

    static dItem::Item *getPockets();                        // .text:0x5C
    virtual int preCreate();                                 // .text:0x80
    virtual void postCreate(MAIN_STATE_e state);             // .text:0xCC
    virtual void postDelete(MAIN_STATE_e state);             // .text:0xE0
    virtual int stateAnm();                                  // .text:0xE4
    virtual int state3();                                    // .text:0x188
    virtual int execute();                                   // .text:0x230
    virtual int draw();                                      // .text:0x2F4
    bool load();                                             // .text:0x3F0
    int createLayouts();                                     // .text:0x474
    bool remove();                                           // .text:0x4C8
    void setMark(int slot, bool on);                         // .text:0x54C
    void markSlot(int slot);                                 // .text:0x638
    void unmarkSlot(int slot);                               // .text:0x640
    int hitPocket(mVec2_c pos, float scale);          // .text:0x648
    int hitButton(mVec2_c pos, float scale);          // .text:0x6E8
    int hitLetter(mVec2_c pos, float scale);          // .text:0x80C
    int hitTab(const mVec2_c *pos);                          // .text:0x8B0
    bool isPocketEmpty(int slot);                            // .text:0x914
    BOOL isLetterEmpty(int slot);                            // .text:0x988
    u16 getPocketItem(int slot);                             // .text:0x9F0
    int getPocketFlag(int slot);                             // .text:0xA24
    dMail_c *getLetter(int slot);                            // .text:0xA58
    void setLetter(int slot, const dMail_c *mail);           // .text:0xA94
    void clearLetter(int slot);                              // .text:0xAF8
    void takePresent(int letter, int pocket);                // .text:0xB58
    u16 setSlotItem(int slot, dItem::Item item, int flag);   // .text:0xD08
    u16 setPocket(int slot, dItem::Item item, int flag);     // .text:0xD60
    u16 dropOnPocket(int slot, dItem::Item item);            // .text:0xD90
    bool addToPockets(dItem::Item item, int flag);           // .text:0xE68
    void pickUp();                                           // .text:0xEE4
    int dropItem(BOOL putBack);                              // .text:0xFFC
    int dropLetter();                                        // .text:0x11E4
    int dropToPocket();                                      // .text:0x13E4
    int putInLetter();                                       // .text:0x14A4
    bool calcReturn();                                       // .text:0x16A0
    bool isReturning();                                      // .text:0x1768
    void finishReturn();                                     // .text:0x17E0
    void startReturn(int slot);                              // .text:0x188C
    void closeWindow();                                      // .text:0x191C
    void closeWindowSe();                                    // .text:0x1978
    void windowUp(bool keepTab);                             // .text:0x19D0
    void windowIn(bool keepTab);                             // .text:0x1A40
    mVec2_c getPocketPos(int slot, float dx, float dy);      // .text:0x1AA8
    mVec2_c getSlotPos(int slot, float dx, float dy);        // .text:0x1B1C
    void getSlotName(dScript::Word_c *word, int slot);       // .text:0x1C68
    void showSlotTutorial(int slot, int faceMsg, BOOL tail); // .text:0x1E2C
    void hideTutorial();                                     // .text:0x1F80
    void openSelect(int slot, BOOL pointer);                 // .text:0x1F84
    void openSubSelect();                                    // .text:0x22A0
    void openSelectBelow(BOOL pointer);                      // .text:0x25F4
    int getMoney();                                          // .text:0x28F8
    void setMoney(int money);                                // .text:0x291C
    bool addMoney(int money);                                // .text:0x294C
    BOOL depositMoney(int money, int slot);                  // .text:0x29B0
    void calcMoney();                                        // .text:0x2A98
    void updateHeldItem();                                   // .text:0x2AA0
    void calcLayouts();                                      // .text:0x2AB0
    void clearHeld();                                        // .text:0x2BC4
    bool calcGift(int slot);                                 // .text:0x2BDC
    bool canDropPocket(int slot);                            // .text:0x2BE4
    bool canDrop(const dItem::Item &item, int flag);                // .text:0x2C3C
    bool isEdible(const dItem::Item *item);                  // .text:0x2D58
    BOOL isMedicine(const dItem::Item *item);                // .text:0x2DB0
    BOOL findWater(mVec3_c *pos);                            // .text:0x2DE0
    bool canReleaseInsect(const dItem::Item *item);          // .text:0x2E60

    // Implicit destructor: .text:0x2EBC.

    /* 0x00D4 */ dMenuLtr::Window_c mLtrWin;
    /* 0x13B4 */ dMenuItem::Window_c mItemWin;
    /* 0x3CB4 */ dSelect::MComp_c mSelect;
    /* 0x4660 */ dSelect::MComp_c mSubSelect;
    /* 0x500C */ int mCursor; // SLOT_e under the cursor
    /* 0x5010 */ u8 _5010[4];
    /* 0x5014 */ int mHeldFrom; // the slot the held item / letter came from
    /* 0x5018 */ int mReturnTo; // where a released item / letter flies back to
    /* 0x501C */ dItem::Item mHeldItem;
    /* 0x5020 */ int mHeldFlag;   // the held item's pocket flag
    /* 0x5024 */ int mSelectSlot; // the slot the answer menu was opened for
    /* 0x5028 */ u8 _5028[4]; // not used by this REL; the derived menus start at 0x502C
};

namespace dMenuItemXX {
bool isPlayerPane(nw4r::lyt::Pane *pane);                  // .text:0x3220
bool selectTab(dMenuItemXX_c *menu, int slot, BOOL delay); // .text:0x3254
void showButtonTutorial(dMenuItemXX_c *menu, int slot,
                        int faceMsg); // .text:0x32EC
} // namespace dMenuItemXX

#endif
