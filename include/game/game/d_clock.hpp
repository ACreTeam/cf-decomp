#pragma once

// d_clock (DOL TU d_clock.cpp, .text 8018BAF8..8018D778): the clock process dClock_c (named by its RTTI
// "dClock_c"; profile execute order 5, draw order 0x124) and the countdown timer (dClock::Timer_c).
//
// The clock ("Layout/time.arc") shows the time with two hands, the date, the weekday and a day / night
// animation; it slides in when the player is idle and out during demos or while B is held. The timer
// ("Layout/timer.arc") counts down 3, 5, 10 or 15 minutes (or a hide and seek game's time limit) with a hand,
// the time left and the fish and bugs caught meanwhile (hidden in hide and seek; the totals are posted as a chat
// message when the time is up), and is kept in step between the consoles of an online session
// (NET_PACKET_CLOCK). Only one of the two layouts is shown at a time.
//
// The (dClock) namespace and every name except dClock_c are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_time_stamp.hpp>

namespace nw4r {
namespace lyt {
class Pane;
}
} // namespace nw4r

class dClock_c : public fBase_c {
public:
    // mClockState / mTimerState
    enum STATE_e {
        STATE_HIDDEN,
        STATE_SHOWN,
    };

    // The in / out animations (setClockAnm / setTimerAnm).
    enum ANM_e {
        ANM_IN,
        ANM_OUT,
    };

    // The sky animations (setDayNightAnm, mDayNight): each is bound for the half day that ends with it.
    enum DAY_NIGHT_ANM_e {
        DAY_NIGHT_ANM_NONE = -1,
        DAY_NIGHT_ANM_DAY,   // "time_00_forDay": 19:00-7:00, played 5:00-7:00
        DAY_NIGHT_ANM_NIGHT, // "time_00_forNight": 7:00-19:00, played 17:00-19:00
    };

    // The separator between the date numbers (setDateSeparator).
    enum DATE_SEP_e {
        DATE_SEP_SLASH,  // "/"
        DATE_SEP_DASH,   // "ー"
        DATE_SEP_PERIOD, // the "N_period" pane
    };

    virtual int create();   // 8018BBB4
    virtual int doDelete(); // 8018BD24
    virtual int execute();  // 8018BD78
    virtual int draw();     // 8018BE70

    void updateClock();                // 8018BEC4: hands, day / night, date and weekday
    void setDateSeparator(int type);   // 8018C134: DATE_SEP_e
    void initDateSeparator();          // 8018C200: by language
    void updateTimer();                // 8018C268: scale marks, counters, hand and time left
    BOOL canShow();                    // 8018C4FC: not in a demo scene, a (non-tag) demo or while B is held
    BOOL canShowClock();               // 8018C5C4
    BOOL canShowTimer();               // 8018C65C
    void setClockAnm(int anm);         // 8018C6DC: ANM_e
    void setDayNightAnm(int anm);      // 8018C728: DAY_NIGHT_ANM_e
    void unbindDayNightAnm();          // 8018C784
    void updateDayNight();             // 8018C794: the sky colour animation around 5:00-7:00 and 17:00-19:00
    void setTimerAnm(int anm);         // 8018C8FC: ANM_e
    void stateClockShown();            // 8018C948
    void stateClockHidden();           // 8018C990
    void stateTimerShown();            // 8018CA08
    void stateTimerHidden();           // 8018CA50
    void startHideAndSeek();           // 8018CB34: the timer runs to the hide and seek game's time limit

    f32 getDayNightAnimLen() const {
        return 7200.0f;
    }

    /* 0x064 */ m2d::ResAccLoader_c mClockRes;
    /* 0x138 */ m2d::ResAccLoader_c mTimerRes;
    /* 0x20C */ m2d::SimpleEx_c mClockLyt;
    /* 0x2A4 */ m2d::SimpleEx_c mTimerLyt;
    /* 0x33C */ nw4r::lyt::Pane *mpHourHand;   // "N_short_00"
    /* 0x340 */ nw4r::lyt::Pane *mpMinuteHand; // "N_long_00"
    /* 0x344 */ nw4r::lyt::Pane *mpTimerHand;  // "N_long_00" of the timer
    /* 0x348 */ m2d::Anm_c mClockAnm;          // in / out
    /* 0x36C */ m2d::Anm_c mDayNightAnm;
    /* 0x390 */ m2d::Anm_c mTimerAnm;          // in / out
    /* 0x3B4 */ m2d::Anm_c mTimerPtrnAnm;      // frame 1: hide and seek
    /* 0x3D8 */ int mClockState;               // STATE_e
    /* 0x3DC */ int mTimerState;               // STATE_e
    /* 0x3E0 */ int mDayNight;                 // DAY_NIGHT_ANM_e: the bound sky animation
    /* 0x3E4 */ int mYearDay;                  // the day of the year shown, -1: none
    /* 0x3E8 */ int mShowDelay;                // frames before the clock may slide in again
    /* 0x3EC */ u8 mShownFishCount;            // Timer_c::mFishCount as shown ("T_count_01"), 0xFF: none
    /* 0x3ED */ u8 mShownBugCount;             // Timer_c::mBugCount as shown ("T_count_00"), 0xFF: none
}; // size 0x3F0

