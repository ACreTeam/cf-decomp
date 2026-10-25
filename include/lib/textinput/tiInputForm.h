#pragma once

// textinput::inputform: the text field (see tiTextInputBase.h).

#include <lib/textinput/tiUtil.h>
#include <lib/textinput/tiGUIManager.h>
#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiTextDrawer.h>
#include <lib/textinput/tiCandidateBox.h>
#include <lib/textinput/tiStringWithAtok.h>
#include <lib/textinput/tiZiString.h>
#include <revolution/kpr.h>

namespace textinput {

class Manager;

enum HVKCode {};

namespace inputform {

class DeadKeyStream {
  public:
#if VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
    DeadKeyStream() {
        KPRInitRegionJP();
        Init();
    }

    void Init() {
        KPRInitQueue(&mKPRQueue);
        KPRSetMode(&mKPRQueue, KPR_MODE_NONE);
    }
#else
    DeadKeyStream() {
        KPRInitRegionEU();
        Init();
    }

    void Init() {
        KPRInitQueue(&mKPRQueue);
        KPRSetMode(&mKPRQueue, KPR_MODE_DEADKEY);
    }
#endif

    static wchar_t ToCombineClass(Language language, wchar_t ch);
    static wchar_t ToIndependentClass(wchar_t ch);

