// A player's record of Mom's letters (dSaveMotherMail_c). .text 8011041C..80110634.
#include <game/game/d_save_mother_mail.hpp>

// 8011041C
dSaveMotherMail_c::dSaveMotherMail_c() {}

// 80110430
dSaveMotherMail_c::~dSaveMotherMail_c() {}

// 80110470
void dSaveMotherMail_c::clear() {
    mLastDate.clear();
    for (int i = 0; i < 4; i++) {
        mEventYears[i] = MOTHER_MAIL_EVENT_YEAR_NONE;
    }
    for (int i = 0; i < 15; i++) {
        mSent[i] = 0;
    }
    mRetry = 0;
}

// 801104D8
int dSaveMotherMail_c::getEventYear(int event) {
    u8 year = mEventYears[event];
    if (year == MOTHER_MAIL_EVENT_YEAR_NONE) {
        return -1;
    }
    return year + 2000;
}

// 801104F4
void dSaveMotherMail_c::setEventYear(int event) {
    dTime_c now = *dTime_c::getCurrent();
    mEventYears[event] = now.year - 2000;
}

// 80110588
void dSaveMotherMail_c::setSent(int bit) {
    mSent[bit >> 3] |= 1 << (bit & 7);
}

// 801105AC
void dSaveMotherMail_c::clearSent(int bit) {
    mSent[bit >> 3] &= ~(1 << (bit & 7));
}

// 801105D0
BOOL dSaveMotherMail_c::isSent(int bit) {
    return (1 << (bit & 7)) & mSent[bit >> 3];
}

// 801105F0
void dSaveMotherMail_c::setRetry(int event) {
    mRetry |= 1 << event;
}

// 80110608
void dSaveMotherMail_c::clearRetry(int event) {
    mRetry &= ~(1 << event);
}

// 80110620
BOOL dSaveMotherMail_c::isRetry(int event) {
    return mRetry & (1 << event);
}
