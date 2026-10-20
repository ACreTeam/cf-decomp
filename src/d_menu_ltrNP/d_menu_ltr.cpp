#include <game/game/d_menu_ltr.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_player_mgr.hpp>

namespace dMenuLtr {

// The argument of m2d::isInPane.
struct HitInfo {
    mVec2_c pos;
    float scale;
    nw4r::lyt::Pane *hit;
};

// .text:0x5C
void Window_c::calcLayouts() {
    mWinLyt.calc();
    mBtnLyt.calc();
    mWinLyt.patrolPane(positionIcon, nullptr, mIcons);
    nw4r::lyt::Pane *pane = mWinLyt.findPane("N_ltrWin_all");
    for (int i = 0; i < LETTER_COUNT; i++) {
        mIcons[i].setScale(pane->GetScale());
        mIcons[i].setAlpha(pane->GetAlpha());
        mIcons[i].calcLayout();
    }
}

// .text:0x130
void Window_c::create() {
    mWinLyt.build(&mRes, "ltr_win_00.brlyt");
    mWinLyt.setAnm(&mWinAnm, "ltr_win_00_ltrWinOpn.brlan", nullptr);
    mBtnLyt.build(&mRes, "ltr_btn_00.brlyt");
    mBtnLyt.setAnm(&mBtnAnm, "ltr_btn_00_icn_ltr_bck.brlan", nullptr);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < LETTER_COUNT; i++) {
        mIcons[i].create(&mRes);
        mIcons[i].setMail(&player->mLetters[i], -1);
    }
    mHeldIcon.create(&mRes);
    mHeldMail.clear();
    mHeldIcon.setMail(&mHeldMail, -1);
    setFaceButton(false);
    mMove.stop();
}

// .text:0x238
bool Window_c::load() {
    return mRes.load("Layout/ltr_win.arc");
}

// .text:0x244
void Window_c::drawWindow() {
    mWinLyt.entry();
}

// .text:0x24C
void Window_c::drawIcons() {
    for (int i = 0; i < LETTER_COUNT; i++) {
        mIcons[i].drawLayout();
    }
}

// .text:0x298
void Window_c::drawButton() {
    mBtnLyt.entry();
}

// .text:0x2A0
bool Window_c::remove() {
    return mRes.remove();
}

// .text:0x2CC
void Window_c::setMark(int letter, bool visible) {
    mIcons[letter].setMark(visible);
}

// .text:0x2E4
void Window_c::setNoChoice(int letter, bool visible) {
    mIcons[letter].setNoChoice(visible);
}

// .text:0x2FC
void Window_c::setSelected(int letter, bool selected) {
    mIcons[letter].setSelected(selected);
}

// .text:0x314
void Window_c::startWindowAnm(int anm, bool toEnd) {
    static const char *windowAnmNames[WIN_ANM_COUNT] = {
        "ltr_win_00_ltrWinOpn.brlan", "ltr_win_00_ltrWinCls.brlan",
        "ltr_win_00_ltrWinOut.brlan", "ltr_win_00_ltrWinIn.brlan",
        "ltr_win_00_post_In.brlan",   "ltr_win_00_post_Out.brlan"};
    mWinLyt.setAnm(&mWinAnm, windowAnmNames[anm], nullptr);
    if (toEnd) {
        mWinAnm.setToEnd();
    }
}

// .text:0x378
void Window_c::startButtonAnm(int anm, bool toEnd) {
    static const char *buttonAnmNames[BTN_ANM_COUNT] = {
        "ltr_btn_00_ltrBtnPsh.brlan", "ltr_btn_00_icn_ltr_bck.brlan",
        "ltr_btn_00_bckBtnPsh.brlan", "ltr_btn_00_icn_bck_ltr.brlan"};
    mBtnLyt.changeAnm(&mBtnAnm, buttonAnmNames[anm], nullptr);
    if (toEnd) {
        mBtnAnm.setToEnd();
    } else {
        mBtnAnm.setFrame(0.0f);
    }
    mBtnAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    switch (anm) {
    case BTN_ANM_LETTER_PUSH:
        dMenuBase_c::playSeDecide();
        break;
    case BTN_ANM_BACK_PUSH:
        dMenuBase_c::playSeCancel();
        break;
    }
}

