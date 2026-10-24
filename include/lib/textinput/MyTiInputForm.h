#pragma once

#include <lib/textinput/tiInputForm.h>
#include <lib/textinput/tiString.h>

namespace textinput {
namespace extend {
namespace memo {

class ScrollButton;

class NigaoeEventObserver {
  public:
    virtual bool onNigaoeButton() {
        return false;
    }
    virtual void pointNigaoeButton() {
    }
    virtual void leftNigaoeButton() {
    }
    virtual void moveNigaoeButton() {
    }
};

class InputForm : public inputform::LayoutByNW4R {
  public:
    enum EditMode {
        EM_Appear,
        EM_Disp,
        EM_Edit,
        EM_Disappear,
        EM_Last,
    };

    InputForm(textinput::Manager *manager,
              nw4r::lyt::MultiArcResourceAccessor *multiArc,
              const char *layoutBinName, EventObserver *eventObserver,
              const char *fontName)
        : inputform::LayoutByNW4R(manager, multiArc, layoutBinName,
                                  eventObserver, fontName),
          mpMemoPane(NULL), mpMemoRootPane(NULL), mpTextPane(NULL),
          mpBoundPane(NULL), mfScroll(0.0f), mfScrollFrom(0.0f),
          mfScrollTo(0.0f), mnLine(0), mpNigaoeObserver(NULL),
          mpDefaultNigaoe(NULL), mpSendString(NULL), mbEdited(false),
          mbGoodBye(false), mbEditScrollUp(false), mbEditScrollDown(false),
          mbCloseWithSend(false), mpScrollButton(NULL), mbScrollUp(false),
          mbScrollDown(false), meEditMode(EM_Disp) {
    }
    virtual ~InputForm();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onCommand(INPUT_COMMAND command, void *data);
    virtual void draw();
    virtual void calc();
    virtual void preDraw(u32 pos);
    virtual void finishDraw(u32 pos);
    virtual void doLineFeed();
    virtual void drawCursor(f32 x, f32 y);
    virtual void doScroll(Scroll *scroll);
    virtual u32 calcCursorPos(f32 x, f32 y);
    virtual nw4r::math::VEC2 getScale();
    virtual void moveCursorUp();
    virtual void moveCursorDown();
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release, void *data);
    virtual bool updateInput(input::HKBManager &hkbManager);

    virtual void setScroll(f32 scroll);                        // 0x234
    virtual void setAddScroll(f32 scroll, bool up, bool down); // 0x238
    virtual f32 getScroll() {
        return mfScroll;
    } // 0x23C
    virtual f32 getScrollFrom() {
        return mfScrollFrom;
    } // 0x240
    virtual f32 getScrollTo() {
        return mfScrollTo;
    }                                                                 // 0x244
    virtual void open();                                              // 0x248
    virtual void close();                                             // 0x24C
    virtual bool isWholePaneInAnimation();                            // 0x250
    virtual void setHeaderCaption(const wchar_t *headerCaption);      // 0x254
    virtual void setMiiName(const wchar_t *miiName);                  // 0x258
    virtual void setTouchLetterCaption(const wchar_t *letterCaption); // 0x25C
    virtual void onNigaoeButtonTrig();                                // 0x260
    virtual void onNigaoeButtonPoint();                               // 0x264
    virtual void onNigaoeButtonLeft();                                // 0x268
    virtual void setDefaultNigaoe();                                  // 0x26C
    virtual void onArrowRPoint();                                     // 0x270
    virtual void onArrowRLeft();                                      // 0x274
    virtual void onArrowLPoint();                                     // 0x278
    virtual void onArrowLLeft();                                      // 0x27C
    virtual void onArrowRTrig();                                      // 0x280
    virtual void onArrowLTrig();                                      // 0x284
    virtual void
    setNigaoeEventObserver(NigaoeEventObserver *observer) { // 0x288
        mpNigaoeObserver = observer;
    }
    virtual nw4r::lyt::Pane *getNigaoePane();                           // 0x28C
    virtual nw4r::lyt::Material *getNigaoePaneMaterial();               // 0x290
    virtual f32 getDrawBoxHeight();                                     // 0x294
    virtual f32 getScrollMin();                                         // 0x298
    virtual f32 getScrollMax();                                         // 0x29C
    virtual void beginDraw(const nw4r::ut::Rect &rect);                 // 0x2A0
    virtual void put(wchar_t ch);                                       // 0x2A4
    virtual void createAnimation(MEMAllocator *allocator);              // 0x2A8
    virtual void drawHeader();                                          // 0x2AC
    virtual void drawBody();                                            // 0x2B0
    virtual void drawFooter();                                          // 0x2B4
    virtual void onCommandOnDispMode(INPUT_COMMAND command, void *arg); // 0x2B8
    virtual void onCommandOnEditMode(INPUT_COMMAND command, void *arg); // 0x2BC
    virtual void doAutoScroll();                                        // 0x2C0
    virtual bool isInScroll() {
        return mExScrollAnm.isActive();
    }

