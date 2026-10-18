// d_kbd.cpp (.text 800C7BE4..800CB0CC, .rodata 80472E40..80472E50, .data
// 804EAA10..804EAFF8, .bss 8059FFA8..805A02C0, .sdata 8074A9D0..8074AA58, .sbss
// 8074E498..8074E4B0, .sdata2 80750840..80750860). See
// include/game/game/d_kbd.hpp and notes/TUs/d_kbd.txt.
#include <game/game/d_kbd.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_thunder.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_perf.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/revolution/OS.h>
#include <revolution/GX.h>
#include <revolution/SC.h>
#include <revolution/WPAD.h>
#include <string.h>

namespace dKbd {

#define THREAD_STACK_SIZE 0x2800

// The thread that creates and sets up the manager.
OSThread sThread;

// The keyboard settings between uses (read back from the manager when the
// keyboard is removed).
textinput::extend::memo::Setting sSetting;
u8 sSavedByte;
u8 sSavedFlag;
// The stack of the thread (volatile: the thread is checked from the main loop).
void *volatile sThreadStack;
// Whether the symbol window was open at the last calc.
BOOL sSignWindowOpen;
// The keypad character being picked, as a string.
wchar_t sCandidateText[2];

static void createThread(OSThreadFunc func, void *arg) {
    void *stack = m2d::ResAccLoader_c::getAllocator()->mpHeap->alloc(
        THREAD_STACK_SIZE, 0x20);
    sThreadStack = stack;
    OSCreateThread(&sThread, func, arg, (u8 *)stack + THREAD_STACK_SIZE,
                   THREAD_STACK_SIZE, 28, 1);
    mPerf::setThreadColor(&sThread, dPerf::getThreadColor(11));
    OSResumeThread(&sThread);
}

static BOOL isThreadDone() {
    if (OSIsThreadTerminated(&sThread)) {
        if (sThreadStack != NULL) {
            m2d::ResAccLoader_c::getAllocator()->mpHeap->free(sThreadStack);
            sThreadStack = NULL;
            mPerf::resetThreadColor(&sThread);
        }
        return TRUE;
    }
    return FALSE;
}

static void *createManagerThread(void *arg) {
    static_cast<myManager_c *>(arg)->create(NULL);
    return NULL;
}

static void *setupThread(void *arg) {
    static_cast<Mng_c *>(arg)->setup(Mng_c::SETUP_MODE_0);
    return NULL;
}

static int getConsoleRegion() {
    return getRegion();
}

static int getConsoleLanguage() {
    return getLanguage();
}

void myManager_c::draw() {
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    mpToolBar->draw();
    mpInputForm->draw();
    if (mpToolBar->isQwerty()) {
        if (mpPCKeyboard->getLayout()->GetRootPane()->IsVisible()) {
            mpPCKeyboard->draw();
        }
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

ResLoader_c::ResLoader_c() : mpArc(NULL), mHeapCreated(false) {
}

ResLoader_c::~ResLoader_c() {
}

BOOL ResLoader_c::load() {
    EGG::Heap *heap = mAllocator.mpHeap;
    if (mpArc != NULL) {
        return TRUE;
    }
    mpArc = mLoader.request("Layout/sofkeybd.arc", 0, heap);
    if (mpArc == NULL) {
        return FALSE;
    }
    mArcLink.Set(mpArc, "arc");
    mResAcc.Attach(&mArcLink);
    if (mFontLink.GetFont() != &dFont::g_mainFont.mFont) {
        mFontLink.Set("WiiBitmapFontType1.brfnt", &dFont::g_mainFont.mFont);
    }
    mResAcc.RegistFont(&mFontLink);
    return TRUE;
}

BOOL ResLoader_c::remove() {
    if (mpArc != NULL) {
        mResAcc.Detach(&mArcLink);
        mAllocator.free(mpArc);
        mResAcc.UnregistFont(&mFontLink);
        mpArc = NULL;
    }
    if (!mLoader.release()) {
        return FALSE;
    }
    if (mHeapCreated) {
        mHeapCreated = false;
        mAllocator.destroyHeap();
    }
    return TRUE;
}

BOOL ResLoader_c::createHeap(EGG::Heap *parent, size_t size) {
    BOOL created = mAllocator.createExpHeap(size, parent, NULL, 0x20);
    if (created) {
        mHeapCreated = true;
    }
    return created;
}

Event_c::Event_c() : mEvents(0) {
}

Event_c::~Event_c() {
}

void Event_c::onOK() {
    mEvents |= EVENT_OK;
}

void Event_c::onCancel() {
    mEvents |= EVENT_CANCEL;
}

void Event_c::onSE(textinput::sound::SE seId) {
    switch (seId) {
    case textinput::sound::SE_SELECT:
        dMenuBase_c::playSeCursor();
        break;
    case textinput::sound::SE_CHAR_DECIDE:
        if (sSignWindowOpen) {
            playSe(SE_cancel_little);
        } else {
            mEvents |= EVENT_DECIDE;
        }
        break;
    case textinput::sound::SE_CHAR_INPUT:
        mEvents |= EVENT_CHAR_INPUT;
        break;
    case textinput::sound::SE_15:
        playSe(SE_gamen_okuri);
        break;
    case textinput::sound::SE_SYMBOL_PAGE_OPEN:
        playSe(SE_keyboard_InputModeChange);
        break;
    case textinput::sound::SE_CHAR_DELETE:
        playSe(SE_keyboard_BackSpace);
        mEvents |= EVENT_CHAR_DELETE;
        break;
    case textinput::sound::SE_KETAI_MODE_SWITCHING:
        playSe(SE_keyboard_InputModeChange);
        break;
    case textinput::sound::SE_CHAR_DELETE_ERROR:
        playSeCanNotInput();
        mEvents |= EVENT_CHAR_DELETE;
        break;
    case textinput::sound::SE_CHAR_CURSOR:
        mEvents |= EVENT_CURSOR_FIX | EVENT_CURSOR;
        break;
    case textinput::sound::SE_CHAR_CURSOR_FIX:
        mEvents |= EVENT_CURSOR_FIX;
        break;
    case textinput::sound::SE_18:
        playSe(SE_keyboard_InputModeChange);
        break;
    }
}

void Listener_c::vt_0x0C() {
}

void Listener_c::vt_0x10() {
}

void Listener_c::onKey(textinput::input::KeyEvent *event) {
    mRequest = REQUEST_NONE;
    mSpecialKey = 0;
    if (!(event->mMode & 1)) {
        switch (event->mCode) {
        case KEY_REPEAT:
            mHoldTimer = KEY_HOLD_FRAMES;
            break;
        }
    }
    if (!(event->mMode & 1)) {
        return;
    }
    mHoldTimer = KEY_HOLD_FRAMES;
    int code = event->mCode;
    switch (code) {
    case KEY_CLEAR:
        mRequest = REQUEST_CLEAR;
        break;
    case KEY_SEND:
    case KEY_SEND_2:
        mRequest = REQUEST_SEND;
        break;
    case KEY_EMOTION_0:
    case KEY_EMOTION_0 + 1:
    case KEY_EMOTION_0 + 2:
    case KEY_EMOTION_0 + 3:
        mRequest = code - KEY_EMOTION_0 + REQUEST_EMOTION_0;
        mHoldTimer = 0;
        break;
    case KEY_PAD_0:
    case KEY_PAD_0 + 1:
    case KEY_PAD_0 + 2:
    case KEY_PAD_0 + 3:
    case KEY_PAD_0 + 4:
    case KEY_PAD_0 + 5:
    case KEY_PAD_0 + 6:
    case KEY_PAD_0 + 7:
    case KEY_PAD_0 + 8:
    case KEY_PAD_0 + 9:
        mDigit = code - KEY_PAD_0;
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
        mDigit = code - L'0';
        break;
    case KEY_DIGIT_10:
        mDigit = DIGIT_10;
        break;
    case KEY_DIGIT_11:
        mDigit = DIGIT_11;
        break;
    case KEY_SPECIAL_1:
        mSpecialKey = 1;
        break;
    case KEY_SPECIAL_2:
        mSpecialKey = 2;
        break;
    case KEY_SPECIAL_3:
        mSpecialKey = 3;
        break;
    case KEY_SPECIAL_4:
        mSpecialKey = 4;
        break;
    case KEY_IGNORED_0:
    case KEY_IGNORED_1:
    case KEY_IGNORED_2:
        break;
    }
}

BOOL Listener_c::isKeyHeld() {
    return mHoldTimer != 0;
}

void Listener_c::calc() {
    if (mHoldTimer != 0) {
        mHoldTimer--;
    } else {
        mRequest = REQUEST_NONE;
        mSpecialKey = 0;
    }
}

void Listener_c::clear() {
    mHoldTimer = 0;
    mRequest = REQUEST_NONE;
    mSpecialKey = 0;
    mDigit = DIGIT_NONE;
}

void Listener_c::calcClear() {
    if (mHoldTimer != 0) {
        mHoldTimer--;
    } else {
        clear();
    }
}

int Listener_c::getDigit() {
    u8 digit = mDigit;
    if (digit <= 9) {
        return digit;
    }
    if (digit == DIGIT_10) {
        return DIGIT_10;
    }
    if (digit == DIGIT_11) {
        return DIGIT_11;
    }
    return DIGIT_RESULT_NONE;
}

u8 Listener_c::getSpecialKey() {
    return mSpecialKey;
}

int Listener_c::getEmotion() {
    u8 request = mRequest;
    if (request >= REQUEST_EMOTION_0 && request <= REQUEST_EMOTION_0 + 3) {
        return request - REQUEST_EMOTION_0;
    }
    return -1;
}

// Drops the PC keyboard's input mode.
static void
resetInputMode(textinput::keyboard::pctype::LayoutByNW4R *keyboard) {
    keyboard->mKeyState.setInputMode(0);
    keyboard->vt_0x144();
}

static void
refreshKeyboard(textinput::keyboard::pctype::LayoutByNW4R *keyboard) {
    keyboard->vt_0x13C();
}

static void setStringCursor(textinput::inputform::LayoutByNW4R *form, u32 pos) {
    form->mpString->initKanaConverter();
    form->setCursorPos(form->mpString, pos);
}

static u32 getStringCursor(textinput::inputform::LayoutByNW4R *form) {
    return form->mpString->getCursorPos();
}

static wchar_t *getKanaBuffer(textinput::inputform::LayoutByNW4R *form) {
    return form->mpString->getKanaBuffer();
}

static BOOL confirmKana(textinput::inputform::LayoutByNW4R *form) {
    if (form->mpString->isKanaFix()) {
        return FALSE;
    }
    form->mpString->clearKana();
    return TRUE;
}

static textinput::keyboard::cellphonetype::Candidate *
getCandidate(textinput::keyboard::cellphonetype::LayoutByNW4R *keypad) {
    return &keypad->mCandidate;
}

// Inputs the keypad character being picked.
static BOOL
confirmCandidate(textinput::keyboard::cellphonetype::LayoutByNW4R *keypad) {
    if (keypad->mCandidate.mpKey != NULL) {
        wchar_t c = keypad->mCandidate.mpKey->mChars[keypad->mCandidate.mIndex];
        textinput::keyboard::cellphonetype::KeyCommand command = {
            c, 0x100, keypad->mCandidate.mpKey};
        keypad->sendCommand(textinput::CommandReceiver::INPUT_COMMAND_KEY,
                            &command);
        keypad->mCandidate.mIndex = 0;
        keypad->mCandidate.mpKey = NULL;
        return TRUE;
    }
    return FALSE;
}

static void
closeSignWindow(textinput::keyboard::signtype::LayoutByNW4R *window) {
    if (window->isActive()) {
        window->close();
    }
}

Mng_c::Mng_c() : mpManager(NULL), mFlags(0), mCreateStep(CREATE_STEP_START) {
}

Mng_c::~Mng_c() {
}

BOOL Mng_c::init(int mode) {
    if (mFlags & FLAG_INIT) {
        return TRUE;
    }
    EGG::Heap *heap = m2d::ResAccLoader_c::getAllocator()->mpHeap;
    if (!mResLoader.createHeap(heap, 0x151800)) {
        return FALSE;
    }
    mAllocator.createExpHeap(0x57800, heap, NULL, 8);
    mFlags |= FLAG_INIT;
    if (mode != INIT_MODE_PAD || !dSaveOption::getKeySe()) {
        mFlags &= ~FLAG_CELL_PHONE;
        sSetting.mKeyboardType = 0x11;
    } else {
        mFlags |= FLAG_CELL_PHONE;
        sSetting.mKeyboardType = 0x10;
    }
    mCreateStep = CREATE_STEP_START;
    return TRUE;
}

void Mng_c::setup(int mode) {
    switch (mode) {
    case SETUP_MODE_0:
        mpManager->vt_0x118();
        break;
    case SETUP_MODE_1:
        mpManager->vt_0xE8();
        break;
    }
    textinput::extend::memo::Setting setting = sSetting;
    mpManager->setSetting(setting);
    mpManager->vt_0xE4();
    mpManager->getCellPhoneKeyboard()->vt_0x10C(false);
    mpManager->getPCKeyboard()->vt_0x124(false);
    mpManager->getCandidateBox()->vt_0xF8(false);
}

BOOL Mng_c::loadRes() {
    BOOL loaded = mResLoader.load();
    return loaded & mResAcc.load("Layout/keyboard_add.arc");
}

void Mng_c::createManager() {
    EGG::Heap *heap = mHeap::setCurrentHeap(mAllocator.mpHeap);
    mpManager = new myManager_c(&mAllocator, &mResLoader.mResAcc, &mEvent);
    mHeap::setCurrentHeap(heap);
}

void Mng_c::initManager() {
    mListener.clear();
    mpManager->getKBDListenerRoot()->append(&mListener);
    switch (getConsoleRegion()) {
    case CONSOLE_REGION_JP:
        mpManager->setDestination(textinput::DST_JP);
        break;
    case CONSOLE_REGION_NA:
        mpManager->setDestination(textinput::DST_US);
        break;
    case CONSOLE_REGION_EU:
        mpManager->setDestination(textinput::DST_EU);
        break;
    case CONSOLE_REGION_KR:
        mpManager->setDestination(textinput::DST_US);
        break;
    }
    switch (getConsoleLanguage()) {
    case LANGUAGE_JP:
        mpManager->setLanguage(textinput::JP);
        break;
    case LANGUAGE_US:
        mpManager->setLanguage(textinput::USA);
        break;
    case LANGUAGE_MX:
        mpManager->setLanguage(textinput::SP);
        break;
    case LANGUAGE_QC:
        mpManager->setLanguage(textinput::FR);
        break;
    case LANGUAGE_EN:
        mpManager->setLanguage(textinput::UK);
        break;
    case LANGUAGE_ES:
        mpManager->setLanguage(textinput::SP);
        break;
    case LANGUAGE_FR:
        mpManager->setLanguage(textinput::FR);
        break;
    case LANGUAGE_IT:
        mpManager->setLanguage(textinput::IT);
        break;
    case LANGUAGE_DE:
        mpManager->setLanguage(textinput::DE);
        break;
    case LANGUAGE_KR:
        mpManager->setLanguage(textinput::USA);
        break;
    }
    mpManager->setAspectRatio(SCGetAspectRatio() != SC_ASPECT_WIDE);
    textinput::extend::memo::Setting setting = sSetting;
    mpManager->setSetting(setting);
    mpManager->vt_0xE4();
    mpManager->vt_0xD0(true);
    setupLayout();
}

void Mng_c::createDirect() {
    createManager();
    mpManager->create(NULL);
    initManager();
}

BOOL Mng_c::create() {
    if (mCreateStep >= CREATE_STEP_DONE) {
        return TRUE;
    }
    if (mCreateStep == CREATE_STEP_START) {
        createManager();
        createThread(createManagerThread, mpManager);
        mCreateStep = CREATE_STEP_MANAGER;
        return FALSE;
    }
    if (isThreadDone()) {
        if (mCreateStep == CREATE_STEP_MANAGER) {
            initManager();
            createThread(setupThread, this);
            mCreateStep = CREATE_STEP_SETUP;
        } else if (mCreateStep == CREATE_STEP_SETUP) {
            mCreateStep = CREATE_STEP_DONE;
        }
    }
    return FALSE;
}

void Mng_c::buildLayout() {
    const char *lytName;
    const char *anmName;
    if (mFlags & FLAG_CELL_PHONE) {
        lytName = "KB_base_prtbl_00.brlyt";
        anmName = "KB_base_prtbl_00_scrlIcon_00.brlan";
    } else {
        lytName = "KB_base_00.brlyt";
        anmName = "KB_base_00_scrlIcon_00.brlan";
    }
    mLyt.build(&mResAcc, lytName);
    if (anmName != NULL) {
        mLyt.setAnm(&mAnm, anmName, NULL);
    }
    mFlags |= FLAG_LAYOUT;
}

BOOL Mng_c::remove(int mode) {
    if (mpManager != NULL) {
        if (mCreateStep == CREATE_STEP_MANAGER ||
            mCreateStep == CREATE_STEP_SETUP) {
            if (isThreadDone()) {
                if (mCreateStep == CREATE_STEP_MANAGER) {
                    initManager();
                }
                mCreateStep = CREATE_STEP_DONE;
            } else {
                return FALSE;
            }
        }
        if (mode == REMOVE_MODE_SAVE_SETTING) {
            saveSetting();
        } else {
            saveInputMode();
        }
        mpManager->getKBDListenerRoot()->remove(&mListener);
        EGG::Heap *heap = mHeap::setCurrentHeap(mAllocator.mpHeap);
        delete mpManager;
        mHeap::setCurrentHeap(heap);
        mpManager = NULL;
        mAllocator.destroyHeap();
    }
    if (!mResLoader.remove()) {
        return FALSE;
    }
    if (!mResAcc.remove()) {
        return FALSE;
    }
    mFlags = 0;
    mCreateStep = CREATE_STEP_START;
    return TRUE;
}

myManager_c::~myManager_c() {
}

void Mng_c::calc() {
    if (!(mFlags & FLAG_OPEN)) {
        mpManager->calc();
        updateNoInput();
    }
    mFlags &= ~(FLAG_OPEN | FLAG_CHANGED);
    mListener.calc();
    mEvent.mEvents = 0;
    saveCursorPos();
    if (mFlags & FLAG_LAYOUT) {
        mAnm.play();
        mLyt.calc();
    }
    sSignWindowOpen = isSignWindowOpen();
}

void Mng_c::calcClear() {
    mListener.calcClear();
    mEvent.mEvents = 0;
}

// The pointer stays off the keyboard; the direction buttons come from the
// remote or the pad.
void Mng_c::updatePointerOff() {
    mVec2_c pos(1000.0f, 1000.0f);
    u32 trig =
        mPad::g_currentCore->mCoreStatus[0].trig & dPad::BUTTON_DIRECTION;
    u32 hold =
        mPad::g_currentCore->mCoreStatus[0].hold & dPad::BUTTON_DIRECTION;
    u32 release =
        mPad::g_currentCore->mCoreStatus[0].release & dPad::BUTTON_DIRECTION;
    if (trig == 0 && hold == 0 && release == 0) {
        trig = dPad::getTriggered() & dPad::BUTTON_DIRECTION;
        hold = dPad::getHeld() & dPad::BUTTON_DIRECTION;
        release = dPad::getReleased() & dPad::BUTTON_DIRECTION;
    }
    if (trig != 0) {
        mFlags |= FLAG_CHANGED;
    }
    mpManager->updateInput(0, pos.x, -pos.y, trig, hold, release);
}

// 80750848 / 80750850: the screen size by aspect ratio.
static const f32 l_screenWidth[] = {608.0f, 832.0f};
static const f32 l_screenHeight[] = {456.0f, 456.0f};

BOOL Mng_c::updatePointer() {
    mVec2_c pos;
    if (!mPad::g_currentCore->getDpdValidFlag() || !dObjCursor_c::isVisible()) {
        pos.x = 1000.0f;
        pos.y = 1000.0f;
    } else {
        u8 aspect = (u8)SCGetAspectRatio();
        pos.x = l_screenWidth[aspect] * dPad::getRawPointer().x / 2.0f;
        pos.y = l_screenHeight[aspect] * dPad::getRawPointer().y / 2.0f;
    }
    u32 trig = mPad::g_currentCore->mCoreStatus[0].trig;
    u32 hold = mPad::g_currentCore->mCoreStatus[0].hold;
    u32 release = mPad::g_currentCore->mCoreStatus[0].release;
    if ((trig & dPad::BUTTON_DIRECTION) == 0 &&
        (hold & dPad::BUTTON_DIRECTION) == 0 &&
        (release & dPad::BUTTON_DIRECTION) == 0) {
        trig |= dPad::getTriggered() & dPad::BUTTON_DIRECTION;
        hold |= dPad::getHeld() & dPad::BUTTON_DIRECTION;
        release |= dPad::getReleased() & dPad::BUTTON_DIRECTION;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
        mFlags |= FLAG_CHANGED;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_B) &&
        (mFlags & FLAG_CELL_PHONE)) {
        mFlags |= FLAG_CHANGED;
    }
    if (trig & dPad::BUTTON_DIRECTION) {
        mFlags |= FLAG_CHANGED;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS)) {
        trig &= ~WPAD_BUTTON_MINUS;
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_MINUS)) {
        hold &= ~WPAD_BUTTON_MINUS;
    }
    if (mPad::g_currentCore->upTrigger(WPAD_BUTTON_MINUS)) {
        release &= ~WPAD_BUTTON_MINUS;
    }
    if (!(mFlags & FLAG_CELL_PHONE)) {
        trig &= ~WPAD_BUTTON_B;
        hold &= ~WPAD_BUTTON_B;
        release &= ~WPAD_BUTTON_B;
    }
    mpManager->updateInput(0, pos.x, -pos.y, trig, hold, release);
    return TRUE;
}