    /* 0x00 */ KPRQueue mKPRQueue;
}; // size 0x18

class Base : public CommandReceiver,
             public textdrawer::Base,
             public candidatebox::CandidateBoxCaller {
  public:
    enum PredictMode {
        PM_Off,
        PM_Atok,
        PM_USEn,
        PM_USFr,
        PM_USSp,
        PM_En,
        PM_De,
        PM_Fr,
        PM_Sp,
        PM_It,
        PM_Nl,
        PM_End,
    };

    enum InputMode {
        IM_Direct,
        IM_Roman,
        IM_End,
    };

    enum ScrollFlag {
        SF_NoScroll,
        SF_ScrollOn,
        SF_ScrollByBS,
        SF_End,
    };

    struct Info_ {
        /* 0x0 */ u16 Back;
        /* 0x2 */ u16 Next;
        /* 0x4 */ u16 StrCount;
        /* 0x6 */ u16 DispRowCount;
    }; // size 0x8

    class RowInfoManager {
      public:
        RowInfoManager(u16 maxLength)
            : mpInfo(NULL), mMaxLength(maxLength), mpAllocator(NULL) {
        }
        ~RowInfoManager() {
            MEMFreeToAllocator(mpAllocator, mpInfo);
        }

        void create(MEMAllocator *allocator) {
            mpAllocator = allocator;
            mpInfo = static_cast<Info_ *>(MEMAllocFromAllocator(
                allocator, (mMaxLength + 2) * sizeof(Info_)));
            init();
        }
        void init();
        Info_ *getInfo(u16 index) {
            return &mpInfo[index];
        }
        Info_ *getNext(Info_ *info) {
            return &mpInfo[info->Next];
        }
        Info_ *getBack(Info_ *info) {
            return &mpInfo[info->Back];
        }
        u16 getIndex(Info_ *info) {
            return getNext(info)->Back;
        }
        u16 getMaxLength() const {
            return mMaxLength;
        }
        Info_ *getHead() {
            return getInfo(getMaxLength());
        }
        Info_ *getEmpty() {
            return getInfo(mMaxLength + 1);
        }
        void unlink(Info_ *info) {
            u16 back = info->Back;
            u16 next = info->Next;
            Info_ *backInfo = getBack(info);
            u16 index = getNext(info)->Back;
            getNext(info)->Back = back;
            backInfo->Next = next;
            info->Back = index;
            info->Next = index;
        }
        Info_ *newLine() {
            Info_ *info = getNext(getHead());
            unlink(info);
            linkAfter(getBack(getEmpty()), info);
            return info;
        }
        void linkAfter(Info_ *prev, Info_ *info) {
            if (info->Back != info->Next) {
                return;
            }
            u16 index = info->Back;
            u16 next = prev->Next;
            Info_ *nextInfo = getNext(prev);
            info->Back = nextInfo->Back;
            info->Next = next;
            prev->Next = index;
            nextInfo->Back = index;
        }

        /* 0x0 */ Info_ *mpInfo;
        /* 0x4 */ u16 mMaxLength;
        /* 0x8 */ MEMAllocator *mpAllocator;
    }; // size 0xC

    Base(Manager *manager);

    virtual ~Base();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onCommand(INPUT_COMMAND command, void *data);

    virtual void update(); // 0xD4
#if VERSION == VERSION_RUUK01_01
    virtual void updateCandidateState_();
#endif
    virtual wchar_t *getWCString() const {
        return mpString->getWCString();
    } // 0xD8
    virtual tistring::WithAtok *getAtokString() {
        return mpUnfixString;
    } // 0xDC
    virtual wchar_t getCandidate() const {
        return mpString->getCandidate();
    }                                                     // 0xE0
    virtual u32 getCursorPos();                           // 0xE4
    virtual void setFont(const nw4r::ut::Font &font);     // 0xE8
    virtual bool isEditMode();                            // 0xEC
    virtual bool checkHeadOfSentence(bool);               // 0xF0
    virtual void setDestination(Destination destination); // 0xF4
    virtual void limitStringLength(u32 limitStringLength) {
        muLimitStringLength = limitStringLength;
    }                                           // 0xF8
    virtual void limitRowNum(u32 limitRowNum) { // 0xFC
        muLimitRowNum = limitRowNum;
        if (limitRowNum == 1) {
            doWordWrap(false);
        }
    }
    virtual void doWordWrap(bool wordWrap) {
        mbDoWordWrap = wordWrap;
    } // 0x100
    virtual void setLineDrawInfo(const bool &lineDraw,
                                 const u32 &lineCount) { // 0x104
        mbLineDraw = lineDraw;
        muSpecifyLineDrawCount = lineCount;
    }
    virtual void setLanguage(Language language);                        // 0x108
    virtual void setPredictMode(PredictMode predictMode);               // 0x10C
    virtual PredictMode getPredictMode();                               // 0x110
    virtual u32 isOverRowLimit(u32 limitRowNum, const wchar_t *string); // 0x114
    virtual bool findURL(u32 *, u32 *, const wchar_t *, u32, u32);      // 0x118
    virtual bool isInScroll() {
        return mScrollAnm.isActive();
    } // 0x11C
    virtual void setAtokDictionary(void *atokDict, int atokDictSize,
                                   void *apotDict, int apotDictSize,
                                   void *nintendoDict,
                                   int nintendoDictSize); // 0x120
    virtual void closeAtokDictionary();                   // 0x124
    virtual bool isAtokDictionaryOpened() {
        return mpUnfixString->isDictionaryOpen();
    } // 0x128
    virtual void enableSpaceByRight(bool rightWithSpace) {
        mbRightWithSpace = rightWithSpace;
    }                       // 0x12C
    virtual void onClose(); // 0x130
    virtual bool canConvert() {
        return getCurrentString(false) == mpUnfixString ? true : false;
    }                                              // 0x134
    virtual void setString(const wchar_t *string); // 0x138
    virtual void draw();                           // 0x13C
    virtual void calc();                           // 0x140
    virtual void doBeforeDrawProcess(const wchar_t *wcString, u32 pos,
                                     const DrawInfo &drawInfo); // 0x144
    virtual void doAfterDrawProcess(const wchar_t *wcString, u32 pos,
                                    const DrawInfo &drawInfo) {
    }                              // 0x148
    virtual void preDraw(u32 pos); // 0x14C
    virtual void finishDraw(u32 pos) {
    }                                                         // 0x150
    virtual void doLineFeed();                                // 0x154
    virtual bool onCursor(CursorPos *cursorPos);              // 0x158
    virtual void drawCursor(f32 x, f32 y);                    // 0x15C
    virtual void doScroll(Scroll *scroll);                    // 0x160
    virtual void autoScroll();                                // 0x164
    virtual bool doWordWrap(const wchar_t *string, u32, f32); // 0x168
    void completeDrawCursor_(CursorPos *cursorPos);
    virtual void drawFixString(u32); // 0x16C
    virtual void onSE(sound::SE seId) {
    }                                                     // 0x170
    virtual void clear();                                 // 0x174
    virtual u32 calcCursorPos(f32 x, f32 y);              // 0x178
    virtual nw4r::math::VEC2 getGlobalLeftTopPos() const; // 0x17C
    virtual nw4r::math::VEC2 getScale();                  // 0x180
    virtual void moveCursorUp();                          // 0x184
    virtual void moveCursorDown();                        // 0x188
    virtual void onPressUp();                             // 0x18C
    virtual void onPressDown();                           // 0x190
    virtual void onPressLeft();                           // 0x194
    virtual void onPressRight();                          // 0x198
    virtual void makeUpCursorPos(CursorPos *cursorPos, u32 pos, s32 line,
                                 s32 endLine); // 0x19C

    struct CharInput {
        /* 0x0 */ wchar_t ch;
        /* 0x4 */ u32 flags;
        /* 0x8 */ bool usePCKeyboardMode;
        /* 0x9 */ bool deadKey;
        /* 0xA */ u8 _0A[0x10 - 0x0A];
    }; // size 0x10

    struct KeyInput {
        /* 0x0 */ wchar_t ch;
        /* 0x2 */ u16 letterMode;
        /* 0x4 */ bool direct;
        /* 0x5 */ u8 _05;
        /* 0x6 */ bool checkCandidate;
        /* 0x7 */ bool noSE;
        /* 0x8 */ void *holdingKey;
    }; // size 0xC

    struct PredictInfo {
        /* 0x0 */ PredictMode mode;
        /* 0x4 */ u8 on;
    }; // size 0x8

    struct InputState {
        /* 0x0 */ tistring::Decolated::TranslateMode mode;
        /* 0x4 */ bool kanaMode;
        /* 0x5 */ bool fixMode;
    }; // size 0x8

    struct HKBInput {
        /* 0x0 */ HVKCode code;
        /* 0x4 */ u32 flags;
    }; // size 0x8

    void setCursorPos(tistring::Decolated *string, u32 pos);
    void calcCursorTimer();
    void confirmInput_();
    void inputInputting_(wchar_t ch);
    void confirmInputting_(wchar_t ch, bool direct, u16 letterMode,
                           bool checkCandidate, void *holdingKey);
    void inputCharZi_(wchar_t ch, u32 flags);
    void inputCharDefault_(wchar_t ch);
    tistring::Decolated *getCurrentString(bool ignoreFix);
    bool isAtokActive();
    void onHKBCtrlCode(HVKCode code, u32);
    void onPressDownHWKB();
    void onPressLeftHWKB();
    void onPressRightHWKB();
    void moveCandidateToIdx(s32 index);
    void setCursorSelectFlg(bool flag) {
        mbCursorSelected = flag;
    }
    void toggleAtokMode_(u8);
    void notifyChangeMode();
    void changeInputState(const InputState &state, InputState &prev);
    bool isZiActive() {
        return !mbPredictOn ? false : mePredictMode != PM_Atok;
    }
    bool isZiPredicting();
    void initZiString();
    void setStringLength_(u32 length);
    void startConvertAtok_();

    /* 0x18F8 */ nw4r::ut::Rect mRect;
    /* 0x1908 */ nw4r::math::MTX34 mMtx;
    /* 0x1938 */ Destination meDestination;
    /* 0x193C */ tistring::Decolated *mpString;
    /* 0x1940 */ tistring::WithAtok *mpUnfixString;
    /* 0x1944 */ tistring::WithZi *mpZiString;
    /* 0x1948 */ InputMode meInputMode;
    /* 0x194C */ PredictMode mePredictMode;
    /* 0x1950 */ bool mbPredictOn;
    /* 0x1951 */ bool mbDoWordWrap;
    /* 0x1952 */ bool mbRightWithSpace;
    /* 0x1954 */ f32 mfCursorX;
    /* 0x1958 */ f32 mfCursorY;
    /* 0x195C */ f32 mfScrollX;
    /* 0x1960 */ f32 mfScrollY;
    /* 0x1964 */ util::Animation mScrollAnm;
    /* 0x1984 */ f32 mfSustainTimer;
    /* 0x1988 */ ScrollFlag meScrollFlag;
    /* 0x198C */ u32 muWordWrapCounter;
    /* 0x1990 */ u32 muLimitStringLength;
    /* 0x1994 */ u32 muLimitRowNum;
    /* 0x1998 */ GXColor mSelectedColor;
    /* 0x199C */ f32 mfCursorTimer;
    /* 0x19A0 */ u8 muGlobalAlpha;
    /* 0x19A4 */ nw4r::ut::Color mCharColor;
    /* 0x19A8 */ MEMAllocator *mpAllocator;
    /* 0x19AC */ Manager *mpManager;
    /* 0x19B0 */ DeadKeyStream mDKStream;
    /* 0x19C8 */ Language meLanguage;
    /* 0x19CC */ bool mbZuSelected;
    /* 0x19CD */ bool mbCursorSelected;
    /* 0x19D0 */ RowInfoManager mriManager;
    /* 0x19DC */ Info_ *mpCursorLine;
    /* 0x19E0 */ Info_ *mpDrawEndLine;
    /* 0x19E4 */ u16 mCursorLinePos;
    /* 0x19E6 */ bool mbLineDraw;
    /* 0x19E8 */ u32 muSpecifyLineDrawCount;
    /* 0x19EC */ u32 muCursorTimer;
}; // size 0x19F0

class EventHandler;

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
  public:
    nw4r::lyt::TextBox &getTextBox_();
    LayoutByNW4R(Manager *manager,
                 nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver,
                 const char *fontName)
        : Base(manager), Layout(multiArc, layoutBinName, eventObserver),
          mpszTextBoxName("T_2l_TextBox"), mpszFontName(fontName),
          mbAbleToUp(false), mbAbleToDown(false), mbVisibleSeparator(false),
          mpEventHandler(NULL) {
    }

    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onCommand(INPUT_COMMAND command, void *data);
    virtual void draw();
    virtual void calc();
    virtual void onSE(sound::SE seId);
    virtual nw4r::math::VEC2 getScale();

