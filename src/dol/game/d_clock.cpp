// The clock and the countdown timer (dClock_c, dClock). .text 8018BAF8..8018D778, .ctors 80465828, .data
// 804F9610..804F9880, .bss 80624010, .sdata 8074BEB8..8074BF00, .sbss 8074E9C8, .sdata2 807515C8..807515F8.
// See include/game/game/d_clock.hpp.
#include <game/game/d_clock.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/nw4r/lyt/lyt_pane.h>
#include <lib/nw4r/lyt/lyt_textBox.h>
#include <lib/revolution/WPAD/WPAD.h>
#include <game/snd/snd_id.hpp>

CUSTOM_BASE_PROFILE(CLOCK, dClock_c, 5, 0x124);

#define SHOW_DELAY 80 // frames the clock waits before sliding in again

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR (60 * 60)

// The sky animations run over the two hours from SUNRISE_HOUR and from SUNSET_HOUR.
#define SUNRISE_HOUR 5
#define DAY_HOUR 7
#define SUNSET_HOUR 17
#define NIGHT_HOUR 19
#define TWILIGHT_SECONDS (2 * SECONDS_PER_HOUR)

#define TIMER_TICK_FRAMES 12    // frames between tick sounds
#define TIMER_FINISH_FRAMES 300 // frames STATE_FINISH lasts
#define TIMER_COUNT_SECONDS 11  // the count sound plays for the last 10 seconds
#define COUNT_MAX 99


#define MSG_WEEKDAY_GROUP "sys_2D/SYS_2D" // weekday names, Monday 1 .. Sunday 7
#define MSG_TIMER_TIME 199                // "mm:ss" from word slots 0 and 1
#define MSG_TIMER_RESULT 200              // the counters, from word slots 0 and 1
#define MSG_TIMER_RESULT_GROUP "sys_2D/SYS_2D_menu"

namespace dClock {

// sFlags: the layouts shown.
enum {
    FLAG_CLOCK = 0x1,
    FLAG_TIMER = 0x2,
};

static u16 sFlags;

static Timer_c sTimer;

// A copy of time for dTimeStamp_c::compare.
static inline int compareTime(dTimeStamp_c *stamp, dTime_c time) {
    return stamp->compare(time);
}

} // namespace dClock

using namespace dClock;

// 8018BBB4
int dClock_c::create() {
    bool clockLoaded = mClockRes.load("Layout/time.arc");
    bool timerLoaded = mTimerRes.load("Layout/timer.arc");
    if (!(clockLoaded & timerLoaded)) {
        return NOT_READY;
    }
    sFlags = 0;
    mClockLyt.build(&mClockRes, "time_00.brlyt");
    initDateSeparator();
    mpMinuteHand = mClockLyt.findPane("N_long_00");
    mpHourHand = mClockLyt.findPane("N_short_00");
    mDayNight = -1;
    mYearDay = -1;
    setClockAnm(ANM_OUT);
    mClockAnm.setToEnd();
    mTimerLyt.build(&mTimerRes, "timer_00.brlyt");
    mpTimerHand = mTimerLyt.findPane("N_long_00");
    setTimerAnm(ANM_OUT);
    mTimerAnm.setToEnd();
    mTimerLyt.setAnm(&mTimerPtrnAnm, "timer_00_timerPtrn.brlan", NULL);
    mClockState = STATE_HIDDEN;
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        resetTimer();
    } else if (isHideAndSeekHost()) {
        startHideAndSeek();
    }
    sTimer.setRedraw();
    mShownFishCount = 0xFF;
    mShownBugCount = 0xFF;
    mShowDelay = SHOW_DELAY;
    return SUCCEEDED;
}

namespace dClock {

static void (dClock_c::*l_clockStateFuncs[])() = {
    &dClock_c::stateClockHidden,
    &dClock_c::stateClockShown,
};

static void (dClock_c::*l_timerStateFuncs[])() = {
    &dClock_c::stateTimerHidden,
    &dClock_c::stateTimerShown,
};

} // namespace dClock

// 8018BD24
int dClock_c::doDelete() {
    if (!mClockRes.remove()) {
        return NOT_READY;
    }
    bool ok = mTimerRes.remove();
    return ok;
}

