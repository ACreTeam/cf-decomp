#include <lib/textinput/tiSignWindow.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiLanguageIndependentData.h>
#include <lib/textinput/tiUtil.h>
#include <new>
#include <string.h>
#include <wchar.h>

namespace textinput {
namespace keyboard {
namespace signwindow {

struct AnimationFile {
    /* 0x00 */ u32 id;
    /* 0x04 */ char fileName[0x40];
}; // size 0x44

struct PaneToAnimation {
    /* 0x00 */ u32 type;
    /* 0x04 */ char paneName[0x14];
    /* 0x18 */ u32 count;
    /* 0x1C */ const char *forceAddName;
    /* 0x20 */ const AnimationFile *pAnims[8];
}; // size 0x40

struct PaneNameToControlKey {
    /* 0x00 */ char paneName[0x14];
    /* 0x14 */ int code;
}; // size 0x18

enum ControlCode {
    CC_Close = 0x18,
    CC_Next = 0x19,
    CC_Prev = 0x1A,
    CC_None = 0x1B,
};

static const AnimationFile csAninationFileForSign[] = {
    {0, "fs_signWindow_a_SGN_normal.brlan"},
    {1, "fs_signWindow_a_SGN_FADE-IN.brlan"},
    {8, "fs_signWindow_a_Scroll_FADE-OUT.brlan"},
    {2, "fs_signWindow_a_SGN_Focus-IN.brlan"},
    {6, "fs_signWindow_a_SGN_Focus-OUT.brlan"},
    {3, "fs_signWindow_a_SGN_Roll_over.brlan"},
    {4, "fs_signWindow_a_SGN_Pushed.brlan"},
    {7, "fs_signWindow_a_SGN_scroll_next.brlan"},
    {5, "fs_signWindow_a_SGN_scroll_prev.brlan"},
};

const char *csSgnKeys[KEY_COUNT] = {
    "P_SGNkey_00", "P_SGNkey_01", "P_SGNkey_02", "P_SGNkey_03", "P_SGNkey_04",
    "P_SGNkey_05", "P_SGNkey_06", "P_SGNkey_07", "P_SGNkey_08", "P_SGNkey_09",
    "P_SGNkey_10", "P_SGNkey_11", "P_SGNkey_12", "P_SGNkey_13", "P_SGNkey_14",
    "P_SGNkey_15", "P_SGNkey_16", "P_SGNkey_17", "P_SGNkey_18", "P_SGNkey_19",
};

static const char *COMMON_KEY_ANIM = "P_SGNkey_00";
static const char *COMMON_CONTROL_ANIM = "P_SGNkey_close";

#define SIGN_ALL_ANIMS                                                         \
    {&csAninationFileForSign[0], &csAninationFileForSign[1],                   \
     &csAninationFileForSign[2]}
#define SIGN_SCROLL_ANIMS                                                      \
    {&csAninationFileForSign[0], &csAninationFileForSign[7],                   \
     &csAninationFileForSign[8]}
#define SIGN_BUTTON_ANIMS                                                      \
    {&csAninationFileForSign[0], &csAninationFileForSign[1],                   \
     &csAninationFileForSign[2], &csAninationFileForSign[3],                   \
     &csAninationFileForSign[4], &csAninationFileForSign[5],                   \
     &csAninationFileForSign[6]}

static const PaneToAnimation csPaneToAnimationInSign[] = {
    {0, "N_SGNkeytop_all", 3, NULL, SIGN_ALL_ANIMS},
    {1, "N_SGNkeyall", 3, NULL, SIGN_SCROLL_ANIMS},
    {2, "P_SGNkey_00", 7, NULL, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_01", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_02", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_03", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_04", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_05", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_06", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_07", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_08", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_09", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_10", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_11", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_12", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_13", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_14", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_15", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_16", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_17", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_18", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_19", 7, COMMON_KEY_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_prev", 7, COMMON_CONTROL_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_next", 7, COMMON_CONTROL_ANIM, SIGN_BUTTON_ANIMS},
    {2, "P_SGNkey_close", 7, NULL, SIGN_BUTTON_ANIMS},
};

static const PaneNameToControlKey csPaneNameToControlKey[] = {
    {"B_SGNkey_close", CC_Close},
    {"B_SGNkey_prev", CC_Prev},
    {"B_SGNkey_next", CC_Next},
};

static const LanguageDependencyData csLanguageDependencyData[LanguageEnd] = {
    {4, csSignKeyJP},  {10, csSignKeyUS}, {10, csSignKeyUK}, {10, csSignKeyFR},
    {10, csSignKeyDE}, {10, csSignKeyIT}, {10, csSignKeySP}, {10, csSignKeyNL},
};

static int getControlCode(const char *paneName) {
    for (u16 i = 0; i < 3; i++) {
        if (util::strcmp(csPaneNameToControlKey[i].paneName, paneName)) {
            return csPaneNameToControlKey[i].code;
        }
    }
    return CC_None;
}

static wchar_t getSignWC(const PaneNameToCharCodeInSignMode *keys, u16 page,
                         const char *paneName, u16 count) {
    for (u16 i = 0; i < count; i++) {
        if (util::strcmp(keys[i].szPaneName, paneName)) {
            return keys[i].wc[page];
        }
    }
    return L'\0';
}

void Base::create(MEMAllocator *allocator) {
    muPage = 0;
    mbLocked = false;
    mbDirect = false;
    mpLanguageData = &csLanguageDependencyData[getLanguage()];
}

void Base::init() {
    muPage = 0;
    mbLocked = false;
    mbDirect = false;
    mpLanguageData = &csLanguageDependencyData[getLanguage()];
}

void Base::onKey(u32 key, void *data) {
    const char *paneName = static_cast<const char *>(data);
    if (key == 4) {
        int code = getControlCode(paneName);
        if (code != CC_None) {
            switch (code) {
            case CC_Close:
                close();
                break;
            case CC_Prev:
                movePrevSignPage();
                break;
            case CC_Next:
                moveNextSignPage();
                break;
            }
        } else {
            wchar_t wc =
                getSignWC(mpLanguageData->pKeys, muPage, paneName, KEY_COUNT);
            if (wc != L'\0') {
                inputform::Base::KeyInput input = {L'\0', 3,     false, 0,
                                                   false, false, NULL};
                input.ch = wc;
                input.direct = mbDirect;
                if (mpManager->getInputForm()->isAtokActive()) {
                    input.direct = true;
                }
                sendCommand(CommandReceiver::INPUT_COMMAND_KEY, &input);
            }
        }
    }
}

void Base::close() {
}

void Base::movePrevSignPage() {
    if (muPage == 0) {
        muPage = mpLanguageData->pageCount - 1;
    } else {
        muPage--;
    }
}

void Base::moveNextSignPage() {
    muPage++;
    if (muPage >= mpLanguageData->pageCount) {
        muPage = 0;
    }
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
    for (u16 i = 0; i < 25; i++) {
        const PaneToAnimation &info = csPaneToAnimationInSign[i];
        AnmPane *anmPane = NULL;
        switch (info.type) {
        case 2:
            anmPane = new (MEMAllocFromAllocator(
                allocator, sizeof(CellPhoneSignButtonPane)))
                CellPhoneSignButtonPane(getPane(info.paneName), this);
            break;
        case 1:
            anmPane = new (MEMAllocFromAllocator(
                allocator, sizeof(CellPhoneSignScrollAnmPane)))
                CellPhoneSignScrollAnmPane(getPane(info.paneName), this);
            break;
        case 0:
            anmPane = new (MEMAllocFromAllocator(
                allocator, sizeof(CellPhoneSignAllAnmPane)))
                CellPhoneSignAllAnmPane(getPane(info.paneName), this);
            break;
        }
        nw4r::ut::List_Append(&mAnmPanes, anmPane);
        for (u16 j = 0; j < info.count; j++) {
            void *res = mpMultiArcResourceAccessor->GetResource(
                0, info.pAnims[j]->fileName, NULL);
            AnimTransformPane *anim = reinterpret_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(res,
                                                 mpMultiArcResourceAccessor));
            if (info.forceAddName != NULL) {
                anmPane->forceAddAnimation(allocator, info.pAnims[j]->id, anim,
                                           info.forceAddName, false, true);
            } else {
                anmPane->addAnimation(allocator, info.pAnims[j]->id, anim,
                                      false, true);
            }
        }
    }
    init();
}

void LayoutByNW4R::init() {
    Base::init();
    searchAnmPane("N_SGNkeytop_all")->changeAnimation(0);
    setVisible("N_SGNkeytop_all", true);
    searchAnmPane("P_SGNkey_prev")->changeAnimation(0);
    searchAnmPane("P_SGNkey_next")->changeAnimation(0);
    setSignKeyTop(muPage, muPage);
    setPageNumber(muPage);
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
}

void LayoutByNW4R::onKey(u32 key, void *data) {
    Base::onKey(key, data);
    const char *paneName = static_cast<const char *>(data);
    if (key == 4) {
        int code = getControlCode(paneName);
        if (code != CC_None) {
            switch (code) {
            case CC_Close:
                mpEventObserver->onSE((sound::SE)9);
                break;
            case CC_Next:
            case CC_Prev:
                break;
            }
        } else {
            wchar_t wc =
                getSignWC(mpLanguageData->pKeys, muPage, paneName, KEY_COUNT);
            if (wc != L'\0' && mpParentKeyboard->getType() == 0) {
                sendCommand(0x27, NULL);
            }
        }
    }
}

void LayoutByNW4R::open(KeyboardBase *parent, bool direct) {
    resetAnmSignWindow();
    throwReleaseForAll();
    getPaneManager()->init();
    mbActive = true;
    mbInputEnabled = false;
    mpParentKeyboard = parent;
    initPaneLastDrawReceived();
    mbDirect = direct;
    setSignKeyTop(muPage, muPage);
    setPageNumber(muPage);
    searchAnmPane("N_SGNkeytop_all")->onAnmEvent(nw4rmanager::AnmPane::PE_6);
    mpEventObserver->onSE(sound::SE_SYMBOL_PAGE_OPEN);
}

void LayoutByNW4R::close() {
    if (mpParentKeyboard->getType() == 0) {
        static_cast<pctype::LayoutByNW4R *>(mpParentKeyboard)
            ->getLayout()
            ->GetRootPane()
            ->SetVisible(true);
    }
    searchAnmPane("N_SGNkeytop_all")->onAnmEvent(nw4rmanager::AnmPane::PE_7);
    mbInputEnabled = false;
}

void LayoutByNW4R::endToClose() {
    if (mpParentKeyboard != NULL) {
        mpParentKeyboard->update();
    }
    mbActive = false;
    setVisible("N_SGNkeytop_all", false);
}

void LayoutByNW4R::startToInput() {
    mbInputEnabled = true;
    if (mpParentKeyboard->getType() == 0) {
        static_cast<pctype::LayoutByNW4R *>(mpParentKeyboard)
            ->getLayout()
            ->GetRootPane()
            ->SetVisible(false);
    }
}

void LayoutByNW4R::onChangeAnmState(AnmEvent anmEvent,
                                    nw4rmanager::AnmPane *anmPane,
                                    nw4rmanager::Anim *anim) {
    if (util::strcmp("N_SGNkeyall", anmPane->getPane()->GetName())) {
        if ((anim->muID == 7 || anim->muID == 5) && anmEvent == E_1) {
            setSignKeyTop(muPage, muPage);
        }
    }
    if (util::strcmp("N_SGNkeytop_all", anmPane->getPane()->GetName())) {
        if (anim->muID == 8 && anmEvent == E_1) {
            endToClose();
        }
        if (anim->muID == 1 && anmEvent == E_1) {
            startToInput();
        }
    }
    if (anim->muID == 7 || anim->muID == 5) {
        if (anmEvent == E_0) {
            mbLocked = true;
        }
        if (anmEvent == E_1) {
            mbLocked = false;
        }
    }
}

void LayoutByNW4R::throwReleaseForAll() {
    for (AnmPane *pane =
             static_cast<AnmPane *>(nw4r::ut::List_GetNext(&mAnmPanes, NULL));
         pane != NULL; pane = static_cast<AnmPane *>(
                           nw4r::ut::List_GetNext(&mAnmPanes, pane))) {
        if (pane->getKeyType() == 2) {
            pane->onAnmEvent(nw4rmanager::AnmPane::PE_2);
        }
    }
}

void LayoutByNW4R::resetAnmSignWindow() {
    for (u16 i = 0; i < 25; i++) {
        static_cast<AnmPane *>(
            searchAnmPane(csPaneToAnimationInSign[i].paneName))
            ->init();
    }
}

void LayoutByNW4R::setSignKeyTop(u16 topPage, u16 nextPage) {
    for (u16 i = 0; i < KEY_COUNT; i++) {
        const char *paneName = mpLanguageData->pKeys[i].szPaneName;
        char textName[0x11];
        memset(textName, 0, sizeof(textName));
        strncpy(textName, paneName, strlen(paneName));
        textName[0] = 'T';
        wchar_t top[2];
        top[0] = mpLanguageData->pKeys[i].wc[topPage];
        top[1] = L'\0';
        const char *nextName = mpLanguageData->pKeys[i].szNextPaneName;
        wchar_t next[2];
        next[0] = mpLanguageData->pKeys[i].wc[nextPage];
        next[1] = L'\0';
        static_cast<nw4r::lyt::TextBox *>(getPane(textName))->SetString(top, 0);
        static_cast<nw4r::lyt::TextBox *>(getPane(nextName))
            ->SetString(next, 0);
    }
    setString("T_SGNkey_prev",
              langindependent::cLanguageIndependentString
                  [langindependent::LANG_STRID_PREV][getLanguage()]);
    setString("T_SGNkey_next",
              langindependent::cLanguageIndependentString
                  [langindependent::LANG_STRID_NEXT][getLanguage()]);
    setString("T_SGNkey_close",
              langindependent::cLanguageIndependentString
                  [langindependent::LANG_STRID_CLOSE][getLanguage()]);
}

void LayoutByNW4R::setPageNumber(u16 page) {
    wchar_t text[0x11];
    swprintf(text, 0x11, L"%d/%d", page + 1, mpLanguageData->pageCount);
    static_cast<nw4r::lyt::TextBox *>(getPane("T_SGN_pageNumber"))
        ->SetString(text, 0);
}

void LayoutByNW4R::movePrevSignPage() {
    u8 prevPage = muPage;
    Base::movePrevSignPage();
    u8 page = muPage;
    setSignKeyTop(prevPage, page);
    setPageNumber(page);
    movePrevSignWindow();
    mpEventObserver->onSE((sound::SE)15);
}

void LayoutByNW4R::moveNextSignPage() {
    u8 prevPage = muPage;
    Base::moveNextSignPage();
    u8 page = muPage;
    setSignKeyTop(prevPage, page);
    setPageNumber(page);
    moveNextSignWindow();
    mpEventObserver->onSE((sound::SE)15);
}

void LayoutByNW4R::movePrevSignWindow() {
    searchAnmPane("N_SGNkeyall")->onAnmEvent(nw4rmanager::AnmPane::PE_8);
}

void LayoutByNW4R::moveNextSignWindow() {
    searchAnmPane("N_SGNkeyall")->onAnmEvent(nw4rmanager::AnmPane::PE_9);
}

bool LayoutByNW4R::updateInput(int point, f32 x, f32 y, u32 trig, u32 hold,
                               u32 release, void *data) {
    if (mbInputEnabled) {
        return nw4rmanager::Layout::updateInput(point, x, y, trig, hold,
                                                release, data);
    }
    return false;
}

bool LayoutByNW4R::updateInput(input::HKBManager &hkbManager) {
    if (!mbInputEnabled) {
        return false;
    }
    if (hkbManager.GetTriggeredKeySet().IsValid()) {
        throwReleaseForAll();
        close();
        mpEventObserver->onSE((sound::SE)9);
        return false;
    }
    return nw4rmanager::Layout::updateInput(hkbManager);
}

void EventHandler::onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                             Input *input) {
    const char *paneName = paneComponent->getPane()->GetName();
    if (mpLayout->isLocked() && event == gui::EventHandler::ON_TRIG) {
        return;
    }
    if (event == gui::EventHandler::ON_TRIG && (input->trig & 0x800)) {
        mpLayout->onKey(4, const_cast<char *>(paneName));
    }
    if (paneName[0] != 'B') {
        return;
    }
    char anmName[0x11];
    memset(anmName, 0, sizeof(anmName));
    strncpy(anmName, paneName, strlen(paneName));
    anmName[0] = 'P';
    nw4rmanager::AnmPane *anmPane = mpLayout->searchAnmPane(anmName);
    if (anmPane == NULL) {
        return;
    }
    switch (event) {
    case gui::EventHandler::ON_TRIG:
        if (input->trig & 0x800) {
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

void CellPhoneSignAllAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_6) {
        mpPane->SetVisible(true);
        changeAnimation(1);
    } else if (paneEvent == PE_7) {
        changeAnimation(8);
    } else {
        switch (meState) {
        case 1:
            if (paneEvent == PE_4) {
                changeAnimation(0);
            }
            break;
        case 8:
            if (paneEvent == PE_4) {
                mpPane->SetVisible(false);
            }
            break;
        }
    }
}

void CellPhoneSignScrollAnmPane::onAnmEvent(AnmPaneEvent paneEvent) {
    switch (meState) {
    case 0:
        if (paneEvent == PE_9) {
            changeAnimation(5);
        }
        if (paneEvent == PE_8) {
            changeAnimation(7);
        }
        break;
    case 5:
    case 7:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        break;
    }
}

void CellPhoneSignButtonPane::onAnmEvent(AnmPaneEvent paneEvent) {
    if (paneEvent == PE_0) {
        changeAnimation(4);
    }
    switch (meState) {
    case 0:
        if (paneEvent == PE_1) {
            changeAnimation(2);
        }
        break;
    case 2:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(6);
        }
        break;
    case 3:
        if (paneEvent == PE_2) {
            changeAnimation(6);
        }
        break;
    case 6:
        if (paneEvent == PE_4) {
            changeAnimation(0);
        }
        if (paneEvent == PE_1) {
            changeAnimation(2);
        }
        break;
    case 4:
        if (paneEvent == PE_4) {
            changeAnimation(3);
        }
        if (paneEvent == PE_2) {
            changeAnimation(6);
        }
        break;
    }
}

} // namespace signwindow
} // namespace keyboard
} // namespace textinput
