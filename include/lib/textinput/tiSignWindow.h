#pragma once

// textinput::keyboard::signwindow: the symbol window (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>
#include <lib/textinput/tiSwData.h>

namespace textinput {

class Manager;

namespace keyboard {
namespace signwindow {

struct LanguageDependencyData {
    /* 0x0 */ u8 pageCount;
    /* 0x4 */ const PaneNameToCharCodeInSignMode *pKeys;
}; // size 0x8

class Base : public KeyboardBase {
  public:
    Base(Manager *manager) : mpAllocator(NULL), mpManager(manager) {
    }

    virtual ~Base() {
    }
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onKey(u32 key, void *data);
    virtual int getType() {
        return 2;
    }
    virtual void setLanguage(Language language) {
        meLanguage = language;
        init();
    }
    virtual bool isLocked() {
        return mbLocked;
    } // 0x38
    virtual void setPage(u8 page) {
        muPage = page;
        if (muPage >= mpLanguageData->pageCount) {
            muPage = 0;
        }
    } // 0x3C
    virtual u8 getPage() {
        return muPage;
    } // 0x40
    virtual void movePrevSignPage(); // 0x44
    virtual void moveNextSignPage(); // 0x48
    virtual void close();            // 0x4C

  protected:
    /* 0x14 */ u8 _14;
    /* 0x15 */ bool mbLocked;
    /* 0x16 */ bool mbDirect;
    /* 0x17 */ u8 muPage;
    /* 0x18 */ const LanguageDependencyData *mpLanguageData;
    /* 0x1C */ MEMAllocator *mpAllocator;
    /* 0x20 */ Manager *mpManager;
}; // size 0x24

class EventHandler;

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public nw4rmanager::AnmObserver {
  public:
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver)
        : Base(manager), Layout(multiArc, layoutBinName, eventObserver),
          mbActive(false), mbInputEnabled(false), mpEventHandler(NULL),
          mpParentKeyboard(NULL) {
    }

    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onKey(u32 key, void *data);
    virtual void movePrevSignPage();
    virtual void moveNextSignPage();
    virtual void close();
    virtual void draw() {
        nw4rmanager::Layout::draw();
    } // 0xD0
    virtual void open(KeyboardBase *parent, bool direct); // 0xD4
    virtual bool isActive() {
        return mbActive;
    } // 0xD8
    virtual void onChangeAnmState(AnmEvent anmEvent,
                                  nw4rmanager::AnmPane *anmPane,
                                  nw4rmanager::Anim *anim); // 0xDC
    virtual bool updateInput(int point, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release, void *data);       // 0xE0
    virtual bool updateInput(input::HKBManager &hkbManager); // 0xE4
    virtual void throwReleaseForAll();                       // 0xE8
    virtual void resetAnmSignWindow();                       // 0xEC
    virtual void setSignKeyTop(u16 topPage, u16 nextPage);   // 0xF0
    virtual void setPageNumber(u16 page);                    // 0xF4
    virtual void endToClose();                               // 0xF8
    virtual void startToInput();                             // 0xFC
    virtual void movePrevSignWindow();                       // 0x100
    virtual void moveNextSignWindow();                       // 0x104

  protected:
    /* 0xD0 */ bool mbActive;
    /* 0xD1 */ bool mbInputEnabled;
    /* 0xD4 */ EventHandler *mpEventHandler;
    /* 0xD8 */ KeyboardBase *mpParentKeyboard;
}; // size 0xDC

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayout(layout) {
    }

    virtual ~EventHandler();
    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    /* 0x0C */ LayoutByNW4R *mpLayout;
}; // size 0x10

class AnmPane : public nw4rmanager::AnmPane {
  public:
    AnmPane(nw4r::lyt::Pane *pane, nw4rmanager::AnmObserver *observer)
        : nw4rmanager::AnmPane(pane, observer), meState(0) {
    }

    virtual void init() {
        meState = 0;
    }
    virtual void changeAnimation(u32 id) {
        meState = id;
        nw4rmanager::AnmPane::changeAnimation(id);
    }
    virtual ~AnmPane() {
    }
    virtual int getKeyType() const {
        return meKeyType;
    }

    /* 0x2C */ int meState;
    /* 0x30 */ int meKeyType;
}; // size 0x34

class CellPhoneSignAllAnmPane : public AnmPane {
  public:
    CellPhoneSignAllAnmPane(nw4r::lyt::Pane *pane,
                            nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = 0;
    }

    virtual void init() {
        meState = 0;
    }
    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~CellPhoneSignAllAnmPane() {
    }
};

class CellPhoneSignScrollAnmPane : public AnmPane {
  public:
    CellPhoneSignScrollAnmPane(nw4r::lyt::Pane *pane,
                               nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = 1;
    }

    virtual void init() {
        meState = 0;
    }
    virtual void onAnmEvent(AnmPaneEvent paneEvent);
};

class CellPhoneSignButtonPane : public AnmPane {
  public:
    CellPhoneSignButtonPane(nw4r::lyt::Pane *pane,
                            nw4rmanager::AnmObserver *observer)
        : AnmPane(pane, observer) {
        meKeyType = 2;
    }

    virtual void init() {
        meState = 0;
    }
    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~CellPhoneSignButtonPane() {
    }
};

class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *multiArc,
           const char *layoutBinName, EventObserver *eventObserver)
        : LayoutByNW4R(manager, multiArc, layoutBinName, eventObserver) {
    }
};

} // namespace signwindow
} // namespace keyboard
} // namespace textinput
