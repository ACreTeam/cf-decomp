#include <lib/textinput/MyTiManager.h>
#include <lib/textinput/MyTiInputForm.h>
#include <lib/textinput/MyTiState.h>
#include <lib/textinput/MyTiBg.h>
#include <lib/textinput/tiHwKeyboard.h>
#include <lib/textinput/tiLanguageIndependentData.h>
#include <lib/textinput/tiUtil.h>
#include <revolution/gx.h>
#include <new>

namespace textinput {
namespace extend {
namespace memo {

struct AnmRange {
    /* 0x0 */ f32 start;
    /* 0x4 */ f32 end;
}; // size 0x8

static AnmRange sKeyTopAnmY = {-200.0f, 0.0f};
static AnmRange sKeyTopAnmA = {0.0f, 255.0f};
static AnmRange sInputFormAnmY = {0.0f, 145.0f};

static DispMemoState sDispMemoState;
static EditMemoState sEditMemoState;
static AppearMemoState sAppearMemoState;
static DisappearMemoState sDisappearMemoState;

Manager::~Manager() {
    if (mpDefaultPCKeyboard != NULL) {
        mpDefaultPCKeyboard->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultPCKeyboard);
    }
    if (mpDefaultCellPhoneKeyboard != NULL) {
        mpDefaultCellPhoneKeyboard->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultCellPhoneKeyboard);
    }
    if (mpDefaultInputForm != NULL) {
        mpDefaultInputForm->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultInputForm);
    }
    if (mpDefaultCandidateBox != NULL) {
        mpDefaultCandidateBox->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultCandidateBox);
    }
    if (mpDefaultToolBar != NULL) {
        mpDefaultToolBar->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultToolBar);
    }
    if (mpDefaultPredictLanguageDialog != NULL) {
        mpDefaultPredictLanguageDialog->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultPredictLanguageDialog);
    }
    if (mpDefaultSignWindow != NULL) {
        mpDefaultSignWindow->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpDefaultSignWindow);
    }
    if (mpMemoInputForm != NULL) {
        mpMemoInputForm->~InputForm();
        MEMFreeToAllocator(mpAllocator, mpMemoInputForm);
    }
    if (mpLetterInputForm != NULL) {
        mpLetterInputForm->~InputForm();
        MEMFreeToAllocator(mpAllocator, mpLetterInputForm);
    }
    if (mpBigTextInputForm != NULL) {
        mpBigTextInputForm->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpBigTextInputForm);
    }
    if (mpBackGround != NULL) {
        mpBackGround->~LayoutByNW4R();
        MEMFreeToAllocator(mpAllocator, mpBackGround);
    }
    mpPCKeyboard = NULL;
    mpCellPhoneKeyboard = NULL;
    mpInputForm = NULL;
    mpCandidateBox = NULL;
    mpToolBar = NULL;
    mpPredictLanguageDialog = NULL;
    mpSignWindow = NULL;
}

void Manager::create(MEMAllocator *allocator) {
    mpHWKeyboard = createHWKeyboard();
    mpDefaultPCKeyboard = createPCTypeKeyboard();
    mpDefaultPCKeyboard->create(mpAllocator);
    mpDefaultCellPhoneKeyboard = createCellPhoneTypeKeyboard();
    mpDefaultCellPhoneKeyboard->create(mpAllocator);
    mpDefaultInputForm = createInputForm();
    mpDefaultInputForm->create(mpAllocator);
    mpDefaultCandidateBox = createCandidateBox();
    mpDefaultCandidateBox->create(mpAllocator);
    mpDefaultToolBar = createToolBar();
    mpDefaultToolBar->create(mpAllocator);
    mpDefaultPredictLanguageDialog = createPredictLanguageDialog();
    mpDefaultPredictLanguageDialog->create(mpAllocator);
    mpDefaultSignWindow = createSignWindow();
    mpDefaultSignWindow->create(mpAllocator);
    mpBigTextInputForm = createBigTextInputForm();
    mpBigTextInputForm->create(mpAllocator);
    mpBackGround = createBG();
    mpBackGround->create(mpAllocator);
    configDefault();
    sDispMemoState.setManager(this);
    sDispMemoState.create();
    sEditMemoState.setManager(this);
    sEditMemoState.create();
    sAppearMemoState.setManager(this);
    sAppearMemoState.create();
    sDisappearMemoState.setManager(this);
    sDisappearMemoState.create();
    initAspect();
    mMemoSetting.init();
    init();
}

