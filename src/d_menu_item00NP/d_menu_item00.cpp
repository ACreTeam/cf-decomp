#include <game/game/d_menu_item00.hpp>
#include <game/framework/f_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_a_npc_sp_yuutaro_loft.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_clock.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>

namespace dMenuItem00 {
// .text:0x5C
BASE_PROFILE(MENU_ITEM00, dMenuItem00_c)
} // namespace dMenuItem00

static const char *l_itmWinAll = "N_itmWin_all";

// .text:0x290
int dMenuItem00_c::create() {
    int loaded = load();
    loaded &= mBtn.load();
    loaded &= mMsgWin.load();
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
    mMsgWin.create();
    if (getMenuNo() == dMenuMng_c::MENU_ITEM) {
        startAnm(mItemWin.getLayout(), l_itmWinAll, ANM_WIN_IN, STATE_MAIN,
                 TRUE);
        mWinMode = WIN_MODE_ITEM;
    } else {
        setState(STATE_3);
        setNextState(STATE_MAIN);
        mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_IN);
        mWinMode = WIN_MODE_LETTER;
        mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_OPEN, true);
        mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_LETTER_TO_BACK,
                               true);
        onFlag(FLAG_DRAW_LETTER_WINDOW);
    }
    backToMain();
    dCamera_c::sInstance->mRequestedMode = 1;
    dMenuMng_c::resetToolPocket();
    return SUCCEEDED;
}

// .text:0x3FC
int dMenuItem00_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mMsgWin.remove()) {
        return NOT_READY;
    }
    bool removed = remove();
    return removed;
}

// .text:0x45C
int dMenuItem00_c::stateMain() {
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x4A4
int dMenuItem00_c::stateAnm() {
    int result = dMenuItemXX_c::stateAnm();
    mBtn.setPos(mItemWin.getLayout(), l_itmWinAll);
    return result;
}

// .text:0x4FC
int dMenuItem00_c::stateClose() {
    if (isFlag(FLAG_RUB)) {
        u16 msg;
        dAcNpcSpYuutaroLoft_c *loft =
            (dAcNpcSpYuutaroLoft_c *)fManager_c::searchBaseByProfName(
                fProf::A_NPC_SP_YUUTARO_LOFT, NULL);
        if (loft != NULL) {
            loft->summon(mSelectSlot);
            msg = 0;
        } else if (dNet::isSessionActive() && dNet::isHost()) {
            if (dNet::getMemberNum() > 1) {
                msg = dAcNpcSpYuutaroLoft_c::talk_c::MSG_NET_GUESTS;
            } else {
                msg = dAcNpcSpYuutaroLoft_c::talk_c::MSG_NET_ALONE;
            }
        } else if (dScene::isSceneAttr(dScene::getCurrentScene(),
                                       dScene::SCENE_ATTR_PLAYER_HOUSE)) {
            if (dScene::isCurrentSceneMyHouse()) {
                msg = dAcNpcSpYuutaroLoft_c::talk_c::MSG_ATTIC_MY_HOUSE;
            } else {
                msg = dAcNpcSpYuutaroLoft_c::talk_c::MSG_ATTIC_OTHER_HOUSE;
            }
        } else {
            msg = dAcNpcSpYuutaroLoft_c::talk_c::MSG_ATTIC_ELSEWHERE;
        }
        if (msg != 0) {
            dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
            comp->attachActor(&mRcpt);
            mRcpt.setMessageLabel("npc_SP/NPC_yutarou");
            mRcpt.setMessageCode(msg);
            dItem::Item key((u16)SP_NPC_GHOST);
            mRcpt.setVoiceType(dNpcInfo::getSpVoice(&key));
            dHmnName::Word_c name;
            dItem::Item npc((u16)SP_NPC_GHOST);
            dNpc::getNpcName(&name, &npc, 10);
            dScript::Word_c *word = &name;
            mRcpt.setSpeakerName((const u16 *)word->getBuffer(), 2);
            comp->requestStart(0);
            setSubState(SUB_MSG_WAIT);
            setState(STATE_MAIN);
            dAcUitag::hideTab();
            return SUCCEEDED;
        }
    }
    return dMenuBase_c::stateClose();
}

// .text:0x6B8
void dMenuItem00_c::subStateMain() {
    if (isReturning()) {
        return;
    }
    hideTutorial();
    int tab = dAcUitag::getTabInput(0, 0);
    if (tab != -1 && dMenuItemXX::selectTab(this, tab + SLOT_TAB, TRUE)) {
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = hitTab(&pos);
    if (mCursor != SLOT_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            if (dMenuItemXX::selectTab(this, mCursor, FALSE)) {
                return;
            }
        }
        return;
    }
    mCursor = hitPocket(pos, 1.0f);
    if (mCursor != SLOT_NONE) {
        if (isPocketEmpty(mCursor)) {
            mCursor = SLOT_NONE;
            return;
        }
        markSlot(mCursor);
        showSlotTutorial(mCursor, 0, 0);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setSubState(SUB_POCKET_PRESS);
            mHoldTimer = 0;
            mPressSlot = mCursor;
        }
        return;
    }
    mCursor = hitButton(pos, 1.0f);
    if (mCursor == SLOT_LETTER_BUTTON) {
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_LETTER_PUSH,
                                   false);
            setSubState(SUB_LETTER_OPEN);
            hideTutorial();
        } else {
            showSlotTutorial(SLOT_LETTER_BUTTON, 0, 0);
        }
    } else if (mCursor == SLOT_MONEY) {
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            openMoneyAnswers();
            openSelectBelow(TRUE);
            setSubState(SUB_SELECT);
            hideTutorial();
        } else {
            showSlotTutorial(SLOT_MONEY, 0, 0);
        }
    } else if (mCursor == SLOT_PLAYER) {
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            openPlayerAnswers();
            openSelect(mCursor, TRUE);
            setSubState(SUB_SELECT);
        }
    } else {
        mCursor = SLOT_NONE;
    }
}

// .text:0x9E8
void dMenuItem00_c::subStatePocketPress() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        if (openPocketAnswers(mPressSlot)) {
            openSelect(mPressSlot, TRUE);
            setSubState(SUB_SELECT);
            mPressSlot = SLOT_NONE;
            mCursor = SLOT_NONE;
        } else {
            setSubState(SUB_MAIN);
        }
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int hit = hitPocket(pos, 1.0f);
    mCursor = mPressSlot;
    markSlot(mCursor);
    if (hit == mPressSlot && mHoldTimer < dMenuMove_c::getHoldFrames()) {
        mHoldTimer++;
    } else {
        pickUp();
        setSubState(SUB_HOLD_ITEM);
        hideTutorial();
    }
}

