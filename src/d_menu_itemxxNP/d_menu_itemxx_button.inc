// The window buttons and the tab bar. The original file compiled this part
// first (its strings start .data) but its code follows the main section, as an
// #included source file does with -sym on.

// stripped function with 1.0f
#pragma force_active on
extern const float unused_one;
const float unused_one = 1.0f;
#pragma force_active reset

namespace dMenuItemXX {

// .text:0x3220
bool isPlayerPane(nw4r::lyt::Pane *pane) {
    return strcmp(pane->GetName(), "BC_player") == 0;
}

// .text:0x3254
bool selectTab(dMenuItemXX_c *menu, int slot, BOOL delay) {
    if (!dMenuItemXX_c::isTab(slot)) {
        return false;
    }
    if (dAcUitag::selectTab(slot - dMenuItemXX_c::SLOT_TAB, delay)) {
        menu->startAnm(menu->mItemWin.getLayout(), "N_itmWin_all",
                       dMenuBase_c::ANM_WIN_OUT, dMenuBase_c::STATE_CLOSE,
                       FALSE);
        return true;
    }
    return false;
}

// .text:0x32EC
void showButtonTutorial(dMenuItemXX_c *menu, int slot, int faceMsg) {
    u16 msg = 0;
    nw4r::lyt::Pane *pane = nullptr;
    int tail = 0;
    switch (slot) {
    case dMenuItemXX_c::SLOT_MONEY:
        msg = 9;
        pane = menu->mItemWin.getLayout()->findPane("N_btnL_00_all");
        break;
    case dMenuItemXX_c::SLOT_LETTER_BUTTON:
        if (faceMsg) {
            msg = 0xC;
            tail = 1;
        } else {
            msg = 0xA;
        }
        pane = menu->mItemWin.getLayout()->findPane("N_ltrBtn_all");
        break;
    case dMenuItemXX_c::SLOT_FACE:
        msg = faceMsg;
        pane = menu->mItemWin.getLayout()->findPane("N_btnL_01_all");
        break;
    }
    if (msg != 0 && pane != nullptr && dObjTutorial_c::request(0, msg, TRUE)) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        mVec2_c pos2;
        pos2.x = pos.x;
        pos2.y = pos.y;
        dObjTutorial_c::setMsg(&pos2, msg, tail);
    }
}

} // namespace dMenuItemXX
