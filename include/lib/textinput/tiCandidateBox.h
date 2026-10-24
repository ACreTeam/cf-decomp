#ifndef TEXTINPUT_CANDIDATE_BOX_H
#define TEXTINPUT_CANDIDATE_BOX_H

#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiUtil.h>
#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiString.h>
#include <lib/textinput/tiTextInputBase.h>
#include <string.h>

namespace textinput {
class Manager;

namespace candidatebox {
class Base;
class LayoutByNW4R;
class OnOffAnmPane;
class CandidateScrollAnmPane;
class CandidateTextAnmPane;
class PredictWindow;

enum Animation {
    ANM_Normal,
    ANM_FocusIn,
    ANM_FocusOut,
    ANM_RollOver,
    ANM_Pushed,
    ANM_FadeIn,
    ANM_FadeOut,
    ANM_OnOffPushed,
    ANM_PredictOn,
    ANM_PredictOff,
    ANM_InvalidNormal,
    ANM_InvalidOn,
    ANM_InvalidOff,
    ANM_Last,
};

enum KeyType {
    KT_ScrollButton,
    KT_CandidateText,
    KT_OnOffButton,
    KT_Window,
    KT_Last,
};

class CandidateBoxCaller {
public:
    struct Candidates {
        /* 0x000 */ s32 mNumCandidate;
        /* 0x004 */ wchar_t mszCandidate[50][61];
    }; // size 0x17D8

    CandidateBoxCaller() : mpCandidateBox(NULL) { mCandidates.mNumCandidate = 0; }

    virtual void resetCandidate() { // 0x08
        mCandidates.mNumCandidate = 0;
        updateCandidate();
    }
    virtual void addCandidate(const wchar_t *wcString) { // 0x0C
        wcsncpy(mCandidates.mszCandidate[mCandidates.mNumCandidate], wcString, 61);
        mCandidates.mNumCandidate++;
    }
    virtual void setCandidateBox(Base *candidateBox) { mpCandidateBox = candidateBox; } // 0x10
    virtual void updateCandidate();    // 0x14
    virtual void makeEmptyCandidate(); // 0x18

    /* 0x04 */ Base *mpCandidateBox;
    /* 0x08 */ Candidates mCandidates;
}; // size 0x17E0

class Base : public CommandSender {
  public:
    Base(Manager *manager)
        : mbOn(false), mbInvalid(true), meLanguage(USA), mpManager(manager) {
        memset(&mCandidates, 0, sizeof(mCandidates));
    }

    virtual void init() {
    }
    virtual void setCommandReceiver(textinput::CommandReceiver *receiver);
    virtual void updateFromReceiver(u32 command, void *data);
    virtual void updateCandidate(CandidateBoxCaller::Candidates *candidates);
    virtual void draw() {
    }
    virtual void turnOn() {
    }
    virtual void turnOff() {
    }
    virtual void makeEmpty(CandidateBoxCaller::Candidates *candidates) {
    }
    virtual bool isOn() const {
        return mbOn;
    }
    virtual void onClose() {
    }
    virtual void setOnOff(bool on);
    virtual void setInvalid(bool invalid) {
        mbInvalid = invalid;
    }
    virtual bool isInvalid() const {
        return mbInvalid;
    }
    virtual void setLanguage(Language language) {
        meLanguage = language;
    }

  protected:
    Manager *mgr() {
        return mpManager;
    }
    CandidateBoxCaller::Candidates mCandidates;
    bool mbOn;
    bool mbInvalid;
    Language meLanguage;
    Manager *mpManager;
};

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayoutByNW4R(layout) {
    }

    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    LayoutByNW4R *mpLayoutByNW4R;
};

class UIObj : public gui::EventHandler {
  public:
    class Listener {
      public:
        virtual void onEvent(UIObj *, u32, void *);
    };

    UIObj(u32 id, LayoutByNW4R *layout, Listener *listener)
        : mId(id), mpManager(layout), mpListener(listener) {
    }

    virtual ~UIObj() {
    }
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input) {
    }

    virtual void onEvent(gui::GUIComponent &component, u32 event, void *data) {
        onGUIEvent(static_cast<gui::PaneComponent &>(component), event,
                   static_cast<nw4rmanager::TiEventHandler::Input *>(data));
    }

    void riseEvent(u32, void *);

    LayoutByNW4R *mgr() {
        return mpManager;
    }
    u32 getID() const {
        return mId;
    }

  protected:
    u32 mId;
    LayoutByNW4R *mpManager;
    Listener *mpListener;
};

class UITextArea : public UIObj, public util::AnimObserver {
  public:
    UITextArea(u32 id, LayoutByNW4R *layout, UIObj::Listener *listener);

    virtual void onAnmEvent(AnmEvent event, void *);
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

