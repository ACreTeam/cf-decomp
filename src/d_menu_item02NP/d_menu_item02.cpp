#include <game/game/d_menu_item02.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>

namespace dMenuItem02 {
// .text:0x5C
BASE_PROFILE(MENU_ITEM02, dMenuItem02_c)
} // namespace dMenuItem02

// .text:0x28C
int dMenuItem02_c::create() {
    int loaded = dMenuItemXX_c::load();
    loaded &= mBtn.load();
    loaded &= mSlct.load();
    loaded &= mTitle.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!createLayouts()) {
        return NOT_READY;
    }
    onFlag(FLAG_CALC_ITEM);
    onFlag(FLAG_DRAW_ITEM);
    onFlag(FLAG_CALC_LETTER);
    onFlag(FLAG_DRAW_LETTER_BUTTON);
    mBtn.create();
    setupButtons();
    mSlct.create();
    mTitle.create();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_ITEM_SONG:
        mTitle.setIcon(ICON_SONG);
        mTitle.setMessage(MSG_CHOOSE_SONG);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE:
        mTitle.setIcon(ICON_GIVE);
        mTitle.setMessage(MSG_GIVE_WHAT);
        break;
    case dMenuMng_c::MENU_ITEM_QUEST:
        mTitle.setIcon(ICON_QUEST);
        mTitle.setMessage(MSG_GIVE_WHAT);
        break;
    case dMenuMng_c::MENU_ITEM_SELL:
        mTitle.setIcon(ICON_SELL);
        mTitle.setMessage(MSG_SELL);
        break;
    case dMenuMng_c::MENU_ITEM_AUCTION:
        mTitle.setIcon(ICON_AUCTION);
        mTitle.setMessage(MSG_AUCTION);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE_FOOD:
        mTitle.setIcon(ICON_GIVE_A);
        mTitle.setMessage(MSG_GIVE_WHICH);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE_CANDY:
        mTitle.setIcon(ICON_GIVE_B);
        mTitle.setMessage(MSG_GIVE_WHICH_B);
        break;
    case dMenuMng_c::MENU_ITEM_DONATE:
        mTitle.setIcon(ICON_DONATE);
        mTitle.setMessage(MSG_DONATE);
        break;
    case dMenuMng_c::MENU_ITEM_IDENTIFY:
        mTitle.setIcon(ICON_DONATE);
        mTitle.setMessage(MSG_IDENTIFY);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE_FISH:
        mTitle.setIcon(ICON_GIVE_C);
        mTitle.setMessage(MSG_GIVE_WHICH_C);
        break;
    case dMenuMng_c::MENU_ITEM_SELECT:
        mTitle.setIcon(ICON_SWAP);
        mTitle.setMessage(MSG_SWAP);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE_INSECT:
        mTitle.setIcon(ICON_GIVE_D);
        mTitle.setMessage(MSG_GIVE_WHICH_C);
        break;
    case dMenuMng_c::MENU_ITEM_GIVE_SHIP_PART:
        mTitle.setIcon(ICON_GIVE_E);
        mTitle.setMessage(MSG_GIVE_WHAT_B);
        break;
    case dMenuMng_c::MENU_ITEM_GYROID:
        mTitle.setIcon(ICON_GYROID);
        mTitle.setMessage(MSG_GIVE_GYROID);
        break;
    }
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mTitle.startAnm(ANM_UPPER_WIN_IN);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_DESIGN_IN);
    setSubState(SUB_MAIN);
    switch (mMenuNo) {
    case dMenuMng_c::MENU_ITEM_SELL:
        onFlag(FLAG_MULTI);
        break;
    }
    mDisabledMask = dMenuMng_c::getPocketMask();
    mSelectedMask = 0;
    mSelectedSlot = SELECTED_NONE;
    updateDisabled();
    setConfirmDisabled(TRUE);
    return SUCCEEDED;
}

