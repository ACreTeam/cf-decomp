#include <lib/textinput/MyTiInputForm.h>

#include <lib/textinput/tiLayout.h>
#include <lib/textinput/tiManager.h>

#include <new>

namespace textinput {
namespace extend {
namespace memo {

enum MemoAnm {
    MANM_Normal,
    MANM_MailIn,
    MANM_MailOut,
    MANM_SendOut,
    MANM_NigaoeFocusIn,
    MANM_NigaoeFocusOut,
};

enum ScrollButtonAnm {
    SBANM_Off,
    SBANM_Appear,
    SBANM_Lost,
    SBANM_HDActionEnd,
    SBANM_HDActionStart,
    SBANM_Loop,
    SBANM_Select,
    SBANM_FocusOff,
    SBANM_FocusOn,
    SBANM_None,
};

enum KeyType {
    KT_WholePane,
    KT_NigaoePane,
};

struct AnimationFile {
    /* 0x00 */ u32 id;
    /* 0x04 */ char fileName[0x40];
}; // size 0x44

struct PaneToAnimation {
    /* 0x00 */ s32 type;
    /* 0x04 */ const char *paneName;
    /* 0x08 */ u32 count;
    /* 0x0C */ const AnimationFile *pAnims[12];
}; // size 0x3C

struct GroupAnimationSet {
    /* 0x00 */ s32 group;
    /* 0x04 */ s32 anims[3];
}; // size 0x10

static const char *scPaneName[] = {
    "N_Header", "N_Body",   "B_Body",   "N_Footer",      "Nigaoe",
    "T_Nigaoe", "B_Nigaoe", "T_Letter", "T_TouchLetter",
};

static const AnimationFile csAninationFile[] = {
    {MANM_Normal, "my_Memo_a_normal.brlan"},
    {MANM_MailIn, "my_Memo_a_MailIn.brlan"},
    {MANM_MailOut, "my_Memo_a_MailOut.brlan"},
    {MANM_SendOut, "my_Memo_a_SendOut.brlan"},
    {MANM_NigaoeFocusIn, "my_Memo_a_NigaoeFoucusIn.brlan"},
    {MANM_NigaoeFocusOut, "my_Memo_a_NigaoeFoucusOut.brlan"},
};

static const PaneToAnimation csPaneToAnimation[] = {
    {KT_WholePane,
     "N_Memo",
     4,
     {&csAninationFile[0], &csAninationFile[1], &csAninationFile[2],
      &csAninationFile[3]}},
    {KT_NigaoePane, "Nigaoe", 2, {&csAninationFile[4], &csAninationFile[5]}},
};

class EventHandler : public inputform::EventHandler {
  public:
    EventHandler(InputForm *inputForm)
        : inputform::EventHandler(inputForm), mpOwnerForm(inputForm) {
    }

    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    /* 0x10 */ InputForm *mpOwnerForm;
}; // size 0x14

class AnmPane : public nw4rmanager::AnmPane {
  public:
    AnmPane(nw4r::lyt::Pane *pane)
        : nw4rmanager::AnmPane(pane, NULL), meState(0) {
    }

    virtual void init() {
        meState = 0;
    }
    virtual u32 getKeyType() const {
        return meKeyType;
    }
    virtual void changeAnimation(u32 id) {
        meState = id;
        nw4rmanager::AnmPane::changeAnimation(id);
    }
    virtual int getState() {
        return meState;
    }

    /* 0x2C */ int meState;
    /* 0x30 */ u32 meKeyType;
}; // size 0x34

class WholePane : public AnmPane {
  public:
    WholePane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = KT_WholePane;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~WholePane() {
    }
};

class NigaoePane : public AnmPane {
  public:
    NigaoePane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = KT_NigaoePane;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent) {
    }
    virtual ~NigaoePane() {
    }
};

class ScrollButton {
  public:
    ScrollButton(TiLayout *layout)
        : mnGroup(0), muAnim(0), mpLayout(layout), mbHoldLeft(false),
          mbHoldRight(false) {
    }

    virtual void create(MEMAllocator *allocator,
                        nw4r::lyt::MultiArcResourceAccessor *multiArc);
    virtual void init() {
    }
    virtual void calc();
    virtual void changeAnimation(bool left, u32 id);

  private:
    /* 0x04 */ int mnGroup;
    /* 0x08 */ u32 muAnim;
    /* 0x0C */ TiLayout *mpLayout;
    /* 0x10 */ nw4rmanager::AnimPaneGroup *mpGroups[9];
    /* 0x34 */ bool mbGroupOn[9];
    /* 0x3D */ bool mbHoldLeft;
    /* 0x3E */ bool mbHoldRight;
}; // size 0x40

class SimpleAnmPane : public nw4rmanager::AnmPane {
  public:
    SimpleAnmPane(nw4r::lyt::Pane *pane) : nw4rmanager::AnmPane(pane, NULL) {
    }