namespace dClock {

// The timer lengths (getMinutes): 3, 5, 10 and 15 minutes.
enum {
    TIMER_MINUTES_NUM = 4,
};

// NET_PACKET_CLOCK commands (one byte).
enum Cmd_e {
    CMD_START = 0,                                   // + minutes index, -> host: start the timer
    CMD_STOP = CMD_START + TIMER_MINUTES_NUM,        // -> host: stop the timer
    CMD_SYNC_START,                                  // + minutes index, host -> all: the timer started
    CMD_SYNC_STOP = CMD_SYNC_START + TIMER_MINUTES_NUM, // host -> all: the timer stopped
};

// The countdown timer (sTimer, 80624010).
class Timer_c {
public:
    enum STATE_e {
        STATE_STOP,
        STATE_RUN,
        STATE_FINISH,
    };

    enum FLAG_e {
        FLAG_REDRAW = 0x1,       // the scale marks need an update (checkRedraw)
        FLAG_SEC = 0x2,          // the elapsed seconds changed (checkSec)
        FLAG_HIDE_AND_SEEK = 0x4,
    };

    void reset();                    // 8018CECC
    void setRedraw();                // 8018CEEC
    void execute();                  // 8018CEFC
    BOOL isActive();                 // 8018CF6C: running or finishing
    BOOL isRunning();                // 8018CF80
    void stateStop();                // 8018CF94
    void stateRun();                 // 8018CF98
    void stateFinish();              // 8018D1C0
    void start(int minutes);         // 8018D1E4
    void startHideAndSeek();         // 8018D278
    BOOL isStopped();                // 8018D318
    void stop();                     // 8018D328
    BOOL isCounting();               // 8018D370: same as isRunning
    void updateElapsed();            // 8018D384
    int getElapsed();                // 8018D570: seconds
    int getRemain();                 // 8018D578: seconds
    BOOL checkRedraw();              // 8018D5B0: FLAG_REDRAW, cleared
    BOOL isHideAndSeek() const;            // 8018D5D4
    BOOL checkSec();                 // 8018D5EC: FLAG_SEC, cleared
    u8 getFishCount();               // 8018D610
    void addFishCount();             // 8018D618: up to 99
    u8 getBugCount();                // 8018D630
    void addBugCount();              // 8018D638: up to 99
    int getMinutesIndex();           // 8018D650

    /* 0x00 */ dTimeStamp_c mStart;
    /* 0x08 */ dTimeStamp_c mEnd;
    /* 0x10 */ int mMinutes;
    /* 0x14 */ int mState;       // STATE_e
    /* 0x18 */ int mFinishTimer; // frames left of STATE_FINISH
    /* 0x1C */ int mElapsed;     // seconds
    /* 0x20 */ int mLastRemain;  // the time left (seconds) of the last count sound, from 11
    /* 0x24 */ int mTickTimer;   // frames to the next tick sound
    /* 0x28 */ u16 mFlags;       // FLAG_e
    /* 0x2A */ u8 mFishCount;  // fish caught while running (no trash); shown in "T_count_01"
    /* 0x2B */ u8 mBugCount;   // bugs caught while running; shown in "T_count_00"
}; // size 0x30

BOOL isShown();                                    // 8018C6BC: the clock or the timer is shown (d_obj_chat)
BOOL isHideAndSeekHost();                          // 8018CAC8
BOOL isHideAndSeek();                              // 8018CAFC: the town's hide and seek game runs
void resetTimer();                                 // 8018CB40 (d_save_data)
int getMinutesIndex(int minutes);                  // 8018CB4C
int getMinutes(int index);                         // 8018CBA8
void startTimer(int minutes);                      // 8018CBBC
void stopTimer();                                  // 8018CC48
BOOL isTimerActive();                              // 8018CCAC
void requestStartTimer(int minutes);               // 8018CCB8: online guests ask the host
void requestStopTimer();                           // 8018CD18
void addFishCount();                               // 8018CD60: d_a_player, a fish caught (not trash)
void addBugCount();                                // 8018CDA0: d_a_player, a bug caught
void sendCmd(u8 cmd, int member);                  // 8018CDE0: NET_PACKET_CLOCK
void receive(int member, const void *data);        // 8018CE28: NET_PACKET_CLOCK

} // namespace dClock