// .text:0x5F4
int dMenuItem02_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    if (mTitle.remove() == FALSE) {
        return NOT_READY;
    }
    return dMenuItemXX_c::remove();
}

// .text:0x66C
int dMenuItem02_c::stateMain() {
    updateReturn();
    executeSubState();
    stubUpdate();
    return SUCCEEDED;
}

// .text:0x6AC
int dMenuItem02_c::stateAnm() {
    return dMenuItemXX_c::stateAnm();
}

// .text:0x6B0
int dMenuItem02_c::stateClose() {
    if (mMenuNo == dMenuMng_c::MENU_ITEM_SONG &&
        !dAcUitag::openMenuB(dAcUitag::MENU_B_MUSIC)) {
        return SUCCEEDED;
    }
    dMenuBase_c::stateClose();
    return SUCCEEDED;
}

// .text:0x708
void dMenuItem02_c::subStateMain() {
    if (isReturning()) {
        return;
    }
    mBtn.clearSelect();
    hideTutorial();
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mBtn.push(btn, FALSE);
        setSubState(SUB_PUSH);
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = hitPocket(pos, 1.0f);
    if (mCursor != SLOT_NONE) {
        showSlotTutorial(mCursor, 0, FALSE);
        if (isPocketEmpty(mCursor)) {
            mCursor = SLOT_NONE;
            return;
        }
        markSlot(mCursor);
        int slot = mCursor;
        if (isDisabled(slot)) {
            return;
        }
        if (!mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            return;
        }
        if (isFlag(FLAG_MULTI)) {
            setSelected(slot, !isSelected(slot));
            setConfirmDisabled(mSelectedMask == 0);
            if (isSelected(slot)) {
                playSeDecide2();
            } else {
                playSeCancel2();
            }
            return;
        }
        if (mSelectedSlot < dMenuItem::Window_c::POCKET_COUNT) {
            setSelected(mSelectedSlot, FALSE);
        }
        if (mSelectedSlot != slot) {
            mSelectedSlot = slot;
            setSelected(slot, TRUE);
            switch (mMenuNo) {
            case dMenuMng_c::MENU_ITEM_SONG:
                playSeSongSelect();
                break;
            default:
                playSeDecide2();
                break;
            }
            setConfirmDisabled(FALSE);
        } else {
            mSelectedSlot = SELECTED_NONE;
            switch (mMenuNo) {
            case dMenuMng_c::MENU_ITEM_SONG:
                playSeSongDeselect();
                break;
            default:
                playSeCancel2();
                break;
            }
            setConfirmDisabled(TRUE);
        }
    } else {
        btn = mBtn.hitCheck(&pos);
        if (btn != -1) {
            mBtn.setSelect(btn, TRUE, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                mBtn.push(btn, FALSE);
                setSubState(SUB_PUSH);
            }
        }
        mCursor = SLOT_NONE;
    }
}

// .text:0x9D8
void dMenuItem02_c::subStatePush() {
    if (!mBtn.isPushing()) {
        if (mBtn.mPushedBtn == BTN_QUIT) {
            dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
        } else {
            if (mMenuNo == dMenuMng_c::MENU_ITEM_SONG) {
                if (learnSong()) {
                    closeMenu();
                } else {
                    showDialog(MSG_SONG_KNOWN);
                }
                return;
            }
            dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
            if (isFlag(FLAG_MULTI)) {
                dMenuMng_c::setSelMask(mSelectedMask);
                dMenuMng_c::setSelItems();
            } else {
                dMenuMng_c::setSelSlot(mSelectedSlot);
            }
        }
        closeMenu();
    }
}

