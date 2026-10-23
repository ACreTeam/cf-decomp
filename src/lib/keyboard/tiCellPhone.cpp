#include <lib/textinput/tiCellPhone.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiUtil.h>
#include <new>
#include <string.h>

namespace textinput {
namespace keyboard {
namespace cellphonetype {

struct PaneNameToControlKey {
    /* 0x00 */ char paneName[0x14];
    /* 0x14 */ int code;
}; // size 0x18

struct AnimationFile {
    /* 0x00 */ u32 id;
    /* 0x04 */ char fileName[0x40];
}; // size 0x44

struct PaneToAnimation {
    /* 0x00 */ char paneName[0x14];
    /* 0x14 */ const char *forceAddName;
}; // size 0x18

static const PaneNameToControlKey csPaneNameToControlKey[] = {
    {"B_CPkey_LF", VK_LineFeed},
    {"B_CPkey_DELETE", VK_BackSpace},
    {"B_othersBT_JP", VK_SignWindow},
    {"B_ChngTag_00", VK_Hiragana},
    {"B_ChngTag_01", VK_Katakana},
    {"B_ChngTag_02", VK_Alphabet},
    {"B_ChngTag_03", VK_Numeric},
    {"B_othersBT_EU", VK_SignWindow},
    {"B_CPkey_Prdc_JP", VK_PredictLanguage},
    {"B_prdcModeBT_EU", VK_PredictLanguage},
    {"B_smlCptChngeBT", VK_CapsChange},
    {"B_spaceBT_JP", VK_Space},
};

static const AnimationFile csAninationFileForNormalKey[] = {
    {0, "fs_VK_cellPhone_a_normal.brlan"},
    {1, "fs_VK_cellPhone_a_Focus-IN.brlan"},
    {2, "fs_VK_cellPhone_a_Focus-OUT.brlan"},
    {4, "fs_VK_cellPhone_a_Pushed.brlan"},
    {3, "fs_VK_cellPhone_a_Roll_over.brlan"},
};

static const AnimationFile csAninationFileForControlKey[] = {
    {0, "fs_VK_cellPhone_a_normal.brlan"},
    {1, "fs_VK_cellPhone_a_Focus-IN.brlan"},
    {2, "fs_VK_cellPhone_a_Focus-OUT.brlan"},
    {4, "fs_VK_cellPhone_a_Pushed.brlan"},
    {5, "fs_VK_cellPhone_a_toggle-ON.brlan"},
    {6, "fs_VK_cellPhone_a_toggle-OFF.brlan"},
};

static const LanguageDependency csLanguageDependencyData[LanguageEnd] = {
    {5,
     {&csKeySetHiragana, &csKeySetKatakana, &csKeySetABCJP, &csKeySetNumber,
      &csKeySetabcJP}},
    {5,
     {&csKeySetUSAbc, &csKeySetUSabc, &csKeySetUSABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetUKAbc, &csKeySetUKabc, &csKeySetUKABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetFRAbc, &csKeySetFRabc, &csKeySetFRABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetDEAbc, &csKeySetDEabc, &csKeySetDEABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetITAbc, &csKeySetITabc, &csKeySetITABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetSPAbc, &csKeySetSPabc, &csKeySetSPABC, &csKeySetNumber, NULL}},
    {5,
     {&csKeySetNLAbc, &csKeySetNLabc, &csKeySetNLABC, &csKeySetNumber, NULL}},
};

const wchar_t *csszPredictLanguage[] = {
    L"", L"", L"Eng", L"Fra", L"Esp", L"EN", L"DE", L"FR", L"ES", L"IT", L"NL",
};

const char *csPaneNameNormalKey[] = {
    "B_CPkey_00",      "B_CPkey_01",      "B_CPkey_02",    "B_CPkey_03",
    "B_CPkey_04",      "B_CPkey_05",      "B_CPkey_06",    "B_CPkey_07",
    "B_CPkey_08",      "B_CPkey_09",      "B_CPkey_10",    "B_CPkey_11",
    "B_CPkey_LF",      "B_CPkey_DELETE",  "B_othersBT_JP", "B_othersBT_EU",
    "B_spaceBT_JP",    "B_ChngTag_00",    "B_ChngTag_01",  "B_ChngTag_02",
    "B_ChngTag_03",    "B_SGNkey_close",  "B_SGNkey_prev", "B_SGNkey_next",
    "B_CPkey_Prdc_JP", "B_prdcModeBT_EU",
};

static const char *COMMON_KEY_ANIM = "W_CPkey_00";
static const char *COMMON_TAG_ANIM = "W_ChngTag_00";
static const char *COMMON_OTHERS_ANIM = "W_othersBT_EU";

static const PaneToAnimation csPaneNameNormalAnimationKey[] = {
    {"W_CPkey_00", NULL},
    {"W_CPkey_01", COMMON_KEY_ANIM},
    {"W_CPkey_02", COMMON_KEY_ANIM},
    {"W_CPkey_03", COMMON_KEY_ANIM},
    {"W_CPkey_04", COMMON_KEY_ANIM},
    {"W_CPkey_05", COMMON_KEY_ANIM},
    {"W_CPkey_06", COMMON_KEY_ANIM},
    {"W_CPkey_07", COMMON_KEY_ANIM},
    {"W_CPkey_08", COMMON_KEY_ANIM},
    {"W_CPkey_09", COMMON_KEY_ANIM},
    {"W_CPkey_10", COMMON_KEY_ANIM},
    {"W_CPkey_11", COMMON_KEY_ANIM},
    {"W_CPkey_LF", COMMON_KEY_ANIM},
    {"W_CPkey_DELETE", COMMON_KEY_ANIM},
    {"W_spaceBT_JP", COMMON_KEY_ANIM},
    {"W_othersBT_JP", COMMON_OTHERS_ANIM},
    {"W_othersBT_EU", NULL},
    {"W_prdcModeBT_EU", NULL},
    {"W_smlCptChngeBT", NULL},
};

static const PaneToAnimation csPaneNameToggleAnimationKey[] = {
    {"W_ChngTag_00", NULL},
    {"W_ChngTag_01", COMMON_TAG_ANIM},
    {"W_ChngTag_02", COMMON_TAG_ANIM},
    {"W_ChngTag_03", COMMON_TAG_ANIM},
};

static const PaneNameToCharCode *searchKey(const LanguageDependency *language,
                                           Base::InputMode mode,
                                           const char *paneName) {
    const PaneNameToCharCode *keys =
        language->pKeySets[mode]->pPaneNameToCharCode;
    for (u16 i = 0; i < KEY_COUNT; i++) {
        if (util::strcmp(keys[i].szPaneName, paneName)) {
            return &keys[i];
        }
    }
    return NULL;
}

static int getControlCode(const char *paneName) {
    for (u16 i = 0; i < 12; i++) {
        if (util::strcmp(csPaneNameToControlKey[i].paneName, paneName)) {
            return csPaneNameToControlKey[i].code;
        }
    }
    return VK_None;
}

static bool availableB(const char *paneName) {
    for (int i = 0; i < KEY_COUNT; i++) {
        if (util::strcmp(paneName, csPaneNameNormalKey[i])) {
            return true;
        }
    }
    return false;
}

struct KanaMode {
    /* 0x0 */ bool kana;
}; // size 0x1

void Base::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    mState.mIndex = 0;
    mState.meInputMode = IM_Hiragana;
    mState.mpKey = NULL;
    mState.mbLocked = false;
    mState.mbNumericWithDot = false;
    mState.mbNumeric = false;
    mState.mbUpperCase = true;
    mState.mbCapitalize = true;
    mState.mbPredictOn = false;
    mpLanguageData = &csLanguageDependencyData[getLanguage()];
    inputform::Base::KeyInput input = {0};
    sendCommand(3, &input);
}

void Base::init() {
    mState.mIndex = 0;
    mState.meInputMode = IM_Hiragana;
    mState.mpKey = NULL;
    mState.mbLocked = false;
    mState.mbNumericWithDot = false;
    mState.mbNumeric = false;
    mState.mbUpperCase = true;
    mState.mbCapitalize = true;
    mState.mbPredictOn = false;
    mpLanguageData = &csLanguageDependencyData[getLanguage()];
    inputform::Base::KeyInput input = {0};
    sendCommand(3, &input);
}

void Base::calc() {
}

void Base::onKey(u32 key, void *data) {
    OnKeyInfo *info = static_cast<OnKeyInfo *>(data);
    if (!mState.mbLocked) {
        const char *paneName = info->szPaneName;
        if (key == 4) {
            bool reverse = info->bBackward;
            int code = getControlCode(paneName);
            if (code != VK_None) {
                switch (code) {
                case VK_LineFeed:
                    sendCommand(7, NULL);
                    break;
                case VK_BackSpace:
                    sendCommand(1, NULL);
                    break;
                case VK_Hiragana:
                    changeInputMode(IM_Hiragana);
                    break;
                case VK_Katakana:
                    changeInputMode(IM_Katakana);
                    break;
                case VK_Alphabet:
                    changeInputMode(IM_Alphabet);
                    break;
                case VK_Numeric:
                    changeInputMode(IM_Numeric);
                    break;
                case VK_SignWindow:
                    goSignInputMode();
                    break;
                case VK_PredictLanguage:
                    changePredictLanguage();
                    break;
                case VK_CapsChange:
                    setAbcMode(!mState.mbUpperCase);
                    break;
                }
            } else if (!isZiActive() || mState.meInputMode == IM_Numeric ||
                       mState.meInputMode == IM_Numeric) {
                if (mState.mpKey == NULL) {
                    InputMode mode = (InputMode)mState.meInputMode;
                    if (getLanguage() == JP && mode == IM_Alphabet) {
                        if (!mState.mbUpperCase) {
                            mode = IM_AlphabetLower;
                        }
                    } else if (mpLanguageData->pKeySets[mode]->uType == 1 &&
                               !mState.mbUpperCase) {
                        mode = IM_Katakana;
                    }
                    mState.mpKey = searchKey(mpLanguageData, mode, paneName);
                    if (reverse) {
                        for (mState.mIndex = 15;
                             mState.mpKey->wc[mState.mIndex] == 0;
                             mState.mIndex--) {
                        }
                    } else {
                        mState.mIndex = 0;
                    }
                } else {
                    if (reverse) {
                        mState.mIndex--;
                    } else {
                        mState.mIndex++;
                    }
                    if (mState.mIndex == 16) {
                        mState.mIndex = 0;
                    } else if (mState.mIndex < 0) {
                        for (mState.mIndex = 15;
                             mState.mpKey->wc[mState.mIndex] == 0;
                             mState.mIndex--) {
                        }
                    }
                    if (mState.mpKey->wc[mState.mIndex] == 0) {
                        mState.mIndex = 0;
                    }
                }
                if (mState.mpKey != NULL) {
                    inputform::Base::KeyInput input = {0};
                    input.ch = mState.mpKey->wc[mState.mIndex];
                    input.holdingKey = (void *)mState.mpKey;
                    if (mState.meInputMode == IM_Numeric ||
                        mState.meInputMode == IM_Numeric) {
                        if (mState.mpKey->wc[1] != L'0' &&
                            mState.mpKey->wc[0] != L' ') {
                            input.direct = true;
                        }
                    }
                    if (mState.mpKey->wc[1] == 0) {
                        if (mState.mbNumericWithDot && input.ch == L'*') {
                            input.ch = L'.';
                        }
                        sendCommand(CommandReceiver::INPUT_COMMAND_KEY, &input);
                        mState.mIndex = 0;
                        mState.mpKey = NULL;
                    } else {
                        if (getLanguage() == JP && input.ch == L' ' &&
                            getInputMode() != IM_AlphabetLower &&
                            getInputMode() != IM_Alphabet) {
                            input.ch = 0x3000;
                        }
                        sendCommand(3, &input);
                    }
                }
            } else {
                for (u16 i = 0; i < KEY_COUNT; i++) {
                    if (util::strcmp(
                            csKeySetNumber.pPaneNameToCharCode[i].szPaneName,
                            paneName)) {
                        wchar_t ch =
                            csKeySetNumber.pPaneNameToCharCode[i].wc[0];
                        bool converting;
                        inputform::Base::KeyInput input = {0};
                        input.ch = ch;
                        input.letterMode = mState.meInputMode;
                        input.holdingKey = (void *)mState.mpKey;
                        sendCommand(0x20, &converting);
                        if (ch == L'0') {
                            if (converting) {
                                sendCommand(6, NULL);
                            }
                            inputform::Base::KeyInput space = {
                                L' ', 0, true, 0, true, false, NULL};
                            space.letterMode = mState.meInputMode;
                            space.holdingKey = (void *)mState.mpKey;
                            sendCommand(CommandReceiver::INPUT_COMMAND_KEY,
                                        &space);
                            mState.mIndex = 0;
                            mState.mpKey = NULL;
                        } else {
                            input.holdingKey = (void *)searchKey(
                                mpLanguageData, (InputMode)mState.meInputMode,
                                paneName);
                            if (mpLanguageData->pKeySets[mState.meInputMode]
                                        ->uType == 1 &&
                                !mState.mbCapitalize) {
                                input.letterMode = 1;
                                input.holdingKey = (void *)searchKey(
                                    mpLanguageData, IM_Katakana, paneName);
                            }
                            sendCommand(CommandReceiver::INPUT_COMMAND_KEY,
                                        &input);
                        }
                    }
                }
            }
        }
        if (key == 1 && mState.mpKey != NULL &&
            util::strcmp(paneName, mState.mpKey->szPaneName)) {
            wchar_t ch = mState.mpKey->wc[mState.mIndex];
            if (ch != 0x309B && ch != 0x309C) {
                if (getLanguage() == JP && ch == L' ' &&
                    getInputMode() != IM_AlphabetLower &&
                    getInputMode() != IM_Alphabet) {
                    ch = 0x3000;
                }
                inputform::Base::KeyInput input = {L'\0', 0,     false, 0,
                                                   true,  false, NULL};
                input.ch = ch;
                input.holdingKey = (void *)mState.mpKey;
                if (mState.meInputMode == IM_Numeric ||
                    mState.meInputMode == IM_Numeric) {
                    input.direct = true;
                }
                sendCommand(CommandReceiver::INPUT_COMMAND_KEY, &input);
            }
            mState.mIndex = 0;
            mState.mpKey = NULL;
        }
    }
}

void Base::goSignInputMode() {
}

void Base::changePredictLanguage() {
}

void Base::updateFromReceiver(u32 command, void *data) {
    if (getLanguage() != JP) {
        if ((int)command == 0x21) {
            setAbcMode(true);
        }
        if ((int)command == 5 &&
            !static_cast<inputform::Base::KeyInput *>(data)->_05) {
            setAbcMode(false);
            mState.mbCapitalize = false;
        }
    }
}

void Base::onActive() {
    u32 translateMode = 0;
    sendCommand(0x12, &translateMode);
    if (mState.meInputMode == IM_Hiragana) {
        KanaMode kanaMode = {true};
        sendCommand(0x13, &kanaMode);
    }
    if (mState.meInputMode == IM_Katakana) {
        KanaMode kanaMode = {false};
        sendCommand(0x13, &kanaMode);
    }
    if (mState.meInputMode == IM_Hiragana) {
        changeInputMode((InputMode)mState.meInputMode);
    }
    updateFixMode();
}

void Base::onClose() {
}

void Base::changeInputMode(InputMode mode) {
    mState.meInputMode = mode;
    if (getLanguage() != JP) {
        mState.mbUpperCase = true;
        mState.mbCapitalize = true;
    }
    bool numeric = false;
    if (mode == IM_Numeric) {
        numeric = true;
    }
    sendCommand(0x22, &numeric);
    sendCommand(6, NULL);
    if (mode == IM_Hiragana) {
        KanaMode kanaMode = {true};
        sendCommand(0x13, &kanaMode);
    }
    if (mode == IM_Katakana) {
        KanaMode kanaMode = {false};
        sendCommand(0x13, &kanaMode);
    }
    updateFixMode();
    sendCommand(0x29, NULL);
}

bool Base::isZiActive() {
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    int mode = predictMode.mode;
    if (predictMode.on) {
        return mode != 1;
    }
    return false;
}

bool Base::isAtokActive() {
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    int mode = predictMode.mode;
    if (predictMode.on && mode == 1) {
        return true;
    }
    return false;
}

void Base::setAbcMode(bool abc) {
    mState.mbUpperCase = abc;
    if (abc) {
        mState.mbCapitalize = true;
    }
}

void Base::doNumericMode(bool enable) {
    mState.mbNumeric = enable;
    if (enable) {
        input::HKBManager::sInstance.SetModifierState(0x100, 0x100);
        changeInputMode(IM_Numeric);
    }
}

void Base::updateFixMode() {
    if ((mpManager->getToolBar() != NULL &&
         mpManager->getToolBar()->isQwerty()) ||
        getLanguage() != JP) {
        return;
    }
    bool fix = false;
    if (mState.meInputMode == IM_AlphabetLower ||
        mState.meInputMode == IM_Alphabet) {
        fix = true;
    } else {
        fix = false;
    }
    sendCommand(0x14, &fix);
}

LayoutByNW4R::~LayoutByNW4R() {
    mpEventHandler->~EventHandler();
    MEMFreeToAllocator(mpAllocator, mpEventHandler);
    for (nw4rmanager::AnmPane *pane = static_cast<nw4rmanager::AnmPane *>(
             nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<nw4rmanager::AnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, NULL))) {
        nw4r::ut::List_Remove(&mAnmPanes, pane);
        pane->destroy(mpAllocator);
    }
}

inline EventHandler::~EventHandler() {
}

void LayoutByNW4R::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    Base::create(allocator);
    mpEventHandler =
        new (MEMAllocFromAllocator(allocator, sizeof(EventHandler)))
            EventHandler(this);
    nw4rmanager::Layout::createWithEventHandler(allocator, mpEventHandler);
    mpPaneManager->setAllComponentTriggerTarget(false);
    mpPaneManager->setAllBoundingBoxComponentTriggerTarget(true);
    for (u16 i = 0; i < 19; i++) {
        void *mem = MEMAllocFromAllocator(allocator, sizeof(CellPhoneAnmPane));
        CellPhoneAnmPane *anmPane = new (mem) CellPhoneAnmPane(
            getPane(csPaneNameNormalAnimationKey[i].paneName), NULL);
        nw4r::ut::List_Append(&mAnmPanes, anmPane);
        for (u16 j = 0; j < 5; j++) {
            void *res = mpMultiArcResourceAccessor->GetResource(
                0, csAninationFileForNormalKey[j].fileName, NULL);
            AnimTransformPane *anim = reinterpret_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(res,
                                                 mpMultiArcResourceAccessor));
            if (csPaneNameNormalAnimationKey[i].forceAddName != NULL) {
                anmPane->forceAddAnimation(
                    allocator, csAninationFileForNormalKey[j].id, anim,
                    csPaneNameNormalAnimationKey[i].forceAddName, false, true);
            } else {
                anmPane->addAnimation(allocator,
                                      csAninationFileForNormalKey[j].id, anim,
                                      false, true);
            }
        }
    }
    for (u16 i = 0; i < 4; i++) {
        void *mem =
            MEMAllocFromAllocator(allocator, sizeof(CellPhoneControlAnmPane));
        CellPhoneControlAnmPane *anmPane = new (mem) CellPhoneControlAnmPane(
            getPane(csPaneNameToggleAnimationKey[i].paneName), NULL);
        nw4r::ut::List_Append(&mAnmPanes, anmPane);
        for (u16 j = 0; j < 6; j++) {
            void *res = mpMultiArcResourceAccessor->GetResource(
                0, csAninationFileForControlKey[j].fileName, NULL);
            AnimTransformPane *anim = reinterpret_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(res,
                                                 mpMultiArcResourceAccessor));
            if (csPaneNameToggleAnimationKey[i].forceAddName != NULL) {
                anmPane->forceAddAnimation(
                    allocator, csAninationFileForControlKey[j].id, anim,
                    csPaneNameToggleAnimationKey[i].forceAddName, false, true);
            } else {
                anmPane->addAnimation(allocator,
                                      csAninationFileForControlKey[j].id, anim,
                                      false, true);
            }
        }
    }
    init();
    setLanguage(getLanguage());
}