BOOL Mng_c::updateKeyboard() {
    if (mFlags & FLAG_CELL_PHONE) {
        return FALSE;
    }
    if (!mListener.isKeyHeld()) {
        return FALSE;
    }
    mpManager->updateInput(*getHKBManager());
    mFlags |= FLAG_CHANGED;
    return TRUE;
}

void Mng_c::open(int noPointer, int noUpdate) {
    mpManager->calc();
    mFlags = (mFlags | FLAG_OPEN) & ~FLAG_CHANGED;
    updateKeyboard();
    if (noUpdate) {
        updateNoInput();
    } else if (noPointer) {
        updatePointerOff();
    } else {
        updatePointer();
    }
    if (isCharInput() || isCharDelete() || isDecide()) {
        mFlags |= FLAG_CHANGED;
    }
}

BOOL Mng_c::reopen() {
    mpManager->calc();
    mFlags = (mFlags | FLAG_OPEN) & ~FLAG_CHANGED;
    updateNoInput();
    if (isCharInput()) {
        updateInput();
        return TRUE;
    }
    return FALSE;
}

void Mng_c::updateNoInput() {
    mpManager->updateInput(0, 1000.0f, 1000.0f, 0, 0, 0);
}

BOOL Mng_c::isTextChanged() {
    if (isCursorMoved() && mCursorPos != getCursorPos()) {
        dMenuBase_c::playSeCursor();
    }
    if (mFlags & FLAG_CHANGED) {
        return TRUE;
    }
    if (mFlags & FLAG_CELL_PHONE) {
        textinput::keyboard::cellphonetype::Candidate *candidate =
            getCandidate(mpManager->getCellPhoneKeyboard());
        if (mFlags & FLAG_CANDIDATE) {
            if (candidate->mpKey == NULL) {
                mFlags &= ~FLAG_CANDIDATE;
                return TRUE;
            }
        } else if (candidate->mpKey != NULL) {
            mFlags |= FLAG_CANDIDATE;
        }
    }
    return isCursorFixed() != FALSE;
}

