#include <lib/textinput/tiPcKeyboard.h>
#include <lib/textinput/tiCellPhone.h>
#include <lib/textinput/tiCandidateBox.h>
#include <lib/textinput/tiInputForm.h>
#include <lib/textinput/tiToolBar.h>
#include <lib/textinput/tiPredictLang.h>
#include <lib/textinput/tiSignWindow.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiHwKeyboard.h>
#include <revolution/gx.h>
#include <revolution/sc.h>
#include <new>

namespace textinput {

static EventObserver sDefaultEventObserver;

Manager::Manager(MEMAllocator *allocator,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 EventObserver *event)
    : mpAllocator(allocator), mpMultiArcResourceAccessor(multiArc),
      mpHWKeyboard(NULL), mpPCKeyboard(NULL), mpCellPhoneKeyboard(NULL),
      mpInputForm(NULL), mpCandidateBox(NULL), mpToolBar(NULL),
      mpPredictLanguageDialog(NULL), mpSignWindow(NULL), mpEventObserver(event),
      meDestination(DST_JP), meLanguage(JP), mbAspectRatio4x3(true) {
    if (mpEventObserver == NULL) {
        mpEventObserver = &sDefaultEventObserver;
    }
}

Manager::~Manager() {
    if (mpPCKeyboard != NULL) {
        mpPCKeyboard->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpPCKeyboard);
    }
    if (mpCellPhoneKeyboard != NULL) {
        mpCellPhoneKeyboard->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpCellPhoneKeyboard);
    }
    if (mpInputForm != NULL) {
        mpInputForm->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpInputForm);
    }
    if (mpCandidateBox != NULL) {
        mpCandidateBox->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpCandidateBox);
    }
    if (mpToolBar != NULL) {
        mpToolBar->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpToolBar);
    }
    if (mpPredictLanguageDialog != NULL) {
        mpPredictLanguageDialog->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpPredictLanguageDialog);
    }
    if (mpSignWindow != NULL) {
        mpSignWindow->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpSignWindow);
    }
    if (mpHWKeyboard != NULL) {
        mpHWKeyboard->~HWKeyboard();
        MEMFreeToAllocator(mpAllocator, mpHWKeyboard);
    }
}

void Manager::create(MEMAllocator *allocator) {
    mpHWKeyboard = createHWKeyboard();
    mpPCKeyboard = createPCTypeKeyboard();
    mpPCKeyboard->create(mpAllocator);
    mpCellPhoneKeyboard = createCellPhoneTypeKeyboard();
    mpCellPhoneKeyboard->create(mpAllocator);
    mpInputForm = createInputForm();
    mpInputForm->create(mpAllocator);
    mpCandidateBox = createCandidateBox();
    mpCandidateBox->create(mpAllocator);
    mpToolBar = createToolBar();
    mpToolBar->create(mpAllocator);
    mpPredictLanguageDialog = createPredictLanguageDialog();
    mpPredictLanguageDialog->create(mpAllocator);
    mpSignWindow = createSignWindow();
    mpSignWindow->create(mpAllocator);
    initAspect();
    init();
}

void Manager::initAspect() {
    if (SCGetAspectRatio() == SC_ASPECT_RATIO_16x9) {
        setLayoutScaleFor16x9();
        mbAspectRatio4x3 = false;
    } else {
        setLayoutScaleFor4x3();
        mbAspectRatio4x3 = true;
    }
}

