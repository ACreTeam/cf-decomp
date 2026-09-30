#pragma once

class dDemoActor_c;

// Only the actor-binding interface is recovered here; the demo's layout is not.
class dDemo_c {
public:
    void attachActor(dDemoActor_c *actor);
    void detachActor();
    static dDemo_c *mInstance;
};
