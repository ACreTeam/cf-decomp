// Deferred save callbacks share one weak code section.
// 801C30C0
inline void dSvMgr_c::stepLoad_c::initializeLoad() {
    nextStep();
}

// 801C3158
inline void dSvMgr_c::stepLoad_c::startLoad() {
    if (dNand::loadAll()) {
        nextStep();
    }
}

// 801C31FC
inline void dSvMgr_c::stepLoad_c::waitLoad() {
    if (dNand::isIdle()) {
        nextStep();
    }
}

inline void dSvMgr_c::setLoadErrorMessage(u16 code) {
    sLoadErrorMessage = code;
}

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
    static_cast<Step *>(this)->checkSaveResult();
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
    static_cast<Step *>(this)->checkSaveResult();
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
    static_cast<Step *>(this)->checkSaveResult();
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
    static_cast<Step *>(this)->checkSaveResult();
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
                releaseMessage(demo);
                demo->setNextMsgCode(0x0C);
                demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
                this->mCurrentMethod = &Step::saveFailed;
            } else {
                releaseMessage(demo);
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
    enableReset();
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
            releaseMessage(demo);
            demo->setNextMsgCode(0x0C);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            demo->setNextMsgCode(8);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            demo->setNextMsgCode(9);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            demo->setNextMsgCode(0x13);
            demo->setNextMsgLabel(dSvMgr_c::mMessageLabel);
            step->mCurrentMethod = &Step::saveFailed;
            break;
        }
    }
}

#include "d_sv_mgr_dtors.inc"
