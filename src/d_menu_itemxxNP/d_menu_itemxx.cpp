#include <game/game/d_menu_itemxx.hpp>
#include <cstring>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <revolution/SC.h>

// The argument of m2d::isInPane.
struct HitInfo {
    mVec2_c pos;
    float scale;
    nw4r::lyt::Pane *hit;
};

// .text:0x5C
dItem::Item *dMenuItemXX_c::getPockets() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mPockets;
}

#include "d_menu_itemxx_button.inc"

// .text:0x80
int dMenuItemXX_c::preCreate() {
    if (!dMenuBase_c::preCreate()) {
        return NOT_READY;
    }
    mCursor = SLOT_NONE;
    mSelectSlot = SLOT_NONE;
    return SUCCEEDED;
}

// .text:0xCC
void dMenuItemXX_c::postCreate(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        mCursor = SLOT_NONE;
    }
    dMenuBase_c::postCreate(state);
}

// .text:0xE0
void dMenuItemXX_c::postDelete(MAIN_STATE_e state) {
    dMenuBase_c::postDelete(state);
}

// .text:0xE4
int dMenuItemXX_c::stateAnm() {
    int done = calcAnm();
    onFlag(FLAG_CALC_ITEM);
    onFlag(FLAG_CALC_LETTER);
    if (isFlag(FLAG_LETTER_ANM)) {
        int letterDone = mLtrWin.calcWindowAnm();
        if (letterDone) {
            offFlag(FLAG_LETTER_ANM);
        }
        done &= letterDone;
    }
    if (done == 1) {
        changeNextState();
    }
    return done;
}

// .text:0x188
int dMenuItemXX_c::state3() {
    bool playing = mItemWin.calcAnm();
    onFlag(FLAG_CALC_ITEM);
    onFlag(FLAG_CALC_LETTER);
    if (!playing) {
        mItemWin.stopAnm();
        changeNextState();
        if (isState(STATE_CLOSE)) {
            offFlag(FLAG_DRAW_ITEM);
            offFlag(FLAG_DRAW_LETTER_BUTTON);
            offFlag(FLAG_DRAW_LETTER_WINDOW);
        }
    }
    return SUCCEEDED;
}

// .text:0x230
int dMenuItemXX_c::execute() {
    int prev = mCursor;
    if (prev != SLOT_NONE) {
        unmarkSlot(prev);
        mCursor = SLOT_NONE;
    }
    int result = dMenuBase_c::execute();
    int cur = mCursor;
    if (cur != SLOT_NONE && prev != cur && !isTab(cur)) {
        playSeCursor();
    }
    mItemWin.calcIcons();
    mSelect.update();
    mSubSelect.update();
    calcMoney();
    calcLayouts();
    return result;
}

// .text:0x2F4
int dMenuItemXX_c::draw() {
    calcLayouts();
    if (isFlag(FLAG_DRAW_ITEM)) {
        mItemWin.draw();
    }
    if (isFlag(FLAG_DRAW_LETTER_WINDOW)) {
        mLtrWin.drawWindow();
        mLtrWin.drawIcons();
    }
    if (isFlag(FLAG_DRAW_LETTER_BUTTON)) {
        mLtrWin.drawButton();
    }
    if (isFlag(FLAG_HOLD_ITEM) || isFlag(FLAG_RETURN_ITEM)) {
        mItemWin.drawHeldItem();
    }
    if (isFlag(FLAG_HOLD_LETTER) || isFlag(FLAG_RETURN_LETTER)) {
        mLtrWin.drawHeldLetter();
    }
    mSelect.draw();
    mSubSelect.draw();
    return SUCCEEDED;
}

// .text:0x3F0
bool dMenuItemXX_c::load() {
    bool loaded = true;
    loaded &= mItemWin.load();
    loaded &= mSelect.load();
    loaded &= mSubSelect.load();
    loaded &= mLtrWin.load();
    return loaded;
}

// .text:0x474
int dMenuItemXX_c::createLayouts() {
    mSelect.build();
    mSubSelect.build();
    mSubSelect.setPinkWindow();
    mItemWin.create();
    mLtrWin.create();
    return SUCCEEDED;
}

