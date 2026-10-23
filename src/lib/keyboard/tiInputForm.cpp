#include <lib/textinput/tiInputForm.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiDebug.h>
#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiLayoutGather.h>
#include <lib/textinput/tiPcKeyboard.h>
#include <lib/textinput/tiCellPhone.h>
#include <lib/textinput/tiToolBar.h>
#include <extras.h>
#include <new>
#include <string.h>

namespace textinput {
namespace inputform {

static GXColor sKanaBoxColor = {0x80, 0xFF, 0x80, 0xFF};
static GXColor sCandidateBoxColor = {0xFF, 0xD2, 0x0C, 0xFF};
static const char *COMMON_SCROLL_ANIM = "P_txtScrll_UP";
static bool sbFilterInvalidChars = true;
static bool sbWordHead = true;

static nw4r::ut::Color sCandidateColor(200, 50, 50, 255);
static nw4r::ut::Color sTextBoxColor(20, 20, 20, 255);
static nw4r::ut::Color sZiInputColor(255, 50, 50, 255);
static nw4r::ut::Color sZiPredictColor(192, 192, 192, 255);
static nw4r::ut::Color sZiSelectedColor(50, 100, 50, 255);
static nw4r::ut::Color sCyanColor(100, 200, 200, 255);
static nw4r::ut::Color sSpaceCandidateColor(255, 20, 20, 255);

struct AnimationFile {
    /* 0x00 */ u32 id;
    /* 0x04 */ char fileName[0x40];
}; // size 0x44

struct PaneToAnimation {
    /* 0x00 */ u32 type;
    /* 0x04 */ const char *paneName;
    /* 0x08 */ u32 count;
    /* 0x0C */ const char *forceAddName;
    /* 0x10 */ const AnimationFile *pAnims[12];
}; // size 0x40

static const AnimationFile csAnimationFile[] = {
    {0, "fs_VK_textBox_a_normal.brlan"},
    {1, "fs_VK_textBox_a_Foucus_IN.brlan"},
    {2, "fs_VK_textBox_a_Focus-OUT.brlan"},
    {3, "fs_VK_textBox_a_Roll_over.brlan"},
    {4, "fs_VK_textBox_a_Pushed.brlan"},
    {5, "fs_VK_textBox_a_Fade_IN.brlan"},
    {6, "fs_VK_textBox_a_Fade_OUT.brlan"},
    {7, "fs_VK_textBox_a_Off.brlan"},
};

static const wchar_t scURLHeads[2][10] = {L"http://", L"https://"};

static const PaneToAnimation csPaneToAnimation[] = {
    {0,
     "P_txtScrll_UP",
     8,
     NULL,
     {&csAnimationFile[0], &csAnimationFile[1], &csAnimationFile[2],
      &csAnimationFile[3], &csAnimationFile[4], &csAnimationFile[5],
      &csAnimationFile[6], &csAnimationFile[7]}},
    {0,
     "P_txtScrll_DOWN",
     8,
     COMMON_SCROLL_ANIM,
     {&csAnimationFile[0], &csAnimationFile[1], &csAnimationFile[2],
      &csAnimationFile[3], &csAnimationFile[4], &csAnimationFile[5],
      &csAnimationFile[6], &csAnimationFile[7]}},
};

struct CharTable {
    /* 0x00 */ wchar_t chars[16];
}; // size 0x20

static CharTable sInvalidChars = {
    {0xA4, 0xAC, 0xAF, 0xB2, 0xB3, 0xB6, 0xB8, 0xB9, 0xBC, 0xBD, 0xBE, 0xD0,
     0xDE, 0xF0, 0xFE, 0x00},
};

inline bool isValidInputChar(wchar_t ch, CharTable table) {
    if (ch < table.chars[0] || ch > table.chars[14]) {
        return true;
    }
    for (int i = 0; i < 15; i++) {
        if (ch == table.chars[i]) {
            return false;
        }
    }
    return true;
}

wchar_t DeadKeyStream::ToCombineClass(Language language, wchar_t ch) {
    switch (language) {
    case DE:
        if (ch == 0xB4) {
            return 0x301;
        }
        if (ch == 0x60) {
            return 0x300;
        }
        break;
    case FR:
        if (ch == 0x5E) {
            return 0x302;
        }
        if (ch == 0xA8) {
            return 0x308;
        }
        break;
    case SP:
        if (ch == 0x60) {
            return 0x300;
        }
        if (ch == 0xB4) {
            return 0x301;
        }
        if (ch == 0x5E) {
            return 0x302;
        }
        if (ch == 0x7E) {
            return 0x303;
        }
        if (ch == 0xA8) {
            return 0x308;
        }
        break;
    case NL:
        if (ch == 0x60) {
            return 0x300;
        }
        if (ch == 0x5E) {
            return 0x302;
        }
        if (ch == 0x7E) {
            return 0x303;
        }
        if (ch == 0x27) {
            return 0x30D;
        }
        if (ch == 0x22) {
            return 0x30E;
        }
        break;
    }
    return ch;
}

wchar_t DeadKeyStream::ToIndependentClass(wchar_t ch) {
    if (ch < 0x300 || ch > 0x330) {
        return ch;
    }
    switch (ch) {
    case 0x300:
        return 0x60;
    case 0x301:
        return 0xB4;
    case 0x302:
        return 0x5E;
    case 0x303:
        return 0x7E;
    case 0x308:
        return 0xA8;
    case 0x30D:
        return 0x27;
    case 0x30E:
        return 0x22;
    case 0x327:
        return 0xB8;
    }
    return ch;
}

void Base::RowInfoManager::init() {
    for (u16 i = 0; i < mMaxLength; i++) {
        Info_ *info = getInfo(i);
        info->Back = i - 1;
        info->Next = i + 1;
        info->StrCount = 0;
        info->DispRowCount = 0;
    }
    Info_ *head = getHead();
    u16 last = mMaxLength - 1;
    head->Back = last;
    head->Next = 0;
    Info_ *infos = mpInfo;
    infos[last].Next = mMaxLength;
    infos[0].Back = mMaxLength;
    Info_ *empty = getInfo(mMaxLength + 1);
    empty->Back = mMaxLength + 1;
    empty->Next = mMaxLength + 1;
}

void Base::setCursorPos(tistring::Decolated *string, u32 pos) {
    string->setCursorPos(pos);
}

Base::Base(Manager *manager)
    : mpString(NULL), mpUnfixString(NULL), mpZiString(NULL),
      meInputMode(IM_Direct), mePredictMode(PM_USEn), mbPredictOn(false),
      mbDoWordWrap(true), mbRightWithSpace(true), mfCursorX(0.0f),
      mfCursorY(0.0f), mfScrollX(0.0f), mfScrollY(0.0f), mfSustainTimer(0.0f),
      meScrollFlag(SF_NoScroll), muWordWrapCounter(0),
      muLimitStringLength(0x400), muLimitRowNum(9999), mfCursorTimer(0.0f),
      muGlobalAlpha(0xFF), mCharColor(0xFFFFFFFF), mpAllocator(NULL),
      mpManager(manager), meLanguage(USA), mbZuSelected(false),
      mbCursorSelected(false), mriManager(muLimitStringLength),
      mpCursorLine(NULL), mCursorLinePos(0), mbLineDraw(true),
      muSpecifyLineDrawCount(0), muCursorTimer(0) {
}

Base::~Base() {
    if (mpString != NULL) {
        mpString->~Decolated();
        MEMFreeToAllocator(mpAllocator, mpString);
    }
    if (mpUnfixString != NULL) {
        mpUnfixString->~WithAtok();
        MEMFreeToAllocator(mpAllocator, mpUnfixString);
    }
    if (mpZiString != NULL) {
        mpZiString->~WithZi();
        MEMFreeToAllocator(mpAllocator, mpZiString);
    }
}

void Base::init() {
    CommandReceiver::init();
    meInputMode = IM_Direct;
    mfScrollY = mfScrollX = mfCursorY = mfCursorX = 0.0f;
    mfSustainTimer = 0.0f;
    mfCursorTimer = 0.0f;
    muGlobalAlpha = 0xFF;
    mDKStream.Init();
    SetFixedWidth(GetFont()->GetWidth());
    EnableFixedWidth(false);
    enableSpaceByRight(true);
    mpUnfixString->resetRelation();
    update();
    dirtyDrawCache();
    dirtyCursorCache();
    mfCursorCacheBaseY = mfScrollY;
}

bool Base::isEditMode() {
    switch (mePredictMode) {
    case PM_Off:
    case PM_Atok:
        break;
    default:
        if (!mpZiString->isFix()) {
            return false;
        }
        break;
    }
    return true;
}

bool Base::checkHeadOfSentence(bool skipSpace) {
    bool converting;
    onCommand((INPUT_COMMAND)0x20, &converting);
    if (converting) {
        return false;
    }

    bool moved = false;
    if (mpString->getCursorPos() < 1) {
        return true;
    }
    if (skipSpace) {
        switch (mpString->getWCharAtCursor()) {
        case L' ':
        case L'\n':
            break;
        default:
            return false;
        }
        moved = mpString->moveCursorLeft();
    }
    bool result = mpString->atTheBeginningOfASentence();
    if (moved) {
        mpString->moveCursorRight();
    }
    return result;
}

void Base::setDestination(Destination destination) {
    meDestination = destination;
}

void Base::setLanguage(Language language) {
    meLanguage = language;
    switch (language) {
    case JP:
        setPredictMode(PM_Atok);
        break;
    case USA:
        setPredictMode(PM_USEn);
        break;
    case UK:
        setPredictMode(PM_En);
        break;
    case DE:
        setPredictMode(PM_De);
        break;
    case IT:
        setPredictMode(PM_It);
        break;
    case NL:
        setPredictMode(PM_Nl);
        break;
    case SP:
        if (meDestination == DST_EU) {
            setPredictMode(PM_Sp);
        } else {
            setPredictMode(PM_USSp);
        }
        break;
    case FR:
        if (meDestination == DST_EU) {
            setPredictMode(PM_Fr);
        } else {
            setPredictMode(PM_USFr);
        }
        break;
    }
}

void Base::setPredictMode(PredictMode predictMode) {
    switch (meDestination) {
    case DST_EU:
        if (predictMode != PM_En && predictMode != PM_De &&
            predictMode != PM_Fr && predictMode != PM_Sp &&
            predictMode != PM_It && predictMode != PM_Nl) {
            return;
        }
        break;
    case DST_JP:
        if (predictMode != PM_Atok) {
            return;
        }
        break;
    case DST_US:
        if (predictMode != PM_USEn && predictMode != PM_USFr &&
            predictMode != PM_USSp) {
            return;
        }
        break;
    }
    PredictMode mode;
    onCommand((INPUT_COMMAND)0x1F, &mode);
    mode = predictMode;
    onCommand((INPUT_COMMAND)0x1D, &mode);
}

Base::PredictMode Base::getPredictMode() {
    PredictMode mode;
    onCommand((INPUT_COMMAND)0x1F, &mode);
    return mode;
}

void Base::onCommand(INPUT_COMMAND command, void *data) {
    CommandReceiver::onCommand(command, data);
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    bool isMainString = string == mpString;
    switch (command) {
    case 40:
        if (string == mpUnfixString) {
            if (LayoutGather::Singleton::getInstance().isHoldingShift()) {
                onPressUp();
            } else {
                onPressDown();
            }
            return;
        }
        if (KPRLookAhead(&mDKStream.mKPRQueue, NULL, 0)) {
            KPRPutChar(&mDKStream.mKPRQueue, 0xFFFF);
        } else {
            KPRPutChar(&mDKStream.mKPRQueue, L' ');
        }
        if (mDKStream.mKPRQueue.oCount == 0) {
            onSE((sound::SE)10);
        }
        if (getCurrentString(false) == mpUnfixString) {
            mpUnfixString->isConverting();
        }
        for (;;) {
            wchar_t ch = KPRGetChar(&mDKStream.mKPRQueue);
            if (sbFilterInvalidChars) {
                while (!isValidInputChar(ch, sInvalidChars)) {
                    ch = KPRGetChar(&mDKStream.mKPRQueue);
                }
            }
            if (ch == L'\0') {
                break;
            }
            if (isZiActive()) {
                inputCharZi_(ch, 0);
            } else {
                inputCharDefault_(ch);
            }
        }
        break;
    case 0: {
        CharInput *input = static_cast<CharInput *>(data);
        wchar_t ch = input->ch;
        u32 flags = input->flags;
        InputState prevState;
        if (input->usePCKeyboardMode) {
            InputState state;
            state.mode =
                (tistring::Decolated::TranslateMode)mpManager->getPCKeyboard()->getTranslateMode();
            state.fixMode = mpManager->getPCKeyboard()->getTranslateMode() == 0;
            state.kanaMode =
                mpManager->getPCKeyboard()->getTranslateMode() == 1;
            changeInputState(state, prevState);
        }
        if (input->deadKey) {
            ch = DeadKeyStream::ToCombineClass(meLanguage, ch);
        }
        if (ch == L' ' && KPRLookAhead(&mDKStream.mKPRQueue, NULL, 0)) {
            KPRPutChar(&mDKStream.mKPRQueue, 0xFFFF);
        } else {
            KPRPutChar(&mDKStream.mKPRQueue, ch);
        }
        if (mDKStream.mKPRQueue.oCount == 0) {
            onSE((sound::SE)10);
        }
        if (getCurrentString(false) == mpUnfixString) {
            mpUnfixString->isConverting();
        }
        for (;;) {
            wchar_t c = KPRGetChar(&mDKStream.mKPRQueue);
            if (sbFilterInvalidChars) {
                while (!isValidInputChar(c, sInvalidChars)) {
                    c = KPRGetChar(&mDKStream.mKPRQueue);
                }
            }
            if (c == L'\0') {
                break;
            }
            if (isZiActive()) {
                inputCharZi_(c, flags);
            } else {
                inputCharDefault_(c);
            }
        }
        if (string == mpZiString) {
            mbZuSelected = false;
        }
        if (input->usePCKeyboardMode) {
            InputState unused;
            changeInputState(prevState, unused);
        }
        if (checkHeadOfSentence(true)) {
            onCommand((INPUT_COMMAND)33, NULL);
        }
        break;
    }
    case 1:
        if (string->canBackSpace()) {
            onSE((sound::SE)7);
        } else {
            onSE((sound::SE)8);
        }
        if (string == mpString) {
            mpUnfixString->resetRelation();
            update();
            mpString->getCursorPos();
            mpString->backSpace();
            mpString->getCursorPos();
        } else if (string == mpZiString) {
            mbZuSelected = false;
            string->backSpace();
        } else {
            if (!mpUnfixString->isConverting()) {
                mpUnfixString->backSpace();
            }
            update();
        }
        meScrollFlag = SF_ScrollByBS;
        if (checkHeadOfSentence(true)) {
            onCommand((INPUT_COMMAND)33, NULL);
        }
        break;
    case 2:
        if (string != mpString) {
            onSE((sound::SE)8);
            return;
        }
        if (!mpString->deleteForward()) {
            onSE((sound::SE)8);
        } else {
            onSE((sound::SE)7);
        }
        break;
    case 7:
        if (string == mpString) {
            mpUnfixString->resetRelation();
            update();
            bool playSE;
            bool lineFeed;
            bool enableLineFeed;
            if (mpManager->getToolBar()->isQwerty()) {
                enableLineFeed =
                    mpManager->getPCKeyboard()->isLineFeedEnabled();
            } else {
                enableLineFeed =
                    mpManager->getCellPhoneKeyboard()->isLineFeedEnabled();
            }
            lineFeed = true;
            if (mpString->isKanaFix()) {
                bool canFeed = false;
                if (enableLineFeed && muLimitRowNum >= getLine() + 1) {
                    canFeed = true;
                }
                if (!canFeed) {
                    lineFeed = false;
                }
            }
            playSE = true;
            if (muLimitRowNum >= getLine() + 1 &&
                !mpManager->getCandidateBox()->isActive()) {
                playSE = false;
            }
            if (lineFeed) {
                onSE((sound::SE)9);
                mpString->getCursorPos();
                mpString->inputChar(L'\n');
                mpString->getCursorPos();
            } else if (playSE) {
                onSE((sound::SE)8);
                return;
            } else {
                return;
            }
        } else if (string == mpZiString) {
            onSE((sound::SE)9);
            if (!(mbCursorSelected | mbZuSelected) &&
                mpManager->getToolBar()->isQwerty()) {
                if (mbPredictOn && mePredictMode != PM_Atok) {
                    mpZiString->getCurrentInput(ziInput, 64);
                    mpString->getCursorPos();
                    mpString->inputString(ziInput);
                    mpString->getCursorPos();
                    initZiString();
                }
            } else {
                confirmInput_();
            }
        } else {
            onSE((sound::SE)9);
            if (mpUnfixString->isConverting()) {
                s16 selected = mpUnfixString->getSelectedConverting();
                mpUnfixString->commitPredicted(selected);
                update();
            } else {
                confirmInput_();
            }
        }
        meScrollFlag = SF_ScrollOn;
        if (checkHeadOfSentence(true)) {
            onCommand((INPUT_COMMAND)33, NULL);
        }
        break;
    case 6:
        confirmInput_();
        break;
    case 3:
        inputInputting_(static_cast<CharInput *>(data)->ch);
        break;
    case 5: {
        KeyInput *key = static_cast<KeyInput *>(data);
        if (key->ch == L'\0') {
            return;
        }
        confirmInputting_(key->ch, key->direct, key->letterMode,
                          key->checkCandidate, key->holdingKey);
        if (!key->noSE) {
            if (key->checkCandidate) {
                onSE((sound::SE)9);
            } else {
                onSE((sound::SE)10);
            }
        }
        meScrollFlag = SF_ScrollOn;
        if (checkHeadOfSentence(true)) {
            onCommand((INPUT_COMMAND)33, NULL);
        }
        break;
    }
    case 8:
        if (string == mpString) {
            onPressLeft();
        } else {
            onHKBCtrlCode((HVKCode)31, 0);
        }
        return;
    case 9:
        if (string == mpString) {
            onPressRight();
        } else {
            onHKBCtrlCode((HVKCode)32, 0);
        }
        return;
    case 10:
        onPressUp();
        return;
    case 11:
        onPressDown();
        return;
    case 24:
        doScroll(static_cast<Scroll *>(data));
        return;
    case 14:
        if (isMainString && string->isKanaFix()) {
            nw4r::math::VEC2 pos = getGlobalLeftTopPos();
            nw4r::math::VEC2 *point = static_cast<nw4r::math::VEC2 *>(data);
            string->setCursorPos(
                calcCursorPos(point->x - pos.x, point->y - pos.y));
            onSE((sound::SE)5);
            onCommand((INPUT_COMMAND)12, NULL);
            mpUnfixString->resetRelation();
            update();
        } else {
            onSE((sound::SE)9);
            onCommand((INPUT_COMMAND)6, NULL);
        }
        return;
    case 15:
        if (string->isOnSustain()) {
            nw4r::math::VEC2 pos = getGlobalLeftTopPos();
            nw4r::math::VEC2 *point = static_cast<nw4r::math::VEC2 *>(data);
            string->setCursorPos(
                calcCursorPos(point->x - pos.x, point->y - pos.y));
            onCommand((INPUT_COMMAND)13, NULL);
        }
        return;
    case 16:
        if (string->isOnSustain()) {
            nw4r::math::VEC2 pos = getGlobalLeftTopPos();
            u32 cursor, anchor;
            string->getCursorPos(&cursor, &anchor);
            nw4r::math::VEC2 *point = static_cast<nw4r::math::VEC2 *>(data);
            u32 newPos = calcCursorPos(point->x - pos.x, point->y - pos.y);
            string->setCursorPos(newPos);
            if (cursor != newPos) {
                onSE((sound::SE)5);
            }
            meScrollFlag = SF_ScrollOn;
        }
        return;
    case 17:
        if (data != NULL && *static_cast<bool *>(data)) {
            mpUnfixString->confirm(NULL);
            onCommand((INPUT_COMMAND)6, NULL);
            CandidateBoxCaller::resetCandidate();
            CandidateBoxCaller::updateCandidate();
        }
        break;
    case 21:
        if (mbPredictOn) {
            if (mePredictMode == PM_Atok) {
                if (mpUnfixString->isConverting()) {
                    mpUnfixString->commitPredicted(*static_cast<int *>(data));
                } else {
                    mpUnfixString->getPredicted(*static_cast<int *>(data),
                                                atokPredicted);
                    mpString->getCursorPos();
                    mpString->inputString(atokPredicted,
                                          tistring::Decolated::TM_Direct);
                    mpString->getCursorPos();
                    mpUnfixString->commitPredicted(*static_cast<int *>(data));
                }
            } else {
                mpZiString->getPredicted(*static_cast<int *>(data),
                                         ziPredicted);
                mpString->getCursorPos();
                mpString->inputString(ziPredicted);
                mpString->getCursorPos();
                initZiString();
            }
        }
        meScrollFlag = SF_ScrollOn;
        break;
    case 18: {
        tistring::Decolated::TranslateMode mode =
            *static_cast<tistring::Decolated::TranslateMode *>(data);
        mpString->getCursorPos();
        mpString->setTranslateMode(mode);
        mpString->getCursorPos();
        mpUnfixString->setTranslateMode(mode);
        break;
    }
    case 19:
        mpUnfixString->changeKanaMode(*static_cast<bool *>(data));
        break;
    case 20:
        mpUnfixString->setFixMode(*static_cast<bool *>(data));
        break;
    case 22:
        moveCandidateToIdx(*static_cast<s32 *>(data));
        return;
    case 23:
        if (!mbPredictOn) {
            break;
        }
        if (mePredictMode == PM_Atok) {
            mpUnfixString->setSelectedCandidate(-1);
        }
        return;
    case 25:
        string->converDakuten();
        onSE((sound::SE)10);
        break;
    case 26:
        string->converHandaku();
        onSE((sound::SE)10);
        break;
    case 27:
        string->convertAll();
        break;
    case 28:
        string->converSmall();
        onSE((sound::SE)10);
        break;
    case 29: {
        PredictInfo *info = static_cast<PredictInfo *>(data);
        mePredictMode = info->mode;
        mbPredictOn = info->on;
        tistring::Decolated::TranslateMode mode = mpString->getTranslateMode();
        tistring::Decolated::TranslateMode unfixMode =
            mpUnfixString->getTranslateMode();
        mpString->initKanaConverter();
        mpUnfixString->initKanaConverter();
        mpString->getCursorPos();
        mpString->setTranslateMode(mode);
        mpString->getCursorPos();
        mpUnfixString->setTranslateMode(unfixMode);
        switch (mePredictMode) {
        case PM_Off:
            break;
        case PM_Atok:
            if (!mbPredictOn) {
                if (mpUnfixString->isDictionaryOpen() &&
                    string == mpUnfixString) {
                    mpUnfixString->confirm(NULL);
                    const wchar_t *confirmed =
                        mpUnfixString->getConfirmedWCString();
                    mpString->getCursorPos();
                    mpString->confirm(confirmed);
                    mpString->getCursorPos();
                    mpUnfixString->enableConfirmedString(false);
                }
            } else {
                mpUnfixString->resetRelation();
                update();
            }
            return;
        case PM_USEn:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)0);
            break;
        case PM_USFr:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)1);
            break;
        case PM_USSp:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)2);
            break;
        case PM_En:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)3);
            break;
        case PM_De:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)4);
            break;
        case PM_Fr:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)5);
            break;
        case PM_Sp:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)6);
            break;
        case PM_It:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)7);
            break;
        case PM_Nl:
            mpZiString->setPredictLaunguage(
                (tistring::WithZi::PredictLanguage)8);
            break;
        }
        onCommand((INPUT_COMMAND)30, NULL);
        return;
    }
    case 30: {
        mpUnfixString->confirm(NULL);
        const wchar_t *confirmed = mpUnfixString->getConfirmedWCString();
        mpString->getCursorPos();
        mpString->confirm(confirmed);
        mpString->getCursorPos();
        mpUnfixString->enableConfirmedString(false);
        initZiString();
        CandidateBoxCaller::resetCandidate();
        return;
    }
    case 31: {
        PredictInfo *info = static_cast<PredictInfo *>(data);
        info->mode = mePredictMode;
        info->on = mbPredictOn;
        return;
    }
    case 32:
        *static_cast<bool *>(data) = isZiPredicting();
        return;
    case 38: {
        HKBInput *input = static_cast<HKBInput *>(data);
        onHKBCtrlCode(input->code, input->flags);
        return;
    }
    case 41:
        notifyChangeMode();
        return;
    case 42:
        onPressLeftHWKB();
        return;
    case 43:
        onPressRightHWKB();
        return;
    case 44:
        onPressUp();
        return;
    case 45:
        onPressDownHWKB();
        return;
    case 39:
        KPRClearQueue(&mDKStream.mKPRQueue);
        return;
    case 46:
        return;
    case 36: {
        setStringLength_(*static_cast<u32 *>(data));
        return;
    }
    }
    update();
}

