#ifndef GAME_D_MENU_DESI_HPP
#define GAME_D_MENU_DESI_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>

class dDesign_c;
namespace dMenuDesi {
// Names inferred from layout resources and behavior; no class RTTI survives.
class Desi_c {
  public:
    enum FLAG_e {
        FLAG_CAPTURE = 2,
        FLAG_PICKUP = 4,
        FLAG_PICKUP_MII = 8,
        FLAG_LEAVE_ANM = 16
    };
    bool load();                                                  // .text:0x5C
    void create(BOOL pickup);                                     // .text:0x68
    BOOL remove();                                                // .text:0x140
    void calc();                                                  // .text:0x194
    void draw();                                                  // .text:0x1E0
    m2d::SimpleEx_c *getLayout();                                 // .text:0x1E8
    void setPreviousSelection(int slot);                          // .text:0x1F0
    int getPreviousSelection();                                   // .text:0x284
    BOOL setSelection(int slot, BOOL visible);                    // .text:0x28C
    void setFaceVisible(int kind, bool visible);                  // .text:0x364
    void initPanes();                                             // .text:0x38C
    void updateDesigns();                                         // .text:0x484
    dDesign_c *getDesign(int slot);                               // .text:0x530
    void setDesign(int slot, dDesign_c *design);                  // .text:0x568
    int hitDesign(const mVec2_c &pos, BOOL setOffset, f32 scale); // .text:0x5B0
    int hitFace(const mVec2_c &pos, BOOL setOffset, f32 scale);   // .text:0x690
    mVec2_c getDesignPos(int slot, f32 x, f32 y);                 // .text:0x790
    mVec2_c getFacePos(int kind, f32 x, f32 y);                   // .text:0x85C
    void setDesignVisible(int slot, bool visible);                // .text:0x920
    bool isProDesign(int slot);                                   // .text:0x9CC
    bool setupMii();                                              // .text:0x9FC
    void setMiiVisible(bool visible);                             // .text:0xB60
    void drawPickup();                                            // .text:0xB74
    void calcPickup();                                            // .text:0xB90
    bool calcMove();                                              // .text:0xC20
    void startMove(const mVec2_c &pos, BOOL mii);                 // .text:0xC28
    void calcFollow();                                            // .text:0xC88
    void startFollow(BOOL mii);                                   // .text:0xC90
    void setPickupDesign(dDesign_c *design);                      // .text:0xCF0
    void setOffset(const mVec2_c &pos);                           // .text:0xD7C
    void setPickupPos(const mVec2_c &pos);                        // .text:0xD84
    void startLeaveAnm(int anm);                                  // .text:0xDBC
    BOOL calcLeaveAnm();                                          // .text:0xE10
    void startMiiPush();                                          // .text:0xE70
    void setMiiAnm(int anm);                                      // .text:0xE80

    /* 0x000 */ m2d::ResAccLoader_c mRes;
    /* 0x0D4 */ m2d::SimpleEx_c mLyt;
    /* 0x16C */ nw4r::lyt::Pane *mDesignPanes[8];
    /* 0x18C */ dMenuMove_c mMove;
    /* 0x1AC */ m2d::SimpleEx_c mPickupLyt;
    /* 0x244 */ m2d::SimpleEx_c mPickupMiiLyt;
    /* 0x2DC */ u32 mFlags;
    /* 0x2E0 */ int mSelection;
    /* 0x2E4 */ int mPreviousSelection;
    /* 0x2E8 */ m2d::captureNode_c mMiiCapture;
    /* 0x304 */ m2d::captureNode_c mPickupMiiCapture;
    /* 0x320 */ m2d::Anm_c mLeaveAnm;
    /* 0x344 */ m2d::Anm_c mScrollAnm;
    /* 0x368 */ m2d::Anm_c mMiiPushAnm;
    /* 0x38C */ m2d::Anm_c mMiiAnm;
}; // size 0x3B0
} // namespace dMenuDesi
#endif