// .text:0x4C8
bool dMenuItemXX_c::remove() {
    if (mLtrWin.remove() == FALSE) {
        return false;
    }
    if (mSelect.remove() == FALSE) {
        return false;
    }
    if (mSubSelect.remove() == FALSE) {
        return false;
    }
    return mItemWin.remove();
}

// .text:0x54C
void dMenuItemXX_c::setMark(int slot, bool on) {
    if (isPocket(slot)) {
        mItemWin.setMark(slot, on);
    } else if (isLetter(slot)) {
        mLtrWin.setMark(slot - SLOT_LETTER, on);
    } else if (slot == SLOT_FACE) {
        mItemWin.setFaceButton(on);
    } else if (slot == SLOT_LETTER_BUTTON) {
        mLtrWin.setFaceButton(on);
        onFlag(FLAG_CALC_LETTER);
    } else if (slot == SLOT_MONEY) {
        mItemWin.setMoneyButton(on);
    } else if (slot == SLOT_PLAYER) {
        mItemWin.setPlayerFace(on);
    }
}

// .text:0x638
void dMenuItemXX_c::markSlot(int slot) {
    setMark(slot, true);
}

// .text:0x640
void dMenuItemXX_c::unmarkSlot(int slot) {
    setMark(slot, false);
}

// .text:0x648
int dMenuItemXX_c::hitPocket(mVec2_c pos, float scale) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    bool setOffset = !isFlag(FLAG_HOLD_ITEM);
    mVec2_c p = pos;
    int pocket = mItemWin.hitPocket(&p, setOffset, scale);
    if (pocket != -1) {
        return pocket;
    }
    return SLOT_NONE;
}

// .text:0x6E8
int dMenuItemXX_c::hitButton(mVec2_c pos, float scale) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    HitInfo arg;
    arg.hit = nullptr;
    arg.scale = scale;
    arg.pos = pos;
    mItemWin.getLayout()->patrolPane(m2d::isInPane, m2d::isRectButton, &arg);
    if (arg.hit) {
        return m2d::getNameNumber(arg.hit->GetName(), 2) + SLOT_MONEY;
    }
    mVec2_c p = pos;
    if (mLtrWin.hitButton(&p, scale) != -1) {
        return SLOT_LETTER_BUTTON;
    }
    mItemWin.getLayout()->patrolPane(m2d::isInPane, dMenuItemXX::isPlayerPane,
                                     &arg);
    if (arg.hit) {
        return SLOT_PLAYER;
    }
    return SLOT_NONE;
}

// .text:0x80C
int dMenuItemXX_c::hitLetter(mVec2_c pos, float scale) {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    bool setOffset = !isFlag(FLAG_HOLD_LETTER);
    mVec2_c p = pos;
    int letter = mLtrWin.hitLetter(&p, setOffset, scale);
    return letter == -1 ? SLOT_NONE : letter + SLOT_LETTER;
}

// .text:0x8B0
int dMenuItemXX_c::hitTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return SLOT_NONE;
    }
    return tab + SLOT_TAB;
}

// .text:0x914
bool dMenuItemXX_c::isPocketEmpty(int slot) {
    if (isPocket(slot)) {
        return getPockets()[slot].mId == dItem::ITEM_ID_NONE;
    }
    return true;
}

// .text:0x988
BOOL dMenuItemXX_c::isLetterEmpty(int slot) {
    if (isLetter(slot)) {
        return dPlayerMgr::Mgr_c::getCurrentPlayer()
            ->mLetters[slot - SLOT_LETTER]
            .isEmpty();
    }
    return TRUE;
}

// .text:0x9F0
u16 dMenuItemXX_c::getPocketItem(int slot) {
    return getPockets()[slot].mId;
}

// .text:0xA24
int dMenuItemXX_c::getPocketFlag(int slot) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->getPocketFlag(slot);
}

// .text:0xA58
dMail_c *dMenuItemXX_c::getLetter(int slot) {
    return &dPlayerMgr::Mgr_c::getCurrentPlayer()->mLetters[slot - SLOT_LETTER];
}

// .text:0xA94
void dMenuItemXX_c::setLetter(int slot, const dMail_c *mail) {
    int letter = slot - SLOT_LETTER;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setLetter(mail, letter);
    mLtrWin.setMail(letter, (dMail_c *)mail, -1);
}

// .text:0xAF8
void dMenuItemXX_c::clearLetter(int slot) {
    dMail_c mail;
    mail.clear();
    setLetter(slot, &mail);
}