inline void Base::changeInputState(const InputState &state, InputState &prev) {
    tistring::Decolated *string = getCurrentString(false);
    prev.mode = string->getTranslateMode();
    if (string == mpString) {
        mpString->getCursorPos();
        mpString->setTranslateMode(state.mode);
        mpString->getCursorPos();
    } else {
        string->setTranslateMode(state.mode);
    }
    if (isAtokActive()) {
        prev.fixMode = mpUnfixString->isFixMode();
        prev.kanaMode = mpUnfixString->isKanaMode();
        mpUnfixString->setFixMode(state.fixMode);
        mpUnfixString->changeKanaMode(state.kanaMode);
    }
}

void Base::update() {
    wchar_t predicted[61];
    if (!mbPredictOn) {
        return;
    }
    if (mePredictMode == PM_Atok) {
        if (mpUnfixString->hasConfirmedString()) {
            const wchar_t *confirmed = mpUnfixString->getConfirmedWCString();
            mpString->getCursorPos();
            mpString->confirm(confirmed);
            mpString->getCursorPos();
            mpUnfixString->enableConfirmedString(false);
            onSE((sound::SE)9);
        }
        bool converting = false;
        CommandReceiver::onCommand((INPUT_COMMAND)0x23, &converting);
        CandidateBoxCaller::resetCandidate();
        if (!converting) {
            int num = mpUnfixString->getCurrentNumPredicted();
            for (int i = 0; i < num; i++) {
                mpUnfixString->getPredicted(i, predicted);
                CandidateBoxCaller::addCandidate(predicted);
            }
            CandidateBoxCaller::updateCandidate();
        }
    } else {
        CandidateBoxCaller::resetCandidate();
        int num = mpZiString->getCurrentNumPredicted();
        for (int i = 0; i < num; i++) {
            mpZiString->getPredicted(i, predicted);
            CandidateBoxCaller::addCandidate(predicted);
        }
        CandidateBoxCaller::updateCandidate();
    }
}

