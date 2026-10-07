// Bug-Off standings, villager entries and results. See include/game/game/d_bug_off.hpp.
// .text 80113158..8011524C (__sinit 80115234), .data 804EE380..804EE398, .bss 805ED1D0..805ED570,
// .sdata 8074B018..8074B040, .sbss 8074E6F0..8074E700, .sdata2 80750AE0..80750B60.
#include <game/game/d_bug_off.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_post_office.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_800CBF34(int slot, int value, int digits, int format); // 800CBF34: number message word
void fn_800CBDA0(int slot, const dItem::Item *item);           // 800CBDA0: item name word
void fn_800CBAF0(int slot, int month);                         // 800CBAF0: month name word
void fn_800CBB50(int slot, u8 day);                            // 800CBB50: day word
void fn_800CBC10(int slot, const dLandID_c *land);             // 800CBC10: town name word
void fn_800CBBB0(int slot, const dPersonalID_c *pid);          // 800CBBB0: player name word
void fn_800CBC70(int slot, const dAnmPersonalID_c *animal);    // 800CBC70: villager name word
int fn_800C60B4(dItem::Item *out, int num, const void *table, int tableNum, const void *filter,
                const dItem::Item *exclude, int excludeNum, int); // 800C60B4: random item
extern u8 lbl_8059FF80[];
}

// C++ linkage (mangled in symbols.txt).
void fn_800EBB24(u16 kind, const char *label, const dTime_c *time, const void *arg); // 800EBB24: post a notice
BOOL fn_800F4C08(dAnimal_c *animal, int arg);                                        // 800F4C08

// {kind, sub} range for fn_800C60B4.
struct dBugOffItemRange_c {
    dBugOffItemRange_c(int kind, int sub) : mKind(kind), mSub(sub) {}

    /* 0x0 */ int mKind;
    /* 0x4 */ int mSub;
};

// 8074E6F0: constructed by the __sinit, never used.
struct dBugOffUnk_c {
    dBugOffUnk_c(dQuestEvent_e event, u8 flag) : mEvent(event), mFlag(flag) {}

    /* 0x0 */ dQuestEvent_e mEvent;
    /* 0x4 */ u8 mFlag;
};
static dBugOffUnk_c sUnk(EVENT_FISHING_TOURNEY, 1);

// The literals become anonymous .sdata temporaries.
static inline void setupMail(dMail_c *mail, const u16 &kind, const u8 &sender, const dPersonalID_c *to,
                             const int &paper) {
    mail->setupSystem(&kind, "MAIL_EV_Bug", &sender, to, &paper);
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

// 80113158
void dBugOff_c::reset() {
    mCursor.reset();
    mPlayer[0].clear();
    mPlayer[1].clear();
    mAnimal[0].clear();
    mAnimal[1].clear();
    mScore[0] = 0;
    mScore[1] = 0;
    mItem[0] = dItem::Item();
    mItem[1] = dItem::Item();
    mStart.reset();
    mEnd.reset();
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (getTownPlayer(i)->mPID.isValid()) {
            getTownPlayer(i)->clearFlag0(BUG_OFF_FLAG0_ENTERED);
        }
    }
    mLettersSent = FALSE;
    mNoticePending = FALSE;
}

// 8011322C
void dBugOff_c::checkResult() {
    if (!mCursor.isNone()) {
        sendResultLetters();
        postResultNotice();
    }
}

// The letter's words: the event date, the winning score and insect, this town.
static inline void setLetterWords(dBugOff_c *bugOff) {
    dTimeStamp_c stamp(bugOff->mStart.getTicks());
    stamp.toDayStart();
    dTime_c date = stamp.get();
    fn_800CBF34(0, date.year, 4, 0);
    fn_800CBF34(3, bugOff->getScore(0), 4, 9);
    dItem::Item item = bugOff->getItem(0);
    fn_800CBDA0(0, &item);
    fn_800CBAF0(1, date.month);
    fn_800CBB50(2, date.mday);
    fn_800CBC10(3, getLandID());
}

