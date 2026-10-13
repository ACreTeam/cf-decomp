#pragma once

#include <game/game/d_state_step.hpp>

// RTTI confirms this wrapper adds no storage or virtual slots beyond its
// destructor. Nonvirtual save callbacks are inferred from the matching emission order.
template <class Owner, class Step>
class dSvStep_c : public dState::step_c<Owner, Step> {
public:
    void initializeLoad();
    void startLoad();
    void waitLoad();
    void initializeVisitorSave();
    void startVisitorSave();
    void waitVisitorSave();
    void initializeFullSave();
    void startFullSave();
    void waitFullSave();
    void initializeHostSave();
    void startHostSave();
    void waitHostSave();
    void initializeTownSave();
    void startTownSave();
    void waitTownSave();
    void initializeDownloadBox();
    void createDownloadBox();
    void disableHomeAndReset();
    void enableHomeAndReset();
    void saveFailed();

    dSvStep_c() {}
    virtual ~dSvStep_c() {}
}; // sizeof = 0x30
