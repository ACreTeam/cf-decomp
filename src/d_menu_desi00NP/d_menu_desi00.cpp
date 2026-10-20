#include <game/game/d_menu_desi00.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuDesi00 {
// .text:0x5C
BASE_PROFILE(MENU_DESI00, dMenuDesi00_c)
} // namespace dMenuDesi00

static const char *l_allPane = "N_slctD_Win_all";

// .text:0x2F0
int dMenuDesi00_c::create() {
    bool loaded = mDesi.load();
    loaded &= mSelect.load();
    loaded &= mSubSelect.load();
    loaded &= mDialog.load();
    if (!loaded) {
        return NOT_READY;
    }
    hideTutorial();
    mSelect.build();
    mSubSelect.build();
    mSubSelect.setPinkWindow();
    mDialog.create();
    mDesi.create(TRUE);
    if (mDesi.setupMii()) {
        onFlag(FLAG_MII);
        mDesi.setMiiAnm(hasMiiMask());
    }
    onFlag(FLAG_DRAW);
    setSubState(SUB_MAIN);
    dCamera_c::sInstance->mRequestedMode = 1;
    startAnm(mDesi.getLayout(), l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    if (mMenuNo == dMenuMng_c::MENU_DESIGN_SELECT) {
        updateWorn(dMenuMng_c::getDesignIndex());
    }
    return SUCCEEDED;
}

// .text:0x468
int dMenuDesi00_c::doDelete() {
    if (mDesi.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSelect.remove() == FALSE) {
        return NOT_READY;
    }
    if (mSubSelect.remove() == FALSE) {
        return NOT_READY;
    }
    return mDialog.remove() != 0;
}

// .text:0x4EC
int dMenuDesi00_c::stateMain() {
    if (executeClose()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x534
int dMenuDesi00_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != SLOT_NONE && prevCursor != mCursor && !isTab(mCursor)) {
        playSeCursor();
    }
    mSelect.update();
    mSubSelect.update();
    mDialog.execute();
    calcPickup();
    return result;
}

// .text:0x5D4
int dMenuDesi00_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mDesi.draw();
    }
    if (isFlag(FLAG_DIALOG)) {
        mDialog.draw();
    }
    mSelect.draw();
    mSubSelect.draw();
    if (isFlag(FLAG_FOLLOW) || isFlag(FLAG_MOVE) || isFlag(FLAG_PLACED)) {
        mDesi.drawPickup();
    }
    return SUCCEEDED;
}

// .text:0x688
void dMenuDesi00_c::subMain() {
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, FALSE);
        mCursor = SLOT_NONE;
    }
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_DESIGN, FALSE);
    if (tab != -1 && selectTab(tab, TRUE)) {
        return;
    }
    mCursor = hitCheck(TRUE);
    if (isTab(mCursor)) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            selectTab(mCursor, FALSE);
            hideTutorial();
        }
        return;
    }
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, TRUE);
    }
    showSlotTutorial(mCursor);
    if (isDesign(mCursor)) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setSubState(SUB_GRAB_DESIGN);
            mHoldTimer = 0;
        }
    } else if (mCursor == SLOT_MII && isFlag(FLAG_MII)) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            setSubState(SUB_GRAB_MII);
            mHoldTimer = 0;
        }
    } else if (mCursor == SLOT_PLAYER) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            buildRemoveSelect();
            openSelect(mCursor, -1);
            setSubState(SUB_SELECT);
        }
    }
}

// .text:0x8BC
void dMenuDesi00_c::subGrabDesign() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        buildSelect();
        openSelect(mCursor, -1);
        setSubState(SUB_SELECT);
    } else {
        int hit = hitCheck(FALSE);
        if (hit == mCursor && mHoldTimer < dMenuMove_c::getHoldFrames()) {
            mHoldTimer++;
        } else {
            pickUpDesign(mCursor);
        }
    }
}

// .text:0x97C
void dMenuDesi00_c::subDragDesign() {
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, FALSE);
        hideTutorial();
        mCursor = SLOT_NONE;
    }
    mCursor = hitCheck(FALSE);
    if (mCursor == SLOT_MII) {
        mCursor = SLOT_NONE;
    }
    if (mCursor != SLOT_NONE) {
        if (mHeld != mCursor) {
            showSlotTutorial(mCursor);
        }
        setHighlight(mCursor, TRUE);
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        offFlag(FLAG_FOLLOW);
        dObjCursor_c::show();
        if (isDesign(mCursor)) {
            if (mHeld != mCursor) {
                buildSwapSelect();
                openSelect(mCursor, mCursor);
                setSubState(SUB_SELECT);
                onFlag(FLAG_PLACED);
                mDesi.setPickupPos(getSlotPos(mCursor, 15.0f, 15.0f));
            } else {
                dropHeld(mHeld);
                playSeItemDrop();
            }
        } else if (mCursor == SLOT_PLAYER) {
            dropHeld(mHeld);
            buildWearSelect();
            openSelect(mCursor, mHeld);
            setSubState(SUB_SELECT);
        } else {
            mDesi.startMove(getSlotPos(mHeld, 0.0f, 0.0f), FALSE);
            onFlag(FLAG_MOVE);
            setSubState(SUB_MOVE);
        }
    }
}

