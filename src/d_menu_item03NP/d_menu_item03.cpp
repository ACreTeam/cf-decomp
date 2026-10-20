#include <game/game/d_menu_item03.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuItem03 {
// .text:0x5C
BASE_PROFILE(MENU_ITEM03, dMenuItem03_c)
} // namespace dMenuItem03

static const char *l_winPane = "N_itmWin_all";

// .text:0x268
int dMenuItem03_c::create() {
    BOOL loaded = dMenuItemXX_c::load();
    loaded &= mBtn.load();
    loaded &= mSlct.load();
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
    initButtons();
    mSlct.create();
    startAnm(mItemWin.getLayout(), l_winPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    if (mMenuNo == dMenuMng_c::MENU_SWAP_FISH) {
        mPos = g_menuPos;
        onFlag(FLAG_WATER);
    } else if (findWater(&mPos)) {
        onFlag(FLAG_WATER);
    }
    updatePockets();
    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_1;
    return SUCCEEDED;
}

// .text:0x3DC
int dMenuItem03_c::doDelete() {
    if (!mBtn.remove()) {
        return NOT_READY;
    }
    if (!mSlct.remove()) {
        return NOT_READY;
    }
    bool removed = remove();
    if (removed) {
        dCamera_c *camera = dCamera_c::sInstance;
        camera->mRequestedMode = camera->mDefaultMode;
    }
    return removed;
}

// .text:0x454
int dMenuItem03_c::stateMain() {
    calcReturnItem();
    executeSubState();
    calcUnused();
    return SUCCEEDED;
}

// .text:0x494
int dMenuItem03_c::stateAnm() {
    int result = dMenuItemXX_c::stateAnm();
    mBtn.setPos(mItemWin.getLayout(), l_winPane);
    return result;
}

// .text:0x4EC
int dMenuItem03_c::stateClose() {
    dMenuBase_c::stateClose();
    return SUCCEEDED;
}

// .text:0x510
void dMenuItem03_c::subStateMain() {
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
    mVec2_c cursor = dObjCursor_c::getPos();
    mCursor = hitPocket(cursor, 1.0f);
    if (mCursor != SLOT_NONE) {
        showSlotTutorial(mCursor, 0, FALSE);
        if (isPocketEmpty(mCursor)) {
            mCursor = SLOT_NONE;
            return;
        }
        markSlot(mCursor);
        if (isDisabled(mCursor)) {
            return;
        }
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            openAnswers(mCursor);
            openSelect(mCursor, TRUE);
            setSubState(SUB_SELECT);
            mCursor = SLOT_NONE;
        }
    } else {
        btn = mBtn.hitCheck(&cursor);
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

// .text:0x710
void dMenuItem03_c::subStatePush() {
    if (!mBtn.isPushing()) {
        dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
        closeWindowSe();
    }
}

// .text:0x758
void dMenuItem03_c::subStatePutEffect() {
    switch (dFgobjManager::playPutEffect(mTask)) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED:
        confirm();
        closeWindowSe();
        break;
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        showError(MSG_NO_SPACE_TO_PUT_DOWN, TRUE);
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return;
    }
    dFgobjManager::clearRequest(mTask);
}

// .text:0x7E0
void dMenuItem03_c::subStateRelease() {
    const mVec3_c *pos = nullptr;
    dItem::Item item = getPocketItem(mSelectSlot);
    if (item.getKind() == dItem::KIND_FISH) {
        pos = &mPos;
    }
    if (dPlayerMgr::showCatchAt(&item, pos)) {
        int player = dNet::getMyNo();
        switch (mMenuNo) {
        case dMenuMng_c::MENU_SWAP_PICK_UP:
        case dMenuMng_c::MENU_SWAP_DUG:
            fgMngTask_finishActive(player);
            break;
        }
        confirm();
        closeWindowSe();
    }
}

// .text:0x87C
void dMenuItem03_c::subStateUseTool() {
    switch (dFgobjManager::getRequestResult(mTask)) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED: {
        u16 *pos = fgMngTask_getActive(dNet::getMyNo());
        u16 unit = *pos;
        dPlayerMgr::putItemInHole(unit >> 8, unit & 0xFF, TRUE);
        setSubState(SUB_WAIT_IDLE);
        break;
    }
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        showError(MSG_NO_SPACE_TO_PUT_DOWN, TRUE);
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return;
    }
    dFgobjManager::clearRequest(mTask);
}