void Manager::init() {
    input::HKBManager::sInstance.ClearState();
    mpInputForm->clearSender();
    mpHWKeyboard->setCommandReceiver(mpInputForm);
    mpPCKeyboard->setCommandReceiver(mpInputForm);
    mpCellPhoneKeyboard->setCommandReceiver(mpInputForm);
    mpCandidateBox->setCommandReceiver(mpInputForm);
    mpPredictLanguageDialog->setCommandReceiver(mpInputForm);
    mpSignWindow->setCommandReceiver(mpInputForm);
    mpInputForm->setCandidateBox(mpCandidateBox);
    mpPCKeyboard->setPredictLanguageDialog(mpPredictLanguageDialog);
    mpPCKeyboard->setSignWindow(mpSignWindow);
    mpCellPhoneKeyboard->setPredictLanguageDialog(mpPredictLanguageDialog);
    mpCellPhoneKeyboard->setSignWindow(mpSignWindow);
    if (mbAspectRatio4x3 == true) {
        setLayoutScaleFor4x3();
    } else {
        setLayoutScaleFor16x9();
    }
    mpHWKeyboard->init();
    mpPCKeyboard->init();
    mpCellPhoneKeyboard->init();
    mpInputForm->init();
    mpCandidateBox->init();
    mpToolBar->init();
    mpPredictLanguageDialog->init();
    mpSignWindow->init();
    mpPredictLanguageDialog->setDestination(meDestination);
    mpInputForm->setDestination(meDestination);
    mpInputForm->setLanguage(meLanguage);
    mpPCKeyboard->setLanguage(meLanguage);
    mpCellPhoneKeyboard->setLanguage(meLanguage);
    mpSignWindow->setLanguage(meLanguage);
    mpCandidateBox->setLanguage(meLanguage);
    mpToolBar->setLanguage(meLanguage);
    mpPredictLanguageDialog->setLanguage(meLanguage);
    mpHWKeyboard->setLanguage(meDestination, meLanguage);
    setFixedPredictionJP(0, NULL);
    setSecretInputMode(false);
}

void Manager::calc() {
    if (mpToolBar->isQwerty()) {
        mpPCKeyboard->calc();
    } else {
        mpCellPhoneKeyboard->calc();
    }
    mpToolBar->calc();
    mpCandidateBox->calc();
    mpInputForm->calc();
    if (mpPredictLanguageDialog->isActive()) {
        mpPredictLanguageDialog->calc();
    }
    if (mpSignWindow->isActive()) {
        mpSignWindow->calc();
    }
}

void Manager::draw() {
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    mpToolBar->draw();
    mpInputForm->draw();
    if (mpToolBar->isQwerty()) {
        mpPCKeyboard->draw();
    } else {
        mpCellPhoneKeyboard->draw();
    }
    mpCandidateBox->draw();
    if (mpPredictLanguageDialog->isActive()) {
        mpPredictLanguageDialog->draw();
    }
    if (mpSignWindow->isActive()) {
        mpSignWindow->draw();
    }
}

bool Manager::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                          u32 release) {
    bool keyboard = false;
    bool qwerty = mpToolBar->isQwerty();
    nw4rmanager::TiEventHandler::Input input = {chan, x,    y,
                                                trig, hold, release};
    if (mpPredictLanguageDialog->isActive()) {
        return mpPredictLanguageDialog->updateInput(chan, x, y, trig, hold,
                                                    release, &input);
    }
    mpInputForm->updateInputCommon(chan, trig, hold, release, &input);
    if (mpSignWindow->isActive()) {
        return mpSignWindow->updateInput(chan, x, y, trig, hold, release,
                                         &input);
    }
    if (qwerty) {
        keyboard =
            mpPCKeyboard->updateInput(chan, x, y, trig, hold, release, &input);
    } else {
        keyboard = mpCellPhoneKeyboard->updateInput(chan, x, y, trig, hold,
                                                    release, &input);
    }
    bool toolBar =
        mpToolBar->updateInput(chan, x, y, trig, hold, release, &input);
    bool candidateBox =
        mpCandidateBox->updateInput(chan, x, y, trig, hold, release, &input);
    bool inputForm =
        mpInputForm->updateInput(chan, x, y, trig, hold, release, &input);
    keyboard = (bool)(toolBar | candidateBox | inputForm | keyboard);
    if (qwerty != mpToolBar->isQwerty()) {
        getInputForm()->onCommand((CommandReceiver::INPUT_COMMAND)6, NULL);
        if (!mpToolBar->isQwerty() &&
            mpCellPhoneKeyboard->getInputMode() == 3) {
            mpCandidateBox->setInvalid(true);
        } else {
            mpCandidateBox->setInvalid(false);
        }
        if (mpToolBar->isQwerty()) {
            mpPCKeyboard->onActive();
        } else {
            mpCellPhoneKeyboard->onActive();
        }
    }
    return keyboard;
}

