#pragma once

#include <types.h>
#include <revolution/mem.h>
#include <revolution/kpad.h>
#include <revolution/mtx.h>
#include <nw4r/lyt.h>
#include <stddef.h>

namespace textinput {
namespace gui {

class GUIComponent;
class GUIManager;
class EventHandler;

class GUIPointer {
public:
    GUIPointer(int point, f32 x, f32 y, u32 hold, u32 trig, u32 release)
        : mCtrlNo(point), mPosX(x), mPosY(y), mTrig(trig), mHold(hold), mRelease(release) {}

    int getCtrlNo() const { return mCtrlNo; }
    f32 getPosX() const { return mPosX; }
    f32 getPosY() const { return mPosY; }
    u32 isPressed(u32 button) const { return mHold & button; }

    /* 0x00 */ int mCtrlNo;
    /* 0x04 */ f32 mPosX;
    /* 0x08 */ f32 mPosY;
    /* 0x0C */ u32 mTrig;
    /* 0x10 */ u32 mHold;
    /* 0x14 */ u32 mRelease;
}; // size 0x18

class GUIInterface {
public:
    virtual ~GUIInterface() {}      // 0x08
    virtual void create() {}        // 0x0C
    virtual void init() {}          // 0x10
    virtual void calc() {}          // 0x14
    virtual void draw(Mtx &mtx) {}  // 0x18
    virtual void draw() {}          // 0x1C
};

class EventHandler {
public:
    enum {
        ON_POINT,
        ON_LEFT,
        ON_MOVE,
        ON_DRAG,
        ON_TRIG,
        ON_RELEASE,
    };

    EventHandler() : muLatestEventCtrlNo(0) {}

    virtual ~EventHandler() {}                                                       // 0x08
    virtual void onEvent(GUIComponent &component, u32 event, void *data) {}          // 0x0C
    virtual void setLatestEventCtrlNo(int ctrlNo) { muLatestEventCtrlNo = ctrlNo; }  // 0x10
    virtual int getLatestEventCtrlNo() { return muLatestEventCtrlNo; }               // 0x14

protected:
    /* 0x04 */ int muLatestEventCtrlNo;
}; // size 0x8

class GUIComponent : public GUIInterface {
public:
    enum {
        POINTS_MAX = 8,
    };

    enum {
        EVENT_NONE,
        EVENT_POINT,
        EVENT_LEFT,
        EVENT_MOVE,
        EVENT_TRIG,
        EVENT_RELEASE,
    };

    GUIComponent(u32 id)
        : mbInitialize(false), mDraggingButton(0xFFFF), mID(id), mbTriggerTarget(false), mpManager(NULL),
          mpListener(NULL) {
        init();
    }

    virtual ~GUIComponent() {} // 0x08
    virtual void init() {      // 0x10
        if (mbInitialize) {
            return;
        }
        for (int i = 0; i < POINTS_MAX; i++) {
            mbPointed[i] = false;
            mDraggingPos[i].z = mDraggingPos[i].y = mDraggingPos[i].x = 0.0f;
            mbDragging[i] = false;
            mFlightDuration[i] = 0;
        }
    }
    virtual u32 getID() { return mID; }                                             // 0x20
    virtual bool isPointed(int point) { return mbPointed[point]; }                  // 0x24
    virtual bool isDragging(int point) { return mbDragging[point]; }                // 0x28
    virtual void setPointed(int point, bool enable) { mbPointed[point] = enable; }  // 0x2C
    virtual void onPointIn(int point) { mFlightDuration[point] = 0; }               // 0x30
    virtual void onPointOut(int point) { mFlightDuration[point] = 0; }              // 0x34
    virtual void onDrag(f32 x, f32 y) {}                                            // 0x38
    virtual void onMove(int point, f32 x, f32 y) { mFlightDuration[point]++; }      // 0x3C
    virtual void onTrig(int point, u32 trig, Vec &dragPos) {                        // 0x40
        if (trig & mDraggingButton) {
            mDraggingPos[point] = dragPos;
            mbDragging[point] = true;
            mFlightDuration[point] = 0;
        }
    }
    virtual void setDraggingButton(u32 button) { mDraggingButton = button; }                      // 0x44
    virtual u32 updatePointer(const GUIPointer &pointer) { return updatePointerImpl(pointer); }  // 0x48
    virtual u32 updatePointer(int point, f32 x, f32 y, u32 trig, u32 hold, u32 release) {        // 0x4C
        GUIPointer pointer(point, x, y, hold, trig, release);
        return updatePointerImpl(pointer);
    }
    virtual bool isTriggerTarget() { return mbTriggerTarget; }                // 0x50
    virtual void setTriggerTarget(bool enable) { mbTriggerTarget = enable; }  // 0x54
    virtual bool isVisible() { return true; }                                 // 0x58
    virtual u16 getFlightDuration(int point) { return mFlightDuration[point]; }                       // 0x5C
    virtual void setFlightDuration(int point, u16 duration) { mFlightDuration[point] = duration; }   // 0x60
    virtual bool contain(f32 x, f32 y) = 0;                                                         // 0x64