// .text:0xB98
void dMenuDesi00_c::subGrabMii() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        mDesi.startMiiPush();
        if (hasMiiMask()) {
            takeOffMii();
        } else {
            putOnMii();
        }
    } else {
        int hit = hitCheck(FALSE);
        if (hit == mCursor && mHoldTimer < dMenuMove_c::getHoldFrames()) {
            mHoldTimer++;
        } else {
            pickUpMii();
        }
    }
}

// .text:0xC50
void dMenuDesi00_c::subDragMii() {
    if (mCursor != SLOT_NONE) {
        setHighlight(mCursor, FALSE);
        mCursor = SLOT_NONE;
    }
    mCursor = hitCheck(FALSE);
    if (mCursor == SLOT_PLAYER) {
        setHighlight(mCursor, TRUE);
    } else {
        mCursor = SLOT_NONE;
    }
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A) ||
        !dObjCursor_c::isVisible()) {
        offFlag(FLAG_FOLLOW);
        dObjCursor_c::show();
        if (mCursor == SLOT_PLAYER) {
            dropHeld(mHeld);
            putOnMii();
        } else {
            mDesi.startMove(getSlotPos(mHeld, 0.0f, 0.0f), TRUE);
            onFlag(FLAG_MOVE);
            setSubState(SUB_MOVE);
        }
    }
}

// .text:0xD7C
void dMenuDesi00_c::subMove() {
    if (mDesi.calcMove()) {
        dropHeld(mHeld);
        playSeItemDrop();
        offFlag(FLAG_MOVE);
    }
}

// .text:0xDD0
void dMenuDesi00_c::subSelect() {
    if (mSelect.isMode(&dSelect::BComp_c::executeLock)) {
        if (!buildSubSelect()) {
            return;
        }
        openSubSelect();
        setSubState(SUB_SUB_SELECT);
    }
    if (static_cast<dSelect::BaseComp_c &>(mSelect).isClosed()) {
        mAnswer = mSelect.mCode;
        if (mAnswer != ANSWER_QUIT) {
            hideTutorial();
        }
        switch (mAnswer) {
        default:
            endSub();
            break;
        case ANSWER_CANCEL:
            dropHeld(mHeld);
            break;
        case ANSWER_EDIT:
            editDesign();
            break;
        case ANSWER_DISPLAY:
            displayDesign();
            break;
        case ANSWER_CLOTHES:
            wearDesign(WEAR_SHIRT);
            break;
        case ANSWER_HAT:
            wearDesign(WEAR_HAT);
            break;
        case ANSWER_UMBRELLA:
            wearDesign(WEAR_HELD);
            break;
        case ANSWER_REMOVE_EQUIPMENT:
            removeItem(WEAR_HELD);
            break;
        case ANSWER_REMOVE_HEADGEAR:
            removeItem(WEAR_HAT);
            break;
        case ANSWER_REMOVE_ACCESSORY:
            removeItem(WEAR_ACCESSORY);
            break;
        case ANSWER_REMOVE_MII:
            takeOffMii();
            break;
        case ANSWER_COPY:
            copyHeld();
            break;
        case ANSWER_SWAP:
            dropHeld(mHeld);
            swapHeld();
            break;
        }
    }
}

// .text:0xF74
void dMenuDesi00_c::subSubSelect() {
    if (mSubSelect.isMode(&dSelect::BComp_c::executeOut)) {
        mAnswer = mSubSelect.mCode;
        if (mAnswer != ANSWER_BACK) {
            mSelect.unlock(true);
        }
        setSubState(SUB_SUB_ANSWER);
    }
}

// .text:0x1004
void dMenuDesi00_c::subSubAnswer() {
    if (static_cast<dSelect::BaseComp_c &>(mSubSelect).isClosed()) {
        mAnswer = mSubSelect.mCode;
        if (mAnswer != ANSWER_QUIT) {
            hideTutorial();
        }
        switch (mAnswer) {
        default:
            endSub();
            break;
        case ANSWER_BACK:
            mSelect.unlock(false);
            setSubState(SUB_SELECT);
            break;
        case ANSWER_FLOOR:
        case ANSWER_ROTATE_FLOOR:
        case ANSWER_WALL:
        case ANSWER_ROTATE_WALL:
            installDesign();
            break;
        case ANSWER_CLOTHES:
            wearDesign(WEAR_SHIRT);
            break;
        case ANSWER_HAT:
            wearDesign(WEAR_HAT);
            break;
        case ANSWER_UMBRELLA:
            wearDesign(WEAR_HELD);
            break;
        case ANSWER_EASEL:
            putInRoom(ROOM_EASEL);
            break;
        case ANSWER_ROOM_HAT:
            putInRoom(ROOM_HAT);
            break;
        case ANSWER_ROOM_CLOTHES:
            putInRoom(ROOM_CLOTHES);
            break;
        case ANSWER_ROOM_UMBRELLA:
            putInRoom(ROOM_UMBRELLA);
            break;
        case ANSWER_QUIT:
            endSub();
            break;
        }
    }
}

