#include <game/game/d_menu_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/cLib/c_counter.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_item_def.hpp>

namespace dMenuItem {

// .text:0x5C
int Window_c::load() {
    int loaded = mRes.load("Layout/item_win.arc");
    loaded &= mIconRes.load("Layout/icon.arc");
    return loaded;
}

// .text:0xB0
void Window_c::create() {
    mItemAnm.create("item_icon_00_item.brlan", &mIconRes);
    mLyt.build(&mRes, "itm_win_00.brlyt");
    mLyt.setAnm(&mMoneyAnm, "itm_win_00_mnyBtnOff.brlan", nullptr);
    mLyt.setAnm(&mScrollAnm, "itm_win_00_scrlIcon_00.brlan", nullptr);
    mScrollAnm.mFrameCtrl.mFlags = 0;
    for (int i = 0; i < POCKET_COUNT; i++) {
        mIcons[i].create(&mIconRes, &mItemAnm);
    }
    resetMoney();
    updatePockets(-1);
    reset();
    mHeldIcon.create(&mIconRes, &mItemAnm);
    mHeldIcon.setMark(false);
    mMove.stop();
}

// .text:0x1A8
void Window_c::setFaceButton(bool visible) {
    mLyt.setVisible("btnL_icn_01", !visible);
    mLyt.setVisible("btnL_icnSdw_01", !visible);
    mLyt.setVisible("btnL_fc_01", visible);
}

// .text:0x22C
void Window_c::setPlayerFace(bool visible) {
    mLyt.setVisible("playerFc", visible);
}

// .text:0x240
void Window_c::getPocketName(dScript::Word_c *word, int pocket) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    switch (player->getPocketFlag(pocket)) {
    case 1:
        dString::loadString(word, 0x5C, "sys_2D/SYS_2D_menu");
        break;
    case 4:
    case 5:
        dString::loadString(word, 0x67, "sys_2D/SYS_2D_menu");
        break;
    case 2:
    case 3:
        static_cast<const dQuestErrand_c *>(player->mErrand.get(0))
            ->getAnimal(1)
            ->setWord(&name, 10);
        dString::setWord(0, &name);
        dString::loadString(word, 0x6E, "sys_2D/SYS_2D_menu");
        break;
    default:
        word->copy(&dItem::name_c(player->mPockets[pocket]), 0);
        break;
    }
}

// .text:0x374
void Window_c::updateMoneyText() {
    mLyt.setNumberSep("T_btnL_00", mDisplayedMoney, 5,
                      dScript::NUM_FORMAT_REGION);
    mFlags |= FLAG_CALC;
}

// .text:0x3C4
void Window_c::reset() {
    setFaceButton(false);
    setPlayerFace(false);
    setMoneyButton(false);
    for (int i = 0; i < POCKET_COUNT; i++) {
        setMark(i, false);
    }
}

// .text:0x434
BOOL Window_c::remove() {
    if (mRes.remove() == FALSE) {
        return FALSE;
    }
    if (mIconRes.remove() == FALSE) {
        return FALSE;
    }
    return TRUE;
}

// .text:0x484
void Window_c::calcIcons() {
    for (int i = 0; i < POCKET_COUNT; i++) {
        mIcons[i].calcRotate();
    }
    mScrollAnm.play();
    mFlags |= FLAG_CALC;
}

// .text:0x4F0
int Window_c::positionIcon(nw4r::lyt::Pane *pane, void *arg) {
    int index = m2d::getNameNumber(pane->GetName(), 2);
    if (index >= 0 && index < POCKET_COUNT) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        ((Icon_c *)arg)[index].setRootPos(&pos);
    }
    return 0;
}

// .text:0x56C
void Window_c::calcLayouts() {
    mLyt.calc();
    mLyt.patrolPane(positionIcon, m2d::isCircleButton, mIcons);
    for (int i = 0; i < POCKET_COUNT; i++) {
        mIcons[i].calcLayouts();
    }
    mFlags &= ~FLAG_CALC;
}

