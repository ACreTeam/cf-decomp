#pragma once

#include <lib/textinput/tiKeyboard.h>
#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiPkData.h>
#include <lib/textinput/tiString.h>

namespace textinput {

class Manager;

namespace predictlang {
class LayoutByNW4R;
}

namespace keyboard {

namespace signwindow {
class LayoutByNW4R;
}

namespace pctype {

static const int GRID_KEY_COUNT = 56;
static const int CONTROL_KEY_COUNT = 21;
static const int VISIBLE_PANE_COUNT = 24;
static const int ABLAUT_COUNT = 16;
static const int ANIMATION_FILE_COUNT = 16;
static const int PANE_ANIMATION_COUNT = 127;

enum ControlKey {
    CK_LineFeed,
    CK_BackSpace,
    CK_Space,
    CK_Caps,
    CK_Shift,
    CK_PredictLanguage = 0x0A,
    CK_ABC,
    CK_Kana,
    CK_SignWindow,
    CK_0E,
    CK_Direct,
    CK_RomaHiragana,
    CK_RomaKatakana,
    CK_Hiragana,
    CK_Katakana,
    CK_Dakuten,
    CK_Handaku,
    CK_Small,
    CK_None = 0x1B,
};

struct PaneNameToCharCodeWithMode {
    /* 0x00 */ char szPaneName[0x12];
    /* 0x12 */ wchar_t wc[4];
}; // size 0x1A

struct PaneNameToControlKey {
    /* 0x00 */ char szPaneName[0x14];
    /* 0x14 */ ControlKey code;
}; // size 0x18

struct VisiblePanes {
    /* 0x00 */ u16 uVisibleNum;
    /* 0x02 */ u16 uInvisibleNum;
    /* 0x04 */ const char *szVisiblePanes[VISIBLE_PANE_COUNT];
    /* 0x64 */ const char *szInvisiblePanes[VISIBLE_PANE_COUNT];
}; // size 0xC4

struct CharCodeToAblaut {
    /* 0x0 */ wchar_t wc;
    /* 0x2 */ wchar_t ablaut[5];
}; // size 0xC

struct LanguageDependency {
    /* 0x00 */ const PaneNameToCharCodeWithModeAscii *pKeyboard;
    /* 0x04 */ const PaneNameToCharCodeWithMode *pGridKeyboard;
    /* 0x08 */ const PaneNameToControlKey *pControlKeys;
    /* 0x0C */ const VisiblePanes *pVisiblePanes;
    /* 0x10 */ const CharCodeToAblaut *pAblaut;
}; // size 0x14

class Base;
class LayoutByNW4R;

class Base : public KeyboardBase {
  public:
    enum TranslateMode {
        TM_Direct,
        TM_Hiragana,
        TM_Katakana,
    };

    enum InputType {
        IT_00,
        IT_01,
        IT_02,
        IT_03,
        IT_04,
    };

    enum InputMode {
        IM_00,
        IM_01,
        IM_02,
        IM_03,
        IM_04,
        IM_05,
        IM_06,
        IM_07,
    };

    class KeyState {
      public:
        KeyState(Base *base)
            : muFlag(0), mInputType(0), mInputMode(0), meLanguage(USA),
              mpLanguageData(NULL), mpBase(base) {
        }

        void refresh_();
        void refreshText(nw4r::lyt::Pane *pane);
        void setABCFlag(u32 flag);
        void setInputMode(u32 mode);
        wchar_t getWCCode(u32 key);
        wchar_t getWCCode(char *paneName);
        ControlKey getControlKey(char *paneName);

        int getTranslateMode() const {
            switch (muTranslateMode) {
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
                return 2;
            }
            return 0;
        }
        int getAIUInputMode() const {
            switch (getKanaMode()) {
            case 0:
                return 6;
            case 1:
                return 7;
            }
            return 6;
        }
        u32 getTranslateBits() const {
            return muFlag & 0xF;
        }
        u32 getABCFlag() const {
            return muFlag & ~0xF;
        }
        u32 getKanaMode() const {
            return mInputMode & 0xF;
        }
        void init();
        void setTranslateMode(u32 mode) {
            if ((muFlag & 0xF) == mode) {
                return;
            }
            muFlag = (muFlag & ~0xF) | (mode & 0xF);
            refresh_();
        }
        void setKanaMode(u32 mode) {
            if ((mInputMode & 0xF) == mode) {
                return;
            }
            mInputMode = (mInputMode & ~0xF) | (mode & 0xF);
            refresh_();
        }
        void setKana(bool kana) {
            if (kana != mInputType) {
                mInputType = kana;
                refresh_();
            }
        }
        void clearFlag(u32 flag) {
            if ((muFlag & ~0xF) == 0) {
                return;
            }
            muFlag &= ~flag;
            refresh_();
        }
        void flipFlag(u32 flag) {
            u32 flipped = muFlag ^ flag;
            muFlag = (muFlag & ~flag) | (flipped & flag);
            refresh_();
        }