void LayoutByNW4R::init() {
    Base::init();
    getAnmPane(IM_Hiragana)->changeAnimation(0);
    getAnmPane(IM_Katakana)->changeAnimation(0);
    getAnmPane(IM_Alphabet)->changeAnimation(0);
    getAnmPane(IM_Numeric)->changeAnimation(0);
    getAnmPane((InputMode)mState.meInputMode)->changeAnimation(5);
    setLineFeedButton(true);
    doNumericMode(false);
    setPredictLanguageButton(true);
    setSignWindowButton(true);
    setVisible("W_ChngTag_00", true);
    setVisible("W_ChngTag_01", true);
    setVisible("W_ChngTag_02", true);
    setVisible("W_ChngTag_03", true);
    for (int i = 0; i < 4; i++) {
        char textName[0x11];
        memset(textName, 0, sizeof(textName));
        strncpy(textName, csPaneNameToggleAnimationKey[i].paneName,
                strlen(csPaneNameToggleAnimationKey[i].paneName));
        textName[0] = 'T';
        static_cast<nw4r::lyt::TextBox *>(getPane(textName))
            ->SetString(mpLanguageData->pKeySets[i]->szKeySetName, 0);
    }
    if (getLanguage() == JP) {
        setVisible("W_prdcModeBT_EU", false);
        setVisible("W_othersBT_EU", false);
        setVisible("W_smlCptChngeBT", true);
        setVisible("W_othersBT_JP", true);
        setVisible("W_CPkey_Prdc_JP", true);
        setVisible("W_spaceBT_JP", true);
        setVisible("W_CPkey_09", true);
        setVisible("W_CPkey_11", true);
    } else {
        setVisible("W_prdcModeBT_EU", true);
        setVisible("W_othersBT_EU", true);
        setVisible("W_smlCptChngeBT", false);
        setVisible("W_othersBT_JP", false);
        setVisible("W_CPkey_Prdc_JP", false);
        setVisible("W_spaceBT_JP", false);
        setVisible("W_CPkey_09", false);
        setVisible("W_CPkey_11", false);
        setPredictLanguageButton(false);
    }
    changeInputMode((InputMode)mState.meInputMode);
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    updatePredictLanguage(&predictMode);
    initPaneLastDrawReceived();
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
    nw4r::lyt::Picture *space = nw4r::ut::DynamicCast<nw4r::lyt::Picture *>(
        mpLayout->GetRootPane()->FindPaneByName("P_spaceBT_JP", true));
    nw4r::lyt::Picture *henkan = nw4r::ut::DynamicCast<nw4r::lyt::Picture *>(
        mpLayout->GetRootPane()->FindPaneByName("P_HENKAN_JP", true));
    space->SetVisible(true);
    henkan->SetVisible(false);
}

