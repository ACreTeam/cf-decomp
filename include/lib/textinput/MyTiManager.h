#pragma once

#include <lib/textinput/tiManager.h>
#include <lib/textinput/MyTiInputForm.h>

namespace textinput {
namespace extend {

namespace bg {
class LayoutByNW4R;
}

namespace letter {
class InputForm;
}

namespace memo {

struct Setting {
    void init() {
        uRevisionAndType = 0xF0;
        for (u32 i = 0; i < sizeof(uRawData.val); i++) {
            uRawData.val[i] = 0;
        }
    }

    /* 0x00 */ u8 uRevisionAndType;
    union {
        struct {
            /* 0x01 */ u8 uDictionary;
            /* 0x02 */ u8 uPredictOnOff;
            /* 0x03 */ u8 uSignPage;
            /* 0x04 */ u8 uKeitaiUpperCaseJP : 1;
            /* 0x04 */ u8 uKeitaiInputMode : 4;
            /* 0x04 */ u8 uQwertyABC : 1;
            /* 0x04 */ u8 uNumLockOff : 1;
            /* 0x04 */ u8 uReserve2 : 1;
            /* 0x05 */ u8 uABCInputMode : 4;
            /* 0x05 */ u8 uAIUInputMode : 4;
            /* 0x06 */ u8 uReserve3;
            /* 0x07 */ u8 uReserve4;
        };
        struct {
            /* 0x01 */ u8 val[7];
        } uRawData;
    };
}; // size 0x8

class State;

class Manager : public textinput::Manager {
  public:
    enum ConfigType {
        CT_Default,
        CT_Letter,
        CT_PhotoLetter,
        CT_Numeric,
        CT_NormalWithoutLineFeed,
        CT_NormalBigTextWithoutLineFeed,
        CT_OnlyQwertyWithoutLineFeedAndSign,
        CT_OnlyQwertyBigTextWithoutLineFeedAndSign,
        CT_NumericWithDot,
        CT_NumericBigTextWithDot,
        CT_NormalBigTextWithoutLineFeedWithSign,
        CT_NumericWithSeparator,
        CT_NormalWithoutLineFeedWithSign,
        CT_PredictWithoutLineFeed,
        CT_Last,
    };

    enum StateType {
        ST_Hidden,
        ST_Appearing,
        ST_Visible,
        ST_Disappearing,
        ST_Last,
    };

    enum StateTimeLine {
        STL_Still,
        STL_Transition,
        STL_Last,
    };

    Manager(MEMAllocator *allocator,
            nw4r::lyt::MultiArcResourceAccessor *multiArc, EventObserver *event)
        : textinput::Manager(allocator, multiArc, event), mpCurrentState(NULL),
          meConfigType(CT_Letter), mpMemoInputForm(NULL),
          mpLetterInputForm(NULL), mpBigTextInputForm(NULL),
          mpDefaultPCKeyboard(NULL), mpDefaultCellPhoneKeyboard(NULL),
          mpDefaultInputForm(NULL), mpDefaultCandidateBox(NULL),
          mpDefaultToolBar(NULL), mpDefaultPredictLanguageDialog(NULL),
          mpDefaultSignWindow(NULL), mpBackGround(NULL) {
    }

    virtual ~Manager();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void calc();
    virtual void draw();
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release);
    virtual bool updateInput(input::HKBManager &hkbManager);
    virtual void start();
    virtual void end();
    virtual inputform::LayoutByNW4R *createInputForm();

    virtual void memoDraw(); // 0xC4
    virtual void setSetting(const Setting &setting) {
        mMemoSetting = setting;
    } // 0xC8
    virtual Setting getSetting() {
        return mMemoSetting;
    }                                                 // 0xCC
    virtual void changeState(StateTimeLine timeLine); // 0xD0
    virtual State *getState() {
        return mpCurrentState;
    }                                           // 0xD4
    virtual void setState(StateType stateType); // 0xD8
    virtual ConfigType getConfigType() {
        return meConfigType;
    }                                                             // 0xDC
    virtual void setSaveData_();                                  // 0xE0
    virtual void reflectSaveData();                               // 0xE4
    virtual void configDefault();                                 // 0xE8
    virtual void configLetter();                                  // 0xEC
    virtual void configPhotoLetter();                             // 0xF0
    virtual void configNumeric();                                 // 0xF4
    virtual void configNormalWithoutLineFeed();                   // 0xF8
    virtual void configNormalBigTextWithoutLineFeed();            // 0xFC
    virtual void configOnlyQwertyWithoutLineFeedAndSign();        // 0x100
    virtual void configOnlyQwertyBigTextWithoutLineFeedAndSign(); // 0x104
    virtual void configNumericWithDot();                          // 0x108
    virtual void configNumericBigTextWithDot();                   // 0x10C
    virtual void configNormalBigTextWithoutLineFeedWithSign();    // 0x110
    virtual void configNumericWithSeparator();                    // 0x114
    virtual void configNormalWithoutLineFeedWithSign();           // 0x118
    virtual void configPredictWithoutLineFeed();                  // 0x11C
    virtual InputForm *createMemoInputForm();                     // 0x120
    virtual InputForm *createLetterInputForm();                   // 0x124
    virtual inputform::LayoutByNW4R *createBigTextInputForm();    // 0x128
    virtual bg::LayoutByNW4R *createBG();                         // 0x12C

