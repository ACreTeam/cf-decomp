#pragma once

// textinput::nw4rmanager: the NW4R layout of a keyboard part (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <nw4r/lyt.h>

namespace textinput {

class EventObserver;

namespace input {
class HKBManager;
}

namespace gui {
class PaneManager;
}

namespace nw4rmanager {

class TiEventHandler;
class AnmPane;
class Anim;

class AnmObserver {
public:
    enum AnmEvent {
        E_0,
        E_1,
        E_Last,
    };

    virtual void onChangeAnmState(AnmEvent anmEvent, AnmPane *anmPane, Anim *anim) = 0; // 0x08
};

class PaneFifo {
public:
    virtual void init();                         // 0x08
    virtual void push(nw4r::lyt::Pane *pane);    // 0x0C
    virtual nw4r::lyt::Pane *get(int i);         // 0x10
    virtual nw4r::lyt::Pane *getLast();          // 0x14
    virtual int getSize();                       // 0x18

private:
    /* 0x04 */ nw4r::lyt::Pane *mpaFifo[5];
}; // size 0x18

class Layout {
public:
    virtual ~Layout();                                                                                  // 0x08
    virtual void createWithEventHandler(MEMAllocator *allocator, TiEventHandler *eventHandler);         // 0x0C
    virtual void init();                                                                                // 0x10
    virtual void initPaneManager();                                                                     // 0x14
    virtual void draw();                                                                                // 0x18
    virtual void calc();                                                                                // 0x1C
    virtual bool updateInput(int point, f32 x, f32 y, u32 trig, u32 hold, u32 release, void *data);     // 0x20
    virtual bool updateInput(input::HKBManager &hkbManager);                                            // 0x24
    virtual nw4r::lyt::Layout *getLayout() { return mpLayout; }                                         // 0x28
    virtual nw4r::lyt::Pane *getPane(const char *paneName);                                             // 0x2C
    virtual u32 getFlightDuration(int point, const char *paneName);                                     // 0x30
    virtual void setAnimOn(bool flag);                                                                  // 0x34
    virtual void drawPane(const char *paneName);                                                        // 0x38
    virtual void setRootPaneScaleFor16x9();                                                             // 0x3C
    virtual void setRootPaneScaleFor4x3();                                                              // 0x40
    virtual void setProjectionMtx();                                                                    // 0x44
    virtual void setPaneLastDrawReceived(nw4r::lyt::Pane *paneLastDraw);                                // 0x48
    virtual void initPaneLastDrawReceived();                                                            // 0x4C
    virtual void setVisible(const char *paneName, bool flag);                                           // 0x50
    virtual bool isVisible(const char *paneName, bool *result) const;                                   // 0x54
    virtual void setString(const char *paneName, const wchar_t *wcString);                              // 0x58
    virtual AnmPane *searchAnmPane(const char *paneName);                                               // 0x5C
    virtual AnmPane *searchAnmPane(wchar_t ch);                                                         // 0x60
    virtual nw4r::ut::List &getAnmPaneList();                                                           // 0x64
    virtual gui::PaneManager *getPaneManager();                                                         // 0x68
    virtual void AdjustPaneMtx(Mtx &mtx, const nw4r::lyt::DrawInfo &drawInfo, const nw4r::math::MTX34 &globalMtx); // 0x6C
    virtual gui::PaneManager *createPaneManager(MEMAllocator *allocator, TiEventHandler *eventHandler); // 0x70

protected:
    /* 0x04 */ nw4r::lyt::Layout *mpLayout;
    /* 0x08 */ const char *mpszLayoutBinName;
    /* 0x0C */ nw4r::lyt::MultiArcResourceAccessor *mpMultiArcResourceAccessor;
    /* 0x10 */ gui::PaneManager *mpPaneManager;
    /* 0x14 */ EventObserver *mpEventObserver;
    /* 0x18 */ nw4r::lyt::DrawInfo mDrawInfo;
    /* 0x6C */ nw4r::ut::List mAnmPanes;
    /* 0x78 */ nw4r::ut::Rect mScreenRect;
    /* 0x88 */ bool mbAnimOn;
    /* 0x89 */ bool mbAspectRatio4x3;
    /* 0x8C */ PaneFifo mAnmPaneFifo;
    /* 0xA4 */ MEMAllocator *mpNw4rAllocator;
}; // size 0xA8

} // namespace nw4rmanager
} // namespace textinput
