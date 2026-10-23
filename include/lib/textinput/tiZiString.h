#pragma once

#include <lib/textinput/tiString.h>

namespace textinput {
namespace tistring {

class WithZi : public Decolated {
public:
    enum LetterMode {};
    enum PredictLanguage {};
    enum ZiLang {};

    WithZi(u16 maxLength) : Decolated(maxLength) {}

    virtual ~WithZi() {}                                     // 0x08
    virtual void pushBack(wchar_t ch) {}                     // 0x10
    virtual void popBack() {}                                // 0x14
    virtual void inputChar(wchar_t ch) {}                    // 0x50
    virtual void backSpace() {}                              // 0x5C
    virtual void confirm(const wchar_t *string) {}           // 0x60
    virtual bool moveCursorRight() { return false; }         // 0x64
    virtual bool moveCursorLeft() { return false; }          // 0x68
    virtual void init() {}                                   // 0xCC
    virtual int getCurrentNumPredicted() { return 0; }       // 0xD0
    virtual void getPredicted(int index, wchar_t *predicted) {} // 0xD4
    virtual bool isFix() { return true; }                    // 0xD8
    virtual wchar_t *getCurrentInput() { return NULL; }      // 0xDC
    virtual void changeLetterMode(LetterMode mode) {}        // 0xE0
    virtual void setPredictLaunguage(PredictLanguage language) {} // 0xE4
    virtual void setSelectedCandidate(u32 index) {}          // 0xE8
    virtual u32 getInputStringLength() { return 0; }         // 0xEC
    virtual void ChangeDictionaryLanguage(ZiLang language) {} // 0xF0
    virtual void setCellPhoneHoldingkey(void *key) {}        // 0xF4

    void getCurrentInput(wchar_t *buffer, u32 length) {}
};

} // namespace tistring
} // namespace textinput