// .text:0xB58
void dMenuItemXX_c::takePresent(int letter, int pocket) {
    dMail_c mail(
        dPlayerMgr::Mgr_c::getCurrentPlayer()->mLetters[letter - SLOT_LETTER]);
    u16 present = mail.getPresent();
    u8 flag = mail.get38C_hi();
    mail.setPresent(dItem::ITEM_ID_NONE, 0xFF);
    setLetter(letter, &mail);
    setPocket(pocket, present, flag);
}

// .text:0xD08
u16 dMenuItemXX_c::setSlotItem(int slot, dItem::Item item, int flag) {
    if (isPocket(slot)) {
        return setPocket(slot, item, flag);
    }
    return dItem::ITEM_ID_NONE;
}

// .text:0xD60
u16 dMenuItemXX_c::setPocket(int slot, dItem::Item item, int flag) {
    return mItemWin.setPocket(slot, item, flag);
}

// .text:0xD90
u16 dMenuItemXX_c::dropOnPocket(int slot, dItem::Item item) {
    if (getPocketFlag(slot)) {
        return setPocket(slot, item, 0);
    }
    dItem::Item dst(getPocketItem(slot));
    dItem::Item src(item);
    if (dMenuItem::combineItems(dst, src)) {
        setPocket(slot, dst, 0);
        return src.mId;
    }
    return setPocket(slot, item, 0);
}

// .text:0xE68
bool dMenuItemXX_c::addToPockets(dItem::Item item, int flag) {
    int pocket = dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0);
    if (pocket == -1) {
        return false;
    }
    setPocket(pocket, item, flag);
    return true;
}

// .text:0xEE4
void dMenuItemXX_c::pickUp() {
    int slot = mCursor;
    if (isPocket(slot)) {
        getPockets();
        mHeldFlag = getPocketFlag(slot);
        mHeldItem = setPocket(slot, dItem::ITEM_ID_NONE, 0);
        onFlag(FLAG_HOLD_ITEM);
        updateHeldItem();
        mItemWin.startFollow();
    } else if (isLetter(slot)) {
        onFlag(FLAG_HOLD_LETTER);
        mLtrWin.setHeldMail(getLetter(slot));
        clearLetter(slot);
        mLtrWin.startFollow();
    } else {
        return;
    }
    mHeldFrom = slot;
    playSeItemLift();
    dObjCursor_c::hold();
}

// .text:0xFFC
int dMenuItemXX_c::dropItem(BOOL putBack) {
    hideTutorial();
    if (isFlag(FLAG_HOLD_ITEM)) {
        offFlag(FLAG_HOLD_ITEM);
        dObjCursor_c::show();
        if (mCursor == SLOT_NONE) {
            if (putBack) {
                setSlotItem(mHeldFrom, mHeldItem, mHeldFlag);
                clearHeld();
            } else {
                onFlag(FLAG_RETURN_ITEM);
                startReturn(mHeldFrom);
            }
        } else {
            unmarkSlot(mCursor);
            int slot = mCursor;
            if (isPocket(slot)) {
                playSeItemDrop();
                return dropToPocket();
            }
            if (isLetter(slot)) {
                int result = putInLetter();
                if (result != DROP_DONE) {
                    setSlotItem(mHeldFrom, mHeldItem, mHeldFlag);
                    clearHeld();
                }
                playSeItemDrop();
                return result;
            }
            if (slot == SLOT_FACE) {
                return DROP_FACE;
            }
            if (slot == SLOT_MONEY) {
                BOOL done = depositMoney(mHeldItem.getPrice(), mHeldFrom);
                clearHeld();
                if (!done) {
                    return DROP_WALLET_FULL;
                }
            } else if (slot == SLOT_PLAYER) {
                if (isEdible(&mHeldItem)) {
                    return DROP_EAT;
                }
                if (isMedicine(&mHeldItem)) {
                    return DROP_MEDICINE;
                }
                return DROP_NONE;
            }
        }
    }
    return DROP_DONE;
}