// .text:0xB34
void dMenuItem00_c::subStateHoldItem() {
    hideTutorial();
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = hitPocket(pos, 1.2f);
    if (mCursor == SLOT_NONE) {
        mCursor = hitButton(pos, 1.0f);
        if (mHeldFlag != 0) {
            mCursor = SLOT_NONE;
        } else {
            switch (mCursor) {
            case SLOT_FACE:
                if (canPlant(&mHeldItem)) {
                    dMenuItemXX::showButtonTutorial(this, SLOT_FACE,
                                                    dObjTutorial_c::MENU_MSG_2);
                } else if (canDrop(mHeldItem, mHeldFlag)) {
                    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                                            dScene::SCENE_ATTR_TOWN)) {
                        dMenuItemXX::showButtonTutorial(
                            this, SLOT_FACE, dObjTutorial_c::MENU_MSG_0);
                    } else {
                        dMenuItemXX::showButtonTutorial(
                            this, SLOT_FACE, dObjTutorial_c::MENU_MSG_1);
                    }
                } else {
                    mCursor = SLOT_NONE;
                }
                break;
            case SLOT_MONEY:
                if (!mHeldItem.isMoney()) {
                    mCursor = SLOT_NONE;
                }
                break;
            case SLOT_PLAYER:
                if (!canWear(&mHeldItem) && !isEdible(&mHeldItem) &&
                    !isMedicine(&mHeldItem)) {
                    mCursor = SLOT_NONE;
                }
                break;
            default:
                mCursor = SLOT_NONE;
                break;
            }
        }
    } else {
        showSlotTutorial(mCursor, 0, 0);
    }
    if (mCursor != SLOT_NONE) {
        markSlot(mCursor);
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_A) && dObjCursor_c::isVisible()) {
        return;
    }
    int result = dropItem(FALSE);
    offFlag(FLAG_ANSWER);
    switch (result) {
    case DROP_NONE:
        if (!startEquip(&mHeldItem)) {
            setSlotItem(mHeldFrom, dItem::Item(mHeldItem.getId()), 0);
            clearHeld();
            showMessage(MSG_REMOVE_ACCESSORY_FIRST, FALSE);
        }
        break;
    case DROP_EAT:
        mItem = mHeldItem;
        clearHeld();
        unmarkSlot(SLOT_PLAYER);
        answerEat();
        break;
    case DROP_MEDICINE:
        mItem = mHeldItem;
        clearHeld();
        unmarkSlot(SLOT_PLAYER);
        answerTakeMedicine();
        break;
    case DROP_FACE:
        mItem = mHeldItem;
        clearHeld();
        unmarkSlot(SLOT_FACE);
        if (canPlant(&mItem)) {
            plantItem();
        } else {
            answerDrop();
        }
        break;
    case DROP_DONE:
        backToMain();
        break;
    case DROP_WALLET_FULL:
        showMessage(MSG_WALLET_FULL, FALSE);
        break;
    default:
        backToMain();
        break;
    }
}

// .text:0xEA4
void dMenuItem00_c::subStateLetterMain() {
    if (mCursor != SLOT_NONE) {
        unmarkSlot(mCursor);
        mCursor = SLOT_NONE;
    }
    if (isReturning()) {
        return;
    }
    hideTutorial();
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B)) {
        mCursor = SLOT_LETTER_BUTTON;
        markSlot(SLOT_LETTER_BUTTON);
        mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_BACK_PUSH, false);
        setSubState(SUB_LETTER_CLOSE);
        return;
    }
    mCursor = hitSlot();
    if (isPocket(mCursor)) {
        if (isPocketEmpty(mCursor)) {
            mCursor = SLOT_NONE;
            return;
        }
        showSlotTutorial(mCursor, 0, 1);
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setSubState(SUB_LETTER_POCKET_PRESS);
            mHoldTimer = 0;
            mPressSlot = mCursor;
        }
    } else if (isLetter(mCursor)) {
        if (isLetterEmpty(mCursor)) {
            mCursor = SLOT_NONE;
            return;
        }
        showSlotTutorial(mCursor, 0, 0);
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setSubState(SUB_LETTER_PRESS);
            mHoldTimer = 0;
            mPressSlot = mCursor;
        }
    } else if (mCursor == SLOT_LETTER_BUTTON) {
        markSlot(mCursor);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_BACK_PUSH,
                                   false);
            setSubState(SUB_LETTER_CLOSE);
        } else {
            showSlotTutorial(SLOT_LETTER_BUTTON, 1, 0);
        }
    } else {
        mCursor = SLOT_NONE;
    }
}

// .text:0x1148
void dMenuItem00_c::subStateLetterPocketPress() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        if (openPresentAnswers(mPressSlot)) {
            openSelect(mPressSlot, TRUE);
            setSubState(SUB_SELECT);
            mPressSlot = SLOT_NONE;
            mCursor = SLOT_NONE;
        } else {
            mCursor = mPressSlot;
            markSlot(mCursor);
            setSubState(SUB_LETTER_MAIN);
        }
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int hit = hitPocket(pos, 1.0f);
    mCursor = mPressSlot;
    markSlot(mCursor);
    if (hit == mPressSlot && mHoldTimer < dMenuMove_c::getHoldFrames()) {
        mHoldTimer++;
    } else {
        pickUp();
        setSubState(SUB_LETTER_HOLD_ITEM);
        hideTutorial();
    }
}

// .text:0x12A4
void dMenuItem00_c::subStateLetterHoldItem() {
    hideTutorial();
    mCursor = hitSlot();
    if (isPocket(mCursor)) {
        showSlotTutorial(mCursor, 0, 1);
        markSlot(mCursor);
    } else if (isLetter(mCursor)) {
        if (!isLetterEmpty(mCursor)) {
            showSlotTutorial(mCursor, 0, 0);
            markSlot(mCursor);
        } else {
            mCursor = SLOT_NONE;
        }
    } else {
        mCursor = SLOT_NONE;
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_A) && dObjCursor_c::isVisible()) {
        return;
    }
    int result = dropItem(FALSE);
    offFlag(FLAG_ANSWER);
    switch (result) {
    case DROP_DONE:
    case DROP_HAS_PRESENT:
        backToMain();
        break;
    case DROP_INSECT:
        showMessage(MSG_NO_BUG_IN_LETTER, FALSE);
        break;
    case DROP_FISH:
        showMessage(MSG_NO_FISH_IN_LETTER, FALSE);
        break;
    case DROP_TURNIP:
        showMessage(MSG_NO_TURNIP_IN_LETTER, FALSE);
        break;
    default:
        backToMain();
        break;
    }
}

// .text:0x145C
void dMenuItem00_c::subStateLetterPress() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        openLetterAnswers(mPressSlot);
        openSelect(mPressSlot, TRUE);
        setSubState(SUB_SELECT);
        return;
    }
    dObjCursor_c::getPos();
    int hit = hitSlot();
    mCursor = mPressSlot;
    markSlot(mCursor);
    if (hit == mPressSlot && mHoldTimer < dMenuMove_c::getHoldFrames()) {
        mHoldTimer++;
    } else {
        pickUp();
        setSubState(SUB_HOLD_LETTER);
        hideTutorial();
    }
}

// .text:0x155C
void dMenuItem00_c::subStateHoldLetter() {
    hideTutorial();
    mCursor = hitSlot();
    if (isLetter(mCursor)) {
        markSlot(mCursor);
    } else {
        mCursor = SLOT_NONE;
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_A) && dObjCursor_c::isVisible()) {
        return;
    }
    int result = dropLetter();
    offFlag(FLAG_ANSWER);
    switch (result) {
    case DROP_DONE:
    case DROP_HAS_PRESENT:
        backToMain();
        break;
    default:
        backToMain();
        break;
    }
}

// .text:0x1648
void dMenuItem00_c::subStateSelect() {
    if (mSelect.isMode(&dSelect::BComp_c::executeLock)) {
        int code = mSelect.mCode;
        mSubSelect.reset();
        mSubSelect.mNum = 0;
        switch (code) {
        case ANS_MEASURE_TIME:
            if (!addTimerAnswers()) {
                mSelect.unlock(true);
                return;
            }
            break;
        default:
            mSelect.unlock(true);
            return;
        }
        mSubSelect.addItem(MSG_QUIT, ANS_CANCEL, 0, 0);
        mSubSelect.setCancelLast();
        openSubSelect();
        setSubState(SUB_SUB_SELECT_OUT);
    }
    dSelect::BaseComp_c *select = &mSelect;
    if (select->isClosed()) {
        u16 code = mSelect.mCode;
        onFlag(FLAG_ANSWER);
        if (code != 0) {
            hideTutorial();
        }
        onAnswer(code);
    }
}

// .text:0x1790
void dMenuItem00_c::subStateSubSelectOut() {
    if (mSubSelect.isOut()) {
        if (mSubSelect.mCode != ANS_BACK) {
            mSelect.unlock(true);
        }
        setSubState(SUB_SUB_SELECT);
    }
}

