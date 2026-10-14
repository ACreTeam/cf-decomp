#pragma once

// The save manager's steps: the load and save step callbacks (d_sv_mgr, d_s_boot_static).

#include <game/game/d_state_step.hpp>
#include <game/game/d_sv_runtime.hpp>
#include <game/game/d_wifi.hpp>
#include <game/game/d_date.hpp>

class dSvMgr_c;

template <class Step> void checkSaveResultImpl(Step *step);

// RTTI confirms this wrapper adds no storage or virtual slots beyond its
// destructor. Nonvirtual save callbacks are inferred from the matching emission order.
template <class Owner, class Step>
class dSvStep_c : public dState::step_c<Owner, Step> {
public:
    friend void checkSaveResultImpl<Step>(Step *step);

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

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeLoad() {
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::startLoad() {
    if (dNand::loadAll()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::waitLoad() {
    if (dNand::isIdle()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeVisitorSave() {
    dSaveOption::storeToCurrentPlayer();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::startVisitorSave() {
    if (dNand::savePlayer()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::waitVisitorSave() {
    checkSaveResultImpl(static_cast<Step *>(this));
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeFullSave() {
    dSaveData_c::get()->prepareSave();
    dTime_c::saveOffset();
    dNand::updateCheckFlags();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::startFullSave() {
    if (dNand::saveAll()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::waitFullSave() {
    checkSaveResultImpl(static_cast<Step *>(this));
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeHostSave() {
    dSaveOption::storeToCurrentPlayer();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::startHostSave() {
    if (dNand::savePlayer()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::waitHostSave() {
    checkSaveResultImpl(static_cast<Step *>(this));
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeTownSave() {
    dSaveData_c::get()->prepareSave();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::startTownSave() {
    if (dNand::saveTown()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::waitTownSave() {
    checkSaveResultImpl(static_cast<Step *>(this));
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::initializeDownloadBox() {
    if (dNand::getType(lbl_8074B010) == 1) {
        this->nextStep();
    } else if (dWifi::startSetupDownload()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::createDownloadBox() {
    if (dNand::getType(lbl_8074B010) == 1) {
        this->nextStep();
    } else {
        dMsg::Comp_c *demo = dMsg::Comp_c::mInstance;
        switch (dWifi::getThreadResult()) {
        case -1:
            break;
        case 0:
            if (dWifi::getNetErrorMessage() == 0x0C) {
                dSvRuntime::releaseMessage(demo);
                demo->setNextMsgCode(0x0C);
                demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
                this->mCurrentMethod = &Step::saveFailed;
            } else {
                dSvRuntime::releaseMessage(demo);
                demo->setNextMsgCode(0x13);
                demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
                this->mCurrentMethod = &Step::saveFailed;
            }
            break;
        case 1:
            this->nextStep();
            break;
        }
    }
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::disableHomeAndReset() {
    dHomeButton_c::getInstance()->mFlags |= dHomeButton_c::FLAG_SAVING;
    dReset::Manage_c::GetInstance()->SetResetDisable();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::enableHomeAndReset() {
    dSvRuntime::enableReset();
    this->nextStep();
}

template <class Owner, class Step>
void dSvStep_c<Owner, Step>::saveFailed() {
}

template <class Step>
void checkSaveResultImpl(Step *step) {
    dMsg::Comp_c *demo = dMsg::Comp_c::mInstance;
    if (dNand::isIdle()) {
        switch (dNand::getResult()) {
        case 1:
            step->nextStep();
            break;
        case 2:
            dSvRuntime::releaseMessage(demo);
            demo->setNextMsgCode(0x0C);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        case 7:
            dSvRuntime::enableReset();
            dSvRuntime::releaseMessage(demo);
            demo->setNextMsgCode(8);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        case 8:
            dSvRuntime::enableReset();
            dSvRuntime::releaseMessage(demo);
            demo->setNextMsgCode(9);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        default:
            dSvRuntime::enableReset();
            dSvRuntime::releaseMessage(demo);
            demo->setNextMsgCode(0x13);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        }
    }
}
