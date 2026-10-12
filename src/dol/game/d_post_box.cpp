// The post boxes: a player's house mailbox and the letters held at the post office.
// .text 8013EBCC..8013F458. See include/game/game/d_post_box.hpp.
#include <game/game/d_post_box.hpp>

// 8013EBCC
static void clearMails(dMail_c *mails, int num) {
    for (int i = 0; i < num; i++) {
        mails[i].clear();
    }
}

// 8013EC28: -1 when full
static int findEmptyMail(dMail_c *mails, int num) {
    for (int i = 0; i < num; i++) {
        if (mails[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

// 8013EC98: moves the letters to the front; returns the count
static int packMails(dMail_c *mails, int num) {
    int i;
    int count;
    for (i = 0, count = 0; i < num; i++) {
        dMail_c *mail = &mails[i];
        if (!mail->isEmpty()) {
            if (i != count) {
                mails[count].copy(mail);
                mail->clear();
            }
            count++;
        }
    }
    return count;
}

// A copy of time for dTimeStamp_c::compare.
static inline int compareTime(dTimeStamp_c *stamp, dTime_c time) {
    return stamp->compare(&time);
}

// 8013ED24
dPlayerPostBox_c::dPlayerPostBox_c() {}

// 8013ED88
dPlayerPostBox_c::~dPlayerPostBox_c() {}

// 8013EDFC
void dPlayerPostBox_c::set(int idx, const dMail_c *mail, BOOL keepStatus) {
    dMail_c *dst = &mMails[idx];
    dst->copy(mail);
    if (!keepStatus) {
        dst->markSent();
    }
}

// 8013EE54
void dPlayerPostBox_c::clear() {
    clearMails(mMails, PLAYER_POST_BOX_NUM);
    mFutureMail.clear();
}

// 8013EE8C
int dPlayerPostBox_c::findEmpty() {
    return findEmptyMail(mMails, PLAYER_POST_BOX_NUM);
}

// 8013EE94
BOOL dPlayerPostBox_c::hasMail() {
    for (int i = 0; i < PLAYER_POST_BOX_NUM; i++) {
        if (!mMails[i].isEmpty()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8013EEF4
void dPlayerPostBox_c::setFutureMail(const dMail_c *mail, const dTime_c *date) {
    mFutureMail.copy(mail);
    mFutureMailDate.set(date);
    mFutureMailDate.setTimeOfDay(9, 0, 0);
}

// 8013EF50
BOOL dPlayerPostBox_c::deliverFutureMail() {
    if (!hasFutureMail()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (compareTime(&mFutureMailDate, now) == -1) {
        return FALSE;
    }
    int idx = findEmpty();
    if (idx == -1) {
        return FALSE;
    }
    set(idx, &mFutureMail, FALSE);
    mFutureMail.clear();
    return TRUE;
}

// 8013F064
dTime_c dPlayerPostBox_c::getFutureMailDate() {
    return mFutureMailDate.get();
}

// 8013F06C
BOOL dPlayerPostBox_c::hasFutureMail() {
    return !mFutureMail.isEmpty();
}

// 8013F098
dPostBox_c::dPostBox_c() {}

// 8013F120
void dPostBox_c::set(int idx, dMail_c *mail) {
    mMails[idx].copy(mail);
}

// 8013F134
void dPostBox_c::setHeld(dMail_c *mail) {
    mHeld.copy(mail);
}

// 8013F140
void dPostBox_c::clearAll() {
    clearMails(mMails, POST_BOX_NUM);
    mHeld.clear();
    mLastDelivery.reset();
    mNextDelivery.reset();
    mFlags = 0;
}

// 8013F1A0
int dPostBox_c::findEmpty() {
    return findEmptyMail(mMails, POST_BOX_NUM);
}

// 8013F1A8
int dPostBox_c::pack() {
    return packMails(mMails, POST_BOX_NUM);
}

// 8013F1B0
void dPostBox_c::clear(int idx) {
    mMails[idx].clear();
}

// 8013F1BC
void dPostBox_c::clearHeld() {
    mHeld.clear();
}

// 8013F1C8
BOOL dPostBox_c::checkDelivery(BOOL reschedule) {
    if (!(mFlags & 1)) {
        if (reschedule) {
            scheduleDelivery();
        }
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (compareTime(&mLastDelivery, now) == -1) {
        if (reschedule) {
            scheduleDelivery();
        }
        return FALSE;
    }
    if (compareTime(&mNextDelivery, now) >= 0) {
        if (reschedule) {
            scheduleDelivery();
        }
        return TRUE;
    }
    return FALSE;
}

// 8013F354
void dPostBox_c::scheduleDelivery() {
    dTime_c now = *dTime_c::getCurrent();
    mLastDelivery.set(&now);
    mNextDelivery.set(&now);
    int hour;
    if (now.hour < 9) {
        hour = 9;
    } else if (now.hour < 17) {
        hour = 17;
    } else {
        hour = 9;
        mNextDelivery.addDays(1);
    }
    mNextDelivery.setTimeOfDay(hour, 0, 0);
    mFlags |= 1;
}