BOOL Mng_c::isSendReq() {
    return mListener.mRequest == Listener_c::REQUEST_SEND;
}

BOOL Mng_c::isClearReq() {
    return mListener.mRequest == Listener_c::REQUEST_CLEAR;
}

int Mng_c::getDigit() {
    return mListener.getDigit();
}

int Mng_c::getEmotion() {
    return mListener.getEmotion();
}

u8 Mng_c::getSpecialKey() {
    return mListener.getSpecialKey();
}

void Mng_c::clearEmotion() {
    mListener.clear();
}

void Mng_c::draw() {
    if (mFlags & FLAG_LAYOUT) {
        mLyt.draw();
    }
    mpManager->draw();
}

void Mng_c::postCalc() {
    mpManager->getInputForm()->draw();
}

bool Mng_c::isOK() {
    u16 event = mEvent.mEvents & Event_c::EVENT_OK;
    return event;
}

bool Mng_c::isCancel() {
    u16 event = mEvent.mEvents & Event_c::EVENT_CANCEL;
    return event;
}

bool Mng_c::isCharInput() {
    u16 event = mEvent.mEvents & Event_c::EVENT_CHAR_INPUT;
    return event;
}

bool Mng_c::isCharDelete() {
    u16 event = mEvent.mEvents & Event_c::EVENT_CHAR_DELETE;
    return event;
}

