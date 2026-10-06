#pragma once

#include <types.h>
#include <game/game/d_date.hpp>

// Event letters from Mom, one a year each (index into mEventYears / mRetry).
enum {
    MOTHER_MAIL_EVENT_BIRTHDAY,     // the player's birthday
    MOTHER_MAIL_EVENT_MOTHERS_DAY,
    MOTHER_MAIL_EVENT_FATHERS_DAY,
    MOTHER_MAIL_EVENT_NUM,
};

#define MOTHER_MAIL_EVENT_YEAR_NONE 0xFF

// A player's record of Mom's letters ("MAIL_ETC_Mother*"), at dPrivateData_c::mMotherMail.
// The letters themselves are sent from the daily update at 800D11C8.
// Source: src/dol/game/d_save_mother_mail.cpp (.text 8011041C..80110634).
class dSaveMotherMail_c { // 0x18
public:
    dSaveMotherMail_c(); // 8011041C
    ~dSaveMotherMail_c(); // 80110430
    void clear(); // 80110470
    int getEventYear(int event); // 801104D8: or -1
    void setEventYear(int event); // 801104F4: to this year
    void setSent(int bit); // 80110588
    void clearSent(int bit); // 801105AC
    BOOL isSent(int bit); // 801105D0
    void setRetry(int event); // 801105F0
    void clearRetry(int event); // 80110608
    BOOL isRetry(int event); // 80110620

    /* 0x00 */ dYMD_c mLastDate; // the last letter
    /* 0x04 */ u8 mEventYears[4]; // year - 2000 of each event's last letter, or MOTHER_MAIL_EVENT_YEAR_NONE
    /* 0x08 */ u8 mSent[15]; // letters already sent: event variants, then the regular and seasonal letters
    /* 0x17 */ u8 mRetry; // event letters to send again (the mailbox was full)
};