// .text:0x181C
void dMenuItem00_c::subStateSubSelect() {
    dSelect::BaseComp_c *sub = &mSubSelect;
    if (sub->isClosed()) {
        u16 code = mSubSelect.mCode;
        onFlag(FLAG_ANSWER);
        switch (code) {
        default:
            answerCancel();
            break;
        case ANS_BACK:
            mSelect.unlock(false);
            setSubState(SUB_SELECT);
            break;
        case ANS_CANCEL:
            answerCancel();
            break;
        case ANS_TIMER_3:
            startTimer(3);
            break;
        case ANS_TIMER_5:
            startTimer(5);
            break;
        case ANS_TIMER_10:
            startTimer(10);
            break;
        case ANS_TIMER_15:
            startTimer(15);
            break;
        }
    }
}

// .text:0x1914
void dMenuItem00_c::subStateLetterOpen() {
    if (mLtrWin.calcButtonAnm()) {
        startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_DOWN, STATE_MAIN,
                 TRUE);
        mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_OPEN, false);
        onFlag(FLAG_LETTER_ANM);
        setSubState(SUB_LETTER_ANM);
        setNextSubState(SUB_LETTER_MAIN);
        mWinMode = WIN_MODE_LETTER;
        onFlag(FLAG_DRAW_LETTER_WINDOW);
        dAcUitag::hideTab();
        mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_LETTER_TO_BACK,
                               false);
    }
    onFlag(FLAG_CALC_LETTER);
}

// .text:0x19E8
void dMenuItem00_c::subStateLetterAnm() {
    if (mLtrWin.calcButtonAnm()) {
        changeNextSubState();
    }
    onFlag(FLAG_CALC_LETTER);
}

// .text:0x1A34
void dMenuItem00_c::subStateLetterClose() {
    if (mLtrWin.calcButtonAnm()) {
        dAcUitag::showTab();
        startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_UP, STATE_MAIN,
                 TRUE);
        mLtrWin.startWindowAnm(dMenuLtr::Window_c::WIN_ANM_CLOSE, false);
        onFlag(FLAG_LETTER_ANM);
        setSubState(SUB_LETTER_CLOSE_ANM);
        setNextSubState(SUB_MAIN);
        mWinMode = WIN_MODE_ITEM;
    }
    onFlag(FLAG_CALC_LETTER);
}

// .text:0x1AEC
void dMenuItem00_c::subStateLetterCloseAnm() {
    mLtrWin.startButtonAnm(dMenuLtr::Window_c::BTN_ANM_BACK_TO_LETTER, false);
    offFlag(FLAG_DRAW_LETTER_WINDOW);
    setSubState(SUB_LETTER_ANM);
    subStateLetterAnm();
}

// .text:0x1B50
void dMenuItem00_c::subStateChangeEquip() {
    BOOL changed;
    switch (mEquipKind) {
    case EQUIP_HELD:
        changed = dPlayerMgr::changeHeldItem(&mEquipItem);
        break;
    case EQUIP_SHIRT:
        changed = dPlayerMgr::changeShirt(&mEquipItem);
        break;
    case EQUIP_ACCESSORY:
        changed = dPlayerMgr::changeAccessory(&mEquipItem);
        break;
    case EQUIP_HAT:
        changed = dPlayerMgr::changeHat(&mEquipItem);
        break;
    default:
        backToMain();
        return;
    }
    if (changed) {
        setSubState(SUB_CHANGE_EQUIP_END);
    }
}

// .text:0x1C00
void dMenuItem00_c::subStateChangeEquipEnd() {
    if (!dPlayerMgr::isChangingEquipment()) {
        if (isDesignWear(&mTakeOffItem)) {
            mTakeOffItem = dItem::Item();
        }
        int slot = isFlag(FLAG_ANSWER) ? mSelectSlot : mHeldFrom;
        if (slot != SLOT_NONE) {
            mTakeOffItem = setSlotItem(slot, mTakeOffItem, 0);
        }
        backToMain();
    }
}

// .text:0x1CA0
void dMenuItem00_c::subStatePutHeld() {
    switch (dFgobjManager::playPutEffect(mRequest)) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED:
        backToMain();
        if (isFlag(FLAG_ANSWER)) {
            int slot = mSelectSlot;
            clearSlotItem(slot);
        }
        dFgobjManager::clearRequest(mRequest);
        break;
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        playSeCanNotLeave();
        showMessage(MSG_NO_SPACE_TO_PUT, TRUE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        dFgobjManager::clearRequest(mRequest);
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return;
    }
}

// .text:0x1D88
void dMenuItem00_c::subStateRoomDecor() {
    BOOL done;
    if (mEquipKind == EQUIP_CARPET) {
        done = dBgUtil::isCarpetDone();
    } else {
        done = dBgUtil::isWallpaperDone();
    }
    if (done) {
        backToMain();
    }
}

// .text:0x1DD8
void dMenuItem00_c::subStateShowCatch() {
    if (dPlayerMgr::showCatch(&mItem)) {
        setSubState(SUB_SHOW_CATCH_WAIT);
        windowUp(false);
        dCamera_c::sInstance->mRequestedMode = 2;
    }
}

// .text:0x1E44
void dMenuItem00_c::subStateShowCatchWait() {
    if (dPlayerMgr::isShowCatchFinished()) {
        mShowOff = 1;
        mBtn.setVisible(0, false);
        mBtn.setVisible(1, false);
        mBtn.setText(2, 0x20, 0);
        setSubState(SUB_CATCH_BUTTON);
        windowIn(true);
    }
}

// .text:0x1ED8
void dMenuItem00_c::subStateCatchButton() {
    mBtn.clearSelect();
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mBtn.push(btn, FALSE);
        setSubState(SUB_CATCH_PUSH);
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        mBtn.setSelect(btn, TRUE, FALSE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            mBtn.push(btn, FALSE);
            setSubState(SUB_CATCH_PUSH);
        }
    }
}

// .text:0x1FD4
void dMenuItem00_c::subStateCatchPush() {
    if (!mBtn.isPushing()) {
        dPlayerMgr::finishShowCatch();
        setSubState(SUB_CATCH_END);
        windowUp(true);
        dCamera_c::sInstance->mRequestedMode = 1;
    }
}

// .text:0x2044
void dMenuItem00_c::subStateCatchEnd() {
    backToMain();
    mShowOff = 0;
    windowIn(false);
}

// .text:0x2084
void dMenuItem00_c::subStateUseTool() {
    switch (dFgobjManager::getRequestResult(mRequest)) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED:
        if (isFlag(FLAG_ANSWER)) {
            unmarkSlot(mSelectSlot);
            int slot = mSelectSlot;
            clearSlotItem(slot);
        }
        dFgobjManager::clearRequest(mRequest);
        dPlayerMgr::putItemInHole(mHoleX, mHoleZ, FALSE);
        setSubState(SUB_WAIT_HOLE);
        break;
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        showMessage(MSG_CANT_BURY, FALSE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        dFgobjManager::clearRequest(mRequest);
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return;
    }
}

// .text:0x2194
void dMenuItem00_c::subStateWaitHole() {
    if (dPlayerMgr::isState(dPlayerMgr::STATE_DEMO_START,
                            dPlayerMgr::PLAYER_INDEX_CURRENT) == 1u) {
        backToMain();
    }
}

// .text:0x21D8
void dMenuItem00_c::subStatePlant() {
    switch (dFgobjManager::getRequestResult(mRequest)) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED:
        if (isFlag(FLAG_ANSWER)) {
            int slot = mSelectSlot;
            clearSlotItem(slot);
        }
        backToMain();
        dFgobjManager::clearRequest(mRequest);
        break;
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        showMessage(MSG_CANT_PLANT, FALSE);
        dFgobjManager::clearRequest(mRequest);
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return;
    }
}