void LayoutByNW4R::draw() {
    nw4rmanager::Layout::draw();
}

void LayoutByNW4R::calc() {
    nw4rmanager::Layout::calc();
    nw4r::lyt::Picture *space = nw4r::ut::DynamicCast<nw4r::lyt::Picture *>(
        mpLayout->GetRootPane()->FindPaneByName("P_spaceBT_JP", true));
    nw4r::lyt::Picture *henkan = nw4r::ut::DynamicCast<nw4r::lyt::Picture *>(
        mpLayout->GetRootPane()->FindPaneByName("P_HENKAN_JP", true));
    if (mpManager->getInputForm()->canConvert()) {
        space->SetVisible(false);
        henkan->SetVisible(true);
    } else {
        space->SetVisible(true);
        henkan->SetVisible(false);
    }
}

void LayoutByNW4R::update() {
}

void LayoutByNW4R::onKey(u32 key, void *data) {
    Base::onKey(key, data);
    if (!mState.mbLocked) {
        const char *paneName = static_cast<OnKeyInfo *>(data)->szPaneName;
        if (key == 4) {
            int code = getControlCode(paneName);
            if (code != VK_None) {
                switch (code) {
                case VK_Alphabet:
                case VK_Numeric:
                case VK_Hiragana:
                case VK_Katakana:
                    mpEventObserver->onSE(sound::SE_KETAI_MODE_SWITCHING);
                    break;
                case VK_CapsChange:
                    if (!mState.mbUpperCase) {
                        mpEventObserver->onSE(sound::SE_CHAR_LOWER);
                    } else {
                        mpEventObserver->onSE(sound::SE_CHAR_LARGER);
                    }
                    break;
                case VK_Space:
                    if (mpManager->getInputForm()->canConvert()) {
                        sendCommand(0x28, NULL);
                    } else {
                        inputform::Base::CharInput input = {L' ', 0, false,
                                                            true};
                        sendCommand(0, &input);
                    }
                    break;
                }
            }
        }
    }
}