    void reflectSaveDataRev1();
    void reflectSaveDataDefault();
    bg::LayoutByNW4R *getBG() {
        return mpBackGround;
    }

  protected:
    /* 0x3C */ State *mpCurrentState;
    /* 0x40 */ ConfigType meConfigType;
    /* 0x44 */ Setting mMemoSetting;
    /* 0x4C */ InputForm *mpMemoInputForm;
    /* 0x50 */ InputForm *mpLetterInputForm;
    /* 0x54 */ inputform::LayoutByNW4R *mpBigTextInputForm;
    /* 0x58 */ keyboard::pctype::LayoutByNW4R *mpDefaultPCKeyboard;
    /* 0x5C */ keyboard::cellphonetype::LayoutByNW4R
        *mpDefaultCellPhoneKeyboard;
    /* 0x60 */ inputform::LayoutByNW4R *mpDefaultInputForm;
    /* 0x64 */ candidatebox::LayoutByNW4R *mpDefaultCandidateBox;
    /* 0x68 */ toolbar::LayoutByNW4R *mpDefaultToolBar;
    /* 0x6C */ predictlang::LayoutByNW4R *mpDefaultPredictLanguageDialog;
    /* 0x70 */ keyboard::signwindow::LayoutByNW4R *mpDefaultSignWindow;
    /* 0x74 */ bg::LayoutByNW4R *mpBackGround;
}; // size 0x78

class State {
  public:
    State() : mpManager(NULL) {
    }

    virtual ~State() {
    } // 0x08
    virtual Manager::StateType getStateType() = 0; // 0x0C
    virtual void create() = 0;                     // 0x10
    virtual void init() = 0;                       // 0x14
    virtual void draw() = 0;                       // 0x18
    virtual void memoDraw() = 0;                   // 0x1C
    virtual void calc() = 0;                       // 0x20
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release) = 0;                          // 0x24
    virtual bool updateInput(input::HKBManager &hkbManager) = 0;        // 0x28
    virtual void start() = 0;                                           // 0x2C
    virtual void end() = 0;                                             // 0x30
    virtual extend::memo::InputForm *InputForm() {
        return static_cast<extend::memo::InputForm *>(
            mpManager->getInputForm());
    } // 0x34
    virtual keyboard::hwkey::HWKeyboard *HWKeyboard() {
        return mpManager->getHWKeyboard();
    } // 0x38
    virtual keyboard::pctype::LayoutByNW4R *PCKeyboard() {
        return mpManager->getPCKeyboard();
    } // 0x3C
    virtual keyboard::cellphonetype::LayoutByNW4R *CellPhoneKeyboard() {
        return mpManager->getCellPhoneKeyboard();
    } // 0x40
    virtual keyboard::signwindow::LayoutByNW4R *SignKeyboard() {
        return mpManager->getSignKeyboard();
    } // 0x44
    virtual candidatebox::LayoutByNW4R *CandidateBox() {
        return mpManager->getCandidateBox();
    } // 0x48
    virtual toolbar::LayoutByNW4R *ToolBar() {
        return mpManager->getToolBar();
    } // 0x4C
    virtual predictlang::LayoutByNW4R *PredictLanguageSelectDialog() {
        return mpManager->getPredictLanguageSelectDialog();
    } // 0x50
    virtual bg::LayoutByNW4R *BG() {
        return mpManager->getBG();
    } // 0x54

    void setManager(Manager *manager) {
        mpManager = manager;
    }

  protected:
    /* 0x04 */ Manager *mpManager;
}; // size 0x8

} // namespace memo
} // namespace extend

typedef extend::memo::Manager MemoManager;

} // namespace textinput