// .text:0x113C
void dMenuDesi00_c::subWaitRoom() {
    BOOL done;
    if (mAnswer == ANSWER_FLOOR || mAnswer == ANSWER_ROTATE_FLOOR) {
        done = dBgUtil::isCarpetDone();
    } else {
        done = dBgUtil::isWallpaperDone();
    }
    if (done) {
        setSubState(SUB_MAIN);
    }
}

// .text:0x11A0
void dMenuDesi00_c::subWear() {
    BOOL changed;
    switch (mWearKind) {
    case WEAR_HELD:
        changed = dPlayerMgr::changeHeldItem(&mItem);
        break;
    case WEAR_SHIRT:
        changed = dPlayerMgr::changeShirt(&mItem);
        break;
    case WEAR_HAT:
        changed = dPlayerMgr::changeHat(&mItem);
        break;
    case WEAR_ACCESSORY:
        changed = dPlayerMgr::changeAccessory(&mItem);
        break;
    default:
        setSubState(SUB_MAIN);
        return;
    }
    if (changed) {
        setSubState(SUB_WAIT_WEAR);
    }
}

// .text:0x1260
void dMenuDesi00_c::subWearOutfit() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    const dItem::Item *held =
        isFlag(FLAG_WEAR_HELD) ? &player->mEquipment.mHeld : NULL;
    const dItem::Item *hat =
        isFlag(FLAG_WEAR_HAT) ? &player->mEquipment.mHat : NULL;
    const dItem::Item *shirt =
        isFlag(FLAG_WEAR_SHIRT) ? &player->mEquipment.mShirt : NULL;
    if (dPlayerMgr::changeOutfitAndHeldItem(shirt, hat, held)) {
        setSubState(SUB_WAIT_WEAR);
    }
}

// .text:0x1334
void dMenuDesi00_c::subWaitWear() {
    if (!dPlayerMgr::isChangingEquipment()) {
        setSubState(SUB_MAIN);
        if (isFlag(FLAG_MII)) {
            mDesi.setMiiAnm(hasMiiMask());
        }
    }
}

// .text:0x13A8
void dMenuDesi00_c::subPutOnMii() {
    if (dPlayerMgr::putOnMiiMask()) {
        setSubState(SUB_WAIT_WEAR);
    }
}

// .text:0x13F4
void dMenuDesi00_c::subTakeOffMii() {
    if (dPlayerMgr::takeOffMiiMask()) {
        setSubState(SUB_WAIT_WEAR);
    }
}

// .text:0x1440
void dMenuDesi00_c::subWaitCopy() {
    if (dPatternCopy::isCopyAcked()) {
        setSubState(SUB_MAIN);
        updateWorn(mDesign - SLOT_DESIGN);
    }
}

// .text:0x149C
bool dMenuDesi00_c::closeMain() {
    dAcUitag::closeTab();
    startCloseAnm();
    return true;
}

// .text:0x14D4
bool dMenuDesi00_c::closeDrag() {
    offFlag(FLAG_FOLLOW);
    dObjCursor_c::show();
    dropHeld(mHeld);
    offFlag(FLAG_MOVE);
    return closeMain();
}

// .text:0x1528
bool dMenuDesi00_c::closeSelect() {
    bool ready = false;
    if (mSelect.isReady() && mSubSelect.isReady()) {
        ready = true;
    }
    if (!ready) {
        return false;
    }
    mSelect.close();
    mSubSelect.close();
    return closeMain();
}

// .text:0x15A8
bool dMenuDesi00_c::closeDialog() {
    if (!mDialog.isWinAnmEnd()) {
        return false;
    }
    mDialog.open(1, 0, 0);
    return closeMain();
}

// .text:0x1604
bool dMenuDesi00_c::executeClose() {
    if (!dMenuMng_c::isCloseRequested()) {
        return false;
    }
    static bool (dMenuDesi00_c::*l_close[SUB_NUM])() = {
        &dMenuDesi00_c::closeMain,
        NULL,
        &dMenuDesi00_c::closeDrag,
        NULL,
        &dMenuDesi00_c::closeDrag,
        NULL,
        &dMenuDesi00_c::closeSelect,
        &dMenuDesi00_c::closeSelect,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &dMenuDesi00_c::closeDialog,
        NULL,
        NULL,
        NULL};
    if (!l_close[mSubState]) {
        return false;
    }
    bool closed = (this->*l_close[mSubState])();
    if (closed) {
        dMenuMng_c::clearCloseRequest();
    }
    return closed;
}