// .text:0x434
int Window_c::hitLetter(const mVec2_c *pos, bool setOffset, float scale) {
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    HitInfo arg;
    arg.hit = nullptr;
    arg.scale = scale;
    arg.pos = *pos;
    mWinLyt.patrolPane(m2d::isInPane, m2d::isRectButton, &arg);
    if (arg.hit) {
        int index = m2d::getNameNumber(arg.hit->GetName(), 2);
        if (setOffset) {
            mVec3_c screen;
            m2d::getScreenPos(&screen, arg.hit);
            mVec2_c offset;
            offset.x = screen.x - pos->x;
            offset.y = screen.y - pos->y;
            mMove.setOfs(offset);
        }
        return index;
    }
    return -1;
}

// .text:0x53C
int Window_c::hitButton(const mVec2_c *pos, float scale) {
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    HitInfo arg;
    arg.hit = nullptr;
    arg.scale = scale;
    arg.pos = *pos;
    mBtnLyt.patrolPane(m2d::isInPane, m2d::isCircleButton, &arg);
    if (arg.hit) {
        return m2d::getNameNumber(arg.hit->GetName(), 2);
    }
    return -1;
}

// .text:0x5F0
void Window_c::setPos(m2d::SimpleEx_c *base) {
    base->copyPos(&mWinLyt, "N_ltrWin_all");
    base->copyPos(&mBtnLyt, "N_ltrBtn_all");
}

// .text:0x64C
// The letter panes are "Bxx"; "BC_..." panes are not letters.
int Window_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    const char *name = pane->GetName();
    if (name[0] == 'B' && name[1] != 'C') {
        int index = m2d::getNameNumber(name, 2);
        if (index >= 0 && index < LETTER_COUNT) {
            mVec3_c pos;
            m2d::getScreenPos(&pos, pane);
            ((Icon_c *)arg)[index].setRootPos(&pos);
        }
    }
    return 0;
}

// .text:0x6E4
void Window_c::setFaceButton(bool visible) {
    mBtnLyt.setVisible("ltrBtn_fc_00", visible);
}

// .text:0x6F8
bool Window_c::calcButtonAnm() {
    bool stopped = mBtnAnm.mFrameCtrl.isStop();
    mBtnAnm.play();
    return stopped;
}

// .text:0x740
bool Window_c::calcWindowAnm() {
    bool stopped = mWinAnm.mFrameCtrl.isStop();
    mWinAnm.play();
    return stopped;
}

// .text:0x788
void Window_c::playWindowAnm() {
    bool playing = false;
    if (!mWinAnm.mFrameCtrl.isStop()) {
        mWinAnm.play();
        playing = true;
    }
    if (playing) {
        calcLayouts();
    }
}

// .text:0x7E8
bool Window_c::calcVanish(int letter) {
    return mIcons[letter].calcVanish();
}

// .text:0x7F8
void Window_c::calcRotate() {
    for (int i = 0; i < LETTER_COUNT; i++) {
        mIcons[i].calcRotate();
    }
}

// .text:0x844
void Window_c::setMail(int letter, dMail_c *mail, int unused) {
    mIcons[letter].setMail(mail, unused);
}

// .text:0x860
mVec2_c Window_c::getLetterPos(int letter, float dx, float dy) {
    mVec3_c pos = mIcons[letter].getRootPos();
    mVec2_c ret(pos.x, pos.y);
    ret.incX(dx);
    ret.incY(dy);
    return ret;
}

// .text:0x8E8
void Window_c::drawHeldLetter() {
    mHeldIcon.drawLayout();
}

// .text:0x8F0
void Window_c::calcHeldLetter() {
    mHeldIcon.setPos(mMove.getPos());
    mHeldIcon.calcLayout();
}

// .text:0x934
void Window_c::setHeldMail(const dMail_c *mail) {
    mHeldMail.copy(mail);
    mHeldIcon.setMail(&mHeldMail, -1);
}

// .text:0x974
dMail_c *Window_c::getHeldMail() {
    return &mHeldMail;
}

// .text:0x97C
void Window_c::clearHeldMail() {
    mHeldMail.clear();
    mHeldIcon.setMail(&mHeldMail, -1);
}

// .text:0x9BC
void Window_c::fn_5_9BC() {
}

