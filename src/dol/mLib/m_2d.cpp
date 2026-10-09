// Based on the Skyward Sword decompilation (zeldaret/ss), m/m2d.cpp
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/gfxe/eggScreen.h>
#include <nw4r/lyt.h>
#include <nw4r/ut.h>
#include <revolution/GX.h>
#include <revolution/MTX/mtx.h>
#include <revolution/SC/scapi.h>
#include <constants/sjis_constants.h>
#include <string.h>

namespace m2d {

// Draws a quad that only writes depth, to clear the Z buffer behind 2D layouts (RTTI
// "m2d::zBuffClear_c"). Member names unofficial.
class zBuffClear_c : public Base_c {
public:
    virtual ~zBuffClear_c() {}
    virtual void draw();

    void set(f32 top, f32 bottom, f32 left, f32 right, f32 z);

    /* 0x10 */ f32 mTop;
    /* 0x14 */ f32 mBottom;
    /* 0x18 */ f32 mLeft;
    /* 0x1C */ f32 mRight;
    /* 0x20 */ f32 mZ;
    /* 0x24 */ bool mAlphaTest;
};

nw4r::ut::List l_list;
mAllocator_c *l_allocator;

void reset() {
    nw4r::ut::List_Init(&l_list, 0);
}

bool create(EGG::Heap *parentHeap, size_t size) {
    nw4r::lyt::LytInit();
    EGG::ExpHeap *heap = EGG::ExpHeap::create(size, parentHeap, 4);
    heap->setName(M2D_HEAP_NAME);
    l_allocator = new (heap, 4) mAllocator_c();
    l_allocator->attach(heap, 4);
    nw4r::lyt::Layout::SetAllocator(l_allocator);
    reset();
    return true;
}

void defaultSet() {
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    static const GXColor l_clearColor = {0x00000000};
    GXSetFog(GX_FOG_NONE, l_clearColor, 0.0f, 0.0f, 0.0f, 0.0f);
}

void draw() {
    defaultSet();

    Base_c *next = (Base_c *)nw4r::ut::List_GetNext(&l_list, nullptr);
    while (next != nullptr) {
        next->draw();
        next = (Base_c *)nw4r::ut::List_GetNext(&l_list, next);
    }
    reset();
}

void drawBefore(u8 priority) {
    Base_c *next = (Base_c *)nw4r::ut::List_GetNext(&l_list, nullptr);
    while (next != nullptr) {
        if (next->mPriority >= priority) {
            break;
        }
        next->draw();
        next = (Base_c *)nw4r::ut::List_GetNext(&l_list, next);
    }
}

void drawAfter(u8 priority) {
    Base_c *next = (Base_c *)nw4r::ut::List_GetNext(&l_list, nullptr);
    while (next != nullptr) {
        if (next->mPriority > priority) {
            next->draw();
        }
        next = (Base_c *)nw4r::ut::List_GetNext(&l_list, next);
    }
}

mAllocator_c *getAllocator() {
    return l_allocator;
}

nw4r::lyt::AnimTransform *Layout_c::CreateAnimTransform(const void *animResBuf, nw4r::lyt::ResourceAccessor *pResAcsr) {
    nw4r::lyt::AnimTransform *transform = nw4r::lyt::Layout::CreateAnimTransform(animResBuf, pResAcsr);
    if (transform != nullptr) {
        mAnimTransList.Erase(transform);
    }
    return transform;
}

ResAcc_c::ResAcc_c() {
    creater();
}

ResAcc_c::~ResAcc_c() {}

bool ResAccIf_c::attach(void *resource, const char *rootDir) {
    if (mpResource != nullptr) {
        return false;
    }
    mpResource = resource;
    return mpResAccessor->Attach(mpResource, rootDir);
}

void ResAccIf_c::detach() {
    mpResource = nullptr;
    mpResAccessor->Detach();
}

void *ResAccIf_c::getResource(ulong type, const char *name) {
    return mpResAccessor->GetResource(type, name, nullptr);
}

FrameCtrl_c::FrameCtrl_c() : mEndFrame(0.0f), mCurrFrame(0.0f), mPrevFrame(0.0f), mRate(1.0f) {}

FrameCtrl_c::~FrameCtrl_c() {}

void FrameCtrl_c::play() {
    mPrevFrame = mCurrFrame;
    f32 newFrame;
    if (isBackwards()) {
        if (mCurrFrame >= mRate) {
            newFrame = mCurrFrame - mRate;
        } else if (!notLooping()) {
            newFrame = mCurrFrame + (mEndFrame - mRate);
        } else {
            newFrame = 0.0f;
        }
    } else {
        newFrame = mCurrFrame + mRate;
        if (!notLooping()) {
            if (newFrame >= mEndFrame) {
                newFrame = newFrame - mEndFrame;
            }
        } else {
            f32 forwardsEnd = mEndFrame - 1.0f;
            if (newFrame >= forwardsEnd) {
                newFrame = forwardsEnd;
            }
        }
    }
    mCurrFrame = newFrame;
}

void FrameCtrl_c::set(f32 endFrame, u8 flags, f32 rate, f32 currFrame) {
    if (currFrame < 0.0f) {
        currFrame = 0.0f;
    }
    mEndFrame = endFrame;
    mCurrFrame = currFrame;
    setRate(rate);
    mFlags = flags;
    mPrevFrame = mCurrFrame;
}

f32 FrameCtrl_c::getFrame() const {
    return mCurrFrame;
}

void FrameCtrl_c::setFrame(f32 frame) {
    mCurrFrame = frame;
    mPrevFrame = frame;
}

void FrameCtrl_c::setToEnd() {
    setFrame(mEndFrame - 1.0f);
}

void FrameCtrl_c::setRate(f32 rate) {
    mRate = rate;
}

bool FrameCtrl_c::isStop() const {
    switch (mFlags) {
        case NO_LOOP:
            return mCurrFrame >= mEndFrame - 1.0f;
        case NO_LOOP | REVERSE:
            return mCurrFrame == 0.0f;
        default:
            return false;
    }
}

Anm_c::Anm_c() : mpTransform(nullptr), mFlags(0) {}

Anm_c::~Anm_c() {
    remove();
}

bool Anm_c::create(const char *name, ResAccIf_c *resAcc) {
    if (mpTransform != nullptr) {
        remove();
    }

    static Layout_c l_layout;

    void *resource = resAcc->getResource(0, name);
    nw4r::lyt::AnimTransform *transform = l_layout.CreateAnimTransform(resource, resAcc->getResAccessor());
    if (transform == nullptr) {
        return false;
    }

    setTransform(transform);
    return true;
}

void Anm_c::setTransform(nw4r::lyt::AnimTransform *transform) {
    mpTransform = transform;
    u8 flags = FrameCtrl_c::NO_LOOP;
    if (mpTransform->IsLoopData()) {
        flags = 0;
    }
    set(mpTransform->GetFrameSize(), flags, 1.0f, -1.0f);
}

void Anm_c::remove() {
    if (mpTransform != nullptr) {
        mpTransform->~AnimTransform();
        nw4r::lyt::Layout::FreeMemory(mpTransform);
        mpTransform = nullptr;
    }
}

void Anm_c::set(f32 endFrame, u8 flags, f32 rate, f32 currFrame) {
    mFrameCtrl.set(endFrame, flags, rate, currFrame);
    updateFrame();
}

void Anm_c::setFrame(f32 frame) {
    mFrameCtrl.setFrame(frame);
    updateFrame();
}

void Anm_c::setToEnd() {
    mFrameCtrl.setToEnd();
    updateFrame();
}

void Anm_c::updateFrame() {
    mpTransform->SetFrame(mFrameCtrl.getFrame());
}

void Anm_c::play() {
    mFrameCtrl.play();
    updateFrame();
}

void Anm_c::changeTargetName(const char *oldName, const char *newName) {
    const nw4r::lyt::res::AnimationBlock *block = mpTransform->GetAnimResource();
    const u32 *offsets = (const u32 *)((u8 *)block + block->animContOffsetsOffset);
    for (u16 i = 0; i < block->animContNum; i++) {
        char *target = (char *)block + offsets[i];
        if (strncmp(target, oldName, 20) == 0) {
            strcpy(target, newName);
        }
    }
}

void Anm_c::patrolTargetName(patrolNameFunc func, void *arg) {
    const nw4r::lyt::res::AnimationBlock *block = mpTransform->GetAnimResource();
    const u32 *offsets = (const u32 *)((u8 *)block + block->animContOffsetsOffset);
    for (u16 i = 0; i < block->animContNum; i++) {
        func((char *)block + offsets[i], arg);
    }
}

Base_c::Base_c() : mPriority(0x80) {}

Base_c::~Base_c() {}

void Base_c::entry() {
    Base_c *next = (Base_c *)nw4r::ut::List_GetNext(&l_list, nullptr);
    while (next != nullptr) {
        if (next->mPriority > mPriority) {
            nw4r::ut::List_Insert(&l_list, next, this);
            return;
        }
        next = (Base_c *)nw4r::ut::List_GetNext(&l_list, next);
    }
    nw4r::ut::List_Append(&l_list, this);
}

Simple_c::Simple_c() : mpResAcc(nullptr), mPos(0.0f, 0.0f, 0.0f), mFlags(0) {
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        mDrawInfo.SetLocationAdjustScale(nw4r::math::VEC2(19.0f / 26.0f, 1.0f));
        mDrawInfo.SetLocationAdjust(true);
    }
}

