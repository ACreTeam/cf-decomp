// The fishing tourney: standings, villager entries and results. See include/game/game/d_ev_fishing.hpp.
// .text 801511B4..80153818 (__sinit 801537F4), .data 804EF5E8..804EF638, .bss 805F2AD0..805F2E78,
// .sdata 8074B180..8074B1A8, .sbss 8074E7E8..8074E7F0, .sdata2 80750CF8..80750D98.
// The counterpart of d_bug_off.cpp; the functions are in the same order.
#include <game/game/d_ev_fishing.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_notice.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_region.hpp>
#include <game/cLib/c_math.hpp>

// 805F2AD0: the hour the villagers' catches stop getting bigger (tryNpcEntry); the other fields are unused.
struct dEvFishingHour_c {
    dEvFishingHour_c(int hour, int a, u8 flag) : mHour(hour), _4(a), _8(flag) {}

    /* 0x0 */ int mHour;
    /* 0x4 */ int _4;
    /* 0x8 */ u8 _8;
};
static dEvFishingHour_c sEndHour(18, -2, 1);

// The literals become anonymous .sdata temporaries.
static inline void setupMail(dMail_c *mail, const u16 &kind, const u8 &sender, const dPersonalID_c *to,
                             const int &paper) {
    mail->setupSystem(&kind, "MAIL_EV_Fishing", &sender, to, &paper);
}

static inline const dLandID_c *getLandID() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return &save->mLandID;
}

static inline dPrivateData_c *getTownPlayer(int idx) {
    return dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, idx);
}

static inline dAnimal_c *getTownAnimal(int idx) {
    return dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
}

// 801511B4
void dEvFishing_c::reset() {
    mCursor.reset();
    mPlayer[0].clear();
    mPlayer[1].clear();
    mAnimal[0].clear();
    mAnimal[1].clear();
    mSize[0] = 0;
    mSize[1] = 0;
    mFish = dItem::Item();
    mItem[0] = dItem::Item();
    mItem[1] = dItem::Item();
    mStart.reset();
    mEnd.reset();
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (getTownPlayer(i)->mPID.isValid()) {
            getTownPlayer(i)->clearFlag0(PRIVATE_FLAG0_FISHING_ENTERED);
        }
    }
    mLettersSent = FALSE;
    mNoticePending = FALSE;
}

// 8015128C
void dEvFishing_c::checkResult() {
    if (!mCursor.isNone()) {
        sendResultLetters();
        postResultNotice();
    }
}