void LayoutByNW4R::onActive() {
    Base::onActive();
    nw4rmanager::Layout::init();
}

void LayoutByNW4R::onClose() {
    initPaneLastDrawReceived();
    changeAnimationAllToNormal();
}

void LayoutByNW4R::updateFromReceiver(u32 command, void *data) {
    Base::updateFromReceiver(command, data);
    switch (command) {
    case 0x1D:
        updatePredictLanguage(
            static_cast<CommandReceiver::ChangePredictMode *>(data));
        break;
    }
}

void LayoutByNW4R::changeInputMode(InputMode mode) {
    Base::changeInputMode(mode);
    if (getLanguage() == JP) {
        if (!mState.mbUpperCase && mode == IM_Alphabet) {
            mode = IM_AlphabetLower;
        }
        switch (mode) {
        case IM_Hiragana:
        case IM_Katakana:
            setVisible("W_smlCptChngeBT", false);
            setVisible("W_CPkey_09", true);
            setVisible("P_CPkey_dakuten", true);
            setVisible("W_smlCptChngeBT", false);
            break;
        case IM_Alphabet:
        case IM_AlphabetLower:
            setVisible("W_smlCptChngeBT", true);
            setVisible("W_CPkey_09", false);
            setVisible("P_CPkey_dakuten", false);
            setVisible("W_smlCptChngeBT", true);
            break;
        case IM_Numeric:
            setVisible("W_smlCptChngeBT", false);
            setVisible("W_CPkey_09", true);
            setVisible("P_CPkey_dakuten", false);
            setVisible("W_smlCptChngeBT", false);
            break;
        }
    }
    changeKeyTop(mpLanguageData->pKeySets[mode]->pPaneNameToCharCode);
    initPaneLastDrawReceived();
}