    void setEdited(bool edited) {
        mbEdited = edited;
    }
    void setGoodBye(bool goodbye) {
        mbGoodBye = goodbye;
    }
    void setEditScrollUp(bool up) {
        mbEditScrollUp = up;
    }
    void setEditScrollDown(bool down) {
        mbEditScrollDown = down;
    }
    void setCloseWithSend(bool closeWithSend) {
        mbCloseWithSend = closeWithSend;
    }
    void setEditMode(EditMode editMode);
    bool isEdited() {
        return mbEdited;
    }
    bool isGoodBye() {
        return mbGoodBye;
    }
    bool isEditScrollUp() {
        return mbEditScrollUp;
    }
    bool isEditScrollDown() {
        return mbEditScrollDown;
    }
    bool isCloseWithSend() {
        return mbCloseWithSend;
    }
    EditMode getEditMode() {
        return meEditMode;
    }
    tistring::Decolated *getSendString() {
        return mpSendString;
    }

  private:
    /* 0x1AE0 */ nw4r::lyt::Pane *mpMemoPane;
    /* 0x1AE4 */ nw4r::lyt::Pane *mpMemoRootPane;
    /* 0x1AE8 */ nw4r::lyt::Pane *mpTextPane;
    /* 0x1AEC */ nw4r::lyt::Pane *mpBoundPane;
    /* 0x1AF0 */ f32 mfScroll;
    /* 0x1AF4 */ f32 mfScrollFrom;
    /* 0x1AF8 */ f32 mfScrollTo;
    /* 0x1AFC */ int mnLine;
    /* 0x1B00 */ nw4r::ut::Rect mDrawRect;
    /* 0x1B10 */ nw4r::math::VEC3 mDefaultDrawTrans;
    /* 0x1B1C */ nw4r::lyt::Size mDefaultDrawSize;
    /* 0x1B24 */ nw4r::math::VEC3 mDefaultBoundTrans;
    /* 0x1B30 */ nw4r::lyt::Size mDefaultBoundSize;
    /* 0x1B38 */ Mtx mDrawMtx;
    /* 0x1B68 */ Mtx mBoundMtx;
    /* 0x1B98 */ util::Animation mExScrollAnm;
    /* 0x1BB8 */ NigaoeEventObserver *mpNigaoeObserver;
    /* 0x1BBC */ GXTexObj *mpDefaultNigaoe;
    /* 0x1BC0 */ tistring::Decolated *mpSendString;
    /* 0x1BC4 */ bool mbEdited;
    /* 0x1BC5 */ bool mbGoodBye;
    /* 0x1BC6 */ bool mbEditScrollUp;
    /* 0x1BC7 */ bool mbEditScrollDown;
    /* 0x1BC8 */ bool mbCloseWithSend;
    /* 0x1BCC */ ScrollButton *mpScrollButton;
    /* 0x1BD0 */ bool mbScrollUp;
    /* 0x1BD1 */ bool mbScrollDown;
    /* 0x1BD4 */ EditMode meEditMode;
}; // size 0x1BD8

} // namespace memo

namespace letter {

class InputForm : public memo::InputForm {
  private:
    /* 0x1BD8 */ u8 _1BD8[0x1BE0 - 0x1BD8];
}; // size 0x1BE0

} // namespace letter
} // namespace extend

typedef extend::memo::InputForm MemoInputForm;

} // namespace textinput
