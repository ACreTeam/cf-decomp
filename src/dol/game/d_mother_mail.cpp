// Mom's letters. See include/game/game/d_mother_mail.hpp.
#include <game/game/d_mother_mail.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_paper.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_box.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_mother_mail.hpp>

namespace dMotherMail {

struct Present_s {
    int mLetter;
    int mItem; // an item index, or one of the PRESENT_* lists
};

// The mSent bits: two per letter day, then one per regular letter, then the seasonal letters.
enum {
    SENT_EVENT = 0,
    SENT_ALWAYS = DAY_NUM * 2,
    SENT_SEASON = 0x4B,
    SENT_NUM = 0x74,
};

enum {
    PRESENT_CLOTH = 0xF000,
    PRESENT_FRUIT,
    PRESENT_CANDY,
    PRESENT_FLOWER_SEEDS,
};

static u8 l_daySender = MAIL_FROM_MOM;
static u8 l_seasonSender = MAIL_FROM_MOM;
static u8 l_alwaysSender = MAIL_FROM_MOM;
static u8 l_firstSender = MAIL_FROM_MOM;

static const dQuestEvent_e l_events[MOTHER_MAIL_EVENT_NUM] = {
    EVENT_PLAYER_BIRTHDAY_0,
    EVENT_MOTHERS_DAY,
    EVENT_FATHERS_DAY,
};

static const dQuestEvent_e l_birthdays[PLAYER_NUM] = {
    EVENT_PLAYER_BIRTHDAY_0,
    EVENT_PLAYER_BIRTHDAY_1,
    EVENT_PLAYER_BIRTHDAY_2,
    EVENT_PLAYER_BIRTHDAY_3,
};

static const dQuestEvent_e l_days[DAY_NUM - DAY_NEW_YEARS_DAY] = {
    EVENT_NEW_YEARS_DAY,
    EVENT_APRIL_FOOLS_DAY,
    EVENT_TOY_DAY,
};

// 800D11C8
void update(dTime_c *last, int days) {
    if (days == 0) {
        return;
    }
    static dMail_c l_mail;
    BOOL wait;
    BOOL sent;
    int letter;
    dPrivateData_c *player;
    dSaveMotherMail_c *mail;
    bool korea = dSaveData_c::getTown()->mRegionInfo.region == CONSOLE_REGION_KR;
    int i;
    dTime_c day;
    dItem::Item paper;
    dTime_c now = *last;
    now.add(0, -6, 0, 0);
    now.hour = 6;
    now.min = now.sec = now.msec = now.usec = 0;
    for (i = 0; i < PLAYER_NUM; i++) {
        player = dPlayerMgr::Mgr_c::getPlayer(i);
        if (!player->mPID.isValid()) {
            continue;
        }
        mail = &player->mMotherMail;
        paper = dPaper::getPaper(LOOKS_TYPE_NORMAL, now.getSeason());
        if (!player->isFlag0(PRIVATE_FLAG0_MOM_FIRST_LETTER)) {
            if (player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
                continue;
            }
            if (!isDaysPassed(mail, 3, &now)) {
                continue;
            }
            if (!sendFirst(player, &paper)) {
                continue;
            }
        }
        sent = FALSE;
        for (int event = 0; event <= MOTHER_MAIL_EVENT_FATHERS_DAY; event++) {
            if (!checkEvent(mail, event, i, &now, days, &day)) {
                continue;
            }
            if (korea && event == MOTHER_MAIL_EVENT_MOTHERS_DAY) {
                letter = selectEventLetter(mail, event) + 4;
            } else {
                letter = selectEventLetter(mail, event);
            }
            paper = dPaper::getPaper(LOOKS_TYPE_NORMAL, day.getSeason());
#if VERSION == VERSION_RUUK01_01
            u8 sender = event == MOTHER_MAIL_EVENT_FATHERS_DAY ? MAIL_FROM_DAD : MAIL_FROM_MOM;
            if (korea && event == MOTHER_MAIL_EVENT_MOTHERS_DAY) {
                sender = MAIL_FROM_MOM_AND_DAD;
            }
            u16 kind = letter;
            l_mail.setupSystem(&kind, "MAIL_ETC_MotherEvent", &sender, &player->mPID, &paper);
#else
            const dPersonalID_c *pid = &player->mPID;
            u8 sender = event == MOTHER_MAIL_EVENT_FATHERS_DAY ? MAIL_FROM_DAD : MAIL_FROM_MOM;
            u16 kind = letter;
            l_mail.setupSystem(&kind, "MAIL_ETC_MotherEvent", &sender, pid, &paper);
#endif
            setPresent(&l_mail, TYPE_EVENT, letter);
            if (!dPostOffice::deliverToPlayer(&l_mail)) {
                mail->setRetry(event);
            } else {
                mail->setEventYear(event);
                mail->clearRetry(event);
                mail->mLastDate.set(&now);
                markEventLetter(mail, event, letter);
                sent = TRUE;
            }
        }
        if (sent) {
            continue;
        }
        if (player->getPostBox()->findEmpty() < 0) {
            continue;
        }
        for (int d = DAY_NEW_YEARS_DAY; d < DAY_NUM; d++) {
            if (isDay(d, &now)) {
                letter = selectEventLetter(mail, d);
                u16 kind = letter - DAY_NEW_YEARS_DAY * 2;
                l_mail.setupSystem(&kind, "MAIL_ETC_MotherDay", &l_daySender, &player->mPID, &paper);
                setPresent(&l_mail, TYPE_DAY, letter - DAY_NEW_YEARS_DAY * 2);
                if (dPostOffice::deliverToPlayer(&l_mail)) {
                    mail->mLastDate.set(&now);
                    markEventLetter(mail, d, letter);
                    sent = TRUE;
                }
                break;
            }
        }
        if (sent) {
            continue;
        }
        wait = FALSE;
        if (!mail->getLastDate().isNone()) {
            if (!isDaysPassed(mail, 7, &now)) {
                wait = TRUE;
            }
        }
        if (wait) {
            continue;
        }
        if (fgMngProc_rndF(100.0f) < 80.0f) {
            mail->mLastDate.set(&now);
            continue;
        }
        if (hasSeasonLetter(mail, now.mon)) {
            letter = selectSeasonLetter(mail, now.mon);
            u16 kind = letter;
            l_mail.setupSystem(&kind, "MAIL_ETC_MotherSeason", &l_seasonSender, &player->mPID, &paper);
            setPresent(&l_mail, TYPE_SEASON, letter);
            dPostOffice::deliverToPlayer(&l_mail);
            mail->mLastDate.set(&now);
            mail->setSent(letter + SENT_SEASON - 1);
        } else {
            letter = selectAlwaysLetter(mail);
            if (letter == 0x21) {
                dLetter::setPersonalName(0, &player->mPID);
            }
            u16 kind = letter;
            l_mail.setupSystem(&kind, "MAIL_ETC_MotherAlways", &l_alwaysSender, &player->mPID, &paper);
            setPresent(&l_mail, TYPE_ALWAYS, letter);
            dPostOffice::deliverToPlayer(&l_mail);
            mail->mLastDate.set(&now);
            markAlwaysLetter(mail, letter + SENT_ALWAYS - 1);
        }
    }
}

// 800D16E8
void resetDate() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
        if (player != NULL && player->mPID.isValid()) {
            dTime_c now = *dTime_c::getCurrent();
            now.add(0, -6, 0, 0);
            player->mMotherMail.mLastDate.set(&now);
        }
    }
}

