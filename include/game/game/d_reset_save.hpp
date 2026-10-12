#pragma once

// d_reset_save (DOL TU d_reset_save.cpp, .text 80107A28..80107D6C, .rodata 80475CC0, .sbss 8074E640): the network
// shutdown run before a reset or an error return (requested by d_reset, d_caution, d_wifi_err). A step machine:
// once requested (or on a d_wifi_err error / a reset), it closes the network connection, waits for the Mii icon
// thread and NAND, saves the friend list, and ends the session when the scene is deleted. No RTTI: the names are
// inferred.

#include <types.h>

namespace dResetSave {

enum Step_e {
    STEP_WAIT,           // waits for a request, an error or a reset
    STEP_CLOSE_NET,      // dNet::closeNetConnection
    STEP_WAIT_MII_ICON,  // the d_mii_icon thread is idle
    STEP_WAIT_NAND,      // dNand is idle
    STEP_SAVE_FRIENDS,   // saves the friend list (a local player with a save file)
    STEP_WAIT_SAVE,      // dNand is idle again
    STEP_END_SESSION,    // dNet::endSession, run from the scene delete (dScene_c::postDelete)
    STEP_DONE,

    STEP_NUM,
};

BOOL stepWait();         // 80107A28
BOOL stepCloseNet();     // 80107AC0
BOOL stepWaitMiiIcon();  // 80107AFC
BOOL stepWaitNand();     // 80107B38
BOOL stepSaveFriends();  // 80107B74
BOOL stepWaitSave();     // 80107BDC
BOOL stepEndSession();   // 80107C18
BOOL stepDone();         // 80107C48

void calc(BOOL sceneDelete); // 80107C50: runs the steps until one waits
void clear();                // 80107CD4: clears its state (d_reset soft reset)
void execute();              // 80107CE4: the steps but STEP_END_SESSION (d_sys frame)
void executeSceneDelete();   // 80107CEC: STEP_END_SESSION only (dScene_c::postDelete)
BOOL request();              // 80107CF4: requests it; TRUE while it has not started (d_reset, d_caution, d_wifi_err)
BOOL isNetClosed();          // 80107D24: STEP_END_SESSION or later (d_net; d_wifi_err before closing its window)
BOOL isIdle();               // 80107D44: not started, or the network is closed

extern int sStep;        // 8074E640: Step_e
extern bool sRequested;  // 8074E644

} // namespace dResetSave
