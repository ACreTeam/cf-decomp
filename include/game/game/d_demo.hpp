#pragma once

class dDemoActor_c;

// Actor binding and message-state callbacks; the demo's layout is unrecovered.
class dDemo_c {
public:
    void attachActor(dDemoActor_c *actor);
    void detachActor();
    void fn_801A4518(); // Message-lock state.
    void fn_801A334C(); // Message-end state.
    static dDemo_c *mInstance;
};