// 8018BD78
int dClock_c::execute() {
    sTimer.execute();
    mClockAnm.play();
    mTimerAnm.play();
    if (mClockAnm.mFrameCtrl.isStop()) {
        (this->*l_clockStateFuncs[mClockState])();
    }
    if (mTimerAnm.mFrameCtrl.isStop()) {
        (this->*l_timerStateFuncs[mTimerState])();
    }
    if (sFlags & FLAG_CLOCK) {
        updateClock();
        mClockLyt.calc();
    }
    if (sFlags & FLAG_TIMER) {
        updateTimer();
        mTimerLyt.calc();
    }
    return SUCCEEDED;
}

// 8018BE70
int dClock_c::draw() {
    if (sFlags & FLAG_CLOCK) {
        mClockLyt.entry();
    }
    if (sFlags & FLAG_TIMER) {
        mTimerLyt.entry();
    }
    return SUCCEEDED;
}

// 8018BEC4
void dClock_c::updateClock() {
    dTime_c now = *dTime_c::getCurrent();
    mVec3_c rot;
    rot.x = 0.0f;
    rot.y = 0.0f;
    // 0.1 degrees per second / 1 degree per 120 seconds
    rot.z = -(0.1f * (now.sec + now.min * SECONDS_PER_MINUTE));
    mpMinuteHand->SetRotate(rot);
    int hour = now.hour;
    if (hour > 12) {
        hour -= 12;
    }
    rot.z = -((now.sec + (now.min + hour * 60) * SECONDS_PER_MINUTE) / 120.0f);
    mpHourHand->SetRotate(rot);
    updateDayNight();
    if (now.yday != mYearDay) {
        mYearDay = now.yday;
        dString::Word_c word;
        switch (getLanguage()) {
        case dItem::LANG_MX:
        case dItem::LANG_QC:
        case dItem::LANG_EN:
        case dItem::LANG_ES:
        case dItem::LANG_FR:
        case dItem::LANG_IT:
        case dItem::LANG_DE:
            // day.month: the "T_day" pane comes second
            mClockLyt.setNumber("T_day", now.month + 1, 12, dScript::NUM_FORMAT_REGION);
            mClockLyt.setNumber("T_month", now.mday, 12, dScript::NUM_FORMAT_REGION);
            break;
        default:
            mClockLyt.setNumber("T_month", now.month + 1, 12, dScript::NUM_FORMAT_REGION);
            mClockLyt.setNumber("T_day", now.mday, 12, dScript::NUM_FORMAT_REGION);
            break;
        }
        nw4r::lyt::TextBox *text = mClockLyt.findTextBox("T_yobi");
        dString::loadString(&word, (now.wday + 6) % 7 + 1, MSG_WEEKDAY_GROUP);
        text->SetString(word.getText(), 0);
    }
}

// 8018C134
void dClock_c::setDateSeparator(int type) {
    mClockLyt.setVisible("T_slsh", type != DATE_SEP_PERIOD);
    mClockLyt.setVisible("N_period", type == DATE_SEP_PERIOD);
    static const wchar_t l_slash[] = L"/";
    static const wchar_t l_dash[] = L"\x30FC";
    m2d::stringWord_c word;
    if (type != DATE_SEP_PERIOD) {
        if (type == DATE_SEP_SLASH) {
            word.set(l_slash, 0);
        } else if (type == DATE_SEP_DASH) {
            word.set(l_dash, 0);
        }
        mClockLyt.setWord("T_slsh", &word);
    }
}

// 8018C200
void dClock_c::initDateSeparator() {
    int lang = getLanguage();
    if (lang == dItem::LANG_ES) {
        setDateSeparator(DATE_SEP_SLASH);
    } else if (lang == dItem::LANG_DE) {
        setDateSeparator(DATE_SEP_PERIOD);
    } else {
        setDateSeparator(DATE_SEP_SLASH);
    }
}

namespace dClock {

// The timer's scale marks, by minutes index.
static const char *l_memoriNames[TIMER_MINUTES_NUM] = {
    "memori_00",
    "memori_01",
    "memori_02",
    "memori_03",
};

} // namespace dClock