    void Create(nw4rmanager::Layout *layout);
    void SetCandidates(CandidateBoxCaller::Candidates *candidates) {
        mpCandidates = candidates;
    }
    void Init();
    void Clear();
    void Calc();
    void Draw(const nw4r::lyt::DrawInfo &);
    void StartScroll(s32);
    void StartScrollToIdx(s32);
    void StartScrollPageToIdx(s32);
    void StartRightPageScroll();
    void StartLeftPageScroll();
    void ScrollToSelectedText();
    void StopScroll();
    void ResetCandidate();
    void RefreshCandidates();
    void ChangeBaseIdx_(s32);
    bool CanLeftScroll() const {
        return mnBaseIdx > 0;
    }
    bool CanRightScroll() const {
        return mnDispEndCompleteIdx < mnCandidateNum - 1;
    }
    void SetScreenSize(nw4r::ut::Rect &);
    void ChangeSelectedText(s32);
    s32 GetPaneIdx(s32) const;
    s32 GetCandidateIdx(s32) const;

    bool IsScrolling();
    s32 GetDispOffset() const {
        return mnBaseIdx;
    }
    s32 GetSelectedPaneIdx() const {
        return GetPaneIdx(mnSelectedIdx);
    }
    void SetSelectedText(s32 pane) {
        mnSelectedIdx = pane;
    }
    s32 GetSelectedTextIdx() const {
        return mnSelectedIdx;
    }
    void SetFocusedText(s32 pane) {
        mnFocusedIdx = pane;
    }
    s32 GetFocusedText() {
        return mnFocusedIdx;
    }
    f32 GetWidthScale_() const {
        return 608.0f / mfScreenWidth;
    }
    f32 GetMargin_() const {
        if (mfScreenWidth > 700.0f) {
            return 10.0f;
        }
        return 20.0f;
    }
    s32 GetNextPageIdx_(s32 page) const;
    s32 GetPrevPageIdx_(s32 page) const;
    s32 GetDispNum() const;

  private:
    void CalcPaneLocate_();

    static const int NUM_PANES = 20;

    util::Animation mScrollAnimation;
    gui::PaneComponent *mpTextBoxPane[NUM_PANES];
    gui::PaneComponent *mpBoundingPane[NUM_PANES];
    gui::PaneComponent *mpTextAreaPane;
    gui::PaneComponent *mpTextsPane;
    CandidateTextAnmPane *mpTextAnmPane[NUM_PANES];
    nw4rmanager::AnmPane::AnmState mAnmState[NUM_PANES];
    s32 mnSelectedIdx;
    s32 mnFocusedIdx;
    s32 mnBaseIdx;
    f32 mfXOffset;
    s32 mnCandidateNum;
    s32 mnDispBeginIdx;
    s32 mnDispEndIdx;
    s32 mnDispEndCompleteIdx;
    f32 mfTextWidth;
    f32 mfScreenWidth;
    bool mbOnTextArea;
    CandidateBoxCaller::Candidates *mpCandidates;
};

class UIButton : public UIObj {
  public:
    UIButton(u32 id, LayoutByNW4R *layout, UIObj::Listener *listener);

    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

    void Create(nw4rmanager::Layout *layout, const char *s1, const char *s2);
    void Init();

    void SetVisible(bool visible);
    bool IsVisible() const {
        return mpButtonPane->getPane()->IsVisible();
    }

  private:
    gui::PaneComponent *mpButtonPane;
    gui::PaneComponent *mpButtonBoundPane;
    CandidateScrollAnmPane *mpButtonAnmPane;
};

class UIOnOffButton : public UIObj, public nw4rmanager::AnmObserver {
  public:
    UIOnOffButton(u32 id, LayoutByNW4R *layout, UIObj::Listener *listener);

    virtual void onChangeAnmState(AnmEvent anmEvent,
                                  nw4rmanager::AnmPane *anmPane,
                                  nw4rmanager::Anim *anim);
    virtual void onGUIEvent(gui::PaneComponent &component, u32 event,
                            nw4rmanager::TiEventHandler::Input *input);

    void Create(nw4rmanager::Layout *layout);
    void Init();
    void SetOnOff(bool on);
    void SetLanguage(Language);
    bool IsTurning() const;

  private:
    gui::PaneComponent *mpOnPictPane;
    gui::PaneComponent *mpOnBoundPane;
    gui::PaneComponent *mpOffPictPane;
    gui::PaneComponent *mpOffBoundPane;
    OnOffAnmPane *mpOnAnmPane;
    OnOffAnmPane *mpOffAnmPane;
    GXTexObj mOffTexDefault;
    GXTexObj mOffTexJP;
};

class UITextWindow : public UIObj, public nw4rmanager::AnmObserver {
  public:
    UITextWindow(u32 id, LayoutByNW4R *layout, UIObj::Listener *listener);

    virtual void onChangeAnmState(AnmEvent anmEvent,
                                  nw4rmanager::AnmPane *anmPane,
                                  nw4rmanager::Anim *anim);

    void Create(nw4rmanager::Layout *layout, const char *s1);
    void Init();
    void TurnOnOff(bool on);
    void TurnInvalid(bool invalid);
    void SetVisible(bool visible);

    void ChangeAnimation(u32);

