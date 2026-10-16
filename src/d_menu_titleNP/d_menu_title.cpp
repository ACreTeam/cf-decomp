#include <game/game/d_menu_title.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>

namespace dMenuTitle {

// .text:0x5C
bool Title_c::load() {
    return mRes.load("Layout/menu_title.arc");
}

// .text:0x68
void Title_c::create() {
    mLyt.build(&mRes, "common_title_win_00.brlyt");
    mLyt.setAnm(&mIconAnm, "common_title_win_00_iconPattern.brlan", nullptr);
    mLyt.calc();
    mFlags = 0;
    mFlags |= FLAG_DRAW;
}

// .text:0xDC
void Title_c::setMessage(u16 msgNo) {
    mLyt.setMessage("T_msg_00", msgNo, nullptr);
}

// .text:0xF4
bool Title_c::remove() {
    return mRes.remove() != FALSE;
}

// .text:0x120
void Title_c::calcAnm() {
    if (!mWinAnm.mFrameCtrl.isStop()) {
        mWinAnm.play();
        mFlags |= FLAG_CALC;
    }
}

// .text:0x16C
void Title_c::draw() {
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    if (mFlags & FLAG_DRAW) {
        mLyt.entry();
    }
}

// .text:0x1D0
void Title_c::startAnm(int anm) {
    m2d::ResAccIf_c *resAcc = dMenuMng_c::getWinResAcc();
    mLyt.setAnm(&mWinAnm, dMenuBase_c::getAnmName(anm), resAcc);
}

// .text:0x230
void Title_c::setIcon(int icon) {
    mIconAnm.setFrame(icon);
    mFlags |= FLAG_CALC;
}

} // namespace dMenuTitle