        union {
            struct {
                /* 0x00 */ u32 _00 : 24;
                /* 0x00 */ u32 mbShift : 1;
                /* 0x00 */ u32 mbCaps : 1;
                /* 0x00 */ u32 _1A : 2;
                /* 0x00 */ u32 muTranslateMode : 4;
            };
            /* 0x00 */ u32 muFlag;
        };
        /* 0x04 */ u32 mInputType;
        /* 0x08 */ u32 mInputMode;
        /* 0x0C */ Language meLanguage;
        /* 0x10 */ const LanguageDependency *mpLanguageData;
        /* 0x14 */ Base *mpBase;
    }; // size 0x18

    Base(Manager *manager)
        : mKeyState(this), mpAllocator(NULL), mbKanaLock(false),
          mpManager(manager) {
    }
    virtual ~Base() {
    }
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void onKey(u32 key, void *data);
    virtual int getType() {
        return 0;
    }
    virtual void setLanguage(Language language);
    virtual void onActive();
    virtual void inputCharCode(wchar_t ch); // 0x38
    virtual void *getState() {
        return &mState;
    } // 0x3C
    virtual bool isShiftOn() const; // 0x40

    void setShiftFlag(bool on) {
        u32 flag = mKeyState.getABCFlag();
        if (on) {
            flag |= 0x80;
        } else {
            flag &= ~0x80;
        }
        mKeyState.setABCFlag(flag);
    }

    virtual bool isCapsOn() {
        return mKeyState.mbCaps;
    } // 0x44
    virtual bool isABC() {
        return mKeyState.mInputType == 0;
    }                              // 0x48
    virtual void setABC(bool abc); // 0x4C
    virtual InputMode getABCInputMode() const {
        return IM_00;
    } // 0x50
    virtual InputMode getAIUInputMode() const {
        return (InputMode)mKeyState.getAIUInputMode();
    } // 0x54
    virtual TranslateMode getTranslateMode() const {
        return (TranslateMode)mKeyState.getTranslateMode();
    } // 0x58
    virtual void setTranslateMode(TranslateMode mode); // 0x5C
    virtual void setLangKeyActive(bool active);        // 0x60
    virtual void setInputModeJP(bool abc, u32 abcMode, u32 aiuMode) {
    } // 0x64
    virtual void refreshState();                          // 0x68
    virtual void changeABCInputMode(InputMode mode);      // 0x6C
    virtual void changeAIUInputMode(InputMode mode);      // 0x70
    virtual void onClose();                               // 0x74
    virtual wchar_t getWCCode(char *paneName);            // 0x78
    virtual ControlKey getControlKey(char *paneName);     // 0x7C
    virtual void sendInputWChar(wchar_t ch, bool direct); // 0x80
    virtual void goSignInputMode();                       // 0x84
    virtual void changePredictLanguage();                 // 0x88
    virtual void updateFixMode();                         // 0x8C

    struct State {
        /* 0x0 */ bool bInputReceived;
        /* 0x4 */ u32 _04;
        /* 0x8 */ const LanguageDependency *pLanguageData;
    }; // size 0xC

    /* 0x14 */ State mState;
    /* 0x20 */ KeyState mKeyState;
    /* 0x38 */ MEMAllocator *mpAllocator;
    /* 0x3C */ bool mbKanaLock;
    /* 0x40 */ Manager *mpManager;
}; // size 0x44

inline void Base::KeyState::init() {
    mInputType = 0;
    if (meLanguage == JP && !mpBase->mbKanaLock) {
        muFlag = TM_Hiragana;
    } else {
        muFlag = TM_Direct;
    }
    mInputMode = 0;
    refresh_();
}

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayout(layout) {
    }

    virtual ~EventHandler();
    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    /* 0x0C */ LayoutByNW4R *mpLayout;
}; // size 0x10

class UIObj : public gui::EventHandler {
  public:
    class Listener {
      public:
        virtual void onEvent(UIObj *object, u32 event, void *data) = 0;
    };

    UIObj(u32 id, LayoutByNW4R *layout, Listener *listener)
        : muID(id), mpLayout(layout), mpListener(listener) {
    }

    virtual ~UIObj() {
    }
    virtual void onEvent(gui::GUIComponent &component, u32 event, void *data) {
        onGUIEvent(static_cast<gui::PaneComponent &>(component), event,
                   static_cast<nw4rmanager::TiEventHandler::Input *>(data));
    }
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input) {
    }

  protected:
    /* 0x08 */ u32 muID;
    /* 0x0C */ LayoutByNW4R *mpLayout;
    /* 0x10 */ Listener *mpListener;
}; // size 0x14

class UIModifierButton : public UIObj {
  public:
    UIModifierButton(u32 id, LayoutByNW4R *layout, Listener *listener);
    virtual ~UIModifierButton() {
    }
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