// .text:0x1780
void dMenuDesi00_c::executeSubState() {
    static void (dMenuDesi00_c::*l_sub[SUB_NUM])() = {
        &dMenuDesi00_c::subMain,       &dMenuDesi00_c::subGrabDesign,
        &dMenuDesi00_c::subDragDesign, &dMenuDesi00_c::subGrabMii,
        &dMenuDesi00_c::subDragMii,    &dMenuDesi00_c::subMove,
        &dMenuDesi00_c::subSelect,     &dMenuDesi00_c::subSubSelect,
        &dMenuDesi00_c::subSubAnswer,  &dMenuDesi00_c::subWaitRoom,
        &dMenuDesi00_c::subWear,       &dMenuDesi00_c::subWearOutfit,
        &dMenuDesi00_c::subWaitWear,   &dMenuDesi00_c::subPutOnMii,
        &dMenuDesi00_c::subTakeOffMii, &dMenuDesi00_c::subWaitCopy,
        &dMenuDesi00_c::subDialog,     &dMenuDesi00_c::subDialogPush,
        &dMenuDesi00_c::subDialogEnd,  &dMenuDesi00_c::subWaitPut};
    (this->*l_sub[mSubState])();
}

// .text:0x17B8
void dMenuDesi00_c::startCloseAnm() {
    startAnm(mDesi.getLayout(), l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    hideTutorial();
}

// .text:0x1810
bool dMenuDesi00_c::selectTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startCloseAnm();
        return true;
    }
    return false;
}

// .text:0x1860
mVec2_c dMenuDesi00_c::getSlotPos(int slot, f32 x, f32 y) {
    mVec2_c pos(0.0f, 0.0f);
    if (isDesign(slot)) {
        return mDesi.getDesignPos(slot - SLOT_DESIGN, x, y);
    } else if (slot == SLOT_MII) {
        return mDesi.getFacePos(1, x, y);
    } else if (slot == SLOT_PLAYER) {
        return mDesi.getFacePos(0, x, y);
    }
    return pos;
}

// .text:0x18FC
void dMenuDesi00_c::setHighlight(int slot, BOOL on) {
    if (isDesign(slot)) {
        if (mDesi.setSelection(slot - SLOT_DESIGN, on != 0)) {
            onFlag(FLAG_SELECTED);
        }
    } else if (slot == SLOT_PLAYER) {
        mDesi.setFaceVisible(0, on);
    } else if (slot == SLOT_MII) {
        mDesi.setFaceVisible(1, on);
    }
}

// .text:0x19B8
void dMenuDesi00_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isDesign(slot)) {
        word->set(getDesign(slot - SLOT_DESIGN)->mName, 0);
    }
}

// .text:0x1A40
void dMenuDesi00_c::showSlotTutorial(int slot) {
    mVec2_c balloon;
    if (isDesign(slot)) {
        if (dObjTutorial_c::request(4, slot, 0)) {
            balloon = getSlotPos(slot, 0.0f, 0.0f);
            m2d::stringWord_c word;
            getSlotName(&word, slot);
            dObjTutorial_c::setWord(&balloon, &word, 0);
        }
    } else if (slot == SLOT_MII) {
        int msg = hasMiiMask() ? MSG_REMOVE_MII_MASK : MSG_WEAR_MII_MASK;
        if (dObjTutorial_c::request(0, msg, 0)) {
            balloon = getSlotPos(slot, 0.0f, 0.0f);
            dObjTutorial_c::setMsg(&balloon, msg, 0);
        }
    } else {
        hideTutorial();
    }
}

// .text:0x1B98
void dMenuDesi00_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x1B9C
dDesign_c *dMenuDesi00_c::getDesign(int index) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getDesign(
        (u8)index);
}

// .text:0x1BD4
int dMenuDesi00_c::getOrder(int index) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getOrder(
        (u8)index);
}

// .text:0x1C10
void dMenuDesi00_c::swapDesigns(int slotA, int slotB) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.mOrder.swap(
        (u8)(slotA - SLOT_DESIGN), (u8)(slotB - SLOT_DESIGN));
    mDesi.updateDesigns();
}

// .text:0x1C74
void dMenuDesi00_c::copyDesign(int from, int to) {
    if (dNet::isSessionActive() == TRUE) {
        int src = getOrder(from - SLOT_DESIGN);
        int dst = getOrder(to - SLOT_DESIGN);
        dPatternCopy::copy(dPatternCopy::PATTERN_MY_PLAYER, src,
                           dPatternCopy::PATTERN_MY_PLAYER, dst, TRUE);
    } else {
        mDesi.setDesign(to - SLOT_DESIGN, getDesign(from - SLOT_DESIGN));
    }
    mDesi.updateDesigns();
}

// .text:0x1D1C
void dMenuDesi00_c::openSelect(int slot, int target) {
    mVec2_c pos = getSlotPos(slot, 10.0f, -50.0f);
    if (target == -1) {
        target = slot;
    }
    mDesign = target;
    mSelect.placeAbove(pos);
    mSelect.openPointer();
}

// .text:0x1EE8
void dMenuDesi00_c::openSubSelect() {
    mVec2_c pos(0.0f, 0.0f);
    pos.x = mSelect.getLeft();
    pos.x += 36.0f;
    pos.y = mSelect.getCursorY();
    mSubSelect.place(pos);
    mSubSelect.openPointer();
}

// .text:0x20B4
void dMenuDesi00_c::endSub() {
    setSubState(SUB_MAIN);
}