// 8018C268
void dClock_c::updateTimer() {
    int index = sTimer.getMinutesIndex();
    if (sTimer.checkRedraw()) {
        for (int i = 0; i < TIMER_MINUTES_NUM; i++) {
            mTimerLyt.setVisible(l_memoriNames[i], i == index);
        }
        BOOL showCount;
        if (sTimer.isHideAndSeek()) {
            showCount = FALSE;
            mTimerPtrnAnm.setFrame(1.0f);
        } else {
            showCount = TRUE;
            mTimerPtrnAnm.setFrame(0.0f);
        }
        mTimerLyt.setVisible("T_count_00", showCount);
        mTimerLyt.setVisible("T_count_01", showCount);
    }
    if (sTimer.getFishCount() != mShownFishCount) {
        mShownFishCount = sTimer.getFishCount();
        mTimerLyt.setNumber("T_count_01", mShownFishCount, 2, dScript::NUM_FORMAT_PLAIN);
    }
    if (sTimer.getBugCount() != mShownBugCount) {
        mShownBugCount = sTimer.getBugCount();
        mTimerLyt.setNumber("T_count_00", mShownBugCount, 2, dScript::NUM_FORMAT_PLAIN);
    }
    if (sTimer.checkSec()) {
        int elapsed = sTimer.getElapsed();
        mVec3_c rot;
        rot.x = 0.0f;
        rot.y = 0.0f;
        rot.z = -(0.4f * elapsed); // one turn in 15 minutes
        mpTimerHand->SetRotate(rot);
        int remain = sTimer.getRemain();
        dString::Word_c word;
        dScript::formatNumber(&word, remain / SECONDS_PER_MINUTE, 2, dScript::NUM_FORMAT_ZERO_PAD, 0);
        dString::setWord(0, &word);
        word.clear();
        dScript::formatNumber(&word, remain % SECONDS_PER_MINUTE, 2, dScript::NUM_FORMAT_ZERO_PAD, 0);
        dString::setWord(1, &word);
        mTimerLyt.setMessage("T_time_00", MSG_TIMER_TIME, NULL);
    }
}

// 8018C4FC
BOOL dClock_c::canShow() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        return FALSE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY) && !dDemo::isType(dDemo::TYPE_UITAG) &&
        !dDemo::isType(dDemo::TYPE_KEYBOARD)) {
        if (dDemo::isTalking()) {
            mShowDelay = SHOW_DELAY;
        }
        return FALSE;
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_B)) {
        mShowDelay = SHOW_DELAY;
        return FALSE;
    }
    return TRUE;
}

// 8018C5C4
BOOL dClock_c::canShowClock() {
    if (sTimer.isActive()) {
        return FALSE;
    }
    if (sFlags & FLAG_TIMER) {
        return FALSE;
    }
    if (dPlayerMgr::fn_800FFAC0() || dPlayerMgr::isWalking(4) || dPlayerMgr::fn_800FFB18()) {
        mShowDelay = SHOW_DELAY;
        return FALSE;
    }
    return canShow();
}

// 8018C65C
BOOL dClock_c::canShowTimer() {
    if (!sTimer.isActive()) {
        return FALSE;
    }
    if (sFlags & FLAG_CLOCK) {
        return FALSE;
    }
    return canShow();
}

namespace dClock {

// 8018C6BC
BOOL isShown() {
    return (sFlags & FLAG_CLOCK) || (sFlags & FLAG_TIMER);
}

} // namespace dClock

// 8018C6DC
void dClock_c::setClockAnm(int anm) {
    static const char *l_clockAnmNames[] = {
        "time_00_timeIn.brlan",
        "time_00_timeOut.brlan",
    };
    mClockLyt.setAnm(&mClockAnm, l_clockAnmNames[anm], NULL);
    mClockAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
}

// 8018C728
void dClock_c::setDayNightAnm(int anm) {
    static const char *l_dayNightAnmNames[] = {
        "time_00_forDay.brlan",
        "time_00_forNight.brlan",
    };
    mDayNightAnm.create(l_dayNightAnmNames[anm], &mClockRes);
    mClockLyt.bind(&mDayNightAnm);
    mDayNight = anm;
}

// 8018C784
void dClock_c::unbindDayNightAnm() {
    mClockLyt.unbind(&mDayNightAnm);
}

