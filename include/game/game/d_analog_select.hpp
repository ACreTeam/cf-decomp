#pragma once

#include <types.h>
#include <game/game/d_select.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_state.hpp>
#include <game/game/d_m2d.hpp>

namespace dAnalogSelect {
BOOL isConfirmTriggered(); // 8000C14C
BOOL isConfirmHeld();      // 8000C150
BOOL isUpPressed();        // 8000C154
BOOL isDownPressed();      // 8000C16C
f32 getStickY();           // 8000C184

class Comp_c : public dSelect::BaseComp_c, public dState::mode_c<Comp_c> {
public:
    bool isMode(Method method) const { return mCurrentMethod == method; }

    Comp_c(); // 8000C188
    virtual ~Comp_c(); // 8000C228
    BOOL load(); // 8000C2D0
    void build(); // 8000C2E0
    BOOL create(); // 8000C560
    BOOL remove(); // 8000C5BC
    BOOL update(); // 8000C658
    BOOL draw(); // 8000C6AC
    f32 getTextWidth(); // 8000C740
    void calcWindowSize(); // 8000C7B8
    void resizeWindow(); // 8000C824
    void moveCursorLine(); // 8000C8D8
    void moveCursor(); // 8000C92C
    void updateLayout(); // 8000C978
    void moveWindow(); // 8000C9BC
    void updateStick(); // 8000CA28
    void updateButtons(); // 8000CC04
    void updatePointer(); // 8000CC68
    int getSelection(); // 8000CEC4
    void confirm(); // 8000CEE8
    virtual void onOpen(); // 8000D054: prepare the layout
    virtual void onClose(); // 8000D090
    dSelect::Separate_c *getSeparate(int idx); // 8000D094
    virtual void reset(); // 8000D0A4
    void changeWindowAnm(int idx); // 8000D104
    void changeCursorAnm(int idx); // 8000D170
    void changeCursorPressAnm(int idx); // 8000D1DC
    void setResultLabel(const char *label); // 8000D2A8
    void setResultCode(u16 code); // 8000D2FC
    void setText(int idx, dScript::Word_c *text); // 8000D304
    void setItemName(int idx, dItem::Item item); // stripped item-name choice overload
    void setMessage(int idx, u16 code, const char *label); // 8000D340
    void setCode(int idx, u16 code, int label); // 8000D3A0
    virtual void open(); // 8000D3EC
    virtual bool isClosed(); // 8000D548
    void initHidden(); // 8000D594
    void executeHidden(); // 8000D598
    void initIn(); // 8000D59C
    void executeIn(); // 8000D6B4
    void initSelect(); // 8000D820
    void executeSelect(); // 8000D824
    void initOut(); // 8000D920
    void executeOut(); // 8000D994

    /* 0x06C */ m2d::ResAccLoader_c mResources;
    /* 0x140 */ m2d::SimpleEx_c mLayout;
    /* 0x1D8 */ m2d::Anm_c mWindowAnm;
    /* 0x1FC */ m2d::Anm_c mCursorPressAnm;
    /* 0x220 */ m2d::Anm_c mCursorAnm;
    /* 0x244 */ nw4r::lyt::Pane *mpWindow;
    /* 0x248 */ nw4r::lyt::Pane *mpChoiceWindow;
    /* 0x24C */ nw4r::lyt::Pane *mpChoiceShadow;
    /* 0x250 */ nw4r::lyt::Pane *mpCursor;
    /* 0x254 */ nw4r::lyt::Pane *mpCursorLine;
    /* 0x258 */ nw4r::lyt::Pane *mpMessage;
    /* 0x25C */ nw4r::lyt::Pane *mpCursorButton;
    /* 0x260 */ nw4r::lyt::Pane *mpConfirmButton;
    /* 0x264 */ f32 mOriginalTextWidth;
    /* 0x268 */ f32 mTextWidth;
    /* 0x26C */ f32 mLeftMargin;
    /* 0x270 */ f32 mRightMargin;
    /* 0x274 */ f32 mCursorPos;
    /* 0x278 */ f32 mStickSpeed;
    /* 0x27C */ f32 mPrevStick;
    /* 0x280 */ dSelect::Separate_c mChoices[8];
    /* 0xC80 */ int mWindowAnmId;
    /* 0xC84 */ int mCursorPressAnmId;
    /* 0xC88 */ int mCursorAnmId;
    /* 0xC8C */ int mPointerButton;
    /* 0xC90 */ mVec2_c mPointerPos;
    /* 0xC98 */ u8 mPointerMode;
    /* 0xC99 */ bool mCreated;
}; // size 0xC9C
} // namespace dAnalogSelect

typedef dAnalogSelect::Comp_c dMsgAnalogSelect_c;