// .text:0x600
void Window_c::draw() {
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    mLyt.entry();
    for (int i = 0; i < POCKET_COUNT; i++) {
        mIcons[i].drawLayouts();
    }
}

// .text:0x680
void Window_c::setMoneyButton(bool visible) {
    mFlags |= FLAG_CALC;
    if (visible) {
        mMoneyAnm.setToEnd();
    } else {
        mMoneyAnm.setFrame(0.0f);
    }
}

// .text:0x6AC
void Window_c::setMark(int pocket, bool visible) {
    Icon_c *icon = &mIcons[pocket];
    icon->setMark(visible);
    if (visible) {
        icon->calcLayouts();
    }
}

// .text:0x704
void Window_c::setNoChoice(int pocket, bool visible) {
    mIcons[pocket].setNoChoice(visible);
}

// .text:0x71C
void Window_c::setSelected(int pocket, int selected) {
    mIcons[pocket].setSelected(selected);
}

// .text:0x734
u16 Window_c::setPocket(int pocket, dItem::Item item, int flag) {
    mIcons[pocket].setItem(item, flag, -1);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u16 old = player->mPockets[pocket].mId;
    player->setPocket(&item, pocket, flag);
    return old;
}

// .text:0x7BC
void Window_c::updatePockets(int unused) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < POCKET_COUNT; i++) {
        mIcons[i].setItem(player->mPockets[i], player->getPocketFlag(i),
                          unused);
    }
}

// .text:0x840
bool Window_c::calcGift(int pocket) {
    return mIcons[pocket].calcGift();
}

// .text:0x850
mVec3_c Window_c::getPocketPos(int pocket) {
    return mIcons[pocket].getRootPos();
}

// .text:0x8A0
m2d::SimpleEx_c *Window_c::getLayout() {
    return &mLyt;
}

// .text:0x8A8
m2d::ResAccLoader_c *Window_c::getIconRes() {
    return &mIconRes;
}

// .text:0x8B0
m2d::Anm_c *Window_c::getItemAnm() {
    return &mItemAnm;
}

// .text:0x8B8
int Window_c::getMoney() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mBells;
}

// .text:0x8DC
void Window_c::resetMoney() {
    mDisplayedMoney = getMoney();
    mMoneyTimer = 0;
    updateMoneyText();
}

// .text:0x91C
void Window_c::calcMoney() {
    int money = getMoney();
    if (mMoneyTimer > 0) {
        if (--mMoneyTimer == 0) {
            mDisplayedMoney = money;
            playSe(SE_Register);
        } else {
            if (mDisplayedMoney < money) {
                mDisplayedMoney += mMoneyStep;
            } else {
                mDisplayedMoney -= mMoneyStep;
            }
            fn_8000F928(SE_Lev_MONEY_DISP_MOVE);
        }
        updateMoneyText();
    } else if (money != mDisplayedMoney) {
        int diff = mDisplayedMoney - money;
        if (money > mDisplayedMoney) {
            diff = money - mDisplayedMoney;
        }
        mMoneyTimer = MONEY_CHANGE_FRAMES;
        mMoneyStep = diff / (mMoneyTimer - 1);
        if (mMoneyStep % MONEY_STEP_MODULUS == 0) {
            mMoneyStep--;
        }
        if (mMoneyStep <= 1) {
            mMoneyTimer = 1;
        }
    }
}