void Manager::init() {
    mpCurrentState = &sDispMemoState;
    mpCurrentState->start();
    mpCurrentState->init();
    mpDefaultPCKeyboard->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
    mpDefaultCellPhoneKeyboard->getLayout()->GetRootPane()->SetInfluencedAlpha(
        true);
    mpDefaultInputForm->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
    mpBigTextInputForm->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
    mpDefaultCandidateBox->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
    mpDefaultToolBar->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
    mpBackGround->getLayout()->GetRootPane()->SetInfluencedAlpha(true);
}

void Manager::calc() {
    mpCurrentState->calc();
}

void Manager::draw() {
    mpCurrentState->draw();
}

void Manager::memoDraw() {
    mpCurrentState->memoDraw();
}

bool Manager::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                          u32 release) {
    return mpCurrentState->updateInput(chan, x, y, trig, hold, release);
}

bool Manager::updateInput(input::HKBManager &hkbManager) {
    return mpCurrentState->updateInput(hkbManager);
}

void Manager::changeState(StateTimeLine timeLine) {
    if (timeLine == STL_Transition) {
        setState(mpCurrentState->getStateType());
    }
}

void Manager::setState(StateType stateType) {
    switch (stateType) {
    case ST_Hidden:
        mpEventObserver->onSE(sound::SE_OPEN);
        mpCurrentState->end();
        mpCurrentState = &sAppearMemoState;
        mpCurrentState->start();
        break;
    case ST_Appearing:
        mpEventObserver->onSE(sound::SE_APPEARED);
        mpCurrentState->end();
        mpCurrentState = &sEditMemoState;
        mpCurrentState->start();
        break;
    case ST_Visible:
        mpCurrentState->end();
        mpCurrentState = &sDisappearMemoState;
        mpCurrentState->start();
        break;
    case ST_Disappearing:
        mpEventObserver->onSE(sound::SE_DISAPPEARD);
        mpCurrentState->end();
        mpCurrentState = &sDispMemoState;
        mpCurrentState->start();
        break;
    }
}

InputForm *Manager::createMemoInputForm() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(InputForm));
    return new (p)
        InputForm(this, mpMultiArcResourceAccessor, "fs_VK_textBox_a.brlyt",
                  mpEventObserver, "WiiBitmapFontType2.brfnt");
}

InputForm *Manager::createLetterInputForm() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(letter::InputForm));
    return new (p)
        InputForm(this, mpMultiArcResourceAccessor, "fs_VK_textBox_a.brlyt",
                  mpEventObserver, "WiiBitmapFontType2.brfnt");
}

inputform::LayoutByNW4R *Manager::createInputForm() {
    void *p =
        MEMAllocFromAllocator(mpAllocator, sizeof(inputform::LayoutByNW4R));
    return new (p) inputform::LayoutByNW4R(
        this, mpMultiArcResourceAccessor, "fs_VK_textBox_a.brlyt",
        mpEventObserver, "WiiBitmapFontType1.brfnt");
}

inputform::LayoutByNW4R *Manager::createBigTextInputForm() {
    void *p =
        MEMAllocFromAllocator(mpAllocator, sizeof(inputform::LayoutByNW4R));
    return new (p) inputform::LayoutByNW4R(
        this, mpMultiArcResourceAccessor, "fs_VK_textBox_b.brlyt",
        mpEventObserver, "WiiBitmapFontType1.brfnt");
}

bg::LayoutByNW4R *Manager::createBG() {
    void *p = MEMAllocFromAllocator(mpAllocator, sizeof(bg::LayoutByNW4R));
    return new (p) bg::LayoutByNW4R(mpMultiArcResourceAccessor,
                                    "fs_VK_bg_a.brlyt", mpEventObserver);
}