bool Mng_c::isDecide() {
    u16 event = mEvent.mEvents & Event_c::EVENT_DECIDE;
    return event;
}

bool Mng_c::isCursorFixed() {
    u16 event = mEvent.mEvents & Event_c::EVENT_CURSOR_FIX;
    return event;
}

bool Mng_c::isCursorMoved() {
    u16 event = mEvent.mEvents & Event_c::EVENT_CURSOR;
    return event;
}

void Mng_c::setMemoMode1() {
    mpManager->vt_0xD8(1);
}

void Mng_c::setMemoMode2() {
    mpManager->vt_0xD8(2);
}

BOOL Mng_c::isMemoEmpty() {
    return mpManager->getMemo()->vt_0x0C() == 0;
}

void Mng_c::setPos(mVec3_c pos) {
    if (mFlags & FLAG_LAYOUT) {
        mLyt.findPane("N_kb_base_all")->SetTranslate(pos);
        mLyt.calc();
    }
    pos.y -= 23.0f;
    if (mFlags & FLAG_CELL_PHONE) {
        textinput::keyboard::cellphonetype::LayoutByNW4R *keypad =
            mpManager->getCellPhoneKeyboard();
        keypad->getLayout()->GetRootPane()->SetTranslate(pos);
        keypad->refreshState();
    } else {
        textinput::keyboard::pctype::LayoutByNW4R *keyboard =
            mpManager->getPCKeyboard();
        keyboard->getLayout()->GetRootPane()->SetTranslate(pos);
        keyboard->refreshState();
    }
    mpManager->getSignKeyboard()->getLayout()->GetRootPane()->SetTranslate(pos);
}

