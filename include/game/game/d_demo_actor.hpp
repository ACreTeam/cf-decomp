#pragma once

#include <game/game/d_actor.hpp>
#include <game/game/d_line.hpp>

class dDemoActor_c;
namespace dMsg {
class Rcpt_c;
}

// Commands a demo actor receives (onDemoCmd), here and, for net-synced actors, on the scene owner's console
// (dDemoSync). The talk kinds are also the start check a talk demo asks for (dNetReqPool::Req_c::mCheck,
// dDemoSync::sendStart); the names follow the GC game's m_demo talk types.
enum dDemoCmd_e {
    DEMO_CMD_TALK = 1,   // the player talks to the actor (check: canInteract)
    DEMO_CMD_SPEAK = 2,  // the actor speaks to the player (check: canSpeak)
    DEMO_CMD_SPEECH = 3, // check: canSpeech
    DEMO_CMD_START = 4,  // the start check passed (DEMO_SYNC_START)
    DEMO_CMD_EXIT = 5,   // the player leaves the scene (dDemo TYPE_EXITSCENE)
    DEMO_CMD_ENTER = 7,  // the player arrives in the actor's building (dDemo TYPE_ENTER)
    DEMO_CMD_REPORT = 8, // no actor check
    DEMO_CMD_END = 9,    // the talk demo ended
};

// Demo actors use their own ID-bearing nodes, independent of fBase's lists.
// Node/list names and behavioral method names are inferred from the binary.
class dDemoActorNode_c : public cLineNd_c {
public:
    dDemoActorNode_c() : mID(0) {}
    u32 mID;
    dDemoActor_c *mpActor;
};

class dDemoActorList_c : public dLineMg_c {
public:
    bool add(dDemoActorNode_c *node);
    dDemoActorNode_c *find(u32 id);
};

class dDemoActor_c : public dActor_c {
public:
    // TU: 80081868..80081F04. RTTI: 807498C8. Vtable: 804D5938.
    dDemoActor_c();
    virtual ~dDemoActor_c();
    virtual int preCreate();
    virtual void postCreate(MAIN_STATE_e state);
    virtual int preDelete();
    virtual int preExecute();

    // Additional vtable slots, in binary order. Unknown hooks retain slot names;
    // their signatures are provisional where only a zero-return stub is known.
    virtual bool canInteract(dDemoActor_c *actor); // DEMO_CMD_TALK start check (in range and facing: checkInteraction)
    virtual void onDemoCmd(int cmd, u8 member); // dDemoCmd_e (dDemoSync::receive, d_demo); member 4 = this console
    virtual mVec3_c *getDemoPosition();
    virtual mVec3_c *demoHook58(); // a position of the actor (dAcNpc_c: look/talk target)
    virtual int canSpeech(dDemoActor_c *actor); // DEMO_CMD_SPEECH start check (dDemo TYPE_SPEECH)
    virtual int canSpeak(dDemoActor_c *actor); // DEMO_CMD_SPEAK start check (dDemo TYPE_SPEAK): no range check
    virtual int demoHook64(mVec3_c *pos); // dAcNpc_c: writes a position of the npc

    static void initActorList();
    u32 getDemoID();
    void setDemoID(u32 id);
    static dDemoActor_c *findByDemoID(u32 id);
    static dDemoActor_c *findInteraction(dDemoActor_c *actor);
    bool checkInteractionAngle(dDemoActor_c *actor, int minAngle, int maxAngle); // 80081B30
    bool checkInteractionRange(dDemoActor_c *actor); // 80081C0C
    bool checkInteraction(dDemoActor_c *actor);

    void setInteractionRadius(float radius);
    void setInteractionHeight(float height);
    void setDemoState1();
    void setDemoState2();
    void clearDemoState();
    u8 getDemoState();
    void setDemoFlag8();
    void clearDemoFlag8();
    bool hasDemoFlag8();
    void setDemoNetSync(); // flag 4: run by the scene owner's console (dAcNpc_c); dDemo asks it to start demos
    bool isDemoNetSync();
    bool hasDemoFlags(u32 flags);
    void setDemoFlags(u32 flags);
    void clearDemoFlags(u32 flags);
    // Binds / unbinds a message receiver (dMsg::Rcpt_c) to the message controller.
    void attachDemoActor(dMsg::Rcpt_c *rcpt);
    void detachDemoActor(dMsg::Rcpt_c *rcpt);

    static dDemoActorList_c mActorList;
    static const s16 mInteractionAngle; // 80750538; inferred name, shared with RELs

protected:
    dDemoActorNode_c mDemoNode; // 0xA0
    float mInteractionRadiusSquared; // 0xB0
    float mInteractionHeight; // 0xB4
    u16 mDemoFlags; // 0xB8
};
