#include <game/game/d_analog_select.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_pad.hpp>
#include <math.h>
#include <string.h>

namespace dAnalogSelect {
static const char *sTextNames[] = {"T_msg_00", "T_msg_01"};
static const dState::mode_c<Comp_c>::Entry sModes[] = {
    {"\x94\xF1\x95\x5C\x8E\xA6", &Comp_c::initHidden, &Comp_c::executeHidden},
    {"\x8F\x6F\x8C\xBB", &Comp_c::initIn, &Comp_c::executeIn},
    {"\x95\x5C\x8E\xA6", &Comp_c::initSelect, &Comp_c::executeSelect},
    {"\x91\xDE\x8F\xEA", &Comp_c::initOut, &Comp_c::executeOut},
};

// 8000C14C
BOOL isConfirmTriggered() { return fn_800FA748(); }
// 8000C150
BOOL isConfirmHeld() { return fn_800FA754(); }
// 8000C154
BOOL isUpPressed() { return mPad::getCore()->down(8); }
// 8000C16C
BOOL isDownPressed() { return mPad::getCore()->down(4); }
// 8000C184
f32 getStickY() { return fn_800FA704(); }
// 8000C188
Comp_c::Comp_c() {}
// 8000C228
Comp_c::~Comp_c() {}
// 8000C2D0
BOOL Comp_c::load() { return mResources.load("Layout/anlg_chc_win.arc"); }
// 8000C2E0
void Comp_c::build() {
    mLayout.mpResAcc = &mResources;
    mLayout.build("anlg_chc_win_00.brlyt", NULL);
    mLayout.mPriority = 0x85;
    mLayout.createAnm(&mWindowAnm, "anlg_chc_win_00_anlgChcWinOut.brlan", NULL);
    mLayout.createAnm(&mCursorPressAnm, "anlg_chc_win_00_anlgCsrUp.brlan", NULL);
    mLayout.createAnm(&mCursorAnm, "anlg_chc_win_00_anlgCsrOut.brlan", NULL);
    mWindowAnmId = 1;
    mCursorPressAnmId = 1;
    mCursorAnmId = 1;
    mWindowAnm.mFrameCtrl.mFlags = 1;
    mCursorPressAnm.mFrameCtrl.mFlags = 1;
    mCursorAnm.mFrameCtrl.mFlags = 1;
    mLayout.bind(&mWindowAnm);
    mLayout.bind(&mCursorPressAnm);
    mLayout.bind(&mCursorAnm);
    mCursorPressAnm.setFrame(mCursorPressAnm.mFrameCtrl.mEndFrame);
    mCursorAnm.setFrame(mCursorAnm.mFrameCtrl.mEndFrame);
    mLayout.calc();
    mpWindow = mLayout.findPane("N_anlgChcWin_all");
    mpChoiceWindow = mLayout.findPane("N_chcWin");
    mpChoiceShadow = mLayout.findPane("N_ChcWinSdw");
    mpCursor = mLayout.findPane("N_csr_all");
    mpCursorLine = mLayout.findPane("N_lineV_all");
    mpMessage = mLayout.findPane("N_msg");
    mpCursorButton = mLayout.findPane("B_csr_00");
    mpConfirmButton = mLayout.findPane("B_btn_00");
    mLayout.setFont(&dFont::g_mainFont.mFont);
    for (int i = 0; i < 2; ++i) {
        nw4r::lyt::TextBox *text = mLayout.findTextBox(sTextNames[i]);
        text->SetTagProcessor(&g_tagProcessor);
        text->AllocStringBuffer(101);
        mChoices[i].mpTextBox = text;
    }
    mOriginalTextWidth = mChoices[0].mpTextBox->GetSize().width;
    mCursorPos = 80.0f;
    mPointerButton = 0;
    mLayout.setMessage("T_btn_00", 0x22, "sys_2D/SYS_2D_menu");
    initialize("dAnalogSelect::Comp_c", sModes, 4, false);
}
// 8000C560
BOOL Comp_c::create() {
    if (!load()) return FALSE;
    if (!mCreated) { build(); mCreated = true; }
    return TRUE;
}
// 8000C5BC
BOOL Comp_c::remove() {
    if (!mResources.remove()) return FALSE;
    if (mCreated == true) {
        for (int i = 0; i < 2; ++i) {
            if (mChoices[i].mpTextBox) mChoices[i].mpTextBox->FreeStringBuffer();
        }
        mCreated = false;
    }
    return TRUE;
}
// 8000C658
BOOL Comp_c::update() { execute(); return TRUE; }
// 8000C6AC
BOOL Comp_c::draw() {
    if (isMode(&Comp_c::executeHidden) == false) {
        mWindowAnm.play();
        mCursorPressAnm.play();
        mCursorAnm.play();
        mLayout.calc();
        mLayout.entry();
    }
    return TRUE;
}
// 8000C740
f32 Comp_c::getTextWidth() {
    f32 width = 53.0f;
    for (int i = 0; i < 2; ++i) {
        f32 w = m2d::calcTextWidth(mChoices[i].mpTextBox, NULL, 0);
        if (width < w) width = w;
    }
    return width;
}
// 8000C7B8
void Comp_c::calcWindowSize() {
    mTextWidth = getTextWidth();
    f32 rate = (mTextWidth - 53.0f) / 237.0f;
    mLeftMargin = 62.0f + rate * 8.0f;
    mRightMargin = 30.0f + rate * 7.0f;
}
// 8000C824
void Comp_c::resizeWindow() {
    f32 scale = (mRightMargin + (mTextWidth + mLeftMargin)) / 360.0f;
    mVec2_c scaleVec(scale, 1.0f);
    mpChoiceWindow->SetScale(scaleVec);
    mpChoiceShadow->SetScale(scaleVec);
    mVec3_c pos = mpChoiceWindow->GetTranslate();
    f32 half = 0.5f;
    pos.x += mLeftMargin * half - mRightMargin * half;
    pos.x += mOriginalTextWidth * half - mTextWidth * half;
    mpMessage->SetTranslate(pos);
}
// 8000C8D8
void Comp_c::moveCursorLine() {
    mVec3_c pos = mpChoiceWindow->GetTranslate();
    pos.x -= mTextWidth / 2;
    pos.x += -21.0f;
    mpCursorLine->SetTranslate(pos);
}
// 8000C92C
void Comp_c::moveCursor() {
    mVec3_c pos = mpCursorLine->GetTranslate();
    pos.y += 80.0f - mCursorPos;
    mpCursor->SetTranslate(pos);
}
// 8000C978
void Comp_c::updateLayout() { calcWindowSize(); resizeWindow(); moveCursorLine(); moveWindow(); }
// 8000C9BC
void Comp_c::moveWindow() {
    f32 height = 292.0f;
    f32 width = mRightMargin + (mTextWidth + mLeftMargin);
    mVec3_c pos;
    pos.z = 0.0f;
    pos.x = 261.0f;
    pos.y = -135.0f;
    pos.x -= width / 2;
    pos.x += mRightMargin;
    pos.y = pos.y + height / 2 - 44.0f;
    mpWindow->SetTranslate(pos);
}
// 8000CA28
void Comp_c::updateStick() {
    f32 previous = mPrevStick;
    f64 input = mPrevStick = getStickY();
    f32 stick = input;
    BOOL up = isUpPressed();
    BOOL down = isDownPressed();
    if (up || down || (stick != 0.0f && ((stick > 0.0f && previous > 0.0f) ||
                                       (stick < 0.0f && previous < 0.0f)))) {
        f32 rate = 0.03f;
        if (stick != 0.0f) rate = (f32)__fabs(stick) * 0.03f;
        mStickSpeed += rate;
        mStickSpeed = mStickSpeed < 0.0f ? 0.0f : mStickSpeed > 1.0f ? 1.0f : mStickSpeed;
        if (stick > 0.0f || up) mCursorPos -= (int)(mStickSpeed * 16.0f);
        if (stick < 0.0f || down) mCursorPos += (int)(mStickSpeed * 16.0f);
    } else {
        mStickSpeed = 0.1f;
    }
    mCursorPos = mCursorPos < 0.0f ? 0.0f : mCursorPos > 159.0f ? 159.0f : mCursorPos;
}
// 8000CC04
void Comp_c::updateButtons() {
    changeCursorAnm(2); changeWindowAnm(3); changeCursorPressAnm(0); updateStick();
    if (isConfirmTriggered()) confirm();
}
// 8000CC68
void Comp_c::updatePointer() {
    struct Hit { mVec2_c pos; f32 scale; nw4r::lyt::Pane *pane; } hit;
    hit.pane = NULL; hit.scale = 1.0f;
    hit.pos = dObjCursor_c::getPos();
    f32 delta = hit.pos.y - mPointerPos.y;
    mPointerPos = hit.pos;
    if (mPointerMode != fn_8017FE28()) delta = 0.0f;
    mLayout.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    if (mPointerButton == 0) {
        if (hit.pane == mpCursorButton) { mPointerButton = 1; changeCursorAnm(0); }
        else if (hit.pane == mpConfirmButton) { mPointerButton = 2; changeWindowAnm(2); }
        else {
            changeWindowAnm(3); changeCursorPressAnm(1);
            if (mCursorAnmId != 1) changeCursorAnm(3);
        }
    } else if (mPointerButton == 1) {
        if (isConfirmHeld()) {
            changeCursorPressAnm(0);
            f32 next = mCursorPos - delta;
            mCursorPos = next < 0.0f ? 0.0f : next > 159.0f ? 159.0f : next;
        } else {
            changeCursorPressAnm(1);
            if (hit.pane == mpConfirmButton) {
                mPointerButton = 2; changeCursorAnm(1); changeWindowAnm(2);
            } else if (hit.pane != mpCursorButton) { mPointerButton = 0; changeCursorAnm(1); }
        }
    } else if (mPointerButton == 2) {
        if (hit.pane == mpConfirmButton) { if (isConfirmTriggered()) confirm(); }
        else if (hit.pane == mpCursorButton) {
            mPointerButton = 1; changeCursorAnm(0); changeWindowAnm(3);
        } else { mPointerButton = 0; changeWindowAnm(3); }
    }
}
// 8000CEC4
int Comp_c::getSelection() { return (int)(mCursorPos / 20.0f); }
// 8000CEE8
void Comp_c::confirm() { mCursor = getSelection(); playSe(0x16D7); changeMode(&Comp_c::executeOut); }
// 8000D054
void Comp_c::onOpen() { dSelect::BaseComp_c::onOpen(); updateLayout(); moveCursor(); }
// 8000D090
void Comp_c::onClose() { dSelect::BaseComp_c::onClose(); }
// 8000D094
dSelect::Separate_c *Comp_c::getSeparate(int idx) { return &mChoices[idx]; }
// 8000D0A4
void Comp_c::reset() { for (int i = 0; i < 8; ++i) mChoices[i].clear(); mCursorPos = 80.0f; }
// 8000D104
void Comp_c::changeWindowAnm(int idx) {
    static const char *names[] = {"anlg_chc_win_00_anlgChcWinIn.brlan", "anlg_chc_win_00_anlgChcWinOut.brlan",
        "anlg_chc_win_00_btn_00_in.brlan", "anlg_chc_win_00_btn_00_out.brlan", "anlg_chc_win_00_btn_00_psh.brlan", NULL};
    if (mWindowAnmId != idx) {
        mWindowAnmId = idx;
        mLayout.changeAnm(&mWindowAnm, names[idx], NULL);
        mWindowAnm.mFrameCtrl.mFlags = 1; mWindowAnm.setFrame(0.0f);
    }
}
// 8000D170
void Comp_c::changeCursorAnm(int idx) {
    static const char *names[] = {"anlg_chc_win_00_anlgCsrIn.brlan", "anlg_chc_win_00_anlgCsrOut.brlan",
        "anlg_chc_win_00_CrsrBlue.brlan", "anlg_chc_win_00_CrsrYellow.brlan", NULL};
    if (mCursorAnmId != idx) {
        mCursorAnmId = idx;
        mLayout.changeAnm(&mCursorAnm, names[idx], NULL);
        mCursorAnm.mFrameCtrl.mFlags = 1; mCursorAnm.setFrame(0.0f);
    }
}
// 8000D1DC
void Comp_c::changeCursorPressAnm(int idx) {
    static const char *names[] = {"anlg_chc_win_00_anlgCsrDwn.brlan", "anlg_chc_win_00_anlgCsrUp.brlan", NULL, NULL, NULL};
    if (mCursorPressAnmId != idx) {
        mCursorPressAnmId = idx;
        mLayout.changeAnm(&mCursorPressAnm, names[idx], NULL);
        mCursorPressAnm.mFrameCtrl.mFlags = 1;
        if (isMode(&Comp_c::executeSelect) != false) {
            if (idx == 0) playSe(0x16E1);
            mCursorPressAnm.setFrame(0.0f);
        } else mCursorPressAnm.setFrame(mCursorPressAnm.mFrameCtrl.mEndFrame);
    }
}
// 8000D2A8
void Comp_c::setResultLabel(const char *label) { if (strlen(label)) strncpy(mLabel, label, 60); }
// 8000D2FC
void Comp_c::setResultCode(u16 code) { mCode = code; }
// 8000D304
void Comp_c::setText(int idx, dScript::Word_c *text) { getSeparate(idx)->mWord.copy(text, 0); }
// 8000D340
void Comp_c::setMessage(int idx, u16 code, const char *label) { dString::Word_c word(code, label); setText(idx, &word); }
// 8000D3A0
void Comp_c::setCode(int idx, u16 code, int label) {
    dSelect::Separate_c *choice = getSeparate(idx);
    choice->mMsgCode = code;
    if (label) choice->setLabel(reinterpret_cast<const char *>(label));
}
// 8000D3EC
void Comp_c::open() { changeMode(&Comp_c::executeIn); }
// 8000D548
bool Comp_c::isClosed() { return isMode(&Comp_c::executeHidden); }
// 8000D594
void Comp_c::initHidden() {}
// 8000D598
void Comp_c::executeHidden() {}
// 8000D59C
void Comp_c::initIn() {
    changeWindowAnm(0);
    if (fn_8017FE28() != 0) { changeCursorAnm(3); changeCursorPressAnm(1); mPointerButton = 0; }
    else { changeCursorAnm(2); changeCursorPressAnm(0); mPointerButton = 0; }
    mPointerMode = fn_8017FE28();
    for (int i = 0; i < 2; ++i) {
        mChoices[i].applyText();
        mChoices[i].mpTextBox->SetVisible(true);
    }
    onOpen();
}
// 8000D6B4
void Comp_c::executeIn() { if (mWindowAnm.mFrameCtrl.isStop()) changeMode(&Comp_c::executeSelect); }
// 8000D820
void Comp_c::initSelect() {}
// 8000D824
void Comp_c::executeSelect() {
    int oldButton = mPointerButton;
    f32 oldPos = mCursorPos;
    if (fn_8017FE28() != 0) updatePointer(); else updateButtons();
    if (oldButton != mPointerButton && mPointerButton != 0) playSe(0x16BE);
    if (mCursorPos != oldPos) {
        int oldSelection = (int)(oldPos / 10.0f);
        int newSelection = (int)(mCursorPos / 10.0f);
        if (oldSelection != newSelection) playSe(0x16CE);
        moveCursor();
    }
    mPointerMode = fn_8017FE28();
    if (mPad::getCore()->downTrigger(0x10)) confirm();
}
// 8000D920
void Comp_c::initOut() {
    changeWindowAnm(4);
    dSelect::Separate_c *choice = getSeparate(mCursor);
    setResultCode(choice->mMsgCode); setResultLabel(choice->mLabel); onClose();
}
// 8000D994
void Comp_c::executeOut() {
    if (mWindowAnm.mFrameCtrl.isStop() && mWindowAnmId == 4) changeWindowAnm(1);
    if (mWindowAnm.mFrameCtrl.isStop() && mWindowAnmId == 1) changeMode(&Comp_c::executeHidden);
}
void Comp_c::setItemName(int idx, dItem::Item item) {
    dItem::name_c name(item);
    setText(idx, &name);
}
} // namespace dAnalogSelect
