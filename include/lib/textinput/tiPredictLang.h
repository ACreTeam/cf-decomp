#ifndef TEXTINPUT_PREDICT_LANG_H
#define TEXTINPUT_PREDICT_LANG_H

#include <lib/textinput/tiInputForm.h>
#include <lib/textinput/tiKeyboard.h>
#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
class Manager;

namespace predictlang {
class LayoutByNW4R;

class Base : public CommandSender {
  public:
    Base(Manager *manager)
        : mePredictMode(inputform::Base::PM_USEn), mpAllocator(NULL),
          mpManager(manager) {
    }
    virtual void create(MEMAllocator *allocator) {
        mpAllocator = allocator;
    }
    virtual void init() {
    }
    virtual void setPredictMode(inputform::Base::PredictMode mode) {
        mePredictMode = mode;
    }

  protected:
    inputform::Base::PredictMode mePredictMode;
    MEMAllocator *mpAllocator;
    Manager *mpManager;
};

class AnmPane;
class EventHandler;

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public nw4rmanager::AnmObserver {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *accessor,
                 const char *layoutName, EventObserver *observer)
        : Base(manager), nw4rmanager::Layout(accessor, layoutName, observer),
          mbActive(false), mbInput(false), mpEventHandler(NULL),
          meDestination(DST_US), mpCaller(NULL) {
    }
    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void draw() {
        nw4rmanager::Layout::draw();
    }

    virtual void open(inputform::Base::PredictMode predictMode,
                      keyboard::KeyboardBase *keyboard);
    virtual void endToOpen();
    virtual void startToClose();
    virtual void endToClose();
    virtual bool isActive() {
        return mbActive;
    }
    virtual bool canBeInputed() {
        return mbInput;
    }

    virtual void onChangeAnmState(AnmEvent anmEvent,
                                  nw4rmanager::AnmPane *anmPane,
                                  nw4rmanager::Anim *anim);

    virtual void setAllButtonToggleOffWithout(const char *paneName);
    virtual void setAllButtonNormal();
    virtual void setDestination(Destination destination);
    virtual void setLanguage(Language language);
    virtual void changeDestination(Destination destination);

    virtual nw4rmanager::AnmPane *
    getTargetAnmPane(Destination destination,
                     inputform::Base::PredictMode predictMode);

  private:
    bool mbActive;
    bool mbInput;
    EventHandler *mpEventHandler;
    Destination meDestination;
    keyboard::KeyboardBase *mpCaller;
};

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayoutByNW4R(layout) {
    }

    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

  private:
    LayoutByNW4R *mpLayoutByNW4R;
};

enum Animation {
    ANM_Normal,
    ANM_FocusIn,
    ANM_FocusOut,
    ANM_RollOver,
    ANM_Pushed,
    ANM_FadeIn,
    ANM_FadeOut,
    ANM_ToggleOn,
    ANM_ToggleOff,

    ANM_Last,
};

enum KeyType {
    KT_NormalButton,
    KT_ChangeButton,
    KT_Whole,
    KT_Last,
};

class AnmPane : public nw4rmanager::AnmPane {
  public:
    AnmPane(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : nw4rmanager::AnmPane(pane, observer), meState(ANM_Normal) {
    }

    virtual void changeAnimation(u32 id) {
        meState = static_cast<Animation>(id);
        nw4rmanager::AnmPane::changeAnimation(id);
    }
    virtual void init() {
        meState = ANM_Normal;
    }
    virtual enum KeyType getKeyType() const {
        return meKeyType;
    }

  protected:
    Animation meState;
    KeyType meKeyType;
};

class WholeAnmPane : public AnmPane {
  public:
    WholeAnmPane(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = KT_Whole;
    }
    virtual ~WholeAnmPane() {
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};

class NormalButtonAnmPane : public AnmPane {
  public:
    NormalButtonAnmPane(nw4r::lyt::Pane *pane,
                        nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = KT_NormalButton;
    }
    virtual ~NormalButtonAnmPane() {
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};
class ChangeButtonAnmPane : public AnmPane {
  public:
    ChangeButtonAnmPane(nw4r::lyt::Pane *pane,
                        nw4rmanager::AnmObserver *observer,
                        LayoutByNW4R *layout)
        : AnmPane(pane, observer), mpLayout(layout) {
        meKeyType = KT_ChangeButton;
    }
    virtual ~ChangeButtonAnmPane() {
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);

  private:
    LayoutByNW4R *mpLayout;
};
class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *accessor,
           const char *layoutName, EventObserver *observer)
        : LayoutByNW4R(manager, accessor, layoutName, observer) {
    }
};

} // namespace predictlang
} // namespace textinput

#endif