// 801512D0. &getItem(0) takes the address of the returned temporary, as the target does.
void dEvFishing_c::sendResultLetters() {
    static dMail_c sMail;
    if (mLettersSent) {
        return;
    }
    if (mCursor.diffDaysFromNow(TRUE, FALSE) > 0 || dEvent::isOver(EVENT_FISHING_TOURNEY)) {
        mPlayer[0].isValid();
        BOOL sentWinner = FALSE;
        for (int i = 0; i < PLAYER_NUM; i++) {
            dTimeStamp_c stamp(mStart.getTicks());
            stamp.toDayStart();
            dTime_c date = stamp.get();
            dLetter::formatNumber(0, date.year, 4, dScript::NUM_FORMAT_PLAIN);
            dLetter::formatNumber(3, toDisplaySize(getSize(0)), 4, dScript::NUM_FORMAT_REGION);
            dSaveTown_c *town = dSaveData_c::getTown();
            dLetter::setItemName(0, &town->mFishing.getItem(0));
            dLetter::setMonthName(1, date.month);
            dLetter::setDayName(2, date.mday);
            dLetter::setLandName(3, getLandID());
            if (getTownPlayer(i)->mPID.isValid() && getTownPlayer(i)->isFlag0(PRIVATE_FLAG0_FISHING_ENTERED)) {
                if (mPlayer[0].isValid() && mPlayer[0] == getTownPlayer(i)->mPID) {
                    dPrivateData_c *player = getTownPlayer(i);
                    setupMail(&sMail, cM::rndInt(3) + 1, 11, &player->mPID, 0x14C);
                    dItem::Item present;
                    if (!mFish.isValid()) {
                        dItemSelRange_c range(3, 0x1C);
                        fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
                    } else {
                        dItemSelRange_c range(3, 0x1D);
                        fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
                    }
                    sMail.setPresent(present.mId, 0xFF);
                    if (!dPostOffice::add(&sMail)) {
                        getTownPlayer(i)->mFutureSelfLetter.copy(&sMail);
                        getTownPlayer(i)->setFlag0(PRIVATE_FLAG0_EVENT_LETTER_HELD);
                    }
                    sentWinner = TRUE;
                } else {
                    dPrivateData_c *player = getTownPlayer(i);
                    setupMail(&sMail, cM::rndInt(3) + 4, 11, &player->mPID, 0x14C);
                    if (!dPostOffice::add(&sMail)) {
                        getTownPlayer(i)->mFutureSelfLetter.copy(&sMail);
                        getTownPlayer(i)->setFlag0(PRIVATE_FLAG0_EVENT_LETTER_HELD);
                    }
                }
            }
        }
        if (mPlayer[0].isValid() && !sentWinner) {
            // The winner is from another town.
            dTimeStamp_c stamp(mStart.getTicks());
            stamp.toDayStart();
            dTime_c date = stamp.get();
            dLetter::formatNumber(0, date.year, 4, dScript::NUM_FORMAT_PLAIN);
            dLetter::formatNumber(3, toDisplaySize(getSize(0)), 4, dScript::NUM_FORMAT_REGION);
            dSaveTown_c *town = dSaveData_c::getTown();
            dLetter::setItemName(0, &town->mFishing.getItem(0));
            dLetter::setMonthName(1, date.month);
            dLetter::setDayName(2, date.mday);
            dLetter::setLandName(3, getLandID());
            setupMail(&sMail, cM::rndInt(3) + 7, 11, &mPlayer[0], 0x14C);
            dItem::Item present;
            if (!mFish.isValid()) {
                dItemSelRange_c range(3, 0x1C);
                fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
            } else {
                dItemSelRange_c range(3, 0x1D);
                fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
            }
            sMail.setPresent(present.mId, 0xFF);
            dPostOffice::holdMail(&sMail);
        }
        mLettersSent = TRUE;
    }
}

// 80151960
void dEvFishing_c::postResultNotice() {
    if (mCursor.isToday(TRUE)) {
        return;
    }
    if (!mNoticePending) {
        return;
    }
    dTimeStamp_c stamp(mCursor.getTicks());
    stamp.toDayStart();
    stamp.toDayStart();
    dTime_c date = stamp.get();
    stamp.addDays(1);
    dTime_c postDate = stamp.get();
    dLetter::formatNumber(0, date.month + 1, 2, dScript::NUM_FORMAT_PLAIN);
    dLetter::formatNumber(1, date.mday, 2, dScript::NUM_FORMAT_PLAIN);
    dLetter::formatNumber(2, toDisplaySize(getSize(0)), 4, dScript::NUM_FORMAT_PLAIN);
    dLetter::setLandName(2, &dSaveData_c::getRaw()->mFishing.getPlayer(0)->land);
    if (dSaveData_c::getRaw()->mFishing.getPlayer(0)->isValid()) {
        dLetter::setPersonalName(0, dSaveData_c::getRaw()->mFishing.getPlayer(0));
    } else {
        dLetter::setAnmPersonalName(0, dSaveData_c::getRaw()->mFishing.getAnimal(0));
    }
    dSaveData_c *save = dSaveData_c::getRaw();
    dItem::Item item = save->mFishing.getItem(0);
    dLetter::setItemName(1, &item);
    if (dSaveData_c::getRaw()->mFishing.getPlayer(0)->isValid() &&
        !dSaveData_c::getRaw()->mFishing.getPlayer(0)->isFromTown()) {
        if (!mFish.isValid()) {
            fn_800EBB24(cM::rndInt(3) + 7, "BBS_fishing", &postDate, NULL);
        } else {
            fn_800EBB24(cM::rndInt(3) + 10, "BBS_fishing", &postDate, NULL);
        }
    } else {
        if (!mFish.isValid()) {
            fn_800EBB24(cM::rndInt(3) + 1, "BBS_fishing", &postDate, NULL);
        } else {
            fn_800EBB24(cM::rndInt(3) + 4, "BBS_fishing", &postDate, NULL);
        }
    }
}

// 80151CBC
void dEvFishing_c::update() {
    catchUp();
    checkResult();
}