// .text:0x20C8
void dMenuDesi00_c::addWearAnswers() {
    mSubSelect.addItem(MSG_HAT, ANSWER_HAT, 0, 0);
    mSubSelect.addItem(MSG_CLOTHES, ANSWER_CLOTHES, 0, 0);
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN) ||
        dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_CITY_FIELD)) {
        mSubSelect.addItem(MSG_UMBRELLA, ANSWER_UMBRELLA, 0, 0);
    }
    mSubSelect.addItem(MSG_NEVER_MIND, ANSWER_QUIT, 0, 0);
}

// .text:0x2180
void dMenuDesi00_c::addPlaceAnswers() {
    mSubSelect.addItem(MSG_EASEL, ANSWER_EASEL, 0, 0);
    mSubSelect.addItem(MSG_ROOM_HAT, ANSWER_ROOM_HAT, 0, 0);
    mSubSelect.addItem(MSG_ROOM_CLOTHES, ANSWER_ROOM_CLOTHES, 0, 0);
    mSubSelect.addItem(MSG_ROOM_UMBRELLA, ANSWER_ROOM_UMBRELLA, 0, 0);
    mSubSelect.addItem(MSG_ROOM_NEVER_MIND, ANSWER_QUIT, 0, 0);
}

// .text:0x2220
void dMenuDesi00_c::addInstallAnswers() {
    mSubSelect.addItem(MSG_PLACE_ON_FLOOR, ANSWER_FLOOR, 0, 2);
    mSubSelect.addItem(MSG_ROTATE_AND_PLACE, ANSWER_ROTATE_FLOOR, 0, 2);
    mSubSelect.addItem(MSG_HANG_ON_WALL, ANSWER_WALL, 0, 2);
    mSubSelect.addItem(MSG_ROTATE_AND_HANG, ANSWER_ROTATE_WALL, 0, 2);
    mSubSelect.addItem(MSG_INSTALL_NEVER_MIND, ANSWER_QUIT, 0, 0);
}

// .text:0x22C0
void dMenuDesi00_c::editDesign() {
    if (dNet::isSessionActive()) {
        if (dNet::getMemberNum() > 1) {
            showMessage(MSG_HOSTING, FALSE);
        } else {
            showMessage(MSG_GATE_OPEN, FALSE);
        }
    } else if (getDesign(mDesign - SLOT_DESIGN)->mIsPro) {
        askOverwritePro();
    } else {
        selectDesign();
    }
}

// .text:0x2358
void dMenuDesi00_c::displayDesign() {
    int player = dNet::getMyNo();
    mPutRequest =
        dFgobjManager::putDesign(player, getOrder(mDesign - SLOT_DESIGN));
    if (mPutRequest == -1) {
        showMessage(MSG_NO_PLACE_FOR_DESIGN, FALSE);
    } else {
        setSubState(SUB_WAIT_PUT);
        subWaitPut();
    }
}

// .text:0x23EC
void dMenuDesi00_c::wearDesign(int kind) {
    mWearKind = kind;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u8 index = mDesign - SLOT_DESIGN;
    dItem::Item current;
    switch (mWearKind) {
    case WEAR_HELD:
        current = player->mEquipment.mHeld;
        mItem = player->mOrgDesigns.getDesignItem(index, 0x14);
        break;
    case WEAR_HAT:
        current = player->mEquipment.mHat;
        mItem = player->mOrgDesigns.getDesignItem(index, 0x15);
        break;
    case WEAR_SHIRT:
        current = player->mEquipment.mShirt;
        mItem = player->mOrgDesigns.getDesignItem(index, 0x13);
        break;
    }
    if (current.isValid() && isDesignItem(&current)) {
        current = dItem::Item();
    }
    if (current.isValid()) {
        int pocket = player->findEmptyPocket(0);
        if (pocket == -1) {
            showMessage(MSG_POCKETS_FULL_DESIGNS, FALSE);
            return;
        }
        player->setPocket(&current, pocket, FALSE);
    }
    setSubState(SUB_WEAR);
}

// .text:0x2544
void dMenuDesi00_c::putInRoom(int kind) {
    if (!dSTR::isAlone()) {
        showMessage(MSG_COMPANY_ROOM, FALSE);
        return;
    }
    mWearKind = kind;
    u32 info;
    int result;
    u8 order = getOrder(mDesign - SLOT_DESIGN);
    switch (mWearKind) {
    case ROOM_UMBRELLA:
        result = dFtrm_c::putOrgUmb(&info, order);
        break;
    case ROOM_CLOTHES:
        result = dFtrm_c::putOrgCloth(&info, order);
        break;
    case ROOM_HAT:
        result = dFtrm_c::putOrgCap(&info, order);
        break;
    case ROOM_EASEL:
        result = dFtrm_c::putOrgEasel(&info, order);
        break;
    }
    switch (result) {
    case 0:
        dFtrm_c::createFtr(info);
        break;
    default:
        showMessage(MSG_NO_PLACE_FOR_DESIGN, TRUE);
        return;
    }
    setSubState(SUB_MAIN);
}