    virtual void init() {
    }
    virtual void changeAnimation(u32 id) {
        nw4rmanager::AnmPane::changeAnimation(id);
    }
}; // size 0x2C

InputForm::~InputForm() {
    mpSendString->~Decolated();
    MEMFreeToAllocator(mpAllocator, mpSendString);
    MEMFreeToAllocator(mpAllocator, mpDefaultNigaoe);
}

void InputForm::setAddScroll(f32 scroll, bool up, bool down) {
    setScroll(mfScroll + scroll);
    if (up && !mbScrollUp) {
        mpScrollButton->changeAnimation(false, SBANM_HDActionStart);
    } else if (down && !mbScrollDown) {
        mpScrollButton->changeAnimation(true, SBANM_HDActionStart);
    }
    if (!up && mbScrollUp) {
        mpScrollButton->changeAnimation(false, SBANM_HDActionEnd);
    } else if (!down && mbScrollDown) {
        mpScrollButton->changeAnimation(true, SBANM_HDActionEnd);
    }
    mbScrollUp = up;
    mbScrollDown = down;
}

void InputForm::moveCursorUp() {
    f32 y = 1.0f + (mfCursorY - getLineHeight());
    if (y <= 0.0f) {
        u32 cursor;
        u32 anchor;
        mpString->getCursorPos(&cursor, &anchor);
        if (cursor == 0 && anchor == 0) {
            onSE(sound::SE_CHAR_CURSOR_FIX);
        } else {
            onSE(sound::SE_CHAR_CURSOR);
        }
        setCursorPos(mpString, 0);
    } else {
        setCursorPos(mpString, calcCursorPos(mfCursorX, y - mfScroll));
        onSE(sound::SE_CHAR_CURSOR);
    }
    meScrollFlag = SF_ScrollOn;
}

void InputForm::moveCursorDown() {
    f32 y = 1.0f + (mfCursorY + getLineHeight());
    if (y / getLineHeight() >= getLine()) {
        u32 cursor;
        u32 anchor;
        mpString->getCursorPos(&cursor, &anchor);
        if (cursor == mpString->getLength() &&
            anchor == mpString->getLength()) {
            onSE(sound::SE_CHAR_CURSOR_FIX);
        } else {
            onSE(sound::SE_CHAR_CURSOR);
        }
        setCursorPos(mpString, mpString->getLength());
    } else {
        setCursorPos(mpString, calcCursorPos(mfCursorX, y - mfScroll));
        onSE(sound::SE_CHAR_CURSOR);
    }
    meScrollFlag = SF_ScrollOn;
}

void InputForm::onCommand(INPUT_COMMAND command, void *data) {
    if (meEditMode == EM_Edit) {
        onCommandOnEditMode(command, data);
    } else if (meEditMode == EM_Disp) {
        onCommandOnDispMode(command, data);
    } else {
        switch (command) {
        case 6:
        case 21:
        case 29:
        case 31:
            inputform::LayoutByNW4R::onCommand(command, data);
            break;
        }
    }
}

void InputForm::onCommandOnEditMode(INPUT_COMMAND command, void *arg) {
    switch (command) {
    case 24: {
        if (!mExScrollAnm.isActive()) {
            Scroll *scroll = static_cast<Scroll *>(arg);
            if (scroll->y < 0.0f) {
                mExScrollAnm.startAnm(mfScroll, mfScroll + getLineHeight(),
                                      15.0f, NULL, NULL);
            } else {
                mExScrollAnm.startAnm(mfScroll, mfScroll - getLineHeight(),
                                      15.0f, NULL, NULL);
            }
            onSE(sound::SE_LINE_SCROLL);
        }
        break;
    }
    default:
        inputform::LayoutByNW4R::onCommand(command, arg);
        break;
    }
}

void InputForm::onCommandOnDispMode(INPUT_COMMAND command, void *arg) {
    switch (command) {
    case 14: {
        nw4r::math::VEC2 *point = static_cast<nw4r::math::VEC2 *>(arg);
        nw4r::math::VEC2 leftTop = getGlobalLeftTopPos();
        int line = (point->y - leftTop.y) / getLineHeight();
        if (line > getLine() - 1) {
            line = getLine() - 1;
        }
        if (getLine() > 1 && line >= getLine() - 1) {
            line = getLine() - 2;
        }
        mfScrollFrom = mfScroll;
        mfScrollTo = line * getLineHeight();
        nw4r::math::VEC2 pos;
        pos.x = point->x;
        pos.y = point->y - mfScroll;
        inputform::LayoutByNW4R::onCommand((INPUT_COMMAND)14, &pos);
        int cursorLine = line;
        struct {
            u32 mode;
            int *line;
        } cursor;
        cursor.mode = 0;
        cursor.line = &cursorLine;
        inputform::LayoutByNW4R::onCommand((INPUT_COMMAND)37, &cursor);
        mbEdited = true;
        CandidateBoxCaller::resetCandidate();
        CandidateBoxCaller::makeEmptyCandidate();
        break;
    }
    case 29:
    case 31:
    case 41:
        inputform::LayoutByNW4R::onCommand(command, arg);
        break;
    }
}

void InputForm::create(MEMAllocator *allocator) {
    inputform::LayoutByNW4R::create(allocator);

    void *stringBuf =
        MEMAllocFromAllocator(allocator, sizeof(tistring::Decolated));
    mpSendString = new (stringBuf) tistring::Decolated(0x400);
    mpSendString->create(allocator);

    MEMFreeToAllocator(allocator, mpEventHandler);
    void *eventBuf = MEMAllocFromAllocator(allocator, sizeof(EventHandler));
    mpEventHandler = new (eventBuf) EventHandler(this);
    mpEventHandler->setEventObserver(mpEventObserver);
    mpPaneManager->changeEventHandler(mpEventHandler);

    mpDefaultNigaoe = static_cast<GXTexObj *>(
        MEMAllocFromAllocator(allocator, sizeof(GXTexObj)));
    getNigaoePaneMaterial()->GetTexture(0).Get(mpDefaultNigaoe);

    setVisible("P_2l_TextBox", true);

    mpTextPane = mpLayout->GetRootPane()->FindPaneByName("T_2l_TextBox", true);
    mDefaultBoundTrans = mpTextPane->GetTranslate();
    mDefaultBoundSize = mpTextPane->GetSize();

    mpBoundPane = mpLayout->GetRootPane()->FindPaneByName("B_2l_TextBox", true);
    mpPaneManager->getPaneComponentByPane(mpBoundPane)->setTriggerTarget(false);
    mDrawRect = mpBoundPane->GetPaneRect(mDrawInfo);
    mDefaultDrawTrans = mpBoundPane->GetTranslate();
    mDefaultDrawSize = mpBoundPane->GetSize();

    mpMemoPane = mpLayout->GetRootPane()->FindPaneByName("N_Memo", true);
    mpMemoRootPane =
        mpLayout->GetRootPane()->FindPaneByName("N_MemoRoot", true);

    setScroll(0.0f);
    mnLine = 1;
    mfScrollFrom = 0.0f;
    mfScrollTo = 0.0f;
    PSMTXIdentity(mDrawMtx);
    PSMTXIdentity(mBoundMtx);

    createAnimation(allocator);

    void *buttonBuf = MEMAllocFromAllocator(allocator, sizeof(ScrollButton));
    mpScrollButton = new (buttonBuf) ScrollButton(mpLayout);
    mpScrollButton->create(allocator, mpMultiArcResourceAccessor);
}

void InputForm::createAnimation(MEMAllocator *allocator) {
    for (u16 i = 0; i < ARRAY_LENGTH(csPaneToAnimation); i++) {
        const PaneToAnimation &p = csPaneToAnimation[i];
        AnmPane *pane = NULL;
        switch (p.type) {
        case KT_WholePane: {
            void *paneBuf = MEMAllocFromAllocator(allocator, sizeof(WholePane));
            pane = new (paneBuf) WholePane(getPane(p.paneName));
            break;
        }
        case KT_NigaoePane: {
            void *paneBuf =
                MEMAllocFromAllocator(allocator, sizeof(NigaoePane));
            pane = new (paneBuf) NigaoePane(getPane(p.paneName));
            break;
        }
        }

        nw4r::ut::List_Append(&mAnmPanes, pane);

        for (u16 j = 0; j < p.count; j++) {
            void *pResource = mpMultiArcResourceAccessor->GetResource(
                0, p.pAnims[j]->fileName, NULL);
            AnimTransformPane *transform = static_cast<AnimTransformPane *>(
                getLayout()->CreateAnimTransform(pResource,
                                                 mpMultiArcResourceAccessor));
            pane->addAnimation(allocator, p.pAnims[j]->id, transform, false,
                               true);
        }
    }
}

static const char *csGroupName[] = {
    "G_ArwRoop",  "G_ArwR_Focus", "G_ArwL_Focus", "G_ArwR_Ac",   "G_ArwL_Ac",
    "G_ArwL_End", "G_ArwR_End",   "G_ArwL_HDAc",  "G_ArwR_HDAc",
};

static const AnimationFile csGroupAninationFile[] = {
    {SBANM_Off, "my_Memo_a_Off.brlan"},
    {SBANM_Appear, "my_Memo_a_Appear.brlan"},
    {SBANM_Lost, "my_Memo_a_Lost.brlan"},
    {SBANM_HDActionEnd, "my_Memo_a_HDActionEnd.brlan"},
    {SBANM_HDActionStart, "my_Memo_a_HDActionStart.brlan"},
    {SBANM_Loop, "my_Memo_a_Loop.brlan"},
    {SBANM_Select, "my_Memo_a_Select.brlan"},
    {SBANM_FocusOff, "my_Memo_a_FocusOff.brlan"},
    {SBANM_FocusOn, "my_Memo_a_FocusOn.brlan"},
};

static const GroupAnimationSet csGroupToAnimation[] = {
    {0, {SBANM_Loop, SBANM_None, SBANM_None}},
    {1, {SBANM_FocusOn, SBANM_FocusOff, SBANM_None}},
    {2, {SBANM_FocusOn, SBANM_FocusOff, SBANM_None}},
    {3, {SBANM_Select, SBANM_None, SBANM_None}},
    {4, {SBANM_Select, SBANM_None, SBANM_None}},
    {5, {SBANM_Lost, SBANM_Appear, SBANM_None}},
    {6, {SBANM_Lost, SBANM_Appear, SBANM_None}},
    {7, {SBANM_HDActionStart, SBANM_HDActionEnd, SBANM_None}},
    {8, {SBANM_HDActionStart, SBANM_HDActionEnd, SBANM_None}},
};

void ScrollButton::create(MEMAllocator *allocator,
                          nw4r::lyt::MultiArcResourceAccessor *multiArc) {
    for (int i = 0; i < 9; i++) {
        mbGroupOn[i] = false;
        nw4r::lyt::Group *group =
            mpLayout->GetGroupContainer()->FindGroupByName(csGroupName[i]);
        void *groupBuf = MEMAllocFromAllocator(
            allocator, sizeof(nw4rmanager::AnimPaneGroup));
        mpGroups[i] = new (groupBuf) nw4rmanager::AnimPaneGroup(group);

        for (nw4r::lyt::detail::PaneLinkList::Iterator it =
                 group->GetPaneList().GetBeginIter();
             it != group->GetPaneList().GetEndIter(); it++) {
            void *paneBuf =
                MEMAllocFromAllocator(allocator, sizeof(SimpleAnmPane));
            SimpleAnmPane *pane = new (paneBuf) SimpleAnmPane(it->mTarget);
            for (u16 j = 0; j < 3; j++) {
                s32 id = csGroupToAnimation[i].anims[j];
                if (id != SBANM_None) {
                    void *pResource = multiArc->GetResource(
                        0, csGroupAninationFile[id].fileName, NULL);
                    AnimTransformPane *transform =
                        static_cast<AnimTransformPane *>(
                            mpLayout->CreateAnimTransform(pResource, multiArc));
                    bool loop = false;
                    if (id == SBANM_Loop) {
                        loop = true;
                    }
                    pane->addAnimation(allocator, id, transform, loop, false);
                }
            }
            mpGroups[i]->addAnimPane(pane);
        }
    }
    changeAnimation(true, SBANM_Loop);
    changeAnimation(false, SBANM_Loop);
    changeAnimation(true, SBANM_Lost);
    changeAnimation(false, SBANM_Lost);
    for (int i = 0; i < 10; i++) {
        calc();
    }
}

void ScrollButton::calc() {
    for (int i = 0; i < 9; i++) {
        mpGroups[i]->calc();
    }
}

void ScrollButton::changeAnimation(bool left, u32 id) {
    muAnim = id;
    if (left) {
        switch (id) {
        case SBANM_Off:
            mnGroup = 0;
            break;
        case SBANM_Appear:
            mnGroup = 5;
            break;
        case SBANM_Lost:
            mnGroup = 5;
            break;
        case SBANM_HDActionEnd:
            if (!mbHoldLeft) {
                return;
            }
            mnGroup = 8;
            mbHoldLeft = false;
            break;
        case SBANM_HDActionStart:
            if (mbHoldLeft) {
                return;
            }
            mnGroup = 8;
            mbHoldLeft = true;
            break;
        case SBANM_Loop:
            mnGroup = 0;
            break;
        case SBANM_Select:
            mnGroup = 4;
            break;
        case SBANM_FocusOff:
            mnGroup = 2;
            break;
        case SBANM_FocusOn:
            mnGroup = 2;
            break;
        }
    } else {
        switch (id) {
        case SBANM_Off:
            mnGroup = 0;
            break;
        case SBANM_Appear:
            mnGroup = 6;
            break;
        case SBANM_Lost:
            mnGroup = 6;
            break;
        case SBANM_HDActionEnd:
            if (!mbHoldRight) {
                return;
            }
            mnGroup = 7;
            mbHoldRight = false;
            break;
        case SBANM_HDActionStart:
            if (mbHoldRight) {
                return;
            }
            mnGroup = 7;
            mbHoldRight = true;
            break;
        case SBANM_Loop:
            mnGroup = 0;
            break;
        case SBANM_Select:
            mnGroup = 3;
            break;
        case SBANM_FocusOff:
            mnGroup = 1;
            break;
        case SBANM_FocusOn:
            mnGroup = 1;
            break;
        }
    }
    mbGroupOn[mnGroup] = true;
    mpGroups[mnGroup]->changeAnimation(id);
}

void InputForm::onArrowRPoint() {
    if (meEditMode == EM_Disp) {
        mpScrollButton->changeAnimation(false, SBANM_FocusOn);
        onSE(sound::SE_BT_TARGETTING);
    }
}

void InputForm::onArrowRLeft() {
    if (meEditMode == EM_Disp) {
        mpScrollButton->changeAnimation(false, SBANM_FocusOff);
    }
}

void InputForm::onArrowLPoint() {
    if (meEditMode == EM_Disp) {
        mpScrollButton->changeAnimation(true, SBANM_FocusOn);
        onSE(sound::SE_BT_TARGETTING);
    }
}

void InputForm::onArrowLLeft() {
    if (meEditMode == EM_Disp) {
        mpScrollButton->changeAnimation(true, SBANM_FocusOff);
    }
}

void InputForm::onArrowRTrig() {
    if (meEditMode == EM_Disp && !mExScrollAnm.isActive()) {
        mpScrollButton->changeAnimation(false, SBANM_Select);
        f32 to = mfScroll - 3.0f * getLineHeight();
        if (to <= getScrollMin()) {
            to = getScrollMin();
        }
        mExScrollAnm.startAnm(mfScroll, to, 15.0f, NULL, NULL);
        onSE(sound::SE_LINE_SCROLL);
    }
}

void InputForm::onArrowLTrig() {
    if (meEditMode == EM_Disp && !mExScrollAnm.isActive()) {
        mpScrollButton->changeAnimation(true, SBANM_Select);
        f32 to = mfScroll + 3.0f * getLineHeight();
        if (to >= getScrollMax()) {
            to = getScrollMax();
        }
        mExScrollAnm.startAnm(mfScroll, to, 15.0f, NULL, NULL);
        onSE(sound::SE_LINE_SCROLL);
    }
}

bool InputForm::updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                            u32 release, void *data) {
    if (meEditMode == EM_Disp) {
        if (hold & 0x400) {
            return inputform::LayoutByNW4R::updateInput(chan, 0.0f, 0.0f, 0, 0,
                                                        0, data);
        }
        if (trig & 8) {
            mpScrollButton->changeAnimation(false, SBANM_HDActionStart);
        }
        if (release & 8) {
            mpScrollButton->changeAnimation(false, SBANM_HDActionEnd);
        }
        if (trig & 4) {
            mpScrollButton->changeAnimation(true, SBANM_HDActionStart);
        }
        if (release & 4) {
            mpScrollButton->changeAnimation(true, SBANM_HDActionEnd);
        }
        if (hold & 8) {
            if (getScrollMin() <= mfScroll) {
                setScroll(mfScroll - 3.5f);
                if (mfScroll <= getScrollMin()) {
                    setScroll(getScrollMin());
                } else {
                    onSE(sound::SE_MESSAGE_SCROLL);
                }
            }
            mpScrollButton->changeAnimation(false, SBANM_HDActionStart);
        } else {
            mpScrollButton->changeAnimation(false, SBANM_HDActionEnd);
        }
        if (hold & 4) {
            if (getScrollMax() >= mfScroll) {
                setScroll(mfScroll + 3.5f);
                if (mfScroll >= getScrollMax()) {
                    setScroll(getScrollMax());
                } else {
                    onSE(sound::SE_MESSAGE_SCROLL);
                }
            }
            mpScrollButton->changeAnimation(true, SBANM_HDActionStart);
        } else {
            mpScrollButton->changeAnimation(true, SBANM_HDActionEnd);
        }
    }
    return inputform::LayoutByNW4R::updateInput(chan, x, y, trig, hold, release,
                                                data);
}

bool InputForm::updateInput(input::HKBManager &hkbManager) {
    return inputform::LayoutByNW4R::updateInput(hkbManager);
}

void InputForm::init() {
    static bool sLineDraw = true;
    static u32 sLineDrawCount = 10;

    if (mpTextPane != NULL) {
        mCharColor.r = 20;
        mCharColor.g = 20;
        mCharColor.b = 20;
        mCharColor.a = 255;
    }
    mbAbleToUp = false;
    mbAbleToDown = false;
    searchAnmPane("P_txtScrll_UP")->changeAnimation(7);
    searchAnmPane("P_txtScrll_DOWN")->changeAnimation(7);
    mbGoodBye = false;
    mbEditScrollUp = false;
    mbEditScrollDown = false;
    setScroll(0.0f);
    mfScrollFrom = 0.0f;
    mfScrollTo = 0.0f;
    mpUnfixString->resetRelation();
    update();
    setLineDrawInfo(sLineDraw, sLineDrawCount);
}

void InputForm::draw() {
    setProjectionMtx();
    drawHeader();
    drawBody();
    drawFooter();
    if (meEditMode == EM_Edit) {
        getPane("N_txt_scrl")->Draw(mDrawInfo);
    }
    inputform::Base::draw();
    if (meEditMode == EM_Disp) {
        setProjectionMtx();
        getPane("N_TopBtn_00")->Animate(0);
        getPane("N_TopBtn_00")->CalculateMtx(mDrawInfo);
        getPane("N_TopBtn_00")->Draw(mDrawInfo);
    }
    mnLine = getLine();
    if (mnLine <= 4) {
        mnLine = 4;
    }
}

void InputForm::setScroll(f32 scroll) {
    mfScroll = scroll;
    mfCursorCacheBaseY = meEditMode == EM_Edit ? -mfScroll : 0.0f;
}

static nw4r::math::VEC2 calcLeftBottomPos(const Mtx mtx, nw4r::ut::Rect &rect) {
    Vec pos;
    pos.x = rect.left;
    pos.y = rect.bottom;
    pos.z = 0.0f;
    PSMTXMultVec(mtx, &pos, &pos);
    nw4r::math::VEC2 leftBottom(pos.x, -pos.y);
    return leftBottom;
}

void InputForm::calc() {
#pragma push
#pragma explicit_zero_data on
    static f32 sCursorCacheBaseY = 0.0f;
#pragma pop

    nw4rmanager::Layout::calc();
    muGlobalAlpha = getLayout()->GetRootPane()->GetAlpha();
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
    mpLayout->GetRootPane()
        ->FindPaneByName("N_Memo", true)
        ->SetTranslate(nw4r::math::VEC2(0.0f, mfScroll));

    f32 drawHeight = mnLine * getLineHeight();
    f32 drawY =
        mDefaultDrawTrans.y - (drawHeight - mDefaultDrawSize.height) / 2.0f;
    mpBoundPane->SetSize(
        nw4r::lyt::Size(mpBoundPane->GetSize().width, drawHeight));
    mpBoundPane->SetTranslate(nw4r::math::VEC2(mDefaultDrawTrans.x, drawY));
    mpBoundPane->CalculateMtx(mDrawInfo);
    AdjustPaneMtx(mDrawMtx, mDrawInfo, mpBoundPane->GetGlobalMtx());

    nw4r::ut::Rect rect;
    rect = mpBoundPane->GetPaneRect(mDrawInfo);
    nw4r::math::VEC2 scale(getScale().x, 1.0f);
    nw4r::math::VEC2 leftBottom = calcLeftBottomPos(mDrawMtx, rect);
    mDrawRect.left = leftBottom.x;
    mDrawRect.bottom = leftBottom.y;
    mDrawRect.right = mDrawRect.left + (rect.right - rect.left) * scale.x;
    mDrawRect.top = mDrawRect.bottom + (rect.top - rect.bottom) * scale.y;

    f32 boundHeight = mnLine * getLineHeight();
    f32 boundY =
        mDefaultBoundTrans.y - (boundHeight - mDefaultBoundSize.height) / 2.0f;
    if (meEditMode == EM_Edit) {
        boundHeight = 2.0f * getLineHeight();
        boundY = mDefaultBoundTrans.y -
                 (boundHeight - mDefaultBoundSize.height) / 2.0f - mfScroll;
    } else {
        mfCursorCacheBaseY = sCursorCacheBaseY;
    }
    mpTextPane->SetSize(
        nw4r::lyt::Size(mpTextPane->GetSize().width, boundHeight));
    mpTextPane->SetTranslate(nw4r::math::VEC2(mDefaultBoundTrans.x, boundY));
    mpTextPane->CalculateMtx(mDrawInfo);
    AdjustPaneMtx(mBoundMtx, mDrawInfo, mpTextPane->GetGlobalMtx());
    AdjustPaneMtx(mMtx.m, mDrawInfo, mpTextPane->GetGlobalMtx());
    mRect = mpTextPane->GetPaneRect(mDrawInfo);
    muGlobalAlpha = mpTextPane->GetAlpha();

    mExScrollAnm.calc();
    if (mExScrollAnm.isActive()) {
        setScroll(mExScrollAnm.getValue());
    } else if (meEditMode == EM_Edit) {
        doAutoScroll();
        if (!mExScrollAnm.isActive()) {
            if (mfScroll - getLineHeight() / 2.0f > 0.0f) {
                if (!mbAbleToUp) {
                    searchAnmPane("P_txtScrll_UP")->changeAnimation(5);
                    mpPaneManager
                        ->getPaneComponentByPane(getPane("B_txtScrll_UP"))
                        ->init();
                }
                mbAbleToUp = true;
            } else {
                if (mbAbleToUp) {
                    searchAnmPane("P_txtScrll_UP")->changeAnimation(6);
                }
                mbAbleToUp = false;
            }
            f32 textHeight = getLine() * getLineHeight();
            if (mfScroll + boundHeight + getLineHeight() / 2.0f < textHeight) {
                if (!mbAbleToDown) {
                    searchAnmPane("P_txtScrll_DOWN")->changeAnimation(5);
                    mpPaneManager
                        ->getPaneComponentByPane(getPane("B_txtScrll_DOWN"))
                        ->init();
                }
                mbAbleToDown = true;
            } else {
                if (mbAbleToDown) {
                    searchAnmPane("P_txtScrll_DOWN")->changeAnimation(6);
                }
                mbAbleToDown = false;
            }
        }
    } else {
        mbAbleToUp = false;
        mbAbleToDown = false;
        searchAnmPane("P_txtScrll_UP")->changeAnimation(7);
        searchAnmPane("P_txtScrll_DOWN")->changeAnimation(7);
        getPane("N_txt_scrl")->Animate(0);
        getPane("N_txt_scrl")->CalculateMtx(mDrawInfo);
    }

    nw4r::lyt::Pane *scrollPane = getPane("N_txt_scrl");
    scrollPane->SetTranslate(
        nw4r::math::VEC2(scrollPane->GetTranslate().x, 0.0f));

    if (meEditMode == EM_Disp) {
        mpScrollButton->calc();
        if (!mbGoodBye) {
            if (mbEditScrollUp && getScrollMin() >= mfScroll) {
                mbEditScrollUp = false;
                mpScrollButton->changeAnimation(false, SBANM_Lost);
            } else if (!mbEditScrollUp && getScrollMin() < mfScroll) {
                mbEditScrollUp = true;
                mpScrollButton->changeAnimation(false, SBANM_Appear);
            }
            if (mbEditScrollDown && getScrollMax() <= mfScroll) {
                mbEditScrollDown = false;
                mpScrollButton->changeAnimation(true, SBANM_Lost);
            }
            if (!mbEditScrollDown && getScrollMax() > mfScroll) {
                mbEditScrollDown = true;
                mpScrollButton->changeAnimation(true, SBANM_Appear);
            }
            if (mpNigaoeObserver != NULL) {
                mpNigaoeObserver->moveNigaoeButton();
            }
        }
    }
    calcCursorTimer();
}

void InputForm::doAutoScroll() {
    if (!mExScrollAnm.isActive() &&
        (meScrollFlag == SF_ScrollOn || meScrollFlag == SF_ScrollByBS)) {
        if (meEditMode == EM_Edit) {
            f32 cursorY = mfCursorY + getLineHeight() / 2.0f;
            if (mfScroll > cursorY) {
                int line = (mfScroll - mfCursorY) / getLineHeight();
                mExScrollAnm.startAnm(mfScroll,
                                      mfScroll - line * getLineHeight(), 15.0f,
                                      NULL, NULL);
                onSE(sound::SE_LINE_SCROLL);
            }
            if (mfScroll + 2.0f * getLineHeight() < cursorY) {
                int line =
                    (int)((cursorY - (mfScroll + 2.0f * getLineHeight())) /
                          getLineHeight()) +
                    1;
                mExScrollAnm.startAnm(mfScroll,
                                      mfScroll + line * getLineHeight(), 15.0f,
                                      NULL, NULL);
                onSE(sound::SE_LINE_SCROLL);
            }
        }
        meScrollFlag = SF_NoScroll;
    }
}

void InputForm::beginDraw(const nw4r::ut::Rect &rect) {
    if (mDrawRect.left == mDrawRect.right) {
        mDrawRect.right += 0.1f;
    }
    if (mDrawRect.top == mDrawRect.bottom) {
        mDrawRect.top += 0.1f;
    }
    textdrawer::Base::beginDraw(mDrawRect);
}

void InputForm::doScroll(Scroll *scroll) {
}

u32 InputForm::calcCursorPos(f32 x, f32 y) {
    u32 line = getLine();
    if (line > muLimitRowNum) {
        line = muLimitRowNum;
    }
    if (y + mfScroll > line * getLineHeight()) {
        return inputform::Base::calcCursorPos(x, line * getLineHeight());
    }
    return inputform::Base::calcCursorPos(x, y + mfScroll);
}

void InputForm::drawCursor(f32 x, f32 y) {
    if (meEditMode == EM_Disp) {
        return;
    }
    inputform::Base::drawCursor(x, y);
}

nw4r::math::VEC2 InputForm::getScale() {
    if (mpMemoRootPane == NULL) {
        return nw4r::math::VEC2(1.0f, 1.0f);
    }
    return mpMemoRootPane->GetScale();
}

void InputForm::drawHeader() {
    getPane(scPaneName[0])->Draw(mDrawInfo);
    getPane("Nigaoe")->Draw(mDrawInfo);
    getPane("T_Nigaoe")->Draw(mDrawInfo);
}

void InputForm::drawBody() {
    nw4r::lyt::Pane *pane = getPane(scPaneName[1]);
    f32 y = 0.0f;
    for (int i = 0; i < mnLine; i++) {
        pane->SetTranslate(nw4r::math::VEC2(0.0f, y));
        pane->CalculateMtx(mDrawInfo);
        pane->Draw(mDrawInfo);
        y -= pane->GetSize().height;
    }
    if (!mbEdited) {
        getPane("T_TouchLetter")->Draw(mDrawInfo);
    }
}

void InputForm::drawFooter() {
    f32 y = (mnLine - 1) * (-1.0f * getPane(scPaneName[1])->GetSize().height);
    nw4r::lyt::Pane *pane = getPane(scPaneName[3]);
    pane->SetTranslate(nw4r::math::VEC2(0.0f, y));
    pane->CalculateMtx(mDrawInfo);
    pane->Draw(mDrawInfo);
}

void InputForm::open() {
    setScroll(0.0f);
    mfScrollFrom = 0.0f;
    mfScrollTo = 0.0f;
    mpLayout->GetRootPane()->SetTranslate(nw4r::math::VEC2(0.0f, 0.0f));
    searchAnmPane("N_Memo")->changeAnimation(MANM_MailIn);
    searchAnmPane("P_txtScrll_UP")->changeAnimation(7);
    searchAnmPane("P_txtScrll_DOWN")->changeAnimation(7);
}

void InputForm::close() {
    if (!mbCloseWithSend) {
        searchAnmPane("N_Memo")->changeAnimation(MANM_MailOut);
    } else {
        searchAnmPane("N_Memo")->changeAnimation(MANM_SendOut);
    }
    if (mbEditScrollUp) {
        mpScrollButton->changeAnimation(false, SBANM_Lost);
        mbEditScrollUp = false;
    }
    if (mbEditScrollDown) {
        mpScrollButton->changeAnimation(true, SBANM_Lost);
        mbEditScrollDown = false;
    }
    mbGoodBye = true;
}

bool InputForm::isWholePaneInAnimation() {
    return searchAnmPane("N_Memo")->isInAnimation();
}

void InputForm::setHeaderCaption(const wchar_t *headerCaption) {
    static_cast<nw4r::lyt::TextBox *>(
        mpLayout->GetRootPane()->FindPaneByName("T_Header", true))
        ->SetString(headerCaption, 0);
}

void InputForm::setMiiName(const wchar_t *miiName) {
    static_cast<nw4r::lyt::TextBox *>(
        mpLayout->GetRootPane()->FindPaneByName("T_Nigaoe", true))
        ->SetString(miiName, 0);
}

void InputForm::setTouchLetterCaption(const wchar_t *letterCaption) {
    static_cast<nw4r::lyt::TextBox *>(
        mpLayout->GetRootPane()->FindPaneByName("T_TouchLetter", true))
        ->SetString(letterCaption, 0);
    mbEdited = false;
}

nw4r::lyt::Pane *InputForm::getNigaoePane() {
    return getPane("Nigaoe");
}

nw4r::lyt::Material *InputForm::getNigaoePaneMaterial() {
    return getPane("Nigaoe")->FindMaterialByName("Nigaoe", true);
}

f32 InputForm::getScrollMin() {
    return 0.0f;
}

f32 InputForm::getScrollMax() {
    return getDrawBoxHeight() - 100.0f;
}

f32 InputForm::getDrawBoxHeight() {
    return mnLine * getLineHeight();
}

void InputForm::onNigaoeButtonTrig() {
    if (meEditMode == EM_Disp && mpNigaoeObserver != NULL &&
        mpNigaoeObserver->onNigaoeButton()) {
        onSE(sound::SE_DECIDE);
    }
}

void InputForm::onNigaoeButtonPoint() {
    if (meEditMode == EM_Disp) {
        searchAnmPane("Nigaoe")->changeAnimation(MANM_NigaoeFocusIn);
        if (mpNigaoeObserver != NULL) {
            mpNigaoeObserver->pointNigaoeButton();
        }
        onSE(sound::SE_BT_TARGETTING);
    }
}

void InputForm::onNigaoeButtonLeft() {
    if (meEditMode == EM_Disp) {
        if (mpNigaoeObserver != NULL) {
            mpNigaoeObserver->leftNigaoeButton();
        }
        searchAnmPane("Nigaoe")->changeAnimation(MANM_NigaoeFocusOut);
    }
}

void InputForm::setDefaultNigaoe() {
    getNigaoePaneMaterial()->SetTexture(0, nw4r::lyt::TexMap(*mpDefaultNigaoe));
}

void InputForm::setEditMode(EditMode editMode) {
    meEditMode = editMode;
    switch (editMode) {
    case EM_Disp:
        getPane("N_txt_scrl")->SetVisible(false);
        break;
    case EM_Edit:
        getPane("N_txt_scrl")->SetVisible(true);
        break;
    case EM_Disappear:
        onCommand((INPUT_COMMAND)6, NULL);
        CandidateBoxCaller::resetCandidate();
        CandidateBoxCaller::makeEmptyCandidate();
        break;
    }
}

void InputForm::preDraw(u32 pos) {
    if (meEditMode == EM_Disp) {
        mpSendString->clear();
    }
    inputform::Base::preDraw(pos);
}

void InputForm::doLineFeed() {
    if (meEditMode == EM_Disp) {
        mpSendString->pushBack(L'\n');
    }
    inputform::Base::doLineFeed();
}

void InputForm::put(wchar_t ch) {
    if (meEditMode == EM_Disp) {
        mpSendString->pushBack(ch);
    }
    textdrawer::Base::put(ch);
}

void InputForm::finishDraw(u32 pos) {
    if (meEditMode == EM_Disp) {
        mpSendString->pushBack(L'\0');
    }
}

void WholePane::onAnmEvent(AnmPaneEvent paneEvent) {
}

void EventHandler::onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                             Input *input) {
    inputform::EventHandler::onTiEvent(paneComponent, event, input);
    const char *name = paneComponent->getPane()->GetName();
    if (name[0] == 'B') {
        if (event == 4 && (input->trig & 0x800)) {
            if (util::strcmp("B_Nigaoe", name)) {
                mpOwnerForm->onNigaoeButtonTrig();
            }
            if (util::strcmp("B_ArwR", name)) {
                mpOwnerForm->onArrowRTrig();
            }
            if (util::strcmp("B_ArwL", name)) {
                mpOwnerForm->onArrowLTrig();
            }
        }
        if (event == 0) {
            if (util::strcmp("B_Nigaoe", name)) {
                mpOwnerForm->onNigaoeButtonPoint();
            }
            if (util::strcmp("B_ArwR", name)) {
                mpOwnerForm->onArrowRPoint();
            }
            if (util::strcmp("B_ArwL", name)) {
                mpOwnerForm->onArrowLPoint();
            }
        }
        if (event == 1) {
            if (util::strcmp("B_Nigaoe", name)) {
                mpOwnerForm->onNigaoeButtonLeft();
            }
            if (util::strcmp("B_ArwR", name)) {
                mpOwnerForm->onArrowRLeft();
            }
            if (util::strcmp("B_ArwL", name)) {
                mpOwnerForm->onArrowLLeft();
            }
        }
    }
}

} // namespace memo
} // namespace extend
} // namespace textinput