// 80151CF0
void dEvFishing_c::checkDay() {
    if (!mCursor.isNone() && mCursor.isToday(TRUE)) {
        dTime_c now = *dTime_c::getCurrent();
        dTime_c cursor = mCursor.get();
        // The else-return keeps the dead "b" after the inner return.
        if (!dTime_c::isSameOrAfter(now, cursor)) {
            dTime_c end = mEnd.get();
            if (dTime_c::isSameOrAfter(now, end)) {
                return;
            }
        } else {
            return;
        }
    }
    mStart.isNone(); // result unused (probably a retail assert)
    reset();
    setup();
    catchUp();
}

// 80151F88
void dEvFishing_c::setup() {
    if (dEvent::isActive(EVENT_FISHING_TOURNEY)) {
        dTime_c time;
        if (dEvent::getStartTime(EVENT_FISHING_TOURNEY, &time)) {
            mStart.set(time);
        } else {
            return;
        }
        if (dEvent::getEndTime(EVENT_FISHING_TOURNEY, &time)) {
            mEnd.set(time);
        } else {
            return;
        }
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player->mPID.isValid()) {
                player->clearFlag0(PRIVATE_FLAG0_FISHING_ENTERED);
                player->clearFlag1(PRIVATE_FLAG1_FISHING_JOINED);
                player->clearFlag1(PRIVATE_FLAG1_FISHING_TALKED_BEFORE_START);
                player->clearFlag1(PRIVATE_FLAG1_FISHING_RESULT_HEARD);
            }
        }
        mCursor.set(mStart.getTicks());
        // January, March, May and November have a tourney fish; June..September have no tourney.
        switch (mStart.get().month) {
        case 0:
        case 2:
        case 4:
        case 10:
            mFish = dItem::Item(dItem::ITEM_IDX_BITTERLING, dFishInfo::getTourneyFish(mStart.get()), FALSE);
            break;
        case 1:
        case 3:
        case 9:
        case 11:
            mFish = dItem::Item();
            break;
        default:
            mFish = dItem::Item();
            return;
        }
        // The address of the returned temporary (a named copy adds a copy).
        int idx = pickAnimal(&mCursor.get());
        dAnimal_c *animal = getTownAnimal(idx);
        tryNpcEntry(animal, mStart.get(), TRUE);
    }
}

// 801521C4
BOOL dEvFishing_c::entryPlayer(const dPersonalID_c *pid, const dItem::Item *item, int *size) {
    int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, pid);
    if (idx >= 0 && idx < PLAYER_NUM) {
        getTownPlayer(idx)->setFlag0(PRIVATE_FLAG0_FISHING_ENTERED);
    }
    *size = 0;
    if (getFish().isValid() && item->isNotSame(getFish())) {
        return FALSE;
    }
    f32 length = dFishInfo::getLength(dItem::Item_getIdxInKind(*item));
    f32 rate = 0.7f + cM::rndF(0.5f);
    // A small catch gets a second try, unless the player already leads.
    if (rate < 0.8f && !(getPlayer(0)->isValid() && *getPlayer(0) == *pid)) {
        rate = 0.7f + cM::rndF(0.5f);
    }
    f32 fsize = length * rate;
    fsize *= 3.93707;
    *size = fsize;
    if (!isNewRecord(*size)) {
        return FALSE;
    }
    pushDown();
    mPlayer[0].copy(pid);
    mAnimal[0].clear();
    mSize[0] = *size;
    mItem[0] = *item;
    return TRUE;
}

