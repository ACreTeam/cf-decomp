#ifndef GAME_D_MENU_ITEM_HPP
#define GAME_D_MENU_ITEM_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_item.hpp>

namespace dMenuItem {

// Names inferred from layout resources, pocket operations and animation usage.
enum {
    PAPER_STACK_SIZE = 4,
    TURNIP_STACK_SIZE = 10,
    SMALL_MONEY_LIMIT = 1000,
    MONEY_CHANGE_FRAMES = 30,
    MONEY_STEP_MODULUS = 5,
    ICON_PATTERN_COUNT = 0x174
};
class Icon_c : public m2d::SimpleEx_c {
  public:
    enum FLAG_e { FLAG_GIFT = 1, FLAG_HIDDEN = 2, FLAG_ROTATE = 4 };
    enum BASE_ANM_e { BASE_ANM_NORMAL, BASE_ANM_SELECTED, BASE_ANM_COUNT };
    enum GIFT_ANM_e {
        GIFT_ANM_PRESENT,
        GIFT_ANM_PRESENT_ALT,
        GIFT_ANM_EGG,
        GIFT_ANM_COUNT,
        GIFT_ANM_NONE = 0xFF
    };
    void setSelected(int selected);                         // .text:0xF94
    void setNoChoice(bool visible);                         // .text:0x1070
    void setMark(bool visible);                             // .text:0x1084
    void create(m2d::ResAccIf_c *res, m2d::Anm_c *itemAnm); // .text:0x1098
    void setPos(const mVec2_c *pos);                        // .text:0x116C
    void calcLayouts();                                     // .text:0x1190
    void drawLayouts();                                     // .text:0x1200
    void calcRotate();                                      // .text:0x1254
    void setVisible(bool visible);                          // .text:0x12A4
    void setGiftAnm(int gift);                              // .text:0x12CC
    void setRootPos(const mVec3_c *pos);                    // .text:0x1374
    const mVec3_c &getRootPos();                            // .text:0x1404
    void setItem(dItem::Item item, int flag, int unused);   // .text:0x1440
    bool calcGift();                                        // .text:0x1720

    /* 0x098 */ m2d::Anm_c *mpItemAnm;
    /* 0x09C */ m2d::Anm_c mPresentAnm;
    /* 0x0C0 */ m2d::SimpleEx_c mGiftLyt;
    /* 0x158 */ m2d::Anm_c mGiftAnm;
    /* 0x17C */ m2d::Anm_c mBaseAnm;
    /* 0x1A0 */ m2d::Anm_c mRotateAnm;
    /* 0x1C4 */ m2d::SimpleEx_c mMarkLyt;
    /* 0x25C */ u16 mIconFlags;
    /* 0x25E */ u8 mGiftType;
}; // size 0x260

class Window_c {
  public:
    enum { POCKET_COUNT = 15 };
    enum FLAG_e { FLAG_CALC = 1, FLAG_ANM = 2 };
    enum ANM_e {
        ANM_CHEST_IN,
        ANM_CHEST_OUT,
        ANM_DESIGN_IN,
        ANM_DESIGN_OUT,
        ANM_COUNT
    };
    int load();                                                // .text:0x5C
    void create();                                             // .text:0xB0
    void setFaceButton(bool visible);                          // .text:0x1A8
    void setPlayerFace(bool visible);                          // .text:0x22C
    void getPocketName(dScript::Word_c *word, int pocket);     // .text:0x240
    void updateMoneyText();                                    // .text:0x374
    void reset();                                              // .text:0x3C4
    BOOL remove();                                             // .text:0x434
    void calcIcons();                                          // .text:0x484
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0x4F0
    void calcLayouts();                                        // .text:0x56C
    void draw();                                               // .text:0x600
    void setMoneyButton(bool visible);                         // .text:0x680
    void setMark(int pocket, bool visible);                    // .text:0x6AC
    void setNoChoice(int pocket, bool visible);                // .text:0x704
    void setSelected(int pocket, int selected);                // .text:0x71C
    u16 setPocket(int pocket, dItem::Item item, int flag);     // .text:0x734
    void updatePockets(int unused);                            // .text:0x7BC
    bool calcGift(int pocket);                                 // .text:0x840
    mVec3_c getPocketPos(int pocket);                          // .text:0x850
    m2d::SimpleEx_c *getLayout();                              // .text:0x8A0
    m2d::ResAccLoader_c *getIconRes();                         // .text:0x8A8
    m2d::Anm_c *getItemAnm();                                  // .text:0x8B0
    int getMoney();                                            // .text:0x8B8
    void resetMoney();                                         // .text:0x8DC
    void calcMoney();                                          // .text:0x91C
    int hitPocket(const mVec2_c *pos, bool setOffset,
                  float scale);                          // .text:0xA18
    bool calcAnm();                                      // .text:0xB10
    void startAnm(int anm);                              // .text:0xB78
    void stopAnm();                                      // .text:0xBCC
    void drawHeldItem();                                 // .text:0xBF0
    void calcHeldItem();                                 // .text:0xBF8
    void setHeldItem(const dItem::Item &item, int flag); // .text:0xC3C
    bool isMoveEnd();                                    // .text:0xC70
    bool calcMove();                                     // .text:0xC78
    void startMove(const mVec2_c &pos);                  // .text:0xC80
    void calcFollow();                                   // .text:0xC88
    void startFollow();                                  // .text:0xC90
    void setMoveOffset(const mVec2_c &ofs);              // .text:0xC98

    /* 0x0000 */ m2d::ResAccLoader_c mRes;
    /* 0x00D4 */ m2d::ResAccLoader_c mIconRes;
    /* 0x01A8 */ m2d::SimpleEx_c mLyt;
    /* 0x0240 */ Icon_c mIcons[POCKET_COUNT];
    /* 0x25E0 */ m2d::Anm_c mMoneyAnm;
    /* 0x2604 */ m2d::Anm_c mWinAnm;
    /* 0x2628 */ m2d::Anm_c mScrollAnm;
    /* 0x264C */ m2d::Anm_c mItemAnm;
    /* 0x2670 */ int mDisplayedMoney;
    /* 0x2674 */ int mMoneyTimer;
    /* 0x2678 */ int mMoneyStep;
    /* 0x267C */ Icon_c mHeldIcon;
    /* 0x28DC */ dMenuMove_c mMove;
    /* 0x28FC */ u32 mFlags;
}; // size 0x2900

bool combineItems(dItem::Item &dst, dItem::Item &src); // .text:0xCA0
bool needsSpecialHandling(const dItem::Item *item);    // .text:0xF4C

} // namespace dMenuItem

#endif