// 80113270. &getItem(0) takes the address of the returned temporary, as the target does.
void dBugOff_c::sendResultLetters() {
    static dMail_c sMail;
    if (mLettersSent) {
        return;
    }
    if (mCursor.diffDaysFromNow(TRUE, FALSE) > 0 || dEvent::isOver(EVENT_BUG_OFF)) {
        mPlayer[0].isValid();
        BOOL sentWinner = FALSE;
        for (int i = 0; i < PLAYER_NUM; i++) {
            dTimeStamp_c stamp(mStart.getTicks());
            stamp.toDayStart();
            dTime_c date = stamp.get();
            fn_800CBF34(0, date.year, 4, 0);
            fn_800CBF34(3, getScore(0), 4, 9);
            fn_800CBDA0(0, &getItem(0));
            fn_800CBAF0(1, date.month);
            fn_800CBB50(2, date.mday);
            fn_800CBC10(3, getLandID());
            if (getTownPlayer(i)->mPID.isValid() && getTownPlayer(i)->isFlag0(BUG_OFF_FLAG0_ENTERED)) {
                if (mPlayer[0].isValid() && mPlayer[0] == getTownPlayer(i)->mPID) {
                    dPrivateData_c *player = getTownPlayer(i);
                    setupMail(&sMail, cM::rndInt(3) + 1, 12, &player->mPID, 0x14C);
                    dItem::Item present;
                    dBugOffItemRange_c range(3, 0x1E);
                    fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
                    sMail.setPresent(present.mId, 0xFF);
                    if (dPostOffice::add(&sMail)) {
                        getTownPlayer(i)->clearFlag0(BUG_OFF_FLAG0_ENTERED);
                    } else {
                        getTownPlayer(i)->mFutureSelfLetter.copy(&sMail);
                        getTownPlayer(i)->setFlag0(BUG_OFF_FLAG0_LETTER_HELD);
                    }
                    sentWinner = TRUE;
                } else {
                    dPrivateData_c *player = getTownPlayer(i);
                    setupMail(&sMail, cM::rndInt(3) + 4, 12, &player->mPID, 0x14C);
                    if (dPostOffice::add(&sMail)) {
                        getTownPlayer(i)->clearFlag0(BUG_OFF_FLAG0_ENTERED);
                    } else {
                        getTownPlayer(i)->mFutureSelfLetter.copy(&sMail);
                        getTownPlayer(i)->setFlag0(BUG_OFF_FLAG0_LETTER_HELD);
                    }
                }
            }
        }
        if (mPlayer[0].isValid() && !sentWinner) {
            // The winner is from another town.
            dTimeStamp_c stamp(mStart.getTicks());
            stamp.toDayStart();
            dTime_c date = stamp.get();
            fn_800CBF34(0, date.year, 4, 0);
            fn_800CBF34(3, getScore(0), 4, 9);
            fn_800CBDA0(0, &getItem(0));
            fn_800CBAF0(1, date.month);
            fn_800CBB50(2, date.mday);
            fn_800CBC10(3, getLandID());
            setupMail(&sMail, cM::rndInt(3) + 7, 12, &mPlayer[0], 0x14C);
            dItem::Item present;
            dBugOffItemRange_c range(3, 0x1E);
            fn_800C60B4(&present, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
            sMail.setPresent(present.mId, 0xFF);
            dPostOffice::holdMail(&sMail);
        }
        mLettersSent = TRUE;
    }
}

// 80113834
void dBugOff_c::postResultNotice() {
    if (mCursor.isToday(TRUE)) {
        return;
    }
    if (!mNoticePending) {
        return;
    }
    dTimeStamp_c stamp(mCursor.getTicks());
    stamp.toDayStart();
    dTime_c date = stamp.get();
    stamp.addDays(1);
    dTime_c postDate = stamp.get();
    fn_800CBF34(0, date.month + 1, 2, 0);
    fn_800CBF34(1, date.mday, 2, 0);
    fn_800CBF34(2, getScore(0), 4, 0);
    fn_800CBC10(2, &dSaveData_c::getRaw()->mBugOff.getPlayer(0)->land);
    if (dSaveData_c::getRaw()->mBugOff.getPlayer(0)->isValid()) {
        fn_800CBBB0(0, dSaveData_c::getRaw()->mBugOff.getPlayer(0));
    } else {
        fn_800CBC70(0, dSaveData_c::getRaw()->mBugOff.getAnimal(0));
    }
    dItem::Item item = getItem(0);
    fn_800CBDA0(1, &item);
    if (dSaveData_c::getRaw()->mBugOff.getPlayer(0)->isValid() &&
        !dSaveData_c::getRaw()->mBugOff.getPlayer(0)->isFromTown()) {
        fn_800EBB24(cM::rndInt(3) + 4, "BBS_insect", &postDate, NULL);
    } else {
        fn_800EBB24(cM::rndInt(3) + 1, "BBS_insect", &postDate, NULL);
    }
}

// 80113AEC
void dBugOff_c::update() {
    catchUp();
    checkResult();
}

// 80113B20
void dBugOff_c::checkDay() {
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

// 80113DB8
void dBugOff_c::setup() {
    if (dEvent::isActive(EVENT_BUG_OFF)) {
        dTime_c time;
        if (dEvent::getStartTime(EVENT_BUG_OFF, &time)) {
            mStart.set(&time);
        } else {
            return;
        }
        if (dEvent::getEndTime(EVENT_BUG_OFF, &time)) {
            mEnd.set(&time);
        } else {
            return;
        }
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player->mPID.isValid()) {
                player->clearFlag0(BUG_OFF_FLAG0_ENTERED);
                player->clearFlag1(BUG_OFF_FLAG1_TALK_0);
                player->clearFlag1(BUG_OFF_FLAG1_TALK_1);
                player->clearFlag1(BUG_OFF_FLAG1_TALK_2);
                player->clearFlag1(BUG_OFF_FLAG1_TALK_3);
            }
        }
        mCursor.set(mStart.getTicks());
        // The address of the returned temporary (a named copy adds a copy).
        int idx = pickAnimal(&mCursor.get());
        dAnimal_c *animal = getTownAnimal(idx);
        tryNpcEntry(animal, mStart.get(), TRUE);
    }
}