void LayoutByNW4R::changeKeyTop(const PaneNameToCharCode *keys) {
    for (u16 i = 0; i < KEY_COUNT; i++) {
        char textName[0x11];
        util::replaceChar(textName, sizeof(textName), keys[i].szPaneName, 0,
                          'T');
        nw4r::lyt::TextBox *textBox =
            nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(
                mpLayout->GetRootPane()->FindPaneByName(textName, true));
        if (textBox != NULL) {
            textBox->SetString(keys[i].szPaneDisp, 0);
        }
    }
    changeSpaceKeyTop(keys);
}

void LayoutByNW4R::changeSpaceKeyTop(const PaneNameToCharCode *keys) {
    if (getLanguage() != JP) {
        char textName[0x11];
        util::replaceChar(textName, sizeof(textName), keys[10].szPaneName, 0,
                          'T');
        nw4r::lyt::TextBox *textBox = static_cast<nw4r::lyt::TextBox *>(
            mpLayout->GetRootPane()->FindPaneByName(textName, true));
        if (keys[10].wc[0] != L'0') {
            if (!mState.mbPredictOn) {
                textBox->SetString(csSpaceZero, 0);
            } else {
                textBox->SetString(csSpace[0], 0);
            }
        }
    }
}

nw4rmanager::AnmPane *LayoutByNW4R::getAnmPane(InputMode mode) {
    switch (mode) {
    case IM_Alphabet:
        return searchAnmPane(csPaneNameToggleAnimationKey[2].paneName);
    case IM_Numeric:
        return searchAnmPane(csPaneNameToggleAnimationKey[3].paneName);
    case IM_Hiragana:
        return searchAnmPane(csPaneNameToggleAnimationKey[0].paneName);
    case IM_Katakana:
        return searchAnmPane(csPaneNameToggleAnimationKey[1].paneName);
    }
    return NULL;
}