// .text:0x928
void dMenuItem03_c::subStateWaitIdle() {
    if (static_cast<u32>(
            dPlayerMgr::isIdle(dPlayerMgr::PLAYER_INDEX_CURRENT)) == TRUE) {
        confirm();
        closeWindowSe();
    }
}

// .text:0x970
void dMenuItem03_c::subStateDialogOpen() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x9D0
void dMenuItem03_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0xA34
void dMenuItem03_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
    }
}

// .text:0xA90
void dMenuItem03_c::subStateSelect() {
    if (static_cast<dSelect::BaseComp_c &>(mSelect).isClosed()) {
        u16 answer = mSelect.mCode;
        onFlag(FLAG_ANSWERED);
        if (answer != ANSWER_CANCEL) {
            hideTutorial();
        }
        selectAnswer(answer);
    }
}

// .text:0xB08
void dMenuItem03_c::executeSubState() {
    typedef void (dMenuItem03_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem03_c::subStateMain,        &dMenuItem03_c::subStatePush,
        &dMenuItem03_c::subStatePutEffect,   &dMenuItem03_c::subStateRelease,
        &dMenuItem03_c::subStateUseTool,     &dMenuItem03_c::subStateWaitIdle,
        &dMenuItem03_c::subStateDialogOpen,  &dMenuItem03_c::subStateDialogPush,
        &dMenuItem03_c::subStateDialogClose, &dMenuItem03_c::subStateSelect,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xB40
void dMenuItem03_c::calcReturnItem() {
    // The branch keeps the call from becoming a tail call.
    if (!calcReturn()) {
        return;
    }
}

// .text:0xB60
void dMenuItem03_c::calcUnused() {
}

// .text:0xB64
int dMenuItem03_c::execute() {
    int result = dMenuItemXX_c::execute();
    mBtn.execute();
    mSlct.execute();
    return result;
}

// .text:0xBB0
int dMenuItem03_c::draw() {
    int result = dMenuItemXX_c::draw();
    if (isFlag(FLAG_DRAW_ITEM)) {
        mBtn.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return result;
}

// .text:0xC24
void dMenuItem03_c::confirm() {
    dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
    dMenuMng_c::setSelSlot(mSelectSlot);
    dItem::Item item = dMenuMng_c::getMenuItem();
    setSlotItem(mSelectSlot, item, 0);
}

// .text:0xC7C
void dMenuItem03_c::startSwap(int slot) {
    mItem = getPocketItem(slot);
    dScene::getCurrentScene();
    const dItem::BITM *bitm = dItem::getBITM(mItem.getId());
    if (!dScene::isCurrentSceneMyHouse() || bitm->getFtrFunc() == 0) {
        startPutDown();
    } else {
        dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
        closeWindowSe();
    }
}

// .text:0xD18
void dMenuItem03_c::startPutDown() {
    int player = dNet::getMyNo();
    switch (mMenuNo) {
    case dMenuMng_c::MENU_SWAP_PICK_UP:
        mTask = dFgobjManager::swapItem(player, mItem.getId());
        break;
    case dMenuMng_c::MENU_SWAP_DUG:
        mTask = dFgobjManager::putItemAside(player, mItem.getId());
        break;
    case dMenuMng_c::MENU_SWAP_INSECT:
    case dMenuMng_c::MENU_SWAP_FISH:
        mTask = dFgobjManager::putHeldItem(player, mItem.getId());
        break;
    default:
        mTask = -1;
        break;
    }
    if (mTask == -1) {
        showError(MSG_NO_SPACE_TO_PUT_DOWN, TRUE);
    } else {
        setSubState(SUB_PUT_EFFECT);
        subStatePutEffect();
    }
}

// .text:0xDE4
void dMenuItem03_c::initButtons() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_QUIT, MSG_QUIT, TRUE);
}

// .text:0xE40
bool dMenuItem03_c::isDisabled(int slot) {
    u16 mask = 1 << slot;
    return (mDisabled & mask) != 0;
}

// .text:0xE64
void dMenuItem03_c::setDisabled(int slot, BOOL disabled) {
    u16 mask = 1 << slot;
    mItemWin.setNoChoice(slot, disabled != 0);
    if (disabled) {
        mDisabled |= mask;
    } else {
        mDisabled &= ~mask;
    }
}

// .text:0xEE4
void dMenuItem03_c::updatePockets() {
    for (int slot = 0; slot < dMenuMng_c::POCKET_NUM; slot++) {
        dItem::Item item = getPocketItem(slot);
        BOOL disabled = TRUE;
        if (item.getId() == dItem::ITEM_ID_NONE) {
            disabled = FALSE;
        } else if (getPocketFlag(slot)) {
            disabled = TRUE;
        } else if (canDropPocket(slot)) {
            disabled = FALSE;
        } else if (canReleaseFish(&item)) {
            disabled = FALSE;
        } else if (canReleaseInsect(&item)) {
            disabled = FALSE;
        }
        setDisabled(slot, disabled);
    }
}

// .text:0xFD0
bool dMenuItem03_c::canReleaseFish(const dItem::Item *item) {
    if (item->getKind() != dItem::KIND_FISH) {
        return false;
    }
    return isFlag(FLAG_WATER);
}

// .text:0x101C
bool dMenuItem03_c::canBury(const dItem::Item *item) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN)) {
        dItem::Item copy = *item;
        return dItem::infoBank_c::get()->getBITM(copy)->getKindFlag6() != 0;
    }
    return false;
}