// .text:0x22BC
void dMenuItem00_c::subStateGift() {
    if (calcGift(mSelectSlot)) {
        setSlotItem(mSelectSlot, dItem::Item(getPocketItem(mSelectSlot)), 0);
        backToMain();
    }
}

// .text:0x2320
void dMenuItem00_c::subStateVanishLetter() {
    BOOL vanished = TRUE;
    if (isLetter(mSelectSlot)) {
        vanished = mLtrWin.calcVanish(mSelectSlot - SLOT_LETTER);
    }
    if (vanished) {
        clearLetter(mSelectSlot);
        backToMain();
    }
}

// .text:0x239C
void dMenuItem00_c::subStateEat() {
    if (dPlayerMgr::eatItem(&mItem)) {
        if (isFlag(FLAG_ANSWER)) {
            int slot = mSelectSlot;
            setSlotItem(slot, dItem::Item(), 0);
            unmarkSlot(mSelectSlot);
        }
        setSubState(SUB_WAIT_EAT);
    }
}

// .text:0x242C
void dMenuItem00_c::subStateWaitEat() {
    if (dPlayerMgr::isState(dPlayerMgr::STATE_DEMO_START,
                            dPlayerMgr::PLAYER_INDEX_CURRENT) == 1u) {
        backToMain();
    }
}

// .text:0x2470
void dMenuItem00_c::subStateRemoveMask() {
    if (dPlayerMgr::removeMaskForEating()) {
        if (isFlag(FLAG_ANSWER)) {
            int slot = mSelectSlot;
            setSlotItem(slot, dItem::Item(), 0);
            unmarkSlot(mSelectSlot);
        }
        setSubState(SUB_WAIT_EQUIP_END);
    }
}

// .text:0x24FC
void dMenuItem00_c::subStateWaitEquipEnd() {
    if (!dPlayerMgr::isChangingEquipment()) {
        backToMain();
    }
}

// .text:0x2538
void dMenuItem00_c::subStateBeginDemo() {
    if (dPlayerMgr::beginDemo(mCatchPos.x, mCatchPos.z)) {
        setSubState(SUB_WAIT_DEMO);
    }
}

// .text:0x258C
void dMenuItem00_c::subStateRelease() {
    const mVec3_c *pos = NULL;
    if (mItem.getKind() == dItem::KIND_FISH) {
        pos = &mCatchPos;
    }
    if (dPlayerMgr::showCatchAt(&mItem, pos)) {
        closeWindow();
    }
}

// .text:0x25F0
void dMenuItem00_c::subStateWaitDemo() {
    if (dPlayerMgr::isStartingDemo(dPlayerMgr::PLAYER_INDEX_CURRENT) == 1u) {
        backToMain();
    }
}

// .text:0x2630
void dMenuItem00_c::subStateMsgWin() {
    if (mMsgWin.isWinAnmEnd() && mMsgWin.checkInput()) {
        setSubState(SUB_MSG_WIN_PUSH);
    }
}

// .text:0x2690
void dMenuItem00_c::subStateMsgWinPush() {
    if (mMsgWin.isPushEnd()) {
        setSubState(SUB_MSG_WIN_CLOSE);
        mMsgWin.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x26F4
void dMenuItem00_c::subStateMsgWinClose() {
    if (mMsgWin.isWinAnmEnd()) {
        backToMain();
        offFlag(FLAG_DRAW_MSG_WIN);
    }
}

// .text:0x2740
void dMenuItem00_c::subStateMsgWait() {
    dMsg::Comp_c *comp = mRcpt.getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setSubState(SUB_MSG_END);
        }
    }
}

// .text:0x27C0
void dMenuItem00_c::subStateMsgEnd() {
    dMsg::Comp_c *comp = mRcpt.getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            mRcpt.getController()->detachActor();
            dMenuBase_c::stateClose();
            dAcUitag::showTab();
        }
    }
}

// .text:0x283C
bool dMenuItem00_c::closeByMode() {
    if (mWinMode == WIN_MODE_ITEM) {
        return forceClose();
    }
    return forceCloseLetter();
}

// .text:0x2850
bool dMenuItem00_c::forceClose() {
    closeWindow();
    mCursor = SLOT_NONE;
    if (isFlag(FLAG_HOLD_ITEM)) {
        dropItem(TRUE);
    }
    if (isFlag(FLAG_HOLD_LETTER)) {
        dropLetter();
    }
    finishReturn();
    return true;
}

// .text:0x28CC
bool dMenuItem00_c::forceCloseLetter() {
    if (!forceClose()) {
        return false;
    }
    dAcUitag::setHideAfterMenu();
    return true;
}

// .text:0x2904
bool dMenuItem00_c::closeSelects() {
    bool ready = false;
    if (mSelect.isReady() && mSubSelect.isReady()) {
        ready = true;
    }
    if (!ready) {
        return false;
    }
    mSelect.close();
    mSubSelect.close();
    closeByMode();
    return true;
}

// .text:0x2988
bool dMenuItem00_c::closeMsgWin() {
    if (!mMsgWin.isWinAnmEnd()) {
        return false;
    }
    mMsgWin.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    closeByMode();
    return true;
}

// .text:0x29E8
bool dMenuItem00_c::closeButtons() {
    closeWindow();
    dAcUitag::setHideAfterMenu();
    return true;
}

// .text:0x2A10
void dMenuItem00_c::answerCancel() {
    backToMain();
}

// .text:0x2A14
void dMenuItem00_c::answerTakeBag(int item) {
    dItem::Item bag(item);
    if (addToPockets(bag, 0)) {
        addMoney(-bag.getPrice());
        backToMain();
    } else {
        showMessage(MSG_CANT_TAKE_BELLS, FALSE);
    }
}

// .text:0x2A94
void dMenuItem00_c::answerTakeAll() {
    int money = getMoney();
    int item;
    if (money >= 1000) {
        item = money / 1000 + 0x5B;
    } else {
        item = money / 100 + 0x52;
    }
    answerTakeBag(item);
}

// .text:0x2B10
void dMenuItem00_c::answerAddToWallet() {
    if (depositMoney(mItem.getPrice(), mSelectSlot)) {
        backToMain();
    } else {
        showMessage(MSG_WALLET_FULL, FALSE);
    }
}

// .text:0x2B74
void dMenuItem00_c::answerWear() {
    dItem::Item item(getPocketItem(mSelectSlot));
    if (!startEquip(&item)) {
        showMessage(MSG_REMOVE_ACCESSORY_FIRST, FALSE);
    }
}

// .text:0x2BCC
void dMenuItem00_c::answerSetDecor() {
    int kind = mItem.getKind();
    dItem::Item result;
    if (!dSTR::isAlone()) {
        if (kind == dItem::KIND_CARPET) {
            showMessage(MSG_CANT_CHANGE_FLOORING, FALSE);
        } else {
            showMessage(MSG_CANT_CHANGE_WALLPAPER, FALSE);
        }
        return;
    }
    switch (kind) {
    case dItem::KIND_CARPET:
        mEquipKind = EQUIP_CARPET;
        dBgUtil::setCarpet(dItem::Item(mItem.getId()), 2, 1, 1, 1);
        result = dBgUtil::getCarpet();
        break;
    case dItem::KIND_WALL:
        mEquipKind = EQUIP_WALLPAPER;
        dBgUtil::setWallpaper(dItem::Item(mItem.getId()), 1, 1, 1, 1);
        result = dBgUtil::getWallpaper();
        break;
    }
    setSlotItem(mSelectSlot, result, 0);
    setSubState(SUB_ROOM_DECOR);
}

