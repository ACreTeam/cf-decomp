#pragma once

// textinput::nw4rmanager: the NW4R layout of a keyboard part (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <lib/textinput/tiLayout.h>
#include <lib/textinput/tiGUIManager.h>
#include <nw4r/lyt.h>

namespace textinput {

class EventObserver;

namespace input {
class HKBManager;
}

namespace gui {
class PaneManager;
}

class AnimTransformPane;

namespace nw4rmanager {

class TiEventHandler : public gui::EventHandler {
public:
    struct Input {
        /* 0x00 */ int ctrlNo;
        /* 0x04 */ f32 x;
        /* 0x08 */ f32 y;
        /* 0x0C */ u32 trig;
        /* 0x10 */ u32 hold;
        /* 0x14 */ u32 release;
    }; // size 0x18

    TiEventHandler() {}

    virtual void onEvent(gui::GUIComponent &component, u32 event, void *data);                  // 0x0C
    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event, Input *input) = 0;     // 0x18
    virtual void setEventObserver(EventObserver *eventObserver) { mpEventObserver = eventObserver; } // 0x1C

protected:
    /* 0x08 */ EventObserver *mpEventObserver;
}; // size 0xC

class AnmPane;

class Anim {
public:
    Anim(u32 id, nw4r::lyt::AnimTransform *animTransform, bool loop, bool noOff)
        : mpAnimTransform(animTransform), muID(id), mfCurrentFrame(0.0f), mbLoop(loop), mbNoOff(noOff) {}

    /* 0x00 */ nw4r::lyt::AnimTransform *mpAnimTransform;
    /* 0x04 */ u32 muID;
    /* 0x08 */ f32 mfCurrentFrame;
    /* 0x0C */ bool mbLoop;
    /* 0x0D */ bool mbNoOff;
    /* 0x10 */ nw4r::ut::Link mLink;
}; // size 0x18

class AnmObserver {
public:
    enum AnmEvent {
        E_0,
        E_1,
        E_Last,
    };

    virtual void onChangeAnmState(AnmEvent anmEvent, AnmPane *anmPane, Anim *anim) = 0; // 0x08
};

class AnmPane {
public:
    enum AnmPaneEvent {
        PE_0,
        PE_1,
        PE_2,
        PE_3,
        PE_4,
        PE_5,
        PE_6,
        PE_7,
        PE_8,
        PE_9,
        PE_10,
        PE_11,
        PE_Last,
    };

    AnmPane(nw4r::lyt::Pane *pane, AnmObserver *observer) : mpPane(pane), mpCurrentAnim(NULL), mpObserver(observer) {
        nw4r::ut::List_Init(&mAnms, offsetof(Anim, mLink));
    }

    virtual void init() {}                               // 0x08
    virtual void calc();                                 // 0x0C
    virtual void onAnmEvent(AnmPaneEvent paneEvent) {}   // 0x10
    virtual void changeAnimation(u32 id);                // 0x14
    virtual bool isInAnimation() { return mpCurrentAnim != NULL; } // 0x18
    virtual Anim *searchAnimation(u32 id) {              // 0x1C
        for (Anim *anim = (Anim *)nw4r::ut::List_GetNext(&mAnms, NULL); anim != NULL;
             anim = (Anim *)nw4r::ut::List_GetNext(&mAnms, anim)) {
            if (anim->muID == id) {
                return anim;
            }
        }
        return NULL;
    }
    virtual ~AnmPane();                                  // 0x20

    struct AnmState {
        s32 currentAnim;
        f32 currentFrame;
    };

    nw4r::lyt::Pane *getPane() { return mpPane; }
    void storeAnmState(AnmState *state);
    void restoreAnmState(const AnmState *state);
    void addAnimation(MEMAllocator *allocator, u32 id, AnimTransformPane *animTransformPane, bool loop, bool noOff);
    void forceAddAnimation(MEMAllocator *allocator, u32 id, AnimTransformPane *animTransformPane, const char *paneName,
                           bool loop, bool noOff);
    void destroy(MEMAllocator *allocator);

protected:
    /* 0x04 */ nw4r::lyt::Pane *mpPane;
    /* 0x08 */ nw4r::ut::List mAnms;
    /* 0x14 */ Anim *mpCurrentAnim;
    /* 0x18 */ AnmObserver *mpObserver;

public:
    /* 0x1C */ nw4r::ut::Link mLink;
    /* 0x24 */ nw4r::ut::Link mGroupLink;
}; // size 0x2C

class AnimPaneGroup {
  public:
    AnimPaneGroup(nw4r::lyt::Group *group) : mpGroup(group) {
        nw4r::ut::List_Init(&mAnmPaneList, offsetof(AnmPane, mGroupLink));
    }

