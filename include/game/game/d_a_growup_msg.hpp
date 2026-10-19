#pragma once

// The new day's announcement (REL d_a_growup_msgNP, profile A_GROWUP_MSG, RTTI
// "daGrowupMsg_c"): after the day change warp (dDemo::isDayChangeWarp, the
// town has "grown up" by a day), it holds the demo, shows "Hey, everybody!
// It's the start of a new day..." (sys_MESS/SYS_Announcement) once the fade
// is over, then leaves the scene by its exit and ends the warp. Source:
// src/d_a_growup_msgNP/d_a_growup_msg.cpp. Names are inferred.

#include <game/framework/f_base.hpp>
#include <game/game/d_msg_rcpt.hpp>

namespace dAGrowupMsg {

// sys_MESS/SYS_Announcement.
enum {
    MSG_NEW_DAY = 0x01, // "Hey, everybody! It's the start of a new day, ..."
};

} // namespace dAGrowupMsg

class daGrowupMsg_c : public fBase_c, public dMsg::Rcpt_c {
  public:
    // mStep (execute).
    enum {
        STEP_WAIT_FADE, // waits for the fade, then starts the message
        STEP_OPEN,      // the message window is opening
        STEP_SHOW,      // the message is shown, until the window closes
        STEP_DONE,      // the scene change is requested
    };

    virtual int create();
    virtual int doDelete();
    virtual int execute();

    /* 0xC8 */ int mStep;
}; // size 0xCC