inline bool Base::isZiPredicting() {
    if (!mbPredictOn) {
        return false;
    }
    if (mePredictMode != PM_Atok && mpZiString->getCurrentNumPredicted() > 0) {
        return true;
    }
    return false;
}

void Base::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    textdrawer::Base::create(allocator);
    mpString =
        new (MEMAllocFromAllocator(allocator, sizeof(tistring::Decolated)))
            tistring::Decolated(0x400);
    mpString->create(allocator);
    mpUnfixString =
        new (MEMAllocFromAllocator(allocator, sizeof(tistring::WithAtok)))
            tistring::WithAtok(0x400);
    mpUnfixString->create(allocator);
    mpZiString =
        new (MEMAllocFromAllocator(allocator, sizeof(tistring::WithZi)))
            tistring::WithZi(0x400);
    mpZiString->create(allocator);
    mriManager.create(allocator);
    Info_ *line = mriManager.newLine();
    line->StrCount = 0;
    line->DispRowCount = 1;
    mpCursorLine = line;
}

void Base::draw() {
    SetCursor(getScale().x, mfScrollY);
    nw4r::math::VEC2 scale = getScale();
    nw4r::ut::Rect rect;
    nw4r::math::VEC2 pos = getGlobalLeftTopPos();
    rect.left = pos.x;
    rect.bottom = pos.y;
    rect.right = pos.x + scale.x * (mRect.right - mRect.left);
    rect.top = pos.y + scale.y * (mRect.top - mRect.bottom);
    u32 cursorPos = mpString->getCursorPos();
    beginDraw(rect);
    drawFixString(cursorPos);
    endDraw();
}

nw4r::math::VEC2 Base::getScale() {
    return nw4r::math::VEC2(1.0f, 1.0f);
}

void Base::makeUpCursorPos(CursorPos *cursorPos, u32 pos, s32 line,
                           s32 endLine) {
    u32 counter = muWordWrapCounter;
    muWordWrapCounter = pos;
    textdrawer::Base::makeUpCursorPos(cursorPos, pos, line, endLine);
    muWordWrapCounter = counter;
}

void Base::completeDrawCursor_(CursorPos *cursorPos) {
    if (mpString->getWCString() != NULL) {
        s32 endLine = getDrawModifyEndLine();
        makeUpCursorPos(cursorPos, getEndPos(), getLine(), endLine);
    }
}

void Base::drawFixString(u32 pos) {
    CursorPos cursorPos;
    cursorPos.uCursorPos = pos;
    cursorPos.fCursorX = 0.0f;
    cursorPos.fCursorY = 0.0f;
    mCharColor.a = muGlobalAlpha;
    SetTextColor(mCharColor);
    textdrawer::Base::setDrawString(mpString->getWCString(), 0,
                                    mpString->getLength());
    textdrawer::Base::draw(&cursorPos);
    drawCursor(cursorPos.fCursorX, cursorPos.fCursorY);
}

inline void Base::setStringLength_(u32 length) {
    if (mpString->getLength() > length) {
        mpString->getLength();
    }
    mpString->setLength(length);
}

f32 halfDifference(f32 a, f32 b) {
    return (a - b) / 2.0f;
}

void Base::calc() {
    static f32 sfSelectedColorTimer;
    if (mScrollAnm.isActive()) {
        mfScrollY = mScrollAnm.getValue();
        mfCursorCacheBaseY = mfScrollY;
    }
    if (!mScrollAnm.isActive()) {
        autoScroll();
    }
    if (!mpString->isOnSustain()) {
        mfSustainTimer = 1.0f + mfSustainTimer;
    }
    mScrollAnm.calc();

    f32 sin = nw4r::math::SinDeg(sfSelectedColorTimer);
    mSelectedColor.r = 2;
    mSelectedColor.g = 90;
    mSelectedColor.b = 253;
    mSelectedColor.a = 150.0f + 30.0f * sin;
    if (mpString->isOnSustain()) {
        mSelectedColor.r = 52;
        mSelectedColor.g = 140;
        mSelectedColor.b = 253;
        mSelectedColor.a = 50.0f + 10.0f * sin;
    } else if (mfSustainTimer <= 50.0f) {
        mSelectedColor.r = util::hermiteInterporation(
            mfSustainTimer, 0.0f, 52.0f, 0.0f, 50.0f, 2.0f, 0.0f);
        mSelectedColor.g = util::hermiteInterporation(
            mfSustainTimer, 0.0f, 140.0f, 0.0f, 50.0f, 90.0f, 0.0f);
        mSelectedColor.b = util::hermiteInterporation(
            mfSustainTimer, 0.0f, 253.0f, 0.0f, 50.0f, 253.0f, 0.0f);
        mSelectedColor.a =
            util::hermiteInterporation(mfSustainTimer, 0.0f, 50.0f, 0.0f, 50.0f,
                                       150.0f, 0.0f) +
            20.0f * nw4r::math::SinDeg(sfSelectedColorTimer);
    }
    calcCursorTimer();
    sfSelectedColorTimer += 2.0f;
}

void Base::calcCursorTimer() {
    muCursorTimer += 8;
}

void Base::setFont(const nw4r::ut::Font &font) {
    textdrawer::Base::setFont(font);
}

void Base::setString(const wchar_t *string) {
    mpString->set(string);
}

void Base::clear() {
    init();
    mpString->clear();
}

void Base::preDraw(u32 pos) {
    muWordWrapCounter = pos;
    sbWordHead = false;
}

bool Base::findURL(u32 *start, u32 *end, const wchar_t *string, u32 pos,
                   u32 length) {
    bool found = false;
    bool separated = true;
    const wchar_t(*heads)[10] = scURLHeads;
    for (u32 i = pos; i < length; i++) {
        switch (string[i]) {
        case L'\0':
            if (found && end != NULL) {
                *end = i;
            }
            return found;
        case L'\n':
        case L'\r':
        case L' ':
        case 0x3000:
            if (!found) {
                separated = true;
                break;
            }
            if (end != NULL) {
                *end = i;
            }
            return true;
        case L'h':
        case L'H':
            if (separated && !found) {
                separated = false;
                for (u32 j = 0; j < 2; j++) {
                    size_t len = wcslen(heads[j]);
                    if (__msl_wcsnicmp(&string[i], heads[j], len) == 0) {
                        if (start != NULL) {
                            *start = i;
                        }
                        found = true;
                        i = i + len - 1;
                        break;
                    }
                }
            }
            break;
        case L'0':
        case L'1':
        case L'2':
        case L'3':
        case L'4':
        case L'5':
        case L'6':
        case L'7':
        case L'8':
        case L'9':
        case L'A':
        case L'B':
        case L'C':
        case L'D':
        case L'E':
        case L'F':
        case L'G':
        case L'I':
        case L'J':
        case L'K':
        case L'L':
        case L'M':
        case L'N':
        case L'O':
        case L'P':
        case L'Q':
        case L'R':
        case L'S':
        case L'T':
        case L'U':
        case L'V':
        case L'W':
        case L'X':
        case L'Y':
        case L'Z':
        case L'a':
        case L'b':
        case L'c':
        case L'd':
        case L'e':
        case L'f':
        case L'g':
        case L'i':
        case L'j':
        case L'k':
        case L'l':
        case L'm':
        case L'n':
        case L'o':
        case L'p':
        case L'q':
        case L'r':
        case L's':
        case L't':
        case L'u':
        case L'v':
        case L'w':
        case L'x':
        case L'y':
        case L'z':
            separated = false;
            break;
        default:
            separated = true;
            break;
        }
    }
    return found;
}