const wchar_t *Mng_c::getText() {
    return mpManager->getWCString();
}

void Mng_c::setText(const wchar_t *str) {
    mpManager->setWCString(str);
}

void Mng_c::setMaxLength(int len) {
    mpManager->limitStringLength(len);
}

int Mng_c::getInputPos() {
    return getStringCursor(mpManager->getInputForm());
}

int Mng_c::getCursorPos() {
    int pos = getInputPos();
    return pos + getInputLength();
}

int Mng_c::getInputLength() {
    return m2d::Word_c::getLength(getInputText());
}

int Mng_c::getTextLength() {
    const wchar_t *text = getText();
    const wchar_t *input = getInputText();
    int textLen = m2d::Word_c::getLength(text);
    return textLen + m2d::Word_c::getLength(input);
}

void Mng_c::setCursorPos(int pos) {
    mCursorPos = pos;
    setStringCursor(mpManager->getInputForm(), pos);
    if (!(mFlags & FLAG_CELL_PHONE)) {
        resetInputMode(mpManager->getPCKeyboard());
    }
}

void Mng_c::saveCursorPos() {
    mCursorPos = getInputPos();
}

void Mng_c::resetCursor() {
    setCursorPos(mCursorPos);
    if (!(mFlags & FLAG_CELL_PHONE)) {
        textinput::keyboard::pctype::LayoutByNW4R *keyboard =
            mpManager->getPCKeyboard();
        keyboard->onActive();
        refreshKeyboard(keyboard);
    }
}