// 80152428
BOOL dEvFishing_c::advance(BOOL force) {
    if (mCursor.isNone()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    dTime_c limit = mCursor.get();
    limit.add(0, 0, 30, 0);
    if (!force && dTime_c::isSameOrAfter(now, limit) == TRUE) {
        force = TRUE;
    }
    BOOL entered = FALSE;
    dTime_c time = mCursor.get();
    time.add(0, 0, 3, 0);
    while (dTime_c::isSameOrAfter(now, time) == TRUE) {
        if (force) {
            for (int i = 0; i < ANIMAL_NUM; i++) {
                if (isAnimalAvailable(i, &time, FALSE, FALSE, FALSE, FALSE)) {
                    dAnimal_c *animal = getTownAnimal(i);
                    if (!fn_800F4C08(animal, 4)) {
                        if (tryNpcEntry(animal, mCursor.get(), FALSE)) {
                            entered = TRUE;
                        }
                    }
                }
            }
        }
        mCursor.set(time);
        if (dTime_c::isSameOrAfter(time, mEnd.get()) == TRUE) {
            mCursor.set(mEnd.getTicks());
            break;
        }
        time.add(0, 0, 3, 0);
    }
    return entered;
}

// 80152950
BOOL dEvFishing_c::catchUp() {
    if (mCursor.isNone()) {
        return FALSE;
    }
    BOOL entered = FALSE;
    dTime_c now = *dTime_c::getCurrent();
    dTime_c time = mCursor.get();
    time.add(0, 0, 3, 0);
    while (dTime_c::isSameOrAfter(now, time) == TRUE) {
        for (int i = 0; i < ANIMAL_NUM; i++) {
            if (isAnimalAvailable(i, &time, TRUE, FALSE, FALSE, FALSE)) {
                dAnimal_c *animal = getTownAnimal(i);
                if (tryNpcEntry(animal, mCursor.get(), FALSE)) {
                    entered = TRUE;
                }
            }
        }
        mCursor.set(time);
        if (dTime_c::isSameOrAfter(time, mEnd.get()) == TRUE) {
            mCursor.set(mEnd.getTicks());
            break;
        }
        time.add(0, 0, 3, 0);
    }
    return entered;
}

// How far time is into the event, up to sEndHour (0..1).
static inline f32 getProgress(dEvFishing_c *ev, const dTime_c &time) {
    f32 total = sEndHour.mHour - ev->mStart.get().hour;
    return (time.hour - ev->mStart.get().hour) / total;
}

// 80152D10
BOOL dEvFishing_c::tryNpcEntry(dAnimal_c *animal, dTime_c time, BOOL force) {
    dAnmPersonalID_c *id = &animal->mID;
    BOOL wantsFish = FALSE;
    if (animal->mQuest.mWish.isValid() && (int)animal->mQuest.mWish.mKind == 1) {
        wantsFish = TRUE;
    }
    if (!force) {
        f32 r = cM::rnd();
        if (wantsFish) {
            if (r >= 0.3) {
                return FALSE;
            }
        } else if (r >= 0.2) {
            return FALSE;
        }
    }
    // The rarity, as a share of the entry chance above.
    f32 r = cM::rnd();
    int rarity;
    int maxRarity;
    if (wantsFish) {
        if (r < 0.01 / 0.3) {
            rarity = FISH_RARITY_3;
            maxRarity = FISH_RARITY_2;
        } else if (r < 0.05 / 0.3) {
            rarity = FISH_RARITY_2;
            maxRarity = FISH_RARITY_1;
        } else if (r < 0.15 / 0.3) {
            rarity = FISH_RARITY_1;
            maxRarity = FISH_RARITY_COMMON;
        } else {
            rarity = FISH_RARITY_COMMON;
            maxRarity = FISH_RARITY_COMMON;
        }
    } else {
        if (r < 0.01 / 0.2) {
            rarity = FISH_RARITY_3;
            maxRarity = FISH_RARITY_2;
        } else if (r < 0.03 / 0.2) {
            rarity = FISH_RARITY_2;
            maxRarity = FISH_RARITY_1;
        } else if (r < 0.08 / 0.2) {
            rarity = FISH_RARITY_1;
            maxRarity = FISH_RARITY_COMMON;
        } else {
            rarity = FISH_RARITY_COMMON;
            maxRarity = FISH_RARITY_COMMON;
        }
    }
    int type = dFishInfo::getRandomByRarity(mCursor.get(), rarity, maxRarity, FALSE);
    dItem::Item item(dItem::ITEM_IDX_BITTERLING, type, FALSE);
    // Only the tourney's fish counts; the first entry (forced) is made to catch one.
    if (getFish().isValid()) {
        if (item.isNotSame(getFish())) {
            if (force) {
                item = getFish();
                type = dItem::Item_getIdxInKind(item);
            } else {
                return FALSE;
            }
        }
    }
    f32 length = dFishInfo::getLength(type);
    f32 rate;
    if (force) {
        rate = 0.7f + cM::rndF(0.3f);
    } else {
        if (dTime_c::isSameOrAfter(time, mEnd.get()) == TRUE) {
            return FALSE;
        }
        if (!dTime_c::isSameOrAfter(time, mStart.get())) {
            return FALSE;
        }
        // The catches get bigger as the day goes on, up to sEndHour.
        if (time.hour < sEndHour.mHour) {
            double range = 0.3f + 0.2 * getProgress(this, time);
            rate = 0.7f + cM::rndF(range);
            if (rate >= 0.9f) {
                range = 0.2f + 0.2 * getProgress(this, time);
                rate = 0.8f + cM::rndF(range);
                if (rate >= 1.05f) {
                    range = 0.2f + 0.2 * getProgress(this, time);
                    rate = 0.8f + cM::rndF(range);
                }
            }
        } else {
            rate = 0.7f + cM::rndF(0.5f);
            if (rate >= 0.9f) {
                rate = 0.8f + cM::rndF(0.4f);
                if (rate >= 1.05f) {
                    rate = 0.8 + cM::rndF(0.4f);
                }
            }
        }
    }
    length *= rate;
    length *= 3.93707;
    int size = length;
    if (!force && !isNewRecord(size)) {
        return FALSE;
    }
    pushDown();
    mPlayer[0].clear();
    mAnimal[0].copy(id);
    mSize[0] = size;
    mItem[0] = item;
    return TRUE;
}

// 8015339C
BOOL dEvFishing_c::isNewRecord(int size) {
    if (size <= 0) {
        return FALSE;
    }
    return mSize[0] < size;
}

// 801533C8
dPersonalID_c *dEvFishing_c::getPlayer(int rank) {
    if (rank != 0) {
        return &mPlayer[1];
    }
    return &mPlayer[0];
}

// 801533E0
dAnmPersonalID_c *dEvFishing_c::getAnimal(int rank) {
    if (rank != 0) {
        return &mAnimal[1];
    }
    return &mAnimal[0];
}

// 801533F8
int dEvFishing_c::getSize(int rank) {
    if (rank != 0) {
        return mSize[1];
    }
    return mSize[0];
}

// 80153410
dItem::Item dEvFishing_c::getFish() {
    return mFish;
}

// 8015341C
dItem::Item dEvFishing_c::getItem(int rank) {
    if (rank != 0) {
        return mItem[1];
    }
    return mItem[0];
}

// 8015343C
f32 dEvFishing_c::toDisplaySize(f32 size) {
    if (getLanguage() == LANGUAGE_US || getLanguage() == LANGUAGE_EN) {
        f32 inches = size / 10.0;
        inches += 0.01;
        return inches;
    }
    return size / 3.93707;
}

// 801534AC: three passes, each allowing more.
int dEvFishing_c::pickAnimal(const dTime_c *time) {
    int num = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, FALSE, FALSE, FALSE, FALSE)) {
            num++;
        }
    }
    int pick = cM::rndInt(num) + 1;
    int count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, FALSE, FALSE, FALSE, FALSE)) {
            count++;
            if (count >= pick) {
                return i;
            }
        }
    }
    num = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, TRUE, FALSE, FALSE, FALSE)) {
            num++;
        }
    }
    pick = cM::rndInt(num) + 1;
    count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, TRUE, FALSE, FALSE, FALSE)) {
            count++;
            if (count >= pick) {
                return i;
            }
        }
    }
    num = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, TRUE, FALSE, FALSE, TRUE)) {
            num++;
        }
    }
    pick = cM::rndInt(num) + 1;
    count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isAnimalAvailable(i, time, TRUE, FALSE, FALSE, TRUE)) {
            count++;
            if (count >= pick) {
                return i;
            }
        }
    }
    return -1;
}

// 801536B8
BOOL dEvFishing_c::isAnimalAvailable(int idx, const dTime_c *time, BOOL anyPlace, BOOL allowSick, BOOL allowMoving,
                                     BOOL allowSleeping) {
    if ((u32)idx >= ANIMAL_NUM) {
        return FALSE;
    }
    dAnimal_c *animal = getTownAnimal(idx);
    if (animal == NULL) {
        return FALSE;
    }
    if (!animal->mID.isValid()) {
        return FALSE;
    }
    int sick = dSaveData_c::getRaw()->mAnimals.mTown.getSickAnimalIdx();
    if (!anyPlace && animal->mPlace != 0) {
        return FALSE;
    }
    if (!allowSick && idx == sick) {
        return FALSE;
    }
    if (!allowMoving && animal->isMoving()) {
        return FALSE;
    }
    if (!allowSleeping && animal->isSleepTime(time)) {
        return FALSE;
    }
    if (fn_800F4C08(animal, 4)) {
        return FALSE;
    }
    return TRUE;
}
