#include <lib/textinput/tiGUIManager.h>
#include <lib/textinput/tiUtil.h>

#include <revolution/gx.h>

#include <new>

namespace textinput {
namespace gui {

void drawLine_(f32 x0, f32 y0, f32 x1, f32 y1, f32 z, u8 width, GXColor &color) {
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GXSetCullMode(GX_CULL_NONE);

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);

    GXSetNumTexGens(0);
    GXSetNumTevStages(1);

    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);

    Mtx mtx;
    MTXTrans(mtx, 0.0f, 0.0f, 0.0f);
    GXLoadPosMtxImm(mtx, 0);

    GXSetLineWidth(width, GX_TO_ZERO);

    GXBegin(GX_LINES, GX_VTXFMT0, 2);
    {
        GXPosition3f32(x0, y0, z);
        GXColor1u32(*(u32 *)&color);
        GXPosition3f32(x1, y1, z);
        GXColor1u32(*(u32 *)&color);
    }
    GXEnd();
}

u32 GUIComponent::updatePointerImpl(const GUIPointer &pointer) {
    int point = pointer.getCtrlNo();
    u32 event = EVENT_NONE;
    f32 x = pointer.getPosX();
    f32 y = pointer.getPosY();

    if (isVisible()) {
        if (contain(x, y)) {
            if (isPointed(point)) {
                onMove(point, x, y);
                event = EVENT_MOVE;
            } else {
                setPointed(point, true);
                onPointIn(point);
                event = EVENT_POINT;
            }
        } else {
            if (isPointed(point)) {
                setPointed(point, false);
                onPointOut(point);
                event = EVENT_LEFT;
            }
        }

        if (mbDragging[point]) {
            onDrag(x - mDraggingPos[point].x, y - mDraggingPos[point].y);
            mDraggingPos[point].x = x;
            mDraggingPos[point].y = y;
        }
    }

    if (mbDragging[point] && pointer.isPressed(mDraggingButton) == 0) {
        mbDragging[point] = false;
    }

    return event;
}

GUIManager::~GUIManager() {
    IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents));
    while (p != NULL) {
        nw4r::ut::List_Remove(&mComponents, p);
        if (mpAllocator) {
            MEMFreeToAllocator(mpAllocator, p);
        } else {
            delete p;
        }
        p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents));
    }
}

void GUIManager::init() {
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        p->mpComponent->init();
    }
}

void GUIManager::addComponent(GUIComponent *component) {
    int id = component->getID();

    component->setParentManager(this);
    if (mpAllocator) {
        void *buf = MEMAllocFromAllocator(mpAllocator, sizeof(IDToComponent));
        IDToComponent *p = new (buf) IDToComponent(id, component);
        nw4r::ut::List_Append(&mComponents, p);
    } else {
        IDToComponent *p = new IDToComponent(id, component);
        nw4r::ut::List_Append(&mComponents, p);
    }
}

GUIComponent *GUIManager::getComponent(u32 id) {
    IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetNth(&mComponents, (u16)id));
    return p->mpComponent;
}

GUIComponent *GUIManager::getComponent(u32 id) const {
    IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetNth(&mComponents, (u16)id));
    return p->mpComponent;
}

bool GUIManager::update(int point, f32 x, f32 y, u32 trig, u32 hold, u32 release, void *data) {
    bool touched = false;

    GUIComponent *triggerCom = NULL;
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        GUIComponent *component = p->mpComponent;
        u32 result = p->mpComponent->updatePointer(point, x, y, trig, hold, release);

        switch (result) {
        case GUIComponent::EVENT_POINT: {
            onEvent(*component, EventHandler::ON_POINT, point, data);
            goto hasTouched;
        }
        case GUIComponent::EVENT_LEFT: {
            onEvent(*component, EventHandler::ON_LEFT, point, data);
            break;
        }
        case GUIComponent::EVENT_MOVE: {
            onEvent(*component, EventHandler::ON_MOVE, point, data);
            goto hasTouched;
        }
        case GUIComponent::EVENT_TRIG: {
            onEvent(*component, EventHandler::ON_TRIG, point, data);
            break;
        }
        case GUIComponent::EVENT_RELEASE: {
            onEvent(*component, EventHandler::ON_RELEASE, point, data);
            break;
        }

        hasTouched:
            touched = true;
            if (component->isTriggerTarget()) {
                triggerCom = component;
            }
        }

        if (component->isDragging(point)) {
            onEvent(*component, EventHandler::ON_DRAG, point, data);
        }
    }

    if (triggerCom) {
        if (trig) {
            Vec pos = {x, y, 0.0f};
            triggerCom->onTrig(point, trig, pos);
            onEvent(*triggerCom, EventHandler::ON_TRIG, point, data);
        }
        if (release) {
            onEvent(*triggerCom, EventHandler::ON_RELEASE, point, data);
        }
    }

    return touched;
}

void GUIManager::calc() {
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        p->mpComponent->calc();
    }
}

void GUIManager::draw() {
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        p->mpComponent->draw();
    }
}

void GUIManager::setAllComponentTriggerTarget(bool enable) {
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        p->mpComponent->setTriggerTarget(enable);
    }
}

void GUIManager::setDraggingButton(u32 button) {
    for (IDToComponent *p = static_cast<IDToComponent *>(nw4r::ut::List_GetFirst(&mComponents)); p != NULL;
         p = static_cast<IDToComponent *>(nw4r::ut::List_GetNext(&mComponents, p))) {
        p->mpComponent->setDraggingButton(button);
    }
}