// 80113F7C
BOOL dBugOff_c::entryPlayer(const dPersonalID_c *pid, const dItem::Item *item, int *score, int *size) {
    int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, pid);
    if (idx >= 0 && idx < PLAYER_NUM) {
        getTownPlayer(idx)->setFlag0(BUG_OFF_FLAG0_ENTERED);
    }
    if (!judge(TRUE, item, score, size)) {
        return FALSE;
    }
    pushDown();
    mPlayer[0].copy(pid);
    mAnimal[0].clear();
    mScore[0] = *score;
    mItem[0] = *item;
    return TRUE;
}

// 80114070
BOOL dBugOff_c::advance(BOOL force) {
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
        mCursor.set(&time);
        if (dTime_c::isSameOrAfter(time, mEnd.get()) == TRUE) {
            mCursor.set(mEnd.getTicks());
            break;
        }
        time.add(0, 0, 3, 0);
    }
    return entered;
}

// 80114598
BOOL dBugOff_c::catchUp() {
    if (mCursor.isNone()) {
        return FALSE;
    }
    BOOL entered = FALSE;
    dTime_c now = *dTime_c::getCurrent();
    dTime_c time = mCursor.get();
    time.add(0, 0, 3, 0);
    while (dTime_c::isSameOrAfter(now, time) == TRUE) {
        for (int i = 0; i < ANIMAL_NUM; i++) {
            if (isAnimalAvailable(i, &now, TRUE, FALSE, FALSE, FALSE)) {
                dAnimal_c *animal = getTownAnimal(i);
                if (tryNpcEntry(animal, mCursor.get(), FALSE)) {
                    entered = TRUE;
                }
            }
        }
        mCursor.set(&time);
        if (dTime_c::isSameOrAfter(time, mEnd.get()) == TRUE) {
            mCursor.set(mEnd.getTicks());
            break;
        }
        time.add(0, 0, 3, 0);
    }
    return entered;
}

// 80114930
BOOL dBugOff_c::tryNpcEntry(dAnimal_c *animal, dTime_c time, BOOL force) {
    dAnmPersonalID_c *id = &animal->mID;
    BOOL wantsBug = FALSE;
    if (animal->mQuest.mWish.isValid() && animal->mQuest.mWish.mKind == 0) {
        wantsBug = TRUE;
    }
    int rarity;
    int maxRarity;
    if (force) {
        rarity = INSECT_RARITY_1;
        maxRarity = INSECT_RARITY_COMMON;
    } else {
        f32 r = cM::rnd();
        if (wantsBug) {
            if (r >= 0.3) {
                return FALSE;
            }
        } else if (r >= 0.2) {
            return FALSE;
        }
        // The rarity, as a share of the entry chance above.
        r = cM::rnd();
        if (wantsBug) {
            if (r < 0.01 / 0.3) {
                rarity = INSECT_RARITY_3;
                maxRarity = INSECT_RARITY_3;
            } else if (r < 0.02 / 0.3) {
                rarity = INSECT_RARITY_3;
                maxRarity = INSECT_RARITY_2;
            } else if (r < 0.05 / 0.3) {
                rarity = INSECT_RARITY_2;
                maxRarity = INSECT_RARITY_1;
            } else if (r < 0.16 / 0.3) {
                rarity = INSECT_RARITY_1;
                maxRarity = INSECT_RARITY_COMMON;
            } else {
                rarity = INSECT_RARITY_COMMON;
                maxRarity = INSECT_RARITY_COMMON;
            }
        } else {
            if (r < 0.01 / 0.2) {
                rarity = INSECT_RARITY_3;
                maxRarity = INSECT_RARITY_2;
            } else if (r < 0.03 / 0.2) {
                rarity = INSECT_RARITY_2;
                maxRarity = INSECT_RARITY_1;
            } else if (r < 0.08 / 0.2) {
                rarity = INSECT_RARITY_1;
                maxRarity = INSECT_RARITY_COMMON;
            } else {
                rarity = INSECT_RARITY_COMMON;
                maxRarity = INSECT_RARITY_COMMON;
            }
        }
    }
    int type;
    if (force) {
        do {
            type = dInsectInfo::getRandomNpcCatch(mCursor.get(), rarity, maxRarity, FALSE);
        } while (type < 0);
    } else {
        type = dInsectInfo::getRandomNpcCatch(mCursor.get(), rarity, maxRarity, FALSE);
        if (type < 0) {
            return FALSE;
        }
    }
    dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, type, FALSE);
    int score = 0;
    int size = 0;
    BOOL ok = judge(FALSE, &item, &score, &size);
    if (!force && !ok) {
        return FALSE;
    }
    pushDown();
    mPlayer[0].clear();
    mAnimal[0].copy(id);
    mScore[0] = score;
    mItem[0] = item;
    return TRUE;
}