void Manager::configDefault() {
    meConfigType = CT_Default;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpDefaultInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    mpToolBar->setQwerty(true);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configNumeric() {
    meConfigType = CT_Numeric;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpBigTextInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    mpToolBar->setQwerty(false);
    mpToolBar->enableKeytopChange(false);
    mpCellPhoneKeyboard->doNumericMode(true);
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configNumericWithSeparator() {
    configNumeric();
    meConfigType = CT_NumericWithSeparator;
    mpInputForm->visibleSeparator(true);
}

void Manager::configLetter() {
    meConfigType = CT_Letter;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpMemoInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    Language language = mpToolBar->getLanguage();
    const wchar_t *okCaption = langindependent::cLanguageIndependentString
        [langindependent::LANG_STRID_OK][language];
    const wchar_t *cancelCaption = langindependent::cLanguageIndependentString
        [langindependent::LANG_STRID_BACK][language];
    mpToolBar->setOKButtonVisible(true);
    mpToolBar->setOKButtonCaption(okCaption);
    mpToolBar->setCancelButtonCaption(cancelCaption);
    mpInputForm->doWordWrap(true);
    mpInputForm->enableSpaceByRight(true);
    reflectSaveData();
    limitStringLength(0x100);
    limitRowNum(0x10);
    sInputFormAnmY.end = 145.0f;
}

void Manager::configPhotoLetter() {
    meConfigType = CT_PhotoLetter;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpLetterInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    mpToolBar->setOKButtonVisible(true);
    mpToolBar->setOKButtonCaption(
        langindependent::cLanguageIndependentString
            [langindependent::LANG_STRID_OK][mpToolBar->getLanguage()]);
    mpToolBar->setCancelButtonCaption(
        langindependent::cLanguageIndependentString
            [langindependent::LANG_STRID_BACK][mpToolBar->getLanguage()]);
    mpInputForm->doWordWrap(true);
    mpInputForm->enableSpaceByRight(true);
    reflectSaveData();
    limitStringLength(0x100);
    limitRowNum(0x10);
    sInputFormAnmY.end = 151.0f;
}

void Manager::configNormalWithoutLineFeed() {
    configDefault();
    meConfigType = CT_NormalWithoutLineFeed;
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpCellPhoneKeyboard->setSignWindowButton(false);
    mpPCKeyboard->setSignWindowButton(false);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configNormalBigTextWithoutLineFeed() {
    configDefault();
    mpInputForm = mpBigTextInputForm;
    textinput::Manager::init();
    meConfigType = CT_NormalBigTextWithoutLineFeed;
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpToolBar->enableKeytopChange(true);
    mpToolBar->setQwerty(true);
    mpCellPhoneKeyboard->setSignWindowButton(false);
    mpPCKeyboard->setSignWindowButton(false);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configOnlyQwertyWithoutLineFeedAndSign() {
    meConfigType = CT_OnlyQwertyWithoutLineFeedAndSign;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpDefaultInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    mpToolBar->setQwerty(true);
    mpPCKeyboard->setLangKeyActive(true);
    mpToolBar->enableKeytopChange(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpCellPhoneKeyboard->setSignWindowButton(false);
    mpPCKeyboard->setSignWindowButton(false);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configOnlyQwertyBigTextWithoutLineFeedAndSign() {
    meConfigType = CT_OnlyQwertyBigTextWithoutLineFeedAndSign;
    mpPCKeyboard = mpDefaultPCKeyboard;
    mpCellPhoneKeyboard = mpDefaultCellPhoneKeyboard;
    mpInputForm = mpBigTextInputForm;
    mpCandidateBox = mpDefaultCandidateBox;
    mpToolBar = mpDefaultToolBar;
    mpPredictLanguageDialog = mpDefaultPredictLanguageDialog;
    mpSignWindow = mpDefaultSignWindow;
    textinput::Manager::init();
    mpToolBar->setQwerty(true);
    mpPCKeyboard->setLangKeyActive(true);
    mpToolBar->enableKeytopChange(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpCellPhoneKeyboard->setSignWindowButton(false);
    mpPCKeyboard->setSignWindowButton(false);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configNumericWithDot() {
    configNumeric();
    meConfigType = CT_NumericWithDot;
    mpCellPhoneKeyboard->doNumericWithDotMode(true);
}

void Manager::configNumericBigTextWithDot() {
    configNumericWithDot();
    meConfigType = CT_NumericBigTextWithDot;
}

void Manager::configNormalBigTextWithoutLineFeedWithSign() {
    configDefault();
    mpInputForm = mpBigTextInputForm;
    textinput::Manager::init();
    meConfigType = CT_NormalBigTextWithoutLineFeedWithSign;
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpToolBar->enableKeytopChange(true);
    mpToolBar->setQwerty(true);
    mpCellPhoneKeyboard->setSignWindowButton(true);
    mpPCKeyboard->setSignWindowButton(true);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configNormalWithoutLineFeedWithSign() {
    configDefault();
    textinput::Manager::init();
    meConfigType = CT_NormalBigTextWithoutLineFeedWithSign;
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpPCKeyboard->setLineFeedButton(false);
    mpCandidateBox->setActive(false);
    mpCellPhoneKeyboard->setPredictLanguageButton(false);
    mpPCKeyboard->setPredictLanguageButton(false);
    mpToolBar->enableKeytopChange(true);
    mpToolBar->setQwerty(true);
    mpCellPhoneKeyboard->setSignWindowButton(true);
    mpPCKeyboard->setSignWindowButton(true);
    mpInputForm->doWordWrap(false);
    mpInputForm->enableSpaceByRight(false);
}

void Manager::configPredictWithoutLineFeed() {
    configDefault();
    meConfigType = CT_PredictWithoutLineFeed;
    mpCellPhoneKeyboard->setLineFeedButton(false);
    mpPCKeyboard->setLineFeedButton(false);
}

void Manager::start() {
    switch (meConfigType) {
    case CT_Letter:
    case CT_PhotoLetter:
        static_cast<InputForm *>(mpInputForm)->open();
        break;
    }
}

void Manager::end() {
    switch (meConfigType) {
    case CT_Letter:
    case CT_PhotoLetter:
        static_cast<InputForm *>(mpInputForm)->close();
        break;
    }
}

void Manager::setSaveData_() {
    mMemoSetting.uRevisionAndType = mpToolBar->isQwerty() | 0x10;
    mMemoSetting.uDictionary = mpInputForm->getPredictMode();
    mMemoSetting.uPredictOnOff = mpCandidateBox->isOn();
    mMemoSetting.uSignPage = mpSignWindow->getPage();
    mMemoSetting.uKeitaiUpperCaseJP = mpCellPhoneKeyboard->isUpperCase();
    mMemoSetting.uKeitaiInputMode = mpCellPhoneKeyboard->getInputMode();
    mMemoSetting.uQwertyABC = mpPCKeyboard->isABC();
    mMemoSetting.uReserve2 = 0;
    mMemoSetting.uABCInputMode = mpPCKeyboard->mKeyState.muTranslateMode;
    mMemoSetting.uAIUInputMode = mpPCKeyboard->mKeyState.getKanaMode();
    u32 modifier = input::HKBManager::sInstance.GetModifierState();
    mMemoSetting.uNumLockOff = !(modifier & 0x100);
    mMemoSetting.uReserve3 = 0;
    mMemoSetting.uReserve4 = 0;
}

void Manager::reflectSaveData() {
    switch ((mMemoSetting.uRevisionAndType >> 4) & 0xF) {
    case 1:
        reflectSaveDataRev1();
        break;
    case 0:
    default:
        reflectSaveDataDefault();
        break;
    }
}

void Manager::reflectSaveDataRev1() {
    if ((mMemoSetting.uRevisionAndType & 1) == 1) {
        mpToolBar->setQwerty(true);
    } else {
        mpToolBar->setQwerty(false);
    }
    mpSignWindow->setPage(mMemoSetting.uSignPage);
    if (mMemoSetting.uPredictOnOff == 0) {
        mpCandidateBox->turnOff();
    } else {
        mpCandidateBox->turnOn();
    }
    if (mMemoSetting.uDictionary != 0xFA) {
        mpInputForm->setPredictMode(
            (inputform::Base::PredictMode)mMemoSetting.uDictionary);
    }
    if (meLanguage == JP) {
        mpPCKeyboard->setInputModeJP(mMemoSetting.uQwertyABC,
                                     mMemoSetting.uABCInputMode,
                                     mMemoSetting.uAIUInputMode);
    }
    mpCellPhoneKeyboard->setUpperCaseJP(mMemoSetting.uKeitaiUpperCaseJP);
    mpCellPhoneKeyboard->setInputMode((keyboard::cellphonetype::Base::InputMode)
                                          mMemoSetting.uKeitaiInputMode);
    if (!mpToolBar->isQwerty() && mpCellPhoneKeyboard->getInputMode() == 3) {
        mpCandidateBox->setInvalid(true);
    } else {
        mpCandidateBox->setInvalid(false);
    }
    input::HKBManager::sInstance.SetModifierState(
        mMemoSetting.uNumLockOff == 0 ? 0x100 : 0, 0x100);
}

void Manager::reflectSaveDataDefault() {
    mpToolBar->setQwerty(true);
    mpSignWindow->setPage(0);
    if (meLanguage == JP) {
        mpCandidateBox->turnOn();
    } else {
        mpCandidateBox->turnOff();
    }
    if (meLanguage == JP) {
        mpPCKeyboard->setInputModeJP(false, 1, 0);
    }
    mpCellPhoneKeyboard->setUpperCaseJP(true);
    mpCellPhoneKeyboard->setInputMode(
        (keyboard::cellphonetype::Base::InputMode)2);
    if (!mpToolBar->isQwerty() && mpCellPhoneKeyboard->getInputMode() == 3) {
        mpCandidateBox->setInvalid(true);
    } else {
        mpCandidateBox->setInvalid(false);
    }
    input::HKBManager::sInstance.SetModifierState(0x100, 0x100);
}

void DispMemoState::create() {
}

void DispMemoState::init() {
    mpManager->getConfigType();
}

void DispMemoState::calc() {
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm()->calc();
        break;
    }
}

void DispMemoState::draw() {
}

void DispMemoState::memoDraw() {
    if (mpManager->getConfigType() == Manager::CT_Letter ||
        mpManager->getConfigType() == Manager::CT_PhotoLetter) {
        InputForm()->draw();
    }
}

bool DispMemoState::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                                u32 release) {
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm();
        nw4rmanager::TiEventHandler::Input input = {chan, x,    y,
                                                    trig, hold, release};
        return InputForm()->updateInput(chan, x, y, trig, hold, release,
                                        &input);
        break;
    }
    return false;
}

bool DispMemoState::updateInput(input::HKBManager &hkbManager) {
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm();
        return InputForm()->updateInput(hkbManager);
        break;
    }
    return false;
}

void DispMemoState::start() {
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm()->setEditMode(InputForm::EM_Disp);
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, sInputFormAnmY.start));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(0xFF);
        break;
    default:
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, sKeyTopAnmY.start));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.start);
        break;
    }
    PCKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.start));
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.start));
    CandidateBox()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.start));
    ToolBar()->getDownArea()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.start / 3.0f));
    ToolBar()->getUpArea()->SetTranslate(
        nw4r::math::VEC2(0.0f, -sKeyTopAnmY.start / 3.0f));
    BG()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, 0.0f));
    PCKeyboard()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.start);
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetAlpha(
        sKeyTopAnmA.start);
    CandidateBox()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.start);
    ToolBar()->getDownArea()->SetAlpha(sKeyTopAnmA.start);
    ToolBar()->getUpArea()->SetAlpha(sKeyTopAnmA.start);
    BG()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.start);
}

