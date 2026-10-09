#pragma once

// m2d: the 2D layout library. Based on the Skyward Sword decompilation (zeldaret/ss), m/m2d.h.
// One header, like SS's m2d.h: the class order below sets the vtable order of m_2d.cpp, and with
// -sym on all inline functions of this file share one trailing .text section. (m2d::zBuffClear_c is
// only used by m_2d.cpp and is declared there.)
#include <types.h>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/lyt.h>
#include <nw4r/ut.h>

namespace m2d {

void reset();
bool create(EGG::Heap *heap, size_t size);
void defaultSet();
void draw();
void drawBefore(u8 priority);
void drawAfter(u8 priority);
mAllocator_c *getAllocator();
// True when the pane and all its parents are visible. Name unofficial.
bool isVisible(const nw4r::lyt::Pane *pane);

extern nw4r::ut::List l_list;
extern mAllocator_c *l_allocator;

class Layout_c : public nw4r::lyt::Layout {
public:
    virtual nw4r::lyt::AnimTransform *CreateAnimTransform(const void *animResBuf, nw4r::lyt::ResourceAccessor *pResAcsr);
};

class ResAccIf_c {
public:
    ResAccIf_c() : mpResAccessor(nullptr), mpResource(nullptr) {}
    virtual ~ResAccIf_c() {}
    virtual void creater() = 0;

    bool attach(void *resource, const char *rootDir);
    void detach();
    void *getResource(ulong type, const char *name);

    void setResAccessor(nw4r::lyt::ArcResourceAccessor *resAccessor) { mpResAccessor = resAccessor; }
    nw4r::lyt::ArcResourceAccessor *getResAccessor() const { return mpResAccessor; }

private:
    /* 0x04 */ nw4r::lyt::ArcResourceAccessor *mpResAccessor;
    /* 0x08 */ void *mpResource;
};

// A resource accessor interface with its own ArcResourceAccessor (RTTI "m2d::ResAcc_c").
class ResAcc_c : public ResAccIf_c {
public:
    ResAcc_c();
    virtual ~ResAcc_c();
    virtual void creater() { setResAccessor(&mAccessor); }

private:
    /* 0x0C */ nw4r::lyt::ArcResourceAccessor mAccessor;
};

class FrameCtrl_c {
public:
    enum ANM_FLAG_e {
        NO_LOOP = BIT_FLAG(0),
        REVERSE = BIT_FLAG(1),
    };

    FrameCtrl_c();
    virtual ~FrameCtrl_c();

    void play();
    void set(f32 endFrame, u8 flags, f32 rate, f32 currFrame);
    f32 getFrame() const;
    void setFrame(f32 frame);
    void setToEnd();
    void setRate(f32 rate);
    bool isStop() const;

    f32 getLastFrame() const { return mEndFrame - 1.0f; }
    bool notLooping() const { return (mFlags & NO_LOOP) != 0; }
    bool isBackwards() const { return (mFlags & REVERSE) != 0; }

    /* 0x04 */ f32 mEndFrame;
    /* 0x08 */ f32 mCurrFrame;
    /* 0x0C */ f32 mPrevFrame;
    /* 0x10 */ f32 mRate;
    /* 0x14 */ u8 mFlags;
};

// A layout animation (RTTI "m2d::Anm_c"). Member names partly from Skyward Sword, partly unofficial.
class Anm_c {
public:
    typedef void (*patrolNameFunc)(char *name, void *arg);

    Anm_c();
    virtual ~Anm_c();

    bool create(const char *name, ResAccIf_c *resAcc);
    void setTransform(nw4r::lyt::AnimTransform *transform);
    void remove();
    void set(f32 endFrame, u8 flags, f32 rate, f32 currFrame);
    void setFrame(f32 frame);
    void setToEnd();
    void updateFrame();
    void play();
    // Renames the animation targets called oldName (names unofficial).
    void changeTargetName(const char *oldName, const char *newName);
    // Calls func for the name of every animation target (names unofficial).
    void patrolTargetName(patrolNameFunc func, void *arg);

    nw4r::lyt::AnimTransform *getTransform() const { return mpTransform; }
    bool isBound() const { return mFlags & 1; }
    void setBound() { mFlags |= 1; }
    void setUnbound() { mFlags &= ~1; }

    /* 0x04 */ nw4r::lyt::AnimTransform *mpTransform;
    /* 0x08 */ FrameCtrl_c mFrameCtrl;
    /* 0x20 */ u8 mFlags;
};

class Base_c : public nw4r::ut::Link {
public:
    Base_c();
    virtual ~Base_c();
    virtual void draw() {}

    void entry();

    /* 0x0C */ u8 mPriority;
};

class Simple_c : public Base_c {
public:
    enum FLAG_e {
        SKIP_INVISIBLE = BIT_FLAG(0),
    };

    typedef int (*patrolPaneFunc1)(nw4r::lyt::Pane *pane, void *arg);
    typedef bool (*patrolPaneFunc2)(nw4r::lyt::Pane *pane);

    Simple_c();
    virtual ~Simple_c();
    virtual void draw();
    virtual void calc();
    virtual bool build(const char *lytName, ResAccIf_c *resAcc);

    void calcBefore();
    void calcAfter();
    nw4r::lyt::Pane *getRootPane();
    int patrolPane_local(nw4r::lyt::Pane *pane, patrolPaneFunc1 func1, patrolPaneFunc2 func2, void *arg);
    bool patrolPane(patrolPaneFunc1 func1, patrolPaneFunc2 func2, void *arg);
    nw4r::lyt::Pane *findPane(const char *name);
    nw4r::lyt::TextBox *findTextBox(const char *name);
    nw4r::lyt::Window *findWindow(const char *name);
    // Loads an animation for this layout (names unofficial).
    bool createAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc);
    // Reloads an animation, keeping it bound if it was (names unofficial).
    bool changeAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc);
    void bind(Anm_c *anm);
    void unbind(Anm_c *anm);
    // Sets the font of every text box, keeping their font size and character spacing (unofficial).
    void setFont(const nw4r::ut::Font *font);

    /* 0x10 */ Layout_c mLayout;
    /* 0x30 */ nw4r::lyt::DrawInfo mDrawInfo;
    /* 0x84 */ ResAccIf_c *mpResAcc;
    /* 0x88 */ mVec3_c mPos;
    /* 0x94 */ u32 mFlags;
};

} // namespace m2d