// .text:0x11E4
int dMenuItemXX_c::dropLetter() {
    hideTutorial();
    if (!isLetter(mCursor)) {
        mCursor = SLOT_NONE;
    }
    if (isFlag(FLAG_HOLD_LETTER)) {
        offFlag(FLAG_HOLD_LETTER);
        dObjCursor_c::show();
        if (mCursor == SLOT_NONE) {
            onFlag(FLAG_RETURN_LETTER);
            startReturn(mHeldFrom);
        } else {
            dMail_c mail(*getLetter(mCursor));
            setLetter(mCursor, mLtrWin.getHeldMail());
            if (mCursor != mHeldFrom) {
                setLetter(mHeldFrom, &mail);
            }
            mLtrWin.clearHeldMail();
            playSeItemDrop();
        }
    }
    return DROP_DONE;
}

// .text:0x13E4
int dMenuItemXX_c::dropToPocket() {
    int heldFlag = mHeldFlag;
    mHeldFlag = getPocketFlag(mCursor);
    if (mHeldFlag == 0 && heldFlag == 0) {
        mHeldItem = dropOnPocket(mCursor, mHeldItem);
    } else {
        mHeldItem = setPocket(mCursor, mHeldItem, heldFlag);
    }
    if (mHeldItem.getId() != dItem::ITEM_ID_NONE) {
        setSlotItem(mHeldFrom, mHeldItem, mHeldFlag);
        clearHeld();
    }
    return DROP_DONE;
}

// .text:0x14A4
int dMenuItemXX_c::putInLetter() {
    if (isLetterEmpty(mCursor)) {
        return DROP_DONE;
    }
    if (mHeldFlag != 0) {
        return DROP_HAS_PRESENT;
    }
    int kind = mHeldItem.getKind();
    if (kind == dItem::KIND_INSECT) {
        return DROP_INSECT;
    }
    if (kind == dItem::KIND_FISH) {
        return DROP_FISH;
    }
    if (kind == dItem::KIND_KABU || kind == dItem::KIND_BAD_KABU ||
        kind == dItem::KIND_RKABU) {
        return DROP_TURNIP;
    }
    dMail_c mail(*getLetter(mCursor));
    if (mail.getPresent() != dItem::ITEM_ID_NONE) {
        return DROP_HAS_PRESENT;
    }
    mail.setPresent(mHeldItem.mId, mHeldFlag);
    setLetter(mCursor, &mail);
    return DROP_DONE;
}

// .text:0x16A0
bool dMenuItemXX_c::calcReturn() {
    if (isFlag(FLAG_RETURN_ITEM)) {
        if (mItemWin.isMoveEnd()) {
            return false;
        }
        if (mItemWin.calcMove()) {
            finishReturn();
            playSeItemDrop();
        }
        return true;
    }
    if (isFlag(FLAG_RETURN_LETTER)) {
        if (mLtrWin.isMoveEnd()) {
            return false;
        }
        if (mLtrWin.calcMove()) {
            finishReturn();
            playSeItemDrop();
            return true;
        }
    }
    return false;
}

// .text:0x1768
bool dMenuItemXX_c::isReturning() {
    if (isFlag(FLAG_RETURN_ITEM)) {
        return !mItemWin.isMoveEnd();
    }
    if (isFlag(FLAG_RETURN_LETTER)) {
        return !mLtrWin.isMoveEnd();
    }
    return false;
}

// .text:0x17E0
void dMenuItemXX_c::finishReturn() {
    if (isFlag(FLAG_RETURN_ITEM)) {
        mHeldItem = setSlotItem(mReturnTo, mHeldItem, mHeldFlag);
        clearHeld();
        offFlag(FLAG_RETURN_ITEM);
    }
    if (isFlag(FLAG_RETURN_LETTER)) {
        setLetter(mReturnTo, mLtrWin.getHeldMail());
        mLtrWin.clearHeldMail();
        offFlag(FLAG_RETURN_LETTER);
    }
}

// .text:0x188C
void dMenuItemXX_c::startReturn(int slot) {
    mReturnTo = slot;
    if (isFlag(FLAG_RETURN_ITEM)) {
        mItemWin.startMove(getSlotPos(mReturnTo, 0.0f, 0.0f));
    } else {
        mLtrWin.startMove(getSlotPos(mReturnTo, 0.0f, 0.0f));
    }
}

// .text:0x191C
void dMenuItemXX_c::closeWindow() {
    dAcUitag::closeTab();
    startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_OUT, STATE_CLOSE,
             FALSE);
    hideTutorial();
}