    void onEvent(u32 event, void *data) {
        if (mpListener) {
            mpListener->onEvent(*this, event, data);
        }
    }
    void setParentManager(GUIManager *manager) { mpManager = manager; }
    GUIManager *getParentManager() { return mpManager; }
    void setListener(EventHandler *listener) { mpListener = listener; }
    void lockInitialize(bool lock) { mbInitialize = lock; }

protected:
    u32 updatePointerImpl(const GUIPointer &pointer);

    /* 0x04 */ bool mbInitialize;
    /* 0x05 */ bool mbPointed[POINTS_MAX];
    /* 0x0D */ bool mbDragging[POINTS_MAX];
    /* 0x18 */ Vec mDraggingPos[POINTS_MAX];
    /* 0x78 */ int mDraggingButton;
    /* 0x7C */ int mID;
    /* 0x80 */ u16 mFlightDuration[POINTS_MAX];
    /* 0x90 */ bool mbTriggerTarget;
    /* 0x94 */ GUIManager *mpManager;
    /* 0x98 */ EventHandler *mpListener;
}; // size 0x9C

class PaneComponent;
class PaneManager;

class GUIManager : public GUIInterface {
public:
    GUIManager(EventHandler *eventHandler, MEMAllocator *allocator)
        : mpEventHandler(eventHandler), mpAllocator(allocator) {
        nw4r::ut::List_Init(&mComponents, offsetof(IDToComponent, mLink));
    }

    virtual ~GUIManager() DECOMP_DONT_INLINE;                                          // 0x08
    virtual void init();                                                               // 0x10
    virtual void calc();                                                               // 0x14
    virtual void draw();                                                               // 0x1C
    virtual void addComponent(GUIComponent *component);                                // 0x20
    virtual GUIComponent *getComponent(u32 id);                                        // 0x24
    virtual GUIComponent *getComponent(u32 id) const;                                  // 0x28
    virtual bool update(int point, const KPADStatus *kpad, f32 x, f32 y, void *data) { // 0x2C
        return false;
    }
    virtual bool update(int point, f32 x, f32 y, u32 trig, u32 hold, u32 release, void *data); // 0x30
    virtual void onEvent(GUIComponent &component, u32 event, int point, void *data) {  // 0x34
        if (mpEventHandler) {
            mpEventHandler->setLatestEventCtrlNo(point);
            mpEventHandler->onEvent(component, event, data);
        }
        component.onEvent(event, data);
    }
    virtual void setAllComponentTriggerTarget(bool enable);                            // 0x38
    virtual void setEventHandler(EventHandler *eventHandler) {                         // 0x3C
        mpEventHandler = eventHandler;
    }
    virtual EventHandler *changeEventHandler(EventHandler *eventHandler) {             // 0x40
        EventHandler *prev = mpEventHandler;
        mpEventHandler = eventHandler;
        return prev;
    }
    virtual void setDraggingButton(u32 button);                                        // 0x44

protected:
    class IDToComponent {
    public:
        IDToComponent(u32 id, GUIComponent *component) : mID(id), mpComponent(component) {}