void LayoutByNW4R::throwReleaseForAll() {
    for (CellPhoneAnmPane *pane = static_cast<CellPhoneAnmPane *>(
             nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<CellPhoneAnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, pane))) {
        if (pane->getKeyType() == 0) {
            pane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
        }
    }
}

void LayoutByNW4R::changeAnimationAllToNormal() {
    for (CellPhoneAnmPane *pane = static_cast<CellPhoneAnmPane *>(
             nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<CellPhoneAnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, pane))) {
        if (pane->getKeyType() == 0) {
            pane->changeAnimation(0);
        }
    }
}

void LayoutByNW4R::setLanguage(Language language) {
    KeyboardBase::setLanguage(language);
    if (mpLayout != NULL) {
        init();
        initPaneLastDrawReceived();
    }
}

void LayoutByNW4R::changePredictLanguage() {
    throwReleaseForAll();
    CommandReceiver::ChangePredictMode predictMode = {0, 0};
    sendCommand(0x1F, &predictMode);
    mpPredictLanguageDialog->open(
        static_cast<inputform::Base::PredictMode>(predictMode.mode), this);
}

void LayoutByNW4R::updatePredictLanguage(
    CommandReceiver::ChangePredictMode *mode) {
    if (getLanguage() != JP) {
        if (!mode->on) {
            setVisible("N_prdc_EU_ON", false);
            setVisible("N_prdc_EU_OFF", true);
            mState.mbPredictOn = false;
        } else {
            setVisible("N_prdc_EU_ON", true);
            setVisible("N_prdc_EU_OFF", false);
            mState.mbPredictOn = true;
        }
        static_cast<nw4r::lyt::TextBox *>(getPane("N_prdc_EU_lang"))
            ->SetString(csszPredictLanguage[mode->mode], 0);
        changeSpaceKeyTop(
            mpLanguageData->pKeySets[mState.meInputMode]->pPaneNameToCharCode);
    }
}