// 8018C794
void dClock_c::updateDayNight() {
    dTime_c now = *dTime_c::getCurrent();
    int anm;
    if (now.hour >= DAY_HOUR && now.hour < NIGHT_HOUR) {
        anm = DAY_NIGHT_ANM_NIGHT;
    } else {
        anm = DAY_NIGHT_ANM_DAY;
    }
    if (anm != mDayNight) {
        if (mDayNight != DAY_NIGHT_ANM_NONE) {
            unbindDayNightAnm();
        }
        setDayNightAnm(anm);
    }
    int sec;
    if (now.hour >= SUNRISE_HOUR && now.hour < DAY_HOUR) {
        sec = (now.hour - SUNRISE_HOUR) * SECONDS_PER_HOUR;
    } else if (now.hour >= SUNSET_HOUR && now.hour < NIGHT_HOUR) {
        sec = (now.hour - SUNSET_HOUR) * SECONDS_PER_HOUR;
    } else {
        mDayNightAnm.setFrame(0.0f);
        return;
    }
    sec += now.sec + now.min * SECONDS_PER_MINUTE;
    mDayNightAnm.setFrame(mDayNightAnm.mFrameCtrl.calcFrame(sec, TWILIGHT_SECONDS));
}

// 8018C8FC
void dClock_c::setTimerAnm(int anm) {
    static const char *l_timerAnmNames[] = {
        "timer_00_timerIn.brlan",
        "timer_00_timerOut.brlan",
    };
    mTimerLyt.setAnm(&mTimerAnm, l_timerAnmNames[anm], NULL);
    mTimerAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
}

// 8018C948
void dClock_c::stateClockShown() {
    if (!canShowClock()) {
        setClockAnm(ANM_OUT);
        mClockState = STATE_HIDDEN;
    }
}

// 8018C990
void dClock_c::stateClockHidden() {
    sFlags &= ~FLAG_CLOCK;
    if (canShowClock()) {
        if (mShowDelay > 0) {
            mShowDelay--;
        } else {
            setClockAnm(ANM_IN);
            sFlags |= FLAG_CLOCK;
            mClockState = STATE_SHOWN;
        }
    }
}

// 8018CA08
void dClock_c::stateTimerShown() {
    if (!canShowTimer()) {
        setTimerAnm(ANM_OUT);
        mTimerState = STATE_HIDDEN;
    }
}

// 8018CA50
void dClock_c::stateTimerHidden() {
    sFlags &= ~FLAG_TIMER;
    if (canShowTimer()) {
        if (mShowDelay > 0) {
            mShowDelay--;
        } else {
            setTimerAnm(ANM_IN);
            sFlags |= FLAG_TIMER;
            mTimerState = STATE_SHOWN;
        }
    }
}

namespace dClock {

// 8018CAC8
BOOL isHideAndSeekHost() {
    return !dNet::isHost() ? FALSE : isHideAndSeek();
}

// 8018CAFC
BOOL isHideAndSeek() {
    return dSaveData_c::getAnimalBlock()->isHideAndSeekRunning() != FALSE;
}

} // namespace dClock

// 8018CB34
void dClock_c::startHideAndSeek() {
    sTimer.startHideAndSeek();
}

namespace dClock {

// 8018CB40
void resetTimer() {
    sTimer.reset();
}

static int l_timerMinutes[TIMER_MINUTES_NUM] = {3, 5, 10, 15};

// 8018CB4C
int getMinutesIndex(int minutes) {
    for (int i = 0; i < TIMER_MINUTES_NUM; i++) {
        if (minutes == l_timerMinutes[i]) {
            return i;
        }
    }
    return 0;
}

// 8018CBA8
int getMinutes(int index) {
    return l_timerMinutes[index];
}

// 8018CBBC
void startTimer(int minutes) {
    if (sTimer.isStopped()) {
        int index = getMinutesIndex(minutes);
        sTimer.start(minutes);
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            sendCmd(index + CMD_SYNC_START, NET_MEMBER_NONE);
        }
    }
}

// 8018CC48
void stopTimer() {
    if (sTimer.isCounting()) {
        sTimer.stop();
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            sendCmd(CMD_SYNC_STOP, NET_MEMBER_NONE);
        }
    }
}

// 8018CCAC
BOOL isTimerActive() {
    return sTimer.isActive();
}