    virtual void updateInputCommon(int chan, u32 trig, u32 hold, u32 release,
                                   void *data);
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release, void *data);
    virtual bool updateInput(input::HKBManager &hkbManager);
    virtual bool isAbleToUp() {
        return mbAbleToUp;
    }
    virtual bool isAbleToDown() {
        return mbAbleToDown;
    }
    virtual void setRootPaneScaleFor16x9();
    virtual void setRootPaneScaleFor4x3();
    virtual void visibleSeparator(bool flag) {
        mbVisibleSeparator = flag;
    }

    void updateRepeatInput(u32 trig, u32 hold);

    /* 0x1A98 */ const char *mpszTextBoxName;
    /* 0x1A9C */ const char *mpszFontName;
    /* 0x1AA0 */ bool mbAbleToUp;
    /* 0x1AA1 */ bool mbAbleToDown;
    /* 0x1AA2 */ bool mbVisibleSeparator;
    /* 0x1AA4 */ EventHandler *mpEventHandler;
    /* 0x1AA8 */ nw4r::ut::ResFont mResFont;
    /* 0x1AC4 */ u32 muRepeatCountUp;
    /* 0x1AC8 */ u32 muRepeatCountDown;
    /* 0x1ACC */ u32 muRepeatCountLeft;
    /* 0x1AD0 */ u32 muRepeatCountRight;
    /* 0x1AD4 */ u32 muRepeatCountMinus;
    /* 0x1AD8 */ u8 _1AD8[0x1ADC - 0x1AD8];
    /* 0x1ADC */ u32 muRepeatTrig;
};