// .text:0x1978
void dMenuItemXX_c::closeWindowSe() {
    startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_OUT, STATE_CLOSE,
             TRUE);
    hideTutorial();
}

// .text:0x19D0
void dMenuItemXX_c::windowUp(bool keepTab) {
    startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_OUT, STATE_MAIN,
             TRUE);
    hideTutorial();
    if (!keepTab) {
        dAcUitag::hideTab();
    }
}

// .text:0x1A40
void dMenuItemXX_c::windowIn(bool keepTab) {
    startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_IN, STATE_MAIN,
             TRUE);
    if (!keepTab) {
        dAcUitag::showTab();
    }
}

// .text:0x1AA8
mVec2_c dMenuItemXX_c::getPocketPos(int slot, float dx, float dy) {
    mVec3_c pos = mItemWin.getPocketPos(slot);
    mVec2_c ret;
    ret.x = pos.x + dx;
    ret.y = pos.y + dy;
    return ret;
}

// .text:0x1B1C
mVec2_c dMenuItemXX_c::getSlotPos(int slot, float dx, float dy) {
    mVec2_c pos(0.0f, 0.0f);
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        dx *= 0.730769f;
    }
    if (isPocket(slot)) {
        return getPocketPos(slot, dx, dy);
    }
    if (slot == SLOT_PLAYER) {
        pos.x = dx;
        pos.y = 123.0f + dy;
        return pos;
    }
    if (isLetter(slot)) {
        return mLtrWin.getLetterPos(slot - SLOT_LETTER, dx, dy);
    }
    return pos;
}

// .text:0x1C68
void dMenuItemXX_c::getSlotName(dScript::Word_c *word, int slot) {
    word->clear();
    if (isPocket(slot)) {
        mItemWin.getPocketName(word, slot);
    } else if (isLetter(slot)) {
        dMail_c mail(*getLetter(slot));
        mail.getListWord(word);
    }
}

// .text:0x1E2C
void dMenuItemXX_c::showSlotTutorial(int slot, int faceMsg, BOOL tail) {
    int target = SLOT_NONE;
    int lower = 0;
    if (isPocket(slot) && !isPocketEmpty(slot)) {
        target = slot;
        if (tail && slot >= 9) {
            lower = 1;
        }
    }
    if (isLetter(slot) && !isLetterEmpty(slot)) {
        target = slot;
    }
    if (target == SLOT_NONE) {
        hideTutorial();
    } else if (dObjTutorial_c::request(1, target, FALSE)) {
        mVec2_c pos = getSlotPos(target, 0.0f, 0.0f);
        m2d::stringWord_c word;
        getSlotName(&word, target);
        dObjTutorial_c::setWord(&pos, &word, lower);
    }
    dMenuItemXX::showButtonTutorial(this, slot, faceMsg);
}

// .text:0x1F80
void dMenuItemXX_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x1F84
void dMenuItemXX_c::openSelect(int slot, BOOL pointer) {
    mVec2_c pos = getSlotPos(slot, 10.0f, -50.0f);
    mSelectSlot = slot;
    mSelect.placeAbove(pos);
    if (pointer) {
        mSelect.openPointer();
    } else {
        mSelect.openKey();
    }
}

// .text:0x22A0
void dMenuItemXX_c::openSubSelect() {
    mVec2_c pos(0.0f, 0.0f);
    pos.x = mSelect.getLeft();
    pos.x += 36.0f;
    pos.y = mSelect.getCursorY();
    mSubSelect.place(pos);
    if (mSelect.isNextMode(&dSelect::BComp_c::executeSelectDpd)) {
        mSubSelect.openPointer();
    } else {
        mSubSelect.openKey();
    }
}

// .text:0x25F4
void dMenuItemXX_c::openSelectBelow(BOOL pointer) {
    mVec2_c pos(-175.0f, 150.0f);
    mSelect.placeBelow(pos);
    if (pointer) {
        mSelect.openPointer();
    } else {
        mSelect.openKey();
    }
}

// .text:0x28F8
int dMenuItemXX_c::getMoney() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mBells;
}

// .text:0x291C
void dMenuItemXX_c::setMoney(int money) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mBells = money;
}

// .text:0x294C
bool dMenuItemXX_c::addMoney(int money) {
    int total = getMoney();
    total += money;
    if (total < 0 || total > 99999) {
        return false;
    }
    setMoney(total);
    return true;
}