bool Base::doWordWrap(const wchar_t *string, u32 pos, f32 x) {
    if (!mbDoWordWrap) {
        return false;
    }
    if (pos < muWordWrapCounter) {
        return false;
    }
    if (!util::isAlphabet(string[muWordWrapCounter])) {
        muWordWrapCounter = pos;
        return false;
    }

    u32 start = muWordWrapCounter;
    int prevType = 0;
    if (start != 0 && start != getDrawCacheStartPos()) {
        wchar_t prev = string[start - 1];
        if (prev == L'-') {
            prevType = 1;
        } else if (prev != L' ') {
            prevType = 2;
        }
    }

    u32 i;
    f32 width = 0.0f;
    u32 hyphen = 0;
    start = muWordWrapCounter;
    bool wrap = false;
    i = start;
    do {
        if (string[i] == L' ' || string[i] == L'\n') {
            break;
        }
        DrawInfo drawInfo;
        drawInfo.wc = string[i];
        calcRect(drawInfo);
        width += drawInfo.rect.right - drawInfo.rect.left;
        bool over = x != getScale().x &&
                    x + width * getScale().x >= mRect.GetWidth() * getScale().x;
        if (over) {
            wrap = true;
        }
        if (i != start && string[i] == L'-') {
            hyphen = i;
            sbWordHead = true;
        }
        i++;
    } while (string[i] != L'\0');

    if (i == start) {
        sbWordHead = false;
    }
    if (width * getScale().x >= mRect.GetWidth() * getScale().x) {
        wrap = false;
        if (sbWordHead) {
            wrap = true;
        }
        if (prevType == 1) {
            wrap = true;
        }
    }
    if (wrap) {
        if (!findURL(NULL, NULL, string, start, i)) {
            if (hyphen == 0) {
                sbWordHead = false;
                muWordWrapCounter = i + 1;
                return true;
            }
            i = hyphen;
        }
    } else {
        sbWordHead = false;
    }
    muWordWrapCounter = i + 1;
    return false;
}

void Base::doBeforeDrawProcess(const wchar_t *wcString, u32 pos,
                               const DrawInfo &drawInfo) {
    if (doWordWrap(wcString, pos, GetCursorX())) {
        doLineFeed();
    }
    if (GetCursorX() + drawInfo.rect.GetWidth() * getScale().x >=
        mRect.GetWidth() * getScale().x) {
        Base::doLineFeed();
    }
    u32 start, end;
    mpString->getSelected(start, end);
    if (pos >= start && pos < end) {
        f32 left = drawInfo.rect.left + GetCursorX();
        f32 top = drawInfo.rect.top + GetCursorY();
        f32 right = drawInfo.rect.right + GetCursorX();
        f32 bottom = drawInfo.rect.bottom + GetCursorY();
        debug::drawBox_(left, top, right, bottom, 0.0f, 1.0f, mSelectedColor);
    }
}

void Base::doLineFeed() {
    miLineFeedNum++;
    SetCursorY(GetCursorY() + getLineHeight());
    SetCursorX(getScale().x);
}

bool Base::onCursor(CursorPos *cursorPos) {
    bool ret = false;
    nw4r::ut::Color textColor = GetTextColor();
    SetupGX();
    wchar_t *kana = mpString->getKanaBuffer();
    wchar_t candidate = mpString->getCandidate();
    if (mbPredictOn && mePredictMode != PM_Atok) {
        wchar_t *input = mpZiString->getCurrentInput();
        if (input != NULL) {
            u16 i = 0;
            SetTextColor(sZiInputColor);
            for (; *input != L'\0'; input++, i++) {
                if (!(mbCursorSelected | mbZuSelected) &&
                    i == (u16)mpZiString->getInputStringLength()) {
                    cursorPos->fCursorX = GetCursorX();
                    cursorPos->fCursorY = GetCursorY();
                    ret = true;
                }
                if ((u16)mpZiString->getInputStringLength() <= i) {
                    if (mbCursorSelected | mbZuSelected) {
                        SetTextColor(sZiSelectedColor);
                    } else {
                        SetTextColor(sZiPredictColor);
                    }
                }
                if (*input == 0xFFFE) {
                    SetTextColor(sZiInputColor);
                }
                DrawInfo drawInfo;
                drawInfo.wc = *input;
                calcRect(drawInfo);
                if (GetCursorX() + drawInfo.rect.GetWidth() >=
                    mRect.GetWidth()) {
                    doLineFeed();
                }
                Print(*input);
                MoveCursorX(mfCharSpace);
            }
        }
    }

    if (meDestination == DST_JP && candidate != L'\0') {
        DrawInfo drawInfo;
        drawInfo.wc = candidate;
        calcRect(drawInfo);
        f32 left = drawInfo.rect.left + GetCursorX();
        f32 y = GetCursorY();
        f32 top = drawInfo.rect.top + y;
        f32 right = drawInfo.rect.right + GetCursorX();
        f32 height = GetFontHeight();
        f32 bottom = height + y;
        sCandidateBoxColor.a = muGlobalAlpha;
        debug::drawBox_(left, top, right, bottom, 0.0f, 1.0f,
                        sCandidateBoxColor);
        SetupGX();
        SetTextColor(mCharColor);
        switch (candidate) {
        case 0x3000:
        case L' ': {
            DrawInfo spaceInfo;
            spaceInfo.wc = candidate;
            DrawInfo markInfo;
            markInfo.wc = 0xE057;
            calcRect(spaceInfo);
            calcRect(markInfo);
            f32 markWidth = markInfo.rect.GetWidth();
            f32 spaceWidth = spaceInfo.rect.GetWidth();
            f32 offset = getScale().x * halfDifference(spaceWidth, markWidth);
            MoveCursorX(offset);
            Print(0xE057);
            MoveCursorX(offset);
            break;
        }
        default:
            Print(candidate);
            break;
        }
        MoveCursorX(mfCharSpace);
    }

    if (meDestination != DST_JP) {
        wchar_t ch = mpString->getCandidate();
        if (ch != L'\0') {
            SetTextColor(sCandidateColor);
            if (ch == L' ') {
                SetTextColor(sSpaceCandidateColor);
                ch = 0xE057;
            }
            Print(ch);
            MoveCursorX(mfCharSpace);
        }
    }

    for (; *kana != L'\0'; kana++) {
        wchar_t ch = *kana;
        if (mbPredictOn) {
            ch = util::HankakuToZenkaku(ch);
        }
        DrawInfo drawInfo;
        drawInfo.wc = ch;
        calcRect(drawInfo);
        f32 left = drawInfo.rect.left + GetCursorX();
        f32 y = GetCursorY();
        f32 top = drawInfo.rect.top + y;
        f32 right = drawInfo.rect.right + GetCursorX();
        f32 height = GetFontHeight();
        f32 bottom = height + y;
        sKanaBoxColor.a = muGlobalAlpha;
        debug::drawBox_(left, top, right, bottom, 0.0f, 1.0f, sKanaBoxColor);
        SetupGX();
        mCharColor.a = muGlobalAlpha;
        SetTextColor(mCharColor);
        Print(ch);
        MoveCursorX(mfCharSpace);
    }
    SetTextColor(textColor);
    return ret;
}

u32 Base::getCursorPos() {
    u32 cursor, unfixCursor, anchor;
    mpString->getCursorPos(&cursor, &anchor);
    mpUnfixString->getCursorPos(&unfixCursor, &anchor);
    return cursor + unfixCursor;
}

void Base::drawCursor(f32 x, f32 y) {
    mfCursorX = x;
    mfCursorY = y;
    f32 alpha = muGlobalAlpha / 255.0f;
    GXColor color = {0xFF, 0x32, 0x32, 0x00};
    color.a = alpha * (127.0f + 127.0f * nw4r::math::SinDeg(muCursorTimer));
    f32 projWidth = mProjectionRect.GetWidth();
    debug::drawLine_(mfCursorX, 2.0f + mfCursorY, mfCursorX,
                     mfCursorY + GetFontHeight() - 2.0f, 0.0f,
                     14592.0f / projWidth, color);
}

u32 Base::calcCursorPos(f32 x, f32 y) {
    y -= mfScrollY;
    const wchar_t *string = mpString->getWCString();
    if (*string == L'\0') {
        return 0;
    }
    f32 cursorX = getScale().x;
    f32 cursorY = 0.0f;
    f32 lastTop = 0.0f;
    f32 lastBottom = 0.0f;
    nw4r::math::VEC2 scale = getScale();
    u32 pos = 0;
    muWordWrapCounter = 0;
    sbWordHead = false;
    while (*string != L'\0') {
        DrawInfo drawInfo;
        drawInfo.wc = *string;
        calcRect(drawInfo);
        f32 left = cursorX;
        f32 top = cursorY;
        f32 right =
            cursorX + (drawInfo.rect.right - drawInfo.rect.left) * scale.x;
        f32 bottom =
            cursorY + (drawInfo.rect.bottom - drawInfo.rect.top) * scale.y;
        lastTop = top;
        lastBottom = bottom;
        if (doWordWrap(mpString->getWCString(), pos, cursorX)) {
            if (y >= top && y <= bottom) {
                if (pos != 0) {
                    return pos - 1;
                }
                return pos;
            }
            cursorY = cursorY + getLineHeight() * scale.y;
            cursorX = getScale().x;
            left = cursorX;
            top = cursorY;
            right =
                cursorX + (drawInfo.rect.right - drawInfo.rect.left) * scale.x;
            bottom =
                cursorY + (drawInfo.rect.bottom - drawInfo.rect.top) * scale.y;
            lastTop = top;
            lastBottom = bottom;
        }
        if (right >= mRect.GetWidth() * scale.x) {
            if (y >= top && y <= bottom) {
                return pos;
            }
            cursorY = cursorY + getLineHeight() * scale.y;
            cursorX = getScale().x;
            left = cursorX;
            top = cursorY;
            right =
                cursorX + (drawInfo.rect.right - drawInfo.rect.left) * scale.x;
            bottom =
                cursorY + (drawInfo.rect.bottom - drawInfo.rect.top) * scale.y;
            lastTop = top;
            lastBottom = bottom;
        }
        if (x <= drawInfo.rect.left && y >= top && y <= bottom) {
            return pos;
        }
        if (*string == L'\n') {
            if (y >= top && y <= bottom) {
                return pos;
            }
            cursorY = cursorY + getLineHeight() * scale.y;
            cursorX = getScale().x;
            top = cursorY;
            bottom =
                cursorY + (drawInfo.rect.bottom - drawInfo.rect.top) * scale.y;
            lastTop = top;
            lastBottom = bottom;
            if (string[1] == L'\0') {
                if (y >= top && y <= bottom) {
                    return pos + 1;
                }
            }
        } else {
            if (x >= left && x < right && y >= top && y < bottom) {
                if ((right - left) / 2 + left > x) {
                    return pos;
                }
                return pos + 1;
            }
            cursorX += (drawInfo.rect.right - drawInfo.rect.left) * scale.x;
        }
        pos++;
        string++;
        if (*string == L'\0' && y >= top && y <= bottom) {
            return pos;
        }
    }
    if ((lastBottom - lastTop) / 2 + lastTop <= y) {
        return Base::calcCursorPos(x, lastTop + 1 + mfScrollY);
    }
    return Base::calcCursorPos(x, 1);
}