class EventHandler : public nw4rmanager::TiEventHandler {
  public:
    EventHandler(LayoutByNW4R *layout) : mpLayout(layout) {
    }

    virtual ~EventHandler() {
    }
    virtual void onTiEvent(gui::PaneComponent *paneComponent, u32 event,
                           Input *input);

    /* 0x0C */ LayoutByNW4R *mpLayout;
}; // size 0x10

class AnmPane : public nw4rmanager::AnmPane {
  public:
    AnmPane(nw4r::lyt::Pane *pane)
        : nw4rmanager::AnmPane(pane, NULL), meState(0) {
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
    virtual u32 getKeyType() const {
        return meKeyType;
    }
    virtual int getState() {
        return meState;
    }

    /* 0x2C */ int meState;
    /* 0x30 */ u32 meKeyType;
}; // size 0x34

class NormalButtonAnmPane : public AnmPane {
  public:
    NormalButtonAnmPane(nw4r::lyt::Pane *pane) : AnmPane(pane) {
        meKeyType = 0;
    }

    virtual void onAnmEvent(AnmPaneEvent paneEvent);
    virtual ~NormalButtonAnmPane() {
    }
};

class Sample : public LayoutByNW4R {
  public:
    Sample(Manager *manager, nw4r::lyt::MultiArcResourceAccessor *accessor,
           const char *layoutName, EventObserver *observer,
           const char *fontName = NULL)
        : LayoutByNW4R(manager, accessor, layoutName, observer, fontName) {
    }
};

} // namespace inputform
} // namespace textinput