// 80114BAC
int dBugOff_c::getRank(int score) {
    if (score <= 20) {
        return 1;
    }
    if (score <= 40) {
        return 2;
    }
    if (score <= 60) {
        return 3;
    }
    if (score <= 80) {
        return 4;
    }
    return score <= 99 ? 5 : 6;
}

// 80114C04
BOOL dBugOff_c::judge(BOOL isPlayer, const dItem::Item *item, int *score, int *size) {
    int base = dInsectInfo::getBugOffScore(item);
    f32 rate = 1.0f;
    int rank = getRank(base);
    if (isPlayer) {
        switch (rank) {
        case 1:
        case 2:
        case 3:
            rate = 0.7f + cM::rndF(0.5f);
            break;
        case 4:
            rate = 0.8f + cM::rndF(0.4f);
            break;
        case 5:
            rate = 0.9f + cM::rndF(0.3f);
            break;
        case 6:
            rate = 1.0f + cM::rndF(0.2f);
            break;
        }
    } else {
        switch (rank) {
        case 1:
        case 2:
        case 3:
            rate = 0.7f + cM::rndF(0.45f);
            break;
        case 4:
        case 5:
        case 6:
            rate = 0.7f + cM::rndF(0.4f);
            break;
        }
    }
    switch (rank) {
    case 1:
    case 2:
    case 3:
        if (rate <= 0.9f) {
            *size = 0;
        } else if (rate <= 1.1f) {
            *size = 1;
        } else {
            *size = 2;
        }
        break;
    case 4:
    case 5:
        if (rate <= 1.0f) {
            *size = 0;
        } else if (rate <= 1.1f) {
            *size = 1;
        } else {
            *size = 2;
        }
        break;
    case 6:
        if (rate <= 1.1f) {
            *size = 0;
        } else if (rate <= 1.15f) {
            *size = 1;
        } else {
            *size = 2;
        }
        break;
    default:
        *size = 0;
        break;
    }
    *score = 0.5f + base * rate;
    if (*score < mScore[0]) {
        return FALSE;
    }
    if (*score == mScore[0]) {
        *score += 1;
    }
    return TRUE;
}

// 80114E84
dPersonalID_c *dBugOff_c::getPlayer(int rank) {
    if (rank != 0) {
        return &mPlayer[1];
    }
    return &mPlayer[0];
}

// 80114E9C
dAnmPersonalID_c *dBugOff_c::getAnimal(int rank) {
    if (rank != 0) {
        return &mAnimal[1];
    }
    return &mAnimal[0];
}

// 80114EB4
int dBugOff_c::getScore(int rank) {
    if (rank != 0) {
        return mScore[1];
    }
    return mScore[0];
}

// 80114ECC
dItem::Item dBugOff_c::getItem(int rank) {
    if (rank != 0) {
        return mItem[1];
    }
    return mItem[0];
}

// 80114EEC: three passes, each allowing more.
int dBugOff_c::pickAnimal(const dTime_c *time) {
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

// 801150F8
BOOL dBugOff_c::isAnimalAvailable(int idx, const dTime_c *time, BOOL anyPlace, BOOL allowSick, BOOL allowMoving,
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