// .text:0x2D08
void dMenuItem00_c::answerRemove(int kind) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (kind) {
    case EQUIP_HELD:
        mTakeOffItem = player->mEquipment.mHeld;
        break;
    case EQUIP_HAT:
        mTakeOffItem = player->mEquipment.mHat;
        break;
    case EQUIP_ACCESSORY:
        mTakeOffItem = player->mEquipment.mAcc;
        break;
    }
    int slot = SLOT_NONE;
    if (!isDesignWear(&mTakeOffItem)) {
        slot = player->findEmptyPocket(0);
        if (slot == SLOT_NONE) {
            showMessage(MSG_CANT_REMOVE_POCKETS_FULL, FALSE);
            return;
        }
    }
    if (slot == SLOT_NONE) {
        mHeldFrom = SLOT_NONE;
    } else {
        mHeldFrom = slot;
    }
    mSelectSlot = mHeldFrom;
    mEquipKind = kind;
    mEquipItem = dItem::Item();
    setSubState(SUB_CHANGE_EQUIP);
}

// .text:0x2E24
void dMenuItem00_c::answerShowOff() {
    setSubState(SUB_SHOW_CATCH);
    subStateShowCatch();
}

// .text:0x2E68
void dMenuItem00_c::answerBury() {
    int me = dNet::getMyNo();
    if (!dField::getInfo(FD_ID_CURRENT)->getItem(mHoleX, mHoleZ, 0)->isHole()) {
        showMessage(MSG_CANT_BURY, FALSE);
        return;
    }
    mRequest = dFgobjManager::useTool(
        me, mHoleX, mHoleZ, dFgobjManager::USE_TOOL_SHOVEL, 0, mItem.getId());
    if (mRequest == -1) {
        showMessage(MSG_CANT_BURY, FALSE);
    } else {
        setSubState(SUB_USE_TOOL);
        subStateUseTool();
    }
}

// .text:0x2F48
void dMenuItem00_c::plantItem() {
    mRequest = dFgobjManager::putItem(dNet::getMyNo(), FG_MNG_KIND_PLANT,
                                      mItem.getId());
    if (mRequest == -1) {
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        showMessage(MSG_CANT_PLANT, FALSE);
    } else {
        setSubState(SUB_PLANT);
        subStatePlant();
    }
}

// .text:0x2FF0
void dMenuItem00_c::answerPlant() {
    if (fgMngProc_findHoleInFront(&mHoleX, &mHoleZ)) {
        answerBury();
    } else {
        plantItem();
    }
}

// .text:0x3040
void dMenuItem00_c::answerOpenPresent() {
    if (mItem.getKind() == dItem::KIND_PBOX) {
        u8 rate = dHR::getRate(4)[2];
        if (cM::rndInt(100) < rate / 10 + 40) {
            dItemSel::Range_c ftr(dItem::KIND_FTR, 0x54);
            dItemSel::Range_c wall(dItem::KIND_WALL, 0x54);
            dItemSel::Range_c carpet(dItem::KIND_CARPET, 0x54);
            int roll = cM::rndInt(14);
            if (roll == 0) {
                dItemSel::select(&mItem, 1, &carpet, 1,
                                 &dItemSel::sDefaultFilter, NULL, 0, NULL);
            } else if (roll == 1) {
                dItemSel::select(&mItem, 1, &wall, 1, &dItemSel::sDefaultFilter,
                                 NULL, 0, NULL);
            } else {
                dItemSel::select(&mItem, 1, &ftr, 1, &dItemSel::sDefaultFilter,
                                 NULL, 0, NULL);
            }
        } else {
            dItemSel::Range_c ftr(dItem::KIND_FTR, 0);
            dItemSel::select(&mItem, 1, &ftr, 1, &dItemSel::sDefaultFilter,
                             NULL, 0, NULL);
        }
        setSlotItem(mSelectSlot, mItem, 1);
    }
    setSubState(SUB_GIFT);
    mVec2_c pos = getSlotPos(mSelectSlot, 0.0f, 0.0f);
    mMtx_c mtx;
    mtx.trans(pos.x, pos.y, 0.0f);
    dEffect::create("af_ui_boxopen", &mtx);
    playSePresentOpen();
}

// .text:0x3230
void dMenuItem00_c::answerEat() {
    setSubState(SUB_EAT);
    subStateEat();
}

// .text:0x3274
void dMenuItem00_c::answerTakeMedicine() {
    setSubState(SUB_REMOVE_MASK);
    subStateRemoveMask();
}

// .text:0x32B8
bool dMenuItem00_c::addTimerAnswers() {
    if (dClock::isHideAndSeek()) {
        return false;
    }
    mSubSelect.addItem(MSG_3_MINUTES, ANS_TIMER_3, 0, 0);
    mSubSelect.addItem(MSG_5_MINUTES, ANS_TIMER_5, 0, 0);
    mSubSelect.addItem(MSG_10_MINUTES, ANS_TIMER_10, 0, 0);
    mSubSelect.addItem(MSG_15_MINUTES, ANS_TIMER_15, 0, 0);
    return true;
}

// .text:0x3358
void dMenuItem00_c::answerMeasureTime() {
    showMessage(MSG_NO_TIMER_OTHERS_HIDE_SEEK + (dNet::isHost() != 0), FALSE);
}

// .text:0x33A4
void dMenuItem00_c::answerStopTimer() {
    dClock::requestStopTimer();
    backToMain();
}

// .text:0x33D8
void dMenuItem00_c::answerDrop() {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem);
    if (dScene::isCurrentSceneMyHouse() &&
        dItem::clampField(bitm->m_ftrFunc, dItem::FTR_FUNC_COUNT,
                          dItem::FTR_FUNC_PLAIN) != dItem::FTR_FUNC_NONE) {
        placeFurniture();
    } else {
        putOnGround();
    }
}

// .text:0x3460
void dMenuItem00_c::answerToss() {
    setSubState(SUB_VANISH_LETTER);
    hideTutorial();
}

// .text:0x34A4
void dMenuItem00_c::answerRemovePresent() {
    int pocket = dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0);
    if (pocket == -1) {
        showMessage(MSG_CANT_REMOVE_PRESENT, FALSE);
    } else {
        takePresent(mSelectSlot, pocket);
        playSeItemDrop();
        backToMain();
    }
}

// .text:0x3510
void dMenuItem00_c::answerWriteLetter() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->findLetter() == -1) {
        showMessage(MSG_LETTERS_FULL, FALSE);
        return;
    }
    dAcUitag::openMenuA(dAcUitag::MENU_A_ADDRESS);
    dAcUitag::hideTab();
    dMenuMng_c::setPickedLetter(mSelectSlot);
    dItem::Item paper(getPocketItem(mSelectSlot));
    dMail_c mail;
    mail.setupFromPlayer(&paper);
    dItem::Item inviteCard(dItem::ITEM_IDX_INVITE_CARD);
    int paperIdx = dItem::seeker_c::get()->findLike(inviteCard);
    if (paperIdx == mail.mPaper) {
        if (player->isFlag0(PRIVATE_FLAG0_REDD_MEMBER)) {
            mail.flagInvite();
        }
    }
    dMenuMng_c::setMail(&mail);
    closeWindowSe();
    playSeSubMenuClose();
}

// .text:0x3608
void dMenuItem00_c::answerRewrite() {
    dAcUitag::openMenuA(dAcUitag::MENU_A_REWRITE);
    setState(STATE_3);
    setNextState(STATE_CLOSE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_OUT);
    dMenuMng_c::setPickedLetter(mSelectSlot - SLOT_LETTER);
    dMail_c mail(*getLetter(mSelectSlot));
    dMenuMng_c::setMail(&mail);
    playSeSubMenuClose();
}

// .text:0x3790
void dMenuItem00_c::answerBeginDemo() {
    setSubState(SUB_BEGIN_DEMO);
}