void AppearMemoState::create() {
}

void AppearMemoState::init() {
}

void AppearMemoState::calc() {
    InputForm()->calc();
    if (ToolBar()->isQwerty()) {
        PCKeyboard()->calc();
    } else {
        CellPhoneKeyboard()->calc();
    }
    if (CandidateBox()->isActive()) {
        CandidateBox()->calc();
    }
    ToolBar()->calc();
    BG()->calc();
    if (30.0f <= mfCounter) {
        mpManager->changeState(Manager::STL_Transition);
    }
    f32 y = util::hermiteInterporation(mfCounter, 0.0f, sKeyTopAnmY.start, 0.0f,
                                       30.0f, sKeyTopAnmY.end, 0.0f);
    PCKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    CandidateBox()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    ToolBar()->getDownArea()->SetTranslate(nw4r::math::VEC2(0.0f, y / 3.0f));
    ToolBar()->getUpArea()->SetTranslate(nw4r::math::VEC2(0.0f, -y / 3.0f));
    BG()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, 0.0f));
    u8 alpha = util::hermiteInterporation(mfCounter, 0.0f, sKeyTopAnmA.start,
                                          0.0f, 30.0f, sKeyTopAnmA.end, 0.0f);
    PCKeyboard()->getLayout()->GetRootPane()->SetAlpha(alpha);
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetAlpha(alpha);
    CandidateBox()->getLayout()->GetRootPane()->SetAlpha(alpha);
    ToolBar()->getDownArea()->SetAlpha(alpha);
    BG()->getLayout()->GetRootPane()->SetAlpha(alpha);
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        f32 inputFormY =
            util::hermiteInterporation(mfCounter, 0.0f, sInputFormAnmY.start,
                                       0.0f, 30.0f, sInputFormAnmY.end, 0.0f);
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, inputFormY));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(0xFF);
        extend::memo::InputForm *form = InputForm();
        form->setScroll(
            util::hermiteInterporation(mfCounter, 0.0f, form->getScrollFrom(),
                                       0.0f, 30.0f, form->getScrollTo(), 0.0f));
        break;
    default:
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, y));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(alpha);
        break;
    }
    mfCounter += 1.0f;
}

