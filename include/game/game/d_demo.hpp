#pragma once

// d_demo (DOL TU d_demo.cpp, .text 8018DAC8..8018F588): the demo process dDemo_c (named by its RTTI "dDemo_c";
// profile 0x129, draw order 0) and the demo request queue (dDemo).
//
// Systems ask for a demo with dDemo::request (talking to an npc, opening a menu, leaving the scene, ...). A
// request is a dNetReqPool::Req_c node, queued by priority on a list that is emptied every frame. Each frame the
// process runs the current demo through its type's functions (l_typeFuncs: check, start, execute, end) and, when
// none runs, picks the first queued request whose check passes. Demos with an actor are kept in step with the
// other consoles of an online session through dDemoSync. The flags (sFlags) gate the day change and the BGM.
//
// The (dDemo) namespace and every name except dDemo_c are inferred. Type names follow the GC game's m_demo
// (ac-decomp mDemo_TYPE_*) where a type matches: GC's talk types TALK, SPEAK, REPORT and SPEECH are the four types
// isTalking checks, in the same order (REPORT / SPEECH by that order only); EXITSCENE likewise.

#include <types.h>
#include <game/framework/f_base.hpp>

class dDemoActor_c;

class dDemo_c : public fBase_c {
public:
    virtual int create();   // 8018DB4C
    virtual int doDelete(); // 8018DB80
    virtual int execute();  // 8018DB88
    virtual int draw();     // 8018DC2C

    void startCurrent(); // 8018F230: REQ_STATE_START: runs the start function, then REQ_STATE_EXECUTE
    void execCurrent();  // 8018F29C: REQ_STATE_EXECUTE: runs the execute function, then REQ_STATE_END
    void selectNext();   // 8018F1A0: no current demo: the first queued request whose check passes
    void endCurrent();   // 8018F308: REQ_STATE_END: runs the end function, then frees the request
    void clearCurrent(); // 8018F380
}; // size 0x64

namespace dNetReqPool {
class Req_c;
}