Simple_c::~Simple_c() {}

void Simple_c::calc() {
    calcBefore();
    mMtx_c mtx;
    PSMTXIdentity(mtx);
    mVec3_c pos = mPos;
    PSMTXTransApply(mtx, mtx, pos.x, pos.y, pos.z);
    mDrawInfo.SetViewMtx(mtx);
    calcAfter();
}

void Simple_c::calcBefore() {
    u32 option = 0;
    if (mFlags & SKIP_INVISIBLE) {
        option = nw4r::lyt::ANIMOPTION_SKIP_INVISIBLE;
    }
    mLayout.Animate(option);
}

void Simple_c::calcAfter() {
    mDrawInfo.SetViewRect(mLayout.GetLayoutRect());
    mLayout.CalculateMtx(mDrawInfo);
}

void Simple_c::draw() {
    nw4r::ut::Rect rect = mLayout.GetLayoutRect();
    f32 near = 0.0f;
    f32 far = 500.0f;
    EGG::Screen screen;
    bool isWide = EGG::Screen::sTVMode == EGG::Screen::TV_MODE_16_9;
    f32 width_16_9 = EGG::Screen::sTVModeInfo[EGG::Screen::TV_MODE_16_9].width;
    f32 width_4_3 = EGG::Screen::sTVModeInfo[EGG::Screen::TV_MODE_4_3].width;
    f32 left = isWide ? width_16_9 * rect.left / width_4_3 : rect.left;
    f32 right = isWide ? width_16_9 * rect.right / width_4_3 : rect.right;
    screen.mProjType = EGG::Frustum::PROJ_ORTHO;
    screen.ResetOrthographic(rect.top, rect.bottom, left, right, near, far);
    if (isWide) {
        screen.mScale = nw4r::math::VEC3(width_4_3 / width_16_9, 1.0f, 1.0f);
    }
    screen.SetProjectionGX();
    mLayout.Draw(mDrawInfo);
}