        /* 0x00 */ u32 mID;
        /* 0x04 */ GUIComponent *mpComponent;
        /* 0x08 */ nw4r::ut::Link mLink;
    }; // size 0x10

    /* 0x04 */ EventHandler *mpEventHandler;
    /* 0x08 */ MEMAllocator *mpAllocator;
    /* 0x0C */ nw4r::ut::List mComponents;
}; // size 0x18

class PaneManager : public GUIManager {
public:
    PaneManager(EventHandler *eventHandler, MEMAllocator *allocator, const nw4r::lyt::DrawInfo *drawInfo)
        : GUIManager(eventHandler, allocator), mpDrawInfo(drawInfo) {
        mIDCounter = 0;
        nw4r::ut::List_Init(&mPaneComponents, offsetof(PaneToComponent, mLink));
    }

    virtual ~PaneManager();                                                  // 0x08
    virtual void createLayoutScene(const nw4r::lyt::Layout &layout);         // 0x48
    virtual PaneComponent *getPaneComponentByPane(nw4r::lyt::Pane *pane);    // 0x4C
    virtual const nw4r::lyt::DrawInfo *getDrawInfo() { return mpDrawInfo; } // 0x50
    virtual void setDrawInfo(const nw4r::lyt::DrawInfo *drawInfo) {          // 0x54
        mpDrawInfo = drawInfo;
    }
    virtual void setAllBoundingBoxComponentTriggerTarget(bool enable);       // 0x58

    void walkInChildren(nw4r::lyt::PaneList &paneList);
    void addPane_(nw4r::lyt::Pane *pane);
    PaneComponent *searchPaneComponent(const char *name);

protected:
    class PaneToComponent {
    public:
        PaneToComponent(nw4r::lyt::Pane *pane, PaneComponent *component) : mpPane(pane), mpComponent(component) {}

        nw4r::lyt::Pane *getPane() { return mpPane; }

        /* 0x00 */ nw4r::lyt::Pane *mpPane;
        /* 0x04 */ PaneComponent *mpComponent;
        /* 0x08 */ nw4r::ut::Link mLink;
    }; // size 0x10

    /* 0x18 */ const nw4r::lyt::DrawInfo *mpDrawInfo;
    /* 0x1C */ u16 mNumPoint;
    /* 0x20 */ u32 mIDCounter;
    /* 0x24 */ nw4r::ut::List mPaneComponents;
}; // size 0x30

class PaneComponent : public GUIComponent {
public:
    PaneComponent(u32 id) : GUIComponent(id) { mpPane = NULL; }

    virtual ~PaneComponent() {}                                    // 0x08
    virtual void draw();                                           // 0x1C
    virtual bool isVisible();                                      // 0x58
    virtual bool contain(f32 x, f32 y);                            // 0x64
    virtual void setPane(nw4r::lyt::Pane *pane) { mpPane = pane; } // 0x68

    nw4r::lyt::Pane *getPane() { return mpPane; }
    nw4r::lyt::TextBox *getTextPane() { return static_cast<nw4r::lyt::TextBox *>(mpPane); }
    nw4r::lyt::Picture *getPictPane() { return static_cast<nw4r::lyt::Picture *>(mpPane); }
    nw4r::lyt::Bounding *getBoundPane() { return static_cast<nw4r::lyt::Bounding *>(mpPane); }

private:
    /* 0x9C */ nw4r::lyt::Pane *mpPane;
}; // size 0xA0

void drawLine_(f32 x0, f32 y0, f32 x1, f32 y1, f32 z, u8 width, GXColor &color);

} // namespace gui
} // namespace textinput