// .text:0x37A4
void dMenuItem00_c::answerRelease() {
    int slot = mSelectSlot;
    setSlotItem(slot, dItem::Item(), 0);
    setSubState(SUB_RELEASE);
    subStateRelease();
}

// .text:0x3808
void dMenuItem00_c::answerOpenEgg() {
    if (mItem.getKind() == dItem::KIND_EGG_FAKE_BEFORE) {
        dItem::seeker_c::get()->search(dItem::KIND_CANDY, 6, NULL);
        mItem = dItem::seeker_c::get()->getRandom();
    } else {
        mItem.setFromIndex(0xD4, dItem::seeker_c::get()->findLike(mItem), 0);
    }
    setSlotItem(mSelectSlot, mItem, 6);
    setSubState(SUB_GIFT);
    playSeEggCrack();
}

// .text:0x38D4
void dMenuItem00_c::answerRub() {
    onFlag(FLAG_RUB);
    closeWindow();
}

// .text:0x390C
void dMenuItem00_c::answerChangeAddressee() {
    dAcUitag::openMenuA(dAcUitag::MENU_A_ADDRESS_LETTER);
    setState(STATE_3);
    setNextState(STATE_CLOSE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_OUT);
    dMenuMng_c::setPickedLetter(mSelectSlot - SLOT_LETTER);
    playSeSubMenuClose();
}

// .text:0x3970
void dMenuItem00_c::answerRead() {
    dAcUitag::openMenuA(dAcUitag::MENU_A_READ);
    setState(STATE_3);
    setNextState(STATE_CLOSE);
    mItemWin.startAnm(dMenuItem::Window_c::ANM_CHEST_OUT);
    int slot = mSelectSlot;
    dMail_c mail(*getLetter(slot));
    mail.markRead();
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setLetter(&mail, slot - SLOT_LETTER);
    dMenuMng_c::setMail(&mail);
    playSeSubMenuClose();
}

// .text:0x3B10
void dMenuItem00_c::onAnswer(int answer) {
    switch (answer) {
    case ANS_DROP:
        answerDrop();
        break;
    case ANS_TAKE_ALL:
        answerTakeAll();
        break;
    case ANS_TAKE_100:
        answerTakeBag(0x53);
        break;
    case ANS_TAKE_1000:
        answerTakeBag(0x5C);
        break;
    case ANS_TAKE_10000:
        answerTakeBag(0x65);
        break;
    case ANS_ADD_TO_WALLET:
        answerAddToWallet();
        break;
    case ANS_WEAR:
        answerWear();
        break;
    case ANS_SET_DECOR:
        answerSetDecor();
        break;
    case ANS_REMOVE_HELD:
        answerRemove(EQUIP_HELD);
        break;
    case ANS_REMOVE_HAT:
        answerRemove(EQUIP_HAT);
        break;
    case ANS_REMOVE_ACC:
        answerRemove(EQUIP_ACCESSORY);
        break;
    case ANS_SHOW_OFF:
        answerShowOff();
        break;
    case ANS_BURY:
        answerBury();
        break;
    case ANS_PLANT:
        answerPlant();
        break;
    case ANS_OPEN_PRESENT:
        answerOpenPresent();
        break;
    case ANS_EAT:
        answerEat();
        break;
    case ANS_TAKE_MEDICINE:
        answerTakeMedicine();
        break;
    case ANS_TOSS:
        answerToss();
        break;
    case ANS_REMOVE_PRESENT:
        answerRemovePresent();
        break;
    case ANS_WRITE_LETTER:
        answerWriteLetter();
        break;
    case ANS_REWRITE:
        answerRewrite();
        break;
    case ANS_BEGIN_DEMO:
        answerBeginDemo();
        break;
    case ANS_RELEASE:
        answerRelease();
        break;
    case ANS_OPEN_EGG:
        answerOpenEgg();
        break;
    case ANS_CHANGE_ADDRESSEE:
        answerChangeAddressee();
        break;
    case ANS_READ:
        answerRead();
        break;
    case ANS_STOP_TIMER:
        answerStopTimer();
        break;
    case ANS_RUB:
        answerRub();
        break;
    case ANS_MEASURE_TIME:
        answerMeasureTime();
        break;
    default:
        answerCancel();
        break;
    }
}

// .text:0x3BC0
bool dMenuItem00_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return false;
    }
    typedef bool (dMenuItem00_c::*CloseProc)();
    static CloseProc closeProcs[SUB_NUM] = {
        &dMenuItem00_c::forceClose,
        0,
        &dMenuItem00_c::forceClose,
        &dMenuItem00_c::forceCloseLetter,
        0,
        &dMenuItem00_c::forceCloseLetter,
        0,
        &dMenuItem00_c::forceCloseLetter,
        &dMenuItem00_c::closeSelects,
        &dMenuItem00_c::closeSelects,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        &dMenuItem00_c::closeButtons,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        &dMenuItem00_c::closeMsgWin,
        0,
        0,
        0,
        0,
    };
    if (closeProcs[mSubState] == 0) {
        return false;
    }
    bool closed = (this->*closeProcs[mSubState])();
    if (closed) {
        dMenuMng_c::clearCloseRequest();
    }
    return closed;
}