  private:
    gui::PaneComponent *mpWindowPane;
    PredictWindow *mpWindowAnmPane;
};

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public UIObj::Listener {
  public:
    enum {
        UI_ID_TEXT_AREA,
        UI_ID_LEFT_SCROLL,
        UI_ID_RIGHT_SCROLL,
        UI_ID_ONOFF_BUTTON,
        UI_ID_TEXT_WINDOW
    };

    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *accessor,
                 const char *layoutName, EventObserver *observer)
        : Base(manager), nw4rmanager::Layout(accessor, layoutName, observer),
          mEventHandler(this), mpTiString(NULL), mpAllocator(NULL),
          mTextArea(UI_ID_TEXT_AREA, this, this),
          mLeftScroll(UI_ID_LEFT_SCROLL, this, this),
          mRightScroll(UI_ID_RIGHT_SCROLL, this, this),
          mOnOffButton(UI_ID_ONOFF_BUTTON, this, this),
          mTextWindow(UI_ID_TEXT_WINDOW, this, this), mbActive(true) {
    }
    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void updateCandidate(CandidateBoxCaller::Candidates *candidates);
    virtual void draw();
    virtual void turnOn();
    virtual void turnOff();
    virtual void makeEmpty(CandidateBoxCaller::Candidates *candidates);
    virtual void onClose();
    virtual void setOnOff(bool);
    virtual void setInvalid(bool);
    virtual void setLanguage(Language);

    virtual void calc();
    virtual void onRightScroll();
    virtual void onLeftScroll();
    virtual void onRightPageScroll();
    virtual void onLeftPageScroll();
    virtual void onOnOffButton(bool);

    virtual void changeSelectEmpty_compatible();
    virtual void changeSelect(s32 pane);
    virtual void updateDisplay();
    virtual u16 getScroll() {
        return mTextArea.GetDispOffset();
    }
    virtual bool isInScroll() {
        return mTextArea.IsScrolling();
    }
    virtual void setActive(bool active) {
        mbActive = active;
    }
    virtual bool isActive() const {
        return mbActive;
    }
    virtual void setSelectedTextPane(int pane) {
        mTextArea.SetSelectedText(pane);
    }
    virtual void setFocusedTextPane(int pane) {
        mTextArea.SetFocusedText(pane);
    }
    virtual int getFocusedTextPane() {
        return mTextArea.GetFocusedText();
    }
    virtual void SetScreenSize() {
        mTextArea.SetScreenSize(mScreenRect);
    }
    virtual void setRootPaneScaleFor16x9();
    virtual void setRootPaneScaleFor4x3();
    virtual void cancelStateFocusIn();
    virtual void onEvent(UIObj *, u32, void *);

    void onPressedShift(bool shift);
    void onReleasedShift();
    void startTurnOn(bool);
    const UIOnOffButton &getOnOffButton() const {
        return mOnOffButton;
    }

  private:
    void createAnmPane_(MEMAllocator *allocator);

    EventHandler mEventHandler;
    tistring::Decolated *mpTiString;
    MEMAllocator *mpAllocator;

  public:
    UITextArea mTextArea;

  private:
    UIButton mLeftScroll;
    UIButton mRightScroll;
    UIOnOffButton mOnOffButton;
    UITextWindow mTextWindow;
    bool mbActive;
};

class CandidateTextAnmPane : public nw4rmanager::AnmPane {
  public:
    CandidateTextAnmPane(nw4r::lyt::Pane *pane,
                         nw4rmanager::AnmObserver *observer)
        : nw4rmanager::AnmPane(pane, observer), meState(ANM_Normal) {
        init();
        meKeyType = KT_CandidateText;
    }

    virtual void changeAnimation(u32 id) {
        meState = static_cast<Animation>(id);
        nw4rmanager::AnmPane::changeAnimation(id);
    }
    virtual void init() {
        meState = ANM_Normal;
    }
    virtual enum Animation getState() const {
        return meState;
    }
    virtual enum KeyType getKeyType() const {
        return meKeyType;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);

  protected:
    Animation meState;
    KeyType meKeyType;
};

class CandidateScrollAnmPane : public CandidateTextAnmPane {
  public:
    CandidateScrollAnmPane(nw4r::lyt::Pane *pane,
                           nw4rmanager::AnmObserver *observer)
        : CandidateTextAnmPane(pane, observer) {
        meKeyType = KT_ScrollButton;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};

class OnOffAnmPane : public CandidateTextAnmPane {
  public:
    OnOffAnmPane(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : CandidateTextAnmPane(pane, observer) {
        meKeyType = KT_OnOffButton;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);

    bool isTurning() const {
        return meState == ANM_OnOffPushed;
    }
};

class PredictWindow : public CandidateTextAnmPane {
  public:
    PredictWindow(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : CandidateTextAnmPane(pane, observer) {
        meKeyType = KT_Window;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};

class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *accessor,
           const char *layoutName, EventObserver *observer)
        : LayoutByNW4R(manager, accessor, layoutName, observer) {
    }
};

} // namespace candidatebox
} // namespace textinput

#endif
