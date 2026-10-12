#pragma once

// The post boxes in the extra save data (dSaveExtra_c): each player's house mailbox (mPlayerPostBoxes) and the
// letters held at the post office (mPostBox). Source: src/dol/game/d_post_box.cpp (.text 8013EBCC..8013F458).
// No RTTI; the class and function names are inferred (mFutureMail / mFutureMailDate after the New Horizons save
// fields FutureMail / FutureMailDate).

#include <types.h>
#include <game/game/d_mail.hpp>
#include <game/game/d_time_stamp.hpp>

#define PLAYER_POST_BOX_NUM 10 // letters in a player's mailbox
#define POST_BOX_NUM 80        // letters held at the post office

// A player's house mailbox, and the letter the player wrote to their future self.
class dPlayerPostBox_c {
public:
    dPlayerPostBox_c();  // 8013ED24
    ~dPlayerPostBox_c(); // 8013ED88

    void set(int idx, const dMail_c *mail, BOOL keepStatus); // 8013EDFC: keepStatus FALSE: marks it sent
    void clear();                                           // 8013EE54
    int findEmpty();                                        // 8013EE8C: -1 when full
    BOOL hasMail();                                         // 8013EE94
    void setFutureMail(const dMail_c *mail, const dTime_c *date); // 8013EEF4: delivered at 9:00 on date
    BOOL deliverFutureMail();                               // 8013EF50: into the mailbox once it is due
    dTime_c getFutureMailDate();                            // 8013F064
    BOOL hasFutureMail();                                   // 8013F06C

    /* 0x0000 */ dMail_c mMails[PLAYER_POST_BOX_NUM];
    /* 0x23A0 */ dMail_c mFutureMail;
    /* 0x2730 */ dTimeStamp_c mFutureMailDate;
}; // size 0x2738

// The letters held at the post office. They are delivered at 9:00 and 17:00.
class dPostBox_c {
public:
    static dPostBox_c *get(); // dSaveExtra_c::mPostBox (d_save_data.hpp)

    dPostBox_c(); // 8013F098

    void set(int idx, dMail_c *mail);           // 8013F120
    void setHeld(dMail_c *mail);                // 8013F134
    void clearAll();                            // 8013F140
    int findEmpty();                            // 8013F1A0: -1 when full
    int pack();                                 // 8013F1A8: moves the letters to the front; returns the count
    void clear(int idx);                        // 8013F1B0
    void clearHeld();                           // 8013F1BC
    BOOL checkDelivery(BOOL reschedule);        // 8013F1C8: a delivery time has passed
    void scheduleDelivery();                    // 8013F354: the next 9:00 / 17:00

    /* 0x00000 */ dMail_c mMails[POST_BOX_NUM];
    /* 0x11D00 */ dMail_c mHeld;               // index -1 in the send loop
    /* 0x12090 */ dTimeStamp_c mLastDelivery;
    /* 0x12098 */ dTimeStamp_c mNextDelivery;
    /* 0x120A0 */ u16 mFlags;                  // bit 0: a delivery is scheduled
}; // size 0x120A2