// .text:0x3E14
void dMenuItem00_c::executeSubState() {
    calcReturn();
    typedef void (dMenuItem00_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem00_c::subStateMain,
        &dMenuItem00_c::subStatePocketPress,
        &dMenuItem00_c::subStateHoldItem,
        &dMenuItem00_c::subStateLetterMain,
        &dMenuItem00_c::subStateLetterPocketPress,
        &dMenuItem00_c::subStateLetterHoldItem,
        &dMenuItem00_c::subStateLetterPress,
        &dMenuItem00_c::subStateHoldLetter,
        &dMenuItem00_c::subStateSelect,
        &dMenuItem00_c::subStateSubSelectOut,
        &dMenuItem00_c::subStateSubSelect,
        &dMenuItem00_c::subStateLetterOpen,
        &dMenuItem00_c::subStateLetterAnm,
        &dMenuItem00_c::subStateLetterClose,
        &dMenuItem00_c::subStateLetterCloseAnm,
        &dMenuItem00_c::subStateChangeEquip,
        &dMenuItem00_c::subStateChangeEquipEnd,
        &dMenuItem00_c::subStatePutHeld,
        &dMenuItem00_c::subStateRoomDecor,
        &dMenuItem00_c::subStateShowCatch,
        &dMenuItem00_c::subStateShowCatchWait,
        &dMenuItem00_c::subStateCatchButton,
        &dMenuItem00_c::subStateCatchPush,
        &dMenuItem00_c::subStateCatchEnd,
        &dMenuItem00_c::subStateUseTool,
        &dMenuItem00_c::subStateWaitHole,
        &dMenuItem00_c::subStatePlant,
        &dMenuItem00_c::subStateGift,
        &dMenuItem00_c::subStateVanishLetter,
        &dMenuItem00_c::subStateEat,
        &dMenuItem00_c::subStateWaitEat,
        &dMenuItem00_c::subStateRemoveMask,
        &dMenuItem00_c::subStateWaitEquipEnd,
        &dMenuItem00_c::subStateBeginDemo,
        &dMenuItem00_c::subStateRelease,
        &dMenuItem00_c::subStateWaitDemo,
        &dMenuItem00_c::subStateMsgWin,
        &dMenuItem00_c::subStateMsgWinPush,
        &dMenuItem00_c::subStateMsgWinClose,
        &dMenuItem00_c::subStateMsgWait,
        &dMenuItem00_c::subStateMsgEnd,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x3E60
int dMenuItem00_c::execute() {
    int result = dMenuItemXX_c::execute();
    mBtn.execute();
    mMsgWin.execute();
    return result;
}

// .text:0x3EAC
int dMenuItem00_c::draw() {
    if (mShowOff) {
        mBtn.draw();
    } else {
        dMenuItemXX_c::draw();
    }
    if (isFlag(FLAG_DRAW_MSG_WIN)) {
        mMsgWin.draw();
    }
    return SUCCEEDED;
}

// .text:0x3F10
bool dMenuItem00_c::addPocketAnswers(int slot) {
    if (isPocketEmpty(slot)) {
        return false;
    }
    mItem = getPocketItem(slot);
    int flag = getPocketFlag(slot);
    switch (flag) {
    case 0:
        if (mItem.getKind() == dItem::KIND_TIMER) {
            if (dClock::isHideAndSeek()) {
                mSelect.addItem(MSG_MEASURE_TIME, ANS_MEASURE_TIME, 1, 0);
            } else if (!dClock::isTimerActive()) {
                mSelect.addItem(MSG_MEASURE_TIME, ANS_MEASURE_TIME, 1, 0);
            } else {
                mSelect.addItem(MSG_STOP, ANS_STOP_TIMER, 0, 0);
            }
        }
        if (canDrop(mItem, flag)) {
            if (dScene::isSceneAttr(dScene::getCurrentScene(),
                                    dScene::SCENE_ATTR_TOWN)) {
                mSelect.addItem(MSG_DROP, ANS_DROP, 0, 0);
            } else {
                mSelect.addItem(MSG_PLACE_IN_ROOM, ANS_DROP, 0, 0);
            }
        }
        if (canWear(&mItem)) {
            if (mItem.getHideBone() == 9) {
                mSelect.addItem(MSG_CARRY, ANS_WEAR, 0, 0);
            } else {
                mSelect.addItem(MSG_WEAR, ANS_WEAR, 0, 0);
            }
        }
        switch (getDecorType(&mItem)) {
        case DECOR_CARPET:
            mSelect.addItem(MSG_SPREAD_ON_FLOOR, ANS_SET_DECOR, 0, 0);
            break;
        case DECOR_WALLPAPER:
            mSelect.addItem(MSG_HANG_ON_WALL, ANS_SET_DECOR, 0, 0);
            break;
        }
        if (canReleaseFish(&mItem) || canReleaseInsect(&mItem)) {
            mSelect.addItem(MSG_RELEASE, ANS_RELEASE, 0, 0);
        }
        if (canShowOff(&mItem)) {
            mSelect.addItem(MSG_SHOW_IT_OFF, ANS_SHOW_OFF, 0, 0);
        }
        if (canPlant(&mItem)) {
            mSelect.addItem(MSG_PLANT, ANS_PLANT, 0, 0);
        }
        if (canBury(&mItem)) {
            mSelect.addItem(MSG_BURY, ANS_BURY, 0, 0);
        }
        if (isMedicine(&mItem)) {
            mSelect.addItem(MSG_TAKE, ANS_TAKE_MEDICINE, 0, 0);
        }
        if (mItem.isMoney()) {
            mSelect.addItem(MSG_ADD_TO_WALLET, ANS_ADD_TO_WALLET, 0, 0);
        }
        if (isStationery(&mItem)) {
            mSelect.addItem(MSG_WRITE_A_LETTER, ANS_WRITE_LETTER, 0, 0);
        }
        if (isEgg(&mItem)) {
            mSelect.addItem(MSG_OPEN, ANS_OPEN_EGG, 0, 0);
        }
        if (isMagicLamp(&mItem)) {
            mSelect.addItem(MSG_RUB, ANS_RUB, 0, 0);
        }
        break;
    case 1:
    case 2:
    case 4:
        mSelect.addItem(MSG_OPEN, ANS_OPEN_PRESENT, 0, 0);
        break;
    }
    return true;
}

// .text:0x42E8
bool dMenuItem00_c::openPocketAnswers(int slot) {
    bool opened = false;
    mSelect.reset();
    mSelect.mNum = 0;
    if (isPocket(slot)) {
        opened = addPocketAnswers(slot);
    }
    if (opened) {
        mSelect.addItem(MSG_QUIT, ANS_CANCEL, 0, 0);
        mSelect.setCancelLast();
    }
    return opened;
}

// .text:0x439C
void dMenuItem00_c::openMoneyAnswers() {
    mSelect.reset();
    mSelect.mNum = 0;
    int money = getMoney();
    if (money >= 100) {
        mSelect.addItem(MSG_ALL, ANS_TAKE_ALL, 0, 0);
        mSelect.addItem(MSG_100_BELLS, ANS_TAKE_100, 0, 0);
    }
    if (money >= 1000) {
        mSelect.addItem(MSG_1000_BELLS, ANS_TAKE_1000, 0, 0);
    }
    if (money >= 10000) {
        mSelect.addItem(MSG_10000_BELLS, ANS_TAKE_10000, 0, 0);
    }
    mSelect.addItem(MSG_NEVER_MIND, ANS_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x448C
void dMenuItem00_c::openPlayerAnswers() {
    mSelect.reset();
    mSelect.mNum = 0;
    dEquip_c &equip = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment;
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_ROOM) &&
        equip.mHeld.getId() != dItem::ITEM_ID_NONE) {
        mSelect.addItem(MSG_REMOVE_EQUIPMENT, ANS_REMOVE_HELD, 0, 0);
    }
    if (equip.mHat.getId() != dItem::ITEM_ID_NONE) {
        mSelect.addItem(MSG_REMOVE_HEADGEAR, ANS_REMOVE_HAT, 0, 0);
    }
    if (equip.mAcc.getId() != dItem::ITEM_ID_NONE) {
        mSelect.addItem(MSG_REMOVE_ACCESSORY, ANS_REMOVE_ACC, 0, 0);
    }
    if (mSelect.mNum == 0) {
        mSelect.addItem(MSG_NOTHING_TO_REMOVE, ANS_CANCEL, 0, 0);
    } else {
        mSelect.addItem(MSG_QUIT, ANS_CANCEL, 0, 0);
    }
    mSelect.setCancelLast();
}

// .text:0x45AC
void dMenuItem00_c::openLetterAnswers(int slot) {
    mSelect.reset();
    mSelect.mNum = 0;
    dMail_c *mail = getLetter(slot);
    if (mail->isWritten()) {
        mSelect.addItem(MSG_REWRITE, ANS_REWRITE, 0, 0);
        mSelect.addItem(MSG_CHANGE_ADDRESSEE, ANS_CHANGE_ADDRESSEE, 0, 0);
    } else {
        mSelect.addItem(MSG_READ, ANS_READ, 0, 0);
    }
    if (mail->getPresent() == dItem::ITEM_ID_NONE) {
        if (mail->getStatus() != 1 &&
            dScene::isSceneAttr(dScene::getCurrentScene(),
                                dScene::SCENE_ATTR_TOWN)) {
            mSelect.addItem(MSG_TOSS, ANS_TOSS, 0, 3);
        }
    } else {
        mSelect.addItem(MSG_REMOVE_PRESENT, ANS_REMOVE_PRESENT, 0, 0);
    }
    mSelect.addItem(MSG_QUIT, ANS_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x46F8
bool dMenuItem00_c::openPresentAnswers(int slot) {
    if (!isPocket(slot)) {
        return false;
    }
    if (isPocketEmpty(slot)) {
        return false;
    }
    if (getPocketFlag(slot) != 1) {
        return false;
    }
    mItem = getPocketItem(slot);
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_OPEN, ANS_OPEN_PRESENT, 0, 0);
    mSelect.addItem(MSG_QUIT, ANS_CANCEL, 0, 0);
    mSelect.setCancelLast();
    return true;
}

// .text:0x47F4
void dMenuItem00_c::backToMain() {
    if (dObjCursor_c::isVisible()) {
        mCursor = mSelectSlot;
    }
    setSubState(mWinMode);
}

// .text:0x4848
void dMenuItem00_c::showMessage(u16 msg, BOOL cancelSe) {
    mMsgWin.setBtnVisible(0, false);
    mMsgWin.setBtnVisible(1, false);
    mMsgWin.setBtn(2, 0xEC, NULL, 0);
    if (cancelSe) {
        playSeCanNotLeave();
    } else {
        playSeCanNot();
    }
    mMsgWin.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_MSG_WIN);
    onFlag(FLAG_DRAW_MSG_WIN);
    mMsgWin.setMsg(msg, NULL);
}

// .text:0x491C
BOOL dMenuItem00_c::startEquip(const dItem::Item *item) {
    dEquip_c &equip = dPlayerMgr::Mgr_c::getCurrentPlayer()->mEquipment;
    mEquipItem = *item;
    if (mEquipItem.getKind() == dItem::KIND_CLOTH) {
        mTakeOffItem = equip.mShirt;
        mEquipKind = EQUIP_SHIRT;
    } else {
        switch (mEquipItem.getHideBone()) {
        case 9:
            mTakeOffItem = equip.mHeld;
            mEquipKind = EQUIP_HELD;
            break;
        case 3:
        case 5:
            mTakeOffItem = equip.mHat;
            mEquipKind = EQUIP_HAT;
            break;
        case 2:
        case 4:
        case 6:
        case 7:
            if (equip.mAcc.getId() != dItem::ITEM_ID_NONE) {
                return FALSE;
            }
            mTakeOffItem = equip.mHat;
            mEquipKind = EQUIP_HAT;
            break;
        case 0:
        case 1:
            mTakeOffItem = equip.mHat;
            mEquipKind = EQUIP_HAT;
            break;
        case 8:
            mTakeOffItem = equip.mAcc;
            if (mTakeOffItem.getId() == dItem::ITEM_ID_NONE) {
                mTakeOffItem = equip.mHat;
                if (mTakeOffItem.getId() != dItem::ITEM_ID_NONE) {
                    int bone = mTakeOffItem.getHideBone();
                    if (bone != 7 && bone != 4 && bone != 6 && bone != 2) {
                        mTakeOffItem = dItem::Item();
                    }
                }
            }
            mEquipKind = EQUIP_ACCESSORY;
            break;
        default:
            return FALSE;
        }
    }
    setSubState(SUB_CHANGE_EQUIP);
    return TRUE;
}

// .text:0x4A94
void dMenuItem00_c::placeFurniture() {
    if (!dSTR::isAlone()) {
        showMessage(MSG_CANT_PUT_IN_ROOM, FALSE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        return;
    }
    u32 info;
    switch (dFtrm_c::findPutPos(&info, mItem, 1)) {
    case 0:
        dFtrm_c::createFtr(info);
        if (isFlag(FLAG_ANSWER)) {
            int slot = mSelectSlot;
            clearSlotItem(slot);
        }
        break;
    default:
        showMessage(MSG_NO_SPACE_TO_PUT, TRUE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        return;
    }
    backToMain();
}

// .text:0x4BBC
void dMenuItem00_c::putOnGround() {
    if (dScene::isCurrentSceneMyHouse() && !dSTR::isAlone()) {
        showMessage(MSG_CANT_PUT_IN_ROOM, FALSE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
        return;
    }
    mRequest = dFgobjManager::putHeldItem(dNet::getMyNo(), mItem.getId());
    if (mRequest == -1) {
        showMessage(MSG_NO_SPACE_TO_PUT, TRUE);
        if (!isFlag(FLAG_ANSWER)) {
            setSlotItem(mHeldFrom, mItem, 0);
        }
    } else {
        setSubState(SUB_PUT_HELD);
        subStatePutHeld();
    }
}

// .text:0x4CBC
bool dMenuItem00_c::canWear(const dItem::Item *item) {
    if (item->getKind() == dItem::KIND_CLOTH) {
        return true;
    }
    int bone = item->getHideBone();
    if (bone == 10) {
        return false;
    }
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_ROOM)) {
        if (bone == 9) {
            return false;
        }
    } else if (dScene::isCityScene(dScene::getCurrentScene()) &&
               !dHmnToolBank_c::isHandTool(item)) {
        return false;
    }
    return true;
}

// .text:0x4D78
int dMenuItem00_c::getDecorType(const dItem::Item *item) {
    if (!dScene::isCurrentSceneMyHouse()) {
        return DECOR_NONE;
    }
    if (getRoomFromScene(dScene::getCurrentScene()) == 3) {
        return DECOR_NONE;
    }
    int kind = item->getKind();
    if (kind == dItem::KIND_CARPET) {
        return DECOR_CARPET;
    }
    return -(kind == dItem::KIND_WALL) & DECOR_WALLPAPER;
}

// .text:0x4DFC
bool dMenuItem00_c::canShowOff(const dItem::Item *item) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN)) {
        int kind = item->getKind();
        if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH) {
            return true;
        }
    }
    return false;
}