    void Create(nw4rmanager::Layout *layout, const char *paneName,
                const char *buttonName);

    void release();
    void setPressed(bool pressed);
    void setTriggerTarget(bool enable);

  private:
    /* 0x14 */ gui::PaneComponent *mpPaneComponent;
    /* 0x18 */ gui::PaneComponent *mpButton;
    /* 0x1C */ nw4rmanager::AnmPane *mpAnmPane;
    /* 0x20 */ bool mbPressed;
}; // size 0x24

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public UIObj::Listener {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver)
        : Base(manager), Layout(multiArc, layoutBinName, eventObserver),
          mpEventHandler(NULL), mShiftButton(1, this, this),
          mCapsButton(0, this, this) {
    }
    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void onKey(u32 key, void *data);
    virtual void setLanguage(Language language);
    virtual void onActive();
    virtual void inputCharCode(wchar_t ch);
    virtual void setABC(bool abc);
    virtual void setTranslateMode(TranslateMode mode);
    virtual void setLangKeyActive(bool active);
    virtual void setInputModeJP(bool abc, u32 abcMode, u32 aiuMode);
    virtual void refreshState();
    virtual void onClose();
    virtual void sendInputWChar(wchar_t ch, bool direct);
    virtual void goSignInputMode();
    virtual void changePredictLanguage();
    virtual void draw(); // 0x110
    virtual void calc(); // 0x114
    virtual void setPredictLanguageDialog(predictlang::LayoutByNW4R *dialog) {
        mpPredictLanguageDialog = dialog;
    } // 0x118
    virtual void setSignWindow(signwindow::LayoutByNW4R *window) {
        mpSignWindow = window;
    } // 0x11C
    virtual void setLineFeedButton(bool enable);        // 0x120
    virtual void setPredictLanguageButton(bool enable); // 0x124
    virtual void setSignWindowButton(bool enable);      // 0x128
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release, void *data);       // 0x12C
    virtual bool updateInput(input::HKBManager &hkbManager); // 0x130
    virtual void changeAnimationAllToNormal();               // 0x134
    virtual void throwReleaseForAll();                       // 0x138
    virtual void cancelStateFocusIn();                       // 0x13C
    virtual void
    updatePredictLanguage(CommandReceiver::ChangePredictMode *mode); // 0x140
    virtual void updateDakuten();                                    // 0x144
    virtual void initLayout();                                       // 0x148
    virtual void onEvent(UIObj *object, u32 event, void *data);      // 0x14C

    void createAnmPane_(MEMAllocator *allocator);
    void onPressedCaps();
    void onPressedShift(bool se);
    void onReleasedShift();

    bool isLineFeedEnabled() const {
        return mbEnableLineFeed;
    }

    /* 0xF0 */ EventHandler *mpEventHandler;
    /* 0xF4 */ predictlang::LayoutByNW4R *mpPredictLanguageDialog;
    /* 0xF8 */ signwindow::LayoutByNW4R *mpSignWindow;
    /* 0xFC */ bool mbEnableLineFeed;
    /* 0x100 */ UIModifierButton mShiftButton;
    /* 0x124 */ UIModifierButton mCapsButton;
}; // size 0x148

class AnmPane : public nw4rmanager::AnmPane {
  public:
    AnmPane(nw4r::lyt::Pane *pane)
        : nw4rmanager::AnmPane(pane, NULL), meState(0) {
    }

    virtual void init() {
        meState = 0;
    }
    virtual void changeAnimation(u32 id);
    virtual ~AnmPane() {
    }
    virtual int getState() const {
        return meState;
    }
    virtual int getKeyType() const {
        return meKeyType;
    }

    /* 0x2C */ int meState;
    /* 0x30 */ int meKeyType;
}; // size 0x34

class NormalButtonAnmPane : public AnmPane {
  public:
    NormalButtonAnmPane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = 0;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~NormalButtonAnmPane() {
    }
};

class ShiftCapsAnmPane : public AnmPane {
  public:
    ShiftCapsAnmPane(nw4r::lyt::Pane *pane) : AnmPane(pane), mbOn(false) {
        meKeyType = 1;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~ShiftCapsAnmPane() {
    }

    bool isFocused() const;

    /* 0x34 */ bool mbOn;
}; // size 0x38

class ToggleButtonAnmPane : public AnmPane {
  public:
    ToggleButtonAnmPane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = 2;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~ToggleButtonAnmPane() {
    }
};

class OnOffButtonAnmPane : public AnmPane {
  public:
    OnOffButtonAnmPane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = 3;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~OnOffButtonAnmPane() {
    }
};

class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *multiArc,
           const char *layoutBinName, EventObserver *eventObserver)
        : LayoutByNW4R(manager, multiArc, layoutBinName, eventObserver) {
    }
};

} // namespace pctype
} // namespace keyboard
} // namespace textinput