void AppearMemoState::draw() {
    if (mpManager->getConfigType() != Manager::CT_Letter &&
        mpManager->getConfigType() != Manager::CT_PhotoLetter) {
        BG()->draw();
    }
    ToolBar()->draw();
    if (mpManager->getConfigType() != Manager::CT_Letter &&
        mpManager->getConfigType() != Manager::CT_PhotoLetter) {
        InputForm()->draw();
    }
    if (CandidateBox()->isActive()) {
        CandidateBox()->draw();
    }
    if (ToolBar()->isQwerty()) {
        PCKeyboard()->draw();
    } else {
        CellPhoneKeyboard()->draw();
    }
}

void AppearMemoState::memoDraw() {
    if (mpManager->getConfigType() == Manager::CT_Letter ||
        mpManager->getConfigType() == Manager::CT_PhotoLetter) {
        InputForm()->draw();
    }
}

bool AppearMemoState::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                                  u32 release) {
    return false;
}

bool AppearMemoState::updateInput(input::HKBManager &hkbManager) {
    return false;
}

void AppearMemoState::start() {
    mfCounter = 0.0f;
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm()->setEditMode(InputForm::EM_Appear);
        break;
    }
}

void EditMemoState::create() {
}

void EditMemoState::init() {
}

void EditMemoState::calc() {
    if (ToolBar()->isQwerty()) {
        PCKeyboard()->calc();
    } else {
        CellPhoneKeyboard()->calc();
    }
    ToolBar()->calc();
    if (CandidateBox()->isActive()) {
        CandidateBox()->calc();
    }
    InputForm()->calc();
    if (PredictLanguageSelectDialog()->isActive()) {
        PredictLanguageSelectDialog()->calc();
    }
    if (SignKeyboard()->isActive()) {
        SignKeyboard()->calc();
    }
    if (mpManager->getConfigType() != Manager::CT_Letter &&
        mpManager->getConfigType() != Manager::CT_PhotoLetter) {
        BG()->calc();
    }
}

