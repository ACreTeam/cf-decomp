#pragma once

#include <game/game/d_actor.hpp>
#include <game/cLib/c_line.hpp>

class dDemoActor_c;

// Demo actors use their own ID-bearing nodes, independent of fBase's lists.
// Node/list names and behavioral method names are inferred from the binary.
class dDemoActorNode_c : public cLineNd_c {
public:
    dDemoActorNode_c() : mID(0) {}
    u32 mID;
    dDemoActor_c *mpActor;
};

class dDemoActorList_c : public cLineMg_c {
public:
    void init();
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
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void demoHook50();
    virtual mVec3_c *getDemoPosition();
    virtual int demoHook58();
    virtual int demoHook5C();
    virtual int demoHook60();
    virtual int demoHook64();

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
    int getDemoState();
    void setDemoFlag8();
    void clearDemoFlag8();
    bool hasDemoFlag8();
    void setDemoFlag4();
    bool hasDemoFlag4();
    bool hasDemoFlags(u32 flags);
    void setDemoFlags(u32 flags);
    void clearDemoFlags(u32 flags);
    void attachDemoActor(dDemoActor_c *actor);
    void detachDemoActor(dDemoActor_c *actor);

    static dDemoActorList_c mActorList;
    static const s16 mInteractionAngle; // 80750538; inferred name, shared with RELs

protected:
    dDemoActorNode_c mDemoNode; // 0xA0
    float mInteractionRadiusSquared; // 0xB0
    float mInteractionHeight; // 0xB4
    u16 mDemoFlags; // 0xB8
};