nw4r::lyt::Pane *Simple_c::getRootPane() {
    return mLayout.GetRootPane();
}

bool Simple_c::build(const char *lytName, ResAccIf_c *resAcc) {
    if (mLayout.GetRootPane() != nullptr) {
        return true;
    }
    if (resAcc == nullptr) {
        resAcc = mpResAcc;
        if (resAcc == nullptr) {
            return false;
        }
    }
    void *res = resAcc->getResource(0, lytName);
    if (res == nullptr) {
        return false;
    }
    bool result = mLayout.Build(res, resAcc->getResAccessor());
    if (result) {
        calc();
    }
    return result;
}

int Simple_c::patrolPane_local(nw4r::lyt::Pane *pane, patrolPaneFunc1 func1, patrolPaneFunc2 func2, void *arg) {
    int ret = 0;
    if (func2 == nullptr || func2(pane) == 1) {
        if (ret = func1(pane, arg), ret != 0) {
            return ret;
        }
    }
    nw4r::lyt::PaneList &list = pane->GetChildList();
    for (nw4r::lyt::PaneList::RevIterator it = list.GetEndReverseIter(); it != list.GetBeginReverseIter(); ++it) {
        ret = patrolPane_local(&*it, func1, func2, arg);
        if (ret != 0) {
            return ret;
        }
    }
    return ret;
}

bool Simple_c::patrolPane(patrolPaneFunc1 func1, patrolPaneFunc2 func2, void *arg) {
    nw4r::lyt::Pane *pane = mLayout.GetRootPane();
    int ret = patrolPane_local(pane, func1, func2, arg);
    return ret != 2;
}

nw4r::lyt::Pane *Simple_c::findPane(const char *name) {
    return mLayout.GetRootPane()->FindPaneByName(name, true);
}

nw4r::lyt::TextBox *Simple_c::findTextBox(const char *name) {
    nw4r::lyt::Pane *pane = findPane(name);
    if (pane != nullptr) {
        return nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(pane);
    }
    return nullptr;
}