// .text:0x4E5C
bool dMenuItem00_c::canBury(const dItem::Item *item) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN)) {
        const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(*item);
        if (info->getKindFlag6() == 0) {
            return false;
        }
        return fgMngProc_findHoleInFront(&mHoleX, &mHoleZ) != 0;
    }
    return false;
}

// .text:0x4EF0
bool dMenuItem00_c::canPlant(const dItem::Item *item) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN)) {
        return dItem::infoBank_c::get()->getBITM(*item)->getKindFlag5() != 0;
    }
    return false;
}

// .text:0x4F5C
bool dMenuItem00_c::isStationery(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_PAPER;
}

// .text:0x4F8C
bool dMenuItem00_c::isEgg(const dItem::Item *item) {
    int kind = item->getKind();
    return kind == dItem::KIND_EGG_FAKE_BEFORE ||
           kind == dItem::KIND_EGG_BINGO_BEFORE;
}

// .text:0x4FCC
BOOL dMenuItem00_c::isMagicLamp(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_MAGIC_LAMP));
}

// .text:0x500C
bool dMenuItem00_c::isDesignWear(const dItem::Item *item) {
    int kind = item->getKind();
    return kind == dItem::KIND_ORG_CLOTH || kind == dItem::KIND_ORG_UMB ||
           kind == dItem::KIND_ORG_CAP;
}

// .text:0x504C
BOOL dMenuItem00_c::canReleaseFish(const dItem::Item *item) {
    if (item->getKind() != dItem::KIND_FISH) {
        return FALSE;
    }
    return findWater(&mCatchPos);
}

// .text:0x5098
int dMenuItem00_c::hitSlot() {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int slot = hitLetter(pos, 1.0f);
    if (slot != SLOT_NONE) {
        return slot;
    }
    slot = hitPocket(pos, 1.0f);
    if (slot != SLOT_NONE) {
        return slot;
    }
    slot = hitButton(pos, 1.0f);
    if (slot == SLOT_LETTER_BUTTON) {
        return slot;
    }
    return SLOT_NONE;
}

// .text:0x5170
void dMenuItem00_c::startTimer(int minutes) {
    dClock::requestStartTimer(minutes);
    backToMain();
}

// .text:0x51A8
void dMenuItem00_c::playSeEggCrack() {
    playSe(SE_EggHibiware);
}

// .text:0x51B0
void dMenuItem00_c::playSePresentOpen() {
    playSe(SE_PresentOpen);
}

// Weak functions after the main section (no source here, see d_menu_item00.hpp
// and the implicit members): .text:0x51B8 ~dMenuItem00_c (implicit),
// .text:0x53F4 / 0x5480 dMenuItem::Icon_c ctor / dtor, .text:0x5534 / 0x5590
// dMenuLtr::Icon_c ctor / dtor, .text:0x5624 createHeap, 0x562C deleteReady,
// 0x5630 postDraw, 0x5634 state2 (the last four are declared in the class
// header).