void EditMemoState::draw() {
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    if (mpManager->getConfigType() != Manager::CT_Letter &&
        mpManager->getConfigType() != Manager::CT_PhotoLetter) {
        BG()->draw();
    }
    ToolBar()->draw();
    if (mpManager->getConfigType() != Manager::CT_Letter &&
        mpManager->getConfigType() != Manager::CT_PhotoLetter) {
        InputForm()->draw();
    }
    if (CandidateBox()->isActive()) {
        CandidateBox()->draw();
    }
    if (ToolBar()->isQwerty()) {
        PCKeyboard()->draw();
    } else {
        CellPhoneKeyboard()->draw();
    }
    if (PredictLanguageSelectDialog()->isActive()) {
        PredictLanguageSelectDialog()->draw();
    }
    if (SignKeyboard()->isActive()) {
        SignKeyboard()->draw();
    }
}

void EditMemoState::memoDraw() {
    if (mpManager->getConfigType() == Manager::CT_Letter ||
        mpManager->getConfigType() == Manager::CT_PhotoLetter) {
        InputForm()->draw();
    }
}

bool EditMemoState::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                                u32 release) {
    bool qwerty = ToolBar()->isQwerty();
    nw4rmanager::TiEventHandler::Input input = {chan, x,    y,
                                                trig, hold, release};
    if (PredictLanguageSelectDialog()->isActive()) {
        return PredictLanguageSelectDialog()->updateInput(
            chan, x, y, trig, hold, release, &input);
    }
    InputForm()->updateInputCommon(chan, trig, hold, release, &input);
    if (SignKeyboard()->isActive()) {
        return SignKeyboard()->updateInput(chan, x, y, trig, hold, release,
                                           &input);
    }
    bool keyboard;
    if (qwerty) {
        keyboard =
            PCKeyboard()->updateInput(chan, x, y, trig, hold, release, &input);
    } else {
        keyboard = CellPhoneKeyboard()->updateInput(chan, x, y, trig, hold,
                                                    release, &input);
    }
    bool toolBar =
        ToolBar()->updateInput(chan, x, y, trig, hold, release, &input);
    bool candidateBox = false;
    if (CandidateBox()->isActive()) {
        candidateBox = CandidateBox()->updateInput(chan, x, y, trig, hold,
                                                   release, &input);
    }
    bool inputForm = false;
    if (!candidateBox) {
        inputForm =
            InputForm()->updateInput(chan, x, y, trig, hold, release, &input);
    } else {
        InputForm()->updateInput(chan, NW4R_MATH_INF, NW4R_MATH_INF, trig, hold,
                                 release, &input);
    }
    keyboard = (bool)(toolBar | candidateBox | inputForm | keyboard);
    if (qwerty != ToolBar()->isQwerty()) {
        InputForm()->onCommand((CommandReceiver::INPUT_COMMAND)6, NULL);
        if (!ToolBar()->isQwerty() &&
            CellPhoneKeyboard()->getInputMode() == 3) {
            CandidateBox()->setInvalid(true);
        } else {
            CandidateBox()->setInvalid(false);
        }
        if (ToolBar()->isQwerty()) {
            PCKeyboard()->onActive();
        } else {
            CellPhoneKeyboard()->onActive();
        }
    }
    return keyboard;
}

