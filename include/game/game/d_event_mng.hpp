#pragma once

// d_event_mng (DOL TU d_event_mng.cpp, .text 8018F588..8018FE10): dEventMng_c (named by its RTTI), the process
// that announces the town events (sys_MESS/SYS_Announcement) when one starts or ends while the player is in the
// town, and runs the New Year's countdown. Names are inferred from behaviour.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_msg_rcpt.hpp>

class dEventMng_c : public fBase_c {
public:
    typedef void (dEventMng_c::*StateFunc)();

    enum {
        EVENT_NUM = 9, // the announced events (l_eventIds)
        EVENT_IDX_COUNTDOWN = 3,
    };

    // What the current announcement is for (mKind).
    enum Kind_e {
        KIND_START = 0,     // the event starts: its id becomes the daily id
        KIND_END = 1,       // the event is over: the daily id is reset
        KIND_COUNTDOWN = 2, // midnight on New Year's Eve: the countdown camera, then the announcement
    };

    // mState: index into l_states.
    enum State_e {
        STATE_WAIT = 0,      // waits for an event to start or end
        STATE_OPEN = 1,      // the message window is opening
        STATE_SHOW = 2,      // the message is shown, until the window closes
        STATE_COUNTDOWN = 3, // the countdown runs (mTimer), then the announcement
    };

    dEventMng_c() {}
    virtual ~dEventMng_c(); // 8018FDB4
    virtual int create();   // 8018F5F4
    virtual int doDelete(); // 8018F634
    virtual int execute();  // 8018F63C
    virtual int draw();     // 8018F678

    void executeWait();                    // 8018F680
    void executeOpen();                    // 8018F90C
    void executeShow();                    // 8018F97C
    void executeCountdown();               // 8018F9F0
    void startMessage();                   // 8018FA10
    void endMessage();                     // 8018FAB0
    int findEvent(int eventId);            // 8018FB00: the index of the event with this id, -1 if none
    int getEventId(int idx);               // 8018FB6C: a dQuestEvent_e
    BOOL isEventNow(int idx);              // 8018FB80: the event is today and between its start and end
    int findEventNow();                    // 8018FC28: the first event isEventNow, -1 if none
    int getStartHour(int idx);             // 8018FC88
    int getStartMin(int idx);              // 8018FC9C
    int getEndHour(int idx);               // 8018FCB0
    int getEndMin(int idx);                // 8018FCC4
    u16 getStartMsg(int idx);              // 8018FCD8: the opening message, then the reminder, 0 near the end
    u16 getEndMsg(int idx);                // 8018FDA0

    /* 0x064 */ dMsg::Rcpt_c mRcpt;
    /* 0x0C8 */ int mEventIdx;
    /* 0x0CC */ u16 mMsgCode;
    /* 0x0D0 */ int mTimer;  // the countdown, in frames
    /* 0x0D4 */ u8 mKind;    // Kind_e
    /* 0x0D5 */ u8 mState;   // State_e
}; // size 0xD8