// .text:0x29B0
BOOL dMenuItemXX_c::depositMoney(int money, int slot) {
    if (addMoney(money)) {
        setSlotItem(slot, dItem::ITEM_ID_NONE, 0);
    } else {
        int total = money + getMoney();
        dItem::Item rest(
            dItem::Item::getMoneyItem(total - 99999, TRUE, nullptr));
        setMoney(total - rest.getPrice());
        setSlotItem(slot, rest, 0);
        if (money == rest.getPrice()) {
            return FALSE;
        }
    }
    return TRUE;
}

// .text:0x2A98
void dMenuItemXX_c::calcMoney() {
    mItemWin.calcMoney();
}

// .text:0x2AA0
void dMenuItemXX_c::updateHeldItem() {
    mItemWin.setHeldItem(mHeldItem, mHeldFlag);
}

// .text:0x2AB0
void dMenuItemXX_c::calcLayouts() {
    if (isFlag(FLAG_CALC_ITEM)) {
        offFlag(FLAG_CALC_ITEM);
        mItemWin.calcLayouts();
        mLtrWin.setPos(mItemWin.getLayout());
    }
    if (isFlag(FLAG_CALC_LETTER)) {
        offFlag(FLAG_CALC_LETTER);
        mLtrWin.calcLayouts();
    }
    if (isFlag(FLAG_HOLD_ITEM)) {
        mItemWin.calcFollow();
        mItemWin.calcHeldItem();
    } else if (isFlag(FLAG_HOLD_LETTER)) {
        mLtrWin.calcFollow();
        mLtrWin.calcHeldLetter();
    } else if (isFlag(FLAG_RETURN_ITEM)) {
        mItemWin.calcHeldItem();
    } else if (isFlag(FLAG_RETURN_LETTER)) {
        mLtrWin.calcHeldLetter();
    }
}

// .text:0x2BC4
void dMenuItemXX_c::clearHeld() {
    mHeldItem = dItem::ITEM_ID_NONE;
    mHeldFlag = 0;
}

// .text:0x2BDC
bool dMenuItemXX_c::calcGift(int slot) {
    return mItemWin.calcGift(slot);
}

// .text:0x2BE4
bool dMenuItemXX_c::canDropPocket(int slot) {
    dItem::Item item(getPocketItem(slot));
    return canDrop(item, getPocketFlag(slot));
}

// .text:0x2C3C
// Whether the item can be dropped on the ground here.
bool dMenuItemXX_c::canDrop(const dItem::Item &item, int flag) {
    if (item.getId() == dItem::ITEM_ID_NONE) {
        return false;
    }
    if (flag != 0) {
        return false;
    }
    const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(item);
    u8 scene = dScene::getCurrentScene();
    int kind = info->getKind();
    if (dPlayerMgr::Mgr_c::getCurrentPlayerRaw()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB) &&
        (kind == dItem::KIND_SEED || kind == dItem::KIND_SEEDLING)) {
        return false;
    }
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN)) {
        if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH) {
            return false;
        }
        return kind != dItem::KIND_FLOWER;
    }
    if (dScene::isCurrentSceneMyHouse()) {
        return getRoomFromScene(dScene::getCurrentScene()) != HOME_SCENE_LOFT;
    }
    return false;
}

// .text:0x2D58
bool dMenuItemXX_c::isEdible(const dItem::Item *item) {
    return dItem::infoBank_c::get()->getBITM(*item)->getEdibility() != 0;
}

// .text:0x2DB0
BOOL dMenuItemXX_c::isMedicine(const dItem::Item *item) {
    return item->getKind() == dItem::KIND_MEDICINE;
}

// .text:0x2DE0
BOOL dMenuItemXX_c::findWater(mVec3_c *pos) {
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    dActor_c *player = (dActor_c *)dScene::getScenePlayer();
    return dBGCF::findWaterPos(pos, player->getPosP(), player->getAngle().y,
                               0x10, 2, 64.0f, 20.0f);
}

// .text:0x2E60
bool dMenuItemXX_c::canReleaseInsect(const dItem::Item *item) {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_TOWN) == FALSE) {
        return false;
    }
    return item->getKind() == dItem::KIND_INSECT;
}