bool EditMemoState::updateInput(input::HKBManager &hkbManager) {
    bool qwerty = ToolBar()->isQwerty();
    if (PredictLanguageSelectDialog()->isActive()) {
        return false;
    }
    HWKeyboard()->updateInput(hkbManager);
    if (SignKeyboard()->isActive()) {
        return SignKeyboard()->updateInput(hkbManager);
    }
    bool result;
    if (qwerty) {
        result = PCKeyboard()->updateInput(hkbManager);
    } else {
        result = CellPhoneKeyboard()->updateInput(hkbManager);
    }
    if (qwerty != ToolBar()->isQwerty()) {
        InputForm()->onCommand((CommandReceiver::INPUT_COMMAND)6, NULL);
        if (!ToolBar()->isQwerty() &&
            CellPhoneKeyboard()->getInputMode() == 3) {
            CandidateBox()->setInvalid(true);
        } else {
            CandidateBox()->setInvalid(false);
        }
        if (ToolBar()->isQwerty()) {
            PCKeyboard()->onActive();
        } else {
            CellPhoneKeyboard()->onActive();
        }
    }
    return result;
}

void EditMemoState::start() {
    if (mpManager->getConfigType() == Manager::CT_Letter ||
        mpManager->getConfigType() == Manager::CT_PhotoLetter) {
        InputForm()->setEditMode(InputForm::EM_Edit);
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, sInputFormAnmY.end));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(0xFF);
    } else {
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, sKeyTopAnmY.end));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.end);
    }
    PCKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.end));
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.end));
    CandidateBox()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.end));
    ToolBar()->getDownArea()->SetTranslate(
        nw4r::math::VEC2(0.0f, sKeyTopAnmY.end / 3.0f));
    ToolBar()->getUpArea()->SetTranslate(
        nw4r::math::VEC2(0.0f, -sKeyTopAnmY.end / 3.0f));
    BG()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, 0.0f));
    PCKeyboard()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.end);
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.end);
    CandidateBox()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.end);
    ToolBar()->getDownArea()->SetAlpha(sKeyTopAnmA.end);
    ToolBar()->getUpArea()->SetAlpha(sKeyTopAnmA.end);
    BG()->getLayout()->GetRootPane()->SetAlpha(sKeyTopAnmA.end);
    PCKeyboard()->onActive();
    CellPhoneKeyboard()->onActive();
    if (ToolBar()->isQwerty()) {
        PCKeyboard()->onActive();
    } else {
        CellPhoneKeyboard()->onActive();
    }
}