nw4r::lyt::Window *Simple_c::findWindow(const char *name) {
    nw4r::lyt::Pane *pane = findPane(name);
    if (pane != nullptr) {
        return nw4r::ut::DynamicCast<nw4r::lyt::Window *>(pane);
    }
    return nullptr;
}

bool Simple_c::createAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc) {
    if (resAcc == nullptr) {
        resAcc = mpResAcc;
    }
    void *res = resAcc->getResource(0, name);
    if (res == nullptr) {
        return false;
    }
    nw4r::lyt::AnimTransform *transform = mLayout.CreateAnimTransform(res, resAcc->getResAccessor());
    if (transform == nullptr) {
        return false;
    }
    anm->setTransform(transform);
    return true;
}

bool Simple_c::changeAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc) {
    if (!anm->isBound()) {
        anm->remove();
        return createAnm(anm, name, resAcc);
    }
    unbind(anm);
    anm->remove();
    bool ret = createAnm(anm, name, resAcc);
    bind(anm);
    return ret;
}

void Simple_c::bind(Anm_c *anm) {
    anm->setBound();
    mLayout.BindAnimation(anm->getTransform());
}

void Simple_c::unbind(Anm_c *anm) {
    anm->setUnbound();
    mLayout.UnbindAnimation(anm->getTransform());
}

static int setFontCB(nw4r::lyt::Pane *pane, void *font) {
    nw4r::lyt::TextBox *textBox = nw4r::ut::DynamicCast<nw4r::lyt::TextBox *>(pane);
    if (textBox == nullptr) {
        return 0;
    }
    nw4r::lyt::Size fontSize = textBox->GetFontSize();
    f32 charSpace = textBox->GetCharSpace();
    textBox->SetFont(static_cast<const nw4r::ut::Font *>(font));
    textBox->SetFontSize(fontSize);
    textBox->SetCharSpace(charSpace);
    return 0;
}

void Simple_c::setFont(const nw4r::ut::Font *font) {
    patrolPane(setFontCB, nullptr, (void *)font);
}

static void setupProjection() {
    Mtx mtx;
    PSMTXIdentity(mtx);
    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    EGG::Screen screen;
    bool isWide = EGG::Screen::sTVMode == EGG::Screen::TV_MODE_16_9;
    f32 width_16_9 = EGG::Screen::sTVModeInfo[EGG::Screen::TV_MODE_16_9].width;
    f32 width_4_3 = EGG::Screen::sTVModeInfo[EGG::Screen::TV_MODE_4_3].width;
    f32 left = isWide ? -304.0f * width_16_9 / width_4_3 : -304.0f;
    f32 right = isWide ? 304.0f * width_16_9 / width_4_3 : 304.0f;
    screen.mProjType = EGG::Frustum::PROJ_ORTHO;
    screen.ResetOrthographic(228.0f, -228.0f, left, right, 1.0f, 500.0f);
    if (isWide) {
        screen.mScale = nw4r::math::VEC3(width_4_3 / width_16_9, 1.0f, 1.0f);
    }
    screen.SetProjectionGX();
}

static void setupGX() {
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    static const GXColor l_ambColor = {0xFF, 0xFF, 0xFF, 0xFF};
    GXSetChanAmbColor(GX_COLOR0A0, l_ambColor);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    static const GXColor l_matColor = {0xFF, 0xFF, 0x00, 0xFF};
    GXSetChanMatColor(GX_COLOR0A0, l_matColor);
}

static void drawQuad(f32 top, f32 bottom, f32 left, f32 right, f32 z, bool alphaTest) {
    GXSetZCompLoc(GX_TRUE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    if (alphaTest) {
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    } else {
        GXSetAlphaCompare(GX_NEVER, 0, GX_AOP_AND, GX_GREATER, 0);
    }
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(left, top, z);
    GXPosition3f32(left, bottom, z);
    GXPosition3f32(right, bottom, z);
    GXPosition3f32(right, top, z);
    GXEnd();
}

void zBuffClear_c::draw() {
    setupProjection();
    setupGX();
    drawQuad(mTop, mBottom, mLeft, mRight, mZ, mAlphaTest);
    defaultSet();
}

void zBuffClear_c::set(f32 top, f32 bottom, f32 left, f32 right, f32 z) {
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
    mZ = z;
    mAlphaTest = false;
}

bool isVisible(const nw4r::lyt::Pane *pane) {
    while (pane != nullptr) {
        if (!pane->IsVisible()) {
            return false;
        }
        pane = pane->GetParent();
    }
    return true;
}

} // namespace m2d