nw4r::math::VEC2 Base::getGlobalLeftTopPos() const {
    nw4r::math::VEC3 pos;
    pos.x = mRect.left;
    pos.y = mRect.bottom;
    pos.z = 0.0f;
    PSMTXMultVec(mMtx, pos, pos);
    return nw4r::math::VEC2(pos.x, -pos.y);
}

void Base::doScroll(Scroll *scroll) {
    if (!mScrollAnm.isActive()) {
        if (scroll->absY) {
            mfScrollX = scroll->x;
            mfScrollY = scroll->y;
            mfCursorCacheBaseY = mfScrollY;
        } else {
            mfScrollX += scroll->x;
            mScrollAnm.startAnm(mfScrollY, mfScrollY + scroll->y, 15.0f, NULL,
                                NULL);
        }
        onSE((sound::SE)11);
    }
}

void Base::autoScroll() {
    if (mScrollAnm.isActive()) {
        return;
    }
    if (meScrollFlag == SF_ScrollOn || meScrollFlag == SF_ScrollByBS) {
        f32 cursorY = mfCursorY + getLineHeight() / 2.0f;
        if (cursorY < 0.0f) {
            int lines = cursorY / getLineHeight() - 1.0f;
            mScrollAnm.startAnm(mfScrollY, mfScrollY - lines * getLineHeight(),
                                15.0f, NULL, NULL);
            onSE((sound::SE)11);
        }
        if (cursorY >= (mRect.top - mRect.bottom) * getScale().y) {
            int lines =
                (int)((cursorY - (mRect.top - mRect.bottom) * getScale().y) /
                      getLineHeight()) +
                1;
            mScrollAnm.startAnm(mfScrollY, mfScrollY - lines * getLineHeight(),
                                15.0f, NULL, NULL);
            onSE((sound::SE)11);
        }
        meScrollFlag = SF_NoScroll;
    }
}

void Base::setAtokDictionary(void *atokDict, int atokDictSize, void *apotDict,
                             int apotDictSize, void *nintendoDict,
                             int nintendoDictSize) {
    mpUnfixString->openDictionary(atokDict, atokDictSize, apotDict,
                                  apotDictSize, nintendoDict, nintendoDictSize);
}

void Base::closeAtokDictionary() {
    mpUnfixString->closeDictionary();
}

void Base::confirmInput_() {
    if (!mbPredictOn) {
        if (!mpString->isKanaFix()) {
            mpString->getCursorPos();
            mpString->confirmKana();
            mpString->getCursorPos();
        }
    } else if (mePredictMode == PM_Atok) {
        if (mpUnfixString->isCandidateSelected()) {
            int selected = mpUnfixString->getSelectedCandidate();
            onCommand((INPUT_COMMAND)21, &selected);
        } else {
            mpUnfixString->confirm(NULL);
            const wchar_t *confirmed = mpUnfixString->getConfirmedWCString();
            mpString->getCursorPos();
            mpString->confirm(confirmed);
            mpString->getCursorPos();
            mpUnfixString->enableConfirmedString(false);
        }
    } else {
        wchar_t *input = mpZiString->getCurrentInput();
        mpString->getCursorPos();
        mpString->inputString(input);
        mpString->getCursorPos();
        mpZiString->init();
        mbZuSelected = false;
    }
}

inline void Base::initZiString() {
    mpZiString->init();
    mbZuSelected = false;
}