PaneManager::~PaneManager() {
    PaneToComponent *p = static_cast<PaneToComponent *>(nw4r::ut::List_GetFirst(&mPaneComponents));
    while (p) {
        nw4r::ut::List_Remove(&mPaneComponents, p);
        if (mpAllocator) {
            p->mpComponent->~PaneComponent();
            p->~PaneToComponent();

            MEMFreeToAllocator(mpAllocator, p->mpComponent);
            MEMFreeToAllocator(mpAllocator, p);
        } else {
            delete p->mpComponent;
            delete p;
        }
        p = static_cast<PaneToComponent *>(nw4r::ut::List_GetFirst(&mPaneComponents));
    }
}

void PaneManager::createLayoutScene(const nw4r::lyt::Layout &layout) {
    mIDCounter = 0;
    walkInChildren(layout.GetRootPane()->GetChildList());
}

void PaneManager::walkInChildren(nw4r::lyt::PaneList &paneList) {
    for (nw4r::lyt::PaneList::Iterator it = paneList.GetBeginIter(); it != paneList.GetEndIter(); ++it) {
        addPane_(&*it);
        walkInChildren(it->GetChildList());
    }
}

void PaneManager::addPane_(nw4r::lyt::Pane *pane) {
    PaneComponent *component = NULL;
    PaneToComponent *toComponent = NULL;

    if (mpAllocator) {
        void *comBuf = MEMAllocFromAllocator(mpAllocator, sizeof(PaneComponent));
        void *buf = MEMAllocFromAllocator(mpAllocator, sizeof(PaneToComponent));

        component = new (comBuf) PaneComponent(mIDCounter);
        toComponent = new (buf) PaneToComponent(pane, component);
    } else {
        component = new PaneComponent(mIDCounter);
        toComponent = new PaneToComponent(pane, component);
    }

    nw4r::ut::List_Append(&mPaneComponents, toComponent);
    mIDCounter++;

    component->setPane(pane);

    if (nw4r::ut::DynamicCast<nw4r::lyt::Picture *>(pane)) {
        component->setTriggerTarget(true);
    }
    if (nw4r::ut::DynamicCast<nw4r::lyt::Window *>(pane)) {
        component->setTriggerTarget(true);
    }

    addComponent(component);
}

PaneComponent *PaneManager::searchPaneComponent(const char *name) {
    for (u32 i = 0; i < nw4r::ut::List_GetSize(&mPaneComponents); i++) {
        PaneToComponent *p = static_cast<PaneToComponent *>(nw4r::ut::List_GetNth(&mPaneComponents, (u16)i));
        if (util::strcmp(p->mpPane->GetName(), name)) {
            return p->mpComponent;
        }
    }
    return NULL;
}

PaneComponent *PaneManager::getPaneComponentByPane(nw4r::lyt::Pane *pane) {
    for (u32 i = 0; i < nw4r::ut::List_GetSize(&mPaneComponents); i++) {
        PaneToComponent *p = static_cast<PaneToComponent *>(nw4r::ut::List_GetNth(&mPaneComponents, (u16)i));
        if (p->mpPane == pane) {
            return p->mpComponent;
        }
    }
    return NULL;
}

void PaneManager::setAllBoundingBoxComponentTriggerTarget(bool enable) {
    for (u32 i = 0; i < nw4r::ut::List_GetSize(&mPaneComponents); i++) {
        PaneToComponent *p = static_cast<PaneToComponent *>(nw4r::ut::List_GetNth(&mPaneComponents, (u16)i));
        if (nw4r::ut::DynamicCast<nw4r::lyt::Bounding *>(p->getPane())) {
            p->mpComponent->setTriggerTarget(enable);
        }
    }
}

bool PaneComponent::contain(f32 x, f32 y) {
    if (mpManager == NULL) {
        return false;
    }

    const nw4r::lyt::DrawInfo *drawInfo = static_cast<PaneManager *>(mpManager)->getDrawInfo();
    if (drawInfo == NULL) {
        return false;
    }

    nw4r::math::MTX34 globalMtx;
    MTXInverse(mpPane->GetGlobalMtx(), globalMtx);

    nw4r::math::VEC3 pos;
    MTXMultVec(globalMtx, nw4r::math::VEC3(x, y, 0), pos);

    nw4r::ut::Rect rect = mpPane->GetPaneRect(*drawInfo);

    if (rect.left <= pos.x && pos.x <= rect.right && rect.bottom <= pos.y && pos.y <= rect.top) {
        return true;
    } else {
        return false;
    }
}

void PaneComponent::draw() {
    if (static_cast<PaneManager *>(mpManager)->getDrawInfo()) {
        nw4r::lyt::Size size = mpPane->GetSize();
        const nw4r::math::MTX34 &mtx = mpPane->GetGlobalMtx();

        f32 x = mtx.m[0][3];
        f32 y = mtx.m[1][3];

        GXColor color = {255, 0, 0, 255};
        if (isPointed(0)) {
            color.r = 0;
            color.b = 255;
        }

        drawLine_(x - size.width / 2, y - size.height / 2, x + size.width / 2, y - size.height / 2, 0, 8, color);
        drawLine_(x + size.width / 2, y - size.height / 2, x + size.width / 2, y + size.height / 2, 0, 8, color);
        drawLine_(x + size.width / 2, y + size.height / 2, x - size.width / 2, y + size.height / 2, 0, 8, color);
        drawLine_(x - size.width / 2, y + size.height / 2, x - size.width / 2, y - size.height / 2, 0, 8, color);
    }
}

bool PaneComponent::isVisible() {
    nw4r::lyt::Pane *pane = mpPane;
    if (pane == NULL) {
        return false;
    }
    while (pane != NULL) {
        if (!pane->IsVisible()) {
            return false;
        }
        pane = pane->GetParent();
    }
    return true;
}

} // namespace gui
} // namespace textinput
