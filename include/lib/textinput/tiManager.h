#pragma once

// textinput::Manager: the software keyboard (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <lib/textinput/tiHKBManager.h>
#include <lib/textinput/tiToolBar.h>
#include <lib/textinput/tiPredictLang.h>
#include <lib/textinput/tiSignWindow.h>
#include <lib/textinput/tiPcKeyboard.h>
#include <lib/textinput/tiCellPhone.h>
#include <lib/textinput/tiCandidateBox.h>
#include <lib/textinput/tiInputForm.h>
#include <stdio.h>

namespace textinput {

namespace keyboard {
namespace hwkey {
class HWKeyboard;
}
} // namespace keyboard

// The callbacks of the application.
class EventObserver {
public:
    virtual void onInput(CommandReceiver::INPUT_COMMAND command, void *data) {}   // 0x08
    virtual void onCommand(CommandReceiver::INPUT_COMMAND command, void *data) {} // 0x0C
    virtual void onEvent(nw4r::lyt::Pane *pane, u32 event) {}                     // 0x10
    virtual void onSE(sound::SE seId) {}                                          // 0x14
    virtual void onOK() { printf("OK!\n"); }                                      // 0x18
    virtual void onCancel() { printf("Cancel!\n"); }                              // 0x1C
    virtual void onOutOfLength() { printf("OutOfLength\n"); }                     // 0x20
};

class Manager : public Base {
public:
    Manager(MEMAllocator *allocator, nw4r::lyt::MultiArcResourceAccessor *multiArc, EventObserver *event); // 803071BC
    virtual ~Manager();                                                                // 0x08
    virtual void create(MEMAllocator *allocator);                                      // 0x0C
    virtual void init();                                                               // 0x10
    virtual void calc();                                                               // 0x14
    virtual void draw();                                                               // 0x18
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold, u32 release); // 0x1C
    virtual bool updateInput(input::HKBManager &hkbManager);                           // 0x20
    virtual wchar_t *getWCString() const;                                              // 0x24
    virtual void setWCString(const wchar_t *string);                                   // 0x28
    virtual void setLanguage(Language language);                                       // 0x2C
    virtual void setDestination(Destination destination);                             // 0x30
    virtual void limitStringLength(u32 limitStringLength);                             // 0x34
    virtual void limitRowNum(u32 limitRowNum);                                         // 0x38
    virtual void setAnimationOn(bool flag);                                            // 0x3C
    virtual void setAspectRatio(bool b4x3);                                            // 0x40
    virtual void setSecretInputMode(bool secretInputMode) {                            // 0x44
        mpInputForm->setSecretModeOn(secretInputMode);
    }
    virtual void setDefaultPredictionJP(int num, const char **predicts) {              // 0x48
        getInputForm()->getAtokString()->setDefaultPredictionJP(num, predicts);
    }
    virtual void setFixedPredictionJP(int num, const char **predicts) {                // 0x4C
        getInputForm()->getAtokString()->setFixedPredictionJP(num, predicts);
    }
    virtual void setTitleText(const wchar_t *titleText);                               // 0x50
    virtual void start();                                                              // 0x54
    virtual void end();                                                                // 0x58

    virtual inputform::LayoutByNW4R *getInputForm() { return mpInputForm; }                         // 0x5C
    virtual const inputform::LayoutByNW4R *getInputForm() const { return mpInputForm; }             // 0x60
    virtual keyboard::hwkey::HWKeyboard *getHWKeyboard() { return mpHWKeyboard; }                   // 0x64
    virtual const keyboard::hwkey::HWKeyboard *getHWKeyboard() const { return mpHWKeyboard; }       // 0x68
    virtual keyboard::pctype::LayoutByNW4R *getPCKeyboard() { return mpPCKeyboard; }                // 0x6C
    virtual const keyboard::pctype::LayoutByNW4R *getPCKeyboard() const { return mpPCKeyboard; }    // 0x70
    virtual keyboard::cellphonetype::LayoutByNW4R *getCellPhoneKeyboard() {                         // 0x74
        return mpCellPhoneKeyboard;
    }
    virtual const keyboard::cellphonetype::LayoutByNW4R *getCellPhoneKeyboard() const {             // 0x78
        return mpCellPhoneKeyboard;
    }
    virtual keyboard::signtype::LayoutByNW4R *getSignKeyboard() { return mpSignWindow; }            // 0x7C
    virtual const keyboard::signtype::LayoutByNW4R *getSignKeyboard() const { return mpSignWindow; } // 0x80
    virtual candidatebox::LayoutByNW4R *getCandidateBox() { return mpCandidateBox; }                // 0x84
    virtual const candidatebox::LayoutByNW4R *getCandidateBox() const { return mpCandidateBox; }    // 0x88
    virtual toolbar::LayoutByNW4R *getToolBar() { return mpToolBar; }                               // 0x8C
    virtual const toolbar::LayoutByNW4R *getToolBar() const { return mpToolBar; }                   // 0x90
    virtual predictlang::LayoutByNW4R *getPredictLanguageSelectDialog() {                           // 0x94
        return mpPredictLanguageDialog;
    }
    virtual const predictlang::LayoutByNW4R *getPredictLanguageSelectDialog() const {               // 0x98
        return mpPredictLanguageDialog;
    }

    virtual void *createEditBuffer();            // 0x9C
    virtual void *createHWKeyboard();            // 0xA0
    virtual void *createPCTypeKeyboard();        // 0xA4
    virtual void *createCellPhoneTypeKeyboard(); // 0xA8
    virtual void *createInputForm();             // 0xAC
    virtual void *createCandidateBox();          // 0xB0
    virtual void *createToolBar();               // 0xB4
    virtual void *createPredictLanguageDialog(); // 0xB8
    virtual void *createSignWindow();            // 0xBC
    virtual void setLayoutScale();               // 0xC0

    input::KBDListener *getKBDListenerRoot(); // 80308CF8: the root of HKBManager's listeners

protected:
    /* 0x04 */ MEMAllocator *mpAllocator;
    /* 0x08 */ nw4r::lyt::MultiArcResourceAccessor *mpMultiArcResourceAccessor;
    /* 0x0C */ keyboard::hwkey::HWKeyboard *mpHWKeyboard;
    /* 0x10 */ keyboard::pctype::LayoutByNW4R *mpPCKeyboard;
    /* 0x14 */ keyboard::cellphonetype::LayoutByNW4R *mpCellPhoneKeyboard;
    /* 0x18 */ inputform::LayoutByNW4R *mpInputForm;
    /* 0x1C */ candidatebox::LayoutByNW4R *mpCandidateBox;
    /* 0x20 */ toolbar::LayoutByNW4R *mpToolBar;
    /* 0x24 */ predictlang::LayoutByNW4R *mpPredictLanguageDialog;
    /* 0x28 */ keyboard::signtype::LayoutByNW4R *mpSignWindow;
    /* 0x2C */ EventObserver *mpEventObserver;
    /* 0x30 */ Destination meDestination;
    /* 0x34 */ Language meLanguage;
    /* 0x38 */ bool mbAspectRatio4x3;
}; // size 0x3C

} // namespace textinput
