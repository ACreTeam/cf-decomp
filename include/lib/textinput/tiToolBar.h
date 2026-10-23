#ifndef TEXTINPUT_TOOLBAR_H
#define TEXTINPUT_TOOLBAR_H

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
class Manager;

namespace toolbar {
class Base {
  public:
    Base(Manager *manager)
        : mQwerty(true), meLanguage(USA), mpAllocator(NULL),
          mpManager(manager) {
    }
    virtual void create(MEMAllocator *allocator) {
        mpAllocator = allocator;
    }
    virtual void init() {
    }
    virtual bool isQwerty() const {
        return mQwerty;
    }
    virtual void setQwerty(bool qwerty);
    virtual void setLanguage(Language language) {
        meLanguage = language;
    }
    virtual Language getLanguage() const {
        return meLanguage;
    }

  protected:
    Manager *mgr() {
        return mpManager;
    }

    bool mQwerty;
    Language meLanguage;
    MEMAllocator *mpAllocator;
    Manager *mpManager;
};

class AnmPane;
class EventHandler;

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *accessor,
                 const char *layoutName, EventObserver *observer)
        : Base(manager), nw4rmanager::Layout(accessor, layoutName, observer),
          mpEventHandler(NULL), mbIsEnableQwertyChg(false) {
    }
    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void draw() {
        nw4rmanager::Layout::draw();
    }

    virtual void setQwerty(bool qwerty);
    virtual void setLanguage(Language language);

    virtual void onOK();
    virtual void onCancel();
    virtual void setQwertyWithSE(bool qwerty);
    virtual bool isHWKBMode() {
        return mbIsEnableQwertyChg;
    }
    virtual void enableKeytopChange(bool enable);
    virtual nw4r::lyt::Pane *getUpArea();
    virtual nw4r::lyt::Pane *getDownArea();
    virtual void setOKButtonVisible(bool visible);
    virtual void setCancelButtonVisible(bool visible);
    virtual void setOKButtonCaption(const wchar_t *caption);
    virtual void setCancelButtonCaption(const wchar_t *caption);
    virtual void cancelStateFocusIn();

  private:
    EventHandler *mpEventHandler;
    bool mbIsEnableQwertyChg;
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
    ANM_ToggleOn,
    ANM_ToggleOff,

    ANM_07,
    ANM_08,
    ANM_09,
    ANM_10,
    ANM_11,

    ANM_Last,
};

enum KeyType {
    KT_NormalButton,
    KT_ToggleButton,
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
    virtual enum Animation getState() {
        return meState;
    }

  protected:
    Animation meState;
    KeyType meKeyType;
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

class ToggleButtonAnmPane : public AnmPane {
  public:
    ToggleButtonAnmPane(nw4r::lyt::Pane *pane,
                        nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = KT_ToggleButton;
    }
    virtual ~ToggleButtonAnmPane() {
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

} // namespace toolbar
} // namespace textinput

#endif