// .text:0x264C
void dMenuDesi00_c::putOnMii() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item hat = player->mEquipment.mHat;
    int hatPocket = -1;
    if (hat.isValid() && hat.getKind() != 0x15) {
        hatPocket = player->findEmptyPocket(0);
        if (hatPocket == -1) {
            showMessage(MSG_POCKETS_FULL_ACCESSORIES, FALSE);
            return;
        }
    }
    dItem::Item accessory = player->mEquipment.mAcc;
    int accessoryPocket = -1;
    if (accessory.isValid()) {
        if (hatPocket == -1) {
            accessoryPocket = player->findEmptyPocket(0);
        } else {
            accessoryPocket = player->findEmptyPocket(hatPocket + 1);
        }
        if (accessoryPocket == -1) {
            showMessage(MSG_POCKETS_FULL_ACCESSORIES, FALSE);
            return;
        }
    }
    if (hatPocket != -1) {
        player->setPocket(&hat, hatPocket, FALSE);
    }
    if (accessoryPocket != -1) {
        player->setPocket(&accessory, accessoryPocket, FALSE);
    }
    setSubState(SUB_PUT_ON_MII);
}

// .text:0x278C
void dMenuDesi00_c::removeItem(int kind) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item item;
    switch (kind) {
    case WEAR_HELD:
        item = player->mEquipment.mHeld;
        break;
    case WEAR_HAT:
        item = player->mEquipment.mHat;
        break;
    case WEAR_ACCESSORY:
        item = player->mEquipment.mAcc;
        break;
    }
    if (!isDesignItem(&item)) {
        int pocket = player->findEmptyPocket(0);
        if (pocket == -1) {
            showMessage(MSG_POCKETS_FULL_REMOVE, FALSE);
            return;
        }
        player->setPocket(&item, pocket, FALSE);
    }
    mWearKind = kind;
    mItem = dItem::Item();
    setSubState(SUB_WEAR);
}

// .text:0x2898
void dMenuDesi00_c::takeOffMii() {
    setSubState(SUB_TAKE_OFF_MII);
}

// .text:0x28AC
void dMenuDesi00_c::copyHeld() {
    askOverwrite();
}

// .text:0x28B0
void dMenuDesi00_c::swapHeld() {
    swapDesigns(mHeld, mDesign);
    setSubState(SUB_MAIN);
}

// .text:0x28FC
void dMenuDesi00_c::buildSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_WEAR, ANSWER_WEAR, 1, 0);
    if (dScene::isSceneAttr(dScene::getCurrentScene(), 5) &&
        (!dNet::isSessionActive() || dNet::isHost()) && !isTraveling()) {
        mSelect.addItem(MSG_DISPLAY_ON_GROUND, ANSWER_DISPLAY, 0, 0);
    } else if (dScene::isCurrentSceneMyHouse() &&
               getRoomFromScene(dScene::getCurrentScene()) != 3) {
        mSelect.addItem(MSG_PLACE_IN_ROOM, ANSWER_PLACE_IN_ROOM, 1, 0);
        mSelect.addItem(MSG_INSTALL_IN_ROOM, ANSWER_INSTALL, 1, 0);
    }
    mSelect.addItem(MSG_EDIT_DESIGN, ANSWER_EDIT, 0, 0);
    mSelect.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x2A3C
void dMenuDesi00_c::buildWearSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_HAT, ANSWER_HAT, 0, 0);
    mSelect.addItem(MSG_CLOTHES, ANSWER_CLOTHES, 0, 0);
    if (dScene::isSceneAttr(dScene::getCurrentScene(), 5) ||
        dScene::isSceneAttr(dScene::getCurrentScene(), 9)) {
        mSelect.addItem(MSG_UMBRELLA, ANSWER_UMBRELLA, 0, 0);
    }
    mSelect.addItem(MSG_NEVER_MIND, ANSWER_QUIT, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x2B18
void dMenuDesi00_c::buildRemoveSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    if (hasMiiMask()) {
        mSelect.addItem(MSG_REMOVE_MII, ANSWER_REMOVE_MII, 0, 0);
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->mEquipment.mHeld.isValid()) {
        mSelect.addItem(MSG_REMOVE_EQUIPMENT, ANSWER_REMOVE_EQUIPMENT, 0, 0);
    }
    if (player->mEquipment.mHat.isValid()) {
        mSelect.addItem(MSG_REMOVE_HEADGEAR, ANSWER_REMOVE_HEADGEAR, 0, 0);
    }
    if (player->mEquipment.mAcc.isValid()) {
        mSelect.addItem(MSG_REMOVE_ACCESSORY, ANSWER_REMOVE_ACCESSORY, 0, 0);
    }
    if (mSelect.mNum == 0) {
        mSelect.addItem(MSG_NOTHING_TO_REMOVE, ANSWER_QUIT, 0, 0);
    } else {
        mSelect.addItem(MSG_NEVER_MIND, ANSWER_QUIT, 0, 0);
    }
    mSelect.setCancelLast();
}

// .text:0x2C48
void dMenuDesi00_c::buildSwapSelect() {
    mSelect.reset();
    mSelect.mNum = 0;
    mSelect.addItem(MSG_SWAP, ANSWER_SWAP, 0, 0);
    mSelect.addItem(MSG_COPY, ANSWER_COPY, 0, 0);
    mSelect.addItem(MSG_NEVER_MIND, ANSWER_CANCEL, 0, 0);
    mSelect.setCancelLast();
}