// .text:0xA9C
void dMenuItem02_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0xAFC
void dMenuItem02_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0xB60
void dMenuItem02_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0xBBC
void dMenuItem02_c::executeSubState() {
    typedef void (dMenuItem02_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem02_c::subStateMain,        &dMenuItem02_c::subStatePush,
        &dMenuItem02_c::subStateDialog,      &dMenuItem02_c::subStateDialogPush,
        &dMenuItem02_c::subStateDialogClose,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xBF4
void dMenuItem02_c::updateReturn() {
    if (calcReturn()) {
        return;
    }
}

// .text:0xC14
void dMenuItem02_c::stubUpdate() {
}

// .text:0xC18
int dMenuItem02_c::execute() {
    int result = dMenuItemXX_c::execute();
    mBtn.execute();
    mSlct.execute();
    mTitle.calcAnm();
    if (mItemWin.calcAnm()) {
        onFlag(FLAG_CALC_ITEM);
    }
    return result;
}

// .text:0xC88
int dMenuItem02_c::draw() {
    int result = dMenuItemXX_c::draw();
    if (isFlag(FLAG_DRAW_ITEM)) {
        mBtn.draw();
        mTitle.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return result;
}

// .text:0xD04
void dMenuItem02_c::closeMenu() {
    hideTutorial();
    startAnm(&mBtn.mLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_DESIGN_OUT);
    mTitle.startAnm(ANM_UPPER_WIN_OUT);
}

// .text:0xD68
void dMenuItem02_c::setupButtons() {
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, FALSE);
    switch (mMenuNo) {
    default:
        mBtn.setText(BTN_CONFIRM, dMenuMng_c::getItemMsgParam(), FALSE);
        break;
    case dMenuMng_c::MENU_ITEM_SONG:
        mBtn.setText(BTN_CONFIRM, MSG_CONFIRM, FALSE);
        break;
    }
}

// .text:0xDF4
void dMenuItem02_c::setConfirmDisabled(int disabled) {
    mBtn.setDisable(BTN_CONFIRM, disabled != 0);
}

// .text:0xE10
BOOL dMenuItem02_c::isDisabled(int slot) {
    return (mDisabledMask & (u16)(1 << slot)) != 0;
}

// .text:0xE34
void dMenuItem02_c::setDisabled(int slot, int disabled) {
    u16 bit = 1 << slot;
    mItemWin.setNoChoice(slot, disabled != 0);
    if (disabled) {
        mDisabledMask |= bit;
    } else {
        mDisabledMask &= ~bit;
    }
}

// .text:0xEB4
void dMenuItem02_c::updateDisabled() {
    for (int i = 0; i < dMenuItem::Window_c::POCKET_COUNT; i++) {
        setDisabled(i, isDisabled(i));
    }
}

// .text:0xF10
BOOL dMenuItem02_c::isSelected(int slot) {
    return (mSelectedMask & (u16)(1 << slot)) != 0;
}

// .text:0xF34
void dMenuItem02_c::setSelected(int slot, int selected) {
    u16 bit = 1 << slot;
    mItemWin.setSelected(slot, selected != 0);
    if (selected) {
        mSelectedMask |= bit;
    } else {
        mSelectedMask &= ~bit;
    }
}

// .text:0xFB4
void dMenuItem02_c::showDialog(u16 msgNo) {
    mSlct.setBtnVisible(DIALOG_BTN_YES, FALSE);
    mSlct.setBtnVisible(DIALOG_BTN_NO, FALSE);
    mSlct.setBtn(DIALOG_BTN_OK, MSG_OK, nullptr, 0);
    playSeCanNot();
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msgNo, nullptr);
}

// .text:0x106C
BOOL dMenuItem02_c::learnSong() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int slot = mSelectedSlot;
    dItem::Item song(getPocketItem(slot));
    dPrivateSongs_c &songs = player->mSongs;
    if (songs.isSet(song)) {
        return FALSE;
    }
    songs.set(song);
    dItem::Item none;
    setSlotItem(slot, none, 0);
    return TRUE;
}

// .text:0x1114
void dMenuItem02_c::playSeSongSelect() {
    playSe(SE_MusicChoice);
}

// .text:0x111C
void dMenuItem02_c::playSeSongDeselect() {
    playSe(SE_MusicChoiceCancel);
}