// 800D17C0
BOOL sendFirst(dPrivateData_c *player, const dItem::Item *paper) {
    static dMail_c l_mail;
    int letter = (player->isFlag0(PRIVATE_FLAG0_JOINED_AS_VISITOR) != 0) + 9;
    u16 kind = letter;
    l_mail.setupSystem(&kind, "MAIL_ETC_MotherEvent", &l_firstSender, &player->mPID, paper);
    setPresent(&l_mail, TYPE_EVENT, letter);
    if (dPostOffice::deliverToPlayer(&l_mail) == TRUE) {
        player->setFlag0(PRIVATE_FLAG0_MOM_FIRST_LETTER);
    } else {
        return FALSE;
    }
    return TRUE;
}

// 800D18B4
BOOL isDaysPassed(dSaveMotherMail_c *mail, int days, const dTime_c *now) {
    dYMD_c date = mail->getLastDate();
    int diff = date.diffDays(now, TRUE);
    if (diff >= days) {
        return TRUE;
    }
    if (diff < 0) {
        dTime_c next = date.get();
        next.add(days, 0, 0, 0);
        if (next.year >= 2036) {
            next.year -= 36;
        }
        if (dTime_c::isSameOrAfter(*now, next)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800D1A7C
BOOL checkEvent(dSaveMotherMail_c *mail, int event, int player, dTime_c *now, int days, dTime_c *day) {
    if (now->year == mail->getEventYear(event)) {
        return FALSE;
    }
    dQuestEvent_e quest = l_events[event];
    if (event == MOTHER_MAIL_EVENT_BIRTHDAY) {
        quest = l_birthdays[player];
    }
    if (!mail->isRetry(event)) {
        if (!dEvent::isEventWithin(quest, *now, -days, 0)) {
            return FALSE;
        }
    }
    if (!dEvent::isEventWithin(quest, *now, -14, 0)) {
        mail->clearRetry(event);
        return FALSE;
    }
    *day = *now;
    for (int i = 0; i < 14; i++) {
        if (dEvent::isEventOn(quest, day)) {
            break;
        }
        day->add(-1, 0, 0, 0);
    }
    return TRUE;
}

// 800D1C08
BOOL isDay(int day, const dTime_c *now) {
    return dEvent::isEventOn(l_days[day - DAY_NEW_YEARS_DAY], now) != FALSE;
}

// 800D1C48
int selectEventLetter(dSaveMotherMail_c *mail, int event) {
    int bit = SENT_EVENT + event * 2;
    if (mail->isSent(bit)) {
        if (mail->isSent(bit + 1)) {
            if (fgMngProc_rndF(100.0f) < 50.0f) {
                return bit + 2;
            }
            return bit + 1;
        }
        return bit + 2;
    }
    if (mail->isSent(bit + 1)) {
        return bit + 1;
    }
    if (fgMngProc_rndF(100.0f) < 50.0f) {
        return bit + 2;
    }
    return bit + 1;
}

// 800D1D08
void markEventLetter(dSaveMotherMail_c *mail, int event, int letter) {
    int bit = SENT_EVENT + event * 2;
    if (letter & 1) {
        if (mail->isSent(bit + 1)) {
            mail->clearSent(bit);
            mail->clearSent(bit + 1);
        } else {
            mail->setSent(bit);
        }
    } else {
        if (mail->isSent(bit)) {
            mail->clearSent(bit);
            mail->clearSent(bit + 1);
        } else {
            mail->setSent(bit + 1);
        }
    }
}

// 800D1DB8
void getSeasonRange(int month, int *from, int *to) {
    *from = SENT_SEASON;
    if (month == MONTH_AUGUST) {
        *from = SENT_SEASON + MONTH_AUGUST * 3;
        *to = *from + 8;
    } else if (month < MONTH_AUGUST) {
        *from = SENT_SEASON + month * 3;
        *to = *from + 3;
    } else {
        *from = SENT_SEASON + month * 3 + 5;
        *to = *from + 3;
    }
}

// 800D1E18
BOOL hasSeasonLetter(dSaveMotherMail_c *mail, int month) {
    int from, to;
    getSeasonRange(month, &from, &to);
    for (int i = from; i < to; i++) {
        if (!mail->isSent(i)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800D1E90
int selectSeasonLetter(dSaveMotherMail_c *mail, int month) {
    int letters[8];
    int from, to;
    getSeasonRange(month, &from, &to);
    int *letter = letters;
    int num = 0;
    for (int i = from; i < to; i++) {
        if (!mail->isSent(i)) {
            *letter++ = i;
            num++;
        }
    }
    return letters[(int)fgMngProc_rndF(num)] - (SENT_SEASON - 1);
}

// 800D1F5C
int selectAlwaysLetter(dSaveMotherMail_c *mail) {
    int letters[SENT_SEASON - SENT_ALWAYS];
    int *letter = letters;
    int num = 0;
    for (int i = SENT_ALWAYS; i < SENT_SEASON; i++) {
        if (!mail->isSent(i)) {
            *letter++ = i;
            num++;
        }
    }
    return letters[(int)fgMngProc_rndF(num)] - (SENT_ALWAYS - 1);
}

// 800D2010
void markAlwaysLetter(dSaveMotherMail_c *mail, int bit) {
    mail->setSent(bit);
    for (int i = SENT_ALWAYS; i < SENT_SEASON; i++) {
        if (!mail->isSent(i)) {
            return;
        }
    }
    for (int i = SENT_ALWAYS; i < SENT_NUM; i++) {
        mail->clearSent(i);
    }
}

// The presents of each letter kind, per region (all the same).
#define EVENT_PRESENTS \
    {1, dItem::ITEM_IDX_5000_BELLS}, \
    {2, dItem::ITEM_IDX_5000_BELLS}, \
    {3, dItem::ITEM_IDX_PINK_CARN_BAG}, \
    {4, dItem::ITEM_IDX_PINK_CARN_BAG}, \
    {5, dItem::ITEM_IDX_RED_CARN_BAG}, \
    {6, dItem::ITEM_IDX_RED_CARN_BAG}, \
    {7, dItem::ITEM_IDX_PINK_CARN_BAG}, \
    {8, dItem::ITEM_IDX_PINK_CARN_BAG}, \
    {9, dItem::ITEM_IDX_1000_BELLS}, \
    {10, dItem::ITEM_IDX_5000_BELLS}
#define DAY_PRESENTS \
    {1, dItem::ITEM_IDX_1000_BELLS}, \
    {2, dItem::ITEM_IDX_1000_BELLS}, \
    {5, PRESENT_CLOTH}, \
    {6, PRESENT_CLOTH}
#define SEASON_PRESENTS \
    {0x08, dItem::ITEM_IDX_DOCTORS_MASK}, \
    {0x10, PRESENT_CLOTH}, \
    {0x16, dItem::ITEM_IDX_SUNFLOWER}, \
    {0x19, dItem::ITEM_IDX_FAN}, \
    {0x21, dItem::ITEM_IDX_ELEGANT_MUSHROOM}, \
    {0x24, dItem::ITEM_IDX_MEDICINE}
#define ALWAYS_PRESENTS \
    {0x02, PRESENT_CLOTH}, \
    {0x04, dItem::ITEM_IDX_5000_BELLS}, \
    {0x05, dItem::ITEM_IDX_CHOCOLATES}, \
    {0x06, PRESENT_FRUIT}, \
    {0x0C, PRESENT_FRUIT}, \
    {0x10, dItem::ITEM_IDX_CHOCOLATES}, \
    {0x1B, PRESENT_FRUIT}, \
    {0x25, dItem::ITEM_IDX_APPLE}, \
    {0x28, PRESENT_CLOTH}, \
    {0x30, dItem::ITEM_IDX_CARDBOARD_BOX}, \
    {0x32, dItem::ITEM_IDX_TEA_VASE}, \
    {0x33, PRESENT_FRUIT}, \
    {0x34, dItem::ITEM_IDX_TRASH_BIN}, \
    {0x35, dItem::ITEM_IDX_ALARM_CLOCK}, \
    {0x38, PRESENT_CANDY}, \
    {0x39, PRESENT_FLOWER_SEEDS}, \
    {0x3A, PRESENT_CLOTH}, \
    {0x3B, dItem::ITEM_IDX_ORANGE_KNIT_HAT}, \
    {0x3C, PRESENT_CLOTH}

static const Present_s l_eventPresentsJP[] = {EVENT_PRESENTS};
static const Present_s l_dayPresentsJP[] = {DAY_PRESENTS};
static const Present_s l_seasonPresentsJP[] = {SEASON_PRESENTS};
static const Present_s l_alwaysPresentsJP[] = {ALWAYS_PRESENTS};
static const Present_s l_eventPresentsUS[] = {EVENT_PRESENTS};
static const Present_s l_dayPresentsUS[] = {DAY_PRESENTS};
static const Present_s l_seasonPresentsUS[] = {SEASON_PRESENTS};
static const Present_s l_alwaysPresentsUS[] = {ALWAYS_PRESENTS};
static const Present_s l_eventPresentsEU[] = {EVENT_PRESENTS};
static const Present_s l_dayPresentsEU[] = {DAY_PRESENTS};
static const Present_s l_seasonPresentsEU[] = {SEASON_PRESENTS};
static const Present_s l_alwaysPresentsEU[] = {ALWAYS_PRESENTS};
static const Present_s l_eventPresentsKR[] = {EVENT_PRESENTS};
static const Present_s l_dayPresentsKR[] = {DAY_PRESENTS};
static const Present_s l_seasonPresentsKR[] = {SEASON_PRESENTS};
static const Present_s l_alwaysPresentsKR[] = {ALWAYS_PRESENTS};

static const int l_presentNum[CONSOLE_REGION_NUM][TYPE_NUM] = {
    {ARRAY_SIZE(l_eventPresentsJP), ARRAY_SIZE(l_dayPresentsJP), ARRAY_SIZE(l_seasonPresentsJP), ARRAY_SIZE(l_alwaysPresentsJP)},
    {ARRAY_SIZE(l_eventPresentsUS), ARRAY_SIZE(l_dayPresentsUS), ARRAY_SIZE(l_seasonPresentsUS), ARRAY_SIZE(l_alwaysPresentsUS)},
    {ARRAY_SIZE(l_eventPresentsEU), ARRAY_SIZE(l_dayPresentsEU), ARRAY_SIZE(l_seasonPresentsEU), ARRAY_SIZE(l_alwaysPresentsEU)},
    {ARRAY_SIZE(l_eventPresentsKR), ARRAY_SIZE(l_dayPresentsKR), ARRAY_SIZE(l_seasonPresentsKR), ARRAY_SIZE(l_alwaysPresentsKR)},
};

static const Present_s *l_presents[CONSOLE_REGION_NUM][TYPE_NUM] = {
    {l_eventPresentsJP, l_dayPresentsJP, l_seasonPresentsJP, l_alwaysPresentsJP},
    {l_eventPresentsUS, l_dayPresentsUS, l_seasonPresentsUS, l_alwaysPresentsUS},
    {l_eventPresentsEU, l_dayPresentsEU, l_seasonPresentsEU, l_alwaysPresentsEU},
    {l_eventPresentsKR, l_dayPresentsKR, l_seasonPresentsKR, l_alwaysPresentsKR},
};

// 800D2084
void setPresent(dMail_c *mail, int type, int letter) {
    const Present_s *present = l_presents[getRegion()][type];
    int num = l_presentNum[getLanguageArea(getLanguage())][type];
    for (int i = 0; i < num; present++, i++) {
        if (present->mLetter != letter) {
            continue;
        }
        switch (present->mItem) {
        case PRESENT_CLOTH: {
            dItem::Item item;
            dItemSel::Filter_c filter = dItemSel::sDefaultFilter;
            dItemSel::Range_c range(dItem::KIND_CLOTH, 0);
            dItemSel::select(&item, 1, &range, 1, &filter, NULL, 0, NULL);
            mail->setPresent(item.mId, 0xFF);
            break;
        }
        case PRESENT_FRUIT: {
            dItem::Item item(dItem::ITEM_IDX_APPLE, cM::rndInt(5), FALSE);
            mail->setPresent(item.mId, 0xFF);
            break;
        }
        case PRESENT_CANDY: {
            dItem::Item item(dItem::ITEM_IDX_BLUE_CANDY, cM::rndInt(4), FALSE);
            mail->setPresent(item.mId, 0xFF);
            break;
        }
        case PRESENT_FLOWER_SEEDS: {
            int count = 0;
            dItem::Item items[26];
            dItem::Item item;
            for (int idx = dItem::ITEM_IDX_RED_TULIP_BAG; idx < 0x29; idx++) {
                item.setFromIndex(idx);
#if VERSION == VERSION_RUUK01_01
                if (!dItem::getBITM(item.mId)->m_noPurchase) {
#else
                if (!dItem::getBITM(item.mId)->isNotForSale()) {
#endif
                    items[count] = item;
                    count++;
                }
            }
            if (count > 0) {
                item = items[cM::rndInt(count)];
                mail->setPresent(item.mId, 0xFF);
            }
            break;
        }
        default: {
            dItem::Item item(present->mItem);
            mail->setPresent(item.mId, 0xFF);
            break;
        }
        }
    }
}

} // namespace dMotherMail