namespace dDemo {

typedef dNetReqPool::Req_c Req_c;

enum Type_e {
    TYPE_NONE,           // never starts
    TYPE_MENU,           // a menu opened from a message (mParam: the menu number); requestMenu
    TYPE_TALK,           // the player talks to an actor (DEMO_CMD_TALK: canInteract); requestTalk
    TYPE_SPEAK,          // an actor speaks to the player (DEMO_CMD_SPEAK: canSpeak, no range check); requestSpeak
    TYPE_SCENE_START,    // the scene start (no building); startScene
    TYPE_EXITSCENE,      // leaving the scene by an exit (mParam: the exit index); requestExitScene
    TYPE_ENTER,          // the scene start in a building (mSrcID: the building code); startScene
    TYPE_7,              // a player demo (d_sv_demo, d_a_c_uitag); request7
    TYPE_REPORT,         // DEMO_CMD_REPORT (no actor check); requestReport
    TYPE_9,              // d_player_mgr; request9
    TYPE_10,             // d_player_mgr; request10
    TYPE_NET_SAVE,       // the online save; startNetSave
    TYPE_DAY_CHANGE,     // waits for the player to be free, then sets FLAG_DAY_CHANGE_READY; requestDayChange
    TYPE_EVENT,          // d_event_mng; requestEvent
    TYPE_UITAG,          // d_a_c_uitag; requestUitag
    TYPE_KEYBOARD,       // d_obj_keyboard; requestKeyboard
    TYPE_SPEECH,         // DEMO_CMD_SPEECH: canSpeech; requestSpeech
    TYPE_NUM,
    TYPE_ANY = TYPE_NUM, // isType: any demo
};

// Req_c::mState
enum ReqState_e {
    REQ_STATE_FREE,
    REQ_STATE_QUEUED,
    REQ_STATE_START,
    REQ_STATE_EXECUTE,
    REQ_STATE_END,
};

// Req_c::mStep
enum Step_e {
    STEP_INIT,
    STEP_DONE,          // finished (finish, finishTalk, ...)
    STEP_REJECTED,      // the host rejected the demo
    STEP_START,         // run the target's start check
    STEP_WAIT_PLAYER,   // waiting for the player (dPlayerMgr::canFinishToolAction)
    STEP_RUN,           // running
    STEP_ENTER,         // TYPE_ENTER: ask for the building's demo slot
    STEP_ENTER_WAIT,    // TYPE_ENTER: waiting for the host's answer
    STEP_ENTER_START,   // TYPE_ENTER: start the building's demo
    STEP_WAIT_SYNC,     // waiting for the host's answer (dDemoSync)
};

// sFlags
enum Flag_e {
    FLAG_HOLD_START = 0x1,        // TYPE_SCENE_START / TYPE_ENTER may not finish
    FLAG_DAY_CHANGE_WARP = 0x2,   // the day change warp is under way (d_field_assessment)
    FLAG_DAY_CHANGE_READY = 0x4,  // TYPE_DAY_CHANGE got the player: the day may change
    FLAG_SILENCE = 0x8,           // the BGM is muted (d_bgm Silence_c); set by a scene start during the warp
    FLAG_NO_DAY_CHANGE = 0x10,    // requestDayChange does nothing (d_a_c_uitag)
};

BOOL isFlag(u16 flag);  // 8018DB14
void onFlag(u16 flag);  // 8018DB2C
void offFlag(u16 flag); // 8018DB3C

BOOL requestMenu(u8 menu);                              // 8018DD24 (d_a_c_uitag)
void finishMenu();                                      // 8018DD7C (d_a_c_uitag)
BOOL requestPlayerDemo(int type);                       // 8018DD8C: a demo of the scene player
BOOL isActive(int type);                                // 8018DDE4: a demo of this type runs and isn't finished
BOOL finish(int type);                                  // 8018DE2C
void sendCommand(dDemoActor_c *actor, u8 cmd);          // 8018E010: to the actor here and on the other consoles
BOOL requestTalk(dDemoActor_c *src, dDemoActor_c *target); // 8018E428
BOOL finishTalk(dDemoActor_c *actor);                   // 8018E48C (d_a_snowman)
BOOL requestSpeak(dDemoActor_c *actor);                 // 8018E560 (d_a_snowman)
void reset();                                           // 8018E674 (d_reset soft reset)
void startScene();                                      // 8018E6A0
BOOL finishSceneStart();                                // 8018E734
void onHoldStart();                                     // 8018E79C
void offHoldStart();                                    // 8018E7A4
BOOL isHoldStart();                                     // 8018E7AC
BOOL requestExitScene(dDemoActor_c *actor, int exit);   // 8018E85C
BOOL request7();                                        // 8018EA68 (d_a_c_uitag, d_sv_demo)
BOOL isActive7();                                       // 8018EA70 (d_sv_demo)
BOOL finish7();                                         // 8018EA78 (d_sv_demo)
BOOL requestReport(dDemoActor_c *actor);                // 8018EAE4
BOOL request9();                                        // 8018EB8C (d_player_mgr)
BOOL finish9();                                         // 8018EBD8
BOOL isActive9();                                       // 8018EC18
BOOL request10();                                       // 8018ECA0 (d_player_mgr)
BOOL startNetSave();                                    // 8018ED3C (d_net, the save step)
BOOL endNetSave();                                      // 8018EDE4 (d_net)
BOOL requestDayChange();                                // 8018EE7C (d_field_assessment)
void onDayChangeReady();                                // 8018EEE4
BOOL isDayChangeReady();                                // 8018EEEC (d_field_assessment)
void startDayChangeWarp();                              // 8018EEF4 (d_field_assessment)
BOOL isDayChangeWarp();                                 // 8018EF20
void offDayChangeWarp();                                // 8018EF28 (d_field_assessment)
BOOL isSilence();                                       // 8018EF30 (d_bgm)
void offSilence();                                      // 8018EF38
BOOL requestEvent();                                    // 8018EF98 (d_event_mng)
BOOL isActiveEvent();                                   // 8018EFA0 (d_event_mng)
BOOL finishEvent();                                     // 8018EFA8 (d_event_mng)
BOOL isNoDayChange();                                   // 8018EFB0 (d_a_c_uitag)
BOOL requestUitag();                                    // 8018F010 (d_a_c_uitag)
BOOL isActiveUitag();                                   // 8018F018 (d_a_c_uitag)
BOOL finishUitag();                                     // 8018F020 (d_a_c_uitag)
BOOL requestKeyboard();                                 // 8018F028 (d_obj_keyboard)
BOOL isActiveKeyboard();                                // 8018F030 (d_obj_keyboard)
BOOL finishKeyboard();                                  // 8018F038 (d_obj_keyboard)
BOOL requestSpeech(dDemoActor_c *src, dDemoActor_c *target); // 8018F114
void initList();                                        // 8018F16C
BOOL request(u32 srcID, u32 targetID, u8 type, u8 priority, u8 param); // 8018F3AC
BOOL isType(u8 type);                                   // 8018F438: the current demo is of this type (TYPE_ANY: any)
dDemoActor_c *getTalkActor();                           // 8018F470: the actor of the running talk demo
BOOL isTalking();                                       // 8018F4A8: a talk demo runs (d_money)

} // namespace dDemo