void LayoutByNW4R::setAbcMode(bool abc) {
    Base::setAbcMode(abc);
    if (getLanguage() == JP) {
        InputMode mode = IM_Alphabet;
        if (!mState.mbUpperCase) {
            mode = IM_AlphabetLower;
        }
        changeKeyTop(mpLanguageData->pKeySets[mode]->pPaneNameToCharCode);
        mpEventObserver->onSE(sound::SE_18);
    } else {
        if (mpLanguageData->pKeySets[mState.meInputMode]->uType == 1) {
            int index = IM_Hiragana;
            if (!mState.mbUpperCase) {
                index = IM_Katakana;
            }
            changeKeyTop(mpLanguageData->pKeySets[index]->pPaneNameToCharCode);
        }
    }
}

void LayoutByNW4R::goSignInputMode() {
    bool direct = false;
    if (mState.meInputMode == IM_Numeric) {
        direct = true;
    }
    mpSignWindow->open(this, direct);
    throwReleaseForAll();
}

void LayoutByNW4R::doNumericMode(bool enable) {
    Base::doNumericMode(enable);
    if (enable) {
        setVisible("W_smlCptChngeBT", !enable);
        setVisible("W_CPkey_09", !enable);
        setVisible("P_CPkey_dakuten", !enable);
        setVisible("W_smlCptChngeBT", !enable);
        setVisible("W_prdcModeBT_EU", !enable);
        setVisible("W_othersBT_EU", !enable);
        setVisible("W_spaceBT_JP", !enable);
        setVisible("W_smlCptChngeBT", !enable);
        setVisible("W_othersBT_JP", !enable);
        setVisible("W_CPkey_Prdc_JP", !enable);
        setVisible("W_CPkey_09", !enable);
        setVisible("W_CPkey_11", !enable);
        setVisible("W_ChngTag_00", !enable);
        setVisible("W_ChngTag_01", !enable);
        setVisible("W_ChngTag_02", !enable);
        setVisible("W_ChngTag_03", !enable);
    }
}

void LayoutByNW4R::doNumericWithDotMode(bool enable) {
    doNumericMode(enable);
    setString("T_CPkey_11", L".");
    setVisible("W_CPkey_11", enable);
    mState.mbNumericWithDot = true;
}

void LayoutByNW4R::setLineFeedButton(bool enable) {
    mbEnableLineFeed = enable;
    setVisible("W_CPkey_LF", mbEnableLineFeed);
}

void LayoutByNW4R::setPredictLanguageButton(bool enable) {
    setVisible("W_prdcModeBT_EU", enable);
}

void LayoutByNW4R::setSignWindowButton(bool enable) {
    if (!enable) {
        setVisible("W_othersBT_EU", enable);
        setVisible("W_othersBT_JP", enable);
    } else {
        setVisible("W_othersBT_EU", false);
        setVisible("W_othersBT_JP", false);
        if (getLanguage() == JP) {
            setVisible("W_othersBT_JP", true);
        } else {
            setVisible("W_othersBT_EU", true);
        }
    }
}

void LayoutByNW4R::setInputMode(InputMode mode) {
    changeInputMode(mode);
    getAnmPane(IM_Hiragana)->changeAnimation(0);
    getAnmPane(IM_Katakana)->changeAnimation(0);
    getAnmPane(IM_Alphabet)->changeAnimation(0);
    getAnmPane(IM_Numeric)->changeAnimation(0);
    getAnmPane((InputMode)mState.meInputMode)->changeAnimation(5);
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
}

void LayoutByNW4R::setUpperCaseJP(bool upperCase) {
    if (getLanguage() == JP) {
        mState.mbUpperCase = upperCase;
    }
}

bool LayoutByNW4R::updateInput(input::HKBManager &hkbManager) {
    if (mpManager->getToolBar()->isHWKBMode()) {
        input::HKBManager::KeySet triggered = hkbManager.GetTriggeredKeySet();
        if (triggered.IsValid()) {
            if (mState.mpKey != NULL) {
                OnKeyInfo info = {NULL, false, false};
                info.szPaneName = mState.mpKey->szPaneName;
                onKey(1, &info);
            }
            changeAnimationAllToNormal();
            mpManager->getToolBar()->setQwertyWithSE(true);
            if (!mpManager->getPCKeyboard()->isABC()) {
                mpManager->getPCKeyboard()->setABC(true);
            }
        }
    } else {
        input::HKBManager::KeySet keySet = hkbManager.GetTriggeredKeySet();
        for (; keySet.IsValid(); keySet = keySet.GetNext()) {
            nw4rmanager::AnmPane *anmPane = NULL;
            u8 key = keySet.GetKey();
            wchar_t wc = keySet.GetWChar();
            switch (key) {
            case 0x28:
            case 0x58:
                if (!(hkbManager.GetModifierState() & 4)) {
                    anmPane = searchAnmPane("W_CPkey_LF");
                }
                break;
            default:
                anmPane = searchAnmPane(wc);
                break;
            }
            if (anmPane != NULL) {
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            }
        }
        for (keySet = hkbManager.GetRepeatedKeySet(); keySet.IsValid();
             keySet = keySet.GetNext()) {
            nw4rmanager::AnmPane *anmPane = NULL;
            u8 key = keySet.GetKey();
            keySet.GetWChar();
            switch (key) {
            case 0x2A:
                if (hkbManager.GetModifierState() & 4) {
                    break;
                }
            case 0x4C:
                anmPane = searchAnmPane("W_CPkey_DELETE");
                break;
            }
            if (anmPane != NULL) {
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            }
        }
    }
    return false;
}