bool Manager::updateInput(input::HKBManager &hkbManager) {
    bool keyboard = false;
    bool qwerty = mpToolBar->isQwerty();
    if (mpPredictLanguageDialog->isActive()) {
        return mpPredictLanguageDialog->updateInput(hkbManager);
    }
    if (mpSignWindow->isActive()) {
        return mpSignWindow->updateInput(hkbManager);
    }
    if (qwerty) {
        keyboard = mpPCKeyboard->updateInput(hkbManager);
    } else {
        keyboard = mpCellPhoneKeyboard->updateInput(hkbManager);
    }
    bool toolBar = mpToolBar->updateInput(hkbManager);
    bool candidateBox = mpCandidateBox->updateInput(hkbManager);
    bool inputForm = mpInputForm->updateInput(hkbManager);
    keyboard = (bool)(toolBar | candidateBox | inputForm | keyboard);
    if (qwerty != mpToolBar->isQwerty()) {
        getInputForm()->onCommand((CommandReceiver::INPUT_COMMAND)6, NULL);
        if (!mpToolBar->isQwerty() &&
            mpCellPhoneKeyboard->getInputMode() == 3) {
            mpCandidateBox->setInvalid(true);
        } else {
            mpCandidateBox->setInvalid(false);
        }
        if (mpToolBar->isQwerty()) {
            mpPCKeyboard->onActive();
        } else {
            mpCellPhoneKeyboard->onActive();
        }
    }
    return keyboard;
}

wchar_t *Manager::getWCString() const {
    return mpInputForm->getWCString();
}

void Manager::setWCString(const wchar_t *string) {
    mpInputForm->inputform::Base::setString(string);
}

void Manager::setLanguage(Language language) {
    meLanguage = language;
}

void Manager::setDestination(Destination destination) {
    meDestination = destination;
}

void Manager::limitStringLength(u32 limitStringLength) {
    mpInputForm->limitStringLength(limitStringLength);
}

void Manager::limitRowNum(u32 limitRowNum) {
    mpInputForm->limitRowNum(limitRowNum);
}

void Manager::setAnimationOn(bool flag) {
    mpPCKeyboard->setAnimOn(flag);
    mpCellPhoneKeyboard->setAnimOn(flag);
    mpInputForm->setAnimOn(flag);
    mpCandidateBox->setAnimOn(flag);
    mpToolBar->setAnimOn(flag);
    mpPredictLanguageDialog->setAnimOn(flag);
    mpSignWindow->setAnimOn(flag);
}

void Manager::setAspectRatio(bool b4x3) {
    mbAspectRatio4x3 = b4x3;
    if (mbAspectRatio4x3 == true) {
        setLayoutScaleFor4x3();
    } else {
        setLayoutScaleFor16x9();
    }
}

void Manager::setTitleText(const wchar_t *titleText) {
    nw4r::lyt::TextBox *textBox = nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(
        mpInputForm->getPane("T_title_text"));
    if (textBox != NULL) {
        textBox->SetString(titleText, 0);
    }
}

void Manager::setLayoutScaleFor16x9() {
    mpPCKeyboard->setRootPaneScaleFor16x9();
    mpCellPhoneKeyboard->setRootPaneScaleFor16x9();
    mpInputForm->setRootPaneScaleFor16x9();
    mpCandidateBox->setRootPaneScaleFor16x9();
    mpToolBar->setRootPaneScaleFor16x9();
    mpPredictLanguageDialog->setRootPaneScaleFor16x9();
    mpSignWindow->setRootPaneScaleFor16x9();
}