// 8018CCB8
void requestStartTimer(int minutes) {
    if (!dNet::isSessionActive() || dNet::isHost()) {
        startTimer(minutes);
    } else {
        sendCmd(CMD_START + getMinutesIndex(minutes), NET_MEMBER_HOST);
    }
}

// 8018CD18
void requestStopTimer() {
    if (!dNet::isSessionActive() || dNet::isHost()) {
        stopTimer();
    } else {
        sendCmd(CMD_STOP, NET_MEMBER_HOST);
    }
}

// 8018CD60
void addFishCount() {
    if (sTimer.isRunning()) {
        sTimer.addFishCount();
    }
}

// 8018CDA0
void addBugCount() {
    if (sTimer.isRunning()) {
        sTimer.addBugCount();
    }
}

// 8018CDE0
void sendCmd(u8 cmd, int member) {
    dNet::beginPacket();
    dNet::addPacket(&cmd, sizeof(cmd));
    dNet::sendPacket(NET_PACKET_CLOCK, member);
}

// 8018CE28
void receive(int member, const void *data) {
    u8 cmd = *(const u8 *)data;
    switch (cmd) {
    case CMD_START + 0:
    case CMD_START + 1:
    case CMD_START + 2:
    case CMD_START + 3:
        startTimer(getMinutes(cmd - CMD_START));
        break;
    case CMD_STOP:
        stopTimer();
        break;
    case CMD_SYNC_START + 0:
    case CMD_SYNC_START + 1:
    case CMD_SYNC_START + 2:
    case CMD_SYNC_START + 3:
        sTimer.start(getMinutes(cmd - CMD_SYNC_START));
        break;
    case CMD_SYNC_STOP:
        sTimer.stop();
        break;
    }
}

// 8018CEAC
static void playEndSe() {
    playSe(SE_timer_countdown_hi);
}

// 8018CEB4
static void playCountSe() {
    playSe(SE_timer_countdown_low);
}

// 8018CEBC
static void playStartSe() {
    playSe(SE_timer_start);
}

// 8018CEC4
static void playStopSe() {
    playSe(SE_timer_stop);
}

// 8018CECC
void Timer_c::reset() {
    mState = STATE_STOP;
    mFlags = 0;
    mFinishTimer = 0;
    mFishCount = 0;
    mBugCount = 0;
    mLastRemain = 0;
}

// 8018CEEC
void Timer_c::setRedraw() {
    mFlags |= FLAG_REDRAW;
}

static void (Timer_c::*l_timerFuncs[])() = {
    &Timer_c::stateStop,
    &Timer_c::stateRun,
    &Timer_c::stateFinish,
};

// 8018CEFC
void Timer_c::execute() {
    int elapsed = mElapsed;
    updateElapsed();
    if (elapsed != mElapsed) {
        mFlags |= FLAG_SEC;
    }
    (this->*l_timerFuncs[mState])();
}

// 8018CF6C
BOOL Timer_c::isActive() {
    return mState != STATE_STOP;
}

// 8018CF80
BOOL Timer_c::isRunning() {
    return mState == STATE_RUN;
}

// 8018CF94
void Timer_c::stateStop() {}

// 8018CF98
void Timer_c::stateRun() {
    dTime_c now = *dTime_c::getCurrent();
    if (mTickTimer > 0) {
        mTickTimer--;
    } else {
        mTickTimer = TIMER_TICK_FRAMES;
        playSe(SE_TimerDrive);
    }
    if (mFlags & FLAG_SEC) {
        int remain = getRemain();
        if (remain < mLastRemain) {
            mLastRemain = remain;
            if (remain == 0) {
                playEndSe();
            } else {
                playCountSe();
            }
        }
    }
    if (compareTime(&mEnd, now) != -1) {
        mState = STATE_FINISH;
        mFinishTimer = TIMER_FINISH_FRAMES;
        if (mFlags & FLAG_HIDE_AND_SEEK) {
            dObjTelop_c::request(TELOP_HIDE_AND_SEEK_END);
        } else {
            dObjTelop_c::request(TELOP_TIMER_END);
            dString::Word_c word;
            dScript::formatNumber(&word, mFishCount, 2, dScript::NUM_FORMAT_PLAIN, 0);
            dString::setWord(0, &word);
            dScript::formatNumber(&word, mBugCount, 2, dScript::NUM_FORMAT_PLAIN, 0);
            dString::setWord(1, &word);
            dString::loadString(&word, MSG_TIMER_RESULT, MSG_TIMER_RESULT_GROUP);
            dObjChat::postText(word.getText(), dObjChat::TEXT_KIND_SYSTEM);
        }
    } else if (mFlags & FLAG_HIDE_AND_SEEK) {
        if (!isHideAndSeekHost()) {
            reset();
        }
    }
}