void DisappearMemoState::calc() {
    if (30.0f <= mfCounter) {
        mpManager->changeState(Manager::STL_Transition);
    }
    InputForm()->calc();
    f32 y = util::hermiteInterporation(mfCounter, 0.0f, sKeyTopAnmY.end, 0.0f,
                                       30.0f, sKeyTopAnmY.start, 0.0f);
    PCKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    CandidateBox()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, y));
    ToolBar()->getDownArea()->SetTranslate(nw4r::math::VEC2(0.0f, y / 3.0f));
    ToolBar()->getUpArea()->SetTranslate(nw4r::math::VEC2(0.0f, -y / 3.0f));
    BG()->getLayout()->GetRootPane()->SetTranslate(
        nw4r::math::VEC2(0.0f, 0.0f));
    u8 alpha = util::hermiteInterporation(mfCounter, 0.0f, sKeyTopAnmA.end,
                                          0.0f, 30.0f, sKeyTopAnmA.start, 0.0f);
    PCKeyboard()->getLayout()->GetRootPane()->SetAlpha(alpha);
    CellPhoneKeyboard()->getLayout()->GetRootPane()->SetAlpha(alpha);
    CandidateBox()->getLayout()->GetRootPane()->SetAlpha(alpha);
    ToolBar()->getDownArea()->SetAlpha(alpha);
    ToolBar()->getUpArea()->SetAlpha(alpha);
    BG()->getLayout()->GetRootPane()->SetAlpha(alpha);
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        f32 inputFormY =
            util::hermiteInterporation(mfCounter, 0.0f, sInputFormAnmY.end,
                                       0.0f, 30.0f, sInputFormAnmY.start, 0.0f);
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, inputFormY));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(0xFF);
        break;
    default:
        InputForm()->getLayout()->GetRootPane()->SetTranslate(
            nw4r::math::VEC2(0.0f, y));
        InputForm()->getLayout()->GetRootPane()->SetAlpha(alpha);
        break;
    }
    mfCounter += 1.0f;
}

void DisappearMemoState::start() {
    mfCounter = 0.0f;
    Manager::ConfigType type = mpManager->getConfigType();
    switch (type) {
    case Manager::CT_Letter:
    case Manager::CT_PhotoLetter:
        InputForm()->setEditMode(InputForm::EM_Disappear);
        mpManager->setSaveData_();
        break;
    default:
        InputForm()->onClose();
        break;
    }
}

} // namespace memo
} // namespace extend
} // namespace textinput
