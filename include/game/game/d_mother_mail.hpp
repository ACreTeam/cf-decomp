#pragma once

// Mom's letters ("MAIL_ETC_Mother*"), sent from the day change. Names are inferred.

#include <types.h>
#include <game/game/d_date.hpp>

class dMail_c;
class dPrivateData_c;
class dSaveMotherMail_c;
namespace dItem {
struct Item;
}

namespace dMotherMail {

// The letters of a day (dSaveMotherMail_c event index): the three yearly events, then the holidays.
enum {
    DAY_BIRTHDAY,
    DAY_MOTHERS_DAY,
    DAY_FATHERS_DAY,
    DAY_NEW_YEARS_DAY,
    DAY_APRIL_FOOLS_DAY,
    DAY_TOY_DAY,
    DAY_NUM,
};

// The letter kinds, which pick the present tables.
enum {
    TYPE_EVENT,
    TYPE_DAY,
    TYPE_SEASON,
    TYPE_ALWAYS,
    TYPE_NUM,
};

void update(dTime_c *last, int days);
void resetDate();
BOOL sendFirst(dPrivateData_c *player, const dItem::Item *paper);
BOOL isDaysPassed(dSaveMotherMail_c *mail, int days, const dTime_c *now);
BOOL checkEvent(dSaveMotherMail_c *mail, int event, int player, dTime_c *now, int days, dTime_c *day);
BOOL isDay(int day, const dTime_c *now);
int selectEventLetter(dSaveMotherMail_c *mail, int event);
void markEventLetter(dSaveMotherMail_c *mail, int event, int letter);
void getSeasonRange(int month, int *from, int *to);
BOOL hasSeasonLetter(dSaveMotherMail_c *mail, int month);
int selectSeasonLetter(dSaveMotherMail_c *mail, int month);
int selectAlwaysLetter(dSaveMotherMail_c *mail);
void markAlwaysLetter(dSaveMotherMail_c *mail, int bit);
void setPresent(dMail_c *mail, int type, int letter);

} // namespace dMotherMail
