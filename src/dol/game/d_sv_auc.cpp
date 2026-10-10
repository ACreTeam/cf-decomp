// The auction (see include/game/game/d_sv_auc.hpp).
// .text 8010A120..8010D7E8.
#include <game/game/d_sv_auc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_mail.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>
#include <game/game/d_letter.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
// Letter words (d_letter).

BOOL fn_80177D24();                                     // WiiConnect24 off
}

// Debug tuning (RTTI "dSvAuc_hostIO_c", 805EC1B8). Local to this file: its inline dtor is in the
// main .text section (before __sinit), not in a header's -sym on section. MWCC puts the vtable pointer where the first
// virtual is declared, here after the fields.
class dSvAuc_hostIO_c {
public:
    dSvAuc_hostIO_c() {
        _00 = 0;
        _04 = 0;
        _06.clear();
        _38 = 0;
        _3C = 1000;
        _34 = 0;
        _40 = 5;
        _42 = -1;
        _44 = 5;
        _48 = dSvAuc_c::sToday;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ u8 _04;
    /* 0x06 */ dPersonalID_c _06;
    /* 0x34 */ int _34;
    /* 0x38 */ int _38;
    /* 0x3C */ int _3C;
    /* 0x40 */ s16 _40;
    /* 0x42 */ s16 _42;
    /* 0x44 */ s16 _44;
    /* 0x48 */ int _48;

    virtual ~dSvAuc_hostIO_c() {}                            // 8010D710
    /* 0x4C vtable */
}; // size 0x50

int dSvAuc_c::sToday = -1;
static u8 l_senderKind = MAIL_FROM_AUCTION;
static int l_paper = dItem::ITEM_IDX_AUCTION_PAPER;

static dSvAuc_hostIO_c l_hostIO;
static dPersonalID_c l_noBidder;

// 8010A120
BOOL dSvAuc_c::create() {
    return TRUE;
}

// 8010A128
BOOL dSvAuc_c::resetChanged() {
    getChanged()->makeAllZero();
    return TRUE;
}

// 8010A154
u32 dSvAuc_c::getDay(dTime_c time) {
    if (time.hour < 6) {
        time.add(-1, 0, 0, 0);
    }
    dTime_c next = time;
    next.add(1, 0, 0, 0);
    return dTheater::getWeek(next) >> 1 & 0x7FFF;
}

// 8010A260
u32 dSvAuc_c::getToday() {
    dTime_c now = *dTime_c::getCurrent();
    return getDay(now);
}

// 8010A300
int dSvAuc_c::getDayType(dTime_c time) {
    if (time.hour < 6) {
        time.add(-1, 0, 0, 0);
    }
    int wday = dTime_c::getWeekday(time.year, time.month, time.mday);
    int type;
    if ((dTheater::getWeek(time) & 1) == 0) {
        if (wday == 0) {
            type = 0;
        } else {
            type = 1;
            if (wday == 6) {
                type = 2;
            }
        }
    } else if (wday == 0) {
        type = 2;
    } else if (wday == 6) {
        type = 0;
    } else {
        type = 4;
        if (wday == 1) {
            type = 3;
        }
    }
    return type;
}

// 8010A418
int dSvAuc_c::getTodayType() {
    dTime_c now = *dTime_c::getCurrent();
    return getDayType(now);
}

// 8010A4B8
void dSvAuc_c::getNextOpenDate(int *year, int *month, int *day) {
    dTime_c time = *dTime_c::getCurrent();
    while (true) {
        if (getDayType(time) == 0) {
            if (time.hour < 6) {
                time.add(-1, 0, 0, 0);
            }
            *year = time.year;
            *month = time.month;
            *day = time.mday;
            return;
        }
        time.add(1, 0, 0, 0);
    }
}

// 8010A608
void dSvAuc_c::getNextCloseDate(int *year, int *month, int *day) {
    dTime_c time = *dTime_c::getCurrent();
    while (true) {
        if (getDayType(time) == 2) {
            if (time.hour < 6) {
                time.add(-1, 0, 0, 0);
            }
            *year = time.year;
            *month = time.month;
            *day = time.mday;
            return;
        }
        time.add(1, 0, 0, 0);
    }
}

// 8010A758
void dSvAucItem_c::clear() {
    mItem = dItem::ITEM_ID_NONE;
    mSeller.clear();
    mMinPrice = 1;
    for (int *bid = mBids; bid < mBids + PLAYER_NUM; bid++) {
        *bid = 0;
    }
    mSold = 0;
    mSentResults = 0;
    mHighBid = 0;
    mHighBidder.clear();
}

// 8010A7E8
void dSvAucItem_c::deletePlayer(u32 player) {
    if (player < PLAYER_NUM) {
        mBids[player & 3] = 0;
        dPrivateData_c *data = dPlayerMgr_c::getPlayerRaw(player);
        if (data != NULL) {
            const dPersonalID_c *bidder = &mHighBidder;
            const dPersonalID_c *pid = &data->mPID;
            if (*pid == *bidder) {
                l_noBidder.clear();
                mHighBid = 0;
                mHighBidder = l_noBidder;
            }
        }
    }
}

// 8010A978
void dSvAucItem_c::receive(const dSvAucItem_c *other) {
    *this = *other;
    for (int i = 0; i < PLAYER_NUM; i++) {
        mBids[i] = 0;
    }
    mDay = dSvAuc_c::getToday();
}

// 8010AC50
void dSvAucItem_c::receiveBid(const dSvAucItem_c *other) {
    if (other->mHighBid > mHighBid && other->mHighBidder.isValid()) {
        if (other->mHighBidder.land != dSaveData_c::getTown()->mLandID) {
            mHighBid = other->mHighBid;
            mHighBidder = other->mHighBidder;
        }
    }
}

// 8010ADAC
BOOL dSvAucItem_c::isValid() const {
    if (mSeller.isValid() && dItem::isRealItemId(mItem.mId)) {
        if (dItem::infoBank_c::get()->isBuiltinItemId(mItem.mId)) {
            return TRUE;
        }
        dItem::Item item = mItem;
        BOOL found = dSaveDLItemList_c::getRaw()->getSlot(&item) != -1;
        if (found) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010AE64
BOOL dSvAucItem_c::hasNoBids() const {
    for (u32 i = 0; i < PLAYER_NUM; i++) {
        if (mBids[i & 3] != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8010AEBC
BOOL dSvAucItem_c::isFromTown(const dLandID_c *land) const {
    if (mSeller.isValid()) {
        const dLandID_c *town = land != NULL ? land : &dSaveData_c::getTown()->mLandID;
        return mSeller.land == *town;
    }
    return FALSE;
}

// 8010AF6C
BOOL dSvAucItem_c::isFromOtherTown(const dLandID_c *land) const {
    if (mSeller.isValid()) {
        const dLandID_c *town = land != NULL ? land : &dSaveData_c::getTown()->mLandID;
        return mSeller.land != *town;
    }
    return FALSE;
}

// 8010B01C
void dSvAucItem_c::updateDay(const dLandID_c *land, u32 day) {
    if (isValid()) {
        if (day != mDay) {
            mDay = 0xFFFF;
        }
        if (isOver(day, land)) {
            clear();
        }
    }
}

// 8010B0A0
BOOL dSvAucItem_c::isSellerOffline(const dLandID_c *land) const {
    if (!fn_80177D24() && isFromOtherTown(land)) {
        return FALSE;
    }
    return TRUE;
}

// 8010B100
BOOL dSvAucItem_c::isHighBidderFrom(const dLandID_c *land) const {
    if (!fn_80177D24() && mHighBidder.isValid()) {
        if (mHighBidder.land != *land) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8010B1AC
void dSvAucItem_c::sendResults(const dLandID_c *land, u32 day, BOOL online) {
    if (!isValid()) {
        return;
    }
    int self = fn_801017B8();
    dItem::Item none;
    if ((mSold || day != mDay) && mSentResults != 0xF) {
        int seller = fn_801017EC(&mSeller);
        if (mSold) {
            if (isFromTown(land) && seller != 7 && !(mSentResults >> (seller & 3) & 1)) {
                BOOL other = FALSE;
                if (online && seller != self) {
                    other = TRUE;
                }
                if (!online || other) {
                    if (!isHighBidderFrom(land)) {
                        if (sendResult(seller, 7, 0, &mItem)) {
                            mSentResults |= 1 << (seller & 3);
                        }
                    } else {
                        if (sendResult(seller, 5, mHighBid, &none)) {
                            mSentResults |= 1 << (seller & 3);
                        }
                    }
                }
            }
            int winner = fn_801017EC(&mHighBidder);
            if (!online && (u32)winner < PLAYER_NUM && !(mSentResults >> (winner & 3) & 1) &&
                sendResult(winner, 1, 0, &mItem)) {
                mSentResults |= 1 << (winner & 3);
            }
            for (u32 i = 0; i < PLAYER_NUM; i++) {
                if ((winner == 7 || winner != i) && !(mSentResults >> (i & 3) & 1)) {
                    BOOL other = FALSE;
                    if (online && i != self) {
                        other = TRUE;
                    }
                    if (!online || other) {
                        if (!isSellerOffline(land)) {
                            if (sendResult(i, 4, mBids[i & 3], &none)) {
                                mSentResults |= 1 << (i & 3);
                            }
                        } else {
                            if (sendResult(i, 2, mBids[i & 3], &none)) {
                                mSentResults |= 1 << (i & 3);
                            }
                        }
                    }
                }
            }
        } else {
            if (!online && isFromTown(land) && seller != 7 && !(mSentResults >> (seller & 3) & 1) &&
                sendResult(seller, 6, 0, &mItem)) {
                mSentResults |= 1 << (seller & 3);
            }
            for (u32 i = 0; i < PLAYER_NUM; i++) {
                if (!(mSentResults >> (i & 3) & 1)) {
                    BOOL other = FALSE;
                    if (online && i != self) {
                        other = TRUE;
                    }
                    if (i != seller && (!online || other)) {
                        if (!isSellerOffline(land)) {
                            if (sendResult(i, 4, mBids[i & 3], &none)) {
                                mSentResults |= 1 << (i & 3);
                            }
                        } else {
                            if (sendResult(i, 3, mBids[i & 3], &none)) {
                                mSentResults |= 1 << (i & 3);
                            }
                        }
                    }
                }
            }
        }
    }
}

// 8010B5CC
void dSvAucItem_c::close(const dLandID_c *land, int dayType) {
    if (isValid() && !mSold && isFromTown(land) && dayType == 4 && mHighBid > 0) {
        mSold = 1;
        dSvAuc_c::sendSale(this);
    }
}

// 8010B65C
int dSvAucItem_c::getMyBid() const {
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayerRaw();
    if (current != NULL) {
        for (u32 i = 0; i < PLAYER_NUM; i++) {
            if (dPlayerMgr_c::getPlayer(i) == current) {
                return mBids[i & 3];
            }
        }
    }
    return 0;
}

// 8010B6D8
void dSvAucItem_c::placeBid(int price, dPrivateData_c *current) {
    if (current == NULL) {
        current = dPlayerMgr_c::getCurrentPlayer();
    }
    if (mSeller.isValid()) {
        const dPersonalID_c *pid = &current->mPID;
        const dPersonalID_c *seller = &mSeller;
        if (*seller == *pid) {
            return;
        }
    }
    if (price >= mMinPrice && current != NULL) {
        for (u32 i = 0; i < PLAYER_NUM; i++) {
            if (dPlayerMgr_c::getPlayer(i) == current) {
                mBids[i & 3] = price;
                if (price > mHighBid) {
                    mHighBid = price;
                    mHighBidder = current->mPID;
                    if (isFromOtherTown(NULL)) {
                        dSvAuc_c::sendBid(this);
                    }
                }
                return;
            }
        }
    }
}

// 8010B8B4
void dSvAucItem_c::bid(int price) {
    placeBid(price, NULL);
}

// 8010B8BC
BOOL dSvAucItem_c::isMyUnsold() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid() && !mSold) {
        u16 day = mDay;
        if (day != dSvAuc_c::getToday()) {
            dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayerRaw();
            if (current != NULL) {
                const dPersonalID_c *pid = &current->mPID;
                const dPersonalID_c *seller = &mSeller;
                if (*seller == *pid) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8010B9C0
BOOL dSvAucItem_c::isMyWon() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid()) {
        dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayerRaw();
        if (current != NULL && mSold) {
            const dPersonalID_c *pid = &current->mPID;
            const dPersonalID_c *bidder = &mHighBidder;
            if (*bidder == *pid) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8010BAB4
BOOL dSvAucItem_c::isOpenToday() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid() && !mSold) {
        return dSvAuc_c::getToday() == getDay() && dSvAuc_c::getTodayType() == 2;
    }
    return FALSE;
}

// 8010BB58
BOOL dSvAucItem_c::isNotToday() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid()) {
        u16 day = mDay;
        u32 today = dSvAuc_c::getToday();
        return today != day;
    }
    return TRUE;
}

// 8010BBD8
void dSvAucItem_c::take() {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid()) {
        u32 player = fn_801017B8();
        if (player < PLAYER_NUM) {
            mSentResults |= 1 << (player & 3);
            clear();
        }
    }
}

// 8010BC64
BOOL dSvAucItem_c::hasMyBid() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid()) {
        u32 player = fn_801017B8();
        if (player < PLAYER_NUM) {
            return mBids[player & 3] > 0;
        }
    }
    return FALSE;
}

// 8010BCF0
BOOL dSvAucItem_c::isOver(u32 day, const dLandID_c *land) const {
    if (mSentResults == 0xF) {
        isFromTown(land);
        return TRUE;
    }
    if (isFromOtherTown(land) && day != mDay && hasNoBids()) {
        return TRUE;
    }
    if (dItem::isRealItemId(mItem.mId) && !mSeller.isValid()) {
        return TRUE;
    }
    if (mSeller.isValid() && !dItem::isRealItemId(mItem.mId)) {
        return TRUE;
    }
    return FALSE;
}

// 8010BDF8
BOOL dSvAucItem_c::sendResult(u32 player, u16 kind, int money, const dItem::Item *present) {
    if (player < PLAYER_NUM) {
        static dMail_c sMail;
        dPrivateData_c *data = dPlayerMgr_c::getPlayer(player);
        if (data->mPID.isValid()) {
            if (money > 0 || dItem::isRealItemId(present->mId)) {
                sMail.clear();
                fn_800CBC10(0, &mSeller.land);
                fn_800CBBB0(1, &mSeller);
                if (dItem::isRealItemId(mItem.mId)) {
                    fn_800CBDA0(2, &mItem);
                }
                fn_800CBC10(3, &mHighBidder.land);
                fn_800CBBB0(4, &mHighBidder);
                fn_800CBE0C(0, mHighBid, 5, 9);
                fn_800CBE0C(1, mBids[player & 3], 5, 9);
                u16 label = kind;
                sMail.setupSystem(&label, "MAIL_ETC_Auction", &l_senderKind, &data->mPID, &l_paper);
                if (dItem::isRealItemId(present->mId)) {
                    sMail.setPresent(present->mId, 0xFF);
                }
                if (dPostOffice::deliverToPlayer(&sMail)) {
                    data->addSavings(money);
                    return TRUE;
                }
                if (dPostOffice::add(&sMail)) {
                    data->addSavings(money);
                    return TRUE;
                }
                if (money > 0) {
                    data->addSavings(money);
                    return TRUE;
                }
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 8010C038
const dPersonalID_c *dSvAucItem_c::getSeller() const {
    if (dItem::isRealItemId(mItem.mId) && mSeller.isValid()) {
        return &mSeller;
    }
    return NULL;
}

// 8010C0A4
void dSvAuc_c::clear() {
    for (dSvAucItem_c *item = mItems; item < mItems + 9; item++) {
        item->clear();
    }
    mDay = 0;
}

// Defined here: after sendResult's static letter guard in .sbss.
static u64 l_noSender;

// 8010C104
u64 *dSvAuc_c::getSender(const dSvAucItem_c *item) {
    const dPersonalID_c *pid;
    const dPersonalID_c *seller;
    for (u32 i = 0; i < 9; i++) {
        const dSvAucItem_c *own = getItemConst(i);
        if (own->isValid() && own->mItem.isSame(item->mItem)) {
            seller = &own->mSeller;
            pid = &item->mSeller;
            if (*seller == *pid) {
                return &mSenders[i];
            }
        }
    }
    l_noSender = 0;
    return &l_noSender;
}

// 8010C1F8
u64 *dSvAuc_c::findSender(const dSvAucItem_c *item) {
    return get()->getSender(item);
}

// 8010C22C
void dSvAuc_c::setSender(u32 idx, const u64 *sender) {
    if (idx >= 9) {
        return;
    }
    mSenders[idx] = *sender;
}

// 8010C250
dSvAucItem_c *dSvAuc_c::getItem(u32 idx) {
    if (idx >= 9) {
        return mItems;
    }
    return &mItems[idx];
}

// 8010C264
const dSvAucItem_c *dSvAuc_c::getItemConst(u32 idx) const {
    if (idx >= 9) {
        return mItems;
    }
    return &mItems[idx];
}

// 8010C278
int dSvAuc_c::countEmptyLarge() const {
    int num = 0;
    for (u32 i = 0; i <= 2; i++) {
        if (!dItem::isRealItemId(getItemConst(i)->mItem.mId)) {
            num++;
        }
    }
    return num;
}

// 8010C300
int dSvAuc_c::countEmptySmall() const {
    int num = 0;
    for (u32 i = 3; i <= 8; i++) {
        if (!dItem::isRealItemId(getItemConst(i)->mItem.mId)) {
            num++;
        }
    }
    return num;
}

// 8010C388
int dSvAuc_c::countEmpty() const {
    return countEmptyLarge() + countEmptySmall();
}

// 8010C3CC
int dSvAuc_c::findSlot(const dItem::Item *item) const {
    dItem::Item check = *item;
    if (dItem::isRealItemId(check.mId)) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(check);
        if (bitm != NULL) {
            if (bitm->getFtrSize() == dItem::FTR_SIZE_1x1) {
                for (u32 i = 3; i <= 8; i++) {
                    if (!dItem::isRealItemId(getItemConst(i)->mItem.mId)) {
                        return i;
                    }
                }
            } else {
                for (u32 i = 0; i <= 2; i++) {
                    if (!dItem::isRealItemId(getItemConst(i)->mItem.mId)) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

// 8010C500
BOOL dSvAuc_c::list(u32 idx, const dItem::Item *item, int price, const dPersonalID_c *seller) {
    dSvAucItem_c *slot = getItem(idx);
    slot->clear();
    slot->mItem = *item;
    slot->mMinPrice = price;
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayerRaw();
    seller = seller != NULL ? seller : &current->mPID;
    slot->mSeller = *seller;
    slot->mDay = getToday();
    if (seller->land == dSaveData_c::getTown()->mLandID) {
        sendListing(slot);
    }
    return TRUE;
}

// 8010C688
void dSvAuc_c::cleanLists(int day, BOOL unused) {
    int today;
    if (day == -1) {
        today = getToday();
    } else {
        today = day;
    }
    if (today >= 0) {
        for (u32 i = 0; i < 4; i++) {
            int listDay = dSaveData_c::getExtra()->getAucListings()->getDay(i);
            if (listDay != -1 && listDay != today) {
                dSaveData_c::getExtra()->mAucListings[(int)i].clear();
                dSaveData_c::getExtra()->getAucListings()->clear(i);
            }
        }
        for (u32 i = 0; i < 4; i++) {
            int listDay = dSaveData_c::getExtra()->getAucBids()->getDay(i);
            if (listDay != -1 && listDay != today) {
                dSaveData_c::getExtra()->mAucBids[(int)i].clear();
                dSaveData_c::getExtra()->getAucBids()->clear(i);
            }
        }
        for (u32 i = 0; i < 4; i++) {
            int listDay = dSaveData_c::getExtra()->getAucSales()->getDay(i);
            if (listDay != -1 && listDay != today) {
                dSaveData_c::getExtra()->mAucSales[(int)i].clear();
                dSaveData_c::getExtra()->getAucSales()->clear(i);
                return;
            }
        }
    }
}

// 8010C808
void dSvAuc_c::update(int dayType, u32 day, const dLandID_c *land) {
    const dLandID_c *town = land != NULL ? land : &dSaveData_c::getTown()->mLandID;
    if (dayType == 5) {
        dayType = getTodayType();
    }
    if (day == sToday) {
        day = getToday();
    }
    for (u32 i = 0; i < 9; i++) {
        getItem(i)->updateDay(town, day);
    }
    for (u32 i = 0; i < 9; i++) {
        getItem(i)->close(town, dayType);
    }
    for (u32 i = 0; i < 9; i++) {
        getItem(i)->sendResults(town, day, FALSE);
    }
    for (u32 i = 0; i < 9; i++) {
        getItem(i)->updateDay(town, day);
    }
    cleanLists(day, TRUE);
    get()->pickDLItem();
    mDay = day;
    mDayType = dayType;
}

// 8010C940
void dSvAuc_c::updateOnline() {
    const dLandID_c *land = &dSaveData_c::getTown()->mLandID;
    int day = getToday();
    cleanLists(-1, FALSE);
    for (u32 i = 0; i < 9; i++) {
        get()->getItem(i)->sendResults(land, day, TRUE);
    }
}

// 8010C9C0
void dSvAuc_c::receiveListings(dSvAucList_c *list, const u64 *sender, BOOL online) {
    int type = getTodayType();
    if (!online || type == 1 || type == 2) {
        dSvAucItem_c *item;
        for (u32 i = 0; i < 4; i++) {
            item = &list->mItems[(int)i];
            if ((list->mMask & 1 << i) && item->mSeller.isValid() && item->isValid() && !has(item)) {
                int slot = findSlot(&item->mItem);
                if (slot != -1) {
                    getItem(slot)->receive(item);
                    setSender(slot, sender);
                    getChanged()->set(1);
                } else if (addListing(item, sender)) {
                    getChanged()->set(1);
                }
            }
        }
    }
}

// 8010CAF4
BOOL dSvAuc_c::receiveBids(dSvAucList_c *list, BOOL online) {
    int type = getTodayType();
    BOOL ret = FALSE;
    if (!online || type == 2 || type == 3 || type == 4) {
        u32 day = getToday();
        for (u32 i = 0; i < 4; i++) {
            dSvAucItem_c *item = &list->mItems[(int)i];
            if ((list->mMask & 1 << i) && item->mSeller.isValid() && item->isValid()) {
                int idx = find(item);
                if (idx != -1) {
                    dSvAucItem_c *own = getItem(idx);
                    if (!item->mSold && !own->mSold && item->mHighBid > own->mHighBid) {
                        own->receiveBid(item);
                        getChanged()->set(2);
                    }
                    if (day == own->mDay) {
                        ret = TRUE;
                    }
                }
            }
        }
    }
    return ret;
}

// 8010CC1C
void dSvAuc_c::receiveSales(const dSvAucList_c *list, BOOL online) {
    getTodayType();
    if (!online || getTodayType() == 4) {
        const dSvAucItem_c *item;
        for (u32 i = 0; i < 4; i++) {
            item = &list->mItems[(int)i];
            if (item->mSeller.isValid() && item->isValid() && (list->mMask & 1 << i)) {
                int idx = find(item);
                if (idx != -1) {
                    dSvAucItem_c *own = getItem(idx);
                    if (!own->mSold && item->mSold) {
                        own->mHighBid = item->mHighBid;
                        own->mHighBidder = item->mHighBidder;
                        own->mSold = 1;
                        getChanged()->set(4);
                        fn_801017EC(&item->mHighBidder);
                    }
                }
            }
        }
    }
}

// 8010CDCC
void dSvAuc_c::deletePlayer(int player) {
    for (u32 i = 0; i < 9; i++) {
        getItem(i)->deletePlayer(player);
    }
}

// 8010CE2C
BOOL dSvAuc_c::addListing(const dSvAucItem_c *item, const u64 *sender) {
    dItem::Item check = item->mItem;
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(check);
    if (bitm != NULL) {
        int large = bitm->getFtrSize();
        u32 mask = 0;
        int num = 0;
        for (u32 i = 0; i < 9; i++) {
            dSvAucItem_c *own = getItem(i);
            if (dItem::isRealItemId(own->mItem.mId) && own->mSeller.isValid() && own->isFromOtherTown(NULL) &&
                own->hasNoBids()) {
                if (i >= 3) {
                    if (large == dItem::FTR_SIZE_1x1) {
                        num++;
                        mask |= 1 << i;
                    }
                } else if (large != dItem::FTR_SIZE_1x1) {
                    num++;
                    mask |= 1 << i;
                }
            }
        }
        if (num != 0) {
            int pick = cM::rndInt(num);
            u32 n = 0;
            for (u32 i = 0; i < 9; i++) {
                if (mask >> i & 1) {
                    if (pick == n) {
                        getItem(i)->receive(item);
                        setSender(i, sender);
                        return TRUE;
                    }
                    n++;
                }
            }
        }
    }
    return FALSE;
}

// 8010CFC8
int dSvAuc_c::find(const dSvAucItem_c *item) const {
    const dPersonalID_c *ownSeller;
    const dPersonalID_c *seller;
    if (dItem::isRealItemId(item->mItem.mId) && item->mSeller.isValid()) {
        for (u32 i = 0; i < 9; i++) {
            const dSvAucItem_c *own = getItemConst(i);
            if (item->mItem.isSame(own->mItem)) {
                ownSeller = &own->mSeller;
                seller = &item->mSeller;
                if (*seller == *ownSeller) {
                    return i;
                }
            }
        }
    }
    return -1;
}

// 8010D0D8
BOOL dSvAuc_c::has(const dSvAucItem_c *item) const {
    return find(item) != -1;
}

// 8010D108
BOOL dSvAuc_c::hasOpenToday() const {
    for (u32 i = 0; i < 9; i++) {
        if (getItemConst(i)->isOpenToday()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010D16C
BOOL dSvAuc_c::hasMyUnsold() const {
    for (u32 i = 0; i < 9; i++) {
        if (getItemConst(i)->isMyUnsold()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010D1D0
BOOL dSvAuc_c::hasMyWon() const {
    for (u32 i = 0; i < 9; i++) {
        if (getItemConst(i)->isMyWon()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010D234
BOOL dSvAuc_c::hasListedToday(const dPersonalID_c *pid) const {
    u32 day = getToday();
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayerRaw();
    pid = pid != NULL ? pid : (current != NULL ? &current->mPID : NULL);
    if (pid != NULL) {
        const dPersonalID_c *seller;
        for (u32 i = 0; i < 9; i++) {
            const dSvAucItem_c *item = getItemConst(i);
            if (dItem::isRealItemId(item->mItem.mId) && item->mSeller.isValid()) {
                seller = &item->mSeller;
                if (*seller == *pid && day == item->mDay) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8010D36C
BOOL dSvAuc_c::hasMyBidToday() const {
    u32 day = getToday();
    for (u32 i = 0; i < 9; i++) {
        const dSvAucItem_c *item = getItemConst(i);
        if (item->hasMyBid() && day == item->mDay) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010D3F8
void dSvAuc_c::sendListing(const dSvAucItem_c *item) {
    dSaveData_c::getExtra()->getAucListings()->addMine(item);
    get()->pickDLItem();
}

// 8010D43C
void dSvAuc_c::sendBid(const dSvAucItem_c *item) {
    dSaveData_c::getExtra()->getAucBids()->addMine(item);
}

// 8010D478
void dSvAuc_c::sendSale(const dSvAucItem_c *item) {
    dSaveData_c::getExtra()->getAucSales()->addSeller(item);
}

// 8010D4B4
void dSvAuc_c::pickDLItem() {
    dSaveDLItemList_c *dl = dSaveDLItemList_c::getRaw();
    u32 mask = 0;
    BOOL found;
    int num = 0;
    for (u32 i = 0; i < 9; i++) {
        const dSvAucItem_c *item = getItemConst(i);
        found = FALSE;
        if (dItem::isRealItemId(item->mItem.mId)) {
            s32 slot = dl->getSlot(&dItem::Item(item->mItem.mId));
            if (slot != -1) {
                found = TRUE;
            }
        }
        if (found) {
            num++;
            mask |= 1 << i;
        }
    }
    if (num != 0) {
        int pick = cM::rndInt(num);
        u32 n = 0;
        for (u32 i = 0; i < 9; i++) {
            if (mask >> i & 1) {
                if (pick == n) {
                    dSaveDLItem_c *found = dl->find(getItemConst(i)->mItem);
                    if (found != NULL) {
                        dSaveTown640C8_c *block = &dSaveData_c::getTown()->_0640C8;
                        block->mDLItem = *found;
                        block->mFlags |= 0x20;
                        return;
                    }
                }
                n++;
            }
        }
    }
    int pick = cM::rndInt(dl->getNum());
    u32 n = 0;
    for (u32 i = 0; i < 0x100; i++) {
        dItem::Item item = dl->getItemAt(i);
        if (item.mId != dItem::ITEM_ID_NONE) {
            if (pick == n) {
                dSaveDLItem_c *found = dl->find(dItem::Item(item.mId));
                if (found != NULL) {
                    dSaveTown640C8_c *block = &dSaveData_c::getTown()->_0640C8;
                    block->mDLItem = *found;
                    block->mFlags |= 0x20;
                    return;
                }
            }
            n++;
        }
    }
}

// 8010D6BC
dSvAuc_c *dSvAuc_c::get() {
    return dSaveData_c::getTown()->getAuction();
}

// 8010D6E4
EGG::TBitFlag<u32> *dSvAuc_c::getChanged() {
    static EGG::TBitFlag<u32> sChanged;
    return &sChanged;
}

// 8010D708
BOOL dSvAuc_c::isBusy() {
    return FALSE;
}