void Manager::setLayoutScaleFor4x3() {
    mpPCKeyboard->setRootPaneScaleFor4x3();
    mpCellPhoneKeyboard->setRootPaneScaleFor4x3();
    mpInputForm->setRootPaneScaleFor4x3();
    mpCandidateBox->setRootPaneScaleFor4x3();
    mpToolBar->setRootPaneScaleFor4x3();
    mpPredictLanguageDialog->setRootPaneScaleFor4x3();
    mpSignWindow->setRootPaneScaleFor4x3();
}

keyboard::hwkey::HWKeyboard *Manager::createHWKeyboard() {
    void *p =
        MEMAllocFromAllocator(mpAllocator, sizeof(keyboard::hwkey::HWKeyboard));
    return new (p) keyboard::hwkey::HWKeyboard(this);
}

keyboard::pctype::LayoutByNW4R *Manager::createPCTypeKeyboard() {
    void *p =
        MEMAllocFromAllocator(mpAllocator, sizeof(keyboard::pctype::Sample));
    return new (p)
        keyboard::pctype::Sample(this, mpMultiArcResourceAccessor,
                                 "fs_VK_ascii_keytop_a.brlyt", mpEventObserver);
}

keyboard::cellphonetype::LayoutByNW4R *Manager::createCellPhoneTypeKeyboard() {
    void *p = MEMAllocFromAllocator(mpAllocator,
                                    sizeof(keyboard::cellphonetype::Sample));
    return new (p) keyboard::cellphonetype::Sample(
        this, mpMultiArcResourceAccessor, "fs_VK_cellPhone_a.brlyt",
        mpEventObserver);
}

inputform::LayoutByNW4R *Manager::createInputForm() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(inputform::Sample));
    return new (p) inputform::Sample(this, mpMultiArcResourceAccessor,
                                     "fs_VK_textBox_a.brlyt", mpEventObserver,
                                     "WiiBitmapFontType1.brfnt");
}

candidatebox::LayoutByNW4R *Manager::createCandidateBox() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(candidatebox::Sample));
    return new (p)
        candidatebox::Sample(this, mpMultiArcResourceAccessor,
                             "fs_VK_predictInput_a.brlyt", mpEventObserver);
}

toolbar::LayoutByNW4R *Manager::createToolBar() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(toolbar::Sample));
    return new (p) toolbar::Sample(this, mpMultiArcResourceAccessor,
                                   "fs_VK_toolbar_a.brlyt", mpEventObserver);
}

predictlang::LayoutByNW4R *Manager::createPredictLanguageDialog() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(predictlang::Sample));
    return new (p)
        predictlang::Sample(this, mpMultiArcResourceAccessor,
                            "fs_prdicSelWidw_a.brlyt", mpEventObserver);
}

keyboard::signwindow::LayoutByNW4R *Manager::createSignWindow() {
    void *p = MEMAllocFromAllocator(mpAllocator,
                                    sizeof(keyboard::signwindow::Sample));
    return new (p)
        keyboard::signwindow::Sample(this, mpMultiArcResourceAccessor,
                                     "fs_signWindow_a.brlyt", mpEventObserver);
}

void Manager::startPredictTurnOn(bool on) {
    if (on != getInputForm()->mbPredictOn) {
        getCandidateBox()->startTurnOn(on);
    }
}

bool Manager::isPredictTurning() const {
    return getCandidateBox()->getOnOffButton().IsTurning();
}

input::KBDListener *Manager::getKBDListenerRoot() {
    return input::HKBManager::sInstance.getListenerRoot();
}

void Manager::end() {
}

void Manager::start() {
}

} // namespace textinput