void CellPhoneAnmPane::changeAnimation(u32 id) {
    meState = id;
    nw4rmanager::AnmPane::changeAnimation(id == 7 ? 2 : id);
}

void CellPhoneAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    switch (meState) {
    case 0:
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(7);
        }
        break;
    case 1:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(7);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 7:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        if (paneEvent == PE_0) {
            changeAnimation(7);
        }
        break;
    }
}

void CellPhoneControlAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_5 && meState != 6) {
        if (meState == 5) {
            changeAnimation(6);
            return;
        }
        if (meState != 0 && meState != 2) {
            changeAnimation(2);
            return;
        }
    }
    if (paneEvent == PE_0) {
        changeAnimation(4);
        return;
    }
    switch (meState) {
    case 0:
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
    case 1:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        if (paneEvent == PE_0) {
            changeAnimation(4);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        break;
    case 6:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(5);
        }
        break;
    }
}

void EventHandler::onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                             Input *input) {
    const char *paneName = paneComponent->getPane()->GetName();
    if (paneName[0] == 'B') {
        char anmName[0x11];
        util::replaceChar(anmName, sizeof(anmName), paneName, 0, 'W');
        CellPhoneAnmPane *anmPane =
            static_cast<CellPhoneAnmPane *>(mpLayout->searchAnmPane(anmName));
        if (anmPane != NULL) {
            switch (event) {
            case gui::EventHandler::ON_TRIG:
                if ((input->trig & 0x800) ||
                    ((input->trig & 0x400) && availableB(paneName))) {
                    if (anmPane->getKeyType() == 1) {
                        nw4r::ut::List &list = mpLayout->getAnmPaneList();
                        for (CellPhoneAnmPane *pane =
                                 static_cast<CellPhoneAnmPane *>(
                                     nw4r::ut::List_GetNext(&list, NULL));
                             pane != NULL;
                             pane = static_cast<CellPhoneAnmPane *>(
                                 nw4r::ut::List_GetNext(&list, pane))) {
                            if (pane != anmPane && pane->getKeyType() == 1) {
                                pane->onAnmEvent(nw4rmanager::AnmPane::PE_5);
                            }
                        }
                    }
                    anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_0);
                }
                break;
            case gui::EventHandler::ON_LEFT:
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
                break;
            case gui::EventHandler::ON_POINT:
                mpEventObserver->onSE(sound::SE_SELECT);
                mpLayout->setPaneLastDrawReceived(anmPane->getPane());
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_1);
                break;
            }
        }
    }
    OnKeyInfo info = {paneName, false, false};
    if (event == gui::EventHandler::ON_TRIG) {
        if (input->trig & 0x800) {
            info.bBackward = false;
            mpLayout->onKey(4, &info);
        } else if ((input->trig & 0x400) && availableB(paneName)) {
            info.bBackward = true;
            mpLayout->onKey(4, &info);
            paneComponent->setFlightDuration(input->ctrlNo, 0);
        }
    }
    if (event == gui::EventHandler::ON_LEFT) {
        mpLayout->onKey(1, &info);
    }
    if (event == gui::EventHandler::ON_MOVE && (input->hold & 0x800) &&
        !(input->trig & 0x800)) {
        if (util::strcmp(csPaneNameNormalKey[13], paneName) ||
            util::strcmp(csPaneNameNormalKey[16], paneName)) {
            if (paneComponent->isDragging(input->ctrlNo)) {
                u32 duration =
                    mpLayout->getFlightDuration(input->ctrlNo, paneName);
                if (duration >= 30 && duration % 9 == 0) {
                    char anmName[0x11];
                    util::replaceChar(anmName, sizeof(anmName), paneName, 0,
                                      'W');
                    mpLayout->searchAnmPane(anmName)->onAnmEvent(
                        nw4rmanager::AnmPane::PE_0);
                    mpLayout->onKey(4, &info);
                }
            }
        }
    }
    if (event == gui::EventHandler::ON_MOVE &&
        mpLayout->getFlightDuration(input->ctrlNo, paneName) == 90) {
        info.bHold = true;
        mpLayout->onKey(1, &info);
    }
}

} // namespace cellphonetype
} // namespace keyboard
} // namespace textinput