// .text:0xA18
int Window_c::hitPocket(const mVec2_c *pos, bool setOffset, float scale) {
    struct Hit {
        mVec2_c pos;
        float scale;
        nw4r::lyt::Pane *hit;
    } arg;
    arg.hit = nullptr;
    arg.scale = scale;
    arg.pos = *pos;
    mLyt.patrolPane(m2d::isInPane, m2d::isCircleButton, &arg);
    if (arg.hit) {
        int index = m2d::getNameNumber(arg.hit->GetName(), 2);
        if (index == -1) {
            return -1;
        }
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

// .text:0xB10
bool Window_c::calcAnm() {
    if (!(mFlags & FLAG_ANM)) {
        return false;
    }
    if (!mWinAnm.mFrameCtrl.isStop()) {
        mWinAnm.play();
        calcLayouts();
        return true;
    }
    return false;
}

// .text:0xB78
void Window_c::startAnm(int anm) {
    static const char *windowAnmNames[ANM_COUNT] = {
        "itm_win_00_chst_gthr_In.brlan", "itm_win_00_chst_gthr_Out.brlan",
        "itm_win_00_design_In.brlan", "itm_win_00_design_Out.brlan"};
    mLyt.setAnm(&mWinAnm, windowAnmNames[anm], nullptr);
    mFlags |= FLAG_ANM;
}

// .text:0xBCC
void Window_c::stopAnm() {
    if (!(mFlags & FLAG_ANM)) {
        return;
    }
    mFlags &= ~FLAG_ANM;
    mLyt.unbind(&mWinAnm);
}

// .text:0xBF0
void Window_c::drawHeldItem() {
    mHeldIcon.drawLayouts();
}

// .text:0xBF8
void Window_c::calcHeldItem() {
    mHeldIcon.setPos(mMove.getPos());
    mHeldIcon.calcLayouts();
}

// .text:0xC3C
void Window_c::setHeldItem(const dItem::Item &item, int flag) {
    mHeldIcon.setItem(item, flag, -1);
}

// .text:0xC70
bool Window_c::isMoveEnd() {
    return mMove.isMoveEnd();
}
// .text:0xC78
bool Window_c::calcMove() {
    return mMove.calcMove();
}
// .text:0xC80
void Window_c::startMove(const mVec2_c &pos) {
    mMove.startMove(pos);
}
// .text:0xC88
void Window_c::calcFollow() {
    mMove.calcFollow();
}
// .text:0xC90
void Window_c::startFollow() {
    mMove.startFollow();
}
// .text:0xC98
void Window_c::setMoveOffset(const mVec2_c &ofs) {
    mMove.setOfs(ofs);
}

// .text:0xCA0
bool combineItems(dItem::Item &dst, dItem::Item &src) {
    if (!src.isValid() || !dst.isValid()) {
        return false;
    }
    int kind = src.getKind();
    if (kind != dst.getKind()) {
        return false;
    }
    if (src.isMoney() && dst.isMoney()) {
        int srcMoney = src.getPrice();
        int dstMoney = dst.getPrice();
        if ((srcMoney < SMALL_MONEY_LIMIT) == (dstMoney < SMALL_MONEY_LIMIT)) {
            int total = srcMoney + dstMoney;
            dItem::Item newDst(
                dItem::Item::getMoneyItem(total, FALSE, nullptr));
            dItem::Item newSrc(dItem::Item::getMoneyItem(
                total - newDst.getPrice(), FALSE, nullptr));
            if (newSrc.isSame(src)) {
                return false;
            }
            dst = newDst;
            src = newSrc;
            return true;
        }
    }
    if (kind == dItem::KIND_PAPER &&
        dItem::seeker_c::get()->findLike(src) ==
            dItem::seeker_c::get()->findLike(dst)) {
        int dstCount = (dst.mId & 3) + 1;
        int srcCount = (src.mId & 3) + 1;
        if (dstCount >= PAPER_STACK_SIZE) {
            return false;
        }
        dstCount += srcCount;
        int count = dstCount;
        if (count > PAPER_STACK_SIZE) {
            src = src.getVariant(count - (PAPER_STACK_SIZE + 1));
            count = PAPER_STACK_SIZE;
        } else {
            src = dItem::ITEM_ID_NONE;
        }
        dst = dst.getVariant(count - 1);
        return true;
    }
    if (kind == dItem::KIND_KABU) {
        int srcCount = dItem::seeker_c::get()->findLike(src) + 1;
        int dstCount = dItem::seeker_c::get()->findLike(dst) + 1;
        if (dstCount >= TURNIP_STACK_SIZE) {
            return false;
        }
        dstCount += srcCount;
        int count = dstCount;
        if (count > TURNIP_STACK_SIZE) {
            src.setFromIndex(dItem::ITEM_IDX_10_TURNIPS,
                             count - (TURNIP_STACK_SIZE + 1), FALSE);
            count = TURNIP_STACK_SIZE;
        } else {
            src = dItem::ITEM_ID_NONE;
        }
        dst.setFromIndex(dItem::ITEM_IDX_10_TURNIPS, count - 1, FALSE);
        return true;
    }
    return false;
}

// .text:0xF4C
bool needsSpecialHandling(const dItem::Item *item) {
    int kind = item->getKind();
    if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH ||
        kind == dItem::KIND_KABU || kind == dItem::KIND_BAD_KABU ||
        kind == dItem::KIND_RKABU) {
        return true;
    }
    return false;
}

// .text:0xF94
void Icon_c::setSelected(int selected) {
    static const char *baseAnmNames[BASE_ANM_COUNT] = {
        "item_icon_00_normalBase.brlan", "item_icon_00_selectBase.brlan"};
    setAnm(&mBaseAnm, baseAnmNames[selected], nullptr);
    mBaseAnm.setToEnd();
    if (selected != BASE_ANM_NORMAL) {
        mIconFlags |= FLAG_ROTATE;
        mRotateAnm.setFrame((int)(cCounter_c::m_gameFrame %
                                  (u32)(int)mRotateAnm.mFrameCtrl.mEndFrame));
    } else {
        mIconFlags &= ~FLAG_ROTATE;
    }
    calc();
}

// .text:0x1070
void Icon_c::setNoChoice(bool visible) {
    mMarkLyt.setVisible("noChoice_00", visible);
}
// .text:0x1084
void Icon_c::setMark(bool visible) {
    mMarkLyt.setVisible("icnFc", visible);
}

// .text:0x1098
void Icon_c::create(m2d::ResAccIf_c *res, m2d::Anm_c *itemAnm) {
    mpItemAnm = itemAnm;
    build(res, "item_icon_00.brlyt");
    mGiftLyt.build(res, "gift_icon_00.brlyt");
    mGiftLyt.setVisible("icnBase", false);
    mMarkLyt.build(res, "item_mark_00.brlyt");
    setAnm(&mRotateAnm, "item_icon_00_Fc_rotate.brlan", nullptr);
    mIconFlags = 0;
    mGiftType = GIFT_ANM_NONE;
    setGiftAnm(GIFT_ANM_PRESENT);
    setSelected(BASE_ANM_NORMAL);
    setNoChoice(false);
    setMark(false);
}

// .text:0x116C
void Icon_c::setPos(const mVec2_c *pos) {
    mPos.x = pos->x;
    mPos.y = pos->y;
    mGiftLyt.mPos.x = pos->x;
    mGiftLyt.mPos.y = pos->y;
    mMarkLyt.mPos.x = pos->x;
    mMarkLyt.mPos.y = pos->y;
}

// .text:0x1190
void Icon_c::calcLayouts() {
    if (mIconFlags & FLAG_GIFT) {
        mGiftLyt.calc();
    }
    calc();
    mMarkLyt.calc();
}

// .text:0x1200
void Icon_c::drawLayouts() {
    if (!(mIconFlags & FLAG_HIDDEN)) {
        entry();
        if (mIconFlags & FLAG_GIFT) {
            mGiftLyt.entry();
        }
        mMarkLyt.entry();
    }
}

// .text:0x1254
void Icon_c::calcRotate() {
    if (mIconFlags & FLAG_ROTATE) {
        mRotateAnm.play();
        calc();
    }
}

// .text:0x12A4
void Icon_c::setVisible(bool visible) {
    if (visible) {
        mIconFlags &= ~FLAG_HIDDEN;
    } else {
        mIconFlags |= FLAG_HIDDEN;
    }
}

// .text:0x12CC
void Icon_c::setGiftAnm(int gift) {
    if (gift == mGiftType) {
        mGiftAnm.setFrame(0.0f);
        mPresentAnm.setFrame(0.0f);
    } else {
        static const char *giftNames[GIFT_ANM_COUNT] = {
            "gift_icon_00_present.brlan", "gift_icon_00_present1.brlan",
            "gift_icon_00_egg.brlan"};
        mGiftLyt.setAnm(&mGiftAnm, giftNames[gift], nullptr);
        static const char *itemNames[GIFT_ANM_COUNT] = {
            "item_icon_00_present.brlan", "item_icon_00_present.brlan",
            "item_icon_00_egg.brlan"};
        setAnm(&mPresentAnm, itemNames[gift], nullptr);
    }
    mGiftType = gift;
}

// .text:0x1374
void Icon_c::setRootPos(const mVec3_c *pos) {
    getRootPane()->SetTranslate(*pos);
    mGiftLyt.getRootPane()->SetTranslate(*pos);
    mMarkLyt.getRootPane()->SetTranslate(*pos);
}

// .text:0x1404
// The target returns a reference to a converted temporary. Preserve this
// original lifetime bug.
const mVec3_c &Icon_c::getRootPos() {
    return getRootPane()->GetTranslate();
}

// .text:0x1440
void Icon_c::setItem(dItem::Item item, int flag, int unused) {
    int valid = item.mId != dItem::ITEM_ID_NONE;
    if ((flag == 6 || flag == 1 || flag == 2 || flag == 4) && valid == 1) {
        mIconFlags |= FLAG_GIFT;
        if (flag == 6) {
            setGiftAnm(GIFT_ANM_EGG);
        } else if (flag == 1) {
            setGiftAnm(GIFT_ANM_PRESENT);
        } else {
            setGiftAnm(GIFT_ANM_PRESENT_ALT);
        }
        mGiftLyt.calc();
    } else {
        mPresentAnm.setToEnd();
        mIconFlags &= ~FLAG_GIFT;
    }
    bool visible = valid != 0;
    m2d::SimpleEx_c::setVisible("icnBase", visible);
    m2d::SimpleEx_c::setVisible("icn", visible);
    static const char *backName = "txtBack_00";
    m2d::SimpleEx_c::setVisible(backName, visible);
    static const char *text0 = "TextBox_00";
    m2d::SimpleEx_c::setVisible(text0, visible);
    static const char *text1 = "TextBox_01";
    m2d::SimpleEx_c::setVisible(text1, visible);
    if (valid) {
        const dItem::BITM *info = dItem::infoBank_c::get()->getBITM(item);
        int raw = (s16)info->m_icon;
        int icon = 0;
        if ((u32)raw < ICON_PATTERN_COUNT) {
            icon = raw;
        }
        bind(mpItemAnm);
        mpItemAnm->setFrame(icon);
        calc();
        unbind(mpItemAnm);
        int kind = info->getKind();
        int count = -1;
        if (kind == dItem::KIND_MONEY) {
            if (icon >= 1 && icon <= 9) {
                count = icon;
            } else if (icon >= 10 && icon <= 108) {
                count = icon - 9;
            }
        }
        if (kind == dItem::KIND_PAPER) {
            count = (item.mId & 3) + 1;
        }
        if (kind == dItem::KIND_KABU) {
            count = dItem::seeker_c::get()->findLike(item) + 1;
        }
        if (count == -1) {
            m2d::SimpleEx_c::setVisible(backName, false);
            m2d::SimpleEx_c::setVisible(text1, false);
            m2d::SimpleEx_c::setVisible(text0, false);
        } else {
            setNumber(text0, count, 2, dScript::NUM_FORMAT_REGION);
            setNumber(text1, count, 2, dScript::NUM_FORMAT_REGION);
        }
        calc();
    }
}

// .text:0x1720
bool Icon_c::calcGift() {
    if (!(mIconFlags & FLAG_GIFT)) {
        return true;
    }
    if (mGiftAnm.mFrameCtrl.isStop() && mPresentAnm.mFrameCtrl.isStop()) {
        mIconFlags &= ~FLAG_GIFT;
        return true;
    }
    mGiftAnm.play();
    mGiftLyt.calc();
    mPresentAnm.play();
    calc();
    return false;
}

} // namespace dMenuItem