// 8018D1C0
void Timer_c::stateFinish() {
    if (mFinishTimer > 0) {
        mFinishTimer--;
    } else {
        mState = STATE_STOP;
    }
}

// 8018D1E4
void Timer_c::start(int minutes) {
    reset();
    mStart.setNow();
    mEnd.setNow();
    mEnd.add(0, 0, minutes, 0);
    mMinutes = minutes;
    mState = STATE_RUN;
    mFlags |= FLAG_REDRAW | FLAG_SEC;
    playStartSe();
    mLastRemain = TIMER_COUNT_SECONDS;
    mTickTimer = TIMER_TICK_FRAMES;
    dObjTelop_c::request(TELOP_TIMER_START);
}

// 8018D278
void Timer_c::startHideAndSeek() {
    reset();
    dQuestPlayerAnimal_c *quest = &dSaveData_c::getRaw()->mAnimals.mTown.mHideAndSeek;
    mStart.set(quest->mBase.getTimeLimit());
    mEnd.set(quest->getEndTime(TRUE));
    mMinutes = quest->mMinutes;
    mState = STATE_RUN;
    mLastRemain = TIMER_COUNT_SECONDS;
    mTickTimer = TIMER_TICK_FRAMES;
    mFlags |= FLAG_REDRAW | FLAG_SEC | FLAG_HIDE_AND_SEEK;
}

// 8018D318
BOOL Timer_c::isStopped() {
    return mState == STATE_STOP;
}

// 8018D328
void Timer_c::stop() {
    if (isCounting()) {
        reset();
        playStopSe();
        dObjTelop_c::request(TELOP_TIMER_STOP);
    }
}

// 8018D370
BOOL Timer_c::isCounting() {
    return mState == STATE_RUN;
}

// 8018D384
void Timer_c::updateElapsed() {
    if (mState == STATE_STOP) {
        mElapsed = 0;
        return;
    }
    int limit = mMinutes * 60;
    dTime_c now = *dTime_c::getCurrent();
    if (compareTime(&mEnd, now) != -1) {
        mElapsed = limit;
        return;
    }
    if (compareTime(&mStart, now) != 1) {
        mElapsed = 0;
        return;
    }
    dTime_c start = mStart.get();
    int min = now.min - start.min;
    if (min < 0) {
        min += 60;
    }
    int sec = now.sec - start.sec;
    if (sec < 0) {
        min--;
        sec += 60;
    }
    sec += min * 60;
    mElapsed = sec;
}

// 8018D570
int Timer_c::getElapsed() {
    return mElapsed;
}

// 8018D578
int Timer_c::getRemain() {
    return mMinutes * 60 - getElapsed();
}

// 8018D5B0
BOOL Timer_c::checkRedraw() {
    if (mFlags & FLAG_REDRAW) {
        mFlags &= ~FLAG_REDRAW;
        return TRUE;
    }
    return FALSE;
}

// 8018D5D4
BOOL Timer_c::isHideAndSeek() const {
    u16 flags = mFlags & FLAG_HIDE_AND_SEEK;
    return flags != 0;
}

// 8018D5EC
BOOL Timer_c::checkSec() {
    if (mFlags & FLAG_SEC) {
        mFlags &= ~FLAG_SEC;
        return TRUE;
    }
    return FALSE;
}

// 8018D610
u8 Timer_c::getFishCount() {
    return mFishCount;
}

// 8018D618
void Timer_c::addFishCount() {
    if (mFishCount < COUNT_MAX) {
        mFishCount++;
    }
}

// 8018D630
u8 Timer_c::getBugCount() {
    return mBugCount;
}

// 8018D638
void Timer_c::addBugCount() {
    if (mBugCount < COUNT_MAX) {
        mBugCount++;
    }
}

// 8018D650
int Timer_c::getMinutesIndex() {
    return dClock::getMinutesIndex(mMinutes);
}

} // namespace dClock