// .text:0x9C0
bool Window_c::isMoveEnd() {
    return mMove.isMoveEnd();
}
// .text:0x9C8
bool Window_c::calcMove() {
    return mMove.calcMove();
}
// .text:0x9D0
void Window_c::startMove(const mVec2_c &pos) {
    mMove.startMove(pos);
}
// .text:0x9D8
void Window_c::calcFollow() {
    mMove.calcFollow();
}
// .text:0x9E0
void Window_c::startFollow() {
    mMove.startFollow();
}
// .text:0x9E8
void Window_c::setMoveOffset(const mVec2_c &ofs) {
    mMove.setOfs(ofs);
}
// .text:0x9F0
void Window_c::setMovePos(const mVec2_c &pos) {
    mMove.setPos(pos);
}

// .text:0x9F8
void Icon_c::create(m2d::ResAccIf_c *res) {
    mLyt.build(res, "ltr_icon_00.brlyt");
    mLyt.setAnm(&mTexAnm, "ltr_icon_00_ltrIcn_texPtn.brlan", nullptr);
    mLyt.setAnm(&mVanishAnm, "ltr_icon_00_ltrIcn_Vanish.brlan", nullptr);
    mLyt.setAnm(&mRotateAnm, "ltr_icon_00_Fc_rotate.brlan", nullptr);
    mFlags = 0;
    setSelected(false);
    setNoChoice(false);
    setMark(false);
}

// .text:0xAA0
void Icon_c::setPos(const mVec2_c *pos) {
    mLyt.mPos.x = pos->x;
    mLyt.mPos.y = pos->y;
}

// .text:0xAB4
void Icon_c::calcLayout() {
    mLyt.calc();
}

// .text:0xAC4
void Icon_c::drawLayout() {
    mLyt.entry();
}

// .text:0xAC8
void Icon_c::setRootPos(const mVec3_c *pos) {
    mLyt.getRootPane()->SetTranslate(*pos);
}

// .text:0xB0C
void Icon_c::setScale(const mVec2_c &scale) {
    mLyt.getRootPane()->SetScale(scale);
}

// .text:0xB48
void Icon_c::setAlpha(u8 alpha) {
    mLyt.getRootPane()->SetAlpha(alpha);
}

// .text:0xB78
// Returns a reference to a converted temporary, like
// dMenuItem::Icon_c::getRootPos.
const mVec3_c &Icon_c::getRootPos() {
    return mLyt.getRootPane()->GetTranslate();
}

// .text:0xBB4
void Icon_c::setSelected(bool selected) {
    static const char *baseAnmNames[BASE_ANM_COUNT] = {
        "ltr_icon_00_normalBase.brlan", "ltr_icon_00_selectBase.brlan"};
    mLyt.setAnm(&mBaseAnm, baseAnmNames[selected], nullptr);
    mBaseAnm.setToEnd();
    mLyt.setVisible("checkFc_00", selected);
    mLyt.calc();
}

// .text:0xC34
void Icon_c::setNoChoice(bool visible) {
    mLyt.setVisible("noChoice_00", visible);
}

// .text:0xC44
void Icon_c::setMark(bool visible) {
    mLyt.setVisible("ltrIcnFc", visible);
    if (visible) {
        mLyt.calc();
    }
}

// .text:0xCA4
void Icon_c::setMail(dMail_c *mail, int unused) {
    int valid = !mail->isEmpty();
    bool visible = valid != 0;
    mLyt.setVisible("ltrIcnBase", visible);
    mLyt.setVisible("ltrIcn", visible);
    if (valid) {
        mTexAnm.setFrame(getTexFrame(mail));
        mVanishAnm.setFrame(0.0f);
    }
    calcLayout();
}

// .text:0xD80
// Pairs of frames by letter state; the odd frame has a present attached.
int Icon_c::getTexFrame(dMail_c *mail) {
    int frame;
    switch (mail->getStatus()) {
    case 1:
        frame = 0;
        break;
    case 2:
        frame = 2;
        break;
    case 3:
        frame = 4;
        if (mail->isToOtherTown()) {
            frame = 6;
        } else if (mail->isToFutureSelf()) {
            frame = 8;
        }
        break;
    }
    if (mail->getPresent() != dItem::ITEM_ID_NONE) {
        frame++;
    }
    return frame;
}

// .text:0xE3C
bool Icon_c::calcVanish() {
    if (mVanishAnm.mFrameCtrl.isStop()) {
        return true;
    }
    mVanishAnm.play();
    mLyt.calc();
    return false;
}

// .text:0xE9C
void Icon_c::calcRotate() {
    mRotateAnm.play();
    mLyt.calc();
}

} // namespace dMenuLtr
