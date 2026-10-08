// The town notice board (dNoticeBoard_c, 15 x dNotice_c) and noticeWord_c.
// .text 8011ACB4..8011B400, sinit 8011B3B8. Notes: notes/d_notice.txt.
#include <game/game/d_notice.hpp>
#include <game/game/d_npc_notice.hpp>
#include <game/cLib/c_lib.hpp>
#include <cstring>

// ---------------------------------------------------------------------------
// dNotice_c

// 8011ACB4
dNotice_c::dNotice_c() {}

// 8011ACC8
dNotice_c::~dNotice_c() {}

// 8011AD08
dNotice_c &dNotice_c::operator=(const dNotice_c &other) {
    memcpy(this, &other, sizeof(dNotice_c));
    return *this;
}

// 8011AD3C
void dNotice_c::clear() {
    memset(this, 0, sizeof(dNotice_c));
}

// 8011AD48
void dNotice_c::init(const dTime_c *date) {
    clear();
    mFlags = 1;
    mDate.set(date);
}

// 8011AD94
void dNotice_c::setSender(const wchar_t *name) {
    memcpy(mSender, name, sizeof(mSender));
}

// 8011AD9C
void dNotice_c::setText(const wchar_t *text) {
    memcpy(mText, text, sizeof(mText));
}

// 8011ADA8
wchar_t *dNotice_c::getSender() {
    return mSender;
}

// 8011ADAC
wchar_t *dNotice_c::getText() {
    return mText;
}

// 8011ADB4
BOOL dNotice_c::isValid() {
    return (u16)(mFlags & 1) != 0;
}

// 8011ADCC
BOOL dNotice_c::isRead(int player) {
    return (mFlags & (u16)(2 << player)) != 0;
}

// 8011ADF0
void dNotice_c::setRead(int player) {
    mFlags |= (u16)(2 << player);
}

// 8011AE0C
void dNotice_c::clearRead(int player) {
    mFlags &= ~(u16)(2 << player);
}

// 8011AE28
u8 dNotice_c::getDay() {
    return mDate.day;
}

// 8011AE30
u8 dNotice_c::getMonth() {
    return mDate.month;
}

// 8011AE38
u16 dNotice_c::getYear() {
    return mDate.year;
}

// 8011AE40
BOOL dNotice_c::isFlag20() {
    return (u16)(mFlags & 0x20) != 0;
}

// 8011AE58
void dNotice_c::setFlag20() {
    mFlags |= 0x20;
}

// ---------------------------------------------------------------------------
// noticeWord_c

// 8011AE68
noticeWord_c::noticeWord_c() {
    clear();
}

// 8011AEAC
noticeWord_c::~noticeWord_c() {}

// 8011AF04
u32 noticeWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011AF0C
wchar_t *noticeWord_c::getBuffer() {
    return mBuffer;
}

// ---------------------------------------------------------------------------
// dNoticeBoard_c

// 8011AF14
dNoticeBoard_c::dNoticeBoard_c() {}

// 8011AF70
void dNoticeBoard_c::clear() {
    mHead = 0;
    dNotice_c *notice = mNotices;
    for (int i = 0; i < NOTICE_NUM; i++, notice++) {
        notice->clear();
    }
    mTime.reset();
}

// 8011AFD8
void dNoticeBoard_c::init() {
    fn_800EBB24(1, "BBS_default", NULL, NULL);
    fn_800EBB24(2, "BBS_default", NULL, NULL);
    setTime(dTime_c::getCurrent());
}

// 8011B044
dNotice_c *dNoticeBoard_c::getNotice(int i) {
    return get(i);
}

// 8011B048
dNotice_c *dNoticeBoard_c::get(int i) {
    i += mHead;
    if (i >= NOTICE_NUM) {
        i -= NOTICE_NUM;
    }
    return &mNotices[i];
}

// 8011B06C
dNotice_c *dNoticeBoard_c::at(int i) {
    i += mHead;
    if (i >= NOTICE_NUM) {
        i -= NOTICE_NUM;
    }
    return &mNotices[i];
}

// 8011B090
int dNoticeBoard_c::getNum() {
    for (int i = 0; i < NOTICE_NUM; i++) {
        if (!get(i)->isValid()) {
            return i;
        }
    }
    return NOTICE_NUM;
}

// 8011B0F4
void dNoticeBoard_c::add(const dNotice_c *notice) {
    int num = getNum();
    if (num >= NOTICE_NUM) {
        u8 head = mHead;
        mHead++;
        if (mHead >= NOTICE_NUM) {
            mHead = 0;
        }
        mNotices[head] = *notice;
    } else {
        *at(num) = *notice;
    }
}

// 8011B180
void dNoticeBoard_c::remove(int i) {
    for (int j = i; j < NOTICE_NUM - 1; j++) {
        dNotice_c *dst = at(j);
        *dst = *at(j + 1);
    }
    at(NOTICE_NUM - 1)->clear();
}

// 8011B204
void dNoticeBoard_c::sort(int count) {
    if (count > 1) {
        if (count > NOTICE_NUM) {
            count = NOTICE_NUM;
        }
        int num = getNum();
        dNotice_c tmp;
        dNotice_c *a, *b;
        for (int i = num - count; i < num - 1; i++) {
            a = at(i);
            for (int j = i + 1; j < num; j++) {
                b = at(j);
                if (a->mDate.compare(&b->mDate) == 1) {
                    tmp = *a;
                    *a = *b;
                    *b = tmp;
                }
            }
        }
    }
}

// 8011B2F0
void dNoticeBoard_c::setRead(int i, int player) {
    at(i)->setRead(player);
}

// 8011B324
dTime_c dNoticeBoard_c::getTime() {
    return mTime.get();
}

// 8011B328
void dNoticeBoard_c::setTime(const dTime_c *now) {
    mTime.set(now);
    mTime.toDayStart();
}

// 8011B35C
void dNoticeBoard_c::clearRead(int player) {
    dNotice_c *notice = mNotices;
    for (int i = 0; i < NOTICE_NUM; i++, notice++) {
        notice->clearRead(player);
    }
}

static noticeWord_c sWord; // lbl_805F1430