// .text:0x1088
void dMenuItem03_c::backToMain() {
    setSubState(SUB_MAIN);
}

// .text:0x109C
void dMenuItem03_c::startSwapSelected() {
    startSwap(mSelectSlot);
}

// .text:0x10A4
void dMenuItem03_c::startRelease() {
    setSubState(SUB_RELEASE);
    subStateRelease();
}

// .text:0x10E8
void dMenuItem03_c::startBury() {
    mItem = getPocketItem(mSelectSlot);
    mTask = dFgobjManager::useToolAtTask(
        dNet::getMyNo(), dFgobjManager::USE_TOOL_SHOVEL, 0, mItem.getId());
    if (mTask == -1) {
        showError(MSG_NO_SPACE_TO_PUT_DOWN, FALSE);
    } else {
        setSubState(SUB_USE_TOOL);
        subStateUseTool();
    }
}

// .text:0x1170
void dMenuItem03_c::openAnswers(int slot) {
    dItem::Item item = getPocketItem(slot);
    mSelect.reset();
    mSelect.mNum = 0;
    if (canDropPocket(slot)) {
        mSelect.addItem(MSG_DROP, ANSWER_DROP, 0, 0);
    }
    if (mMenuNo == dMenuMng_c::MENU_SWAP_DUG && canBury(&item)) {
        mSelect.addItem(MSG_BURY, ANSWER_BURY, 0, 0);
    }
    if (canReleaseInsect(&item)) {
        mSelect.addItem(MSG_RELEASE, ANSWER_RELEASE, 0, 0);
    }
    if (canReleaseFish(&item)) {
        mSelect.addItem(MSG_RELEASE, ANSWER_RELEASE, 0, 0);
    }
    mSelect.addItem(MSG_QUIT, ANSWER_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x12A4
void dMenuItem03_c::selectAnswer(int answer) {
    switch (answer) {
    case ANSWER_CANCEL:
        backToMain();
        unmarkSlot(mSelectSlot);
        break;
    case ANSWER_DROP:
        startSwapSelected();
        break;
    case ANSWER_RELEASE:
        startRelease();
        break;
    case ANSWER_BURY:
        startBury();
        break;
    default:
        backToMain();
        break;
    }
}

// .text:0x1328
void dMenuItem03_c::showError(u16 msg, BOOL leave) {
    mSlct.setBtnVisible(SLCT_BTN_LEFT, false);
    mSlct.setBtnVisible(SLCT_BTN_MIDDLE, false);
    mSlct.setBtn(SLCT_BTN_OK, MSG_OK, nullptr, 0);
    if (leave) {
        playSeCanNotLeave();
    } else {
        playSeCanNot();
    }
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, TRUE, FALSE);
    setSubState(SUB_DIALOG_OPEN);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msg, nullptr);
}
