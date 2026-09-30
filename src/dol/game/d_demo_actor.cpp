#include <game/game/d_demo.hpp>
#include <game/game/d_demo_actor.hpp>

typedef char dDemoActorSizeCheck[sizeof(dDemoActor_c) == 0xBC ? 1 : -1];

// Shared RTTI at 804A061C/804A062C/807498C8 is emitted earlier in the link
// and remains in its existing data owner.

dDemoActorList_c dDemoActor_c::mActorList;

dDemoActor_c::dDemoActor_c() {
}
dDemoActor_c::~dDemoActor_c() {
}

void dDemoActor_c::initActorList() {
    mActorList.init();
}

int dDemoActor_c::preCreate() {
    if (!dActor_c::preCreate()) {
        return NOT_READY;
    }
    mDemoNode.mpActor = this;
    mDemoNode.mID = 0;
    setInteractionRadius(48.0f);
    setInteractionHeight(14.0f);
    mDemoFlags = 0;
    setDemoState2();
    return SUCCEEDED;
}

void dDemoActor_c::postCreate(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        mActorList.add(&mDemoNode);
    }
    dActor_c::postCreate(state);
}

int dDemoActor_c::preDelete() {
    if (!dActor_c::preDelete()) {
        return NOT_READY;
    }
    mActorList.removeLineNode(&mDemoNode);
    return SUCCEEDED;
}

int dDemoActor_c::preExecute() {
    return dActor_c::preExecute() != NOT_READY;
}
u32 dDemoActor_c::getDemoID() {
    return mDemoNode.mID;
}
void dDemoActor_c::setDemoID(u32 id) {
    mDemoNode.mID = mProfName << 16;
    mDemoNode.mID |= id & 0xFFFF;
}

dDemoActor_c *dDemoActor_c::findByDemoID(u32 id) {
    dDemoActorNode_c *node = mActorList.find(id);
    return node ? node->mpActor : nullptr;
}

dDemoActor_c *dDemoActor_c::findInteraction(dDemoActor_c *actor) {
    dDemoActor_c *candidate;
    for (cLineNd_c *node = mActorList.getFirst(); node;
         node = node->getNext()) {
        candidate = static_cast<dDemoActorNode_c *>(node)->mpActor;
        if (candidate != actor && candidate->checkInteraction(actor)) {
            return candidate;
        }
    }
    return nullptr;
}

bool dDemoActor_c::checkInteractionAngle(dDemoActor_c *actor, int minAngle,
                                         int maxAngle) {
    mVec3_c origin = *actor->getDemoPosition();
    mVec3_c target = *getDemoPosition();
    s16 angle = sLib::angleDiff((u16)actor->mAngle.y, targetAngleY(&origin, &target));
    return angle >= minAngle && angle <= maxAngle;
}

const s16 dDemoActor_c::mInteractionAngle = 0x2AAA;

bool dDemoActor_c::checkInteractionRange(dDemoActor_c *actor) {
    if (mInteractionRadiusSquared == 0.0f) {
        return true;
    }
    mVec3_c *position = getDemoPosition();
    mVec3_c delta = *actor->getDemoPosition() - *position;
    if (delta.y < -mInteractionHeight || delta.y > mInteractionHeight) {
        return false;
    }
    return delta.x * delta.x + delta.z * delta.z < mInteractionRadiusSquared;
}

bool dDemoActor_c::canInteract(dDemoActor_c *actor) {
    return false;
}

bool dDemoActor_c::checkInteraction(dDemoActor_c *actor) {
    if (checkInteractionRange(actor) &&
        checkInteractionAngle(actor, -mInteractionAngle, mInteractionAngle)) {
        return canInteract(actor);
    }
    return false;
}

void dDemoActor_c::demoHook50() {
}
mVec3_c *dDemoActor_c::getDemoPosition() {
    return &mPos;
}
int dDemoActor_c::demoHook58() {
    return 0;
}
int dDemoActor_c::demoHook5C() {
    return 0;
}
int dDemoActor_c::demoHook60() {
    return 0;
}
int dDemoActor_c::demoHook64() {
    return 0;
}
void dDemoActor_c::setInteractionRadius(float radius) {
    mInteractionRadiusSquared = radius * radius;
}
void dDemoActor_c::setInteractionHeight(float height) {
    mInteractionHeight = height;
}
void dDemoActor_c::setDemoState1() {
    clearDemoFlags(3);
    setDemoFlags(1);
}
void dDemoActor_c::setDemoState2() {
    clearDemoFlags(3);
    setDemoFlags(2);
}
void dDemoActor_c::clearDemoState() {
    clearDemoFlags(3);
}
int dDemoActor_c::getDemoState() {
    if (hasDemoFlags(1)) {
        return 0;
    }
    return hasDemoFlags(2) ? 1 : 2;
}
void dDemoActor_c::setDemoFlag8() {
    setDemoFlags(8);
}
void dDemoActor_c::clearDemoFlag8() {
    clearDemoFlags(8);
}
bool dDemoActor_c::hasDemoFlag8() {
    return hasDemoFlags(8);
}
void dDemoActor_c::setDemoFlag4() {
    setDemoFlags(4);
}
bool dDemoActor_c::hasDemoFlag4() {
    return hasDemoFlags(4);
}
bool dDemoActor_c::hasDemoFlags(u32 flags) {
    return (mDemoFlags & flags) != 0;
}
void dDemoActor_c::setDemoFlags(u32 flags) {
    mDemoFlags |= flags;
}
void dDemoActor_c::clearDemoFlags(u32 flags) {
    mDemoFlags &= ~flags;
}
void dDemoActor_c::attachDemoActor(dDemoActor_c *actor) {
    dDemo_c::mInstance->attachActor(actor);
}
void dDemoActor_c::detachDemoActor(dDemoActor_c *actor) {
    // The target reuses fBase's heap-pointer slot for the actor's demo owner.
    reinterpret_cast<dDemo_c *>(actor->mHeap)->detachActor();
}