void Mng_c::setKeyboardMode(int mode) {
    if (mFlags & FLAG_CELL_PHONE) {
        mpManager->getCellPhoneKeyboard()->vt_0x108(mode);
        mpManager->getCellPhoneKeyboard()->refreshState();
    } else {
        mpManager->getPCKeyboard()->vt_0x120(mode);
        mpManager->getPCKeyboard()->refreshState();
    }
}

void Mng_c::hideToolBar() {
    mpManager->getToolBar()->getLayout()->GetRootPane()->SetVisible(false);
}

// 804EAB74: the keyboard change buttons of the tool bar.
static const char *l_keyboardChangeNames[] = {
    "N_keyboardChange",
    "B_kyChng_QWERTY",
    "B_kyChng_CP",
};

void Mng_c::setupLayout() {
    m2d::setVisible(mpManager->getToolBar()->getLayout()->GetRootPane(),
                    l_keyboardChangeNames, 3, false);
    nw4r::lyt::Layout *layout = mpManager->getInputForm()->getLayout();
    nw4r::lyt::Pane *root = layout->GetRootPane();
    mVec3_c pos = root->GetTranslate();
    pos.x += 1000.0f;
    pos.y += 1000.0f;
    root->SetTranslate(pos);
    nw4r::lyt::DrawInfo drawInfo;
    layout->CalculateMtx(drawInfo);
    root->SetVisible(false);
    mpManager->getPredictLanguageSelectDialog()
        ->getLayout()
        ->GetRootPane()
        ->SetVisible(false);
    mpManager->getCandidateBox()->getLayout()->GetRootPane()->SetVisible(false);
}

