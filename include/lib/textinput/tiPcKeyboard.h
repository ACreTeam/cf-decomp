#pragma once

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>
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

class LayoutByNW4R;
class EventHandler;

class KeyState {
  public:
    void refresh_();
    void refreshText(nw4r::lyt::Pane *pane);
    void setABCFlag(u32 flag);
    void setInputMode(u32 mode);
    wchar_t getWCCode(u32 key);
    wchar_t getWCCode(char *paneName);

    union {
        struct {
            /* 0x00 */ u32 _00 : 25;
            /* 0x00 */ u32 mCapsOn : 1;
            /* 0x00 */ u32 _1A : 2;
            /* 0x00 */ u32 mShift : 4;
        };
        /* 0x00 */ u32 muFlag;
    };
    /* 0x04 */ int mInputType;
    /* 0x08 */ u32 mInputMode : 28;
    /* 0x08 */ u32 _08 : 4;
}; // size 0xC

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

    Base(Manager *manager);
    virtual ~Base();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void onKey(u32 key, void *data);
    virtual int getType();
    virtual void setLanguage(Language language);
    virtual void onActive();
    virtual void inputCharCode(wchar_t ch); // 0x38
    virtual void *getState();               // 0x3C
    virtual bool isShiftOn() const;         // 0x40
    virtual bool isCapsOn() {
        return mKeyState.mCapsOn;
    } // 0x44
    virtual bool isABC() {
        return mKeyState.mInputType == 0;
    }                              // 0x48
    virtual void setABC(bool abc); // 0x4C
    virtual InputMode getABCInputMode() const;      // 0x50
    virtual InputMode getAIUInputMode() const;      // 0x54
    virtual TranslateMode getTranslateMode() const; // 0x58
    virtual void setTranslateMode(TranslateMode mode); // 0x5C
    virtual void setLangKeyActive(bool active);        // 0x60
    virtual void setInputModeJP(bool abc, u32 abcMode, u32 aiuMode); // 0x64
    virtual void refreshState();                                     // 0x68
    virtual void changeABCInputMode(InputMode mode);                 // 0x6C
    virtual void changeAIUInputMode(InputMode mode);                 // 0x70
    virtual void onClose(); // 0x74
    virtual wchar_t getWCCode(char *paneName);            // 0x78
    virtual VKeyCode getControlKey(char *paneName);       // 0x7C
    virtual void sendInputWChar(wchar_t ch, bool direct); // 0x80
    virtual void goSignInputMode();                       // 0x84
    virtual void changePredictLanguage();                 // 0x88
    virtual void updateFixMode();                         // 0x8C

    /* 0x14 */ u8 _14[0x20 - 0x14];
    /* 0x20 */ KeyState mKeyState;
    /* 0x2C */ u8 _2C[0x34 - 0x2C];
    /* 0x34 */ Base *mpBase;
    /* 0x38 */ MEMAllocator *mpAllocator;
    /* 0x3C */ bool mbKanaLock;
    /* 0x40 */ Manager *mpManager;
}; // size 0x44

class UIObj : public gui::EventHandler {
  public:
    class Listener {
      public:
        virtual void onEvent(UIObj *object, u32 event, void *data) = 0;
    };

    virtual ~UIObj();
    virtual void onEvent(gui::GUIComponent &component, u32 event, void *data);
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

  protected:
    /* 0x08 */ u32 muID;
    /* 0x0C */ LayoutByNW4R *mpLayout;
    /* 0x10 */ Listener *mpListener;
}; // size 0x14

class UIModifierButton : public UIObj {
  public:
    UIModifierButton(u32 id, LayoutByNW4R *layout, Listener *listener);
    virtual ~UIModifierButton();
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

  private:
    /* 0x14 */ nw4rmanager::AnmPane *_14;
    /* 0x18 */ nw4rmanager::AnmPane *_18;
    /* 0x1C */ nw4rmanager::AnmPane *_1C;
    /* 0x20 */ bool _20;
}; // size 0x24

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public UIObj::Listener {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver);
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
    virtual void
    setPredictLanguageDialog(predictlang::LayoutByNW4R *dialog);  // 0x118
    virtual void setSignWindow(signwindow::LayoutByNW4R *window); // 0x11C
    virtual void setLineFeedButton(bool enable);                  // 0x120
    virtual void setPredictLanguageButton(bool enable);           // 0x124
    virtual void setSignWindowButton(bool enable);                // 0x128
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
    void onPressedShift(bool shift);
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

} // namespace pctype
} // namespace keyboard
} // namespace textinput