void Base::inputInputting_(wchar_t ch) {
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    if (mbPredictOn && mePredictMode == PM_Atok) {
        if (ch == 0x309B || ch == 0x309C) {
            onCommand((INPUT_COMMAND)27, NULL);
        } else {
            if (mpUnfixString->isCandidateSelected()) {
                onCommand((INPUT_COMMAND)6, NULL);
            }
            mpUnfixString->setInputing(ch);
        }
    } else if (ch == 0x309B || ch == 0x309C) {
        onCommand((INPUT_COMMAND)27, NULL);
    } else {
        string->setCandidate(ch);
    }
    if (ch != L'\0') {
        onSE((sound::SE)10);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::confirmInputting_(wchar_t ch, bool direct, u16 letterMode,
                             bool checkCandidate, void *holdingKey) {
    tistring::Decolated *string = getCurrentString(true);
    if (ch == L'\0') {
        return;
    }
    if (direct) {
        mpString->getCursorPos();
        mpString->inputChar(ch);
        mpString->getCursorPos();
        return;
    }
    if (string == mpZiString) {
        if ((u16)mpZiString->getInputStringLength() == 0) {
            mpZiString->changeLetterMode(
                (tistring::WithZi::LetterMode)letterMode);
            mpZiString->setCellPhoneHoldingkey(holdingKey);
        }
        if ((u16)mpZiString->getInputStringLength() >= 32) {
            onCommand((INPUT_COMMAND)6, NULL);
            mpZiString->changeLetterMode(
                (tistring::WithZi::LetterMode)letterMode);
            mpZiString->inputChar(ch);
            onSE((sound::SE)9);
            update();
            return;
        }
    } else if (string == mpUnfixString) {
        if (checkCandidate && !mpUnfixString->hasCandidate()) {
            return;
        }
    } else if (mbPredictOn && mePredictMode == PM_Atok &&
               string != mpUnfixString && ch == L' ') {
        ch = 0x3000;
        mpUnfixString->setCandidate(L'\0');
    }
    if (string == mpString) {
        mpString->getCursorPos();
        mpString->inputChar(ch);
        mpString->getCursorPos();
    } else {
        string->inputChar(ch);
    }
    onSE((sound::SE)10);
}

void Base::inputCharZi_(wchar_t ch, u32 flags) {
    u32 caseMode = flags & 3;
    if (caseMode != 0) {
        ch = util::toWLower(ch);
    }
    if (ch == L' ') {
        if (isZiPredicting()) {
            confirmInput_();
            mpString->getCursorPos();
            mpString->inputChar(ch);
            mpString->getCursorPos();
        } else {
            mpString->getCursorPos();
            mpString->inputChar(ch);
            mpString->getCursorPos();
        }
        onSE((sound::SE)9);
    } else {
        u16 letterMode;
        switch (caseMode) {
        case 1:
            letterMode = 0;
            break;
        case 2:
            letterMode = 2;
            break;
        default:
            letterMode = 1;
            break;
        }
        confirmInputting_(ch, false, letterMode, false, NULL);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::inputCharDefault_(wchar_t ch) {
    tistring::Decolated *string = getCurrentString(true);
    if (getCurrentString(false) == mpUnfixString &&
        !mpUnfixString->isConverting() &&
        mpUnfixString->getSelectedCandidate() >= 0) {
        confirmInput_();
    }
    if (mpString == string) {
        mpString->getCursorPos();
        mpString->inputChar(ch);
        mpString->getCursorPos();
    } else {
        string->inputChar(ch);
    }
    if (ch == L' ') {
        onSE((sound::SE)9);
    } else if (ch != L'\n') {
        onSE((sound::SE)10);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::onClose() {
    onCommand((INPUT_COMMAND)6, NULL);
    CandidateBoxCaller::makeEmptyCandidate();
    dirtyDrawCache();
    dirtyCursorCache();
}

tistring::Decolated *Base::getCurrentString(bool ignoreFix) {
    if (!mbPredictOn) {
        return mpString;
    }
    switch (mePredictMode) {
    case PM_Atok:
        if (!mpUnfixString->isFix() || ignoreFix) {
            return mpUnfixString;
        }
        break;
    default:
        if (!mpZiString->isFix() || ignoreFix) {
            return mpZiString;
        }
        break;
    }
    return mpString;
}

bool Base::isAtokActive() {
    if (!mbPredictOn) {
        return false;
    }
    if (mePredictMode == PM_Atok) {
        return true;
    }
    return false;
}

void Base::onHKBCtrlCode(HVKCode code, u32 flags) {
    tistring::Decolated *string = getCurrentString(false);
    switch (code) {
    case 2:
        if (mePredictMode == PM_Atok) {
            toggleAtokMode_(0);
        }
        break;
    case 17:
        if (mePredictMode == PM_Atok) {
            if (mpManager->getPCKeyboard()->getTranslateMode() ==
                keyboard::pctype::Base::TM_Hiragana) {
                mpManager->getPCKeyboard()->setTranslateMode(
                    keyboard::pctype::Base::TM_Katakana);
            } else if (mpManager->getPCKeyboard()->getTranslateMode() ==
                       keyboard::pctype::Base::TM_Katakana) {
                mpManager->getPCKeyboard()->setTranslateMode(
                    keyboard::pctype::Base::TM_Hiragana);
            }
            toggleAtokMode_(1);
        }
        break;
    case 18:
        if (mePredictMode == PM_Atok) {
            if (mpManager->getPCKeyboard()->getTranslateMode() ==
                    keyboard::pctype::Base::TM_Direct &&
                !mbPredictOn) {
                if (!mpManager->getPCKeyboard()->mbKanaLock) {
                    onSE((sound::SE)6);
                }
            } else {
                toggleAtokMode_(2);
            }
        }
        break;
    case 36:
        if (mePredictMode == PM_Atok) {
            toggleAtokMode_(0);
        } else if (mePredictMode != PM_Off &&
                   mpManager->getCandidateBox()->isActive()) {
            mpManager->startPredictTurnOn(!mbPredictOn);
        }
        break;
    case 1:
        if (string == mpUnfixString) {
            update();
        }
        break;
    case 9:
        onCommand((INPUT_COMMAND)7, NULL);
        onCommand((INPUT_COMMAND)39, NULL);
        break;
    case 8:
        if (isZiActive()) {
            if (isZiPredicting()) {
                if (!(mbCursorSelected | mbZuSelected) &&
                    mpManager->getToolBar()->isQwerty()) {
                    if (mbPredictOn && mePredictMode != PM_Atok) {
                        wchar_t input[64];
                        mpZiString->getCurrentInput(input, 64);
                        mpString->getCursorPos();
                        mpString->inputString(input);
                        mpString->getCursorPos();
                        initZiString();
                    }
                } else {
                    confirmInput_();
                }
                update();
            }
        } else if (isAtokActive() && string == mpUnfixString) {
            if (!mpUnfixString->isKanaFix()) {
                mpUnfixString->confirmKana();
            }
            if (mpUnfixString->isConverting()) {
                if (mpManager->getCandidateBox()->isInScroll()) {
                    break;
                }
                if (flags & 2) {
                    if (getCurrentString(false) == mpUnfixString) {
                        s16 index = mpUnfixString->getSelectedConverting() - 1;
                        if (index >= mpUnfixString->getCurrentNumPredicted()) {
                            index = 0;
                        } else if (index < 0) {
                            index = mpUnfixString->getCurrentNumPredicted() - 1;
                        }
                        moveCandidateToIdx(index);
                        mpManager->getCandidateBox()
                            ->mTextArea.ScrollToSelectedText();
                    }
                } else if (getCurrentString(false) == mpUnfixString) {
                    s16 index = mpUnfixString->getSelectedConverting() + 1;
                    if (index >= mpUnfixString->getCurrentNumPredicted()) {
                        index = 0;
                    } else if (index < 0) {
                        index = mpUnfixString->getCurrentNumPredicted() - 1;
                    }
                    moveCandidateToIdx(index);
                    mpManager->getCandidateBox()
                        ->mTextArea.ScrollToSelectedText();
                }
                onSE((sound::SE)6);
                mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
            } else {
                startConvertAtok_();
            }
            break;
        }
        if (KPRLookAhead(&mDKStream.mKPRQueue, NULL, 0)) {
            KPRPutChar(&mDKStream.mKPRQueue, 0xFFFF);
        } else {
            KPRPutChar(&mDKStream.mKPRQueue, L' ');
        }
        if (isZiActive()) {
            for (;;) {
                wchar_t ch = KPRGetChar(&mDKStream.mKPRQueue);
                if (sbFilterInvalidChars) {
                    while (!isValidInputChar(ch, sInvalidChars)) {
                        ch = KPRGetChar(&mDKStream.mKPRQueue);
                    }
                }
                if (ch == L'\0') {
                    break;
                }
                inputCharZi_(ch, flags);
            }
        } else {
            for (;;) {
                wchar_t ch = KPRGetChar(&mDKStream.mKPRQueue);
                if (sbFilterInvalidChars) {
                    while (!isValidInputChar(ch, sInvalidChars)) {
                        ch = KPRGetChar(&mDKStream.mKPRQueue);
                    }
                }
                if (ch == L'\0') {
                    break;
                }
                inputCharDefault_(ch);
            }
        }
        update();
        break;
    case 29:
        onCommand((INPUT_COMMAND)44, NULL);
        break;
    case 30:
        onCommand((INPUT_COMMAND)45, NULL);
        break;
    case 31:
        onCommand((INPUT_COMMAND)42, NULL);
        break;
    case 32:
        onCommand((INPUT_COMMAND)43, NULL);
        break;
    case 16:
        if (mePredictMode == PM_Atok) {
            if (mpManager->getPCKeyboard()->getTranslateMode() ==
                keyboard::pctype::Base::TM_Hiragana) {
                mpManager->getPCKeyboard()->setTranslateMode(
                    keyboard::pctype::Base::TM_Katakana);
            } else if (mpManager->getPCKeyboard()->getTranslateMode() ==
                       keyboard::pctype::Base::TM_Katakana) {
                mpManager->getPCKeyboard()->setTranslateMode(
                    keyboard::pctype::Base::TM_Hiragana);
            } else if (!mpManager->getPCKeyboard()->mbKanaLock) {
                mpManager->getPCKeyboard()->setTranslateMode(
                    keyboard::pctype::Base::TM_Hiragana);
            }
        }
        break;
    }
}

void Base::moveCursorUp() {
    f32 y = mfCursorY;
    if (-(y - mfScrollY) >= 0.0f) {
        u32 cursor, anchor;
        mpString->getCursorPos(&cursor, &anchor);
        if (cursor == 0 && anchor == 0) {
            onSE((sound::SE)6);
        } else {
            onSE((sound::SE)5);
        }
        mpString->setCursorPos(0);
    } else {
        y = y >= 0.0f ? y : 0.0f;
        mpString->setCursorPos(
            calcCursorPos(mfCursorX, 1.0f + (y - getLineHeight())));
        onSE((sound::SE)5);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::moveCursorDown() {
    if (-(mfCursorY - mfScrollY) < mfTextHeight) {
        u32 cursor, anchor;
        mpString->getCursorPos(&cursor, &anchor);
        if (cursor == (u16)mpString->getLength() &&
            anchor == (u16)mpString->getLength()) {
            onSE((sound::SE)6);
        } else {
            onSE((sound::SE)5);
        }
        mpString->setCursorPos(mpString->getLength());
    } else {
        mpString->setCursorPos(
            calcCursorPos(mfCursorX, 1.0f + (mfCursorY + getLineHeight())));
        onSE((sound::SE)5);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::onPressUp() {
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    if (mpZiString == string) {
        return;
    }
    if (mpUnfixString == string) {
        if (!mpUnfixString->isKanaFix()) {
            mpUnfixString->confirmKana();
        }
        if (mpUnfixString->isConverting()) {
            if (mpManager->getCandidateBox()->isInScroll()) {
                return;
            }
            if (getCurrentString(false) == mpUnfixString) {
                s16 index = mpUnfixString->getSelectedConverting() - 1;
                if (index >= mpUnfixString->getCurrentNumPredicted()) {
                    index = 0;
                } else if (index < 0) {
                    index = mpUnfixString->getCurrentNumPredicted() - 1;
                }
                moveCandidateToIdx(index);
                mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
            }
            onSE((sound::SE)6);

        } else {
            startConvertAtok_();
        }
        return;
    }
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (mpUnfixString->getCandidate() != L' ') {
            moveCursorUp();
            onCommand((INPUT_COMMAND)46, NULL);
        }
    }
}

void Base::onPressDown() {
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    if (mpZiString == string) {
        return;
    }
    if (mpUnfixString == string) {
        if (!mpUnfixString->isKanaFix()) {
            mpUnfixString->confirmKana();
        }
        if (mpUnfixString->isConverting()) {
            if (mpManager->getCandidateBox()->isInScroll()) {
                return;
            }
            if (getCurrentString(false) == mpUnfixString) {
                s16 index = mpUnfixString->getSelectedConverting() + 1;
                if (index >= mpUnfixString->getCurrentNumPredicted()) {
                    index = 0;
                } else if (index < 0) {
                    index = mpUnfixString->getCurrentNumPredicted() - 1;
                }
                moveCandidateToIdx(index);
                mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
            }
            onSE((sound::SE)6);

        } else {
            startConvertAtok_();
        }
        return;
    }
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (mpUnfixString->getCandidate() != L' ') {
            moveCursorDown();
            onCommand((INPUT_COMMAND)46, NULL);
        }
    }
}

void Base::onPressLeft() {
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    string->getCursorPos();
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (mpUnfixString->getCandidate() == L' ') {
            return;
        }
    }
    if (string->moveCursorLeft()) {
        onSE((sound::SE)5);
    } else {
        onSE((sound::SE)6);
    }
    if (string == mpUnfixString) {
        update();
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::onPressRight() {
    wchar_t atokPredicted[62];
    wchar_t ziPredicted[62];
    wchar_t ziInput[64];
    tistring::Decolated *string = getCurrentString(false);
    string->getCursorPos();
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (mpUnfixString->getCandidate() == L' ') {
            return;
        }
    }
    if (string->moveCursorRight() && mbRightWithSpace) {
        CharInput input = {L' '};
        onCommand((INPUT_COMMAND)0, &input);
        onSE((sound::SE)5);
    }
    if (string == mpZiString || string == mpUnfixString) {
        onSE((sound::SE)6);
    } else {
        onSE((sound::SE)5);
    }
    if (string == mpUnfixString) {
        update();
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::onPressDownHWKB() {
    tistring::Decolated *string = getCurrentString(false);
    u32 modifier = input::HKBManager::sInstance.GetModifierState();
    if (string == mpString) {
        mpUnfixString->resetRelation();
        update();
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        if (mpUnfixString->getCandidate() != L' ') {
            moveCursorDown();
            onCommand((INPUT_COMMAND)46, NULL);
        }
    } else if (mpUnfixString == string) {
        if (!mpUnfixString->isKanaFix()) {
            mpUnfixString->confirmKana();
        }
        if (mpUnfixString->isConverting()) {
            if (modifier & 2) {
                s16 selected = mpUnfixString->getSelectedConverting();
                mpUnfixString->commitPredicted(selected);
                update();
                return;
            }
            if (mpManager->getCandidateBox()->isInScroll()) {
                return;
            }
            if (getCurrentString(false) == mpUnfixString) {
                s16 index = mpUnfixString->getSelectedConverting() + 1;
                if (index >= mpUnfixString->getCurrentNumPredicted()) {
                    index = 0;
                } else if (index < 0) {
                    index = mpUnfixString->getCurrentNumPredicted() - 1;
                }
                moveCandidateToIdx(index);
                mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
            }
            onSE((sound::SE)6);
        } else {
            startConvertAtok_();
        }
    } else if (mpZiString == string) {
        return;
    }
}

void Base::onPressLeftHWKB() {
    tistring::Decolated *string = getCurrentString(false);
    input::HKBManager::sInstance.GetModifierState();
    string->getCursorPos();
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (string->moveCursorLeft()) {
            onSE((sound::SE)5);
        } else {
            onSE((sound::SE)6);
        }
    } else if (string == mpUnfixString) {
        if (mpManager->getCandidateBox()->isInScroll()) {
            return;
        }
        if (mpUnfixString->getCurrentNumPredicted() <= 0) {
            return;
        }
        if (mpUnfixString->isConverting()) {
            onPressLeft();
            return;
        }
        s16 index = mpUnfixString->getSelectedCandidate() - 1;
        if (index < -1) {
            index = 0;
        } else if (index < 0) {
            index = mpUnfixString->getCurrentNumPredicted() - 1;
        }
        moveCandidateToIdx(index);
        mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
        onSE((sound::SE)6);
    } else if (string == mpZiString) {
        if (mpManager->getCandidateBox()->isInScroll()) {
            return;
        }
        bool zuSelected = mbZuSelected;
        mbZuSelected = true;
        if (mpZiString->getCurrentNumPredicted() < 0) {
            return;
        }
        s32 index = -1;
        if (zuSelected) {
            index = -2;
        }
        if (index < 0) {
            index = mpZiString->getCurrentNumPredicted() - 1;
        }
        moveCandidateToIdx(index);
        mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
        onSE((sound::SE)6);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::onPressRightHWKB() {
    tistring::Decolated *string = getCurrentString(false);
    input::HKBManager::sInstance.GetModifierState();
    string->getCursorPos();
    if (string == mpString) {
        if (string->hasCandidate() || !string->isKanaFix()) {
            return;
        }
        mpUnfixString->resetRelation();
        update();
        if (string->moveCursorRight()) {
            onSE((sound::SE)6);
        } else {
            onSE((sound::SE)5);
        }
    } else if (string == mpUnfixString) {
        if (mpManager->getCandidateBox()->isInScroll()) {
            return;
        }
        if (mpUnfixString->getCurrentNumPredicted() <= 0) {
            return;
        }
        if (mpUnfixString->isConverting()) {
            string->moveCursorRight();
            onSE((sound::SE)6);
            update();
        } else {
            s16 index = mpUnfixString->getSelectedCandidate() + 1;
            if (index >= mpUnfixString->getCurrentNumPredicted()) {
                index = 0;
            }
            moveCandidateToIdx(index);
            mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
            onSE((sound::SE)6);
        }
    } else if (string == mpZiString) {
        if (mpManager->getCandidateBox()->isInScroll()) {
            return;
        }
        bool zuSelected = mbZuSelected;
        mbZuSelected = true;
        if (mpZiString->getCurrentNumPredicted() < 0) {
            return;
        }
        s32 index = -1;
        if (zuSelected) {
            index = 0;
        }
        if (index >= mpZiString->getCurrentNumPredicted()) {
            index = 0;
        }
        moveCandidateToIdx(index);
        mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
        onSE((sound::SE)6);
    }
    meScrollFlag = SF_ScrollOn;
}

void Base::moveCandidateToIdx(s32 index) {
    if (mbPredictOn) {
        if (mpManager->getCandidateBox()->isInScroll()) {
            return;
        }
        if (mePredictMode == PM_Atok) {
            if (mpUnfixString->getFixedPredictionNum() != 0 &&
                !mpUnfixString->isConverting()) {
                mpUnfixString->confirm(NULL);
                const wchar_t *confirmed =
                    mpUnfixString->getConfirmedWCString();
                mpString->getCursorPos();
                mpString->confirm(confirmed);
                mpString->getCursorPos();
                mpUnfixString->enableConfirmedString(false);
            }
            mpUnfixString->setSelectedCandidate(index);
        } else {
            mpZiString->setSelectedCandidate(index);
        }
        mpManager->getCandidateBox()->mTextArea.ChangeSelectedText(index);
    } else {
        meScrollFlag = SF_ScrollOn;
    }
}

inline void Base::startConvertAtok_() {
    if ((u16)mpUnfixString->getInputStringLength() != 0) {
        mpUnfixString->startConverting();
        update();
        moveCandidateToIdx(0);
        mpManager->getCandidateBox()->mTextArea.ScrollToSelectedText();
        onCommand((INPUT_COMMAND)46, NULL);
    }
}

void Base::toggleAtokMode_(u8 mode) {
    bool enable;
    if (!(mpManager->getToolBar()->isHWKBMode() ||
          mpManager->getToolBar()->isQwerty())) {
        enable = false;
    } else {
        enable = !mpManager->getPCKeyboard()->mbKanaLock;
    }

    if (!enable) {
        return;
    }
    bool on;
    if (mpManager->getCandidateBox()->isActive()) {
        if (mpManager->isPredictTurning()) {
            return;
        }
        on = !mbPredictOn;
        if (mpManager->getPCKeyboard()->getTranslateMode() ==
            keyboard::pctype::Base::TM_Direct) {
            on = true;
        }
    } else {
        on = mpManager->getPCKeyboard()->getTranslateMode() ==
             keyboard::pctype::Base::TM_Direct;
    }
    if (on && mode == 2) {
        on = false;
    } else if (!on && mode == 1) {
        on = true;
    }
    if (on) {
        if (mpManager->getPCKeyboard()->getTranslateMode() ==
            keyboard::pctype::Base::TM_Direct) {
            mpManager->getPCKeyboard()->setTranslateMode(
                keyboard::pctype::Base::TM_Hiragana);
        }
    } else {
        mpManager->getPCKeyboard()->setTranslateMode(
            keyboard::pctype::Base::TM_Direct);
    }
    if (mpManager->getCandidateBox()->isActive() && mbPredictOn != on) {
        mpManager->startPredictTurnOn(!mbPredictOn);
    }
}

void Base::notifyChangeMode() {
    if (mePredictMode == PM_Atok) {
        if (!mpManager->getToolBar()->isQwerty() ||
            !mpManager->getPCKeyboard()->isABC() ||
            mpManager->getPCKeyboard()->getTranslateMode() !=
                keyboard::pctype::Base::TM_Direct) {
            if (!mpManager->getToolBar()->isQwerty()) {
                mpManager->getCellPhoneKeyboard()->getInputMode();
            }
        }
        if ((u16)mpUnfixString->getInputStringLength() == 0) {
            update();
        }
    }
}

LayoutByNW4R::~LayoutByNW4R() {
    if (mpEventHandler != NULL) {
        mpEventHandler->~EventHandler();
        MEMFreeToAllocator(mpAllocator, mpEventHandler);
    }
    for (nw4rmanager::AnmPane *pane = static_cast<nw4rmanager::AnmPane *>(
             nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<nw4rmanager::AnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, NULL))) {
        nw4r::ut::List_Remove(&mAnmPanes, pane);
        pane->destroy(mpAllocator);
    }
}

inline nw4r::lyt::TextBox &LayoutByNW4R::getTextBox_() {
    return *static_cast<nw4r::lyt::TextBox *>(
        mpLayout->GetRootPane()->FindPaneByName(mpszTextBoxName, true));
}

void LayoutByNW4R::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    textdrawer::Base::create(allocator);
    mpString =
        new (MEMAllocFromAllocator(allocator, sizeof(tistring::Decolated)))
            tistring::Decolated(0x400);
    mpString->create(allocator);
    tistring::WithAtok *stringWithAtok = static_cast<tistring::WithAtok *>(
        MEMAllocFromAllocator(allocator, sizeof(tistring::WithAtok)));
    new (stringWithAtok) tistring::WithAtok(0x400);
    mpUnfixString = stringWithAtok;
    mpUnfixString->create(allocator);
    tistring::WithZi *stringWithZi = static_cast<tistring::WithZi *>(
        MEMAllocFromAllocator(allocator, sizeof(tistring::WithZi)));
    new (stringWithZi) tistring::WithZi(0x400);
    mpZiString = stringWithZi;
    mpZiString->create(allocator);
    mriManager.create(allocator);
    Info_ *line = mriManager.newLine();
    line->StrCount = 0;
    line->DispRowCount = 1;
    mpCursorLine = line;
    mpEventHandler =
        new (MEMAllocFromAllocator(allocator, sizeof(EventHandler)))
            EventHandler(this);
    nw4rmanager::Layout::createWithEventHandler(allocator, mpEventHandler);
    if (mpszFontName == NULL) {
        mpszFontName = "WiiBitmapFontType1.brfnt";
        mResFont.SetResource(
            mpMultiArcResourceAccessor->GetResource(0, mpszFontName, NULL));
        mResFont.SetAlternateChar(0xE06B);
        setFont(mResFont);
    } else {
        mpMultiArcResourceAccessor->GetFont(mpszFontName)
            ->SetAlternateChar(0xE06B);
        setFont(*mpMultiArcResourceAccessor->GetFont(mpszFontName));
    }
    nw4r::lyt::TextBox &textBox = getTextBox_();
    textBox.SetString(L"", 0);
    mfCharSpace = textBox.GetCharSpace();
    mfLineSpace = textBox.GetLineSpace();
    mfFontWidth = textBox.GetFontSize().width;
    mfFontHeight = textBox.GetFontSize().height;
    mRect = textBox.GetPaneRect(mDrawInfo);
    AdjustPaneMtx(mMtx.mtx, mDrawInfo, textBox.GetGlobalMtx());
    sTextBoxColor = textBox.GetTextColor(0);
    setVisible("N_2line", true);
    mpPaneManager->setAllComponentTriggerTarget(false);
    mpPaneManager->setAllBoundingBoxComponentTriggerTarget(true);
    mpPaneManager
        ->getPaneComponentByPane(
            mpLayout->GetRootPane()->FindPaneByName("T_2l_TextBox", true))
        ->setTriggerTarget(true);
    const char *forceAddName;
    for (u16 i = 0; i < 2; i++) {
        const PaneToAnimation &info = csPaneToAnimation[i];
        NormalButtonAnmPane *anmPane = NULL;
        switch (info.type) {
        case 0:
            anmPane = new (
                MEMAllocFromAllocator(allocator, sizeof(NormalButtonAnmPane)))
                NormalButtonAnmPane(getPane(info.paneName));
            break;
        }
        nw4r::ut::List_Append(&mAnmPanes, anmPane);
        forceAddName = info.forceAddName;
        for (u16 j = 0; j < info.count; j++) {
            void *res = mpMultiArcResourceAccessor->GetResource(
                0, info.pAnims[j]->fileName, NULL);
            AnimTransformPane *anim = reinterpret_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(res,
                                                 mpMultiArcResourceAccessor));
            if (forceAddName == NULL) {
                anmPane->addAnimation(allocator, info.pAnims[j]->id, anim,
                                      false, true);
            } else {
                anmPane->forceAddAnimation(allocator, info.pAnims[j]->id, anim,
                                           forceAddName, false, true);
            }
        }
    }
    init();
}

void LayoutByNW4R::init() {
    Base::init();
    searchAnmPane("P_txtScrll_UP")->changeAnimation(7);
    searchAnmPane("P_txtScrll_DOWN")->changeAnimation(7);
    mbAbleToUp = false;
    mbAbleToDown = false;
    muRepeatCountUp = 0;
    muRepeatCountDown = 0;
    muRepeatCountLeft = 0;
    muRepeatCountRight = 0;
    muRepeatTrig = 0;
    mCharColor = sTextBoxColor;
    visibleSeparator(false);
    nw4r::lyt::TextBox *textBox = static_cast<nw4r::lyt::TextBox *>(
        mpLayout->GetRootPane()->FindPaneByName(mpszTextBoxName, true));
    textBox->SetString(L"", 0);
    mfCharSpace = textBox->GetCharSpace();
    mfLineSpace = textBox->GetLineSpace();
    mfFontWidth = textBox->GetFontSize().width;
    mfFontHeight = textBox->GetFontSize().height;
    mRect = textBox->GetPaneRect(mDrawInfo);
    AdjustPaneMtx(mMtx.mtx, mDrawInfo, textBox->GetGlobalMtx());
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
}

void LayoutByNW4R::draw() {
    nw4rmanager::Layout::draw();
    muGlobalAlpha = getLayout()->GetRootPane()->GetAlpha();
    AdjustPaneMtx(mMtx.mtx, mDrawInfo,
                  mpLayout->GetRootPane()
                      ->FindPaneByName(mpszTextBoxName, true)
                      ->GetGlobalMtx());
    Base::draw();
}

void LayoutByNW4R::calc() {
    Base::calc();
    nw4rmanager::Layout::calc();
    bool ableToUp = false;
    bool ableToDown = false;
    if (getPane("N_separateBarAll") != NULL) {
        setVisible("N_separateBarAll", mbVisibleSeparator);
    }
    if (mScrollAnm.isActive()) {
        return;
    }
    if (0.0f > mfScrollY) {
        ableToUp = true;
    }
    if (mfTextHeight < mfScrollY) {
        ableToDown = true;
    }
    if (mbAbleToUp != ableToUp) {
        mbAbleToUp = ableToUp;
        if (ableToUp) {
            searchAnmPane("P_txtScrll_UP")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_6);
            mpPaneManager->searchPaneComponent("P_txtScrll_UP")->init();
        } else {
            searchAnmPane("P_txtScrll_UP")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_7);
        }
    }
    if (mbAbleToDown != ableToDown) {
        mbAbleToDown = ableToDown;
        if (ableToDown) {
            searchAnmPane("P_txtScrll_DOWN")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_6);
            mpPaneManager->searchPaneComponent("P_txtScrll_DOWN")->init();
        } else {
            searchAnmPane("P_txtScrll_DOWN")
                ->onAnmEvent(nw4rmanager::AnmPane::PE_7);
        }
    }
}

void LayoutByNW4R::updateInputCommon(int chan, u32 trig, u32 hold, u32 release,
                                     void *data) {
    updateRepeatInput(trig, hold);
    if (muRepeatTrig & 0x1000) {
        onCommand((INPUT_COMMAND)1, NULL);
    }
}

bool LayoutByNW4R::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                               u32 release, void *data) {
    u32 repeat = muRepeatTrig;
    if (repeat & 1) {
        onCommand((INPUT_COMMAND)8, NULL);
    }
    if (repeat & 2) {
        onCommand((INPUT_COMMAND)9, NULL);
    }
    if (repeat & 8) {
        onCommand((INPUT_COMMAND)10, NULL);
    }
    if (repeat & 4) {
        onCommand((INPUT_COMMAND)11, NULL);
    }
    bool ret =
        nw4rmanager::Layout::updateInput(chan, x, y, trig, hold, release, data);
    if ((release & 0x800) && mpString->isOnSustain()) {
        onCommand((INPUT_COMMAND)13, NULL);
    }
    if (!ret && (hold & 0x800) && mpString->isOnSustain()) {
        nw4r::math::VEC2 pos(x, y);
        if (-y < (mRect.top - mRect.bottom) / 2.0f + getGlobalLeftTopPos().y) {
            pos.y = getGlobalLeftTopPos().y - 1.0f;
        } else {
            pos.y =
                1.0f + ((mRect.top - mRect.bottom) + getGlobalLeftTopPos().y);
        }
        onCommand((INPUT_COMMAND)16, &pos);
    }
    return ret;
}

bool LayoutByNW4R::updateInput(input::HKBManager &hkbManager) {
    return nw4rmanager::Layout::updateInput(hkbManager);
}

void LayoutByNW4R::updateRepeatInput(u32 trig, u32 hold) {
    muRepeatTrig = trig;
    if (hold & 1) {
        if (--muRepeatCountLeft == 0) {
            muRepeatTrig |= 1;
            muRepeatCountLeft = 9;
        }
    } else {
        muRepeatCountLeft = 30;
    }
    if (hold & 2) {
        if (--muRepeatCountRight == 0) {
            muRepeatTrig |= 2;
            muRepeatCountRight = 9;
        }
    } else {
        muRepeatCountRight = 30;
    }
    if (hold & 8) {
        if (--muRepeatCountUp == 0) {
            muRepeatTrig |= 8;
            muRepeatCountUp = 9;
        }
    } else {
        muRepeatCountUp = 30;
    }
    if (hold & 4) {
        if (--muRepeatCountDown == 0) {
            muRepeatTrig |= 4;
            muRepeatCountDown = 9;
        }
    } else {
        muRepeatCountDown = 30;
    }
    if (hold & 0x1000) {
        if (--muRepeatCountMinus == 0) {
            muRepeatTrig |= 0x1000;
            muRepeatCountMinus = 9;
        }
    } else {
        muRepeatCountMinus = 30;
    }
}

nw4r::math::VEC2 LayoutByNW4R::getScale() {
    return mpLayout->GetRootPane()
        ->FindPaneByName(mpszTextBoxName, true)
        ->GetScale();
}

void LayoutByNW4R::onCommand(INPUT_COMMAND command, void *data) {
    mpEventObserver->onInput(command, data);
    mpEventObserver->onCommand(command, data);
    Base::onCommand(command, data);
    if (command == 31) {
        return;
    }
    if (muLimitStringLength < (u16)mpString->getLength()) {
        u32 cursor, anchor;
        mpString->getCursorPos(&cursor, &anchor);
        if (cursor > muLimitStringLength || anchor > muLimitStringLength) {
            Base::onCommand((INPUT_COMMAND)36, &muLimitStringLength);
        }
    }
    switch (command) {
    case 32:
    case 33:
    case 35:
        break;
    default: {
        u32 limit = muLimitStringLength;
        if (limit < (u16)mpString->getLength()) {
            setStringLength_(limit);
            mpEventObserver->onOutOfLength();
            if (command == 6 || command == 5 || command == 0 || command == 38 ||
                command == 21 || command == 7) {
                onSE((sound::SE)8);
            }
        }
        break;
    }
    }
    u32 cursor = isOverRowLimit(muLimitRowNum, mpString->getWCString());
    u32 over = cursor;
    if (over != 0) {
        setStringLength_(over);
        mpEventObserver->onOutOfLength();
        onSE((sound::SE)8);
        Base::onCommand((INPUT_COMMAND)36, &cursor);
    }
}

u32 Base::isOverRowLimit(u32 limitRowNum, const wchar_t *string) {
    const wchar_t *p = string;
    if (*string == L'\0') {
        return 0;
    }
    f32 x = getScale().x;
    nw4r::math::VEC2 scale = getScale();
    u32 rows;
    u32 pos = 0;
    rows = 0;
    muWordWrapCounter = 0;
    sbWordHead = false;
    while (*p != L'\0') {
        DrawInfo drawInfo;
        drawInfo.wc = *p;
        calcRect(drawInfo);
        f32 charRight = x + scale.x * drawInfo.rect.GetWidth();
        if (doWordWrap(string, pos, x)) {
            rows++;
            if (rows >= limitRowNum) {
                for (; pos < muWordWrapCounter; pos++) {
                    f32 width = scale.x * drawInfo.rect.GetWidth();
                    f32 right = x + width;
                    if (right >= scale.x * mRect.GetWidth()) {
                        return pos;
                    }
                    x += width;
                    drawInfo.wc = *++p;
                    calcRect(drawInfo);
                }
                return muWordWrapCounter - 1;
            }
            x = getScale().x;
        } else if (charRight >= scale.x * mRect.GetWidth()) {
            x = getScale().x;
            rows++;
            if (rows >= limitRowNum) {
                return pos;
            }
        }
        if (*p == L'\n') {
            x = getScale().x;
            rows++;
            if (rows >= limitRowNum) {
                return pos;
            }
            if (p[1] == L'\0') {
                return 0;
            }
        } else {
            x += scale.x * drawInfo.rect.GetWidth();
        }
        pos++;
        p++;
    }
    return 0;
}

void LayoutByNW4R::onSE(sound::SE seId) {
    mpEventObserver->onSE(seId);
}

void LayoutByNW4R::setRootPaneScaleFor16x9() {
    nw4rmanager::Layout::setRootPaneScaleFor16x9();
    textdrawer::Base::setAspectRatio(false);
}

void LayoutByNW4R::setRootPaneScaleFor4x3() {
    nw4rmanager::Layout::setRootPaneScaleFor4x3();
    textdrawer::Base::setAspectRatio(true);
}

void EventHandler::onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                             Input *input) {
    const char *paneName = paneComponent->getPane()->GetName();
    nw4r::math::VEC2 pos;
    pos.x = input->x;
    pos.y = -input->y;
    if (paneName[0] == 'B') {
        char anmPaneName[17];
        util::replaceChar(anmPaneName, sizeof(anmPaneName), paneName, 0, 'P');
        if (event == gui::EventHandler::ON_TRIG && (input->trig & 0x800) &&
            !mpLayout->isInScroll()) {
            if (mpLayout->isAbleToUp() &&
                util::strcmp("P_txtScrll_UP", anmPaneName)) {
                CommandReceiver::Scroll scroll = {false, 0.0f, 0.0f};
                scroll.y = mpLayout->getLineHeight();
                mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)24,
                                    &scroll);
                mpLayout->searchAnmPane(anmPaneName)
                    ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            } else if (mpLayout->isAbleToDown() &&
                       util::strcmp("P_txtScrll_DOWN", anmPaneName)) {
                CommandReceiver::Scroll scroll = {false, 0.0f, 0.0f};
                scroll.y = -mpLayout->getLineHeight();
                mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)24,
                                    &scroll);
                mpLayout->searchAnmPane(anmPaneName)
                    ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
            }
        }
        if (event == gui::EventHandler::ON_MOVE && (input->hold & 0x800) &&
            !(input->trig & 0x800)) {
            if (mpLayout->isAbleToUp() &&
                util::strcmp("P_txtScrll_UP", anmPaneName)) {
                if (paneComponent->isDragging(input->ctrlNo)) {
                    u32 duration =
                        mpLayout->getFlightDuration(input->ctrlNo, paneName);
                    if (duration >= 60 && duration % 20 == 0) {
                        CommandReceiver::Scroll scroll = {false, 0.0f, 0.0f};
                        scroll.y = mpLayout->getLineHeight();
                        mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)24,
                                            &scroll);
                        mpLayout->searchAnmPane(anmPaneName)
                            ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
                    }
                }
            } else if (mpLayout->isAbleToDown() &&
                       util::strcmp("P_txtScrll_DOWN", anmPaneName)) {
                if (paneComponent->isDragging(input->ctrlNo)) {
                    u32 duration =
                        mpLayout->getFlightDuration(input->ctrlNo, paneName);
                    if (duration >= 60 && duration % 20 == 0) {
                        CommandReceiver::Scroll scroll = {false, 0.0f, 0.0f};
                        scroll.y = -mpLayout->getLineHeight();
                        mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)24,
                                            &scroll);
                        mpLayout->searchAnmPane(anmPaneName)
                            ->onAnmEvent(nw4rmanager::AnmPane::PE_0);
                    }
                }
            }
        }
        AnmPane *anmPane =
            static_cast<AnmPane *>(mpLayout->searchAnmPane(anmPaneName));
        if (anmPane == NULL) {
            return;
        }
        switch (event) {
        case gui::EventHandler::ON_LEFT:
            anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
            break;
        case gui::EventHandler::ON_POINT:
            if (anmPane->getState() != 7) {
                mpEventObserver->onSE((sound::SE)4);
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_1);
            }
            break;
        }
    } else if (paneName[0] == 'P') {
        AnmPane *anmPane =
            static_cast<AnmPane *>(mpLayout->searchAnmPane(paneName));
        if (anmPane == NULL) {
            return;
        }
        switch (event) {
        case gui::EventHandler::ON_POINT:
            if (anmPane->getState() != 7) {
                mpEventObserver->onSE((sound::SE)4);
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_1);
            }
            break;
        case gui::EventHandler::ON_LEFT:
            if (anmPane->getState() != 7) {
                mpEventObserver->onSE((sound::SE)4);
                anmPane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
            }
            break;
        }
    } else if (util::strcmp(paneName, "T_2l_TextBox")) {
        if (event == gui::EventHandler::ON_TRIG && (input->trig & 0x800)) {
            mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)14, &pos);
        }
        if (event == gui::EventHandler::ON_RELEASE &&
            (input->release & 0x800)) {
            mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)15, &pos);
        }
        if (event == gui::EventHandler::ON_MOVE && (input->hold & 0x800)) {
            mpLayout->onCommand((CommandReceiver::INPUT_COMMAND)16, &pos);
        }
    }
}

void NormalButtonAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_6 && meState == 7) {
        changeAnimation(5);
    } else if (paneEvent == PE_7 && meState != 7) {
        changeAnimation(6);
    } else if (paneEvent == PE_0) {
        changeAnimation(4);
    }
    switch (meState) {
    case 5:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
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
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(1);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(2);
        }
        break;
    case 6:
        if (paneEvent == PE_4) {
            changeAnimation(7);
        }
        break;
    }
}

} // namespace inputform
} // namespace textinput
