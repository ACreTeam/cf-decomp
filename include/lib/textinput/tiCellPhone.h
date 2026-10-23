#pragma once

// textinput::keyboard::cellphonetype: the cell phone keypad (see
// tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>
#include <lib/textinput/tiCpData.h>

namespace textinput {

class Manager;

namespace predictlang {
class LayoutByNW4R;
}

namespace keyboard {

namespace signwindow {
class LayoutByNW4R;
}

namespace cellphonetype {

struct LanguageDependency {
    /* 0x00 */ u16 uNum;
    /* 0x04 */ const KeySet *pKeySets[5];
}; // size 0x18

struct OnKeyInfo {
    /* 0x0 */ const char *szPaneName;
    /* 0x4 */ bool bBackward;
    /* 0x5 */ bool bHold;
}; // size 0x8

struct State {
    /* 0x00 */ int mIndex;
    /* 0x04 */ int meInputMode;
    /* 0x08 */ bool mbLocked;
    /* 0x09 */ bool mbNumeric;
    /* 0x0A */ bool mbNumericWithDot;
    /* 0x0B */ bool mbUpperCase;
    /* 0x0C */ bool mbCapitalize;
    /* 0x0D */ bool mbPredictOn;
    /* 0x10 */ const PaneNameToCharCode *mpKey;
}; // size 0x14

struct KeyCommand {
    /* 0x00 */ wchar_t mChar;
    /* 0x04 */ u32 mFlags;
    /* 0x08 */ const PaneNameToCharCode *mpKey;
};

class Base : public KeyboardBase {
  public:
    enum InputMode {
        IM_Hiragana,
        IM_Katakana,
        IM_Alphabet,
        IM_Numeric,
        IM_AlphabetLower,
    };

    Base(Manager *manager) : mpAllocator(NULL), mpManager(manager) {
    }

    virtual ~Base() {
    }
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void onKey(u32 key, void *data);
    virtual int getType() {
        return 1;
    }
    virtual void onActive();
    virtual void calc(); // 0x38
    virtual void draw() {
    } // 0x3C
    virtual InputMode getInputMode() const {
        return (InputMode)mState.meInputMode;
    } // 0x40
    virtual bool isLocked() {
        return mState.mbLocked;
    } // 0x44
    virtual bool isNumeric() {
        return mState.mbNumeric;
    } // 0x48
    virtual bool isNumericWithDot() {
        return mState.mbNumericWithDot;
    } // 0x4C
    virtual bool isHoldingButton() const {
        return mState.mpKey != NULL;
    }                                          // 0x50
    virtual void doNumericMode(bool enable);   // 0x54
    virtual void changeInputMode(InputMode mode); // 0x58
    virtual bool isUpperCase() {
        return mState.mbUpperCase;
    } // 0x5C
    virtual void setInputMode(InputMode mode) {
    } // 0x60
    virtual void setUpperCaseJP(bool upperCase) {
    }                                     // 0x64
    virtual void onClose();               // 0x68
    virtual void goSignInputMode();       // 0x6C
    virtual void changePredictLanguage(); // 0x70
    virtual void setAbcMode(bool abc);    // 0x74
    virtual bool isZiActive();            // 0x78
    virtual bool isAtokActive();          // 0x7C
    virtual void doInput() {
    }                             // 0x80
    virtual void updateFixMode(); // 0x84

    /* 0x14 */ State mState;
    /* 0x28 */ const LanguageDependency *mpLanguageData;
    /* 0x2C */ MEMAllocator *mpAllocator;
    /* 0x30 */ Manager *mpManager;
}; // size 0x34

class LayoutByNW4R;

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayout(layout) {
    }

    virtual ~EventHandler();
    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    /* 0x0C */ LayoutByNW4R *mpLayout;
}; // size 0x10

class CellPhoneAnmPane : public nw4rmanager::AnmPane {
  public:
    CellPhoneAnmPane(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : nw4rmanager::AnmPane(pane, observer), meState(0), meKeyType(0) {
        init();
        changeAnimation(meState);
    }

    virtual void init() {
        meState = 0;
    }
    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual void changeAnimation(u32 id);
    virtual ~CellPhoneAnmPane() {
    }
    virtual int getKeyType() const {
        return meKeyType;
    }

    /* 0x2C */ int meState;
    /* 0x30 */ int meKeyType;
}; // size 0x34

class CellPhoneControlAnmPane : public CellPhoneAnmPane {
  public:
    CellPhoneControlAnmPane(nw4r::lyt::Pane *pane,
                            nw4rmanager::AnmObserver *observer)
        : CellPhoneAnmPane(pane, observer) {
        meKeyType = 1;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver)
        : Base(manager), Layout(multiArc, layoutBinName, eventObserver),
          mbEnableLineFeed(true), mpEventHandler(NULL),
          mpPredictLanguageDialog(NULL), mpSignWindow(NULL) {
    }

    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void setCommandReceiver(CommandReceiver *receiver) {
        CommandSender::setCommandReceiver(receiver);
        init();
    }
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void onKey(u32 key, void *data);
    virtual void setLanguage(Language language);
    virtual void update();
    virtual void onActive();
    virtual void calc();
    virtual void draw();
    virtual void doNumericMode(bool enable);
    virtual void changeInputMode(InputMode mode);
    virtual void setInputMode(InputMode mode);
    virtual void setUpperCaseJP(bool upperCase);
    virtual void onClose();
    virtual void goSignInputMode();
    virtual void changePredictLanguage();
    virtual void setAbcMode(bool abc);
    virtual void setPredictLanguageDialog(predictlang::LayoutByNW4R *dialog) {
        mpPredictLanguageDialog = dialog;
    } // 0xFC
    virtual void setSignWindow(signwindow::LayoutByNW4R *signWindow) {
        mpSignWindow = signWindow;
    }                                                   // 0x100
    virtual void doNumericWithDotMode(bool enable);     // 0x104
    virtual void setLineFeedButton(bool enable);        // 0x108
    virtual void setPredictLanguageButton(bool enable); // 0x10C
    virtual void setSignWindowButton(bool enable);      // 0x110
    virtual bool updateInput(int point, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release, void *data) {
        return nw4rmanager::Layout::updateInput(point, x, y, trig, hold,
                                                release, data);
    }                                                        // 0x114
    virtual bool updateInput(input::HKBManager &hkbManager); // 0x118
    virtual nw4rmanager::AnmPane *getAnmPane(InputMode mode); // 0x11C
    virtual void throwReleaseForAll();                       // 0x120
    virtual void changeAnimationAllToNormal();               // 0x124
    virtual void
    updatePredictLanguage(CommandReceiver::ChangePredictMode *mode); // 0x128
    virtual void changeKeyTop(const PaneNameToCharCode *keys);      // 0x12C
    virtual void changeSpaceKeyTop(const PaneNameToCharCode *keys); // 0x130

    bool isLineFeedEnabled() const {
        return mbEnableLineFeed;
    }

    /* 0xDC */ bool mbEnableLineFeed;
    /* 0xE0 */ EventHandler *mpEventHandler;
    /* 0xE4 */ predictlang::LayoutByNW4R *mpPredictLanguageDialog;
    /* 0xE8 */ signwindow::LayoutByNW4R *mpSignWindow;
}; // size 0xEC

class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *multiArc,
           const char *layoutBinName, EventObserver *eventObserver)
        : LayoutByNW4R(manager, multiArc, layoutBinName, eventObserver) {
    }
};

} // namespace cellphonetype
} // namespace keyboard
} // namespace textinput