// .text:0x2CDC
bool dMenuDesi00_c::buildSubSelect() {
    u16 answer = mSelect.mCode;
    mSubSelect.reset();
    mSubSelect.mNum = 0;
    switch (answer) {
    case ANSWER_WEAR:
        addWearAnswers();
        break;
    case ANSWER_PLACE_IN_ROOM:
        addPlaceAnswers();
        break;
    case ANSWER_INSTALL:
        addInstallAnswers();
        break;
    default:
        mSelect.unlock(true);
        return false;
    }
    mSubSelect.setCancelLast();
    return true;
}

// .text:0x2D94
void dMenuDesi00_c::calcPickup() {
    if (isFlag(FLAG_FOLLOW)) {
        mDesi.calcFollow();
        mDesi.calcPickup();
    } else if (isFlag(FLAG_MOVE)) {
        mDesi.calcPickup();
    }
    mDesi.calc();
}

// .text:0x2E04
void dMenuDesi00_c::holdCursor() {
    onFlag(FLAG_FOLLOW);
    dObjCursor_c::hold();
    hideTutorial();
}

// .text:0x2E40
void dMenuDesi00_c::pickUpDesign(int slot) {
    int index = slot - SLOT_DESIGN;
    mHeld = slot;
    mDesi.setPickupDesign(getDesign(index));
    mDesi.setDesignVisible(index, false);
    setSubState(SUB_DRAG_DESIGN);
    mDesi.startFollow(FALSE);
    holdCursor();
    playSeItemLift();
}

// .text:0x2ECC
void dMenuDesi00_c::pickUpMii() {
    mHeld = SLOT_MII;
    mDesi.setMiiVisible(false);
    setSubState(SUB_DRAG_MII);
    mDesi.startFollow(TRUE);
    holdCursor();
    playSeItemLift();
}

// .text:0x2F38
void dMenuDesi00_c::dropHeld(int slot) {
    if (isDesign(slot)) {
        mDesi.setDesignVisible(slot - SLOT_DESIGN, true);
    } else {
        mDesi.setMiiVisible(true);
    }
    offFlag(FLAG_FOLLOW);
    offFlag(FLAG_PLACED);
    setSubState(SUB_MAIN);
    hideTutorial();
}

// .text:0x2FD8
void dMenuDesi00_c::installDesign() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item item;
    switch (mAnswer) {
    case ANSWER_FLOOR:
    case ANSWER_ROTATE_FLOOR:
        item = dBgUtil::getCarpet();
        if (!dSTR::isAlone()) {
            showMessage(MSG_COMPANY_FLOOR, FALSE);
            return;
        }
        break;
    case ANSWER_WALL:
    case ANSWER_ROTATE_WALL:
        item = dBgUtil::getWallpaper();
        if (!dSTR::isAlone()) {
            showMessage(MSG_COMPANY_WALL, FALSE);
            return;
        }
        break;
    default:
        return;
    }
    if (item.isValid()) {
        int pocket = player->findEmptyPocket(0);
        if (pocket == -1) {
            showMessage(MSG_POCKETS_FULL_DESIGNS, FALSE);
            return;
        }
        player->setPocket(&item, pocket, FALSE);
    }
    int order = getOrder(mDesign - SLOT_DESIGN);
    BOOL result;
    switch (mAnswer) {
    case ANSWER_FLOOR:
        result = dBgUtil::setDesignCarpet(order, FALSE, 1, 1);
        break;
    case ANSWER_ROTATE_FLOOR:
        result = dBgUtil::setDesignCarpet(order, TRUE, 1, 1);
        break;
    case ANSWER_WALL:
        result = dBgUtil::setDesignWallpaper(order, FALSE, 1, 1);
        break;
    case ANSWER_ROTATE_WALL:
        result = dBgUtil::setDesignWallpaper(order, TRUE, 1, 1);
        break;
    default:
        endSub();
        return;
    }
    if (!result) {
        endSub();
    } else {
        setSubState(SUB_WAIT_ROOM);
    }
}

// .text:0x31B0
bool dMenuDesi00_c::hasMiiMask() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiMask != 0;
}

// .text:0x31E4
bool dMenuDesi00_c::isDesignItem(const dItem::Item *item) {
    return (u32)(item->getKind() - 0x13) <= 2;
}

// .text:0x3224
int dMenuDesi00_c::getTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return -1;
    }
    return tab;
}

// .text:0x3288
int dMenuDesi00_c::hitCheck(BOOL tabs) {
    mVec2_c pos = dObjCursor_c::getPos();
    if (tabs) {
        int tab = getTab(&pos);
        if (tab != -1) {
            return tab;
        }
    }
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    int slot = mDesi.hitDesign(mVec2_c(pos), tabs, 1.0f);
    if (slot != -1) {
        return slot + SLOT_DESIGN;
    }
    slot = mDesi.hitFace(mVec2_c(pos), tabs, 1.0f);
    if (slot != -1) {
        return slot + SLOT_PLAYER;
    }
    return slot;
}