const wchar_t *Mng_c::getInputText() {
    if (mFlags & FLAG_CELL_PHONE) {
        textinput::keyboard::cellphonetype::Candidate *candidate =
            getCandidate(mpManager->getCellPhoneKeyboard());
        wchar_t c;
        if (candidate->mpKey != NULL) {
            c = candidate->mpKey->mChars[candidate->mIndex];
        } else {
            c = 0;
        }
        sCandidateText[0] = c;
        if (c == 0x309B || c == 0x309C) {
            sCandidateText[0] = 0;
        }
        if (sCandidateText[0] == L' ') {
            sCandidateText[0] = 0xE057;
        }
        if (sCandidateText[0] == 0x3000) {
            sCandidateText[0] = 0xE020;
        }
        sCandidateText[1] = 0;
        return sCandidateText;
    }
    return getKanaBuffer(mpManager->getInputForm());
}

BOOL Mng_c::fixInput() {
    if (getInputLength() == 0) {
        return FALSE;
    }
    if (mFlags & FLAG_CELL_PHONE) {
        return confirmCandidate(mpManager->getCellPhoneKeyboard());
    }
    return confirmKana(mpManager->getInputForm());
}

BOOL Mng_c::getComposedText(wchar_t *buf, int size) {
    int pos = getInputPos();
    const wchar_t *text = getText();
    const wchar_t *input = getInputText();
    int textLen = m2d::Word_c::getLength(text);
    int inputLen = m2d::Word_c::getLength(input);
    if (textLen + inputLen >= size) {
        return FALSE;
    }
    memset(buf, 0, size * sizeof(wchar_t));
    if (pos > 0) {
        memcpy(buf, text, pos * sizeof(wchar_t));
    }
    if (inputLen > 0) {
        memcpy(buf + pos, input, inputLen * sizeof(wchar_t));
    }
    if (pos < textLen) {
        memcpy(buf + (pos + inputLen), text + pos,
               (textLen - pos) * sizeof(wchar_t));
    }
    return TRUE;
}

BOOL Mng_c::isKanaInput() {
    if (getConsoleRegion() != CONSOLE_REGION_JP) {
        return FALSE;
    }
    if (mFlags & FLAG_CELL_PHONE) {
        return FALSE;
    }
    return !mpManager->getPCKeyboard()->isABC();
}

void Mng_c::setABC() {
    if (getConsoleRegion() == CONSOLE_REGION_JP &&
        !(mFlags & FLAG_CELL_PHONE)) {
        mpManager->getPCKeyboard()->setABC(true);
    }
}

BOOL Mng_c::isSignWindowOpen() {
    return mpManager->getSignKeyboard()->isActive();
}

void Mng_c::closeSignWindow() {
    dKbd::closeSignWindow(mpManager->getSignKeyboard());
}

BOOL Mng_c::isPCKeyboardVisible() {
    if (mFlags & FLAG_CELL_PHONE) {
        return FALSE;
    }
    return mpManager->getPCKeyboard()->getLayout()->GetRootPane()->IsVisible();
}