    virtual void create() {
    }
    virtual void init() {
    }

    virtual void calc();

    virtual void addAnimPane(AnmPane *anmPane) {
        nw4r::ut::List_Append(&mAnmPaneList, anmPane);
    }

    virtual void changeAnimation(u32 id);

  private:
    nw4r::lyt::Group *mpGroup;
    nw4r::ut::List mAnmPaneList;
};

class PaneFifo {
  private:
    static const int MAX_COUNT = 5;

  public:
    PaneFifo() {
        init();
    }

    virtual void init() {
        for (int i = 0; i < MAX_COUNT; i++) {
            mpaFifo[i] = NULL;
        }
    }

    virtual void push(nw4r::lyt::Pane *pane) {
        if (mpaFifo[MAX_COUNT - 1] == NULL) {
            for (int i = 0; i < MAX_COUNT; i++) {
                if (mpaFifo[i] == NULL) {
                    mpaFifo[i] = pane;
                    return;
                }
            }
        } else {
            for (int i = 1; i < MAX_COUNT; i++) {
                mpaFifo[i - 1] = mpaFifo[i];
                if (pane == mpaFifo[i - 1]) {
                    mpaFifo[i - 1] = NULL;
                }
            }
            mpaFifo[MAX_COUNT - 1] = pane;
        }
    }

    virtual nw4r::lyt::Pane *get(int i) {
        return mpaFifo[i];
    }
    virtual nw4r::lyt::Pane *getLast() {
        for (int i = MAX_COUNT - 1; i > -1; i--) {
            if (mpaFifo[i] != NULL) {
                return mpaFifo[i];
            }
        }
        return NULL;
    }

    virtual int getSize() {
        return MAX_COUNT;
    }

  private:
    nw4r::lyt::Pane *mpaFifo[MAX_COUNT];
};

class Layout {
public:
    Layout(nw4r::lyt::MultiArcResourceAccessor *multiArc, const char *layoutBinName, EventObserver *eventObserver);

    virtual ~Layout();                                                                                  // 0x08
    virtual void createWithEventHandler(MEMAllocator *allocator, TiEventHandler *eventHandler);         // 0x0C
    virtual void init();                                                                                // 0x10
    virtual void initPaneManager();                                                                     // 0x14
    virtual void draw();                                                                                // 0x18
    virtual void calc();                                                                                // 0x1C
    virtual bool updateInput(int point, f32 x, f32 y, u32 trig, u32 hold, u32 release, void *data);     // 0x20
    virtual bool updateInput(input::HKBManager &hkbManager);                                            // 0x24
    virtual TiLayout *getLayout() {
        return mpLayout;
    } // 0x28
    virtual nw4r::lyt::Pane *getPane(const char *paneName);                                             // 0x2C
    virtual u32 getFlightDuration(int point, const char *paneName);                                     // 0x30
    virtual void setAnimOn(bool flag) {
        mbAnimOn = flag;
    } // 0x34
    virtual void drawPane(const char *paneName);                                                        // 0x38
    virtual void setRootPaneScaleFor16x9();                                                             // 0x3C
    virtual void setRootPaneScaleFor4x3();                                                              // 0x40
    virtual void setProjectionMtx();                                                                    // 0x44
    virtual void setPaneLastDrawReceived(nw4r::lyt::Pane *paneLastDraw) {
        mAnmPaneFifo.push(paneLastDraw);
    } // 0x48
    virtual void initPaneLastDrawReceived() {
        mAnmPaneFifo.init();
    } // 0x4C
    virtual void setVisible(const char *paneName, bool flag);                                           // 0x50
    virtual bool isVisible(const char *paneName, bool *result) const;                                   // 0x54
    virtual void setString(const char *paneName, const wchar_t *wcString);                              // 0x58
    virtual AnmPane *searchAnmPane(const char *paneName);                                               // 0x5C
    virtual AnmPane *searchAnmPane(wchar_t ch);                                                         // 0x60
    virtual nw4r::ut::List &getAnmPaneList() {
        return mAnmPanes;
    } // 0x64
    virtual gui::PaneManager *getPaneManager() {
        return mpPaneManager;
    } // 0x68
    virtual void AdjustPaneMtx(Mtx &mtx, const nw4r::lyt::DrawInfo &drawInfo, const nw4r::math::MTX34 &globalMtx); // 0x6C
    virtual gui::PaneManager *createPaneManager(MEMAllocator *allocator, TiEventHandler *eventHandler); // 0x70

protected:
  /* 0x04 */ TiLayout *mpLayout;
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