// .text:0x3368
void dMenuDesi00_c::subDialog() {
    if (mDialog.isWinAnmEnd() && mDialog.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x33C8
void dMenuDesi00_c::subDialogPush() {
    if (mDialog.isPushEnd()) {
        mDialog.open(1, 0, 0);
        setSubState(SUB_DIALOG_END);
    }
}

// .text:0x342C
void dMenuDesi00_c::subDialogEnd() {
    if (!mDialog.isWinAnmEnd()) {
        return;
    }
    switch (mDialog.mPushedBtn) {
    case 2:
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
        break;
    case 0:
        switch (mDialogKind) {
        case DIALOG_OVERWRITE:
            dropHeld(mHeld);
            break;
        }
        setSubState(SUB_MAIN);
        offFlag(FLAG_DIALOG);
        break;
    case 1:
        switch (mDialogKind) {
        case DIALOG_EDIT:
            selectDesign();
            break;
        case DIALOG_OVERWRITE:
            dropHeld(mHeld);
            copyDesign(mHeld, mDesign);
            if (dNet::isSessionActive() == TRUE) {
                setSubState(SUB_WAIT_COPY);
            } else {
                setSubState(SUB_MAIN);
                updateWorn(mDesign - SLOT_DESIGN);
            }
            break;
        default:
            selectDesign();
            break;
        }
        break;
    }
}

// .text:0x3594
void dMenuDesi00_c::subWaitPut() {
    switch (dFgobjManager::playPutEffect(mPutRequest)) {
    case 0:
        break;
    case 1:
        setSubState(SUB_MAIN);
        dFgobjManager::clearRequest(mPutRequest);
        break;
    case 2:
        setSubState(SUB_MAIN);
        dFgobjManager::clearRequest(mPutRequest);
        break;
    default:
        return;
    }
}

// .text:0x362C
void dMenuDesi00_c::showMessage(u16 msg, BOOL cannotLeave) {
    mDialog.setBtnVisible(0, false);
    mDialog.setBtnVisible(1, false);
    mDialog.setBtn(2, MSG_OK, NULL, 0);
    if (cannotLeave) {
        playSeCanNotLeave();
    } else {
        playSeCanNot();
    }
    mDialog.open(0, 1, 0);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mDialog.setMsg(msg, NULL);
}

// .text:0x3700
void dMenuDesi00_c::askOverwritePro() {
    mDialog.setBtn(0, MSG_NO, NULL, SE_cancel_little);
    mDialog.setBtn(1, MSG_YES, NULL, 0);
    mDialog.setBtnVisible(2, false);
    mDialog.open(0, 0, 0);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mDialog.setMsg(MSG_OVERWRITE_PRO, NULL);
    mDialogKind = DIALOG_EDIT;
}

// .text:0x37B8
void dMenuDesi00_c::askOverwrite() {
    mDialog.setBtn(0, MSG_NO, NULL, SE_cancel_little);
    mDialog.setBtn(1, MSG_YES, NULL, 0);
    mDialog.setBtnVisible(2, false);
    mDialog.open(0, 0, 0);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
    mDialog.setMsg(MSG_OVERWRITE, NULL);
    mDialogKind = DIALOG_OVERWRITE;
}

// .text:0x3870
void dMenuDesi00_c::selectDesign() {
    dAcUitag::openMenuA(dAcUitag::MENU_A_EDIT_DESIGN);
    dAcUitag::hideTab();
    int index = mDesign - SLOT_DESIGN;
    dMenuMng_c::setDesignIndex(index);
    dMenuMng_c::setDesign(getDesign(index));
    startCloseAnm();
    playSeSubMenuClose();
}

// .text:0x38D8
void dMenuDesi00_c::updateWorn(int index) {
    offFlag(FLAG_WEAR_HELD);
    offFlag(FLAG_WEAR_HAT);
    offFlag(FLAG_WEAR_SHIRT);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item held = player->mOrgDesigns.getDesignItem((u8)index, 0x14);
    dItem::Item hat = player->mOrgDesigns.getDesignItem((u8)index, 0x15);
    dItem::Item shirt = player->mOrgDesigns.getDesignItem((u8)index, 0x13);
    if (player->mEquipment.mHeld.isSame(held)) {
        onFlag(FLAG_WEAR_HELD);
    }
    if (player->mEquipment.mHat.isSame(hat)) {
        onFlag(FLAG_WEAR_HAT);
    }
    if (player->mEquipment.mShirt.isSame(shirt)) {
        onFlag(FLAG_WEAR_SHIRT);
    }
    bool heldWorn = isFlag(FLAG_WEAR_HELD);
    bool clothesWorn = isFlag(FLAG_WEAR_SHIRT) || isFlag(FLAG_WEAR_HAT);
    if (clothesWorn) {
        setSubState(SUB_WEAR_OUTFIT);
    } else if (heldWorn) {
        mDesign = index + SLOT_DESIGN;
        wearDesign(WEAR_HELD);
    }
}

// .text:0x3A60
bool dMenuDesi00_c::isTraveling() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
        PRIVATE_FLAG0_TRAVELLING);
}