int Mng_c::getShiftState() {
    if (getConsoleRegion() != CONSOLE_REGION_JP) {
        return 0;
    }
    if (mFlags & FLAG_CELL_PHONE) {
        return 0;
    }
    u32 shift = mpManager->getPCKeyboard()->mKeyState.mShift;
    if (shift & 1) {
        return 1;
    }
    return (shift & 2) ? 2 : 0;
}

BOOL Mng_c::isUpdated() {
    if (mFlags & FLAG_CELL_PHONE) {
        return TRUE;
    }
    return mpManager->getPCKeyboard()->isCapsOn();
}

// Unused, stripped from the DOL: its call keeps the keypad's inline slot 0x40
// (800CB09C) in d_kbd.
int Mng_c::getKeypadState() {
    return mpManager->getCellPhoneKeyboard()->vt_0x40();
}

void Mng_c::updateInput() {
    if (isDecide()) {
        if (!(mFlags & FLAG_CELL_PHONE)) {
            playSe(SE_pupupu);
            return;
        }
    } else if (!isCharInput()) {
        return;
    }
    wchar_t c = 0;
    int inputLen = getInputLength();
    int pos = getInputPos();
    BOOL shift = FALSE;
    BOOL updated = isUpdated();
    BOOL pcKeyboard = FALSE;
    if (!isKanaInput() && getShiftState()) {
        updated = TRUE;
    }
    if (inputLen != 0) {
        c = (getInputText() + inputLen)[-1];
        if (!(mFlags & FLAG_CELL_PHONE)) {
            pcKeyboard = TRUE;
        }
    } else if (pos != 0) {
        c = (getText() + pos)[-1];
        if (mFlags & FLAG_CELL_PHONE) {
            shift = TRUE;
        } else if (!isKanaInput() && getShiftState()) {
            shift = TRUE;
        }
    }
    dLetter::playKeySe(c, updated, shift, pcKeyboard);
}

m2d::ResAccLoader_c *Mng_c::getResAcc() {
    return &mResAcc;
}

void Mng_c::saveSetting() {
    mpManager->vt_0xE0();
    sSetting = mpManager->getSetting();
    sSetting.m03 = 0;
}

void Mng_c::saveInputMode() {
    mpManager->vt_0xE0();
    textinput::extend::memo::Setting setting = mpManager->getSetting();
    sSetting.m05_4 = setting.m05_4;
}

Cur_c::Cur_c() {
}

Cur_c::~Cur_c() {
}

BOOL Cur_c::load() {
    return mRes.load("Layout/keyboard_cur.arc");
}

BOOL Cur_c::remove() {
    return mRes.remove();
}

void Cur_c::build() {
    mLyt.build(&mRes, "KB_cursor_00.brlyt");
    mLyt.setAnm(&mBlinkAnm, "KB_cursor_00_blinkNVCursor_00.brlan", NULL);
    mBlinkAnm.mFrameCtrl.mFlags = 0;
    mLyt.setAnm(&mColorAnm, "KB_cursor_00_v_cursor_color.brlan", NULL);
    mLyt.mPriority = 0x82;
    mLyt.calc();
}

void Cur_c::draw() {
    mBlinkAnm.play();
    mLyt.calc();
    mLyt.entry();
}

void Cur_c::setPos(const mVec3_c *pos) {
    mLyt.getRootPane()->SetTranslate(*pos);
}

const mVec3_c &Cur_c::getPos() {
    mVec3_c pos = mLyt.getRootPane()->GetTranslate();
    return pos;
}

textinput::input::HKBManager *getHKBManager() {
    return &textinput::input::HKBManager::sInstance;
}

void initHKB() {
    getHKBManager()->Initialize();
}

void calcHKB() {
    getHKBManager()->Update();
}

void resetSetting() {
    sSetting.mKeyboardType = 0x11;
    sSetting.m01 = 0xFA;
    sSetting.m02 = 0;
    sSetting.m04_7 = 1;
    sSetting.m04_3 = 2;
    sSetting.m05_4 = 1;
    sSetting.m03 = 0;
    sSetting.m05_0 = 0;
    sSetting.m04_2 = 0;
    sSetting.m04_1 = 0;
    sSetting.m04_0 = 0;
    sSetting.m06 = 0;
    sSetting.m07 = 0;
    sSavedByte = 0;
    sSavedFlag = 0;
}

u8 getSavedByte() {
    return sSavedByte;
}

void setSavedByte(u8 value) {
    sSavedByte = value;
}

BOOL getSavedFlag() {
    return sSavedFlag != 0;
}

void setSavedFlag(BOOL flag) {
    sSavedFlag = flag != FALSE;
}
} // namespace dKbd
